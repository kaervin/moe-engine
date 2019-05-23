#!/bin/sh
cd "${0%/*}"
clang gamestep.c -fPIC -shared -O0 -g -Wall -Wno-missing-braces -Wno-unused-variable -Wno-unused-value -Wno-unused-function -ggdb -std=gnu99 -I ../../libs/ -I ../../testbed/ -lglfw -lm -ldl -lGL -lX11 -lpthread ../../libs/gl3w.o ../../libs/UI/nuklear.o -o shared.so