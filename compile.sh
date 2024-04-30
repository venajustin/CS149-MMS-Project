#!/bin/bash

## compile shared library mms
g++ -g -fPIC -c mms.cpp
g++ -g -o libmms.so -shared mms.o
cp libmms.so /usr/lib

## compile and link mmc
g++ -g -L. -o mmc.out mmc.cpp -lmms


## compile and link tests
g++ -g -L. -o test.out test.cpp -lmms
g++ -g -L. -o test2.out test2.cpp -lmms
g++ -g -L. -o test3.out test3.cpp -lmms
g++ -g -L. -o test4.out test4.cpp -lmms

