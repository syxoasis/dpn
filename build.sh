#!/bin/sh

rm -rf *.o
rm -f underlink

gcc -g -c	main.c		-o main.o
gcc -g -c	bucket.c 	-o bucket.o
gcc -g -c	node.c		-o node.o
gcc -g -c 	message.c	-o message.o
gcc -g 		main.o bucket.o	node.o message.o \
				-o underlink
#exit
(
sleep 1;
#ifconfig tun0 inet6 add fdfd:1234:5678:4321::/64 
#ifconfig tun0 up
route add -inet6 fdfd:: -prefixlen 16 -interface tun0
) &
./underlink
