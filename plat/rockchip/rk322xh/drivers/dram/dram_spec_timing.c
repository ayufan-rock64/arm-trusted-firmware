/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
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
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
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

#include <string.h>
#include <stdint.h>
#include <dram.h>
#include "dram_spec_timing.h"

static const uint8_t ddr3_cl_cwl[][7] = {
	/*
	 * speed 0~330 331~400 401~533 534~666 667~800 801~933 934~1066
	 * tCK >3 2.5~3 1.875~2.5 1.5~1.875 1.25~1.5 1.07~1.25 0.938~1.07
	 * cl<<4, cwl  cl<<4, cwl  cl<<4, cwl
	 */
	/* DDR3_800D (5-5-5) */
	{((5 << 4) | 5), ((5 << 4) | 5), 0, 0, 0, 0, 0},
	/* DDR3_800E (6-6-6) */
	{((5 << 4) | 5), ((6 << 4) | 5), 0, 0, 0, 0, 0},
	/* DDR3_1066E (6-6-6) */
	{((5 << 4) | 5), ((5 << 4) | 5), ((6 << 4) | 6), 0, 0, 0, 0},
	/* DDR3_1066F (7-7-7) */
	{((5 << 4) | 5), ((6 << 4) | 5), ((7 << 4) | 6), 0, 0, 0, 0},
	/* DDR3_1066G (8-8-8) */
	{((5 << 4) | 5), ((6 << 4) | 5), ((8 << 4) | 6), 0, 0, 0, 0},
	/* DDR3_1333F (7-7-7) */
	{((5 << 4) | 5), ((5 << 4) | 5), ((6 << 4) | 6), ((7 << 4) | 7),
	 0, 0, 0},
	/* DDR3_1333G (8-8-8) */
	{((5 << 4) | 5), ((5 << 4) | 5), ((7 << 4) | 6), ((8 << 4) | 7),
	 0, 0, 0},
	/* DDR3_1333H (9-9-9) */
	{((5 << 4) | 5), ((6 << 4) | 5), ((8 << 4) | 6), ((9 << 4) | 7),
	 0, 0, 0},
	/* DDR3_1333J (10-10-10) */
	{((5 << 4) | 5), ((6 << 4) | 5), ((8 << 4) | 6), ((10 << 4) | 7),
	 0, 0, 0},
	/* DDR3_1600G (8-8-8) */
	{((5 << 4) | 5), ((5 << 4) | 5), ((6 << 4) | 6), ((7 << 4) | 7),
	 ((8 << 4) | 8), 0, 0},
	/* DDR3_1600H (9-9-9) */
	{((5 << 4) | 5), ((5 << 4) | 5), ((6 << 4) | 6), ((8 << 4) | 7),
	 ((9 << 4) | 8), 0, 0},
	/* DDR3_1600J (10-10-10) */
	{((5 << 4) | 5), ((5 << 4) | 5), ((7 << 4) | 6), ((9 << 4) | 7),
	 ((10 << 4) | 8), 0, 0},
	/* DDR3_1600K (11-11-11) */
	{((5 << 4) | 5), ((6 << 4) | 5), ((8 << 4) | 6), ((10 << 4) | 7),
	 ((11 << 4) | 8), 0, 0},
	/* DDR3_1866J (10-10-10) */
	{((5 << 4) | 5), ((5 << 4) | 5), ((6 << 4) | 6), ((8 << 4) | 7),
	 ((9 << 4) | 8), ((11 << 4) | 9), 0},
	/* DDR3_1866K (11-11-11) */
	{((5 << 4) | 5), ((5 << 4) | 5), ((7 << 4) | 6), ((8 << 4) | 7),
	 ((10 << 4) | 8), ((11 << 4) | 9), 0},
	/* DDR3_1866L (12-12-12) */
	{((6 << 4) | 5), ((6 << 4) | 5), ((7 << 4) | 6), ((9 << 4) | 7),
	 ((11 << 4) | 8), ((12 << 4) | 9), 0},
	/* DDR3_1866M (13-13-13) */
	{((6 << 4) | 5), ((6 << 4) | 5), ((8 << 4) | 6), ((10 << 4) | 7),
	 ((11 << 4) | 8), ((13 << 4) | 9), 0},
	/* DDR3_2133K (11-11-11) */
	{((5 << 4) | 5), ((5 << 4) | 5), ((6 << 4) | 6), ((7 << 4) | 7),
	 ((9 << 4) | 8), ((10 << 4) | 9), ((11 << 4) | 10)},
	/* DDR3_2133L (12-12-12) */
	{((5 << 4) | 5), ((5 << 4) | 5), ((6 << 4) | 6), ((8 << 4) | 7),
	 ((9 << 4) | 8), ((11 << 4) | 9), ((12 << 4) | 10)},
	/* DDR3_2133M (13-13-13) */
	{((5 << 4) | 5), ((5 << 4) | 5), ((7 << 4) | 6), ((9 << 4) | 7),
	 ((10 << 4) | 8), ((12 << 4) | 9), ((13 << 4) | 10)},
	/* DDR3_2133N (14-14-14) */
	{((6 << 4) | 5), ((6 << 4) | 5), ((7 << 4) | 6), ((9 << 4) | 7),
	 ((11 << 4) | 8), ((13 << 4) | 9), ((14 << 4) | 10)},
	/* DDR3_DEFAULT */
	{((6 << 4) | 5), ((6 << 4) | 5), ((8 << 4) | 6), ((10 << 4) | 7),
	 ((11 << 4) | 8), ((13 << 4) | 9), ((14 << 4) | 10)}
};

static const uint16_t ddr3_trc_tfaw[] = {
	/* tRC       tFAW */
	((50 << 8) | 50),	/* DDR3_800D (5-5-5) */
	((53 << 8) | 50),	/* DDR3_800E (6-6-6) */

	((49 << 8) | 50),	/* DDR3_1066E (6-6-6) */
	((51 << 8) | 50),	/* DDR3_1066F (7-7-7) */
	((53 << 8) | 50),	/* DDR3_1066G (8-8-8) */

	((47 << 8) | 45),	/* DDR3_1333F (7-7-7) */
	((48 << 8) | 45),	/* DDR3_1333G (8-8-8) */
	((50 << 8) | 45),	/* DDR3_1333H (9-9-9) */
	((51 << 8) | 45),	/* DDR3_1333J (10-10-10) */

	((45 << 8) | 40),	/* DDR3_1600G (8-8-8) */
	((47 << 8) | 40),	/* DDR3_1600H (9-9-9) */
	((48 << 8) | 40),	/* DDR3_1600J (10-10-10) */
	((49 << 8) | 40),	/* DDR3_1600K (11-11-11) */

	((45 << 8) | 35),	/* DDR3_1866J (10-10-10) */
	((46 << 8) | 35),	/* DDR3_1866K (11-11-11) */
	((47 << 8) | 35),	/* DDR3_1866L (12-12-12) */
	((48 << 8) | 35),	/* DDR3_1866M (13-13-13) */

	((44 << 8) | 35),	/* DDR3_2133K (11-11-11) */
	((45 << 8) | 35),	/* DDR3_2133L (12-12-12) */
	((46 << 8) | 35),	/* DDR3_2133M (13-13-13) */
	((47 << 8) | 35),	/* DDR3_2133N (14-14-14) */

	((53 << 8) | 50)	/* DDR3_DEFAULT */
};

static uint32_t get_max_speed_rate(struct timing_related_config *timing_config)
{
	if (timing_config->ch_cnt > 1)
		return max(timing_config->dram_info[0].speed_rate,
					timing_config->dram_info[1].speed_rate);
	else
		return timing_config->dram_info[0].speed_rate;
}

static uint32_t
get_max_die_capability(struct timing_related_config *timing_config)
{
	uint32_t die_cap = 0;
	uint32_t cs, ch;

	for (ch = 0; ch < timing_config->ch_cnt; ch++) {
		for (cs = 0; cs < timing_config->dram_info[ch].cs_cnt; cs++) {
			die_cap = max(die_cap,
				      timing_config->
				      dram_info[ch].per_die_capability[cs]);
		}
	}
	return die_cap;
}

#define DDR3_TRSTL		(100)	/*tRSTL, 100ns*/
#define DDR3_TRSTH		(500000) /*500us*/
#define DDR3_TREFI_7_8_US	(7800)	/*7.8us*/
#define DDR3_TWR		(15)	/*tWR, 15ns*/
#define DDR3_TRTP		(7) /*tRTP, max(4 tCK,7.5ns)*/
#define DDR3_TRRD_1066		(10)	/*tRRD = max(4nCK, 10ns)*/
#define DDR3_TRRD_1600		(7)	/*tRRD = max(4nCK, 7.5ns)*/
#define DDR3_TRRD_2133		(6)	/*tRRD = max(4nCK, 6ns)*/
#define DDR3_TCCD		(4) /*tCK*/
#define DDR3_TWTR		(7) /*tWTR, max(4 tCK,7.5ns)*/
#define DDR3_TRTW		(0) /*tCK*/
#define DDR3_TRAS		(37)	/*tRAS, 37.5ns(400MHz) 37.5ns(533MHz)*/
#define DDR3_TRFC_512MBIT	(90)	/*ns*/
#define DDR3_TRFC_1GBIT		(110)	/*ns*/
#define DDR3_TRFC_2GBIT		(160)	/*ns*/
#define DDR3_TRFC_4GBIT		(260)	/*ns*/
#define DDR3_TRFC_8GBIT		(350)	/*ns*/
/* pd and sr */
#define DDR3_TXP		(7) /*tXP, max(3 tCK, 7.5ns)(<933MHz)*/
#define DDR3_TXPDLL		(24)	/*tXPDLL, max(10 tCK,24ns)*/
#define DDR3_TDLLK		(512)	/*tXSR, =tDLLK=512 tCK*/
#define DDR3_TCKE_400MHZ	(7) /*tCKE, max(3 tCK,7.5ns)(400MHz)*/
#define DDR3_TCKE_533MHZ	(6) /*tCKE, max(3 tCK,5.625ns)(533MHz)*/
#define DDR3_TCKSRE		(10)	/*tCKSRX, max(5 tCK, 10ns)*/
/* mode register timing */
#define DDR3_TMOD		(15)	/*tMOD, max(12 tCK,15ns)*/
#define DDR3_TMRD		(4) /*tMRD, 4 tCK*/
/* ZQ */
#define DDR3_TZQINIT		(640)/*tZQinit, max(512 tCK, 640ns)*/
#define DDR3_TZQCS		(80)	/*tZQCS, max(64 tCK, 80ns)*/
#define DDR3_TZQOPER		(320)	/*tZQoper, max(256 tCK, 320ns) */
/* Write leveling */
#define DDR3_TWLMRD		(40)  /*tCK*/
#define DDR3_TWLO		(9) /*max 7.5ns*/
#define DDR3_TWLDQSEN		(25) /*tCK*/

/*
 * Description: depend on input parameter "timing_config",
 *		and calculate all ddr3
 *		spec timing to "pdram_timing"
 * parameters:
 *   input: timing_config
 *   output: pdram_timing
 */
static void ddr3_get_parameter(struct timing_related_config *timing_config,
			       struct dram_timing_t *pdram_timing)
{
	uint32_t nmhz = timing_config->freq;
	uint32_t ddr_speed_bin = get_max_speed_rate(timing_config);
	uint32_t ddr_capability_per_die = get_max_die_capability(timing_config);
	uint32_t tmp;

	memset((void *)pdram_timing, 0, sizeof(struct dram_timing_t));
	pdram_timing->mhz = nmhz;
	pdram_timing->al = 0;
	pdram_timing->bl = timing_config->bl;
	if (nmhz <= 330)
		tmp = 0;
	else if (nmhz <= 400)
		tmp = 1;
	else if (nmhz <= 533)
		tmp = 2;
	else if (nmhz <= 666)
		tmp = 3;
	else if (nmhz <= 800)
		tmp = 4;
	else if (nmhz <= 933)
		tmp = 5;
	else
		tmp = 6;

