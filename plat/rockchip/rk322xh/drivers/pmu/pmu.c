/*
 * Copyright (C) 2017, Fuzhou Rockchip Electronics Co., Ltd.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#include <arch_helpers.h>
#include <debug.h>
#include <assert.h>
#include <bakery_lock.h>
#include <bl31.h>
#include <console.h>
#include <delay_timer.h>
#include <errno.h>
#include <mmio.h>
#include <platform.h>
#include <platform_def.h>
#include <plat_private.h>
#include <pmu_sram.h>
#include <pmu.h>
#include <pmu_com.h>
#include <pwm_remotectl.h>
#include <rk322xh_def.h>
#include <rockchip_exceptions.h>
#include <monitor.h>
#include <sram.h>
#include "pmu_prt_dbg.c"

DEFINE_BAKERY_LOCK(rockchip_pd_lock);

#define CPU_PD_CTR_FLG_HW 1

static struct psram_data_t *psram_sleep_cfg =
		(struct psram_data_t *)PSRAM_DT_BASE;

#if !CPU_PD_CTR_FLG_HW
static uint32_t cores_pd_cfg_info[PLATFORM_CORE_COUNT]
#if USE_COHERENT_MEM
__attribute__ ((section("tzfw_coherent_mem")))
#endif
;/* coheront */
#endif

static struct rk3328_sleep_ddr_data ddr_data;
static __sramdata struct rk3328_sleep_sram_data sram_data;

static uint32_t cpu_warm_boot_addr;

#pragma weak rk3328_pmic_suspend
#pragma weak rk3328_pmic_resume

void plat_rockchip_pmusram_prepare(void)
{
	uint32_t *sram_dst, *sram_src;
	size_t sram_size = 2;
	/*
	 * pmu sram code and data prepare
	 */
	sram_dst = (uint32_t *)PMUSRAM_BASE;
	sram_src = (uint32_t *)&pmu_cpuson_entrypoint_start;
	sram_size = (uint32_t *)&pmu_cpuson_entrypoint_end -
		    (uint32_t *)sram_src;
	u32_align_cpy(sram_dst, sram_src, sram_size);

	psram_sleep_cfg->sp = PSRAM_DT_BASE;
}

#if CPU_PD_CTR_FLG_HW
static inline uint32_t get_cpus_pwr_domain_cfg_info(uint32_t cpu_id)
{
	uint32_t pd_reg, apm_reg;

	pd_reg = mmio_read_32(PMU_BASE + PMU_PWRDN_CON) & BIT(cpu_id);
	apm_reg = mmio_read_32(PMU_BASE + PMU_CPUAPM_CON(cpu_id)) &
			       BIT(core_pm_en);

	if (pd_reg && !apm_reg)
		return core_pwr_pd;
	else if (!pd_reg && apm_reg)
		return core_pwr_wfi;

	ERROR("%s: 0x%x, 0x%x\n", __func__, pd_reg, apm_reg);
	while (1)
	;
}

static inline void set_cpus_pwr_domain_cfg_info(uint32_t cpu_id, uint32_t value)
{
}
#else

static inline uint32_t get_cpus_pwr_domain_cfg_info(uint32_t cpu_id)
{
	return cores_pd_cfg_info[cpu_id];
}

static inline void set_cpus_pwr_domain_cfg_info(uint32_t cpu_id, uint32_t value)
{
	cores_pd_cfg_info[cpu_id] = value;

#if !USE_COHERENT_MEM
	flush_dcache_range((uintptr_t)&cores_pd_cfg_info[cpu_id],
			   sizeof(uint32_t));
#endif
}
#endif

