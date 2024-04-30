#!/bin/bash

## compile shared library mms
g++ -g -fPIC -c mms.cpp
g++ -g -o libmms.so -shared mms.o
cp libmms.so /usr/lib

## compile and link mmc
g++ -g -L. -o mmc mmc.cpp -lmms


## compile and link tests
g++ -g -L. -o test1 test1.cpp -lmms
g++ -g -L. -o test2 test2.cpp -lmms
g++ -g -L. -o test3 test3.cpp -lmms
g++ -g -L. -o test4 test4.cpp -lmms