	/* when dll bypss cl = cwl = 6 */
	if (nmhz < 300) {
		pdram_timing->cl = 6;
		pdram_timing->cwl = 6;
	} else {
		pdram_timing->cl = (ddr3_cl_cwl[ddr_speed_bin][tmp] >> 4) & 0xf;
		pdram_timing->cwl = ddr3_cl_cwl[ddr_speed_bin][tmp] & 0xf;
	}

	switch (timing_config->dramds) {
	case 40:
		tmp = DDR3_DS_40;
		break;
	case 34:
	default:
		tmp = DDR3_DS_34;
		break;
	}

	switch (timing_config->dramodt) {
	case 60:
		pdram_timing->mr[1] = tmp | DDR3_RTT_NOM_60;
		break;
	case 40:
		pdram_timing->mr[1] = tmp | DDR3_RTT_NOM_40;
		break;
	case 120:
		pdram_timing->mr[1] = tmp | DDR3_RTT_NOM_120;
		break;
	case 0:
	default:
		pdram_timing->mr[1] = tmp | DDR3_RTT_NOM_DIS;
		break;
	}

	pdram_timing->mr[2] = DDR3_MR2_CWL(pdram_timing->cwl);
	pdram_timing->mr[3] = 0;

	pdram_timing->trstl = ((DDR3_TRSTL * nmhz + 999) / 1000);
	pdram_timing->trsth = ((DDR3_TRSTH * nmhz + 999) / 1000);
	/* tREFI, average periodic refresh interval, 7.8us */
	pdram_timing->trefi = ((DDR3_TREFI_7_8_US * nmhz + 999) / 1000);
	/* base timing */
	pdram_timing->trcd = pdram_timing->cl;
	pdram_timing->trp = pdram_timing->cl;
	pdram_timing->trppb = pdram_timing->cl;
	tmp = ((DDR3_TWR * nmhz + 999) / 1000);
	pdram_timing->twr = tmp;
	pdram_timing->tdal = tmp + pdram_timing->trp;
	if (tmp < 9) {
		tmp = tmp - 4;
	} else {
		tmp += (tmp & 0x1) ? 1 : 0;
		tmp = tmp >> 1;
	}
	if (pdram_timing->bl == 4)
		pdram_timing->mr[0] = DDR3_BC4
				| DDR3_CL(pdram_timing->cl)
				| DDR3_WR(tmp);
	else
		pdram_timing->mr[0] = DDR3_BL8
				| DDR3_CL(pdram_timing->cl)
				| DDR3_WR(tmp);

	if (timing_config->dllbp)
		pdram_timing->mr[1] |= 0x1;
	else
		pdram_timing->mr[0] |= (1 << 8);
	tmp = ((DDR3_TRTP * nmhz + (nmhz >> 1) + 999) / 1000);
	pdram_timing->trtp = max(4, tmp);
	pdram_timing->trc =
		(((ddr3_trc_tfaw[ddr_speed_bin] >> 8) *
		nmhz + 999) / 1000);
	if (nmhz <= 533)
		tmp = ((DDR3_TRRD_1066 * nmhz + 999) / 1000);
	else if (nmhz <= 800)
		tmp = ((DDR3_TRRD_1600 * nmhz +
			(nmhz >> 1) + 999) / 1000);
	else
		tmp = ((DDR3_TRRD_2133 * nmhz + 999) / 1000);

	pdram_timing->trrd = max(4, tmp);
	pdram_timing->tccd = DDR3_TCCD;
	tmp = ((DDR3_TWTR * nmhz + (nmhz >> 1) + 999) / 1000);
	pdram_timing->twtr = max(4, tmp);
	pdram_timing->trtw = DDR3_TRTW;
	pdram_timing->tras_max = 9 * pdram_timing->trefi;
	pdram_timing->tras_min = ((DDR3_TRAS * nmhz + (nmhz >> 1) + 999)
		/ 1000);
	pdram_timing->tfaw =
		(((ddr3_trc_tfaw[ddr_speed_bin] & 0x0ff) * nmhz + 999)
						/ 1000);
	/* tRFC, 90ns(512Mb),110ns(1Gb),160ns(2Gb),300ns(4Gb),350ns(8Gb) */
	if (ddr_capability_per_die <= 0x4000000)
		tmp = DDR3_TRFC_512MBIT;
	else if (ddr_capability_per_die <= 0x8000000)
		tmp = DDR3_TRFC_1GBIT;
	else if (ddr_capability_per_die <= 0x10000000)
		tmp = DDR3_TRFC_2GBIT;
	else if (ddr_capability_per_die <= 0x20000000)
		tmp = DDR3_TRFC_4GBIT;
	else
		tmp = DDR3_TRFC_8GBIT;
	pdram_timing->trfc = (tmp * nmhz + 999) / 1000;
	pdram_timing->txsnr = max(5, (((tmp + 10) * nmhz + 999) / 1000));
	pdram_timing->tdqsck_max = 0;
	/* pd and sr */
	pdram_timing->txsr = DDR3_TDLLK;
	tmp = ((DDR3_TXP * nmhz + (nmhz >> 1) + 999) / 1000);
	pdram_timing->txp = max(3, tmp);
	tmp = ((DDR3_TXPDLL * nmhz + 999) / 1000);
	pdram_timing->txpdll = max(10, tmp);
	pdram_timing->tdllk = DDR3_TDLLK;
	if (nmhz >= 533)
		tmp = ((DDR3_TCKE_533MHZ * nmhz + 999) / 1000);
	else
		tmp = ((DDR3_TCKE_400MHZ * nmhz + (nmhz >> 1) + 999) / 1000);
	pdram_timing->tcke = max(3, tmp);
	pdram_timing->tckesr = (pdram_timing->tcke + 1);
	tmp = ((DDR3_TCKSRE * nmhz + 999) / 1000);
	pdram_timing->tcksre = max(5, tmp);
	pdram_timing->tcksrx = max(5, tmp);
	/* mode register timing */
	tmp = ((DDR3_TMOD * nmhz + 999) / 1000);
	pdram_timing->tmod = max(12, tmp);
	pdram_timing->tmrd = DDR3_TMRD;
	pdram_timing->tmrr = 0;
	/* ODT */
	pdram_timing->todton = pdram_timing->cwl - 2;
	/* ZQ */
	tmp = ((DDR3_TZQINIT * nmhz + 999) / 1000);
	pdram_timing->tzqinit = max(512, tmp);
	tmp = ((DDR3_TZQCS * nmhz + 999) / 1000);
	pdram_timing->tzqcs = max(64, tmp);
	tmp = ((DDR3_TZQOPER * nmhz + 999) / 1000);
	pdram_timing->tzqoper = max(256, tmp);
	/* write leveling */
	pdram_timing->twlmrd = DDR3_TWLMRD;
	pdram_timing->twldqsen = DDR3_TWLDQSEN;
	pdram_timing->twlo = ((DDR3_TWLO * nmhz + (nmhz >> 1) + 999) / 1000);
}

#define LPDDR2_TINIT1		(100) /*ns*/
#define LPDDR2_TINIT2		(5) /*tCK*/
#define LPDDR2_TINIT3		(200000) /*200us*/
#define LPDDR2_TINIT4		(1000) /*1us*/
#define LPDDR2_TINIT5		(10000) /*10us*/
#define LPDDR2_TRSTL		(0)/*tCK*/
#define LPDDR2_TRSTH		(500000) /*500us*/
#define LPDDR2_TREFI_3_9_US	(3900)	/*3.9us*/
#define LPDDR2_TREFI_7_8_US	(7800)	/* 7.8us*/
/* base timing */
#define LPDDR2_TRCD		(24)/*tRCD,15ns(Fast)18ns(Typ)24ns(Slow)*/
#define LPDDR2_TRP_PB		(18)/*tRPpb,15ns(Fast)18ns(Typ)24ns(Slow)*/
#define LPDDR2_TRP_AB_8_BANK	(21)/*tRPab,18ns(Fast)21ns(Typ)27ns(Slow)*/
#define LPDDR2_TWR		(15)/*tWR, max(3tCK,15ns)*/
#define LPDDR2_TRTP		(7) /*tRTP, max(2tCK, 7.5ns)*/
#define LPDDR2_TRRD		(10)/*tRRD, max(2tCK,10ns)*/
#define LPDDR2_TCCD		(2) /*tCK*/
#define LPDDR2_TWTR_GREAT_200MHZ	(7) /*ns*/
#define LPDDR2_TWTR_LITTLE_200MHZ	(10)	/*ns*/
#define LPDDR2_TRTW		(0) /*tCK*/
#define LPDDR2_TRAS_MAX		(70000)/*70us*/
#define LPDDR2_TRAS		(42)	/*tRAS, max(3tCK,42ns)*/
#define LPDDR2_TFAW_GREAT_200MHZ	(50)	/*max(8tCK,50ns)*/
#define LPDDR2_TFAW_LITTLE_200MHZ	(60)	/*max(8tCK,60ns)*/
#define LPDDR2_TRFC_8GBIT	(210)	/*ns*/
#define LPDDR2_TRFC_4GBIT	(130)	/*ns*/
#define LPDDR2_TDQSCK_MIN	(2) /*tDQSCKmin,2.5ns*/
#define LPDDR2_TDQSCK_MAX	(5) /*tDQSCKmax,5.5ns*/
/* pd and sr */
#define LPDDR2_TXP		(7) /*tXP, max(2tCK,7.5ns)*/
#define LPDDR2_TXPDLL		(0)
#define LPDDR2_TDLLK		(0) /*tCK*/
#define LPDDR2_TCKE		(3) /*tCK*/
#define LPDDR2_TCKESR		(15)	/*tCKESR, max(3tCK,15ns)*/
#define LPDDR2_TCKSRE		(1) /*tCK*/
#define LPDDR2_TCKSRX		(2) /*tCK*/
/* mode register timing */
#define LPDDR2_TMOD		(0)
#define LPDDR2_TMRD		(5) /*tMRD, (=tMRW), 5 tCK*/
#define LPDDR2_TMRR		(2) /*tCK*/
/* ZQ */
#define LPDDR2_TZQINIT		(1000)/*ns*/
#define LPDDR2_TZQCS		(90)	/*tZQCS, max(6tCK,90ns)*/
#define LPDDR2_TZQCL		(360)	/*tZQCL, max(6tCK,360ns)*/
#define LPDDR2_TZQRESET		(50) /*ZQreset, max(3tCK,50ns)*/

/*
 * Description: depend on input parameter "timing_config",
 *		and calculate all lpddr2
 *		spec timing to "pdram_timing"
 * parameters:
 *   input: timing_config
 *   output: pdram_timing
 */
static void lpddr2_get_parameter(struct timing_related_config *timing_config,
				 struct dram_timing_t *pdram_timing)
{
	uint32_t nmhz = timing_config->freq;
	uint32_t ddr_capability_per_die = get_max_die_capability(timing_config);
	uint32_t tmp, trp_tmp, trppb_tmp, tras_tmp, twr_tmp, bl_tmp;

	memset((void *)pdram_timing, 0, sizeof(struct dram_timing_t));
	pdram_timing->mhz = nmhz;
	pdram_timing->al = 0;
	pdram_timing->bl = timing_config->bl;

