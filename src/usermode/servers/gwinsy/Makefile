# Copyright (C) 2008 Sergey Gridassov
SHELL=/bin/bash

all: check_config
	make -f Makefile.real all
clean: check_config
	make -f Makefile.real clean
install: check_config
	make -f Makefile.real install
uninstall: check_config
	make -f Makefile.real uninstall


check_config: .config config.h
	( . .config; [ "" != $CONFIG_NEW ] || echo "Warning! Check .config!"; true )

config.h: .config
	echo Generating config.h
	echo "#ifndef CONFIG_H" > config.h
	echo "#define CONFIG_H" >> config.h
	sed 's,#.*,,g;s,\([A-Z_]*\)=\([[:alnum:]]*\),#define \1 \2,g' < .config >> config.h
	echo -e "#endif\n" >> config.h

.config: default.config
	echo "# измененный конфиг из репозитария" >> $@
	cat $^ >> $@
	echo "# после редактирования - удалите" >> $@
	echo "CONFIG_NEW=1" >> $@



.PHONY: check_config all clean install uninstall 