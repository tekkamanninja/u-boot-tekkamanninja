/****************************************************************
 NAME: u2440mon.c
 DESC: u2440mon entry point,menu,download
 HISTORY:
 Mar.25.2002:purnnamu: S3C2400X profile.c is ported for S3C2410X.
 Mar.27.2002:purnnamu: DMA is enabled.
 Apr.01.2002:purnnamu: isDownloadReady flag is added.
 Apr.10.2002:purnnamu: - Selecting menu is available in the waiting loop. 
                         So, isDownloadReady flag gets not needed
                       - UART ch.1 can be selected for the console.
 Aug.20.2002:purnnamu: revision number change 0.2 -> R1.1       
 Sep.03.2002:purnnamu: To remove the power noise in the USB signal, the unused CLKOUT0,1 is disabled.
 ****************************************************************/
#include <common.h>
#include <asm/arch/s3c24x0_cpu.h>
#include <asm/io.h>

#include "usbmain.h"
#include "usbout.h"
#include "usblib.h"
#include "2440usb.h"

#define BIT_DMA2		(0x1<<19)

void Isr_Init(void);
void HaltUndef(void);
void HaltSwi(void);
void HaltPabort(void);
void HaltDabort(void);
void Lcd_Off(void);
__u32 usb_receive(char *buf, size_t len, U32 wait);
void Menu(void);

extern void Timer_InitEx(void);
extern void Timer_StartEx(void);
extern unsigned int Timer_StopEx(void);

extern void (*isr_handle_array[])(void);

/*
 * Reads and returns a character from the serial port
 *   - Times out after delay iterations checking for presence of character
 *   - Sets *error_p to UART error bits or - on timeout
 *   - On timeout, sets *error_p to -1 and returns 0
 */
char awaitkey(unsigned long delay, int* error_p)
{
    int i;
//    char c;

    if (delay == -1) {
        while (1) {
            if (tstc()) /* we got a key press	*/
                return getc();
        }
    }
    else {        
        for (i = 0; i < delay; i++) {
    		if (tstc()) /* we got a key press	*/
    			return getc();
            udelay (10*1000);
        }
    }
    
    if (error_p)
        *error_p = -1;
    return 0;
}

#define CTRL(x)   (x & 0x1f)
#define INTR      CTRL('C')

void Clk0_Enable(int clock_sel);	
void Clk1_Enable(int clock_sel);
void Clk0_Disable(void);
void Clk1_Disable(void);

//#define DOWNLOAD_ADDRESS _RAM_STARTADDRESS
volatile U32 downloadAddress;

void (*restart)(void)=(void (*)(void))0x0;
void (*run)(void);


volatile unsigned char *downPt;
volatile U32 downloadFileSize;
volatile U16 checkSum;
volatile unsigned int err=0;
volatile U32 totalDmaCount;

volatile int isUsbdSetConfiguration;

int download_run=0;
volatile U32 tempDownloadAddress;
int menuUsed=0;

volatile U32 dwUSBBufReadPtr;
volatile U32 dwUSBBufWritePtr;
volatile U32 dwWillDMACnt;
volatile U32 bDMAPending;
volatile U32 dwUSBBufBase;
volatile U32 dwUSBBufSize;

int consoleNum;

void usb_init_slave(void)
{
	struct s3c24x0_gpio * const gpioregs = s3c24x0_get_base_gpio();
	char *mode;

	udelay(100000);

	Isr_Init();

   writel((readl(&gpioregs->MISCCR) & ~((1<<3) | (1<<13))), &gpioregs->MISCCR); 
// USBD is selected instead of USBH1 
// USB port 1 is enabled.
//
//  USBD should be initialized first of all.
//
	isUsbdSetConfiguration=0;

	UsbdMain(); 
	udelay(100000);
    
    writel((readl(&gpioregs->GPCDAT) | (1<<5)), &gpioregs->GPCDAT); 
 /* enable USB Device, thisway.diy */
#if USBDMA
	mode="DMA";
#else
	mode="Int";
#endif
	download_run=0; //The default menu is the Download & Run mode.
	printf("USB slave is enable!\n");
}