	/*	   1066 933 800 667 533 400 333
	 * RL,	 8	 7	 6	 5	 4	 3	 3
	 * WL,	 4	 4	 3	 2	 2	 1	 1
	 */
	/*
	if (nmhz <= 200) {
		pdram_timing->cl = 3;
		pdram_timing->cwl = 1;
		pdram_timing->mr[2] = LPDDR2_RL3_WL1;
	} else */
	if (nmhz <= 266) {
		pdram_timing->cl = 4;
		pdram_timing->cwl = 2;
		pdram_timing->mr[2] = LPDDR2_RL4_WL2;
	} else if (nmhz <= 333) {
		pdram_timing->cl = 5;
		pdram_timing->cwl = 2;
		pdram_timing->mr[2] = LPDDR2_RL5_WL2;
	} else if (nmhz <= 400) {
		pdram_timing->cl = 6;
		pdram_timing->cwl = 3;
		pdram_timing->mr[2] = LPDDR2_RL6_WL3;
	} else if (nmhz <= 466) {
		pdram_timing->cl = 7;
		pdram_timing->cwl = 4;
		pdram_timing->mr[2] = LPDDR2_RL7_WL4;
	} else {
		pdram_timing->cl = 8;
		pdram_timing->cwl = 4;
		pdram_timing->mr[2] = LPDDR2_RL8_WL4;
	}
	switch (timing_config->dramds) {
	case 120:
		pdram_timing->mr[3] = LPDDR2_DS_120;
		break;
	case 80:
		pdram_timing->mr[3] = LPDDR2_DS_80;
		break;
	case 60:
		pdram_timing->mr[3] = LPDDR2_DS_60;
		break;
	case 48:
		pdram_timing->mr[3] = LPDDR2_DS_48;
		break;
	case 40:
		pdram_timing->mr[3] = LPDDR2_DS_40;
		break;
	case 34:
	default:
		pdram_timing->mr[3] = LPDDR2_DS_34;
		break;
	}
	pdram_timing->mr[0] = 0;

	pdram_timing->tinit1 = (LPDDR2_TINIT1 * nmhz + 999) / 1000;
	pdram_timing->tinit2 = LPDDR2_TINIT2;
	pdram_timing->tinit3 = (LPDDR2_TINIT3 * nmhz + 999) / 1000;
	pdram_timing->tinit4 = (LPDDR2_TINIT4 * nmhz + 999) / 1000;
	pdram_timing->tinit5 = (LPDDR2_TINIT5 * nmhz + 999) / 1000;
	pdram_timing->trstl = LPDDR2_TRSTL;
	pdram_timing->trsth = (LPDDR2_TRSTH * nmhz + 999) / 1000;
	/*
	 * tREFI, average periodic refresh interval,
	 * 15.6us(<256Mb) 7.8us(256Mb-1Gb) 3.9us(2Gb-8Gb)
	 */
	if (ddr_capability_per_die >= 0x10000000)
		pdram_timing->trefi = (LPDDR2_TREFI_3_9_US * nmhz + 999)
							/ 1000;
	else
		pdram_timing->trefi = (LPDDR2_TREFI_7_8_US * nmhz + 999)
							/ 1000;
	/* base timing */
	tmp = ((LPDDR2_TRCD * nmhz + 999) / 1000);
	pdram_timing->trcd = max(3, tmp);
	/*
	 * tRPpb, max(3tCK, 15ns(Fast) 18ns(Typ) 24ns(Slow),
	 */
	trppb_tmp = ((LPDDR2_TRP_PB * nmhz + 999) / 1000);
	trppb_tmp = max(3, trppb_tmp);
	pdram_timing->trppb = trppb_tmp;
	/*
	 * tRPab, max(3tCK, 4-bank:15ns(Fast) 18ns(Typ) 24ns(Slow),
	 *	8-bank:18ns(Fast) 21ns(Typ) 27ns(Slow))
	 */
	trp_tmp = ((LPDDR2_TRP_AB_8_BANK * nmhz + 999) / 1000);
	trp_tmp = max(3, trp_tmp);
	pdram_timing->trp = trp_tmp;
	twr_tmp = ((LPDDR2_TWR * nmhz + 999) / 1000);
	twr_tmp = max(3, twr_tmp);
	pdram_timing->twr = twr_tmp;
	bl_tmp = (pdram_timing->bl == 16) ? LPDDR2_BL16 :
			((pdram_timing->bl == 8) ? LPDDR2_BL8 : LPDDR2_BL4);
	pdram_timing->mr[1] = bl_tmp | LPDDR2_N_WR(twr_tmp);
	tmp = ((LPDDR2_TRTP * nmhz + (nmhz >> 1) + 999) / 1000);
	pdram_timing->trtp = max(2, tmp);
	tras_tmp = ((LPDDR2_TRAS * nmhz + 999) / 1000);
	tras_tmp = max(3, tras_tmp);
	pdram_timing->tras_min = tras_tmp;
	pdram_timing->tras_max = ((LPDDR2_TRAS_MAX * nmhz + 999) / 1000);
	pdram_timing->trc = (tras_tmp + trp_tmp);
	tmp = ((LPDDR2_TRRD * nmhz + 999) / 1000);
	pdram_timing->trrd = max(2, tmp);
	pdram_timing->tccd = LPDDR2_TCCD;
	/* tWTR, max(2tCK, 7.5ns(533-266MHz)  10ns(200-166MHz)) */
	if (nmhz > 200)
		tmp = ((LPDDR2_TWTR_GREAT_200MHZ * nmhz + (nmhz >> 1) +
			  999) / 1000);
	else
		tmp = ((LPDDR2_TWTR_LITTLE_200MHZ * nmhz + 999) / 1000);
	pdram_timing->twtr = max(2, tmp);
	pdram_timing->trtw = LPDDR2_TRTW;
	if (nmhz <= 200)
		pdram_timing->tfaw = (LPDDR2_TFAW_LITTLE_200MHZ * nmhz + 999)
							/ 1000;
	else
		pdram_timing->tfaw = (LPDDR2_TFAW_GREAT_200MHZ * nmhz + 999)
							/ 1000;
	/* tRFC, 90ns(<=512Mb) 130ns(1Gb-4Gb) 210ns(8Gb) */
	if (ddr_capability_per_die >= 0x40000000) {
		pdram_timing->trfc =
			(LPDDR2_TRFC_8GBIT * nmhz + 999) / 1000;
		tmp = (((LPDDR2_TRFC_8GBIT + 10) * nmhz + 999) / 1000);
	} else {
		pdram_timing->trfc =
			(LPDDR2_TRFC_4GBIT * nmhz + 999) / 1000;
		tmp = (((LPDDR2_TRFC_4GBIT + 10) * nmhz + 999) / 1000);
	}
	if (tmp < 2)
		tmp = 2;
	pdram_timing->txsr = tmp;
	pdram_timing->txsnr = tmp;
	/* tdqsck use rounded down */
	pdram_timing->tdqsck = ((LPDDR2_TDQSCK_MIN * nmhz + (nmhz >> 1))
					/ 1000);
	pdram_timing->tdqsck_max =
			((LPDDR2_TDQSCK_MAX * nmhz + (nmhz >> 1) + 999)
					/ 1000);
	/* pd and sr */
	tmp = ((LPDDR2_TXP * nmhz + (nmhz >> 1) + 999) / 1000);
	pdram_timing->txp = max(2, tmp);
	pdram_timing->txpdll = LPDDR2_TXPDLL;
	pdram_timing->tdllk = LPDDR2_TDLLK;
	pdram_timing->tcke = LPDDR2_TCKE;
	tmp = ((LPDDR2_TCKESR * nmhz + 999) / 1000);
	pdram_timing->tckesr = max(3, tmp);
	pdram_timing->tcksre = LPDDR2_TCKSRE;
	pdram_timing->tcksrx = LPDDR2_TCKSRX;
	/* mode register timing */
	pdram_timing->tmod = LPDDR2_TMOD;
	pdram_timing->tmrd = LPDDR2_TMRD;
	pdram_timing->tmrr = LPDDR2_TMRR;
	/* ZQ */
	pdram_timing->tzqinit = (LPDDR2_TZQINIT * nmhz + 999) / 1000;
	tmp = ((LPDDR2_TZQCS * nmhz + 999) / 1000);
	pdram_timing->tzqcs = max(6, tmp);
	tmp = ((LPDDR2_TZQCL * nmhz + 999) / 1000);
	pdram_timing->tzqoper = max(6, tmp);
	tmp = ((LPDDR2_TZQRESET * nmhz + 999) / 1000);
	pdram_timing->tzqreset = max(3, tmp);
}

#define LPDDR3_TINIT1		(100) /*ns*/
#define LPDDR3_TINIT2		(5) /*tCK*/
#define LPDDR3_TINIT3		(200000) /*200us*/
#define LPDDR3_TINIT4		(1000) /*1us*/
#define LPDDR3_TINIT5		(10000) /*10us*/
#define LPDDR3_TRSTL		(0)
#define LPDDR3_TRSTH		(0) /*500us*/
#define LPDDR3_TREFI_3_9_US	(3900) /*3.9us*/
/* base timging */
#define LPDDR3_TRCD	(18)	/*tRCD,15ns(Fast)18ns(Typ)24ns(Slow)*/
#define LPDDR3_TRP_PB	(18)	/*tRPpb, 15ns(Fast) 18ns(Typ) 24ns(Slow)*/
#define LPDDR3_TRP_AB	(21)	/*tRPab, 18ns(Fast) 21ns(Typ) 27ns(Slow)*/
#define LPDDR3_TWR	(15)	/*tWR, max(4tCK,15ns)*/
#define LPDDR3_TRTP	(7) /*tRTP, max(4tCK, 7.5ns)*/
#define LPDDR3_TRRD	(10)	/*tRRD, max(2tCK,10ns)*/
#define LPDDR3_TCCD	(4) /*tCK*/
#define LPDDR3_TWTR	(7) /*tWTR, max(4tCK, 7.5ns)*/
#define LPDDR3_TRTW	(0) /*tCK register min valid value*/
#define LPDDR3_TRAS_MAX	(70000)/*70us*/
#define LPDDR3_TRAS	(42)	/*tRAS, max(3tCK,42ns)*/
#define LPDDR3_TFAW	(50)	/*tFAW,max(8tCK, 50ns)*/
#define LPDDR3_TRFC_8GBIT	(210)	/*tRFC, 130ns(4Gb) 210ns(>4Gb)*/
#define LPDDR3_TRFC_4GBIT	(130)	/*ns*/
#define LPDDR3_TDQSCK_MIN	(2) /*tDQSCKmin,2.5ns*/
#define LPDDR3_TDQSCK_MAX	(5) /*tDQSCKmax,5.5ns*/
/* pd and sr */
#define LPDDR3_TXP	(7) /*tXP, max(3tCK,7.5ns)*/
#define LPDDR3_TXPDLL	(0)
#define LPDDR3_TCKE	(7) /*tCKE, (max 7.5ns,3 tCK)*/
#define LPDDR3_TCKESR	(15)	/*tCKESR, max(3tCK,15ns)*/
#define LPDDR3_TCKSRE	(2) /*tCKSRE=tCPDED, 2 tCK*/
#define LPDDR3_TCKSRX	(2) /*tCKSRX, 2 tCK*/
/* mode register timing */
#define LPDDR3_TMOD	(0)
#define LPDDR3_TMRD	(14) /*tMRD, (=tMRW), max(14ns, 10 tCK)*/
#define LPDDR3_TMRR	(4) /*tMRR, 4 tCK*/
#define LPDDR3_TMRRI	LPDDR3_TRCD
/* ODT */
#define LPDDR3_TODTON	(3) /*3.5ns*/
/* ZQ */
#define LPDDR3_TZQINIT	(1000)/*1us*/
#define LPDDR3_TZQCS	(90)	/*tZQCS, 90ns*/
#define LPDDR3_TZQCL	(360) /*360ns*/
#define LPDDR3_TZQRESET	(50) /*ZQreset, max(3tCK,50ns)*/
/* write leveling */
#define LPDDR3_TWLMRD	(40)  /*ns*/
#define LPDDR3_TWLO	(20) /*ns*/
#define LPDDR3_TWLDQSEN	(25) /*ns*/
/* CA training */
#define LPDDR3_TCACKEL	(10) /*tCK*/
#define LPDDR3_TCAENT	(10) /*tCK*/
#define LPDDR3_TCAMRD	(20) /*tCK*/
#define LPDDR3_TCACKEH	(10) /*tCK*/
#define LPDDR3_TCAEXT	(10) /*tCK*/
#define LPDDR3_TADR	(20) /*ns*/
#define LPDDR3_TMRZ	(3) /*ns*/

