# -----------------------------------------------------------------------------
# Copyright (C) 2007 Oleg Fedorov
# -----------------------------------------------------------------------------

TMP_DIR         = $(SYSTEM_ROOT)/tmp
ISO_BUILD_DIR   = $(TMP_DIR)/iso.tmp
ISO_RELEASE_DIR = $(TMP_DIR)/iso
ISO_NAME	= $(SYSTEM_NAME)-current.iso
ISO_OUTPUT	= $(ISO_RELEASE_DIR)/$(ISO_NAME)

iso: clean_iso
	mkdir -p $(TMP_DIR)
	mkdir $(ISO_BUILD_DIR)
	mkdir $(ISO_RELEASE_DIR)
	cp -r $(SYSTEM_ROOT)/boot $(ISO_BUILD_DIR)/
	rm -rf `find $(ISO_BUILD_DIR) -name .svn`

	mkisofs -b boot/grub/stage2_eltorito -no-emul-boot -boot-load-size 4 -boot-info-table \
		-iso-level 3 -r -J \
		-publisher "O.S.Fedorov" \
		-o $(ISO_OUTPUT) $(ISO_BUILD_DIR)

#	bzip2 -kf $(ISO_OUTPUT)
	rm -f $(SYSTEM_ROOT)/src/$(ISO_NAME)
	ln -s $(ISO_OUTPUT) $(SYSTEM_ROOT)/src/$(ISO_NAME)

clean_iso:
	rm -rf $(ISO_BUILD_DIR)
	rm -rf $(ISO_RELEASE_DIR)

# -----------------------------------------------------------------------------
