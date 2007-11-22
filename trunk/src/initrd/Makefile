include ../config.mk
include ../usermode/config.mk
all: initrd.gz
	cp $< $(INSTALLDIR)
genromfs: genromfs.c
	$(ECHO) "Compling image generaotr"
	gcc genromfs.c -o genromfs -s -Os -DVERSION="\"le\""
initrd.gz: initrd
	gzip -f initrd
initrd: genromfs
	./genromfs -f $@ -d contents -V "FOS Initrd" -x .svn
clean:
	rm -f initrd initrd.gz
uninstall:
	rm -f $(INSTALLDIR)/initrd.gz