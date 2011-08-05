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
gcc -g -o build/sigma build/main.o build/modules.o build/types.o
export PUBLIC_KEY=609a3d40fedf79e9d7fa157108efe3dde956d4c2415616c19b8625e7ca4e0260
export PRIVATE_KEY=2ad51b014e6114506b11d1abe201ac2bc65f5c38a4d783d2a4d2f1a0e998b20d
./build/sigma
