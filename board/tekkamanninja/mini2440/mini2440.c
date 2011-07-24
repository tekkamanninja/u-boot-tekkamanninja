/*
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * (C) Copyright 2002
 * David Mueller, ELSOFT AG, <d.mueller@elsoft.ch>
 *
 * (C) Copyright 2005
 * JinHua Luo, GuangDong Linux Center, <luo.jinhua@gd-linux.com>
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
#include <netdev.h>
#include <asm/arch/s3c24x0_cpu.h>
#include <video_fb.h>

#if defined(CONFIG_CMD_NAND)
#include <linux/mtd/nand.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

#define FCLK_SPEED 1

#if FCLK_SPEED==0		/* Fout = 203MHz, Fin = 12MHz for Audio */
#define M_MDIV	0xC3
#define M_PDIV	0x4
#define M_SDIV	0x1
#elif FCLK_SPEED==1		/* Fout = 202.8MHz */

#if defined(CONFIG_S3C2410)
/* Fout = 202.8MHz */
#define M_MDIV	0xA1
#define M_PDIV	0x3
#define M_SDIV	0x1
#endif

#if defined(CONFIG_S3C2440)
/* Fout = 405MHz */
#define M_MDIV 0x7f	
#define M_PDIV 0x2
#define M_SDIV 0x1
#endif
#endif

#define USB_CLOCK 1

#if USB_CLOCK==0
#define U_M_MDIV	0xA1
#define U_M_PDIV	0x3
#define U_M_SDIV	0x1
#elif USB_CLOCK==1

#if defined(CONFIG_S3C2410)
#define U_M_MDIV	0x48
#define U_M_PDIV	0x3
#endif

#if defined(CONFIG_S3C2440)
#define U_M_MDIV 0x38
#define U_M_PDIV 0x2
#endif

#define U_M_SDIV	0x2
#endif

static inline void delay (unsigned long loops)
{
	__asm__ volatile ("1:\n"
			  "subs %0, %1, #1\n"
			  "bne 1b":"=r" (loops):"0" (loops));
}

/*
 * Miscellaneous platform dependent initialisations
 */

int board_init (void)
{
	struct s3c24x0_clock_power * const clk_power =
					s3c24x0_get_base_clock_power();
	struct s3c24x0_gpio * const gpio = s3c24x0_get_base_gpio();

	/* to reduce PLL lock time, adjust the LOCKTIME register */
	clk_power->LOCKTIME = 0xFFFFFF;

	/* configure MPLL */
	clk_power->MPLLCON = ((M_MDIV << 12) + (M_PDIV << 4) + M_SDIV);

	/* some delay between MPLL and UPLL */
	delay (4000);

	/* configure UPLL */
	clk_power->UPLLCON = ((U_M_MDIV << 12) + (U_M_PDIV << 4) + U_M_SDIV);

	/* some delay between MPLL and UPLL */
	delay (8000);

	/* set up the I/O ports */
	gpio->GPACON = 0x007FFFFF;

#if defined(CONFIG_MINI2440) 
	gpio->GPBCON = 0x00295551;
#else
	gpio->GPBCON = 0x00044556;
#endif

	gpio->GPBUP = 0x000007FF;

#if defined(CONFIG_MINI2440) 
	gpio->GPCCON = 0xAAAAA6AA;
	gpio->GPCDAT &= ~(1<<5);
#else
	gpio->GPCCON = 0xAAAAAAAA;
#endif
	gpio->GPCUP = 0xFFFFFFFF;
	gpio->GPDCON = 0xAAAAAAAA;
	gpio->GPDUP = 0xFFFFFFFF;

    gpio->GPECON = 0xAAAAAAAA;
	gpio->GPEUP = 0x0000FFFF;
	gpio->GPFCON = 0x000055AA;
	gpio->GPFUP = 0x000000FF;
	gpio->GPGCON = 0xFF95FF3A;
	gpio->GPGUP = 0x0000FFFF;
	gpio->GPHCON = 0x0016FAAA;
	gpio->GPHUP = 0x000007FF;

	gpio->EXTINT0=0x22222222;
	gpio->EXTINT1=0x22222222;
	gpio->EXTINT2=0x22222222;

#if defined(CONFIG_S3C2410)
	/* arch number of SMDK2410-Board */
	gd->bd->bi_arch_number = MACH_TYPE_SMDK2410;
#endif

#if defined(CONFIG_S3C2440)
/* arch number of S3C2440-Board */
	gd->bd->bi_arch_number = MACH_TYPE_MINI2440 ;
#endif


	/* adress of boot parameters */
	gd->bd->bi_boot_params = 0x30000100;

	icache_enable();
	dcache_enable();
#if	defined(CONFIG_MINI2440_LED)
	gpio->GPBDAT = 0x00000181;
#endif
	return 0;
}



