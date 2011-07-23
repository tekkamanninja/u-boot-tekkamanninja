/*
 * Originates from Samsung's u-boot 1.1.6 port to S3C6400 / SMDK6400
 *
 * Copyright (C) 2008
 * Guennadi Liakhovetki, DENX Software Engineering, <lg@denx.de>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <config.h>
#include <asm/arch/s3c6400.h>

	.globl mem_ctrl_asm_init
mem_ctrl_asm_init:
	/* DMC1 base address 0x7e001000 */
	ldr	r0, =ELFIN_DMC1_BASE

	ldr	r1, =0x4
	str	r1, [r0, #INDEX_DMC_MEMC_CMD]

	ldr	r1, =DMC_DDR_REFRESH_PRD
	str	r1, [r0, #INDEX_DMC_REFRESH_PRD]

	ldr	r1, =DMC_DDR_CAS_LATENCY
	str	r1, [r0, #INDEX_DMC_CAS_LATENCY]

	ldr	r1, =DMC_DDR_t_DQSS
	str	r1, [r0, #INDEX_DMC_T_DQSS]

	ldr	r1, =DMC_DDR_t_MRD
	str	r1, [r0, #INDEX_DMC_T_MRD]

	ldr	r1, =DMC_DDR_t_RAS
	str	r1, [r0, #INDEX_DMC_T_RAS]

	ldr	r1, =DMC_DDR_t_RC
	str	r1, [r0, #INDEX_DMC_T_RC]

	ldr	r1, =DMC_DDR_t_RCD
	ldr	r2, =DMC_DDR_schedule_RCD
	orr	r1, r1, r2
	str	r1, [r0, #INDEX_DMC_T_RCD]

	ldr	r1, =DMC_DDR_t_RFC
	ldr	r2, =DMC_DDR_schedule_RFC
	orr	r1, r1, r2
	str	r1, [r0, #INDEX_DMC_T_RFC]

	ldr	r1, =DMC_DDR_t_RP
	ldr	r2, =DMC_DDR_schedule_RP
	orr	r1, r1, r2
	str	r1, [r0, #INDEX_DMC_T_RP]

	ldr	r1, =DMC_DDR_t_RRD
	str	r1, [r0, #INDEX_DMC_T_RRD]

	ldr	r1, =DMC_DDR_t_WR
	str	r1, [r0, #INDEX_DMC_T_WR]

	ldr	r1, =DMC_DDR_t_WTR
	str	r1, [r0, #INDEX_DMC_T_WTR]

	ldr	r1, =DMC_DDR_t_XP
	str	r1, [r0, #INDEX_DMC_T_XP]

	ldr	r1, =DMC_DDR_t_XSR
	str	r1, [r0, #INDEX_DMC_T_XSR]

	ldr	r1, =DMC_DDR_t_ESR
	str	r1, [r0, #INDEX_DMC_T_ESR]

	ldr	r1, =DMC1_MEM_CFG
	str	r1, [r0, #INDEX_DMC_MEMORY_CFG]

	ldr	r1, =DMC1_MEM_CFG2
	str	r1, [r0, #INDEX_DMC_MEMORY_CFG2]

	ldr	r1, =DMC1_CHIP0_CFG
	str	r1, [r0, #INDEX_DMC_CHIP_0_CFG]

	ldr	r1, =DMC_DDR_32_CFG
	str	r1, [r0, #INDEX_DMC_USER_CONFIG]

	/* DMC0 DDR Chip 0 configuration direct command reg */
	ldr	r1, =DMC_NOP0
	str	r1, [r0, #INDEX_DMC_DIRECT_CMD]

	/* Precharge All */
	ldr	r1, =DMC_PA0
	str	r1, [r0, #INDEX_DMC_DIRECT_CMD]

	/* Auto Refresh 2 time */
	ldr	r1, =DMC_AR0
	str	r1, [r0, #INDEX_DMC_DIRECT_CMD]
	str	r1, [r0, #INDEX_DMC_DIRECT_CMD]

	/* MRS */
	ldr	r1, =DMC_mDDR_EMR0
	str	r1, [r0, #INDEX_DMC_DIRECT_CMD]

	/* Mode Reg */
	ldr	r1, =DMC_mDDR_MR0
	str	r1, [r0, #INDEX_DMC_DIRECT_CMD]

	/* Enable DMC1 */
	mov	r1, #0x0
	str	r1, [r0, #INDEX_DMC_MEMC_CMD]

check_dmc1_ready:
	ldr	r1, [r0, #INDEX_DMC_MEMC_STATUS]
	mov	r2, #0x3
	and	r1, r1, r2
	cmp	r1, #0x1
	bne	check_dmc1_ready
	nop

#ifdef CONFIG_MEM_AUTO_ADAPT_MINI6410
	ldr	r1, =CONFIG_MEM_TESTWORD1_MINI6410
	ldr	r3, =CONFIG_MEM_TESTPOINT_MIRROR_MINI6410
	ldr	r4, =CONFIG_MEM_TESTPOINT_MINI6410
	str	r1, [r3]
	ldr	r2, [r4]
	cmp	r1, r2
	bne	mem_ctrl_asm_init_end

	ldr	r1, =CONFIG_MEM_TESTWORD2_MINI6410
	str	r1, [r4]
	ldr	r2, [r3]
	cmp	r1, r2
	bne	mem_ctrl_asm_init_end

	ldr	r1, =0x3
	str	r1, [r0, #INDEX_DMC_MEMC_CMD]

check_dmc1_pause:
	ldr	r1, [r0, #INDEX_DMC_MEMC_STATUS]
	mov	r2, #0x3
	and	r1, r1, r2
	cmp	r1, #0x2
	bne	check_dmc1_pause
	nop

	ldr	r1, =0x4
	str	r1, [r0, #INDEX_DMC_MEMC_CMD]

check_dmc1_config:
	ldr	r1, [r0, #INDEX_DMC_MEMC_STATUS]
	mov	r2, #0x3
	and	r1, r1, r2
	cmp	r1, #0x0
	bne	check_dmc1_config
	nop

	ldr	r1, =DMC1_MEM_CFG_128MB
	str	r1, [r0, #INDEX_DMC_MEMORY_CFG]

	ldr	r1, =DMC1_CHIP0_CFG_128MB
	str	r1, [r0, #INDEX_DMC_CHIP_0_CFG]
	nop

	/* Enable DMC1 */
	mov	r1, #0x0
	str	r1, [r0, #INDEX_DMC_MEMC_CMD]

check_dmc1_ready_128m:
	ldr	r1, [r0, #INDEX_DMC_MEMC_STATUS]
	mov	r2, #0x3
	and	r1, r1, r2
	cmp	r1, #0x1
	bne	check_dmc1_ready_128m
	nop

mem_ctrl_asm_init_end:
#endif

#if defined(CONFIG_MINI6410)
/* ------------------------------------------------------------------------- */
#define SROM_BC1_REG_Tacs     (0x0)   // 0clk         address set-up
#define SROM_BC1_REG_Tcos     (0x4)   // 4clk         chip selection set-up
#define SROM_BC1_REG_Tacc     (0xE)   // 14clk        access cycle
#define SROM_BC1_REG_Tcoh     (0x1)   // 1clk         chip selection hold
#define SROM_BC1_REG_Tah      (0x4)   // 4clk         address holding time
#define SROM_BC1_REG_Tacp     (0x6)   // 6clk         page mode access cycle
#define SROM_BC1_REG_PMC      (0x0)   // normal(1data)page mode configuration

#define SROM_BW_REG_DATA	((1<<7) | (1<<6) | (1<<4))
#define SROM_BW_REG_BC1		(0xf << 4)
#define SROM_BC1_REG_DATA	((SROM_BC1_REG_Tacs << 28) | (SROM_BC1_REG_Tcos << 24) | \
				(SROM_BC1_REG_Tacc << 16) | (SROM_BC1_REG_Tcoh << 12) | \
				(SROM_BC1_REG_Tah << 8) | (SROM_BC1_REG_Tacp << 4) | \
				(SROM_BC1_REG_PMC))

	ldr	r0, =ELFIN_SROM_BASE

	ldr	r1, [r0, #SROM_BW_REG_DATA]
	mov	r2, #(~SROM_BW_REG_BC1)
	and	r1, r1, r2
	mov	r2, #SROM_BW_REG_DATA
	orr	r1, r1, r2	
	str	r1, [r0, #INDEX_SROM_BW_REG]

	ldr	r1, =SROM_BC1_REG_DATA
	str	r1, [r0, #INDEX_SROM_BC1_REG]
#endif

	mov	pc, lr

	.ltorg