static int cpus_power_domain_on(uint32_t cpu_id)
{
	uint32_t cpu_pd, apm_value, cfg_info, loop = 0;

	cpu_pd = PD_CPU0 + cpu_id;
	cfg_info = get_cpus_pwr_domain_cfg_info(cpu_id);

	if (cfg_info == core_pwr_pd) {
		/* disable apm cfg */
		mmio_write_32(PMU_BASE + PMU_CPUAPM_CON(cpu_id),
			      CORES_PM_DISABLE);
		if (pmu_power_domain_st(cpu_pd) == pmu_pd_on) {
			mmio_write_32(PMU_BASE + PMU_CPUAPM_CON(cpu_id),
				      CORES_PM_DISABLE);
			pmu_power_domain_ctr(cpu_pd, pmu_pd_off);
		}
		pmu_power_domain_ctr(cpu_pd, pmu_pd_on);
	} else {
		/* wait cpu down */
		while (pmu_power_domain_st(cpu_pd) == pmu_pd_on && loop < 100) {
			udelay(2);
			loop++;
		}

		/* return error if can't wait cpu down */
		if (pmu_power_domain_st(cpu_pd) == pmu_pd_on) {
			WARN("%s:can't wait cpu down\n", __func__);
			return -EINVAL;
		}

		/* power up cpu in power down state */
		apm_value = BIT(core_pm_sft_wakeup_en);
		mmio_write_32(PMU_BASE + PMU_CPUAPM_CON(cpu_id), apm_value);
	}

	return 0;
}

static int cpus_power_domain_off(uint32_t cpu_id, uint32_t pd_cfg)
{
	uint32_t cpu_pd, apm_value;

	cpu_pd = PD_CPU0 + cpu_id;
	if (pmu_power_domain_st(cpu_pd) == pmu_pd_off)
		return 0;

	if (pd_cfg == core_pwr_pd) {
		if (check_cpu_wfie(cpu_id, CKECK_WFEI_MSK))
			return -EINVAL;
		/* disable apm cfg */
		mmio_write_32(PMU_BASE + PMU_CPUAPM_CON(cpu_id),
			      CORES_PM_DISABLE);
		set_cpus_pwr_domain_cfg_info(cpu_id, pd_cfg);
		pmu_power_domain_ctr(cpu_pd, pmu_pd_off);
	} else {
		set_cpus_pwr_domain_cfg_info(cpu_id, pd_cfg);
		apm_value = BIT(core_pm_en) | BIT(core_pm_dis_int);
		if (pd_cfg == core_pwr_wfi_int)
			apm_value |= BIT(core_pm_int_wakeup_en);
		mmio_write_32(PMU_BASE + PMU_CPUAPM_CON(cpu_id), apm_value);
	}

	return 0;
}

static void nonboot_cpus_off(void)
{
	uint32_t boot_cpu, cpu;

	/* turn off noboot cpus */
	boot_cpu = plat_my_core_pos();
	for (cpu = 0; cpu < PLATFORM_CORE_COUNT; cpu++) {
		if (cpu == boot_cpu)
			continue;
		cpus_power_domain_off(cpu, core_pwr_pd);
	}
}

int rockchip_soc_cores_pwr_dm_on(unsigned long mpidr, uint64_t entrypoint)
{
	uint32_t cpu_id = plat_core_pos_by_mpidr(mpidr);

	assert(cpuson_flags[cpu_id] == 0);
	cpuson_flags[cpu_id] = PMU_CPU_HOTPLUG;
	cpuson_entry_point[cpu_id] = entrypoint;
	dsb();

	cpus_power_domain_on(cpu_id);

	return 0;
}

int rockchip_soc_cores_pwr_dm_off(void)
{
	uint32_t cpu_id = plat_my_core_pos();

	cpus_power_domain_off(cpu_id, core_pwr_wfi);

	return 0;
}

int rockchip_soc_cores_pwr_dm_suspend(void)
{
	uint32_t cpu_id = plat_my_core_pos();

	assert(cpuson_flags[cpu_id] == 0);
	cpuson_flags[cpu_id] = PMU_CPU_AUTO_PWRDN;
	cpuson_entry_point[cpu_id] = (uintptr_t)plat_get_sec_entrypoint();
	dsb();

	cpus_power_domain_off(cpu_id, core_pwr_wfi_int);

	return 0;
}

int rockchip_soc_cores_pwr_dm_on_finish(void)
{
	uint32_t cpu_id = plat_my_core_pos();

	mmio_write_32(PMU_BASE + PMU_CPUAPM_CON(cpu_id), CORES_PM_DISABLE);

	return 0;
}

int rockchip_soc_cores_pwr_dm_resume(void)
{
	uint32_t cpu_id = plat_my_core_pos();

	mmio_write_32(PMU_BASE + PMU_CPUAPM_CON(cpu_id), CORES_PM_DISABLE);

	return 0;
}

