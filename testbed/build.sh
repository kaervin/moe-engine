#!/bin/sh
clang main.c -O0 -g -Wall -Wno-missing-braces -Wno-unused-variable -Wno-unused-value -Wno-unused-function -ggdb  -I ../libs/ -lglfw -lm -ldl -lGL -lX11 -lpthread ../libs/gl3w.o ../libs/UI/nuklear.o -o main