__u32 usb_receive(char *buf, size_t len, U32 wait)
{
	int first=1;
	U8 tempMem[16];
	U32 j;
	unsigned int dwRecvTimeSec = 0;
	char c;
	struct s3c24x0_interrupt * intregs = s3c24x0_get_base_interrupt();

	dwUSBBufReadPtr = dwUSBBufBase; // USB_BUF_BASE; thiswa.diy, 2006.06.21
	dwUSBBufWritePtr = dwUSBBufBase; // USB_BUF_BASE; thiswa.diy, 2006.06.21
	bDMAPending = 0;

	/* add by thisway.diy */
	tempDownloadAddress = dwUSBBufBase; // USB_BUF_BASE; thiswa.diy, 2006.06.21 // RAM_BASE, changed by thisway.diy for wince, 2006.06.18

	downloadAddress=(U32)tempMem; //_RAM_STARTADDRESS; 
	downPt=(unsigned char *)downloadAddress;
	//This address is used for receiving first 8 byte.
	downloadFileSize=0;
    

    /*******************************/
    /*    File download    */
    /*******************************/
    if(isUsbdSetConfiguration==0)
    {
	    printf("USB host is not connected yet.\n");
    }

    while(downloadFileSize==0) /* wait until send a file */
    {
        if(first==1 && isUsbdSetConfiguration!=0)
        {
            printf("USB host is connected. Waiting a download.\n");
            first=0;
        }
		c = awaitkey(1, 0);
		if ((c & 0x7f) == INTR)
		{
			printf("Cancelled by user\n");
			return 0;
		}
    }

	printf("get downloadFileSize = %d !!\n",downloadFileSize);

    /* add by thisway.diy */
    if (downloadFileSize - 10 > len)
    {
        printf("Length of file is too big : %d > %d\n", downloadFileSize - 10, len);
        return 0;
    }
    
    Timer_InitEx();
    Timer_StartEx();
        
#if USBDMA    

    writel((readl(&intregs->INTMSK) & ~(BIT_DMA2)), &intregs->INTMSK); 
//    intregs->INTMSK&=~(BIT_DMA2);  

    ClearEp3OutPktReady(); 
    	// indicate the first packit is processed.
    	// has been delayed for DMA2 cofiguration.

    if(downloadFileSize>EP3_PKT_SIZE)
    {
        if(downloadFileSize - EP3_PKT_SIZE<=(0x80000))
        {
            /* set the source and length */
            dwUSBBufWritePtr = downloadAddress + EP3_PKT_SIZE-8;
            dwWillDMACnt = downloadFileSize - EP3_PKT_SIZE;
	    }
      	else
      	{
            dwUSBBufWritePtr = downloadAddress + EP3_PKT_SIZE - 8;
            // dwWillDMACnt = 0x80000 - EP3_PKT_SIZE;
            
            /* Changed by thisway.diy, 2006.06.22
             * We want When the first DMA interrupt happened, 
             * it has received (0x80000 + 8) bytes data from PC
             * The format of data PC send out is: <ADDR(DATA):4>+<SIZE(n+10):4>+<DATA:n>+<CS:2>
             * So, the first 8 bytes isn't the real data we want
             * We want the dwUSBBufWritePtr is always 0x80000 aligin
             */
            dwWillDMACnt = 0x80000 + 8 - EP3_PKT_SIZE;
    	}
     	totalDmaCount = 0;
  	    ConfigEp3DmaMode(dwUSBBufWritePtr, dwWillDMACnt);
    }
    else
    {
        dwUSBBufWritePtr = downloadAddress + downloadFileSize - 8;
	    totalDmaCount = downloadFileSize;
    }
#endif

    printf("\nNow, Downloading [ADDRESS:%xh,TOTAL:%d]\n",
    		downloadAddress,downloadFileSize);

    if (wait)
    {
        printf("RECEIVED FILE SIZE:%8d",0);

        j = totalDmaCount + 0x10000;
        while (totalDmaCount != downloadFileSize)
        {
            if (totalDmaCount > j)
            {
        	    printf("\b\b\b\b\b\b\b\b%8d", j);
                j = totalDmaCount + 0x10000;
            }
        }
	    printf("\b\b\b\b\b\b\b\b%8d ", totalDmaCount);
        dwRecvTimeSec = Timer_StopEx();
        if (dwRecvTimeSec == 0)
        {
            dwRecvTimeSec = 1;
        }
        printf("(%dKB/S, %dS)\n", (downloadFileSize/dwRecvTimeSec/1024), dwRecvTimeSec);
    }

    return downloadFileSize - 10;
}


void HaltUndef(void)
{
	printf("Undefined instruction exception!!!\n");
	while(1);
}

void HaltSwi(void)
{
	printf("SWI exception!!!\n");
	while(1);
}

void HaltPabort(void)
{
	printf("Pabort exception!!!\n");
	while(1);
}

void HaltDabort(void)
{
	printf("Dabort exception!!!\n");
	while(1);
}

#if 0
void Clk0_Enable(int clock_sel)	
{	// 0:MPLLin, 1:UPLL, 2:FCLK, 3:HCLK, 4:PCLK, 5:DCLK0
	struct s3c24x0_gpio 		* const  gpioregs	= s3c24x0_get_base_gpio();
	gpioregs->MISCCR = gpioregs->MISCCR&~(7<<4) | (clock_sel<<4);
	gpioregs->GPHCON = gpioregs->GPHCON&~(3<<18) | (2<<18);
}
void Clk1_Enable(int clock_sel)
{	// 0:MPLLout, 1:UPLL, 2:RTC, 3:HCLK, 4:PCLK, 5:DCLK1	
	struct s3c24x0_gpio 		* const  gpioregs	= s3c24x0_get_base_gpio();
	gpioregs->MISCCR = gpioregs->MISCCR&~(7<<8) | (clock_sel<<8);
	gpioregs->GPHCON = gpioregs->GPHCON&~(3<<20) | (2<<20);
}
void Clk0_Disable(void)
{
	struct s3c24x0_gpio 		* const  gpioregs	= s3c24x0_get_base_gpio();
	gpioregs->GPHCON = gpioregs->GPHCON&~(3<<18);	// GPH9 Input
}
void Clk1_Disable(void)
{
	struct s3c24x0_gpio 		* const  gpioregs	= s3c24x0_get_base_gpio();
	gpioregs->GPHCON = gpioregs->GPHCON&~(3<<20);	// GPH10 Input
}
#endif

void udc_disconnect (void)
{
	struct s3c24x0_gpio * const  gpioregs = s3c24x0_get_base_gpio();
	writel((readl(&gpioregs->GPCDAT) & ~(1<<5)), &gpioregs->GPCDAT); 
//	UDCDBG ("disconnect, disable Pullup");

}
