include $(SYSTEM_SRC)/mk/nicebuild.mk

 
#-----------------------------------------------------------------------------#

all:    $(OUTPUT)
$(OUTPUT): $(OBJECTS)
	echo \ Linking $(OUTPUT)
	$(LD) $(LDFLAGS) -o $(OUTPUT) $(OBJECTS)

install:
	cp $(OUTPUT) $(TARGETDIR)

clean:
	rm -f $(OBJECTS)
	rm -f $(OUTPUT)

uninstall:
	rm -f $(TARGETDIR)/$(OUTPUT)

#-----------------------------------------------------------------------------#
.PHONY: $(OUTPUT)