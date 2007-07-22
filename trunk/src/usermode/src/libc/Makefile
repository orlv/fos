
CONFIG      = ../config.mk
MAIN_CONFIG = ../../config.mk

include $(MAIN_CONFIG)
include $(CONFIG)

OBJECTS =	startup.o	\
		fos.o		\
		heap.o		\
		libcpp.o	\
		msg.o		\
		tmutex.o	\
		vsprintf.o

OUTPUT = ../lib/libc.a

all:    $(OBJECTS)
	$(AR) rc $(OUTPUT) $(OBJECTS)

 #--------------------------------- Clean -------------------------------------#

clean:
	@rm -f $(OBJECTS)
	@rm -f $(OUTPUT)

#-----------------------------------------------------------------------------#
