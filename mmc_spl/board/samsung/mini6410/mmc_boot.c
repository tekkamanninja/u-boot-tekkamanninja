/*
 * Copyright (C) 2011 Samsung Electronics
 *
 * (C) Copyright 2011
 * Tekkaman Ninja, <tekkamanninja at gmail.com>
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
#include <asm/types.h>
#include <asm/io.h>
#include <config.h>

/* TCM function for bl2 load */
#if defined(CONFIG_S3C6400)
#define copy_sd_mmc_to_mem(a,b,c,d,e)	\
		(((int(*)(uint, ushort, uint *, uint, int))(*((uint *)(TCM_BASE + 0x8))))(a,b,c,d,e))
#elif defined(CONFIG_S3C6410) || defined(CONFIG_S3C6430)
#define copy_sd_mmc_to_mem(a,b,c,d,e)	\
		(((int(*)(int, uint, ushort, uint *, int))(*((uint *)(TCM_BASE + 0x8))))(a,b,c,d,e))
#endif

void copy_uboot_to_ram(void)
{
	copy_sd_mmc_to_mem(MMC_BOOT_CHANNEL,
						MMC_UBOOT_POS, MMC_UBOOT_BLKCNT, (uint *)(CONFIG_SYS_MMC_U_BOOT_START),
						MMC_INIT_REQUIRED);

#ifdef CONFIG_MMC_ENV_DST
	copy_sd_mmc_to_mem(MMC_BOOT_CHANNEL,
						MMC_ENV_POS, MMC_ENV_BLKCNT, (uint *)CONFIG_MMC_ENV_DST,
						MMC_INIT_REQUIRED);

#ifdef CONFIG_MMC_OFFSET_REDUND
	copy_sd_mmc_to_mem(MMC_BOOT_CHANNEL,
						MMC_ENV_POS_REDUND, MMC_ENV_BLKCNT, (uint *)CONFIG_MMC_ENV_DST+ CONFIG_ENV_SIZE,
						MMC_INIT_REQUIRED);
#endif
#endif
}

void mmc_boot_copy(void)
{
	volatile u32 *mmc_control4;
	__attribute__((noreturn)) void (*uboot)(void);

	if (MMC_BOOT_CHANNEL == 0)		{
		mmc_control4 = (volatile u32 *)0x7C20008C;
	} else	if (MMC_BOOT_CHANNEL == 1)	{
		mmc_control4 = (volatile u32 *)0x7C30008C;
	} else	if (MMC_BOOT_CHANNEL == 2)	{
		mmc_control4 = (volatile u32 *)0x7C40008C;
	}

	writel(readl(mmc_control4) | (0x3 << 16), mmc_control4);

	copy_uboot_to_ram();

	/*
	 * Jump to U-Boot image
	 */
	uboot = (void *)(CONFIG_SYS_MMC_U_BOOT_START);
	(*uboot)();
}
