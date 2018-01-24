#!/bin/bash
#This executable builds the text editor application KILO.

cflags="-Wall -Wextra -pedantic -Wno-multichar -std=c99"
dbg=""
platform_file=""

if [ "$1" == "terminal" ]; then
	platform_file="terminal_kilo.c"
#elif <...>
fi

if [ "$2" == "debug" ]; then
	dbg="-DINTERNAL=1 -g"
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
