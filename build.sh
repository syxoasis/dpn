#!/bin/sh

rm -rf *.o
rm -f underlink

if [ ! -f "./include/crypto_box_curve25519xsalsa20poly1305.h" ]
then
	echo "Building nacl; this will take a while..."
	rm -rf tmp/
	mkdir tmp
	mkdir tmp/nacl
	cd tmp/nacl
	curl -O -q http://hyperelliptic.org/nacl/nacl-20110221.tar.bz2
	bunzip2 -d nacl-20110221.tar.bz2
	tar -xf nacl-20110221.tar --strip-components 1

	if [ "$(uname -m)" = "amd64" -o "$(uname -m)" = "x86_64" ];
	then
		rm -r crypto_onetimeauth/poly1305/amd64
		sed -i -e "s/$/ -fPIC/" okcompilers/c
	fi

	./do
	cd ../../
	NACLDIR="tmp/nacl/build/`hostname | sed 's/\..*//' | tr -cd '[a-z][A-Z][0-9]'`"
	ABI=`"${NACLDIR}/bin/okabi" | head -n 1`
	mkdir -p lib/
	mkdir -p include/
	cp ${NACLDIR}/lib/${ABI}/* lib/
	cp ${NACLDIR}/include/${ABI}/* include/
fi

gcc -g -c 	main.c		-o main.o
gcc -g -c 	bucket.c 	-o bucket.o
gcc -g -c 	node.c		-o node.o
gcc -g -c 	message.c	-o message.o
gcc -g -c 	proto.c		-o proto.o
gcc -g -c 	key.c		-o key.o
gcc -g -c 	uint128.c	-o uint128.o
gcc -g 		main.o \
			bucket.o \
			node.o \
			message.o \
			proto.o \
			key.o \
			uint128.o \
			lib/libnacl.a \
			lib/randombytes.o \
				-o underlink

./underlink
