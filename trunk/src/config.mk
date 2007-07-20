# -----------------------------------------------------------------------------
# Copyright (C) 2007 Oleg Fedorov
# -----------------------------------------------------------------------------

SYSTEM_NAME = FOS
#TOPDIR      = `pwd`/..
MKFLAGS     = MAIN_COMFIG=$(MAIN_CONFIG) TOPDIR=$(TOPDIR)

SYSTEM_ROOT = $(TOPDIR)/root
SYSTEM_SRC  = $(TOPDIR)/src

BINDIR	    = $(SYSTEM_ROOT)/modules

# -- Modules ------------------------------------------------------------------
GRUB_MODULES =	/root/modules/test.txt		\
		/root/modules/namer		\
		/root/modules/init		\
		/root/modules/int16b		\
		/root/modules/shell		\
		/root/modules/tty		\
		/root/modules/keyboard		\
		/root/modules/floppy		\
		/root/modules/test		\
		/root/modules/font.psf


GRUB_MENU    = $(TOPDIR)/boot/grub/menu.lst
GRUB_TFTP_MENU = $(TOPDIR)/boot/grub/tftpmenu.lst
GRUB_TIMEOUT = 0
GRUB_TITLE   = FOS - FOS is Operating System
GRUB_ROOT    = cd0
GRUB_KERNEL  = /root/kernel

# -- User Programms -----------------------------------------------------------
# Базовый адрес пользовательских программ
USER_MEM_BASE = 0x8000000

# -- ISO ----------------------------------------------------------------------
SRC_DIR_NAME    = src
ROOT_DIR_NAME   = root
ISO_BUILD_DIR   = $(TOPDIR)/tmp
ISO_RELEASE_DIR = $(TOPDIR)/iso
# -----------------------------------------------------------------------------
