#----------------------------------------------------------------------------#
#	Copyright (C) 2007 Oleg Fedorov
#----------------------------------------------------------------------------#

include ../config.mk

INSTALLDIR	=	$(LIB)

OUTPUT		=	libpci.a

OBJECTS		=	pci_outb.o	pci_outw.o	\
			pci_outl.o	pci_inb.o	\
			pci_inw.o	pci_inl.o	\
			pci_check.o

all:	$(OBJECTS)
	$(AR) rc $(OUTPUT) $(OBJECTS)
	cp $(OUTPUT) $(INSTALLDIR)/$(OUTPUT)

install:
	cp $(OUTPUT) $(INSTALLDIR)/$(OUTPUT)

#--------------------------------- Clean -------------------------------------#

clean:
	@rm -f $(OBJECTS)
	@rm -f $(OUTPUT)

uninstall:
	rm -f $(INSTALLDIR)/$(OUTPUT)

#-----------------------------------------------------------------------------#
