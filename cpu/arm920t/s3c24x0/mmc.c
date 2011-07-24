/*
 * u-boot S3C2410 MMC/SD card driver
 * (C) Copyright 2006 by OpenMoko, Inc.
 * Author: Harald Welte <laforge@openmoko.org>
 *
 * based on u-boot pxa MMC driver and linux/drivers/mmc/s3c2410mci.c
 * (C) 2005-2005 Thomas Kleffel
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

#include <config.h>
#include <common.h>
#include <mmc.h>
#include <asm/arch/mmc.h>
#include <asm/errno.h>
#include <asm/io.h>
#include <asm/arch/s3c24x0_cpu.h>
#include <part.h>
#include <fat.h>

#if defined(CONFIG_MMC) && defined(CONFIG_MMC_S3C)

#ifdef DEBUG
#define pr_debug(fmt, args...) printf(fmt, ##args)
#else
#define pr_debug(...) do { } while(0)
#endif

#define CONFIG_MMC_WIDE

static struct s3c2410_sdi *sdi;

static block_dev_desc_t mmc_dev;

block_dev_desc_t * mmc_get_dev(int dev)
{
	return ((block_dev_desc_t *)&mmc_dev);
}

/*
 * FIXME needs to read cid and csd info to determine block size
 * and other parameters
 */
static uchar mmc_buf[MMC_BLOCK_SIZE];
static mmc_csd_t mmc_csd;
static int mmc_ready = 0;
static int wide = 0;


#define CMD_F_RESP	0x01
#define CMD_F_RESP_LONG	0x02

#define CMD_F_RESP_R7 CMD_F_RESP

static u_int32_t *mmc_cmd(ushort cmd, ulong arg, ushort flags)
{
	static u_int32_t resp[5];

	u_int32_t ccon, csta;
	u_int32_t csta_rdy_bit = S3C2410_SDICMDSTAT_CMDSENT;

	memset(resp, 0, sizeof(resp));

	debug("mmc_cmd CMD%d arg=0x%08x flags=%x\n", cmd, arg, flags);

	writel(0xffffffff, &sdi->SDICSTA);
	writel(0xffffffff, &sdi->SDIDSTA);
	writel(0xffffffff, &sdi->SDIFSTA);
	writel(arg, &sdi->SDICARG);

	ccon = cmd & S3C2410_SDICMDCON_INDEX;
	ccon |= S3C2410_SDICMDCON_SENDERHOST|S3C2410_SDICMDCON_CMDSTART;

	if (flags & CMD_F_RESP) {
		ccon |= S3C2410_SDICMDCON_WAITRSP;
		csta_rdy_bit = S3C2410_SDICMDSTAT_RSPFIN; /* 1 << 9 */
	}

	if (flags & CMD_F_RESP_LONG)
		ccon |= S3C2410_SDICMDCON_LONGRSP;
	writel(ccon, &sdi->SDICCON);

	while (1) {
		csta = readl(&sdi->SDICSTA);
		if (csta & csta_rdy_bit)
			break;
		if (csta & S3C2410_SDICMDSTAT_CMDTIMEOUT) {
			printf("===============> MMC CMD Timeout\n");
			writel((readl(&sdi->SDICSTA) | S3C2410_SDICMDSTAT_CMDTIMEOUT), &sdi->SDICSTA);
			break;
		}
	}

	debug("final MMC CMD status 0x%x\n", csta);
	writel((readl(&sdi->SDICSTA) | csta_rdy_bit), &sdi->SDICSTA);

	if (flags & CMD_F_RESP) {
		resp[0] = readl(&sdi->SDIRSP0);
		resp[1] = readl(&sdi->SDIRSP1);
		resp[2] = readl(&sdi->SDIRSP2);
		resp[3] = readl(&sdi->SDIRSP3);
	}

	return resp;
}

#define FIFO_FILL(host) ((host->SDIFSTA & S3C2410_SDIFSTA_COUNTMASK) >> 2)

