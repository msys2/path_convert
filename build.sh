#!/bin/sh

mkdir -p $PWD/build
CXXFLAGS="-O0 -ggdb"
g++ $CXXFLAGS -c -o $PWD/build/path_conv.o $PWD/src/path_conv.cpp && \
g++ $CXXFLAGS -c -o $PWD/build/main.o $PWD/src/main.cpp && \
g++ -o path_conv_test.exe $PWD/build/path_conv.o $PWD/build/main.o && ./path_conv_test.exe && echo -e "\nGOOD!!!" || echo -e "\nNOT GOOD"