#define MVAL		(0)
#define MVAL_USED 	(0)		//0=each frame   1=rate by MVAL
#define INVVDEN		(1)		//0=normal       1=inverted
#define BSWP		(0)		//Byte swap control
#define HWSWP		(1)		//Half word swap control


//TFT 240320
#define LCD_XSIZE_TFT_240320 	(240)	
#define LCD_YSIZE_TFT_240320 	(320)

//TFT240320
#define HOZVAL_TFT_240320	(LCD_XSIZE_TFT_240320-1)
#define LINEVAL_TFT_240320	(LCD_YSIZE_TFT_240320-1)

//Timing parameter for NEC3.5"
#define VBPD_240320		(3)		
#define VFPD_240320		(10)
#define VSPW_240320		(1)

#define HBPD_240320		(5)
#define HFPD_240320		(2)
#define HSPW_240320_NEC		(36)  //Adjust the horizontal displacement of the screen :tekkamanninja@163.com
#define HSPW_240320_TD		(23)  //64MB nand mini2440 is 36 ,128MB is 23
				      //+ ： -->    - : <--
#define CLKVAL_TFT_240320	(3) 	
//FCLK=101.25MHz,HCLK=50.625MHz,VCLK=6.33MHz


void board_video_init(GraphicDevice *pGD) 
{ 
	struct s3c24x0_lcd * const lcd	 = s3c24x0_get_base_lcd(); 
	struct s3c2410_nand * const nand = s3c2410_get_base_nand();
    /* FIXME: select LCM type by env variable */ 
	 
	/* Configuration for GTA01 LCM on QT2410 */ 
	lcd->LCDCON1 = 0x00000378; /* CLKVAL=4, BPPMODE=16bpp, TFT, ENVID=0 */ 
	lcd->LCDCON2 = (VBPD_240320<<24)|(LINEVAL_TFT_240320<<14)|(VFPD_240320<<6)|(VSPW_240320); 
	lcd->LCDCON3 = (HBPD_240320<<19)|(HOZVAL_TFT_240320<<8)|(HFPD_240320); 

	if ( (nand->NFCONF) & 0x08 )	{ 
	lcd->LCDCON4 = (MVAL<<8)|(HSPW_240320_TD);
	}
	else	{
	  lcd->LCDCON4 = (MVAL<<8)|(HSPW_240320_NEC);
	}
	
	lcd->LCDCON5 = 0x00000f09; 
	lcd->LPCSEL  = 0x00000000; 
} 


int dram_init (void)
{
	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = PHYS_SDRAM_1_SIZE;

	return 0;
}

#if 0
#if defined(CONFIG_CMD_NAND)
extern ulong nand_probe(ulong physadr);

static inline void NF_Reset(void)
{
	int i;

	NF_SetCE(NFCE_LOW);
	NF_Cmd(0xFF);		/* reset command */
	for(i = 0; i < 10; i++);	/* tWB = 100ns. */
	NF_WaitRB();		/* wait 200~500us; */
	NF_SetCE(NFCE_HIGH);
}

static inline void NF_Init(void)
{
#if 1
#define TACLS   0
#define TWRPH0  3
#define TWRPH1  0
#else
#define TACLS   0
#define TWRPH0  4
#define TWRPH1  2
#endif

	NF_Conf((1<<15)|(0<<14)|(0<<13)|(1<<12)|(1<<11)|(TACLS<<8)|(TWRPH0<<4)|(TWRPH1<<0));
	/*nand->NFCONF = (1<<15)|(1<<14)|(1<<13)|(1<<12)|(1<<11)|(TACLS<<8)|(TWRPH0<<4)|(TWRPH1<<0); */
	/* 1  1    1     1,   1      xxx,  r xxx,   r xxx */
	/* En 512B 4step ECCR nFCE=H tACLS   tWRPH0   tWRPH1 */

	NF_Reset();
}

void nand_init(void)
{
	struct s3c2410_nand * const nand = s3c2410_get_base_nand();

	NF_Init();
#ifdef DEBUG
	printf("NAND flash probing at 0x%.8lX\n", (ulong)nand);
#endif
	printf ("%4lu MB\n", nand_probe((ulong)nand) >> 20);
}
#endif
#endif

#ifdef CONFIG_CMD_NET
int board_eth_init(bd_t *bis)
{
	int rc = 0;
#ifdef CONFIG_CS8900
	rc = cs8900_initialize(0, CONFIG_CS8900_BASE);
#endif
#ifdef CONFIG_DRIVER_DM9000
	rc = dm9000_initialize(bis);
#endif
	return rc;
}
#endif