static int mmc_block_read(uchar *dst, ulong src, ulong len)
{
	u_int32_t dcon, fifo;
	u_int32_t *dst_u32 = (u_int32_t *)dst;
	u_int32_t *resp;

	if (len == 0)
		return 0;

	debug("mmc_block_rd dst %lx src %lx len %d\n", (ulong)dst, src, len);

	/* set block len */
	resp = mmc_cmd(MMC_CMD_SET_BLOCKLEN, len, CMD_F_RESP);
	writel(len, &sdi->SDIBSIZE);

	//sdi->SDIPRE = 0xff;

	/* setup data */
	dcon = (len >> 9) & S3C2410_SDIDCON_BLKNUM;
	dcon |= S3C2410_SDIDCON_BLOCKMODE;
	dcon |= S3C2410_SDIDCON_RXAFTERCMD|S3C2410_SDIDCON_XFER_RXSTART;
	if (wide)
		dcon |= S3C2410_SDIDCON_WIDEBUS;
#if defined(CONFIG_S3C2440) || defined(CONFIG_S3C2442)
	dcon |= S3C2440_SDIDCON_DS_WORD | S3C2440_SDIDCON_DATSTART;
#endif
	writel(dcon, &sdi->SDIDCON);

	/* send read command */
	resp = mmc_cmd(MMC_CMD_READ_BLOCK, (mmc_dev.if_type == IF_TYPE_SDHC) ? (src >> 9) : src, CMD_F_RESP);

	while (len > 0) {
		u_int32_t sdidsta = readl(&sdi->SDIDSTA);
		fifo = FIFO_FILL(sdi);
		if (sdidsta & (S3C2410_SDIDSTA_FIFOFAIL|
				S3C2410_SDIDSTA_CRCFAIL|
				S3C2410_SDIDSTA_RXCRCFAIL|
				S3C2410_SDIDSTA_DATATIMEOUT)) {
			printf("mmc_block_read: err SDIDSTA=0x%08x\n", sdidsta);
			return -EIO;
		}

		while (fifo--) {
			//debug("dst_u32 = 0x%08x\n", dst_u32);
			*(dst_u32++) = readl(&sdi->SDIDAT);
			if (len >= 4)
				len -= 4;
			else {
				len = 0;
				break;
			}
		}
	}

	debug("waiting for SDIDSTA  (currently 0x%08x\n", readl(&sdi->SDIDSTA));
	while (!(readl(&sdi->SDIDSTA) & (1 << 4))) {}
	debug("done waiting for SDIDSTA (currently 0x%08x\n", readl(&sdi->SDIDSTA));

	writel(0, &sdi->SDIDCON);

	if (!(readl(&sdi->SDIDSTA) & S3C2410_SDIDSTA_XFERFINISH))
		debug("mmc_block_read; transfer not finished!\n");

	return 0;
}

static int mmc_block_write(ulong dst, uchar *src, int len)
{
	printf("MMC block write not yet supported on S3C2410!\n");
	return -1;
}


