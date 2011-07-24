#include <asm/types.h>
#include <asm/io.h>
#include <asm/arch/irom-boot.h>
#include <config.h>


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
	CopyMMCtoMem(MMC_BOOT_CHANNEL, MMC_UBOOT_POS, MMC_UBOOT_BLKCNT, (uint *)(CONFIG_SYS_MMC_U_BOOT_START), MMC_INIT_REQUIRED);

#ifdef CONFIG_MMC_ENV_DST
	CopyMMCtoMem(MMC_BOOT_CHANNEL, MMC_ENV_POS, MMC_ENV_BLKCNT, (uint *)CONFIG_MMC_ENV_DST, MMC_INIT_REQUIRED);

#ifdef CONFIG_MMC_OFFSET_REDUND
	CopyMMCtoMem(MMC_BOOT_CHANNEL, MMC_ENV_POS_REDUND, MMC_ENV_BLKCNT, (uint *)CONFIG_MMC_ENV_DST+ CONFIG_ENV_SIZE, MMC_INIT_REQUIRED);
#endif
#endif

	/*
	 * Jump to U-Boot image
	 */
	uboot = (void *)(CONFIG_SYS_MMC_U_BOOT_START);
	(*uboot)();
}