/*
 * Description: depend on input parameter "timing_config",
 *		and calculate all lpddr3
 *		spec timing to "pdram_timing"
 * parameters:
 *   input: timing_config
 *   output: pdram_timing
 */
static void lpddr3_get_parameter(struct timing_related_config *timing_config,
				 struct dram_timing_t *pdram_timing)
{
	uint32_t nmhz = timing_config->freq;
	uint32_t ddr_capability_per_die = get_max_die_capability(timing_config);
	uint32_t tmp, trp_tmp, trppb_tmp, tras_tmp, twr_tmp, bl_tmp;

	memset((void *)pdram_timing, 0, sizeof(struct dram_timing_t));
	pdram_timing->mhz = nmhz;
	pdram_timing->al = 0;
	pdram_timing->bl = timing_config->bl;

	/*
	 * Only support Write Latency Set A here
	 *     1066 933 800 733 667 600 533 400 166
	 * RL, 16   14  12  11  10  9   8   6   3
	 * WL, 8    8   6   6   6   5   4   3   1
	 */
	if (nmhz <= 400) {
		pdram_timing->cl = 6;
		pdram_timing->cwl = 3;
		pdram_timing->mr[2] = LPDDR3_RL6_WL3;
	} else if (nmhz <= 533) {
		pdram_timing->cl = 8;
		pdram_timing->cwl = 4;
		pdram_timing->mr[2] = LPDDR3_RL8_WL4;
	} else if (nmhz <= 600) {
		pdram_timing->cl = 9;
		pdram_timing->cwl = 5;
		pdram_timing->mr[2] = LPDDR3_RL9_WL5;
	} else if (nmhz <= 667) {
		pdram_timing->cl = 10;
		pdram_timing->cwl = 6;
		pdram_timing->mr[2] = LPDDR3_RL10_WL6;
	} else if (nmhz <= 733) {
		pdram_timing->cl = 11;
		pdram_timing->cwl = 6;
		pdram_timing->mr[2] = LPDDR3_RL11_WL6;
	} else if (nmhz <= 800) {
		pdram_timing->cl = 12;
		pdram_timing->cwl = 6;
		pdram_timing->mr[2] = LPDDR3_RL12_WL6;
	} else if (nmhz <= 933) {
		pdram_timing->cl = 14;
		pdram_timing->cwl = 8;
		pdram_timing->mr[2] = LPDDR3_RL14_WL8;
	} else {
		pdram_timing->cl = 16;
		pdram_timing->cwl = 8;
		pdram_timing->mr[2] = LPDDR3_RL16_WL8;
	}
	switch (timing_config->dramds) {
	case 80:
		pdram_timing->mr[3] = LPDDR3_DS_80;
		break;
	case 60:
		pdram_timing->mr[3] = LPDDR3_DS_60;
		break;
	case 48:
		pdram_timing->mr[3] = LPDDR3_DS_48;
		break;
	case 40:
		pdram_timing->mr[3] = LPDDR3_DS_40;
		break;
	case 34:
	default:
		pdram_timing->mr[3] = LPDDR3_DS_34;
		break;
	}
	pdram_timing->mr[0] = 0;
	switch (timing_config->dramodt) {
	case 60:
		pdram_timing->mr11 = LPDDR3_ODT_60;
		break;
	case 120:
		pdram_timing->mr11 = LPDDR3_ODT_120;
		break;
	case 240:
		pdram_timing->mr11 = LPDDR3_ODT_240;
		break;
	case 0:
	default:
		pdram_timing->mr11 = LPDDR3_ODT_DIS;
		break;
	}

	pdram_timing->tinit1 = (LPDDR3_TINIT1 * nmhz + 999) / 1000;
	pdram_timing->tinit2 = LPDDR3_TINIT2;
	pdram_timing->tinit3 = (LPDDR3_TINIT3 * nmhz + 999) / 1000;
	pdram_timing->tinit4 = (LPDDR3_TINIT4 * nmhz + 999) / 1000;
	pdram_timing->tinit5 = (LPDDR3_TINIT5 * nmhz + 999) / 1000;
	pdram_timing->trstl = LPDDR3_TRSTL;
	pdram_timing->trsth = (LPDDR3_TRSTH * nmhz + 999) / 1000;
	/* tREFI, average periodic refresh interval, 3.9us(4Gb-16Gb) */
	pdram_timing->trefi = (LPDDR3_TREFI_3_9_US * nmhz + 999) / 1000;
	/* base timing */
	tmp = ((LPDDR3_TRCD * nmhz + 999) / 1000);
	pdram_timing->trcd = max(3, tmp);
	trppb_tmp = ((LPDDR3_TRP_PB * nmhz + 999) / 1000);
	trppb_tmp = max(3, trppb_tmp);
	pdram_timing->trppb = trppb_tmp;
	trp_tmp = ((LPDDR3_TRP_AB * nmhz + 999) / 1000);
	trp_tmp = max(3, trp_tmp);
	pdram_timing->trp = trp_tmp;
	twr_tmp = ((LPDDR3_TWR * nmhz + 999) / 1000);
	twr_tmp = max(4, twr_tmp);
	pdram_timing->twr = twr_tmp;
	if (twr_tmp <= 6)
		twr_tmp = 6;
	else if (twr_tmp <= 8)
		twr_tmp = 8;
	else if (twr_tmp <= 12)
		twr_tmp = twr_tmp;
	else if (twr_tmp <= 14)
		twr_tmp = 14;
	else
		twr_tmp = 16;
	if (twr_tmp > 9)
		pdram_timing->mr[2] |= (1 << 4); /*enable nWR > 9*/
	twr_tmp = (twr_tmp > 9) ? (twr_tmp - 10) : (twr_tmp - 2);
	bl_tmp = LPDDR3_BL8;
	pdram_timing->mr[1] = bl_tmp | LPDDR3_N_WR(twr_tmp);
	tmp = ((LPDDR3_TRTP * nmhz + (nmhz >> 1) + 999) / 1000);
	pdram_timing->trtp = max(4, tmp);
	tras_tmp = ((LPDDR3_TRAS * nmhz + 999) / 1000);
	tras_tmp = max(3, tras_tmp);
	pdram_timing->tras_min = tras_tmp;
	pdram_timing->trc = (tras_tmp + trp_tmp);
	tmp = ((LPDDR3_TRRD * nmhz + 999) / 1000);
	pdram_timing->trrd = max(2, tmp);
	pdram_timing->tccd = LPDDR3_TCCD;
	tmp = ((LPDDR3_TWTR * nmhz + (nmhz >> 1) + 999) / 1000);
	pdram_timing->twtr = max(4, tmp);
	pdram_timing->trtw =  ((LPDDR3_TRTW * nmhz + 999) / 1000);
	pdram_timing->tras_max = ((LPDDR3_TRAS_MAX * nmhz + 999) / 1000);
	tmp = (LPDDR3_TFAW * nmhz + 999) / 1000;
	pdram_timing->tfaw = max(8, tmp);
	if (ddr_capability_per_die > 0x20000000) {
		pdram_timing->trfc =
			(LPDDR3_TRFC_8GBIT * nmhz + 999) / 1000;
		tmp = (((LPDDR3_TRFC_8GBIT + 10) * nmhz + 999) / 1000);
	} else {
		pdram_timing->trfc =
			(LPDDR3_TRFC_4GBIT * nmhz + 999) / 1000;
		tmp = (((LPDDR3_TRFC_4GBIT + 10) * nmhz + 999) / 1000);
	}
	pdram_timing->txsr = max(2, tmp);
	pdram_timing->txsnr = max(2, tmp);
	/* tdqsck use rounded down */
	pdram_timing->tdqsck =
			((LPDDR3_TDQSCK_MIN * nmhz + (nmhz >> 1))
					/ 1000);
	pdram_timing->tdqsck_max =
			((LPDDR3_TDQSCK_MAX * nmhz + (nmhz >> 1) + 999)
					/ 1000);
	/* pd and sr */
	tmp = ((LPDDR3_TXP * nmhz + (nmhz >> 1) + 999) / 1000);
	pdram_timing->txp = max(3, tmp);
	pdram_timing->txpdll = LPDDR3_TXPDLL;
	tmp = ((LPDDR3_TCKE * nmhz + (nmhz >> 1) + 999) / 1000);
	pdram_timing->tcke = max(3, tmp);
	tmp = ((LPDDR3_TCKESR * nmhz + 999) / 1000);
	pdram_timing->tckesr = max(3, tmp);
	pdram_timing->tcksre = LPDDR3_TCKSRE;
	pdram_timing->tcksrx = LPDDR3_TCKSRX;
	/* mode register timing */
	pdram_timing->tmod = LPDDR3_TMOD;
	tmp = ((LPDDR3_TMRD * nmhz + 999) / 1000);
	pdram_timing->tmrd = max(10, tmp);
	pdram_timing->tmrr = LPDDR3_TMRR;
	tmp = ((LPDDR3_TRCD * nmhz + 999) / 1000);
	pdram_timing->tmrri = max(3, tmp);
	/* ODT */
	pdram_timing->todton = (LPDDR3_TODTON * nmhz + (nmhz >> 1) + 999)
				/ 1000;
	/* ZQ */
	pdram_timing->tzqinit = (LPDDR3_TZQINIT * nmhz + 999) / 1000;
	pdram_timing->tzqcs =
		((LPDDR3_TZQCS * nmhz + 999) / 1000);
	pdram_timing->tzqoper =
		((LPDDR3_TZQCL * nmhz + 999) / 1000);
	tmp = ((LPDDR3_TZQRESET * nmhz + 999) / 1000);
	pdram_timing->tzqreset = max(3, tmp);
	/* write leveling */
	pdram_timing->twlmrd = (LPDDR3_TWLMRD * nmhz + 999) / 1000;
	pdram_timing->twlo = (LPDDR3_TWLO * nmhz + 999) / 1000;
	pdram_timing->twldqsen = (LPDDR3_TWLDQSEN * nmhz + 999) / 1000;
	/* CA training */
	pdram_timing->tcackel = LPDDR3_TCACKEL;
	pdram_timing->tcaent = LPDDR3_TCAENT;
	pdram_timing->tcamrd = LPDDR3_TCAMRD;
	pdram_timing->tcackeh = LPDDR3_TCACKEH;
	pdram_timing->tcaext = LPDDR3_TCAEXT;
	pdram_timing->tadr = (LPDDR3_TADR * nmhz + 999) / 1000;
	pdram_timing->tmrz = (LPDDR3_TMRZ * nmhz + 999) / 1000;
	pdram_timing->tcacd = pdram_timing->tadr + 2;
}

