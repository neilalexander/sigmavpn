#!/bin/sh
rm -rf build
mkdir build

if [ "$(uname -s)" = "Darwin" ]; then
	export CCFLAGS="-g"
	export DYFLAGS="-dynamiclib"
else
	export CCFLAGS="-g"
	export DYFLAGS="-shared -fPIC"
fi

gcc 	$CCFLAGS -c 		-o build/main.o main.c
gcc	$CCFLAGS -c 		-o build/modules.o modules.c
gcc 	$CCFLAGS -c 		-o build/types.o types.c
gcc	$CCFLAGS -c		-o build/ini.o dep/ini.c
gcc 	$CCFLAGS $DYFLAGS 	-o build/proto_raw.o proto/proto_raw.c

if [ ! -f "./include/crypto_box_curve25519xsalsa20poly1305.h" ]
then
	if [ ! "$1" == "--with-nacl" ]
	then
		echo The nacl encryption library has not been found, so features relying on nacl will not be built.
		echo If you wish to build nacl also, re-run build.sh with the --with-nacl parameter.
	fi
fi

if [ "$1" == "--with-nacl" ]
then
	echo "Building nacl; this will take a while..."
	rm -rf tmp/
	mkdir tmp
	mkdir tmp/nacl
	cd tmp/nacl
	curl -O -q http://hyperelliptic.org/nacl/nacl-20110221.tar.bz2
	bunzip2 -d nacl-20110221.tar.bz2
	tar -xf nacl-20110221.tar --strip-components 1
	./do
	cd ../../
	ABI=`tmp/nacl/build/*/bin/okabi | head -n 1`
	HOST=`hostname -s`
	mkdir lib/
	mkdir include/
	cp -r "tmp/nacl/build/${HOST}/lib/${ABI}/" lib/
	cp -r "tmp/nacl/build/${HOST}/include/${ABI}/" include/
fi

if [ -f "./include/crypto_box_curve25519xsalsa20poly1305.h" ]
then
        gcc     $CCFLAGS $DYFLAGS       -o build/proto_nacl0.o proto/proto_nacl0.c lib/libnacl.a build/types.o
	gcc	$CCFLAGS $DYFLAGS	-o build/proto_nacltai.o proto/proto_nacltai.c lib/libnacl.a build/types.o
	gcc	$CCFLAGS -c		-o build/naclkeypair.o naclkeypair.c
	gcc	$CCFLAGS		-o build/naclkeypair build/naclkeypair.o lib/libnacl.a lib/randombytes.o
fi

gcc 	$CCFLAGS $DYFLAGS 	-o build/intf_dummy.o intf/intf_dummy.c
gcc 	$CCFLAGS $DYFLAGS 	-o build/intf_tuntap.o intf/intf_tuntap.c
gcc 	$CCFLAGS $DYFLAGS 	-o build/intf_udp.o intf/intf_udp.c
gcc 	$CCFLAGS -ldl -lpthread -o build/sigma build/main.o build/modules.o build/types.o build/ini.o
