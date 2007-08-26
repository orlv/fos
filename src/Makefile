 #------------------------------------------------------------------------------
# FOS - FOS is Operating System
# Copyright (C) 2004-2007 Oleg S. Fedorov
#------------------------------------------------------------------------------

include config.mk
include config.iso.mk

all:	version build_kernel build_user
	@rm -f `find . -name \*~`
	@expr `cat .build.next` + 1 | xargs $(ECHO) -n > .build.next.tmp
	@mv .build.next.tmp .build.next
	@$(ECHO) \ System Complete!
	@$(ECHO) \ Run \"make install\" to build iso image.
	@$(ECHO) \#------------------------------------------------------------------------------

help:
	@$(ECHO) " "
	@$(ECHO) "\"make all\" 		- compile"
	@$(ECHO) "\"make install\"		- install"
	@$(ECHO) "\"make clean\"		- remove all *.o and image files"
	@$(ECHO) " "

version:
	@rm -f kernel/lib/version.o
	@cat .build.next > BUILD
	@$(ECHO) " "
	@$(ECHO) "FOS Version `cat VERSION`. Build `cat BUILD`."
	@$(ECHO) "Copyright (C) 2004-2007 Oleg Fedorov"
	@$(ECHO) " "
	@$(ECHO) "/* " > ../include/fos/version.h
	@$(ECHO) "include/fos/version.h" >> ../include/fos/version.h
	@$(ECHO) "Automatically generated - do not modify" >> ../include/fos/version.h
	@$(ECHO) "Copyright (C) 2004-2007 Oleg Fedorov" >> ../include/fos/version.h
	@$(ECHO) "*/" >> ../include/fos/version.h
	@$(ECHO) " " >> ../include/fos/version.h
	@$(ECHO) "#define VERSION	\"`cat VERSION`\"" >> ../include/fos/version.h
	@$(ECHO) "#define BUILD	`cat BUILD`" >> ../include/fos/version.h


build_kernel:
	@$(ECHO) \#--------------------------------- Building kernel ----------------------------
	@$(MAKE) -s -C kernel
	@$(ECHO) \#------------------------------------------------------------------------------
	@$(ECHO) 

build_user:
	@$(ECHO) \#---------------------- Building usermode programs ----------------------------
	@$(MAKE) -s -C usermode
	@$(ECHO) \#------------------------------------------------------------------------------

#--------------------------------- Clean -------------------------------------

clean_kernel:
	@$(MAKE) -C kernel clean

clean_user:
	@$(MAKE) -C usermode clean uninstall

clean: clean_kernel clean_user

#--------------------------------- Install -----------------------------------

grub_menu:
	@$(ECHO) "# GRUB Config: generated automatically - do not edit!" > $(GRUB_MENU)
	@$(ECHO) "timeout $(GRUB_TIMEOUT)" >> $(GRUB_MENU)
	@$(ECHO) "default 0" >> $(GRUB_MENU)
	@$(ECHO) "hiddenmenu" >> $(GRUB_MENU)
	@$(ECHO) "title $(GRUB_TITLE) (VERSION `cat VERSION`, BUILD `cat BUILD`)" >> $(GRUB_MENU)
	@$(ECHO) "	root($(GRUB_ROOT))" >> $(GRUB_MENU)
	@$(ECHO) "	kernel $(GRUB_KERNEL)" >> $(GRUB_MENU)
	@$(ECHO) $(GRUB_MODULES) | awk 'BEGIN {RS = " "} {a = "\tmodule "$$0; print a}' >> $(GRUB_MENU)

grub_tftp_menu:
	@$(ECHO) "# GRUB Config: generated automatically - do not edit!" > $(GRUB_TFTP_MENU)
	@$(ECHO) "timeout 1" >> $(GRUB_TFTP_MENU)
	@$(ECHO) "default 0" >> $(GRUB_TFTP_MENU)
	@$(ECHO) "title $(GRUB_TITLE) (VERSION `cat VERSION`, BUILD `cat BUILD`)" >> $(GRUB_TFTP_MENU)
	@$(ECHO) "	root	(nd)" >> $(GRUB_TFTP_MENU)
	@$(ECHO) "	kernel $(GRUB_KERNEL)" >> $(GRUB_TFTP_MENU)
	@$(ECHO) $(GRUB_MODULES) | awk 'BEGIN {RS = " "} {a = "\tmodule "$$0; print a}' >> $(GRUB_TFTP_MENU)

install: grub_menu grub_tftp_menu iso

#-----------------------------------------------------------------------------
