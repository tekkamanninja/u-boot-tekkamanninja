/*
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 * Gary Jennejohn <garyj@denx.de>
 * David Mueller <d.mueller@elsoft.ch>
 *
 * Modified for the friendly-arm SBC-2410X by
 * (C) Copyright 2005
 * JinHua Luo, GuangDong Linux Center, <luo.jinhua@gd-linux.com>
 *
 * Configuation settings for the friendly-arm SBC-2410X board.
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
 
#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * If we are developing, we might want to start armboot from ram
 * so we MUST NOT initialize critical regs like mem-timing ...
 */
#undef CONFIG_SKIP_LOWLEVEL_INIT	/* undef for developing */

/*
 * High Level Configuration Options
 * (easy to change)
 */
#define CONFIG_ARM920T	1	/* This is an ARM920T Core	*/
#define CONFIG_S3C24X0	1	/* in a SAMSUNG S3C24x0-type SoC	*/
//#define CONFIG_S3C2410	1	/* specifically a SAMSUNG S3C2410 SoC	*/
//#define CONFIG_SBC2410X	1	/* on a friendly-arm SBC-2410X Board  */
#define	CONFIG_S3C2440		1	/* in a SAMSUNG S3C2440 SoC     */
#define CONFIG_MINI2440		1	/* on a friendly-arm MINI2440 Board  */
#define CONFIG_MINI2440_LED 	1
#define CONFIG_S3C2410_NAND_SKIP_BAD	1
/* input clock of PLL */
#define CONFIG_SYS_CLK_FREQ	12000000/* the SBC2410X has 12MHz input clock */


#define USE_920T_MMU		1

//#undef CONFIG_USE_IRQ			/* we don't need IRQ/FIQ stuff */
#define CONFIG_USB_DEVICE 1
#ifdef CONFIG_USB_DEVICE
#define CONFIG_USE_IRQ 1
#endif
/*
 * Size of malloc() pool
 */
#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + 128*1024)
#define CONFIG_SYS_GBL_DATA_SIZE	128	/* size in bytes reserved for initial data */

/*
 * Hardware drivers
 */
#if 0
#define CONFIG_NET_MULTI
#define CONFIG_CS8900		/* we have a CS8900 on-board */
#define CONFIG_CS8900_BASE	0x19000300
#define CONFIG_CS8900_BUS16	/* the Linux driver does accesses as shorts */
#endif
#define CONFIG_NET_MULTI		1
#define CONFIG_NET_RETRY_COUNT		20
#define CONFIG_DRIVER_DM9000		1
#define CONFIG_DM9000_BASE		0x20000300
#define DM9000_IO 			CONFIG_DM9000_BASE
#define DM9000_DATA 			(CONFIG_DM9000_BASE+4)
#define CONFIG_DM9000_USE_16BIT		1
#define CONFIG_DM9000_NO_SROM		1
#undef CONFIG_DM9000_DEBUG
/*
 * select serial console configuration
 */
#define CONFIG_S3C24X0_SERIAL
#define CONFIG_SERIAL1          1	/* we use SERIAL 1 on SBC2410X */

/************************************************************
 * RTC
 ************************************************************/
#define	CONFIG_RTC_S3C24X0	1

/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE

#define CONFIG_BAUDRATE		115200


/*
 * BOOTP options
 */
#define CONFIG_BOOTP_BOOTFILESIZE
#define CONFIG_BOOTP_BOOTPATH
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_HOSTNAME


/*
 * Command line configuration.
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_ASKENV
#define CONFIG_CMD_CACHE
#define CONFIG_CMD_DATE
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_ELF
//#define CONFIG_MTD_DEVICE
//#define CONFIG_CMD_MTDPARTS
#define CONFIG_CMD_PING
#define CONFIG_CMD_NAND
#define CONFIG_CMD_REGINFO
#define CONFIG_CMD_FAT 
/* FAT support*/
//#define CONFIG_CMD_EXT2