void __dead2 rockchip_soc_soft_reset(void)
{
	mmio_write_32(CRU_BASE + CRU_CRU_MODE, PLL_SLOW_MODE(CPLL_ID));
	mmio_write_32(CRU_BASE + CRU_CRU_MODE, PLL_SLOW_MODE(GPLL_ID));
	mmio_write_32(CRU_BASE + CRU_CRU_MODE, PLL_SLOW_MODE(NPLL_ID));
	mmio_write_32(CRU_BASE + CRU_CRU_MODE, PLL_SLOW_MODE(APLL_ID));
	dsb();

	mmio_write_32(CRU_BASE + CRU_GLB_SRST_FST, CRU_GLB_SRST_FST_VALUE);
	dsb();
	/*
	 * Maybe the HW needs some times to reset the system,
	 * so we do not hope the core to excute valid codes.
	 */
	while (1)
		;
}

/*
 * For PMIC RK805, its sleep pin is connect with gpio2_d2 from rk3328.
 * If the PMIC is configed for responding the sleep pin to power off it,
 * once the pin is output high,  it will get the pmic power off.
 */
void __dead2 rockchip_soc_system_off(void)
{
	uint32_t val;

	/* gpio config */
	val = mmio_read_32(GRF_BASE + GRF_GPIO2D_IOMUX);
	val &= ~GPIO2_D2_GPIO_MODE;
	mmio_write_32(GRF_BASE + GRF_GPIO2D_IOMUX, val);

	/* config output */
	val = mmio_read_32(GPIO2_BASE + SWPORTA_DDR);
	val |= GPIO2_D2;
	mmio_write_32(GPIO2_BASE + SWPORTA_DDR, val);

	/* config output high level */
	val = mmio_read_32(GPIO2_BASE);
	val |= GPIO2_D2;
	mmio_write_32(GPIO2_BASE, val);
	dsb();

	while (1)
		;
}

static uint32_t clk_ungt_msk[CRU_CLKGATE_NUMS] = {
	0x187f, 0x0000, 0x010c, 0x0000, 0x0200,
	0x0010, 0x0000, 0x0017, 0x001f, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0003, 0x0000,
	0xf001, 0x27c0, 0x04D9, 0x03ff, 0x0000,
	0x0000, 0x0000, 0x0010, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0003, 0x0008
};

static void clks_gating_suspend(uint32_t *ungt_msk)
{
	int i;

	for (i = 0; i < CRU_CLKGATE_NUMS; i++) {
		ddr_data.clk_ungt_save[i] =
			mmio_read_32(CRU_BASE + CRU_CLKGATE_CON(i));
		//dbg_clks_gating_suspend(ungt_msk);
		mmio_write_32(CRU_BASE + CRU_CLKGATE_CON(i),
			      ((~ungt_msk[i]) << 16) | 0xffff);
	}
}

static void clks_gating_resume(void)
{
	int i;

	for (i = 0; i < CRU_CLKGATE_NUMS; i++)
		mmio_write_32(CRU_BASE + CRU_CLKGATE_CON(i),
			      ddr_data.clk_ungt_save[i] | 0xffff0000);
}

static inline void pm_pll_wait_lock(uint32_t pll_id)
{
	uint32_t delay = PLL_LOCKED_TIMEOUT;

	while (delay > 0) {
		if (mmio_read_32(CRU_BASE + PLL_CONS(pll_id, 1)) &
		    PLL_IS_LOCKED)
			break;
		delay--;
	}
	if (delay == 0)
		ERROR("lock-pll: %d\n", pll_id);
}

static inline void pll_pwr_dwn(uint32_t pll_id, uint32_t pd)
{
	mmio_write_32(CRU_BASE + PLL_CONS(pll_id, 1),
		      BITS_WITH_WMASK(1, 1, 15));
	if (pd)
		mmio_write_32(CRU_BASE + PLL_CONS(pll_id, 1),
			      BITS_WITH_WMASK(1, 1, 14));
	else
		mmio_write_32(CRU_BASE + PLL_CONS(pll_id, 1),
			      BITS_WITH_WMASK(0, 1, 14));
}