#define LPDDR4_TINIT1	(200000) /*200us*/
#define LPDDR4_TINIT2	(10) /*10ns*/
#define LPDDR4_TINIT3	(2000000) /*2ms*/
#define LPDDR4_TINIT4	(5) /*tCK*/
#define LPDDR4_TINIT5	(2000) /*2us*/
#define LPDDR4_TRSTL		LPDDR4_TINIT1
#define LPDDR4_TRSTH		LPDDR4_TINIT3
#define LPDDR4_TREFI_3_9_US	(3900) /*3.9us*/
/* base timging */
#define LPDDR4_TRCD	(18)	/*tRCD,max(18ns,4tCK)*/
#define LPDDR4_TRP_PB	(18)	/*tRPpb, max(18ns, 4tCK)*/
#define LPDDR4_TRP_AB	(21)	/*tRPab, max(21ns, 4tCK)*/
#define LPDDR4_TRRD	(10)	/*tRRD, max(4tCK,10ns)*/
#define LPDDR4_TCCD_BL16	(8) /*tCK*/
#define LPDDR4_TCCD_BL32	(16) /*tCK*/
#define LPDDR4_TWTR	(10) /*tWTR, max(8tCK, 10ns)*/
#define LPDDR4_TRTW	(0) /*tCK register min valid value*/
#define LPDDR4_TRAS_MAX (70000)/*70us*/
#define LPDDR4_TRAS	(42)	/*tRAS, max(3tCK,42ns)*/
#define LPDDR4_TFAW	(40)	/*tFAW,min 40ns)*/
#define LPDDR4_TRFC_12GBIT	(280)	/*tRFC, 280ns(>=12Gb)*/
#define LPDDR4_TRFC_6GBIT	(180)	/* 6Gb/8Gb 180ns*/
#define LPDDR4_TRFC_4GBIT	(130)	/* 4Gb 130ns*/
#define LPDDR4_TDQSCK_MIN	(1) /*tDQSCKmin,1.5ns*/
#define LPDDR4_TDQSCK_MAX	(3) /*tDQSCKmax,3.5ns*/
#define LPDDR4_TPPD		(4) /*tCK*/
/* pd and sr */
#define LPDDR4_TXP	(7) /*tXP, max(5tCK,7.5ns)*/
#define LPDDR4_TCKE	(7) /*tCKE, max(7.5ns,4 tCK)*/
#define LPDDR4_TESCKE	(1) /*tESCKE, max(1.75ns, 3tCK)*/
#define LPDDR4_TSR	(15) /*tSR, max(15ns, 3tCK)*/
#define LPDDR4_TCMDCKE	(1) /*max(1.75ns, 3tCK)*/
#define LPDDR4_TCSCKE	(1) /* 1.75ns*/
#define LPDDR4_TCKELCS	(5) /*max(5ns, 5tCK)*/
#define LPDDR4_TCSCKEH	(1) /* 1.75ns*/
#define LPDDR4_TCKEHCS	(7) /*max(7.5ns, 5tCK)*/
#define LPDDR4_TMRWCKEL	(14) /*max(14ns, 10tCK)*/
#define LPDDR4_TCKELCMD	(7) /*max(7.5ns, 3tCK)*/
#define LPDDR4_TCKEHCMD	(7) /*max(7.5ns, 3tCK)*/
#define LPDDR4_TCKELPD	(7) /*max(7.5ns, 3tCK)*/
#define LPDDR4_TCKCKEL	(7) /*max(7.5ns, 3tCK)*/
/* mode register timing */
#define LPDDR4_TMRD	(14) /*tMRD, (=tMRW), max(14ns, 10 tCK)*/
#define LPDDR4_TMRR	(8) /*tMRR, 8 tCK*/
/* ODT */
#define LPDDR4_TODTON	(3) /*3.5ns*/
/* ZQ */
#define LPDDR4_TZQCAL	(1000)/*1us*/
#define LPDDR4_TZQLAT	(30)	/*tZQLAT, max(30ns,8tCK)*/
#define LPDDR4_TZQRESET (50) /*ZQreset, max(3tCK,50ns)*/
#define LPDDR4_TZQCKE	(1)  /*tZQCKE, max(1.75ns, 3tCK)*/
/* write leveling */
#define LPDDR4_TWLMRD	(40)  /*tCK*/
#define LPDDR4_TWLO	(20) /*ns*/
#define LPDDR4_TWLDQSEN (20) /*tCK*/
/* CA training */
#define LPDDR4_TCAENT	(250) /*ns*/
#define LPDDR4_TADR	(20) /*ns*/
#define LPDDR4_TMRZ	(1) /* 1.5ns*/
#define LPDDR4_TVREF_LONG	(250) /*ns*/
#define LPDDR4_TVREF_SHORT	(100) /*ns*/
/* VRCG */
#define LPDDR4_TVRCG_ENABLE	(200) /*ns*/
#define LPDDR4_TVRCG_DISABLE	(100) /*ns*/
/* FSP */
#define LPDDR4_TFC_LONG		(250) /*ns*/
#define LPDDR4_TCKFSPE		(7) /*max(7.5ns, 4tCK)*/
#define LPDDR4_TCKFSPX		(7) /*max(7.5ns, 4tCK)*/

/*
 * Description: depend on input parameter "timing_config",
 *              and calculate all lpddr4
 *              spec timing to "pdram_timing"
 * parameters:
 *   input: timing_config
 *   output: pdram_timing
 */
static void lpddr4_get_parameter(struct timing_related_config *timing_config,
				 struct dram_timing_t *pdram_timing)
{
	uint32_t nmhz = timing_config->freq;
	uint32_t ddr_capability_per_die = get_max_die_capability(timing_config);
	uint32_t tmp, trp_tmp, trppb_tmp, tras_tmp;

	memset((void *)pdram_timing, 0, sizeof(struct dram_timing_t));
	pdram_timing->mhz = nmhz;
	pdram_timing->al = 0;
	pdram_timing->bl = timing_config->bl;

	/*
	 * Only support Write Latency Set A here
	 *      2133 1866 1600 1333 1066 800 533 266
	 *  RL, 36   32   28   24   20   14  10  6
	 *  WL, 18   16   14   12   10   8   6   4
	 * nWR, 40   34   30   24   20   16  10  6
	 * nRTP,16   14   12   10   8    8   8   8
	 */
	tmp = (timing_config->bl == 32) ? 1 : 0;

	/*
	 * we always use WR preamble = 2tCK
	 * RD preamble = Static
	 */
	tmp |= (1 << 2);
	if (nmhz <= 266) {
		pdram_timing->cl = 6;
		pdram_timing->cwl = 4;
		pdram_timing->twr = 6;
		pdram_timing->trtp = 8;
		pdram_timing->mr[2] = LPDDR4_RL6_NRTP8 | LPDDR4_A_WL4;
	} else if (nmhz <= 533) {
		if (timing_config->rdbi) {
			pdram_timing->cl = 12;
			pdram_timing->mr[2] = LPDDR4_RL12_NRTP8 | LPDDR4_A_WL6;
		} else {
			pdram_timing->cl = 10;
			pdram_timing->mr[2] = LPDDR4_RL10_NRTP8 | LPDDR4_A_WL6;
		}
		pdram_timing->cwl = 6;
		pdram_timing->twr = 10;
		pdram_timing->trtp = 8;
		tmp |= (1 << 4);
	} else if (nmhz <= 800) {
		if (timing_config->rdbi) {
			pdram_timing->cl = 16;
			pdram_timing->mr[2] = LPDDR4_RL16_NRTP8 | LPDDR4_A_WL8;
		} else {
			pdram_timing->cl = 14;
			pdram_timing->mr[2] = LPDDR4_RL14_NRTP8 | LPDDR4_A_WL8;
		}
		pdram_timing->cwl = 8;
		pdram_timing->twr = 16;
		pdram_timing->trtp = 8;
		tmp |= (2 << 4);
	} else if (nmhz <= 1066) {
		if (timing_config->rdbi) {
			pdram_timing->cl = 22;
			pdram_timing->mr[2] = LPDDR4_RL22_NRTP8 | LPDDR4_A_WL10;
		} else {
			pdram_timing->cl = 20;
			pdram_timing->mr[2] = LPDDR4_RL20_NRTP8 | LPDDR4_A_WL10;
		}
		pdram_timing->cwl = 10;
		pdram_timing->twr = 20;
		pdram_timing->trtp = 8;
		tmp |= (3 << 4);
	} else if (nmhz <= 1333) {
		if (timing_config->rdbi) {
			pdram_timing->cl = 28;
			pdram_timing->mr[2] = LPDDR4_RL28_NRTP10 |
						LPDDR4_A_WL12;
		} else {
			pdram_timing->cl = 24;
			pdram_timing->mr[2] = LPDDR4_RL24_NRTP10 |
						LPDDR4_A_WL12;
		}
		pdram_timing->cwl = 12;
		pdram_timing->twr = 24;
		pdram_timing->trtp = 10;
		tmp |= (4 << 4);
	} else if (nmhz <= 1600) {
		if (timing_config->rdbi) {
			pdram_timing->cl = 32;
			pdram_timing->mr[2] = LPDDR4_RL32_NRTP12 |
						LPDDR4_A_WL14;
		} else {
			pdram_timing->cl = 28;
			pdram_timing->mr[2] = LPDDR4_RL28_NRTP12 |
						LPDDR4_A_WL14;
		}
		pdram_timing->cwl = 14;
		pdram_timing->twr = 30;
		pdram_timing->trtp = 12;
		tmp |= (5 << 4);
	} else if (nmhz <= 1866) {
		if (timing_config->rdbi) {
			pdram_timing->cl = 36;
			pdram_timing->mr[2] = LPDDR4_RL36_NRTP14 |
						LPDDR4_A_WL16;
		} else {
			pdram_timing->cl = 32;
			pdram_timing->mr[2] = LPDDR4_RL32_NRTP14 |
						LPDDR4_A_WL16;
		}
		pdram_timing->cwl = 16;
		pdram_timing->twr = 34;
		pdram_timing->trtp = 14;
		tmp |= (6 << 4);
	} else {
		if (timing_config->rdbi) {
			pdram_timing->cl = 40;
			pdram_timing->mr[2] = LPDDR4_RL40_NRTP16 |
						LPDDR4_A_WL18;
		} else {
			pdram_timing->cl = 36;
			pdram_timing->mr[2] = LPDDR4_RL36_NRTP16 |
						LPDDR4_A_WL18;
		}
		pdram_timing->cwl = 18;
		pdram_timing->twr = 40;
		pdram_timing->trtp = 16;
		tmp |= (7 << 4);
	}
	pdram_timing->mr[1] = tmp;
	tmp = (timing_config->rdbi ? LPDDR4_DBI_RD_EN : 0) |
	      (timing_config->wdbi ? LPDDR4_DBI_WR_EN : 0);
	switch (timing_config->dramds) {
	case 240:
		pdram_timing->mr[3] = LPDDR4_PDDS_240 | tmp;
		break;
	case 120:
		pdram_timing->mr[3] = LPDDR4_PDDS_120 | tmp;
		break;
	case 80:
		pdram_timing->mr[3] = LPDDR4_PDDS_80 | tmp;
		break;
	case 60:
		pdram_timing->mr[3] = LPDDR4_PDDS_60 | tmp;
		break;
	case 48:
		pdram_timing->mr[3] = LPDDR4_PDDS_48 | tmp;
		break;
	case 40:
	default:
		pdram_timing->mr[3] = LPDDR4_PDDS_40 | tmp;
		break;
	}
	pdram_timing->mr[0] = 0;
	switch (timing_config->dramodt) {
	case 240:
		tmp = LPDDR4_DQODT_240;
		break;
	case 120:
		tmp = LPDDR4_DQODT_120;
		break;
	case 80:
		tmp = LPDDR4_DQODT_80;
		break;
	case 60:
		tmp = LPDDR4_DQODT_60;
		break;
	case 48:
		tmp = LPDDR4_DQODT_48;
		break;
	case 40:
	default:
		tmp = LPDDR4_DQODT_40;
		break;
	}
	switch (timing_config->caodt) {
	case 240:
		pdram_timing->mr11 = LPDDR4_CAODT_240 | tmp;
		break;
	case 120:
		pdram_timing->mr11 = LPDDR4_CAODT_120 | tmp;
		break;
	case 80:
		pdram_timing->mr11 = LPDDR4_CAODT_80 | tmp;
		break;
	case 60:
		pdram_timing->mr11 = LPDDR4_CAODT_60 | tmp;
		break;
	case 48:
		pdram_timing->mr11 = LPDDR4_CAODT_48 | tmp;
		break;
	case 40:
	default:
		pdram_timing->mr11 = LPDDR4_CAODT_40 | tmp;
		break;
	}

