include $(SYSTEM_SRC)/mk/nicebuild.mk

all:	$(OUTPUT)
$(OUTPUT): $(OBJECTS)
	$(ECHO) Creating $@
	$(AR) rcs $(OUTPUT) $(OBJECTS)
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

