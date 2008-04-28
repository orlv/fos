
include ../config.mk

all: initrd.gz ext2-initrd.gz
	cp $^ $(INSTALLDIR)

genromfs: genromfs.c
	$(ECHO) "Compling image generator"
	gcc genromfs.c -o genromfs -s -Os -DVERSION="\"le\""
initrd.gz: initrd
	cp $< .compressing
#	gzip -9 -f $<
	cp $< $@
	mv .compressing $<
initrd: genromfs
	-strip contents/bin/* -R .comment 2>/dev/null
	-strip contents/usr/bin/* -R .comment 2>/dev/null
	-strip contents/sbin/* -R .comment 2>/dev/null

	./genromfs -f $@ -d contents -V "FOS Initrd" -x .svn


clean:
	rm -f initrd initrd.gz
uninstall:
	rm -f $(INSTALLDIR)/initrd.gz

.PHONY: initrd ext2-initrd

ext2-initrd.gz: ext2-initrd
#	cp $< .compressing
#	gzip -9 -f $<
#	mv .compressing $<
	cp $< $@

ext2-initrd:
	REALSIZE=`LC_ALL=C du -sck contents | grep total | sed -e "s/total//"`; \
	ADDTOROOTSIZE=`if  [ $$REALSIZE -ge 20000 ]; then echo 16384; else echo 2400; fi`; \
	SIZE=`expr $$REALSIZE + $$ADDTOROOTSIZE`; \
	ADDTOINODESIZE=`find contents | wc -l`; \
	INODES=`expr $$ADDTOINODESIZE + 400`; \
	echo $$SIZE blocks, $$INODES inodes ; \
	genext2fs -b $$SIZE -N $$INODES -d contents $@
	