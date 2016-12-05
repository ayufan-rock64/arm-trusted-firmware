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

#ifndef __PLAT_SIP_CALLS_H__
#define __PLAT_SIP_CALLS_H__

#define CONFIG_DRAM_INIT	0x00
#define CONFIG_DRAM_SET_RATE	0x01
#define CONFIG_DRAM_ROUND_RATE	0x02
#define CONFIG_DRAM_SET_AT_SR	0x03
#define CONFIG_DRAM_GET_BW	0x04
#define CONFIG_DRAM_GET_RATE	0x05
#define CONFIG_DRAM_CLR_IRQ	0x06
#define CONFIG_DRAM_SET_PARAM	0x07

void fiq_disable_flag(uint32_t cpu_id);
void fiq_enable_flag(uint32_t cpu_id);
uint32_t get_uart_irq_id(void);

#define RK_PLAT_SIP_NUM_CALLS	4

#endif /* __PLAT_SIP_CALLS_H__ */
