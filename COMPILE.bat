@echo off
set INCLUDE=
set LIBS=
g++ main.cpp -Wall -Wno-unused-result --std=c++17 -O3 %INCLUDE% %LIBS% -lgdi32 -o vox2ss.exe
echo Done!