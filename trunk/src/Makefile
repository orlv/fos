 #------------------------------------------------------------------------------
# FOS - FOS is Operating System
# Copyright (C) 2004-2007 Oleg S. Fedorov
#------------------------------------------------------------------------------

include config.mk
include config.iso.mk

all:	version build_kernel build_user
	@rm -f `find . -name \*~`
	@expr `cat .build.next` + 1 | xargs /bin/echo -n > .build.next.tmp
	@mv .build.next.tmp .build.next
	@/bin/echo \ System Complete!
	@/bin/echo \ Run \"make install\" to build iso image.
	@/bin/echo \#------------------------------------------------------------------------------

help:
	@/bin/echo " "
	@/bin/echo "\"make all\" 		- compile"
	@/bin/echo "\"make install\"		- install"
	@/bin/echo "\"make clean\"		- remove all *.o and image files"
	@/bin/echo " "

version:
	@rm -f kernel/lib/version.o
	@cat .build.next > BUILD
	@/bin/echo " "
	@/bin/echo "FOS Version `cat VERSION`. Build `cat BUILD`."
	@/bin/echo "Copyright (C) 2004-2007 Oleg Fedorov"
	@/bin/echo " "
	@/bin/echo -e "/* \n\
    include/fos/version.h \n\
    Automatically generated - do not modify \n\
    Copyright (C) 2004-2007 Oleg Fedorov \n\
*/ \n\
\n\
#define VERSION	\"`cat VERSION`\"\n\
#define BUILD	`cat BUILD`" > ../include/fos/version.h


build_kernel:
	@/bin/echo \#--------------------------------- Building kernel ----------------------------
	@$(MAKE) -s -C kernel
	@/bin/echo \#------------------------------------------------------------------------------
	@/bin/echo 

build_user:
	@/bin/echo \#---------------------- Building usermode programs ----------------------------
	@$(MAKE) -s -C usermode
	@/bin/echo \#------------------------------------------------------------------------------

#--------------------------------- Clean -------------------------------------

clean_kernel:
	@$(MAKE) -C kernel clean

clean_user:
	@$(MAKE) -C usermode clean uninstall

clean: clean_kernel clean_user

#--------------------------------- Install -----------------------------------

grub_menu:
	@/bin/echo "# GRUB Config: generated automatically - do not edit!" > $(GRUB_MENU)
	@/bin/echo "timeout $(GRUB_TIMEOUT)" >> $(GRUB_MENU)
	@/bin/echo "default 0" >> $(GRUB_MENU)
	@/bin/echo "hiddenmenu" >> $(GRUB_MENU)
	@/bin/echo "title $(GRUB_TITLE) (VERSION `cat VERSION`, BUILD `cat BUILD`)" >> $(GRUB_MENU)
	@/bin/echo "	root($(GRUB_ROOT))" >> $(GRUB_MENU)
	@/bin/echo "	kernel $(GRUB_KERNEL)" >> $(GRUB_MENU)
	@/bin/echo $(GRUB_MODULES) | sed 's/ /\n/g' | sed 's/\/boot/\tmodule \/boot/g'  >> $(GRUB_MENU)

grub_tftp_menu:
	@/bin/echo "# GRUB Config: generated automatically - do not edit!" > $(GRUB_TFTP_MENU)
	@/bin/echo "timeout 1" >> $(GRUB_TFTP_MENU)
	@/bin/echo "default 0" >> $(GRUB_TFTP_MENU)
	@/bin/echo "title $(GRUB_TITLE) (VERSION `cat VERSION`, BUILD `cat BUILD`)" >> $(GRUB_TFTP_MENU)
	@/bin/echo "	root	(nd)" >> $(GRUB_TFTP_MENU)
	@/bin/echo "	kernel $(GRUB_KERNEL)" >> $(GRUB_TFTP_MENU)
	@/bin/echo $(GRUB_MODULES) | sed 's/ /\n/g' | sed 's/\/boot/\tmodule \/boot/g'  >> $(GRUB_TFTP_MENU)

install: grub_menu grub_tftp_menu iso

#-----------------------------------------------------------------------------
