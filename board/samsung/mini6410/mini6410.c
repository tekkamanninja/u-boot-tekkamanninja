/*
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger at sysgo.de>
 *
 * (C) Copyright 2002
 * David Mueller, ELSOFT AG, <d.mueller at elsoft.ch>
 *
 * (C) Copyright 2008
 * Guennadi Liakhovetki, DENX Software Engineering, <lg at denx.de>
 *
 * (C) Copyright 2010
 * Alex Ling, <kasimling at gmail.com>
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
#include <net.h>
#include <netdev.h>
#include <video_fb.h>
#include <asm/arch/s3c6400.h>
#include <asm/arch/regs-fb-v4.h>
#include <asm/arch/mmc.h>



DECLARE_GLOBAL_DATA_PTR;

static inline void delay(unsigned long loops)
{
	__asm__ volatile ("1:\n" "subs %0, %1, #1\n"
			  "bne 1b"
			  : "=r" (loops) : "0" (loops));
}

/*
 * Miscellaneous platform dependent initialisations
 */

int board_init(void)
{
	s3c64xx_gpio * const gpio = s3c64xx_get_base_gpio();

	//init gpio func for LCD	+
	gpio->GPECON = 0x00011111;	//GPE0: for LCD Backlight ON-OFF. GPE1~4 is GPIO.
	gpio->GPEDAT =  0x00000001;	//Backlight ON  (some LCD(3.5) need it ,but NEC 4.3 not)
	gpio->GPEPUD = 0x00000000;
	gpio->GPECONSLP = 0x00000000;
	gpio->GPEPUDSLP = 0x00000000;

	gpio->GPFCON = 0x96AAAAAA;	//PWM1: for Backlight Dimming. GPF13 is USB_PWR. GPF0~12 is CAM I/F.
					// PWM0 Control Buzzer,but now we config it as OUTPUT!!!
	gpio->GPFDAT =  0x00002000;	//USB_PWR ON . Buzzer OFF .
	gpio->GPFPUD = 0x00000000;
	gpio->GPFCONSLP = 0x00000000;
	gpio->GPFPUDSLP = 0x00000000;

	gpio->GPICON = 0xAAAAAAAA;
	gpio->GPIPUD = 0x00000000;
	gpio->GPICONSLP = 0x00000000;
	gpio->GPIPUDSLP = 0x00000000;

	gpio->GPJCON = 0x00AAAAAA;
	gpio->GPJPUD = 0x00000000;
	gpio->GPJCONSLP = 0x00000000;
	gpio->GPJPUDSLP = 0x00000000;
	//init gpio func for LCD	-

	//init gpio func for MMC	+
	gpio->GPCCON = 0x22222222;
	gpio->GPCPUD = 0x00000000;
	gpio->GPCCONSLP = 0x00000000;
	gpio->GPCPUDSLP = 0x00000000;

	gpio->GPGCON = 0x02222222;
	gpio->GPGPUD = 0x00000000;
	gpio->GPGCONSLP = 0x00000000;
	gpio->GPGPUDSLP = 0x00000000;

	gpio->GPHCON0 = 0x22222222;
	gpio->GPHCON1 = 0x00000022;
	gpio->GPHPUD = 0x00000000;
	gpio->GPHCONSLP = 0x00000000;
	gpio->GPHPUDSLP = 0x00000000;
	//init gpio func for MMC	-

	gd->bd->bi_arch_number = MACH_TYPE;
	gd->bd->bi_boot_params = PHYS_SDRAM_1 + 0x100;

	return 0;
}

void dram_init_banksize(void)
{
	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = PHYS_SDRAM_1_SIZE;
}

int dram_init(void)
{
	gd->ram_size = get_ram_size((long *)CONFIG_SYS_SDRAM_BASE,
				PHYS_SDRAM_1_SIZE);

	return 0;
}

#ifdef CONFIG_DISPLAY_BOARDINFO
int checkboard(void)
{
	printf("Board:   MINI6410\n");
	return 0;
}
#endif

#ifdef CONFIG_ENABLE_MMU
ulong virt_to_phy_mini6410(ulong addr)
{
	if ((0xc0000000 <= addr) && (addr < 0xc8000000))
		return addr - 0xc0000000 + 0x50000000;
	else
		printf("do not support this address : %08lx\n", addr);

	return addr;
}
#endif

#ifdef CONFIG_CMD_NET
int board_eth_init(bd_t *bi)
{
	int rc = 0;

#if defined(CONFIG_DRIVER_DM9000)
	rc = dm9000_initialize(bi);
#endif
	return rc;
}
#endif

#ifdef CONFIG_VIDEO_S3C64X0	//FIXME


//just init some reg for enable LCD
void board_video_init(GraphicDevice *pGD)
{
	s3c64xx_gpio * const gpio = s3c64xx_get_base_gpio();
	s3c64xx_modem * const modem = s3c64xx_get_base_modem();

	gpio->SPCON &=  ~(LCD_SEL_MASK << LCD_SEL);
	gpio->SPCON |=  (0x01 << LCD_SEL);
	modem->MIFPCON &= ~(1 << SEL_BYPASS);

}
#endif


#ifdef CONFIG_GENERIC_MMC
int board_mmc_init(bd_t *bis)
{
	return s3c64x0_mmc_init(0);
}
#endif