#define CONFIG_CMD_JFFS2
/* JFFS2 Support*/
#define CONFIG_CMD_USB
/* USB Support*/

#define CONFIG_BOOTDELAY	1
#define CONFIG_BOOTARGS		"noinitrd root=/dev/nfs rw nfsroot=192.168.0.1:/home/tekkaman/working/nfs/rootfs ip=192.168.0.2:192.168.0.1::255.255.255.0 console=ttySAC0,115200 init=/linuxrc mem=64M"
#define CONFIG_ETHADDR	        08:08:11:18:12:27
#define CONFIG_NETMASK          255.255.255.0
#define CONFIG_IPADDR		192.168.0.2
#define CONFIG_SERVERIP		192.168.0.1
#define CONFIG_GATEWAYIP	192.168.0.1
#define CONFIG_OVERWRITE_ETHADDR_ONCE

/*#define CONFIG_BOOTFILE	"elinos-lart" */
#define CONFIG_BOOTCOMMAND	"nfs 0x30008000 192.168.0.1:/home/tekkaman/working/nfs/zImage.img;bootm"
#define	CONFIG_EXTRA_ENV_SETTINGS					\
	"tekkaman=bmp d 70000\0 "				\
	"stdin=serial\0"					\
	"stdout=serial\0"					\
	"stderr=serial\0"					\
	""

#if defined(CONFIG_CMD_KGDB)
#define CONFIG_KGDB_BAUDRATE	115200		/* speed to run kgdb serial port */
/* what's this ? it's not used anywhere */
#define CONFIG_KGDB_SER_INDEX	1		/* which serial port to use */
#endif

/*
 * Miscellaneous configurable options
 */
#define	CONFIG_SYS_LONGHELP				/* undef to save memory		*/
#define	CONFIG_SYS_PROMPT		"[u-boot@MINI2440]# "	/* Monitor Command Prompt	*/
#define	CONFIG_SYS_CBSIZE		256		/* Console I/O Buffer Size	*/
#define	CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16) /* Print Buffer Size */
#define	CONFIG_SYS_MAXARGS		16		/* max number of command args	*/
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE	/* Boot Argument Buffer Size	*/

#define CONFIG_SYS_MEMTEST_START	0x30000000	/* memtest works on	*/
#define CONFIG_SYS_MEMTEST_END		0x33F00000	/* 63 MB in DRAM	*/


#define	CONFIG_SYS_LOAD_ADDR		0x30008000	/* default load address	*/

#define	CONFIG_SYS_HZ			1000

/* valid baudrates */
#define CONFIG_SYS_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }

/*-----------------------------------------------------------------------
 * Stack sizes
 *
 * The stack sizes are set up in start.S using the settings below
 */
#define CONFIG_STACKSIZE	(128*1024)	/* regular stack */
#ifdef CONFIG_USE_IRQ
#define CONFIG_STACKSIZE_IRQ	(4*1024)	/* IRQ stack */
#define CONFIG_STACKSIZE_FIQ	(4*1024)	/* FIQ stack */
#endif

/*-----------------------------------------------------------------------
 * Physical Memory Map
 */
#define CONFIG_NR_DRAM_BANKS	1	   /* we have 1 bank of DRAM */
#define PHYS_SDRAM_1		0x30000000 /* SDRAM Bank #1 */
#define PHYS_SDRAM_1_SIZE	0x04000000 /* 64 MB */

#define PHYS_FLASH_1		0x00000000 /* Flash Bank #1 */

#define CONFIG_SYS_FLASH_BASE		PHYS_FLASH_1

/*-----------------------------------------------------------------------
 * FLASH and environment organization
 */
/* #define CONFIG_AMD_LV400	1	/\* uncomment this if you have a LV400 flash *\/ */

//#define CONFIG_AMD_LV800	1	/* uncomment this if you have a LV800 flash */
#define CONFIG_SST_VF1601	1	/* uncomment this if you have a Am29LV160DB flash */

