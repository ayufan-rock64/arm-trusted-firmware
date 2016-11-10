/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
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

#include <debug.h>
#include <mmio.h>
#include <psci.h>
#include <rockchip_sip_svc.h>
#include <runtime_svc.h>
#include <dram.h>
#include <plat_private.h>
#include <plat_sip_calls.h>
#include <arch_helpers.h>
#include <platform.h>
#include <dfs.h>

static int ddr_smc_handler(uint64_t arg0, uint64_t arg1,
		    uint64_t id, struct arm_smccc_res *res)
{
	switch (id) {
	case CONFIG_DRAM_INIT:
		ddr_dfs_init();
		break;
	case CONFIG_DRAM_SET_RATE:
		return ddr_set_rate((uint32_t)arg0);
	case CONFIG_DRAM_ROUND_RATE:
		return ddr_round_rate((uint32_t)arg0);
	case CONFIG_DRAM_GET_RATE:
		return ddr_get_rate();
	case CONFIG_DRAM_CLR_IRQ:
		clr_dcf_irq();
		break;
	case CONFIG_DRAM_SET_PARAM:
		dts_timing_receive((uint32_t)arg0, (uint32_t)arg1);
		break;
	default:
		return SIP_RET_INVALID_PARAMS;
	}

	return SIP_RET_SUCCESS;
}

#pragma weak fiq_debugger_smc_handler
#pragma weak fiq_disable_flag
#pragma weak fiq_enable_flag
#pragma weak get_uart_irq_id

void fiq_disable_flag(uint32_t cpu_id)
{
}

void fiq_enable_flag(uint32_t cpu_id)
{
}

uint32_t get_uart_irq_id(void)
{
	return 0;
}
uint64_t fiq_debugger_smc_handler(uint64_t fun_id, void *handle,
				  uint64_t arg0, uint64_t arg1)
{
	ERROR("%s: unhandled SMC (0x%lx)\n", __func__, fun_id);
	SMC_RET1(handle, SMC_UNK);
}

static int fiq_dbg_enable(void)
{
	uint32_t cpu_id;
	uint32_t interrupt_id;

	interrupt_id = get_uart_irq_id();
	if (interrupt_id) {
		cpu_id = plat_my_core_pos();
		fiq_enable_flag(cpu_id);
		plat_rockchip_gic_fiq_enable(interrupt_id, 0);
		return SIP_RET_SUCCESS;
	}

	return SIP_RET_DENIED;
}

uint64_t rockchip_plat_sip_handler(uint32_t smc_fid,
				   uint64_t x1,
				   uint64_t x2,
				   uint64_t x3,
				   uint64_t x4,
				   void *cookie,
				   void *handle,
				   uint64_t flags)
{
	int ret = SIP_RET_DENIED;
	struct arm_smccc_res res;

	switch (smc_fid) {

	case RK_SIP_DDR_CFG32:
		ret = ddr_smc_handler(x1, x2, x3, &res);
		SMC_RET4(handle, ret, res.a1, res.a2, res.a3);

	case RK_SIP_SUSPEND_MODE32:
		SMC_RET1(handle, suspend_mode_handler(x1, x2, x3));

	case RK_SIP_SHARE_MEM32:
		ret = share_mem_page_get_handler(x1, x2, &res);
		SMC_RET4(handle, ret, res.a1, res.a2, res.a3);

	case RK_SIP_ENABLE_FIQ:
		SMC_RET1(handle, fiq_dbg_enable());

	case RK_SIP_UARTDBG_CFG64:
		ret = fiq_debugger_smc_handler(x3, handle, x1, x2);
		SMC_RET1(handle, ret);

	default:
		ERROR("%s: unhandled SMC (0x%x)\n", __func__, smc_fid);
		SMC_RET1(handle, SMC_UNK);
	}
}
