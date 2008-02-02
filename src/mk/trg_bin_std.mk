include $(SYSTEM_SRC)/mk/nicebuild.mk

 
#-----------------------------------------------------------------------------#

all:    $(OUTPUT)
$(OUTPUT): $(OBJECTS)
	echo \ Linking $(OUTPUT)
	$(LD) $(LDFLAGS) -o $(OUTPUT) $(LIB)/crt0.o $(OBJECTS) $(LDLIBS) --entry=_start --oformat=elf32-i386  --Ttext=$(USER_MEM_BASE)

install:
	cp $(OUTPUT) $(TARGETDIR)
	@strip $(TARGETDIR)/$(OUTPUT)

clean:
	rm -f $(OBJECTS)
	rm -f $(OUTPUT)

uninstall:
	rm -f $(TARGETDIR)/$(OUTPUT)

#-----------------------------------------------------------------------------#
.PHONY: $(OUTPUT)
