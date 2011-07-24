/****************************************************************
 NAME: usblib.c
 DESC: S3C2440X USB library functions
 HISTORY:
 Mar.25.2002:purnnamu: ported for S3C2410X.
 Mar.27.2002:purnnamu: DMA is enabled.
 ****************************************************************/
#include <common.h>
#include <asm/arch/s3c24x0_cpu.h>
#include <asm/io.h>

#include "2440usb.h"
#include "usbmain.h"
#include "usblib.h"
#include "usbsetup.h"
#include "usbmain.h"

#define BIT_USBD		(0x1<<25)

extern volatile U32 dwUSBBufReadPtr;
extern volatile U32 dwUSBBufWritePtr;
extern volatile U32 dwPreDMACnt;
extern volatile U32 dwNextDMACnt;



void ConfigUsbd(void)
{
	struct s3c24x0_interrupt * intregs = s3c24x0_get_base_interrupt();
	ReconfigUsbd();
	writel((readl(&intregs->INTMSK) & ~(BIT_USBD)), &intregs->INTMSK); 
}


void ReconfigUsbd(void)
{
// *** End point information ***
//   EP0: control
//   EP1: bulk in end point
//   EP2: not used
//   EP3: bulk out end point
//   EP4: not used
	struct s3c24x0_usb_device * const usbdevregs	= s3c24x0_get_base_usb_device();   
 
	writeb(PWR_REG_DEFAULT_VALUE, &usbdevregs->PWR_REG); //disable suspend mode
	writeb(0, &usbdevregs->INDEX_REG); 
	writeb(FIFO_SIZE_8, &usbdevregs->MAXP_REG); //EP0 max packit size = 8 
	writeb((EP0_SERVICED_OUT_PKT_RDY | EP0_SERVICED_SETUP_END), & usbdevregs->EP0_CSR_IN_CSR1_REG); //EP0:clear OUT_PKT_RDY & SETUP_END
	writeb(1, &usbdevregs->INDEX_REG); 

#if (EP1_PKT_SIZE==32)
	writeb(FIFO_SIZE_32, &usbdevregs->MAXP_REG); //EP1:max packit size = 32
#else
	writeb(FIFO_SIZE_64, &usbdevregs->MAXP_REG); 	//EP1:max packit size = 64
#endif	
	writeb((EPI_FIFO_FLUSH | EPI_CDT), &usbdevregs->EP0_CSR_IN_CSR1_REG); 
	writeb((EPI_MODE_IN | EPI_IN_DMA_INT_MASK | EPI_BULK), &usbdevregs->IN_CSR2_REG);  //IN mode, IN_DMA_INT=masked    
	writeb(EPO_CDT, &usbdevregs->OUT_CSR1_REG); 
	writeb((EPO_BULK | EPO_OUT_DMA_INT_MASK), &usbdevregs->OUT_CSR2_REG); 

	writeb(2, &usbdevregs->INDEX_REG); 
	writeb(FIFO_SIZE_64, &usbdevregs->MAXP_REG); //EP2:max packit size = 64
	writeb((EPI_FIFO_FLUSH | EPI_CDT | EPI_BULK), &usbdevregs->EP0_CSR_IN_CSR1_REG); 
	writeb((EPI_MODE_IN | EPI_IN_DMA_INT_MASK), &usbdevregs->IN_CSR2_REG);  //IN mode, IN_DMA_INT=masked    
	writeb(EPO_CDT, &usbdevregs->OUT_CSR1_REG); 
	writeb((EPO_BULK | EPO_OUT_DMA_INT_MASK), &usbdevregs->OUT_CSR2_REG); 

	writeb(3, &usbdevregs->INDEX_REG); 
    #if (EP3_PKT_SIZE==32)
	writeb(FIFO_SIZE_32, &usbdevregs->MAXP_REG); //EP3:max packit size = 32
    #else
	writeb(FIFO_SIZE_64, &usbdevregs->MAXP_REG); //EP3:max packit size = 64
    #endif
	writeb((EPI_FIFO_FLUSH | EPI_CDT | EPI_BULK), &usbdevregs->EP0_CSR_IN_CSR1_REG); 	
	writeb((EPI_MODE_OUT | EPI_IN_DMA_INT_MASK), &usbdevregs->IN_CSR2_REG); //OUT mode, IN_DMA_INT=masked    
	writeb(EPO_CDT, &usbdevregs->OUT_CSR1_REG); 
    	//clear OUT_PKT_RDY, data_toggle_bit.
	//The data toggle bit should be cleared when initialization.
	writeb((EPO_BULK | EPO_OUT_DMA_INT_MASK), &usbdevregs->OUT_CSR2_REG); 

	writeb(4, &usbdevregs->INDEX_REG); 
	writeb(FIFO_SIZE_64, &usbdevregs->MAXP_REG); //EP4:max packit size = 64
	writeb((EPI_FIFO_FLUSH | EPI_CDT | EPI_BULK), &usbdevregs->EP0_CSR_IN_CSR1_REG); 
	writeb((EPI_MODE_OUT | EPI_IN_DMA_INT_MASK), &usbdevregs->IN_CSR2_REG); //OUT mode, IN_DMA_INT=masked    
	writeb(EPO_CDT, &usbdevregs->OUT_CSR1_REG); 
    	//clear OUT_PKT_RDY, data_toggle_bit.
	//The data toggle bit should be cleared when initialization.
	writeb((EPO_BULK | EPO_OUT_DMA_INT_MASK), &usbdevregs->OUT_CSR2_REG); 
    
	writeb((EP0_INT | EP1_INT | EP2_INT | EP3_INT | EP4_INT), &usbdevregs->EP_INT_REG); 
	writeb((RESET_INT | SUSPEND_INT | RESUME_INT), &usbdevregs->USB_INT_REG); 
    	//Clear all usbd pending bits
    	
	writeb((EP0_INT | EP1_INT | EP3_INT), &usbdevregs->EP_INT_EN_REG); 
	writeb(RESET_INT, &usbdevregs->USB_INT_EN_REG); 
    ep0State = EP0_STATE_INIT;
    
}


