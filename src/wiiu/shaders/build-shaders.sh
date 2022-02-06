#!/bin/bash

# to build shaders you need to place a copy of latte-assembler into the current directory
# latte-assembler is part of decaf-emu <https://github.com/decaf-emu/decaf-emu>

# display
./latte-assembler assemble --vsh=display.vsh --psh=display.psh display.gsh
xxd -i display.gsh > display.h

# font_texture
./latte-assembler assemble --vsh=font_texture.vsh --psh=font_texture.psh font_texture.gsh
xxd -i font_texture.gsh > font_texture.h
