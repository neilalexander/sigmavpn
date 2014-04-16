#!/bin/bash
mkdir -p build

if [ "$(uname -s)" = "Darwin" ];
then
	export CCFLAGS="-g"
	export DYFLAGS="-dynamiclib"
	export LDFLAGS="-ldl -lpthread"
elif [ "$(uname -s)" = "FreeBSD" ];
then
	export CCFLAGS="-g"
	export DYFLAGS="-shared"
	export LDFLAGS="-lpthread"
else
	export CCFLAGS="-g"
	export DYFLAGS="-shared"
	export LDFLAGS="-ldl -lpthread"
fi

if [ "$(uname -m)" = "amd64" -o "$(uname -m)" = "x86_64" ];
then
	export CCFLAGS="${CCFLAGS} -fPIC"
fi

SODIUM_LDFLAGS="-lsodium"
LDFLAGS="${LDFLAGS} ${SODIUM_LDFLAGS}"
DYFLAGS="${DYFLAGS} ${SODIUM_LDFLAGS}"

gcc 	$CCFLAGS -c 		-o build/main.o main.c
gcc	$CCFLAGS -c 		-o build/modules.o modules.c
gcc 	$CCFLAGS -c 		-o build/types.o types.c
gcc	$CCFLAGS -c		-o build/ini.o dep/ini.c
gcc 	$CCFLAGS $DYFLAGS 	-o build/proto_raw.o proto/proto_raw.c

gcc     $CCFLAGS $DYFLAGS -o build/proto_nacl0.o proto/proto_nacl0.c build/types.o
gcc	$CCFLAGS $DYFLAGS 	-o build/proto_nacltai.o proto/proto_nacltai.c build/types.o
gcc	$CCFLAGS -c		-o build/naclkeypair.o naclkeypair.c

gcc	$CCFLAGS $LDFLAGS		-o build/naclkeypair build/naclkeypair.o

gcc 	$CCFLAGS $DYFLAGS 	-o build/intf_dummy.o intf/intf_dummy.c
gcc 	$CCFLAGS $DYFLAGS 	-o build/intf_tuntap.o intf/intf_tuntap.c
gcc 	$CCFLAGS $DYFLAGS 	-o build/intf_udp.o intf/intf_udp.c

gcc 	$CCFLAGS $LDFLAGS	-o build/sigmavpn build/main.o build/modules.o build/types.o build/ini.o
