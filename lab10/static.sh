#!/bin/bash
echo "Building STATIC"
gcc -c mylib.c
ar cr libmylib.a mylib.o
gcc -static static.c -L. -lmylib -o static
./static
