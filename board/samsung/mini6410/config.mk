#
# (C) Copyright 2002
# Gary Jennejohn, DENX Software Engineering, <garyj at denx.de>
# David Mueller, ELSOFT AG, <d.mueller at elsoft.ch>
#
# (C) Copyright 2008
# Guennadi Liakhovetki, DENX Software Engineering, <lg at denx.de>
#
# (C) Copyright 2010
# Alex Ling, <kasimling at gmail.com>
#
# FriendlyARM MINI6410 board with mDirac3 (ARM1176) cpu
#
# see http://www.samsung.com/ for more information on SAMSUNG

# On MINI6410 we use the 128 MB SDRAM bank at
#
# 0x50000000 to 0x58000000
#
# Linux-Kernel is expected to be at 0x50008000, entry 0x50008000
#
# we load ourselves to 0x57e00000 without MMU
# with MMU, load address is changed to 0xc7e00000
#
# download area is 0x5000c000

sinclude $(OBJTREE)/board/$(BOARDDIR)/config.tmp

ifndef CONFIG_NAND_SPL
ifndef CONFIG_MMC_SPL
CONFIG_SYS_TEXT_BASE = $(RAM_TEXT)
else
CONFIG_SYS_TEXT_BASE = 0
endif
else
CONFIG_SYS_TEXT_BASE = 0
endif
