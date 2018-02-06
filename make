#!/bin/bash
#This executable builds the text editor application KILO.

cflags="-Wall -Wextra -pedantic -Wno-multichar -std=c11"
option=""
platform_file=""

if [ "$1" == "linux" ]; then
	platform_file="X_kilo.c"
#elif <...>
fi

case "$2" in
"debug")
	option+="-DDEBUG=1 -g"
;;
"profile")
	option+="-pg "
;&
"optimize")
	option+="-O1 -finline -funroll-loops"
esac

if [ ! -d "build" ]; then
	mkdir build
else
	if [ -f "kilo" ]; then
		echo "removed kilo"
		rm "kilo"
	fi
	gcc $option $platform_file -o build/kilo $cflags
fi
