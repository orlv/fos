#----------------------------------------------------------------------------#
#	Copyright (C) 2007 Oleg Fedorov
#----------------------------------------------------------------------------#

include ../config.mk

INSTALLDIR	=	$(LIB)
OUTPUT		=	libpgs.a
OBJECTS		=	GUIInit.o CreateWindow.o WaitEvent.o \
			DestroyWindow.o RefreshWindow.o GuiEnd.o \
			pixel.o rect.o pstring.o \
			line.o	SetVisible.o ControlsWindow.o \
			MessageLoop.o drawing.o Controls.o \
			ScreenInfo.o GetDrawingHandle.o

all:	$(OBJECTS)
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
