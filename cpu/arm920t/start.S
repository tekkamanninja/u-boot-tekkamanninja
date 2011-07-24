/*
 *  armboot - Startup Code for ARM920 CPU-core
 *
 *  Copyright (c) 2001	Marius Gröger <mag@sysgo.de>
 *  Copyright (c) 2002	Alex Züpke <azu@sysgo.de>
 *  Copyright (c) 2002	Gary Jennejohn <garyj@denx.de>
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

#include <common.h>
#include <config.h>

/*
 *************************************************************************
 *
 * Jump vector table as in table 3.1 in [1]
 *
 *************************************************************************
 */


.globl _start
_start:	b	start_code
	ldr	pc, _undefined_instruction
	ldr	pc, _software_interrupt
	ldr	pc, _prefetch_abort
	ldr	pc, _data_abort
	ldr	pc, _not_used
	ldr	pc, _irq
	ldr	pc, _fiq

_undefined_instruction:	.word undefined_instruction
_software_interrupt:	.word software_interrupt
_prefetch_abort:	.word prefetch_abort
_data_abort:		.word data_abort
_not_used:		.word not_used
_irq:			.word irq
_fiq:			.word fiq

	.balignl 16,0xdeadbeef


/*
 *************************************************************************
 *
 * Startup Code (called from the ARM reset exception vector)
 *
 * do important init only if we don't start from memory!
 * relocate armboot to ram
 * setup stack
 * jump to second stage
 *
 *************************************************************************
 */

_TEXT_BASE:
	.word	TEXT_BASE

.globl _armboot_start
_armboot_start:
	.word _start

/*
 * These are defined in the board-specific linker script.
 */
.globl _bss_start
_bss_start:
	.word __bss_start

.globl _bss_end
_bss_end:
	.word _end

#ifdef CONFIG_USE_IRQ
/* IRQ stack memory (calculated at run-time) */
.globl IRQ_STACK_START
IRQ_STACK_START:
	.word	0x0badc0de

/* IRQ stack memory (calculated at run-time) */
.globl FIQ_STACK_START
FIQ_STACK_START:
	.word 0x0badc0de
#endif


/*
 * the actual start code
 */

start_code:
	/*
	 * set the cpu to SVC32 mode
	 */
	mrs	r0, cpsr
	bic	r0, r0, #0x1f
	orr	r0, r0, #0xd3
	msr	cpsr, r0

@	bl	coloured_LED_init
@	bl	red_LED_on

#if	defined(CONFIG_AT91RM9200DK) || defined(CONFIG_AT91RM9200EK)
	/*
	 * relocate exception table
	 */
	ldr	r0, =_start
	ldr	r1, =0x0
	mov	r2, #16
copyex:
	subs	r2, r2, #1
	ldr	r3, [r0], #4
	str	r3, [r1], #4
	bne	copyex
#endif

#ifdef CONFIG_S3C24X0
	/* turn off the watchdog */

# if defined(CONFIG_S3C2400)
#  define pWTCON	0x15300000
#  define INTMSK	0x14400008	/* Interupt-Controller base addresses */
#  define CLKDIVN	0x14800014	/* clock divisor register */
#else
#  define pWTCON	0x53000000
#  define INTMSK	0x4A000008	/* Interupt-Controller base addresses */
#  define INTSUBMSK	0x4A00001C
#  define CLKDIVN	0x4C000014	/* clock divisor register */
# endif
#define CLK_CTL_BASE	0x4C000000	/* tekkaman */
#define MDIV_405	0x7f << 12	/* tekkaman */
#define PSDIV_405	0x21		/* tekkaman */
#define MDIV_200	0xa1 << 12	/* tekkaman */
#define PSDIV_200	0x31		/* tekkaman */


	ldr	r0, =pWTCON
	mov	r1, #0x0
	str	r1, [r0]

	/*
	 * mask all IRQs by setting all bits in the INTMR - default
	 */
	mov	r1, #0xffffffff
	ldr	r0, =INTMSK
	str	r1, [r0]
# if defined(CONFIG_S3C2410)
	ldr	r1, =0x7ff
	ldr	r0, =INTSUBMSK
	str	r1, [r0]
# endif

