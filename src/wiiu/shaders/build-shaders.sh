#!/bin/bash

# to build shaders you need to place a copy of latte-assembler into the current directory
# latte-assembler is part of decaf-emu <https://github.com/decaf-emu/decaf-emu>

# display
./latte-assembler assemble --vsh=display.vsh --psh=display.psh display.gsh
xxd -i display.gsh > display.h
sed -i "1s/.*/unsigned char display_gsh[]  __attribute__ ((aligned (512))) = {/" display.h
