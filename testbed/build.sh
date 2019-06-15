#!/bin/sh
clang main.c ../libs/UI/microui.c -O0 -g -rdynamic -Wall -Wno-missing-braces -Wno-unused-variable -Wno-unused-value -Wno-unused-function -ggdb  -I ../libs/ -lglfw -lm -ldl -lGL -lX11 -lpthread ../libs/gl3w.o  -o main
# -pie -fPIC