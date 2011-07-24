/****************************************************************
 NAME: usbin.c
 DESC: usb bulk-IN operation
 HISTORY:
 Mar.25.2002:purnnamu: ported for S3C2410X.
 ****************************************************************/
#include <common.h>
#include <asm/arch/s3c24x0_cpu.h>
#include <asm/io.h>

#include "def.h"

#include "2440usb.h"
#include "usbmain.h"
#include "usb.h"
#include "usblib.h"
#include "usbsetup.h"
#include "usbin.h"

//static void PrintEpiPkt(U8 *pt,int cnt);


// ===================================================================
// All following commands will operate in case 
// - in_csr1 is valid.
// ===================================================================

//#define SET_EP1_IN_PKT_READY()  usbdevregs->EP0_CSR_IN_CSR1_REG= ((in_csr1 & (~EPI_WR_BITS)) | EPI_IN_PKT_READY )
#define SET_EP1_IN_PKT_READY()  writeb(((in_csr1 & (~EPI_WR_BITS)) | EPI_IN_PKT_READY ), &usbdevregs->EP0_CSR_IN_CSR1_REG);	 
// #define SET_EP1_SEND_STALL()	usbdevregs->EP0_CSR_IN_CSR1_REG= ((in_csr1 & (~EPI_WR_BITS)) | EPI_SEND_STALL )
//#define CLR_EP1_SENT_STALL()	usbdevregs->EP0_CSR_IN_CSR1_REG= ((in_csr1 & (~EPI_WR_BITS)) & (~EPI_SENT_STALL) )
#define CLR_EP1_SENT_STALL()	writeb(((in_csr1 & (~EPI_WR_BITS)) & (~EPI_SENT_STALL) ), &usbdevregs->EP0_CSR_IN_CSR1_REG);
// #define FLUSH_EP1_FIFO() 	usbdevregs->EP0_CSR_IN_CSR1_REG= ((in_csr1 & (~EPI_WR_BITS)) | EPI_FIFO_FLUSH )


// ***************************
// *** VERY IMPORTANT NOTE ***
// ***************************
// Prepare the code for the packit size constraint!!!

// EP1 = IN end point. 

U8 ep1Buf[EP1_PKT_SIZE];
int transferIndex=0;

void PrepareEp1Fifo(void) 
{
	struct s3c24x0_usb_device * const usbdevregs = s3c24x0_get_base_usb_device();
	int i;
	U8 in_csr1;
	
	writeb(1,&usbdevregs->INDEX_REG);
//	usbdevregs->INDEX_REG=1;
	in_csr1 = readb(&usbdevregs->EP0_CSR_IN_CSR1_REG);
//	in_csr1=usbdevregs->EP0_CSR_IN_CSR1_REG;
    
	for(i=0;i<EP1_PKT_SIZE;i++) {
		ep1Buf[i]=(U8)(transferIndex+i);
	}
	
	WrPktEp1(ep1Buf,EP1_PKT_SIZE);
	SET_EP1_IN_PKT_READY(); 
}


void Ep1Handler(void)
{
	U8 in_csr1;
	struct s3c24x0_usb_device * const usbdevregs = s3c24x0_get_base_usb_device();
	writeb(1,&usbdevregs->INDEX_REG);
//	usbdevregs->INDEX_REG=1;
	in_csr1 = readb(&usbdevregs->EP0_CSR_IN_CSR1_REG);
//	in_csr1=usbdevregs->EP0_CSR_IN_CSR1_REG;
    
//	DbgPrintf("<1:%x]",in_csr1);

	//I think that EPI_SENT_STALL will not be set to 1.
	if(in_csr1 & EPI_SENT_STALL)
	{   
//		DbgPrintf("[STALL]");
		CLR_EP1_SENT_STALL();
   		return;
	}	

	//IN_PKT_READY is cleared
	//The data transfered was ep1Buf[] which was already configured 

//	PrintEpiPkt(ep1Buf,EP1_PKT_SIZE); 
    
	transferIndex++;

	PrepareEp1Fifo(); 
    	//IN_PKT_READY is set   
    	//This packit will be used for next IN packit.	

    return;
}


#if 0    
void PrintEpiPkt(U8 *pt,int cnt)
{
	int i;
	DbgPrintf("[B_IN:%d:",cnt);
	for(i=0;i<cnt;i++) {
		DbgPrintf("%x,",pt[i]);
	}
	DbgPrintf("]");
}
#endif
