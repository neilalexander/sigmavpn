INSTALLDIR ?= /usr/local
BINDIR ?= $(INSTALLDIR)/bin
SYSCONFDIR ?= $(INSTALLDIR)/etc
LIBEXECDIR ?= $(INSTALLDIR)/lib/sigmavpn

SODIUM_CPPFLAGS ?= -I/usr/local/include
SODIUM_LDFLAGS ?= -L/usr/local/lib -lsodium
CFLAGS ?= -O2 -fPIC
CPPFLAGS += $(SODIUM_CPPFLAGS)
LDFLAGS += $(SODIUM_LDFLAGS) -pthread -ldl
DYLIB_CFLAGS ?= $(CFLAGS) -shared

TARGETS_OBJS = dep/ini.o main.o modules.o naclkeypair.o types.o
TARGETS_BIN = naclkeypair sigmavpn
TARGETS_MODULES = proto/proto_raw.o proto/proto_nacl0.o proto/proto_nacltai.o \
	intf/intf_dummy.o intf/intf_tuntap.o intf/intf_udp.o

TARGETS = $(TARGETS_OBJS) $(TARGETS_BIN) $(TARGETS_MODULES)

all: $(TARGETS)

clean:
	rm -f $(TARGETS)

distclean: clean

install: all
	mkdir -p $(BINDIR) $(SYSCONFDIR) $(LIBEXECDIR)
	cp $(TARGETS_BIN) $(BINDIR)
	cp $(TARGETS_MODULES) $(LIBEXECDIR)

proto/proto_raw.o: proto/proto_raw.c
	$(CC) $(CPPFLAGS) $(SODIUM_CPPFLAGS) $(DYLIB_CFLAGS) $(SODIUM_LDFLAGS) proto/proto_raw.c -o proto/proto_raw.o

proto/proto_nacl0.o: proto/proto_nacl0.c types.o
	$(CC) $(CPPFLAGS) $(SODIUM_CPPFLAGS) $(DYLIB_CFLAGS) $(SODIUM_LDFLAGS) proto/proto_nacl0.c types.o -o proto/proto_nacl0.o

proto/proto_nacltai.o: proto/proto_nacltai.c types.o
	$(CC) $(CPPFLAGS) $(SODIUM_CPPFLAGS) $(DYLIB_CFLAGS) $(SODIUM_LDFLAGS) proto/proto_nacltai.c types.o -o proto/proto_nacltai.o

intf/intf_dummy.o: intf/intf_dummy.c
	$(CC) $(CPPFLAGS) $(DYLIB_CFLAGS) intf/intf_dummy.c -o intf/intf_dummy.o

intf/intf_tuntap.o: intf/intf_tuntap.c
	$(CC) $(CPPFLAGS) $(DYLIB_CFLAGS) intf/intf_tuntap.c -o intf/intf_tuntap.o

intf/intf_udp.o: intf/intf_udp.c
	$(CC) $(CPPFLAGS) $(DYLIB_CFLAGS) intf/intf_udp.c -o intf/intf_udp.o

naclkeypair: naclkeypair.o
	$(CC) $(LDFLAGS) $(SODIUM_LDFLAGS) -o naclkeypair naclkeypair.o

sigmavpn: main.o modules.o types.o dep/ini.o
	$(CC) $(LDFLAGS) $(SODIUM_LDFLAGS) -o sigmavpn main.o modules.o types.o dep/ini.o

%.o: %.c $(HEADERS)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@