#if defined(CONFIG_S3C2440)
	ldr	r1, =0x7fff	
	ldr	r0, =INTSUBMSK
	str	r1, [r0]
#endif


#if defined(CONFIG_S3C2440)
	/* FCLK:HCLK:PCLK = 1:4:8 */
	ldr	r0, =CLKDIVN
	mov	r1, #5
	str	r1, [r0]
	
	mrc	p15, 0, r1, c1, c0, 0	
	orr	r1, r1, #0xc0000000		
	mcr	p15, 0, r1, c1, c0, 0	
	
	
	mov	r1, #CLK_CTL_BASE	
	mov	r2, #MDIV_405	
	add	r2, r2, #PSDIV_405	
	str	r2, [r1, #0x04]		/* MPLLCON tekkaman */

#else
	/* FCLK:HCLK:PCLK = 1:2:4 */
	/* default FCLK is 120 MHz ! */
	ldr	r0, =CLKDIVN
	mov	r1, #3
	str	r1, [r0]

	
	mrc	p15, 0, r1, c1, c0, 0	
	orr	r1, r1, #0xc0000000	
	mcr	p15, 0, r1, c1, c0, 0	/*write ctrl register tekkaman*/
	
	
	mov	r1, #CLK_CTL_BASE	/* tekkaman*/
	mov	r2, #MDIV_200	
	add	r2, r2, #PSDIV_200	
	str	r2, [r1, #0x04]
#endif
#endif	/* CONFIG_S3C24X0 */

	/*
	 * we do sys-critical inits only at reboot,
	 * not when booting from ram!
	 */
#ifndef CONFIG_SKIP_LOWLEVEL_INIT
	bl	cpu_init_crit
#endif

/***************** CHECK_CODE_POSITION ******************************************/
	adr	r0, _start		/* r0 <- current position of code   */
	ldr	r1, _TEXT_BASE		/* test if we run from flash or RAM */
	cmp	r0, r1			/* don't reloc during debug         */
	beq	stack_setup
/***************** CHECK_CODE_POSITION ******************************************/

/***************** CHECK_BOOT_FLASH ******************************************/
	ldr	r1, =( (4<<28)|(3<<4)|(3<<2) )		/* address of Internal SRAM  0x4000003C*/
	mov	r0, #0		/* r0 = 0 */
	str	r0, [r1]


	mov	r1, #0x3c		/* address of men  0x0000003C*/
	ldr	r0, [r1]
	cmp	r0, #0
	bne	relocate

	/* recovery  */
	ldr	r0, =(0xdeadbeef)
	ldr	r1, =( (4<<28)|(3<<4)|(3<<2) )
	str	r0, [r1]
/***************** CHECK_BOOT_FLASH ******************************************/

/***************** NAND_BOOT *************************************************/

#define LENGTH_UBOOT 0x60000
#define NAND_CTL_BASE 0x4E000000

#ifdef CONFIG_S3C2440
/* Offset */
#define oNFCONF 0x00
#define oNFCONT 0x04
#define oNFCMD 0x08
#define oNFSTAT 0x20

	@ reset NAND
	mov	r1, #NAND_CTL_BASE
	ldr	r2, =( (7<<12)|(7<<8)|(7<<4)|(0<<0) )
	str	r2, [r1, #oNFCONF]
	ldr	r2, [r1, #oNFCONF]
	
	ldr	r2, =( (1<<4)|(0<<1)|(1<<0) )	@ Active low CE Control 
	str	r2, [r1, #oNFCONT]
	ldr	r2, [r1, #oNFCONT]
	
	ldr	r2, =(0x6)	@ RnB Clear
	str	r2, [r1, #oNFSTAT]
	ldr	r2, [r1, #oNFSTAT]
	
	mov	r2, #0xff	@ RESET command
	strb	r2, [r1, #oNFCMD]
	
	mov	r3, #0	@ wait
nand1: 
	add	r3, r3, #0x1
	cmp	r3, #0xa
	blt	nand1

nand2:
	ldr	r2, [r1, #oNFSTAT]	@ wait ready
	tst	r2, #0x4
	beq	nand2
	
	
	ldr	r2, [r1, #oNFCONT]
	orr	r2, r2, #0x2	@ Flash Memory Chip Disable
	str	r2, [r1, #oNFCONT]
	
	@ get read to call C functions (for nand_read())
	ldr	sp, DW_STACK_START	@ setup stack pointer
	mov	fp, #0	@ no previous frame, so fp=0

	@ copy U-Boot to RAM
	ldr	r0, =TEXT_BASE
	mov	r1, #0x0
	mov	r2, #LENGTH_UBOOT
	bl	nand_read_ll
	tst	r0, #0x0
	beq	ok_nand_read

bad_nand_read:
loop2:
	b	loop2	@ infinite loop
ok_nand_read:
	@ verify
	mov	r0, #0
	ldr	r1, =TEXT_BASE
	mov	r2, #0x400	@ 4 bytes * 1024 = 4K-bytes
go_next:
	ldr	r3, [r0], #4
	ldr	r4, [r1], #4
	teq	r3, r4
	bne	notmatch
	subs	r2, r2, #4
	beq	stack_setup
	bne	go_next

notmatch:
loop3:
	b	loop3	@ infinite loop
#endif

#ifdef	CONFIG_S3C2410

/* Offset */
#define oNFCONF 0x00
#define oNFCMD 0x04
#define oNFSTAT 0x10

	@ reset NAND
	mov	r1, #NAND_CTL_BASE
	ldr	r2, =0xf830	@ initial value
	str	r2, [r1, #oNFCONF]
	ldr	r2, [r1, #oNFCONF]
	bic	r2, r2, #0x800	@ enable chip
	str	r2, [r1, #oNFCONF]
	mov	r2, #0xff		@ RESET command
	strb	r2, [r1, #oNFCMD]
	
	
	mov	r3, #0	@ wait
nand1:
	add	r3, r3, #0x1
	cmp	r3, #0xa
	blt	nand1

nand2:
	ldr	r2, [r1, #oNFSTAT]	@ wait ready
	tst	r2, #0x1
	beq	nand2
	
	ldr	r2, [r1, #oNFCONF]
	orr	r2, r2, #0x800	@ disable chip
	str	r2, [r1, #oNFCONF]
	
	@ get read to call C functions (for nand_read())
	ldr	sp, DW_STACK_START	@ setup stack pointer
	mov	fp, #0	@ no previous frame, so fp=0
	
	@ copy U-Boot to RAM
	ldr	r0, =TEXT_BASE
	mov	r1, #0x0
	mov	r2, #LENGTH_UBOOT
	bl	nand_read_ll
	tst	r0, #0x0
	beq	ok_nand_read

bad_nand_read:
loop2:
	b	loop2	@ infinite loop


ok_nand_read:
	@ verify
	mov	r0, #0
	ldr	r1, =TEXT_BASE
	mov	r2, #0x400	@ 4 bytes * 1024 = 4K-bytes
go_next:
	ldr	r3, [r0], #4
	ldr	r4, [r1], #4
	teq	r3, r4
	bne	notmatch
	subs	r2, r2, #4
	beq	stack_setup
	bne	go_next

notmatch:
loop3:
	b	loop3	@ infinite loop

#endif
/***************** NAND_BOOT *************************************************/

/***************** NOR_BOOT *************************************************/
relocate:				/* relocate U-Boot to RAM	    */
      /*********** CHECK_FOR_MAGIC_NUMBER***************/
	ldr	r1, =(0xdeadbeef)
	cmp	r0, r1
	bne	loop3
      /*********** CHECK_FOR_MAGIC_NUMBER***************/
	adr	r0, _start		/* r0 <- current position of code   */
	ldr	r1, _TEXT_BASE		/* test if we run from flash or RAM */
	ldr	r2, _armboot_start
	ldr	r3, _bss_start
	sub	r2, r3, r2		/* r2 <- size of armboot            */
	add	r2, r0, r2		/* r2 <- source end address         */

copy_loop:
	ldmia	r0!, {r3-r10}		/* copy from source address [r0]    */
	stmia	r1!, {r3-r10}		/* copy to   target address [r1]    */
	cmp	r0, r2			/* until source end addreee [r2]    */
	ble	copy_loop
/***************** NOR_BOOT *************************************************/

	/* Set up the stack						    */
stack_setup:
	ldr	r0, _TEXT_BASE		/* upper 128 KiB: relocated uboot   */
	sub	r0, r0, #CONFIG_SYS_MALLOC_LEN	/* malloc area              */
	sub	r0, r0, #CONFIG_SYS_GBL_DATA_SIZE /* bdinfo                 */
#ifdef CONFIG_USE_IRQ
	sub	r0, r0, #(CONFIG_STACKSIZE_IRQ+CONFIG_STACKSIZE_FIQ)
#endif
	sub	sp, r0, #12		/* leave 3 words for abort-stack    */

clear_bss:
	ldr	r0, _bss_start		/* find start of bss segment        */
	ldr	r1, _bss_end		/* stop here                        */
	mov	r2, #0x00000000		/* clear                            */

clbss_l:str	r2, [r0]		/* clear loop...                    */
	add	r0, r0, #4
	cmp	r0, r1
	ble	clbss_l

	ldr	pc, _start_armboot

#if defined(CONFIG_MINI2440_LED)
#define GPIO_CTL_BASE 0x56000000
#define oGPIO_B 0x10
#define oGPIO_CON 0x0
/* R/W, Configures the pins of the port */
#define oGPIO_DAT 0x4
#define oGPIO_UP 0x8
/* R/W, Pull-up disable register */
	mov	r1, #GPIO_CTL_BASE
	add	r1, r1, #oGPIO_B
	ldr	r2, =0x295551
	str	r2, [r1, #oGPIO_CON]
	mov	r2, #0xff
	str	r2, [r1, #oGPIO_UP]
	ldr	r2, =0x1c1
	str	r2, [r1, #oGPIO_DAT]
#endif

_start_armboot:	.word start_armboot
#define STACK_BASE 0x33f00000
#define STACK_SIZE 0x10000
	.align	2
DW_STACK_START:	.word	STACK_BASE+STACK_SIZE-4 

/*
 *************************************************************************
 *
 * CPU_init_critical registers
 *
 * setup important registers
 * setup memory timing
 *
 *************************************************************************
 */


#ifndef CONFIG_SKIP_LOWLEVEL_INIT
cpu_init_crit:
	/*
	 * flush v4 I/D caches
	 */
	mov	r0, #0
	mcr	p15, 0, r0, c7, c7, 0	/* flush v3/v4 cache */
	mcr	p15, 0, r0, c8, c7, 0	/* flush v4 TLB */

	/*
	 * disable MMU stuff and caches
	 */
	mrc	p15, 0, r0, c1, c0, 0
	bic	r0, r0, #0x00002300	@ clear bits 13, 9:8 (--V- --RS)
	bic	r0, r0, #0x00000087	@ clear bits 7, 2:0 (B--- -CAM)
	orr	r0, r0, #0x00000002	@ set bit 2 (A) Align
	orr	r0, r0, #0x00001000	@ set bit 12 (I) I-Cache
	mcr	p15, 0, r0, c1, c0, 0

	/*
	 * before relocating, we have to setup RAM timing
	 * because memory timing is board-dependend, you will
	 * find a lowlevel_init.S in your board directory.
	 */
	mov	ip, lr

	bl	lowlevel_init

	mov	lr, ip
	mov	pc, lr
#endif /* CONFIG_SKIP_LOWLEVEL_INIT */

/*
 *************************************************************************
 *
 * Interrupt handling
 *
 *************************************************************************
 */

@
@ IRQ stack frame.
@
#define S_FRAME_SIZE	72

#define S_OLD_R0	68
#define S_PSR		64
#define S_PC		60
#define S_LR		56
#define S_SP		52

#define S_IP		48
#define S_FP		44
#define S_R10		40
#define S_R9		36
#define S_R8		32
#define S_R7		28
#define S_R6		24
#define S_R5		20
#define S_R4		16
#define S_R3		12
#define S_R2		8
#define S_R1		4
#define S_R0		0

#define MODE_SVC	0x13
#define I_BIT		0x80

/*
 * use bad_save_user_regs for abort/prefetch/undef/swi ...
 * use irq_save_user_regs / irq_restore_user_regs for IRQ/FIQ handling
 */

	.macro	bad_save_user_regs
	sub	sp, sp, #S_FRAME_SIZE
	stmia	sp, {r0 - r12}			@ Calling r0-r12
	ldr	r2, _armboot_start
	sub	r2, r2, #(CONFIG_STACKSIZE)
	sub	r2, r2, #(CONFIG_SYS_MALLOC_LEN)
	/* set base 2 words into abort stack */
	sub	r2, r2, #(CONFIG_SYS_GBL_DATA_SIZE+8)
	ldmia	r2, {r2 - r3}			@ get pc, cpsr
	add	r0, sp, #S_FRAME_SIZE		@ restore sp_SVC

	add	r5, sp, #S_SP
	mov	r1, lr
	stmia	r5, {r0 - r3}			@ save sp_SVC, lr_SVC, pc, cpsr
	mov	r0, sp
	.endm

	.macro	irq_save_user_regs
	sub	sp, sp, #S_FRAME_SIZE
	stmia	sp, {r0 - r12}			@ Calling r0-r12
	add	r7, sp, #S_PC
	stmdb	r7, {sp, lr}^			@ Calling SP, LR
	str	lr, [r7, #0]			@ Save calling PC
	mrs	r6, spsr
	str	r6, [r7, #4]			@ Save CPSR
	str	r0, [r7, #8]			@ Save OLD_R0
	mov	r0, sp
	.endm

	.macro	irq_restore_user_regs
	ldmia	sp, {r0 - lr}^			@ Calling r0 - lr
	mov	r0, r0
	ldr	lr, [sp, #S_PC]			@ Get PC
	add	sp, sp, #S_FRAME_SIZE
	/* return & move spsr_svc into cpsr */
	subs	pc, lr, #4
	.endm

	.macro get_bad_stack
	ldr	r13, _armboot_start		@ setup our mode stack
	sub	r13, r13, #(CONFIG_STACKSIZE)
	sub	r13, r13, #(CONFIG_SYS_MALLOC_LEN)
	/* reserve a couple spots in abort stack */
	sub	r13, r13, #(CONFIG_SYS_GBL_DATA_SIZE+8)

	str	lr, [r13]			@ save caller lr / spsr
	mrs	lr, spsr
	str	lr, [r13, #4]

	mov	r13, #MODE_SVC			@ prepare SVC-Mode
	@ msr	spsr_c, r13
	msr	spsr, r13
	mov	lr, pc
	movs	pc, lr
	.endm

	.macro get_irq_stack			@ setup IRQ stack
	ldr	sp, IRQ_STACK_START
	.endm

	.macro get_fiq_stack			@ setup FIQ stack
	ldr	sp, FIQ_STACK_START
	.endm

/*
 * exception handlers
 */
	.align  5
undefined_instruction:
	get_bad_stack
	bad_save_user_regs
	bl	do_undefined_instruction

	.align	5
software_interrupt:
	get_bad_stack
	bad_save_user_regs
	bl	do_software_interrupt

	.align	5
prefetch_abort:
	get_bad_stack
	bad_save_user_regs
	bl	do_prefetch_abort

	.align	5
data_abort:
	get_bad_stack
	bad_save_user_regs
	bl	do_data_abort

	.align	5
not_used:
	get_bad_stack
	bad_save_user_regs
	bl	do_not_used

#ifdef CONFIG_USE_IRQ

	.align	5
irq:
//Apollo +
/*
	get_irq_stack
	irq_save_user_regs
	bl	do_irq
	irq_restore_user_regs
*/
    /* use IRQ for USB and DMA */
        sub    lr, lr, #4             @ the return address
        ldr    sp, IRQ_STACK_START     @ the stack for irq
       stmdb   sp!,  { r0-r12,lr }     @ save registers
        
        ldr    lr, =int_return         @ set the return addr
        ldr    pc, =IRQ_Handle         @ call the isr
int_return:
        ldmia  sp!, { r0-r12,pc }^    @ return from interrupt
//Apollo -

	.align	5
fiq:
	get_fiq_stack
	/* someone ought to write a more effiction fiq_save_user_regs */
	irq_save_user_regs
	bl	do_fiq
	irq_restore_user_regs

#else

	.align	5
irq:
	get_bad_stack
	bad_save_user_regs
	bl	do_irq

	.align	5
fiq:
	get_bad_stack
	bad_save_user_regs
	bl	do_fiq

#endif
