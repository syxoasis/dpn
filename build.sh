#!/bin/sh

rm -rf *.o
rm -f underlink

gcc -g -c	main.c		-o main.o
gcc -g -c	bucket.c 	-o bucket.o
gcc -g -c	node.c		-o node.o
gcc -g -c 	message.c	-o message.o
gcc -g -c	crypto.c	-o crypto.o
gcc -g 		main.o bucket.o	node.o \
			message.o crypto.o \
				-l crypto \
				-o underlink

./underlink