int mmc_read(ulong src, uchar *dst, int size)
{
	ulong end, part_start, part_end, part_len, aligned_start, aligned_end;
	ulong mmc_block_size, mmc_block_address;

	if (size == 0)
		return 0;

	if (!mmc_ready) {
		printf("Please initialize the MMC first\n");
		return -1;
	}

	mmc_block_size = MMC_BLOCK_SIZE;
	mmc_block_address = ~(mmc_block_size - 1);

	src -= CFG_MMC_BASE;
	end = src + size;
	part_start = ~mmc_block_address & src;
	part_end = ~mmc_block_address & end;
	aligned_start = mmc_block_address & src;
	aligned_end = mmc_block_address & end;

	/* all block aligned accesses */
	debug("src %lx dst %lx end %lx pstart %lx pend %lx astart %lx aend %lx\n",
	src, (ulong)dst, end, part_start, part_end, aligned_start, aligned_end);
	if (part_start) {
		part_len = mmc_block_size - part_start;
		debug("ps src %lx dst %lx end %lx pstart %lx pend %lx astart %lx aend %lx\n",
		src, (ulong)dst, end, part_start, part_end, aligned_start, aligned_end);
		if ((mmc_block_read(mmc_buf, aligned_start, mmc_block_size)) < 0)
			return -1;

		memcpy(dst, mmc_buf+part_start, part_len);
		dst += part_len;
		src += part_len;
	}
	debug("src %lx dst %lx end %lx pstart %lx pend %lx astart %lx aend %lx\n",
	src, (ulong)dst, end, part_start, part_end, aligned_start, aligned_end);
	for (; src < aligned_end; src += mmc_block_size, dst += mmc_block_size) {
		debug("al src %lx dst %lx end %lx pstart %lx pend %lx astart %lx aend %lx\n",
		src, (ulong)dst, end, part_start, part_end, aligned_start, aligned_end);
		if ((mmc_block_read((uchar *)(dst), src, mmc_block_size)) < 0)
			return -1;
	}
	debug("src %lx dst %lx end %lx pstart %lx pend %lx astart %lx aend %lx\n",
	src, (ulong)dst, end, part_start, part_end, aligned_start, aligned_end);
	if (part_end && src < end) {
		debug("pe src %lx dst %lx end %lx pstart %lx pend %lx astart %lx aend %lx\n",
		src, (ulong)dst, end, part_start, part_end, aligned_start, aligned_end);
		if ((mmc_block_read(mmc_buf, aligned_end, mmc_block_size)) < 0)
			return -1;

		memcpy(dst, mmc_buf, part_end);
	}
	return 0;
}

int mmc_write(uchar *src, ulong dst, int size)
{
	ulong end, part_start, part_end, part_len, aligned_start, aligned_end;
	ulong mmc_block_size, mmc_block_address;

	if (size == 0)
		return 0;

	if (!mmc_ready) {
		printf("Please initialize the MMC first\n");
		return -1;
	}

	mmc_block_size = MMC_BLOCK_SIZE;
	mmc_block_address = ~(mmc_block_size - 1);

	dst -= CFG_MMC_BASE;
	end = dst + size;
	part_start = ~mmc_block_address & dst;
	part_end = ~mmc_block_address & end;
	aligned_start = mmc_block_address & dst;
	aligned_end = mmc_block_address & end;

	/* all block aligned accesses */
	debug("src %lx dst %lx end %lx pstart %lx pend %lx astart %lx aend %lx\n",
	src, (ulong)dst, end, part_start, part_end, aligned_start, aligned_end);
	if (part_start) {
		part_len = mmc_block_size - part_start;
		debug("ps src %lx dst %lx end %lx pstart %lx pend %lx astart %lx aend %lx\n",
		(ulong)src, dst, end, part_start, part_end, aligned_start, aligned_end);
		if ((mmc_block_read(mmc_buf, aligned_start, mmc_block_size)) < 0)
			return -1;

		memcpy(mmc_buf+part_start, src, part_len);
		if ((mmc_block_write(aligned_start, mmc_buf, mmc_block_size)) < 0)
			return -1;

		dst += part_len;
		src += part_len;
	}
	debug("src %lx dst %lx end %lx pstart %lx pend %lx astart %lx aend %lx\n",
	src, (ulong)dst, end, part_start, part_end, aligned_start, aligned_end);
	for (; dst < aligned_end; src += mmc_block_size, dst += mmc_block_size) {
		debug("al src %lx dst %lx end %lx pstart %lx pend %lx astart %lx aend %lx\n",
		src, (ulong)dst, end, part_start, part_end, aligned_start, aligned_end);
		if ((mmc_block_write(dst, (uchar *)src, mmc_block_size)) < 0)
			return -1;

	}
	debug("src %lx dst %lx end %lx pstart %lx pend %lx astart %lx aend %lx\n",
	src, (ulong)dst, end, part_start, part_end, aligned_start, aligned_end);
	if (part_end && dst < end) {
		debug("pe src %lx dst %lx end %lx pstart %lx pend %lx astart %lx aend %lx\n",
		src, (ulong)dst, end, part_start, part_end, aligned_start, aligned_end);
		if ((mmc_block_read(mmc_buf, aligned_end, mmc_block_size)) < 0)
			return -1;

		memcpy(mmc_buf, src, part_end);
		if ((mmc_block_write(aligned_end, mmc_buf, mmc_block_size)) < 0)
			return -1;

	}
	return 0;
}

