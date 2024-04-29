#!/bin/bash

## compile shared library mms
gcc -g -fPIC -c mms.c
gcc -g -o libmms.so -shared mms.o
cp libmms.so /usr/lib

## compile and link mmc
gcc -g -L. -o mmc.out mmc.c -lmms


## compile and link tests
gcc -g -L. -o test.out test.c -lmms
gcc -g -L. -o test2.out test2.c -lmms
gcc -g -L. -o test3.out test3.c -lmms
gcc -g -L. -o test4.out test4.c -lmms

