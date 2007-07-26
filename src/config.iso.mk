# -----------------------------------------------------------------------------
# Copyright (C) 2007 Oleg Fedorov
# -----------------------------------------------------------------------------

ISO_BUILD_DIR   = $(SYSTEM_ROOT)/tmp/iso.tmp
ISO_RELEASE_DIR = $(SYSTEM_ROOT)/tmp/iso
ISO_NAME	= $(SYSTEM_NAME)-current.iso
ISO_OUTPUT	= $(ISO_RELEASE_DIR)/$(ISO_NAME)

iso: clean_iso
	mkdir $(ISO_BUILD_DIR)
	mkdir $(ISO_RELEASE_DIR)
	cp -r $(SYSTEM_ROOT)/boot $(ISO_BUILD_DIR)/
	rm -rf `find $(ISO_BUILD_DIR)/iso.image/ -name .svn`

	mkisofs -b boot/grub/stage2_eltorito -no-emul-boot -boot-load-size 4 -boot-info-table \
		-iso-level 3 -r -J \
		-publisher "O.S.Fedorov" \
		-o $(ISO_OUTPUT) $(ISO_BUILD_DIR)

	bzip2 -kf $(ISO_OUTPUT)
	rm -f $(SYSTEM_ROOT)/src/$(ISO_NAME)
	ln -s $(ISO_OUTPUT) $(SYSTEM_ROOT)/src/$(ISO_NAME)

clean_iso:
	rm -rf $(ISO_BUILD_DIR)
	rm -rf $(ISO_RELEASE_DIR)

# -----------------------------------------------------------------------------