	pdram_timing->tinit1 = (LPDDR4_TINIT1 * nmhz + 999) / 1000;
	pdram_timing->tinit2 = (LPDDR4_TINIT2 * nmhz + 999) / 1000;
	pdram_timing->tinit3 = (LPDDR4_TINIT3 * nmhz + 999) / 1000;
	pdram_timing->tinit4 = (LPDDR4_TINIT4 * nmhz + 999) / 1000;
	pdram_timing->tinit5 = (LPDDR4_TINIT5 * nmhz + 999) / 1000;
	pdram_timing->trstl = (LPDDR4_TRSTL * nmhz + 999) / 1000;
	pdram_timing->trsth = (LPDDR4_TRSTH * nmhz + 999) / 1000;
	/* tREFI, average periodic refresh interval, 3.9us(4Gb-16Gb) */
	pdram_timing->trefi = (LPDDR4_TREFI_3_9_US * nmhz + 999) / 1000;
	/* base timing */
	tmp = ((LPDDR4_TRCD * nmhz + 999) / 1000);
	pdram_timing->trcd = max(4, tmp);
	trppb_tmp = ((LPDDR4_TRP_PB * nmhz + 999) / 1000);
	trppb_tmp = max(4, trppb_tmp);
	pdram_timing->trppb = trppb_tmp;
	trp_tmp = ((LPDDR4_TRP_AB * nmhz + 999) / 1000);
	trp_tmp = max(4, trp_tmp);
	pdram_timing->trp = trp_tmp;
	tras_tmp = ((LPDDR4_TRAS * nmhz + 999) / 1000);
	tras_tmp = max(3, tras_tmp);
	pdram_timing->tras_min = tras_tmp;
	pdram_timing->trc = (tras_tmp + trp_tmp);
	tmp = ((LPDDR4_TRRD * nmhz + 999) / 1000);
	pdram_timing->trrd = max(4, tmp);
	if (timing_config->bl == 32)
		pdram_timing->tccd = LPDDR4_TCCD_BL16;
	else
		pdram_timing->tccd = LPDDR4_TCCD_BL32;
	pdram_timing->tccdmw = 4 * pdram_timing->tccd;
	tmp = ((LPDDR4_TWTR * nmhz + 999) / 1000);
	pdram_timing->twtr = max(8, tmp);
	pdram_timing->trtw =  ((LPDDR4_TRTW * nmhz + 999) / 1000);
	pdram_timing->tras_max = ((LPDDR4_TRAS_MAX * nmhz + 999) / 1000);
	pdram_timing->tfaw = (LPDDR4_TFAW * nmhz + 999) / 1000;
	if (ddr_capability_per_die > 0x60000000) {
		/* >= 12Gb */
		pdram_timing->trfc =
			(LPDDR4_TRFC_12GBIT * nmhz + 999) / 1000;
		tmp = (((LPDDR4_TRFC_12GBIT + 7) * nmhz + (nmhz >> 1) +
				999) / 1000);
	} else if (ddr_capability_per_die > 0x30000000) {
		pdram_timing->trfc =
			(LPDDR4_TRFC_6GBIT * nmhz + 999) / 1000;
		tmp = (((LPDDR4_TRFC_6GBIT + 7) * nmhz + (nmhz >> 1) +
				999) / 1000);
	} else {
		pdram_timing->trfc =
			(LPDDR4_TRFC_4GBIT * nmhz + 999) / 1000;
		tmp = (((LPDDR4_TRFC_4GBIT + 7) * nmhz + (nmhz >> 1) +
				999) / 1000);
	}
	pdram_timing->txsr = max(2, tmp);
	pdram_timing->txsnr = max(2, tmp);
	/* tdqsck use rounded down */
	pdram_timing->tdqsck =  ((LPDDR4_TDQSCK_MIN * nmhz +
				(nmhz >> 1)) / 1000);
	pdram_timing->tdqsck_max =  ((LPDDR4_TDQSCK_MAX * nmhz +
				(nmhz >> 1) + 999) / 1000);
	pdram_timing->tppd = LPDDR4_TPPD;
	/* pd and sr */
	tmp = ((LPDDR4_TXP * nmhz + (nmhz >> 1) + 999) / 1000);
	pdram_timing->txp = max(5, tmp);
	tmp = ((LPDDR4_TCKE * nmhz + (nmhz >> 1) + 999) / 1000);
	pdram_timing->tcke = max(4, tmp);
	tmp = ((LPDDR4_TESCKE * nmhz +
		((nmhz * 3) / 4) +
		999) / 1000);
	pdram_timing->tescke = max(3, tmp);
	tmp = ((LPDDR4_TSR * nmhz + 999) / 1000);
	pdram_timing->tsr = max(3, tmp);
	tmp = ((LPDDR4_TCMDCKE * nmhz +
		((nmhz * 3) / 4) +
		999) / 1000);
	pdram_timing->tcmdcke = max(3, tmp);
	pdram_timing->tcscke = ((LPDDR4_TCSCKE * nmhz +
		((nmhz * 3) / 4) +
		999) / 1000);
	tmp = ((LPDDR4_TCKELCS * nmhz + 999) / 1000);
	pdram_timing->tckelcs = max(5, tmp);
	pdram_timing->tcsckeh = ((LPDDR4_TCSCKEH * nmhz +
		((nmhz * 3) / 4) +
		999) / 1000);
	tmp = ((LPDDR4_TCKEHCS * nmhz +
		(nmhz >> 1) + 999) / 1000);
	pdram_timing->tckehcs = max(5, tmp);
	tmp = ((LPDDR4_TMRWCKEL * nmhz + 999) / 1000);
	pdram_timing->tmrwckel = max(10, tmp);
	tmp = ((LPDDR4_TCKELCMD * nmhz + (nmhz >> 1) +
		999) / 1000);
	pdram_timing->tckelcmd = max(3, tmp);
	tmp = ((LPDDR4_TCKEHCMD * nmhz + (nmhz >> 1) +
		999) / 1000);
	pdram_timing->tckehcmd = max(3, tmp);
	tmp = ((LPDDR4_TCKELPD * nmhz + (nmhz >> 1) +
		999) / 1000);
	pdram_timing->tckelpd = max(3, tmp);
	tmp = ((LPDDR4_TCKCKEL * nmhz + (nmhz >> 1) +
		999) / 1000);
	pdram_timing->tckckel = max(3, tmp);
	/* mode register timing */
	tmp = ((LPDDR4_TMRD * nmhz + 999) / 1000);
	pdram_timing->tmrd = max(10, tmp);
	pdram_timing->tmrr = LPDDR4_TMRR;
	pdram_timing->tmrri = pdram_timing->trcd + 3;
	/* ODT */
	pdram_timing->todton = (LPDDR4_TODTON * nmhz + (nmhz >> 1) + 999)
				/ 1000;
	/* ZQ */
	pdram_timing->tzqcal = (LPDDR4_TZQCAL * nmhz + 999) / 1000;
	tmp = ((LPDDR4_TZQLAT * nmhz + 999) / 1000);
	pdram_timing->tzqlat = max(8, tmp);
	tmp = ((LPDDR4_TZQRESET * nmhz + 999) / 1000);
	pdram_timing->tzqreset = max(3, tmp);
	tmp = ((LPDDR4_TZQCKE * nmhz +
		((nmhz * 3) / 4) +
		999) / 1000);
	pdram_timing->tzqcke = max(3, tmp);
	/* write leveling */
	pdram_timing->twlmrd = LPDDR4_TWLMRD;
	pdram_timing->twlo = (LPDDR4_TWLO * nmhz + 999) / 1000;
	pdram_timing->twldqsen = LPDDR4_TWLDQSEN;
	/* CA training */
	pdram_timing->tcaent = (LPDDR4_TCAENT * nmhz + 999) / 1000;
	pdram_timing->tadr = (LPDDR4_TADR * nmhz + 999) / 1000;
	pdram_timing->tmrz = (LPDDR4_TMRZ * nmhz + (nmhz >> 1) + 999) / 1000;
	pdram_timing->tvref_long = (LPDDR4_TVREF_LONG * nmhz + 999) / 1000;
	pdram_timing->tvref_short = (LPDDR4_TVREF_SHORT * nmhz + 999) / 1000;
	/* VRCG */
	pdram_timing->tvrcg_enable = (LPDDR4_TVRCG_ENABLE * nmhz +
					999) / 1000;
	pdram_timing->tvrcg_disable = (LPDDR4_TVRCG_DISABLE * nmhz +
					999) / 1000;
	/* FSP */
	pdram_timing->tfc_long = (LPDDR4_TFC_LONG * nmhz + 999) / 1000;
	tmp = (LPDDR4_TCKFSPE * nmhz + (nmhz >> 1) + 999) / 1000;
	pdram_timing->tckfspe = max(4, tmp);
	tmp = (LPDDR4_TCKFSPX * nmhz + (nmhz >> 1) + 999) / 1000;
	pdram_timing->tckfspx = max(4, tmp);
}

static const uint16_t ddr4_cl_cwl[][6] = {
	/* speed 0~625 625~666 666~800  800~933		933~1066  1066~1200
	 * tCK >1.6   1.5-1.6 1.5-1.25 1.25~1.07  1.07~0.938 0.938~0.833
	 * cl<<8, cwl  cl<<8, cwl  cl<<8, cwl
	 */
	/* DDR4_1600J (10-10-10) */
	{((10 << 8) | 9), ((9 << 8) | 9), ((10 << 8) | 9), 0, 0, 0},
	/* DDR4_1600K (11-11-11) */
	{((10 << 8) | 9), ((9 << 8) | 9), ((11 << 8) | 9), 0, 0, 0},
	/* DDR4_1600L (12-12-12) */
	{((10 << 8) | 9), ((10 << 8) | 9), ((12 << 8) | 9), 0, 0, 0},
	/* DDR4_1866L (12-12-12) */
	{((10 << 8) | 9), ((9 << 8) | 9), ((12 << 8) | 9), ((12 << 8) | 10),
		0, 0},
	/* DDR4_1866M (13-13-13) */
	{((10 << 8) | 9), ((9 << 8) | 9), ((11 << 8) | 9), ((13 << 8) | 10),
		0, 0},
	/* DDR4_1866N (14-14-14) */
	{((10 << 8) | 9), ((10 << 8) | 9), ((12 << 8) | 9), ((14 << 8) | 10),
		0, 0},
	/* DDR4_2133N (14-14-14) */
	{((10 << 8) | 9), ((9 << 8) | 9), ((12 << 8) | 9), ((14 << 8) | 10),
	 ((14 << 8) | 11), 0},
	/* DDR4_2133P (15-15-15) */
	{((10 << 8) | 9), ((9 << 8) | 9), ((11 << 8) | 9), ((13 << 8) | 10),
	 ((15 << 8) | 11), 0},
	/* DDR4_2133R (16-16-16) */
	{((10 << 8) | 9), ((10 << 8) | 9), ((12 << 8) | 9), ((14 << 8) | 10),
	 ((16 << 8) | 11), 0},
	/* DDR4_2400P (15-15-15) */
	{((10 << 8) | 9), ((9 << 8) | 9), ((12 << 8) | 9), ((14 << 8) | 10),
	 ((16 << 8) | 11), ((15 << 8) | 12)},
	/* DDR4_2400R (16-16-16) */
	{((10 << 8) | 9), ((9 << 8) | 9), ((11 << 8) | 9), ((13 << 8) | 10),
	 ((15 << 8) | 11), ((16 << 8) | 12)},
	/* DDR4_2400U (18-18-18) */
	{((10 << 8) | 9), ((10 << 8) | 9), ((12 << 8) | 9), ((14 << 8) | 10),
	 ((16 << 8) | 11), ((18 << 8) | 12)},
	/* DEFAULT */
	{((10 << 8) | 9), ((10 << 8) | 9), ((12 << 8) | 9), ((14 << 8) | 10),
	 ((16 << 8) | 11), ((18 << 8) | 12)},
};

