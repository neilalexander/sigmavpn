#!/bin/sh
rm -rf build
mkdir build
gcc -g -c -o build/main.o main.c
gcc -g -c -o build/modules.o modules.c
gcc -g -c -o build/types.o types.c
gcc -g -dynamiclib -o build/proto_raw.o proto/proto_raw.c
gcc -g -dynamiclib -o build/proto_nacl0.o proto/proto_nacl0.c lib/libnacl.a
gcc -g -dynamiclib -o build/intf_dummy.o intf/intf_dummy.c
gcc -g -dynamiclib -o build/intf_tuntap.o intf/intf_tuntap.c
gcc -g -dynamiclib -o build/intf_udp.o intf/intf_udp.c
gcc -g -o build/sigma build/main.o build/modules.o build/types.o
export INTERFACE=/dev/tun0
export REMOTE_ADDRESS=1.3.9.2
export REMOTE_PORT=1000
export LOCAL_ADDRESS=1.3.9.2
export LOCAL_PORT=1001
./build/sigma