#define CONFIG_SYS_MAX_FLASH_BANKS	1	/* max number of memory banks */

#ifdef CONFIG_AMD_LV800
#define PHYS_FLASH_SIZE		0x00100000 /* 1MB */
#define CONFIG_SYS_MAX_FLASH_SECT	(19)	/* max number of sectors on one chip */
#define CONFIG_ENV_ADDR		(CONFIG_SYS_FLASH_BASE + 0x0F0000) /* addr of environment */
#endif

#ifdef CONFIG_AMD_LV400
#define PHYS_FLASH_SIZE		0x00080000 /* 512KB */
#define CONFIG_SYS_MAX_FLASH_SECT	(11)	/* max number of sectors on one chip */
#define CONFIG_ENV_ADDR		(CONFIG_SYS_FLASH_BASE + 0x070000) /* addr of environment */
#endif

/* timeout values are in ticks */
#define CONFIG_SYS_FLASH_ERASE_TOUT	(5*CONFIG_SYS_HZ) /* Timeout for Flash Erase */
#define CONFIG_SYS_FLASH_WRITE_TOUT	(5*CONFIG_SYS_HZ) /* Timeout for Flash Write */


//#if 0
#define CONFIG_CMD_EEPROM
#define CONFIG_CMD_I2C

#define CONFIG_DRIVER_S3C24X0_I2C    1    /* we use the buildin I2C controller */
#define CONFIG_HARD_I2C    1        /* I2C with hardware support */

#define CONFIG_SYS_I2C_SPEED        100000    /* I2C speed and slave address */
#define CONFIG_SYS_I2C_SLAVE        0x7F

#define CONFIG_SYS_I2C_EEPROM_ADDR        0x50    /* EEPROM at24c08        */
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN     1    /* Bytes of address        */
/* mask of address bits that overflow into the "EEPROM chip address"    */
#define CONFIG_SYS_I2C_EEPROM_ADDR_OVERFLOW    0x07
#define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS 4    /* The Catalyst CAT24WC08 has    */
                    /* 16 byte page write mode using*/
                    /* last 4 bits of the address    */
#define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS    10   /* and takes up to 10 msec */
#define CONFIG_SYS_EEPROM_PAGE_WRITE_ENABLE

//#define CONFIG_ENV_IS_IN_EEPROM    1    /* use EEPROM for environment vars */
//#define CONFIG_ENV_OFFSET       0x000    /* environment starts at offset 0 */
//#define CONFIG_ENV_SIZE	        0x400    /* 1KB  */

//#else 
#define CONFIG_ENV_IS_IN_NAND 1
//#define CONFIG_ENV_IS_IN_FLASH 1
#define CONFIG_ENV_OFFSET 0X60000
#define CONFIG_ENV_SIZE			0x20000	/* Total Size of Environment Sector */
//#endif

/* == LENGTH_UBOOT*/
#ifdef CONFIG_SST_VF1601
#define PHYS_FLASH_SIZE		0x00200000 /* 2MB */
#define CONFIG_SYS_MAX_FLASH_SECT	(32)	/* max number of sectors on one chip */
#define CONFIG_ENV_ADDR		(CONFIG_SYS_FLASH_BASE + CONFIG_ENV_OFFSET) /* addr of environment */
#endif


/*-----------------------------------------------------------------------
 * NAND flash settings
 */
#if defined(CONFIG_CMD_NAND)
#define CONFIG_NAND_S3C2410
#define CONFIG_SYS_NAND_BASE 0x4E000000 
#define CONFIG_SYS_MAX_NAND_DEVICE	1	/* Max number of NAND devices		*/
#define SECTORSIZE 512
#define SECTORSIZE_2K 2048
#define NAND_SECTOR_SIZE SECTORSIZE
#define NAND_SECTOR_SIZE_2K SECTORSIZE_2K
#define NAND_BLOCK_MASK 511
#define NAND_BLOCK_MASK_2K 2047
#define NAND_MAX_CHIPS 1
#define CONFIG_MTD_NAND_VERIFY_WRITE 
#define CONFIG_SYS_64BIT_VSPRINTF		/* needed for nand_util.c */
#endif	/* CONFIG_CMD_NAND */

