#!/bin/sh

mkdir /usr/local/lib/sigmavpn/
touch /usr/local/etc/sigmavpn.conf

cp build/intf*.o /usr/local/lib/sigmavpn/
cp build/proto*.o /usr/local/lib/sigmavpn/

cp build/sigmavpn /usr/bin/

if [ -f "build/naclkeypair" ]
then
	cp build/naclkeypair /usr/sbin
fi
