#!/bin/bash
INCLUDE=""
LIBS="-L/usr/X11R6/lib"
LIBS_FLAG="-lm -lpthread -lX11"
g++ main.cpp -Wall -Wno-unused-result --std=c++17 -O3 $INCLUDE $LIBS $LIBS_FLAG -o _main
echo Done!
exit