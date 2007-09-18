# -----------------------------------------------------------------------------
# Automatically generated, do not edit!
# Copyright (C) 2007 Oleg Fedorov
# -----------------------------------------------------------------------------
SYSTEM_NAME     = FOS
SYSTEM_ROOT	= /home/oleg/fos/src/..
SYSTEM_SRC	= /home/oleg/fos/src/../src
BINDIR		= /home/oleg/fos/src/../boot/modules
KERNEL_PATHNAME	= /home/oleg/fos/src/../boot/fos
INCLUDE		= /home/oleg/fos/src/../include
LIB		= /home/oleg/fos/src/../lib

MAKE            = make
ECHO            = /bin/echo -e

# -- Modules ------------------------------------------------------------------
GRUB_MODULES =	/boot/modules/test.txt		\
		/boot/modules/namer		\
		/boot/modules/init		\
		/boot/modules/int16b		\
		/boot/modules/shell		\
		/boot/modules/tty		\
		/boot/modules/keyboard		\
		/boot/modules/floppy		\
		/boot/modules/test		\
		/boot/modules/font.psf		\
		/boot/modules/pci_server


GRUB_MENU    = /home/oleg/fos/src/../boot/grub/menu.lst
GRUB_TFTP_MENU = /home/oleg/fos/src/../boot/grub/tftpmenu.lst
GRUB_TIMEOUT = 0
GRUB_TITLE   = FOS - FOS is Operating System
GRUB_ROOT    = cd0
GRUB_KERNEL  = /boot/fos

# -- User Programms -----------------------------------------------------------
# Базовый адрес пользовательских программ
USER_MEM_BASE = 0x8000000
# -----------------------------------------------------------------------------