ulong mmc_bread(int dev_num, ulong blknr, ulong blkcnt, void *dst)
{
	int mmc_block_size = MMC_BLOCK_SIZE;
	ulong src = blknr * mmc_block_size + CFG_MMC_BASE;

	mmc_read(src, dst, blkcnt*mmc_block_size);
	return blkcnt;
}

/* MMC_DEFAULT_RCA should probably be just 1, but this may break other code
   that expects it to be shifted. */
static u_int16_t rca = MMC_DEFAULT_RCA >> 16;

static u_int32_t mmc_size(const struct mmc_csd *csd)
{
	u_int32_t block_len, mult, blocknr;

	block_len = csd->read_bl_len << 12;
	mult = csd->c_size_mult1 << 8;
	blocknr = (csd->c_size+1) * mult;

	return blocknr * block_len;
}

struct sd_cid {
	char		pnm_0;	/* product name */
	char		oid_1;	/* OEM/application ID */
	char		oid_0;
	uint8_t		mid;	/* manufacturer ID */
	char		pnm_4;
	char		pnm_3;
	char		pnm_2;
	char		pnm_1;
	uint8_t		psn_2;	/* product serial number */
	uint8_t		psn_1;
	uint8_t		psn_0;	/* MSB */
	uint8_t		prv;	/* product revision */
	uint8_t		crc;	/* CRC7 checksum, b0 is unused and set to 1 */
	uint8_t		mdt_1;	/* manufacturing date, LSB, RRRRyyyy yyyymmmm */
	uint8_t		mdt_0;	/* MSB */
	uint8_t		psn_3;	/* LSB */
};

static void print_mmc_cid(mmc_cid_t *cid)
{
	printf("MMC found. Card desciption is:\n");
	printf("Manufacturer ID = %02x%02x%02x\n",
		cid->id[0], cid->id[1], cid->id[2]);
	printf("HW/FW Revision = %x %x\n",cid->hwrev, cid->fwrev);
	cid->hwrev = cid->fwrev = 0;	/* null terminate string */
	printf("Product Name = %s\n",cid->name);
	printf("Serial Number = %02x%02x%02x\n",
		cid->sn[0], cid->sn[1], cid->sn[2]);
	printf("Month = %d\n",cid->month);
	printf("Year = %d\n",1997 + cid->year);
}

static void print_sd_cid(const struct sd_cid *cid)
{
	printf("Manufacturer:       0x%02x, OEM \"%c%c\"\n",
	    cid->mid, cid->oid_0, cid->oid_1);
	printf("Product name:       \"%c%c%c%c%c\", revision %d.%d\n",
	    cid->pnm_0, cid->pnm_1, cid->pnm_2, cid->pnm_3, cid->pnm_4,
	    cid->prv >> 4, cid->prv & 15);
	printf("Serial number:      %u\n",
	    cid->psn_0 << 24 | cid->psn_1 << 16 | cid->psn_2 << 8 |
	    cid->psn_3);
	printf("Manufacturing date: %d/%d\n",
	    cid->mdt_1 & 15,
	    2000+((cid->mdt_0 & 15) << 4)+((cid->mdt_1 & 0xf0) >> 4));
	printf("CRC:                0x%02x, b0 = %d\n",
	    cid->crc >> 1, cid->crc & 1);
}

