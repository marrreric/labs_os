#!/bin/bash
echo "Building DLOPEN"
gcc dynamic_dlopen.c -ldl -o dynamic_dlopen
./dynamic_dlopen
