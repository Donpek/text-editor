#!/bin/bash
#This executable builds the text editor application KILO.

cflags="-Wall -Wextra -pedantic -std=c99"
def_internal=""
platform_file=""

if [ "$1" == "terminal" ]; then
	platform_file="terminal_kilo.c"
#elif <...>
fi

if [ "$2" == "internal" ]; then
	def_internal="-DINTERNAL=1"
fi

if [ ! -d "build" ]; then
	mkdir build
else
	if [ -f "kilo" ]; then
		echo "removed kilo"	
		rm "kilo"
	fi
	gcc $def_internal $platform_file -o build/kilo $cflags
fi
