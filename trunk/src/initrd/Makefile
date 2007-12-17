include ../config.mk
include ../usermode/config.mk
all: initrd.gz
	cp $< $(INSTALLDIR)
genromfs: genromfs.c
	$(ECHO) "Compling image generator"
	gcc genromfs.c -o genromfs -s -Os -DVERSION="\"le\""
initrd.gz: initrd
	cp $< .compressing
	gzip -9 -f $<
	mv .compressing $<
initrd: genromfs
	./genromfs -f $@ -d contents -V "FOS Initrd" -x .svn
	strip contents/bin/* -R .comment
	strip contents/usr/bin/* -R .comment
clean:
	rm -f initrd initrd.gz
uninstall:
	rm -f $(INSTALLDIR)/initrd.gz
.PHONY: initrd