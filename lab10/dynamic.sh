#!/bin/bash
echo "Building DYNAMIC"
gcc -c -fPIC mylib.c
gcc -shared -o libmylib.so mylib.o
gcc dynamic.c -L. -lmylib -o dynamic
LD_LIBRARY_PATH=. ./dynamic
