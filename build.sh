#!/bin/sh

rm -rf *.o
rm -f underlink

gcc -g -c	main.c		-o main.o
gcc -g -c	bucket.c 	-o bucket.o
gcc -g -c	node.c		-o node.o
gcc -g		main.o bucket.o	node.o \
				-o underlink

./underlink
