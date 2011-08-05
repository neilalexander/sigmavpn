#!/bin/sh
rm -rf build
mkdir build

if [ "$(uname -s)" = "Darwin" ]; then
	export CCFLAGS=-g
	export DYFLAGS=-dynamiclib
else
	export DYFLAGS=-shared
fi

gcc 	$CCFLAGS -c 		-o build/main.o main.c
gcc	$CCFLAGS -c 		-o build/modules.o modules.c
gcc 	$CCFLAGS -c 		-o build/types.o types.c
gcc 	$CCFLAGS $DYFLAGS 	-o build/proto_raw.o proto/proto_raw.c
gcc 	$CCFLAGS $DYFLAGS 	-o build/proto_nacl0.o proto/proto_nacl0.c lib/libnacl.a
gcc 	$CCFLAGS $DYFLAGS 	-o build/intf_dummy.o intf/intf_dummy.c
gcc 	$CCFLAGS $DYFLAGS 	-o build/intf_tuntap.o intf/intf_tuntap.c
gcc 	$CCFLAGS $DYFLAGS 	-o build/intf_udp.o intf/intf_udp.c
gcc 	$CCFLAGS -ldl 		-o build/sigma build/main.o build/modules.o build/types.o
