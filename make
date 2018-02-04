#!/bin/bash
#This executable builds the text editor application KILO.

cflags="-Wall -Wextra -pedantic -Wno-multichar -std=c11"
option=""
platform_file=""

if [ "$1" == "linux" ]; then
	platform_file="X_kilo.c"
#elif <...>
fi

if [ "$2" == "debug" ]; then
	option="-DDEBUG=1 -g"
elif [ "$2" == "performance" ]; then
	option="-O1 -finline"
fi

if [ ! -d "build" ]; then
	mkdir build
else
	if [ -f "kilo" ]; then
		echo "removed kilo"
		rm "kilo"
	fi
	gcc $dbg $platform_file -o build/kilo $cflags
fi