static __sramfunc void dpll_suspend(void)
{
	int i;

	/* slow mode */
	mmio_write_32(CRU_BASE + CRU_CRU_MODE, PLL_SLOW_MODE(DPLL_ID));

	/* save pll con */
	for (i = 0; i < CRU_PLL_CON_NUMS; i++)
		sram_data.dpll_con_save[i] =
				mmio_read_32(CRU_BASE + PLL_CONS(DPLL_ID, i));
	mmio_write_32(CRU_BASE + PLL_CONS(DPLL_ID, 1),
		      BITS_WITH_WMASK(1, 1, 15));
	mmio_write_32(CRU_BASE + PLL_CONS(DPLL_ID, 1),
		      BITS_WITH_WMASK(1, 1, 14));
}

static __sramfunc void dpll_resume(void)
{
	uint32_t delay = PLL_LOCKED_TIMEOUT;

	mmio_write_32(CRU_BASE + PLL_CONS(DPLL_ID, 1),
		      BITS_WITH_WMASK(1, 1, 15));
	mmio_write_32(CRU_BASE + PLL_CONS(DPLL_ID, 1),
		      BITS_WITH_WMASK(0, 1, 14));
	mmio_write_32(CRU_BASE + PLL_CONS(DPLL_ID, 1),
		      sram_data.dpll_con_save[1] | 0xc0000000);

	dsb();

	while (delay > 0) {
		if (mmio_read_32(CRU_BASE + PLL_CONS(DPLL_ID, 1)) &
				 PLL_IS_LOCKED)
			break;
		delay--;
	}
	if (delay == 0)
		while (1)
			;

	mmio_write_32(CRU_BASE + CRU_CRU_MODE,
		      PLL_NORM_MODE(DPLL_ID));
}

static inline void pll_suspend(uint32_t pll_id)
{
	int i;

	/* slow mode */
	mmio_write_32(CRU_BASE + CRU_CRU_MODE, PLL_SLOW_MODE(pll_id));

	/* save pll con */
	for (i = 0; i < CRU_PLL_CON_NUMS; i++)
		ddr_data.cru_plls_con_save[pll_id][i] =
				mmio_read_32(CRU_BASE + PLL_CONS(pll_id, i));

	/* powerdown pll */
	pll_pwr_dwn(pll_id, pmu_pd_off);
}

static inline void pll_resume(uint32_t pll_id)
{
	mmio_write_32(CRU_BASE + PLL_CONS(pll_id, 1),
		      ddr_data.cru_plls_con_save[pll_id][1] | 0xc0000000);

	pm_pll_wait_lock(pll_id);

	if (PLL_IS_NORM_MODE(ddr_data.cru_mode_save, pll_id))
		mmio_write_32(CRU_BASE + CRU_CRU_MODE,
			      PLL_NORM_MODE(pll_id));
}

static void pm_plls_suspend(void)
{
	ddr_data.cru_mode_save = mmio_read_32(CRU_BASE + CRU_CRU_MODE);
	ddr_data.clk_sel0 = mmio_read_32(CRU_BASE + CRU_CLKSEL_CON(0));
	ddr_data.clk_sel1 = mmio_read_32(CRU_BASE + CRU_CLKSEL_CON(1));
	ddr_data.clk_sel18 = mmio_read_32(CRU_BASE + CRU_CLKSEL_CON(18));
	ddr_data.clk_sel20 = mmio_read_32(CRU_BASE + CRU_CLKSEL_CON(20));
	ddr_data.clk_sel24 = mmio_read_32(CRU_BASE + CRU_CLKSEL_CON(24));
	ddr_data.clk_sel38 = mmio_read_32(CRU_BASE + CRU_CLKSEL_CON(38));
	dbg_pm_plls_suspend();
	pll_suspend(NPLL_ID);
	pll_suspend(CPLL_ID);
	pll_suspend(GPLL_ID);
	pll_suspend(APLL_ID);

	/* core */
	mmio_write_32(CRU_BASE + CRU_CLKSEL_CON(0),
		      BITS_WITH_WMASK(0, 0x1f, 0));

	/* pclk_dbg */
	mmio_write_32(CRU_BASE + CRU_CLKSEL_CON(1),
		      BITS_WITH_WMASK(0, 0xf, 0));

	/* crypto */
	mmio_write_32(CRU_BASE + CRU_CLKSEL_CON(20),
		      BITS_WITH_WMASK(0, 0x1f, 0));

	/* pwm0 */
	mmio_write_32(CRU_BASE + CRU_CLKSEL_CON(24),
		      BITS_WITH_WMASK(0, 0x7f, 8));

	/* uart2 from 24M */
	mmio_write_32(CRU_BASE + CRU_CLKSEL_CON(18),
		      BITS_WITH_WMASK(2, 0x3, 8));

	/* clk_rtc32k */
	mmio_write_32(CRU_BASE + CRU_CLKSEL_CON(38),
		      BITS_WITH_WMASK(732, 0x3fff, 0) |
		      BITS_WITH_WMASK(2, 0x3, 14));
}

