#!/bin/sh
clang++ main.cpp -O0 -g -ggdb -std=c++11 -I ../libs/ -lglfw -lm -ldl -lGL -lX11 -lpthread ../libs/gl3w.o -o main