static const uint16_t ddr4_trc[] = {
	/* tccd_l	  tRC	 tFAW	*/
	/* DDR4_1600J (10-10-10) */
	47500,
	/* DDR4_1600K (11-11-11) */
	48750,
	/* DDR4_1600L (12-12-12) */
	50000,
	/* DDR4_1866L (12-12-12) */
	46850,
	/* DDR4_1866M (13-13-13) */
	47920,
	/* DDR4_1866N (14-14-14) */
	49000,
	/* DDR4_2133N (14-14-14) */
	46130,
	/* DDR4_2133P (15-15-15) */
	47060,
	/* DDR4_2133R (16-16-16) */
	48000,
	/* DDR4_2400P (15-15-15) */
	44500,
	/* DDR4_2400R (16-16-16) */
	45320,
	/* DDR4_2400U (18-18-18) */
	47000,
	/* DEFAULT */
	50000
};

struct dram_timing {
	uint32_t min_tck;
	uint32_t min_ps;
};

struct ddr4_common_timing {
	struct dram_timing tccd_s;
	struct dram_timing twtr_s;
	struct dram_timing twtr_l;
	struct dram_timing trtp;
	struct dram_timing twr;
	struct dram_timing tmrd;
	struct dram_timing tmod;
	struct dram_timing tmprr;
	struct dram_timing twr_mpr;
	struct dram_timing tzqinit;
	struct dram_timing tzqopen;
	struct dram_timing tzqcs;
	struct dram_timing txpr;
	struct dram_timing txs;
	struct dram_timing txs_abort;
	struct dram_timing txs_fast;
	struct dram_timing tckesr;
	struct dram_timing tcksre;
	struct dram_timing tcksrx;
	struct dram_timing txp;
	struct dram_timing tcke;
	struct dram_timing tcpded;
	struct dram_timing trfc_1_2g;
	struct dram_timing trfc_1_4g;
	struct dram_timing trfc_1_8g;
	struct dram_timing trfc_1_16g;
	struct dram_timing trfc_2_2g;
	struct dram_timing trfc_2_4g;
	struct dram_timing trfc_2_8g;
	struct dram_timing trfc_2_16g;
	struct dram_timing trfc_4_2g;
	struct dram_timing trfc_4_4g;
	struct dram_timing trfc_4_8g;
	struct dram_timing trfc_4_16g;
	struct dram_timing trefi;
	struct dram_timing trstl;
	struct dram_timing trsth;
	struct dram_timing twlmrd;
	struct dram_timing twldqsen;
	struct dram_timing twlo;
};

struct ddr4_spec_speed_timing {
	struct dram_timing tccd_l;
	struct dram_timing trrd_s_2k;
	struct dram_timing trrd_s_1k;
	struct dram_timing trrd_s_1_2k;
	struct dram_timing trrd_l_2k;
	struct dram_timing trrd_l_1k;
	struct dram_timing trrd_l_1_2k;
	struct dram_timing tfaw_2k;
	struct dram_timing tfaw_1k;
	struct dram_timing tfaw_1_2k;
	struct dram_timing tdllk;
	struct dram_timing tras;
};

static struct ddr4_common_timing ddr4_com_timing = {
	{4, 0},			/* tccd_s:4 tck */
	{2, 2500},		/* twtr_s: max(2tck,2.5ns)*/
	{4, 7500},		/* twtr_l: max(4tck,7.5ns)*/
	{4, 7500},		/* trtp: max(4tck,7.5ns)*/
	{0, 15000},		/* twr: max(4tck,7.5ns)*/
	{8, 0},			/* tmrd: 8tck  */
	{24, 15000},		/* tmod:max(24tck,15ns) */
	{1, 0},			/* tmprr: 1tck*/
	{24, 15000},		/* twr_mpr: equal to tmod */
	{1024, 0},		/* tzqinit:1024tck */
	{512, 0},		/* tzqopen:512tck*/
	{128, 0},		/* tzqcs:128tck */
	{5, 10000},		/* txpr:max(5tck,10ns+trfc(min)) */
	{0, 10000},		/* txs:trfc(min) + 10ns */
	{0, 10000},		/* txs_abort:trfc4(min) + 10ns */
	{0, 10000},		/* txs_fast:trfc4(min) + 10ns */
	{1, 0},			/* tckesr:1tck + tcke(min) */
	{5, 10000},		/* tcksre:max(5tck,10ns) */
	{5, 10000},		/* tcksrx:max(5tck,10ns) */
	{4, 6000},		/* txp:max(4tck,6ns) */
	{3, 5000},		/* tcke:max(3tck,5ns) */
	{4, 0},			/* tcpded:4tck */
	{0, 160000},		/* trfc_1_2g:160ns */
	{0, 260000},		/* trfc_1_4g:260ns */
	{0, 350000},		/* trfc_1_8g:350ns */
	{0, 350000},			/* trfc_1_16g:TBD */
	{0, 110000},		/* trfc_2_2g:110ns */
	{0, 160000},		/* trfc_2_4g:160ns */
	{0, 2600000},		/* trfc_2_8g:260ns */
	{0, 2600000},			/* trfc_2_16g:TBD */
	{0, 90000},		/* trfc_4_2g:90ns */
	{0, 110000},		/* trfc_4_4g:110ns */
	{0, 160000},		/* trfc_4_8g:160ns */
	{0, 160000},			/* trfc_4_16g:TBD */
	{0, 7800000},		/* trefi: 7.8us */
	{0, 200000000},		/* trstl: 200us ? */
	{0, 500000000},		/* trsth: 500us */
	{40, 0},		/* twlmrd: 40tck */
	{25, 0},		/* twldqsen: 25tck */
	{0, 9500}		/* twlo: 9.5ns */
};
struct ddr4_spec_speed_timing ddr4_spec_speed_timing[5] = {
	{/* DDR4_1600 */
		{5, 0},		/* tccd_l:5tck */
		{4, 6000},	/* trrd_s_2k: max(4tck,6ns)*/
		{4, 5000},	/* trrd_s_1k: max(4tck,5ns) */
		{4, 5000},	/* trrd_s_1_2k: max(4tck,5ns) */
		{4, 7500},	/* trrd_l_2k: max(4tck,7.5ns) */
		{4, 6000},	/* trrd_l_1k: max(4tck,6ns)*/
		{4, 5000},	/* trrd_l_1_2k:max(4tck,5ns) */
		{0, 35000},	/* tfaw_2k: 35ns */
		{0, 25000},	/* tfaw_1k: 25ns */
		{0, 20000},	/* tfaw_1_2k: 20ns */
		{597, 0},	/* tdllk: 597ns from micron spec */
		{0, 35000}	/* tras: 35ns */
	},
	{/* DDR4_1866 */
		{5, 0},		/* tccd_l:5tck */
		{4, 5300},	/* trrd_s_2k: max(4tck,5.3ns)*/
		{4, 4200},	/* trrd_s_1k: max(4tck,4.2ns) */
		{4, 4200},	/* trrd_s_1_2k: max(4tck,4.2ns) */
		{4, 6400},	/* trrd_l_2k: max(4tck,6.4ns) */
		{4, 5300},	/* trrd_l_1k: max(4tck,5.3ns)*/
		{4, 5300},	/* trrd_l_1_2k:max(4tck,5.3ns) */
		{0, 30000},	/* tfaw_2k: 30ns */
		{0, 23000},	/* tfaw_1k: 23ns */
		{0, 17000},	/* tfaw_1_2k: 17ns */
		{597, 0},	/* tdllk: 597ns from micron spec */
		{0, 34000}	/* tras: 34ns */
	},
	{/* DDR4_2133 */
		{6, 0},		/* tccd_l:6tck */
		{4, 5300},	/* trrd_s_2k: max(4tck,5.3ns)*/
		{4, 3700},	/* trrd_s_1k: max(4tck,3.7ns) */
		{4, 3700},	/* trrd_s_1_2k: max(4tck,3.7ns) */
		{4, 6400},	/* trrd_l_2k: max(4tck,6.4ns) */
		{4, 5300},	/* trrd_l_1k: max(4tck,5.3ns)*/
		{4, 5300},	/* trrd_l_1_2k:max(4tck,5.3ns) */
		{0, 30000},	/* tfaw_2k: 30ns */
		{0, 21000},	/* tfaw_1k: 21ns */
		{0, 15000},	/* tfaw_1_2k: 15ns */
		{768, 0},	/* tdllk: 768ns from micron spec */
		{0, 33000}	/* tras: 33ns */
	},
	{/* DDR4_2400 */
		{6, 0},		/* tccd_l:6tck */
		{4, 5300},	/* trrd_s_2k: max(4tck,5.3ns)*/
		{4, 3300},	/* trrd_s_1k: max(4tck,3.3ns) */
		{4, 3300},	/* trrd_s_1_2k: max(4tck,3.3ns) */
		{4, 6400},	/* trrd_l_2k: max(4tck,6.4ns) */
		{4, 4900},	/* trrd_l_1k: max(4tck,4.9ns)*/
		{4, 4900},	/* trrd_l_1_2k:max(4tck,4.9ns) */
		{0, 30000},	/* tfaw_2k: 30ns */
		{0, 21000},	/* tfaw_1k: 21ns */
		{0, 13000},	/* tfaw_1_2k: 13ns */
		{768, 0},	/* tdllk: 768ns from micron spec */
		{0, 32000}	/* tras: 32ns */
	},
	{/* DDR4_DEFAULT */
		{5, 0},		/* tccd_l:5tck */
		{4, 6000},	/* trrd_s_2k: max(4tck,6ns)*/
		{4, 5000},	/* trrd_s_1k: max(4tck,5ns) */
		{4, 5000},	/* trrd_s_1_2k: max(4tck,5ns) */
		{4, 7500},	/* trrd_l_2k: max(4tck,7.5ns) */
		{4, 6000},	/* trrd_l_1k: max(4tck,6ns)*/
		{4, 5000},	/* trrd_l_1_2k:max(4tck,5ns) */
		{0, 35000},	/* tfaw_2k: 35ns */
		{0, 25000},	/* tfaw_1k: 25ns */
		{0, 20000},	/* tfaw_1_2k: 20ns */
		{597, 0},	/* tdllk: 597ns from micron spec */
		{0, 35000}	/* tras: 35ns */
	}
};

/*
 * Description: depend on input parameter "timing_config",
 *		and calculate all ddr4
 *		spec timing to "pdram_timing"
 * parameters:
 *   input: timing_config
 *   output: pdram_timing
 */
static void ddr4_get_parameter(struct timing_related_config *timing_config,
			       struct dram_timing_t *pdram_timing)
{
	uint32_t nmhz = timing_config->freq;
	uint32_t ddr_speed_bin = get_max_speed_rate(timing_config);
	uint32_t ddr_capability_per_die = get_max_die_capability(timing_config);
	uint32_t tmp, tmp1;
	uint32_t ddr_speed_rate, page_size;
	struct ddr4_spec_speed_timing *p;

	memset((void *)pdram_timing, 0, sizeof(struct dram_timing_t));
	pdram_timing->mhz = nmhz;
	pdram_timing->al = 0;
	pdram_timing->bl = timing_config->bl;

	/* using for get params from ddr4_spec_speed_timing */
	if (ddr_speed_bin < DDR4_1866L)
		ddr_speed_rate = 0;
	else if (ddr_speed_bin < DDR4_2133N)
		ddr_speed_rate = 1;
	else if (ddr_speed_bin < DDR4_2400P)
		ddr_speed_rate = 2;
	else if (ddr_speed_bin < DDR4_DEFAULT)
		ddr_speed_rate = 3;
	else
		ddr_speed_rate = 4;
	p = &ddr4_spec_speed_timing[ddr_speed_rate];