void RdPktEp0(U8 *buf,int num)
{
    int i;
	struct s3c24x0_usb_device * const usbdevregs	= s3c24x0_get_base_usb_device();   
 	
    for(i=0;i<num;i++)
    {
	 buf[i] = readb(&usbdevregs->fifo[0].EP_FIFO_REG);
    }
}
    

void WrPktEp0(U8 *buf,int num)
{
    int i;
	struct s3c24x0_usb_device * const usbdevregs	= s3c24x0_get_base_usb_device();      	
    for(i=0;i<num;i++)
    {
	writeb(buf[i], &usbdevregs->fifo[0].EP_FIFO_REG); 
    }
}


void WrPktEp1(U8 *buf,int num)
{
    int i;
	struct s3c24x0_usb_device * const usbdevregs	= s3c24x0_get_base_usb_device();      	
    for(i=0;i<num;i++)
    {
	writeb(buf[i], &usbdevregs->fifo[1].EP_FIFO_REG); 
    }
}


void WrPktEp2(U8 *buf,int num)
{
    int i;
	struct s3c24x0_usb_device * const usbdevregs	= s3c24x0_get_base_usb_device();      	
    for(i=0;i<num;i++)
    {
	writeb(buf[i], &usbdevregs->fifo[2].EP_FIFO_REG); 
    }
}


void RdPktEp3(U8 *buf,int num)
{
    int i;
	struct s3c24x0_usb_device * const usbdevregs	= s3c24x0_get_base_usb_device();      	
    for(i=0;i<num;i++)
    {
	 buf[i] = readb(&usbdevregs->fifo[3].EP_FIFO_REG);
    }
}


void RdPktEp4(U8 *buf,int num)
{
    int i;
	struct s3c24x0_usb_device * const usbdevregs	= s3c24x0_get_base_usb_device();      	
    for(i=0;i<num;i++)
    {
	 buf[i] = readb(&usbdevregs->fifo[4].EP_FIFO_REG);
    }
}


