#include <common.h>
#include <asm/arch/s3c24x0_cpu.h>
#include <asm/io.h>
#include <asm/proc-armv/ptrace.h>
#include "common_usb.h"

#define BIT_USBD		(0x1<<25)
#define BIT_DMA2		(0x1<<19)
#define BIT_WDT_AC97		(0x1<<9)
#define BIT_TIMER4		(0x1<<14)
#define BIT_ALLMSK		(0xFFFFFFFF)

/* Start : add by www.100ask.net */
void (*isr_handle_array[50])(void);

    
extern void IsrUsbd(void);
extern void IsrDma2(void);

/************************* Timer ********************************/
static int intCount;

void IsrWatchdog(void);

void ClearPending_my(int bit) 
{
	struct s3c24x0_interrupt * const intregs = s3c24x0_get_base_interrupt();
 	writel(bit, &intregs->SRCPND);
	writel(bit, &intregs->INTPND);
}

void Timer_InitEx(void)
{
	struct s3c24x0_interrupt * const intregs = s3c24x0_get_base_interrupt();
	intCount=0;	
 	writel((1<<13), &intregs->SUBSRCPND);
	ClearPending_my(BIT_WDT_AC97/*BIT_WDT*/);
 	writel((readl(&intregs->INTMSK) & ~(BIT_WDT_AC97 /*BIT_WDT*/)), &intregs->INTMSK);
	writel((readl(&intregs->INTSUBMSK) & ~(1<<13)), &intregs->INTSUBMSK);
}


void Timer_StartEx(void)
{
	struct s3c24x0_watchdog * const wdtregs = s3c24x0_get_base_watchdog();
    
	writel((((get_PCLK()/1000000-1)<<8)|(0<<3)|(1<<2)), &wdtregs->WTCON);  // 16us
	writel(0xffff, &wdtregs->WTDAT);
	writel(0xffff, &wdtregs->WTCNT);

	// 1/16/(65+1),interrupt enable,reset disable,watchdog enable
	writel((((get_PCLK()/1000000-1)<<8)|(0<<3)|(1<<2)|(0<<0)|(1<<5)), &wdtregs->WTCON);  
}

unsigned int Timer_StopEx(void)
{
	int count;
	struct s3c24x0_watchdog * const wdtregs = s3c24x0_get_base_watchdog();
	struct s3c24x0_interrupt * const intregs = s3c24x0_get_base_interrupt();

	writel(((get_PCLK()/1000000-1)<<8), &wdtregs->WTCON);
	writel((readl(&intregs->INTMSK) | BIT_WDT_AC97), &intregs->INTMSK);
	writel((readl(&intregs->INTSUBMSK) | (1<<13)), &intregs->INTSUBMSK);	

	count = (0xffff - readl(&wdtregs->WTCNT)) + (intCount*0xffff);
	return ((unsigned int)count*16/1000000);
}


void  IsrWatchdog(void)
{
	struct s3c24x0_interrupt * const intregs = s3c24x0_get_base_interrupt();
	writel((1<<13), &intregs->SUBSRCPND);
	ClearPending_my(BIT_WDT_AC97 /* BIT_WDT */);
	intCount++;   	
}

int g_TimerIntHappen;
void  IsrTimer4(void)
{
	ClearPending_my(BIT_TIMER4);
	*(volatile int *)&g_TimerIntHappen = 1;
}


void Dummy_isr(void)
{
	struct s3c24x0_interrupt * const intregs = s3c24x0_get_base_interrupt();
	printf("Dummy_isr error, interrupt number: %d, INTMSK = 0x%x\n", readl(&intregs->INTOFFSET), readl(&intregs->INTMSK));
	while(1);
}

void Isr_Init(void)
{
	int i = 0;
	struct s3c24x0_interrupt * const intregs = s3c24x0_get_base_interrupt();
    
	for (i = 0; i < sizeof(isr_handle_array) / sizeof(isr_handle_array[0]); i++ )
	{
		isr_handle_array[i] = Dummy_isr;
	}
	writel(0, &intregs->INTMOD);
	writel(BIT_ALLMSK, &intregs->INTMSK);
// All=IRQ mode
// All interrupt is masked.

	isr_handle_array[ISR_TIMER4_OFT] = IsrTimer4;
	isr_handle_array[ISR_WDT_OFT]  = IsrWatchdog;

//#ifdef CONFIG_USB_DEVICE
	isr_handle_array[ISR_USBD_OFT] = IsrUsbd;
	isr_handle_array[ISR_DMA2_OFT] = IsrDma2;
	ClearPending_my(BIT_DMA2);
	ClearPending_my(BIT_USBD);
//#endif    
}


void IRQ_Handle(void)
{
	struct s3c24x0_gpio * const gpio = s3c24x0_get_base_gpio();
	struct s3c24x0_interrupt * const intregs = s3c24x0_get_base_interrupt();
	unsigned long oft = readl(&intregs->INTOFFSET);
    
	// clean int
	if( oft == 4 ) {
		writel((1<<7), &gpio->EINTPEND);
	}
	writel((1<<oft), &intregs->SRCPND);
	writel(readl(&intregs->INTPND), &intregs->INTPND);
 

	/* run the isr */
	isr_handle_array[oft]();
}

/* End : add by www.100ask.net */