	if (timing_config->dram_info[0].die_bw == 0)
		page_size = 1024; /* 8 bit dram page size = 1K */
	else
		page_size = 2048;

	if (nmhz <= 625)
		tmp = 0;
	else if (nmhz <= 666)
		tmp = 1;
	else if (nmhz <= 800)
		tmp = 2;
	else if (nmhz <= 933)
		tmp = 3;
	else if (nmhz <= 1066)
		tmp = 4;
	else
		tmp = 5;

	pdram_timing->cl = (ddr4_cl_cwl[ddr_speed_bin][tmp] >> 8) & 0xff;
	pdram_timing->cwl = ddr4_cl_cwl[ddr_speed_bin][tmp] & 0xff;

	/*
	 * MR1: Qoff = 0, TDQS = 0, Write leveling = 0,
	 * AL = 0, RTT_NOM, DRV, DLL = 1
	 */
	switch (timing_config->dramds) {
	case 48:
		tmp = DDR4_DS_48;
		break;
	case 34:
	default:
		tmp = DDR4_DS_34;
		break;
	}
	switch (timing_config->dramodt) {
	case 60:
		pdram_timing->mr[1] = tmp | DDR4_RTT_NOM_60;
		break;
	case 40:
		pdram_timing->mr[1] = tmp | DDR4_RTT_NOM_40;
		break;
	case 120:
		pdram_timing->mr[1] = tmp | DDR4_RTT_NOM_120;
		break;
	case 240:
		pdram_timing->mr[1] = tmp | DDR4_RTT_NOM_240;
		break;
	case 48:
		pdram_timing->mr[1] = tmp | DDR4_RTT_NOM_48;
		break;
	case 80:
		pdram_timing->mr[1] = tmp | DDR4_RTT_NOM_80;
		break;
	case 34:
		pdram_timing->mr[1] = tmp | DDR4_RTT_NOM_34;
		break;
	case 0:
	default:
		pdram_timing->mr[1] = tmp | DDR4_RTT_NOM_DIS;
		break;
	}
	if (timing_config->dllbp)
		pdram_timing->mr[1] |= DDR4_DLL_OFF;
	else
		pdram_timing->mr[1] |= DDR4_DLL_ON;

	tmp = (pdram_timing->cwl < 13) ? (pdram_timing->cwl - 9) :
		((pdram_timing->cwl == 14) ? 4 :
		((pdram_timing->cwl == 16) ? 5 : 6));
	/*
	 * MR2: write CRC = 0, RTT_WR = 0,
	 * LOW power self-refresh = 0, cwl
	 */
	pdram_timing->mr[2] = DDR4_MR2_CWL(tmp);
	/*
	 * MR3: MPR format, wl when crc dm en, reanularity ref mode,
	 * temp readout, PDA, geardown mdoe, MPR operation, MPR page
	 */
	pdram_timing->mr[3] = 0;

	/*
	 * MR4: wpre, rpre, rpre training mode,
	 * selfrefresh abort, cs to CMD lat,
	 * Internal vref monitor, Temp control refresh, Max power saving,
	 */
	pdram_timing->mr[4] = 0;

	/*
	 * MR5: read dbi, wr dbi, dm, ca parity persistent error. rtt_park,
	 * odt input buffer during pd, c/a parity err status,
	 * crc error, a/a parity lat
	 */
	pdram_timing->mr[5] = DIS_ODT_PD |
				(timing_config->rdbi << 12) |
				DDR4_DM(1);

	/* MR6: tCCD_L, VrefDQ training EN/ Range/value, */
	pdram_timing->tccd_l = p->tccd_l.min_tck;
	pdram_timing->mr[6] = (pdram_timing->tccd_l - 4) << 10;

	pdram_timing->trstl = PS_2_CLK(nmhz, ddr4_com_timing.trstl.min_ps);
	pdram_timing->trsth = PS_2_CLK(nmhz, ddr4_com_timing.trsth.min_ps);
	/* tREFI, average periodic refresh interval, 7.8us */
	pdram_timing->trefi = PS_2_CLK(nmhz, ddr4_com_timing.trefi.min_ps);
	/* base timing */
	pdram_timing->trcd = pdram_timing->cl;
	pdram_timing->trp = pdram_timing->cl;
	pdram_timing->trppb = pdram_timing->cl;
	tmp = PS_2_CLK(nmhz, ddr4_com_timing.twr.min_ps);
	pdram_timing->twr = tmp;
	pdram_timing->tdal = tmp + pdram_timing->trp;

	tmp = (tmp < 10) ? 10 : tmp;

	if (tmp < 24)
		tmp = (tmp - 10 + 1) / 2;
	else
		tmp = 6;

	tmp1 = pdram_timing->cl;

	if (tmp1 <= 16)
		tmp1 -= 9;
	else
		tmp1 = (tmp1 - 16) / 2 + 7;

	if (pdram_timing->bl == 4)
		pdram_timing->mr[0] = DDR4_BC4
				| DDR4_CL(tmp1)
				| DDR4_WR_RTP(tmp);
	else
		pdram_timing->mr[0] = DDR4_BL8
				| DDR4_CL(tmp1)
				| DDR4_WR_RTP(tmp);
	if (timing_config->dllbp == 0)
		pdram_timing->mr[0] |= (1 << 8);

	tmp = PS_2_CLK(nmhz, ddr4_com_timing.trtp.min_ps);
	pdram_timing->trtp = max(ddr4_com_timing.trtp.min_tck, tmp);
	pdram_timing->trc =  PS_2_CLK(nmhz, ddr4_trc[ddr_speed_bin]);

	if (page_size == 1024) {
		tmp = PS_2_CLK(nmhz, p->trrd_s_1k.min_ps);
		pdram_timing->trrd = max(p->trrd_s_1k.min_tck, tmp);

		tmp = PS_2_CLK(nmhz, p->trrd_l_1k.min_ps);
		pdram_timing->trrd_l = max(p->trrd_l_1k.min_tck, tmp);

		pdram_timing->tfaw = PS_2_CLK(nmhz, p->tfaw_1k.min_ps);
	} else {
		tmp = PS_2_CLK(nmhz, p->trrd_s_2k.min_ps);
		pdram_timing->trrd = max(p->trrd_s_2k.min_tck, tmp);

		tmp = PS_2_CLK(nmhz, p->trrd_l_2k.min_ps);
		pdram_timing->trrd_l = max(p->trrd_l_2k.min_tck, tmp);

		pdram_timing->tfaw = PS_2_CLK(nmhz, p->tfaw_2k.min_ps);
	}
	pdram_timing->tccd = ddr4_com_timing.tccd_s.min_tck;

	pdram_timing->twtr = max(ddr4_com_timing.twtr_s.min_tck,
				PS_2_CLK(nmhz, ddr4_com_timing.twtr_s.min_ps));
	pdram_timing->twtr_l = max(ddr4_com_timing.twtr_l.min_tck,
				PS_2_CLK(nmhz, ddr4_com_timing.twtr_l.min_ps));

	pdram_timing->trtw = 0;
	pdram_timing->tras_max = 9 * pdram_timing->trefi;
	pdram_timing->tras_min = PS_2_CLK(nmhz, p->tras.min_ps);

	if (ddr_capability_per_die <= 0x10000000) {
		tmp = ddr4_com_timing.trfc_1_2g.min_ps;
		tmp1 = ddr4_com_timing.trfc_4_2g.min_ps;
	} else if (ddr_capability_per_die <= 0x20000000) {
		tmp = ddr4_com_timing.trfc_1_4g.min_ps;
		tmp1 = ddr4_com_timing.trfc_4_4g.min_ps;
	} else if (ddr_capability_per_die <= 0x40000000) {
		tmp = ddr4_com_timing.trfc_1_8g.min_ps;
		tmp1 = ddr4_com_timing.trfc_4_8g.min_ps;
	} else {
		tmp = ddr4_com_timing.trfc_1_16g.min_ps;
		tmp1 = ddr4_com_timing.trfc_4_16g.min_ps;
	}

	pdram_timing->trfc = PS_2_CLK(nmhz, tmp);
	/* txs */
	pdram_timing->txsnr = PS_2_CLK(nmhz, ddr4_com_timing.txs.min_ps + tmp);
	pdram_timing->txs_abort = PS_2_CLK(nmhz,
					ddr4_com_timing.txs_abort.min_ps +
					tmp);
	pdram_timing->txs_fast = PS_2_CLK(nmhz,
					ddr4_com_timing.txs_fast.min_ps + tmp);
	pdram_timing->tdqsck_max = 0;
	/* pd and sr */
	/* txsdll */
	pdram_timing->txsr = p->tdllk.min_tck;
	pdram_timing->txp = max(ddr4_com_timing.txp.min_tck,
			PS_2_CLK(nmhz, ddr4_com_timing.txp.min_ps));
	pdram_timing->txpdll = p->tdllk.min_tck;
	pdram_timing->tdllk = p->tdllk.min_tck;
	pdram_timing->tcke = max(ddr4_com_timing.tcke.min_tck,
			PS_2_CLK(nmhz, ddr4_com_timing.tcke.min_ps));
	pdram_timing->tckesr = pdram_timing->tcke +
			ddr4_com_timing.tckesr.min_tck;
	pdram_timing->tcksre = max(ddr4_com_timing.tcksre.min_tck,
			PS_2_CLK(nmhz, ddr4_com_timing.tcksre.min_ps));
	pdram_timing->tcksrx = max(ddr4_com_timing.tcksrx.min_tck,
			PS_2_CLK(nmhz, ddr4_com_timing.tcksrx.min_ps));
	/* mode register timing */
	pdram_timing->tmod = max(ddr4_com_timing.tmod.min_tck,
			PS_2_CLK(nmhz, ddr4_com_timing.tmod.min_ps));
	pdram_timing->tmrd = ddr4_com_timing.tmrd.min_tck;
	pdram_timing->tmrr = 0;
	pdram_timing->tmprr = ddr4_com_timing.tmprr.min_tck;
	pdram_timing->twr_mpr = max(ddr4_com_timing.twr_mpr.min_tck,
			PS_2_CLK(nmhz, ddr4_com_timing.twr_mpr.min_ps));
	/* ODT */
	pdram_timing->todton = pdram_timing->al + pdram_timing->cwl - 2;
	/* ZQ */
	tmp = ((DDR3_TZQINIT * nmhz + 999) / 1000);
	pdram_timing->tzqinit = ddr4_com_timing.tzqinit.min_tck;
	pdram_timing->tzqcs = ddr4_com_timing.tzqcs.min_tck;
	pdram_timing->tzqoper = ddr4_com_timing.tzqopen.min_tck;
	/* write leveling */
	pdram_timing->twlmrd = ddr4_com_timing.twlmrd.min_tck;
	pdram_timing->twldqsen = ddr4_com_timing.twldqsen.min_tck;
	pdram_timing->twlo = PS_2_CLK(nmhz, ddr4_com_timing.twlo.min_ps);
}

/*
 * Description: depend on input parameter "timing_config",
 *              and calculate correspond "dram_type"
 *              spec timing to "pdram_timing"
 * parameters:
 *   input: timing_config
 *   output: pdram_timing
 * NOTE: MR ODT is set, need to disable by controller
 */
void dram_get_parameter(struct timing_related_config *timing_config,
			struct dram_timing_t *pdram_timing)
{
	switch (timing_config->dram_type) {
	case DDR3:
		ddr3_get_parameter(timing_config, pdram_timing);
		break;
	case LPDDR2:
		lpddr2_get_parameter(timing_config, pdram_timing);
		break;
	case LPDDR3:
		lpddr3_get_parameter(timing_config, pdram_timing);
		break;
	case LPDDR4:
		lpddr4_get_parameter(timing_config, pdram_timing);
		break;
	case DDR4:
		ddr4_get_parameter(timing_config, pdram_timing);
		break;
	}
}
