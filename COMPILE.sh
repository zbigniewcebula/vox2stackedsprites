#!/bin/bash
INCLUDE="-I/usr/include/libpng16"
LIBS="-L/usr/X11R6/lib -L/usr/local/lib "
LIBS_FLAG="-lm -lpthread -lX11 -lpng"
g++ main.cpp -Wall -Wno-unused-result --std=c++17 -O3 $INCLUDE $LIBS $LIBS_FLAG -o vox2ss
echo Done!
exit