int mmc_init(int verbose)
{
 	int retries, rc = -ENODEV;
	int is_sd = 0;
	u_int32_t *resp;
	struct s3c24x0_clock_power * const clk_power = s3c24x0_get_base_clock_power();
	block_dev_desc_t *mmc_blkdev_p = &mmc_dev;

	sdi = s3c2410_get_base_sdi();

	debug("mmc_init(PCLK=%u)\n", get_PCLK());

	clk_power->CLKCON |= (1 << 9);
	writel(512, &sdi->SDIBSIZE);
#if defined(CONFIG_S3C2410)
	/* S3C2410 has some bug that prevents reliable operation at higher speed */
	//sdi->SDIPRE = 0x3e;  /* SDCLK = PCLK/2 / (SDIPRE+1) = 396kHz */
	writel(0x02, &sdi->SDIPRE); /* 2410: SDCLK = PCLK/2 / (SDIPRE+1) = 11MHz */
	writel(0xffff, &sdi->SDIDTIMER);
#elif defined(CONFIG_S3C2440) || defined(CONFIG_S3C2442)
	writel(0x05, &sdi->SDIPRE); /* 2410: SDCLK = PCLK / (SDIPRE+1) = 11MHz */
	writel(0x7fffff, &sdi->SDIDTIMER);
#endif
	writel(0x0, &sdi->SDIIMSK);
	writel((S3C2410_SDICON_FIFORESET | S3C2410_SDICON_CLOCKTYPE), &sdi->SDICON);
	udelay(125000); /* FIXME: 74 SDCLK cycles */

	mmc_csd.c_size = 0;

	/* reset */
	retries = 10;
	resp = mmc_cmd(MMC_CMD_RESET, 0, 0);

	mmc_dev.if_type = IF_TYPE_UNKNOWN;
	if(verbose)
		puts("mmc: Probing for SDHC ...\n");

	/* Send supported voltage range */
	/* SD cards 1.x do not answer to CMD8 */
        resp = mmc_cmd(MMC_CMD_IF_COND, ((1 << 8) | 0xAA), CMD_F_RESP_R7);
        if (!resp[0]) {
             /*
              * ARC: No answer let's try SD 1.x
              */
             if(verbose)
                     puts("mmc: No answer to CMD8 trying SD\n");
             mmc_blkdev_p->if_type = IF_TYPE_SD;
        } else {
             /*
              * ARC: probably an SDHC card
              */
             mmc_blkdev_p->if_type = IF_TYPE_SDHC;
             if(verbose)
                     puts("mmc: SD 2.0 or later card found\n");

             /* Check if the card supports this voltage */
             if (resp[0] != ((1 << 8) | 0xAA)) {
                     pr_debug("mmc: Invalid voltage range\n");
                     return -ENODEV;
             }
        }

	/*
	 * ARC: HC (30) bit set according to response to
	 * CMD8 command
	 */

	pr_debug("mmc: Sending ACMD41 %s HC set\n",
		         ((mmc_blkdev_p->if_type ==
		           IF_TYPE_SDHC) ? "with" : "without"));

	printf("trying to detect SD Card...\n");
	while (retries--) {
		udelay(100000);
		resp = mmc_cmd(55, 0x00000000, CMD_F_RESP);
		resp = mmc_cmd(41, (mmc_blkdev_p->if_type == IF_TYPE_SDHC)? (0x00300000 | (1<<30)) : 0x00300000, CMD_F_RESP);

		if (resp[0] & (1 << 31)) {
			is_sd = 1;
			break;
		}
	}

	/*
	* ARC: check for HC bit, if its not set
	* sd card is SD
	*/
	if (is_sd && (resp[0] & 0xc0000000) == 0x80000000) {
	       mmc_dev.if_type = IF_TYPE_SD;
	}

	if (retries == 0 && !is_sd) {
		retries = 10;
		printf("failed to detect SD Card, trying MMC\n");
		mmc_blkdev_p->if_type = IF_TYPE_MMC;
		resp = mmc_cmd(MMC_CMD_SEND_OP_COND, 0x00ffc000, CMD_F_RESP);
		while (retries-- && resp && !(resp[4] & 0x80)) {
			debug("resp %x %x\n", resp[0], resp[1]);
			udelay(50);
			resp = mmc_cmd(1, 0x00ffff00, CMD_F_RESP);
		}
	}

	/* try to get card id */
	resp = mmc_cmd(MMC_CMD_ALL_SEND_CID, 0, CMD_F_RESP|CMD_F_RESP_LONG);
	if (resp) {
		if (!is_sd) {
			/* TODO configure mmc driver depending on card
			   attributes */
			mmc_cid_t *cid = (mmc_cid_t *)resp;

			if (verbose)
				print_mmc_cid(cid);
			sprintf((char *) mmc_dev.vendor,
				"Man %02x%02x%02x Snr %02x%02x%02x",
				cid->id[0], cid->id[1], cid->id[2],
				cid->sn[0], cid->sn[1], cid->sn[2]);
			sprintf((char *) mmc_dev.product,"%s",cid->name);
			sprintf((char *) mmc_dev.revision,"%x %x",
				cid->hwrev, cid->fwrev);
		}
		else {
			struct sd_cid *cid = (struct sd_cid *) resp;

			if (verbose)
				print_sd_cid(cid);
			sprintf((char *) mmc_dev.vendor, "Man %02x OEM %c%c \"%c%c%c%c%c\"", cid->mid, cid->oid_0, cid->oid_1, cid->pnm_0, cid->pnm_1, cid->pnm_2, cid->pnm_3, cid->pnm_4);
			sprintf((char *) mmc_dev.product, "%d",
			    cid->psn_0 << 24 | cid->psn_1 << 16 |
			    cid->psn_2 << 8 | cid->psn_3);
			sprintf((char *) mmc_dev.revision, "%d.%d",
			    cid->prv >> 4, cid->prv & 15);
		}

		/* fill in device description */
		if (mmc_dev.if_type == IF_TYPE_UNKNOWN)
			mmc_dev.if_type = IF_TYPE_MMC;
		mmc_dev.part_type = PART_TYPE_DOS;
		mmc_dev.dev = 0;
		mmc_dev.lun = 0;
		mmc_dev.type = 0;
		/* FIXME fill in the correct size (is set to 32MByte) */
		mmc_dev.blksz = 512;
		mmc_dev.lba = 0x10000;
		mmc_dev.removable = 0;
		mmc_dev.block_read = mmc_bread;

		/* MMC exists, get CSD too */
		resp = mmc_cmd(MMC_CMD_SET_RCA, MMC_DEFAULT_RCA, CMD_F_RESP);
		if (is_sd)
			rca = resp[0] >> 16;

		resp = mmc_cmd(MMC_CMD_SEND_CSD, rca<<16, CMD_F_RESP|CMD_F_RESP_LONG);
		if (resp) {
			mmc_csd_t *csd = (mmc_csd_t *)resp;
			memcpy(&mmc_csd, csd, sizeof(csd));
			rc = 0;
			mmc_ready = 1;
			/* FIXME add verbose printout for csd */
			printf("READ_BL_LEN=%u, C_SIZE_MULT=%u, C_SIZE=%u\n",
				csd->read_bl_len, csd->c_size_mult1, csd->c_size);
			printf("size = %u\n", mmc_size(csd));
		}
	}

	resp = mmc_cmd(MMC_CMD_SELECT_CARD, rca<<16, CMD_F_RESP);

	if (verbose)
	       printf("SD Card detected RCA: 0x%x type: %s\n",
	              rca, ((mmc_dev.if_type == IF_TYPE_SDHC) ? "SDHC" : ((mmc_dev.if_type == IF_TYPE_SD) ? "SD" : "MMC")));

#ifdef CONFIG_MMC_WIDE
	if (is_sd) {
		resp = mmc_cmd(55, rca<<16, CMD_F_RESP);
		resp = mmc_cmd(6, 0x02, CMD_F_RESP);
		wide = 1;
	}
#endif

	fat_register_device(&mmc_dev,1); /* partitions start counting with 1 */

	return rc;
}

int
mmc_ident(block_dev_desc_t *dev)
{
	return 0;
}

int
mmc2info(ulong addr)
{
	/* FIXME hard codes to 32 MB device */
	if (addr >= CFG_MMC_BASE && addr < CFG_MMC_BASE + 0x02000000)
		return 1;

	return 0;
}

#endif	/* defined(CONFIG_MMC) && defined(CONFIG_MMC_S3C) */
