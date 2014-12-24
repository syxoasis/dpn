INSTALLDIR ?= /usr/local
BINDIR ?= $(INSTALLDIR)/bin
SYSCONFDIR ?= $(INSTALLDIR)/etc
LIBEXECDIR ?= $(INSTALLDIR)/lib/sigmavpn

SODIUM_CPPFLAGS ?= -I/usr/local/include
SODIUM_LDFLAGS ?= -L/usr/local/lib -lsodium
CFLAGS ?= -O2 -fPIC -Wall -Wextra
CPPFLAGS += $(SODIUM_CPPFLAGS)
LDFLAGS += $(SODIUM_LDFLAGS) -ldl -pthread
DYLIB_CFLAGS ?= $(CFLAGS) -shared

TARGETS_OBJS = bucket.o key.o main.o message.o node.o proto.o uint128.o
TARGETS_BIN = underlink

TARGETS = $(TARGETS_OBJS) $(TARGETS_BIN)

all: $(TARGETS)

clean:
	rm -f $(TARGETS)

distclean: clean

install: all
	mkdir -p $(BINDIR) $(SYSCONFDIR) $(LIBEXECDIR)
	cp $(TARGETS_BIN) $(BINDIR)

underlink: bucket.o key.o main.o message.o node.o proto.o uint128.o
	$(CC) -o underlink bucket.o key.o main.o message.o node.o proto.o uint128.o $(LDFLAGS)

%.o: %.c $(HEADERS)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(SODIUM_CPPFLAGS) -c $< -o $@
