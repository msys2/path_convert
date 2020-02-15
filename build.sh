#!/bin/sh

set -e

MSYSROOT2="$(cd / && pwd -W)" &&
MSYSROOT="$(echo "$MSYSROOT2" | sed 's/\//\\\\/g')" || {
	echo "Unable to determine root directory" >&2
	exit 1
}

MSYSROOT=${MSYSROOT%\\\\}
MSYSROOT2=${MSYSROOT2%/}

mkdir -p $PWD/build
CXXFLAGS="-O0 -ggdb -DMSYSROOT=\"$MSYSROOT\" -DMSYSROOT2=\"$MSYSROOT2\""
g++ $CXXFLAGS -c -o $PWD/build/path_conv.o $PWD/src/path_conv.cpp
g++ $CXXFLAGS -c -o $PWD/build/main.o $PWD/src/main.cpp
g++ -o path_conv_test.exe $PWD/build/path_conv.o $PWD/build/main.o

./path_conv_test.exe