#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG
#define CONFIG_CMDLINE_TAG

#define CONFIG_SYS_HUSH_PARSER
#define CONFIG_SYS_PROMPT_HUSH_PS2   "> "

#define CONFIG_CMDLINE_EDITING

//#ifdef CONFIG_CMDLINE_EDITING
//#undef CONFIG_AUTO_COMPLETE
//#else
#define CONFIG_AUTO_COMPLETE
//#endif


#if 1
#define CONFIG_USB_OHCI
#define CONFIG_USB_STORAGE
//#define CONFIG_KEYBOARD
//#define CONFIG_USB_KEYBOARD
#define CONFIG_DOS_PARTITION
#define CONFIG_SYS_DEVICE_DEREGISTER
#define CONFIG_SUPPORT_VFAT
#define LITTLEENDIAN
#endif 

#define CONFIG_JFFS2_NAND 1
//#undef CONFIG_JFFS2_CMDLINE
#define CONFIG_JFFS2_DEV	"nand0"
#define CONFIG_JFFS2_PART_SIZE	0x480000
#define CONFIG_JFFS2_PART_OFFSET	0x80000

#define CONFIG_JFFS2_CMDLINE 1
#define MTDIDS_DEFAULT "nand0=nandflash0"
#define MTDPARTS_DEFAULT "mtdparts=nandflash0:384k(bootloader)," \
					      "128k(params)," \
					      "5m(kernel)," \
					      "-(root)"


#define ENABLE_CMD_LOADB_X	1
#define ENABLE_CMD_NAND_YAFFS	1
#define ENABLE_CMD_NAND_YAFFS_SKIPFB	1
//#define CFG_NAND_YAFFS1_NEW_OOB_LAYOUT	1

#if 1
#define CONFIG_CMD_BMP 
#define CONFIG_VIDEO 
#define CONFIG_VIDEO_S3C2410 
#define CONFIG_VIDEO_LOGO 
#define VIDEO_FB_16BPP_WORD_SWAP 

#define CONFIG_VIDEO_SW_CURSOR 
#define CONFIG_VIDEO_BMP_LOGO 
//#define CONFIG_CONSOLE_EXTRA_INFO
//#define CONFIG_CONSOLE_CURSOR
//#define CONFIG_CONSOLE_TIME
#define CONFIG_CFB_CONSOLE
#define CONFIG_SYS_CONSOLE_IS_IN_ENV
//#define CFG_CONSOLE_INFO_QUIET 
//#define VIDEO_FB_LITTLE_ENDIAN 
#define CONFIG_SPLASH_SCREEN 
#define CONFIG_SYS_VIDEO_LOGO_MAX_SIZE         (240*320+1024+100) /* 100 = slack */ 
#define CONFIG_VIDEO_BMP_GZIP 
#define CONFIG_CMD_UNZIP 
#define LCD_VIDEO_ADDR         0x33d00000

/*for PC-keyboard*/
#define VIDEO_KBD_INIT_FCT     0 
#define VIDEO_TSTC_FCT         serial_tstc 
#define VIDEO_GETC_FCT         serial_getc

#endif

/*for SD Card*/
#define CONFIG_CMD_MMC
#define CONFIG_MMC		1
#define CONFIG_MMC_S3C	1	/* Enabling the MMC driver */
#define CFG_MMC_BASE		0xff000000


#if 0
#define CONFIG_YAFFS2
//#undef CONFIG_YAFFS_YAFFS2
#undef CONFIG_YAFFS_NO_YAFFS1
#endif

#endif	/* __CONFIG_H */
