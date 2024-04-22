#!/bin/bash

## compile shared library mms
gcc -g -fPIC -c mms.c
gcc -g -o libmms.so -shared mms.o
cp libmms.so /usr/lib

## compile and link mmc
gcc -g -L. -o mmc.out mmc.c -lmms


## compile and link test
gcc -g -L. -o test.out test.c -lmms