static void pm_plls_resume(void)
{
	/* clk_rtc32k */
	mmio_write_32(CRU_BASE + CRU_CLKSEL_CON(38),
		      ddr_data.clk_sel38 |
		      BITS_WMSK(0x3fff, 0) |
		      BITS_WMSK(0x3, 14));

	/* uart2 */
	mmio_write_32(CRU_BASE + CRU_CLKSEL_CON(18),
		      ddr_data.clk_sel18 | BITS_WMSK(0x3, 8));

	/* pwm0 */
	mmio_write_32(CRU_BASE + CRU_CLKSEL_CON(24),
		      ddr_data.clk_sel24 | BITS_WMSK(0x7f, 8));

	/* crypto */
	mmio_write_32(CRU_BASE + CRU_CLKSEL_CON(20),
		      ddr_data.clk_sel20 | BITS_WMSK(0x1f, 0));

	/* pclk_dbg */
	mmio_write_32(CRU_BASE + CRU_CLKSEL_CON(1),
		      ddr_data.clk_sel1 | BITS_WMSK(0xf, 0));

	/* core */
	mmio_write_32(CRU_BASE + CRU_CLKSEL_CON(0),
		      ddr_data.clk_sel0 | BITS_WMSK(0x1f, 0));

	pll_pwr_dwn(APLL_ID, pmu_pd_on);
	pll_pwr_dwn(GPLL_ID, pmu_pd_on);
	pll_pwr_dwn(CPLL_ID, pmu_pd_on);
	pll_pwr_dwn(NPLL_ID, pmu_pd_on);

	pll_resume(APLL_ID);
	pll_resume(GPLL_ID);
	pll_resume(CPLL_ID);
	pll_resume(NPLL_ID);
	dbg_pm_plls_resume();
}

/*
 * For PMIC RK805, its sleep pin is connect with gpio2_d2 from rk3328.
 * If the PMIC is configed for responding the sleep pin
 * to get it into sleep mode,
 * once the pin is output high,  it will get the pmic into sleep mode.
 */
__sramfunc void rk3328_pmic_suspend(void)
{
	sram_data.pmic_sleep_save = mmio_read_32(GRF_BASE + PMIC_SLEEP_REG);
	sram_data.pmic_sleep_gpio_save[1] = mmio_read_32(GPIO2_BASE + 4);
	sram_data.pmic_sleep_gpio_save[0] = mmio_read_32(GPIO2_BASE);
	mmio_write_32(GRF_BASE + PMIC_SLEEP_REG, BITS_WITH_WMASK(0, 0x3, 4));
	mmio_write_32(GPIO2_BASE + 4,
		      sram_data.pmic_sleep_gpio_save[1] | BIT(26));
	mmio_write_32(GPIO2_BASE,
		      sram_data.pmic_sleep_gpio_save[0] | BIT(26));
}

__sramfunc void  rk3328_pmic_resume(void)
{
	mmio_write_32(GPIO2_BASE, sram_data.pmic_sleep_gpio_save[0]);
	mmio_write_32(GPIO2_BASE + 4, sram_data.pmic_sleep_gpio_save[1]);
	mmio_write_32(GRF_BASE + PMIC_SLEEP_REG,
		      sram_data.pmic_sleep_save | BITS_WMSK(0xffff, 0));
	/* Resuming volt need a lot of time */
	sram_udelay(100);
}

