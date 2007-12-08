#
# Portable Graphics System
#       GUI system
# Copyright (c) 2007 Grindars
include ../../config.mk
include config.mk

CFLAGS+=-I../include --std=c99

all: $(OBJECTS_AL) #$(addsuffix .S,$(basename $(OBJECTS_AL)))
	echo $^
	#false
clean:
	rm -f $(OBJECTS_AL)

%.S: %.c
	$(CC) $(CFLAGS) -S $< -o $@