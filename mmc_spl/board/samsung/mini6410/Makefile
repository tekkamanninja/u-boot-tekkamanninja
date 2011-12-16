#
# (C) Copyright 2006-2007
# Stefan Roese, DENX Software Engineering, sr at denx.de.
#
# (C) Copyright 2008
# Guennadi Liakhovetki, DENX Software Engineering, <lg at denx.de>
#
# (C) Copyright 2010
# Alex Ling, <kasimling at gmail.com>
#
# See file CREDITS for list of people who contributed to this
# project.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston,
# MA 02111-1307 USA
#

CONFIG_MMC_SPL	= y
#CONFIG_BOOT_AUTODETECT = y

include $(TOPDIR)/config.mk
include $(TOPDIR)/mmc_spl/board/$(BOARDDIR)/config.mk

LDSCRIPT= $(TOPDIR)/mmc_spl/board/$(BOARDDIR)/u-boot.lds
#LDFLAGS	= -Bstatic -T $(mmcobj)u-boot.lds -Ttext $(CONFIG_SYS_TEXT_BASE) $(PLATFORM_LDFLAGS)
LDFLAGS := -T $(mmcobj)u-boot.lds -Ttext $(CONFIG_SYS_TEXT_BASE) $(LDFLAGS) \
	   $(LDFLAGS_FINAL)
AFLAGS	+= -DCONFIG_PRELOADER -DCONFIG_MMC_SPL
CFLAGS	+= -DCONFIG_PRELOADER -DCONFIG_MMC_SPL

mmcobj	:= $(OBJTREE)/mmc_spl/

COBJS-$(CONFIG_MMC_SPL) += mmc_boot.o
ifdef CONFIG_BOOT_AUTODETECT
COBJS-y += nand_boot.o nand_ecc.o s3c64xx.o
CONFIG_NAND_SPL	= y
AFLAGS	+= -DCONFIG_NAND_SPL
CFLAGS	+= -DCONFIG_NAND_SPL
endif

SOBJS	= start.o cpu_init.o lowlevel_init.o
COBJS	:= $(sort $(COBJS-y))

SRCS	:= $(addprefix $(obj),$(SOBJS:.o=.S) $(COBJS:.o=.c))
OBJS	:= $(addprefix $(obj),$(SOBJS) $(COBJS))
__OBJS	:= $(SOBJS) $(COBJS)
LNDIR	:= $(mmcobj)board/$(BOARDDIR)

ALL	= $(mmcobj)u-boot-spl $(mmcobj)u-boot-spl.bin $(mmcobj)u-boot-spl-16k.bin

all:	$(obj).depend $(ALL)

$(mmcobj)u-boot-spl-16k.bin: $(mmcobj)u-boot-spl
	$(OBJCOPY) ${OBJCFLAGS} --pad-to=$(PAD_TO) -O binary $< $@

$(mmcobj)u-boot-spl.bin:	$(mmcobj)u-boot-spl
	$(OBJCOPY) ${OBJCFLAGS} -O binary $< $@

$(mmcobj)u-boot-spl:	$(OBJS) $(mmcobj)u-boot.lds
	cd $(LNDIR) && $(LD) $(LDFLAGS) $(__OBJS) \
		-Map $(mmcobj)u-boot-spl.map \
		-o $(mmcobj)u-boot-spl

$(mmcobj)u-boot.lds: $(LDSCRIPT)
	$(CPP) $(CPPFLAGS) $(LDPPFLAGS) -ansi -D__ASSEMBLY__ -P - <$^ >$@

# create symbolic links for common files

# from cpu directory
$(obj)start.S:
	@rm -f $@
	@ln -s $(TOPDIR)/arch/arm/cpu/arm1176/start.S $@

# from SoC directory
$(obj)cpu_init.S:
	@rm -f $@
	@ln -s $(TOPDIR)/arch/arm/cpu/arm1176/s3c64xx/cpu_init.S $@

# from board directory
$(obj)lowlevel_init.S:
	@rm -f $@
	@ln -s $(TOPDIR)/board/samsung/mini6410/lowlevel_init.S $@

# from nand_spl directory
$(obj)nand_boot.c:
	@rm -f $@
	@ln -s $(TOPDIR)/nand_spl/nand_boot.c $@

# from mmc_spl/board/samsung/mini6410 directory
$(obj)mmc_boot.c:
ifneq ($(OBJTREE),$(SRCTREE))
	@rm -f $@
	@ln -s $(TOPDIR)/mmc_spl/board/samsung/mini6410/mmc_boot.c $@
endif

# from drivers/mtd/nand directory
$(obj)nand_ecc.c:
	@rm -f $@
	@ln -s $(TOPDIR)/drivers/mtd/nand/nand_ecc.c $@

$(obj)s3c64xx.c:
	@rm -f $@
	@ln -s $(TOPDIR)/drivers/mtd/nand/s3c64xx.c $@

#########################################################################

$(obj)%.o:	$(obj)%.S
	$(CC) $(AFLAGS) -c -o $@ $<

$(obj)%.o:	$(obj)%.c
	$(CC) $(CFLAGS) -c -o $@ $<

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
