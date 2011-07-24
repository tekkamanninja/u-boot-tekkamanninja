/****************************************************************
 NAME: usbmain.c
 DESC: endpoint interrupt handler
       USB init jobs
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
#include "usbout.h"
#include "usbin.h"

#define BIT_USBD		(0x1<<25)
extern void ClearPending_my(int bit); 
    
/**************************
    Some PrepareEp1Fifo() should be deleted
 **************************/   

void UsbdMain(void)
{
    //ChangeUPllValue(0x38,2,1);	// UCLK=96Mhz     
    //ChangeUPllValue(0x38,2,2);	// UCLK=48Mhz     
    InitDescriptorTable();
    //ResetUsbd();
    
    ConfigUsbd(); 

    PrepareEp1Fifo(); 
#if 0    
    while(1)
    {
    	if(DbgPrintfLoop())continue;
    	
    	Delay(5000);
    	if((i++%2)==0)Led_Display(0x8);
    	else Led_Display(0x0);
    }
#endif    
}


void IsrUsbd(void)
{
	struct s3c24x0_usb_device * const usbdevregs	= s3c24x0_get_base_usb_device();
    U8 usbdIntpnd,epIntpnd;
	U8 saveIndexReg = readb(&usbdevregs->INDEX_REG);
	usbdIntpnd = readb(&usbdevregs->USB_INT_REG);
	epIntpnd = readb(&usbdevregs->EP_INT_REG);
//    DbgPrintf( "[INT:EP_I=%x,USBI=%x]",epIntpnd,usbdIntpnd );

    if(usbdIntpnd&SUSPEND_INT)
    {
	writeb(SUSPEND_INT, &usbdevregs->USB_INT_REG);
//    	DbgPrintf( "<SUS]\n");
    }
    if(usbdIntpnd&RESUME_INT)
    {
	writeb(RESUME_INT, &usbdevregs->USB_INT_REG);
//    	DbgPrintf("<RSM]\n");
    }
    if(usbdIntpnd&RESET_INT)
    {
//    	DbgPrintf( "<RST] ReconfigUsbd\n");  
    	
    	//ResetUsbd();
    	ReconfigUsbd();
	writeb(RESET_INT, &usbdevregs->USB_INT_REG); //RESET_INT should be cleared after ResetUsbd().   	
        PrepareEp1Fifo(); 
    }

    if(epIntpnd&EP0_INT)
    {
	writeb(EP0_INT, &usbdevregs->EP_INT_REG);
    	Ep0Handler();
    }
    if(epIntpnd&EP1_INT)
    {
	writeb(EP1_INT, &usbdevregs->EP_INT_REG);
    	Ep1Handler();
    }

    if(epIntpnd&EP2_INT)
    {
	writeb(EP2_INT, &usbdevregs->EP_INT_REG);
 //   	DbgPrintf("<2:TBD]\n");   //not implemented yet	
    	//Ep2Handler();
    }

    if(epIntpnd&EP3_INT)
    {
	writeb(EP3_INT, &usbdevregs->EP_INT_REG);
    	Ep3Handler();
    }

    if(epIntpnd&EP4_INT)
    {
	writeb(EP4_INT, &usbdevregs->EP_INT_REG);
 //   	DbgPrintf("<4:TBD]\n");   //not implemented yet	
    	//Ep4Handler();
    }

    ClearPending_my((int)BIT_USBD);	 
	writeb(saveIndexReg, &usbdevregs->INDEX_REG);    
}




/******************* Consol printf for debug *********************/

#define DBGSTR_LENGTH (0x1000)
U8 dbgStrFifo[DBGSTR_LENGTH];
volatile U32 dbgStrRdPt=0;
volatile U32 dbgStrWrPt=0;



void _WrDbgStrFifo(U8 c)
{
    dbgStrFifo[dbgStrWrPt++]=c;
    if(dbgStrWrPt==DBGSTR_LENGTH)dbgStrWrPt=0;

}


#if 0
void DbgPrintf(char *fmt,...)
{
    int i,slen;
    va_list ap;
    char string[256];

    va_start(ap,fmt);
    vsprintf(string,fmt,ap);
    
//    slen=strlen(string);
    
//    for(i=0;i<slen;i++)
//    	_WrDbgStrFifo(string[i]);
    
    va_end(ap);
    puts(string);
}
#else
void DbgPrintf(char *fmt,...)
{
}
#endif


