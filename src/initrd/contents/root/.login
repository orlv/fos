#!/bin/shell
cd /root
set PS1=fos %s $ 
echo Welcome to FOS Operating System

echo Building font cache
export FONTCONFIG_FILE=/etc/fonts.conf
export FC_DEBUG=8191
fc-cache -v