static inline void rockchip_set_sram_sp(uint64_t set_sp)
{
	__asm volatile("mov sp, %0\n"::"r" (set_sp) : "sp");
}

static __sramfunc void ddr_suspend(void)
{
	sram_data.pd_sr_idle_save = mmio_read_32(DDR_UPCTL_BASE +
						 DDR_PCTL2_PWRCTL) & SELFREF_EN;
	mmio_clrbits_32(DDR_UPCTL_BASE + DDR_PCTL2_PWRCTL, SELFREF_EN);
	sram_data.ddr_grf_con0 = mmio_read_32(DDR_GRF_BASE +
					      DDRGRF_SOC_CON(0));
	mmio_write_32(DDR_GRF_BASE, BIT_WITH_WMSK(14) | WMSK_BIT(15));

	/*
	 * Override csysreq from ddrc and
	 * send valid csysreq signal to PMU,
	 * csysreq is controlled by ddrc only
	 */

	/* in self-refresh */
	mmio_setbits_32(PMU_BASE + PMU_SFT_CON, BIT(0));
	while ((mmio_read_32(DDR_GRF_BASE + DDRGRF_SOC_STATUS(1)) &
		(0x03 << 12)) !=  (0x02 << 12))
		;
	/* ddr retention */
	mmio_setbits_32(PMU_BASE + PMU_SFT_CON, BIT(2));

	/* ddr gating */
	mmio_write_32(CRU_BASE + CRU_CLKGATE_CON(0),
		      BITS_WITH_WMASK(0x7, 0x7, 4));
	mmio_write_32(CRU_BASE + CRU_CLKGATE_CON(7),
		      BITS_WITH_WMASK(1, 1, 4));
	mmio_write_32(CRU_BASE + CRU_CLKGATE_CON(18),
		      BITS_WITH_WMASK(0x1ff, 0x1ff, 1));
	mmio_write_32(CRU_BASE + CRU_CLKGATE_CON(27),
		      BITS_WITH_WMASK(0x3, 0x3, 0));

	dpll_suspend();
}

static __sramfunc  void ddr_resume(void)
{
	dpll_resume();

	/* ddr gating */
	mmio_write_32(CRU_BASE + CRU_CLKGATE_CON(0),
		      BITS_WITH_WMASK(0, 0x7, 4));
	mmio_write_32(CRU_BASE + CRU_CLKGATE_CON(7),
		      BITS_WITH_WMASK(0, 1, 4));
	mmio_write_32(CRU_BASE + CRU_CLKGATE_CON(18),
		      BITS_WITH_WMASK(0, 0x1ff, 1));
	mmio_write_32(CRU_BASE + CRU_CLKGATE_CON(27),
		      BITS_WITH_WMASK(0, 0x3, 0));

	/* ddr de_retention */
	mmio_clrbits_32(PMU_BASE + PMU_SFT_CON, BIT(2));
	/* exit self-refresh */
	mmio_clrbits_32(PMU_BASE + PMU_SFT_CON, BIT(0));
	while ((mmio_read_32(DDR_GRF_BASE + DDRGRF_SOC_STATUS(1)) &
		(0x03 << 12)) !=  (0x00 << 12))
		;

	mmio_write_32(DDR_GRF_BASE, sram_data.ddr_grf_con0 | 0xc0000000);
	if (sram_data.pd_sr_idle_save)
		mmio_setbits_32(DDR_UPCTL_BASE + DDR_PCTL2_PWRCTL, SELFREF_EN);
}

static __sramfunc void sram_dbg_uart_suspend(void)
{
	sram_data.uart2_ier = mmio_read_32(UART2_BASE + UART_IER);
	mmio_write_32(UART2_BASE + UART_IER, UART_INT_DISABLE);
	mmio_write_32(CRU_BASE + CRU_CLKGATE_CON(16), 0x20002000);
	mmio_write_32(CRU_BASE + CRU_CLKGATE_CON(2), 0x00040004);
}

static __sramfunc void sram_dbg_uart_resume(void)
{
	/* restore uart clk and reset fifo */
	mmio_write_32(CRU_BASE + CRU_CLKGATE_CON(16), 0x20000000);
	mmio_write_32(CRU_BASE + CRU_CLKGATE_CON(2), 0x00040000);
	mmio_write_32(UART2_BASE + UART_FCR, UART_FIFO_RESET);
	mmio_write_32(UART2_BASE + UART_IER, sram_data.uart2_ier);
}