void ConfigEp3DmaMode(U32 bufAddr,U32 count)
{
	char j;
	int i;
	struct s3c24x0_usb_device * const usbdevregs = s3c24x0_get_base_usb_device();  
	struct s3c24x0_dmas * const dmaregs = s3c24x0_get_base_dmas();

	writeb(3, &usbdevregs->INDEX_REG); 
	count &= 0xfffff; //transfer size should be <1MB
	writel(((1<<1) | (1<<0)), &dmaregs->dma[2].DISRCC);     
	writel(ADDR_EP3_FIFO, &dmaregs->dma[2].DISRC);  //src=APB,fixed,src=EP3_FIFO
	writel(((0<<1) | (0<<0)), &dmaregs->dma[2].DIDSTC); 
	writel(bufAddr, &dmaregs->dma[2].DIDST); //dst=AHB,increase,dst=bufAddr
#if USBDMA_DEMAND
	writel(((count)|(0<<31)|(0<<30)|(1<<29)|(0<<28)|(0<<27)|(4<<24)|(1<<23)|(0<<22)|(0<<20)), &dmaregs->dma[2].DCON); 
        //demand,requestor=APB,CURR_TC int enable,unit transfer,
        //single service,src=USBD,H/W request,autoreload,byte,CURR_TC
#else
    /* changed by thisway.diy to disable autoreload */
	writel(((count)|(1<<31)|(0<<30)|(1<<29)|(0<<28)|(0<<27)|(4<<24)|(1<<23)|(1<<22)|(0<<20)), &dmaregs->dma[2].DCON); 
        //handshake,requestor=APB,CURR_TC int enable,unit transfer,
        //single service,src=USBD,H/W request,autoreload,byte,CURR_TC
#endif
	writel((1<<1), &dmaregs->dma[2].DMASKTRIG);         
        //DMA 2 on

    //rEP3_DMA_FIFO=0x40; //not needed for OUT operation. 	
	writeb(0xff, &usbdevregs->ep3.EP_DMA_TTC_L); 
	writeb(0xff, &usbdevregs->ep3.EP_DMA_TTC_M); 
	writeb(0x0f, &usbdevregs->ep3.EP_DMA_TTC_H); 

	writeb((readb(&usbdevregs->OUT_CSR2_REG) | EPO_AUTO_CLR | EPO_OUT_DMA_INT_MASK), &usbdevregs->OUT_CSR2_REG); 
    	//AUTO_CLR(OUT_PKT_READY is cleared automatically), interrupt_masking.
#if USBDMA_DEMAND
	writeb(EP3_PKT_SIZE, &usbdevregs->ep3.EP_DMA_TTC_H); 
	writeb((UDMA_DEMAND_MODE | UDMA_OUT_DMA_RUN | UDMA_DMA_MODE_EN), &usbdevregs->ep3.EP_DMA_CON); 
        // deamnd enable,out_dma_run=run,in_dma_run=stop,DMA mode enable
#else
	writeb(0x01, &usbdevregs->ep3.EP_DMA_UNIT);         
	writeb((UDMA_OUT_DMA_RUN | UDMA_DMA_MODE_EN), &usbdevregs->ep3.EP_DMA_CON); 
        // deamnd disable,out_dma_run=run,in_dma_run=stop,DMA mode enable
#endif  
    //wait until DMA_CON is effective.
	j = readb(&usbdevregs->ep3.EP_DMA_CON);
	for(i=0;i<10;i++);    	

    /* add by thisway.diy for non-autoreload */
	writel((1<<1), &dmaregs->dma[3].DMASKTRIG); 
}


void ConfigEp3IntMode(void)
{
	char i;	
	struct s3c24x0_usb_device * const usbdevregs = s3c24x0_get_base_usb_device(); 
	struct s3c24x0_dmas * const dmaregs = s3c24x0_get_base_dmas(); 

	writeb(3, &usbdevregs->INDEX_REG); 
 	writel((0<<1), &dmaregs->dma[2].DMASKTRIG);    

        //DMA channel off
	writeb((readb(&usbdevregs->OUT_CSR2_REG) & ~(EPO_AUTO_CLR)), &usbdevregs->OUT_CSR2_REG); 
    	//AUTOCLEAR off,interrupt_enabled (???)
	writeb(1, &usbdevregs->ep3.EP_DMA_UNIT); 	
	writeb(0, &usbdevregs->ep3.EP_DMA_CON); 
    	// deamnd disable,out_dma_run=stop,in_dma_run=stop,DMA mode disable
	//wait until DMA_CON is effective.
	i = readb(&usbdevregs->ep3.EP_DMA_CON);
    
}