static __sramfunc void sram_soc_enter_lp(void)
{
	uint32_t apm_value;

	apm_value = BIT(core_pm_en) |
		    BIT(core_pm_dis_int) |
		    BIT(core_pm_int_wakeup_en);
	mmio_write_32(PMU_BASE + PMU_CPUAPM_CON(PD_CPU0), apm_value);

	dsb();
	isb();
err_loop:
	wfi();
	/*
	 *Soc will enter low power mode and
	 *do not return to here.
	 */
	goto err_loop;
}

__sramfunc void sram_suspend(void)
{
	/* disable mmu and icache */
	tlbialle3();
	disable_mmu_icache_el3();

	mmio_write_32(SGRF_BASE + SGRF_SOC_CON(1),
		      (PMUSRAM_BASE >> CPU_BOOT_ADDR_ALIGN) |
		      CPU_BOOT_ADDR_WMASK);

	putchar('3');

	/* ddr self-refresh and gating phy */
	ddr_suspend();
	sram_putchar('4');

	rk3328_pmic_suspend();

	sram_putchar('5');

	pwm_remotectl_prepare();

	sram_dbg_uart_suspend();

	sram_soc_enter_lp();
}

static __sramfunc void sys_resume_first(void)
{
	sram_dbg_uart_resume();

	sram_putchar('t');

	if (pwm_remotectl_wakeup())
		sram_soc_enter_lp();
	sram_putchar('m');

	rk3328_pmic_resume();
	sram_putchar('4');

	/* ddr self-refresh exit */
	ddr_resume();
	putchar('3');
	/* disable apm cfg */
	mmio_write_32(PMU_BASE + PMU_CPUAPM_CON(0), CORES_PM_DISABLE);

	/* the warm booting address of cpus */
	mmio_write_32(SGRF_BASE + SGRF_SOC_CON(1),
		      (cpu_warm_boot_addr >> CPU_BOOT_ADDR_ALIGN) |
		      CPU_BOOT_ADDR_WMASK);
}

void __dead2 rockchip_soc_sys_pd_pwr_dn_wfi(void)
{
	rockchip_set_sram_sp(PSRAM_DT_BASE);

	sram_suspend();

	/* should never reach here */
	psci_power_down_wfi();
}

int rockchip_soc_sys_pwr_dm_suspend(void)
{
	putchar('\n');
	dbg_rk322x_irq_prepare();

	putchar('0');

	clks_gating_suspend(clk_ungt_msk);
	putchar('1');

	pm_plls_suspend();
	putchar('2');

	return 0;
}

int rockchip_soc_sys_pwr_dm_resume(void)
{
	putchar('2');

	pm_plls_resume();
	putchar('1');

	clks_gating_resume();
	putchar('0');

	plat_rockchip_gic_cpuif_enable();

	dbg_rk322x_irq_finish();
	putchar('\n');

	return 0;
}

void plat_rockchip_pmu_init(void)
{
	uint32_t cpu;

	for (cpu = 0; cpu < PLATFORM_CORE_COUNT; cpu++)
		cpuson_flags[cpu] = 0;

	cpu_warm_boot_addr = (uint64_t)platform_cpu_warmboot;
	psram_sleep_cfg->ddr_func = (uint64_t)sys_resume_first;
	psram_sleep_cfg->ddr_data = 0x00;
	psram_sleep_cfg->ddr_flag = 0x01;
	psram_sleep_cfg->boot_mpidr = read_mpidr_el1() & 0xffff;

	/* the warm booting address of cpus */
	mmio_write_32(SGRF_BASE + SGRF_SOC_CON(1),
		      (cpu_warm_boot_addr >> CPU_BOOT_ADDR_ALIGN) |
		      CPU_BOOT_ADDR_WMASK);

	nonboot_cpus_off();

	rk_register_interrupt_routing_model();
	prt_rk_soc_monitor_init();

	INFO("%s: pd status 0x%x\n",
	     __func__, mmio_read_32(PMU_BASE + PMU_PWRDN_ST));
}
