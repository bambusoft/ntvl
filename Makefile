
NTVL_VERSION=1.0.0
NTVL_OSNAME=$(shell uname -p)

########

CC=gcc
DEBUG?=-g3

#OPTIONS=static
OPTIONS=

#OPTIMIZATION?=-O2
WARN?=-Wall -Wshadow -Wpointer-arith -Wmissing-declarations -Wnested-externs

#Ultrasparc64 users experiencing SIGBUS should try the following gcc options
#(thanks to Robert Gibbon)
PLATOPTS_SPARC64=-mcpu=ultrasparc -pipe -fomit-frame-pointer -ffast-math -finline-functions -fweb -frename-registers -mapp-regs

NTVL_DEFINES=
NTVL_OBJS_OPT=
LIBS_NODE_OPT=

NTVL_OPTION_AES?="yes"
#NTVL_OPTION_AES=no

ifeq ($(NTVL_OPTION_AES), "yes")
    NTVL_DEFINES+="-DNTVL_HAVE_AES"
    LIBS_NODE_OPT+=-lcrypto
endif

CFLAGS+=$(DEBUG) $(OPTIMIZATION) $(WARN) $(OPTIONS) $(PLATOPTS) $(NTVL_DEFINES)

INSTALL=install
MKDIR=mkdir -p

INSTALL_PROG=$(INSTALL) -m755
INSTALL_DOC=$(INSTALL) -m644


# DESTDIR set in debian make system
PREFIX?=$(DESTDIR)/usr
#BINDIR=$(PREFIX)/bin
SBINDIR=$(PREFIX)/sbin
MANDIR?=$(PREFIX)/share/man
MAN1DIR=$(MANDIR)/man1
MAN7DIR=$(MANDIR)/man7
MAN8DIR=$(MANDIR)/man8
RUNDIR=/run/ntvl

NTVL_LIB=ntvl.a
NTVL_OBJS=ntvl.o ntvl_keyfile.o wire.o minilzo.o twofish.o \
         transform_null.o transform_tf.o transform_aes.o \
         tuntap_freebsd.o tuntap_netbsd.o tuntap_linux.o tuntap_osx.o version.o
LIBS_NODE+=$(LIBS_NODE_OPT)
LIBS_SN=

#For OpenSolaris (Solaris too?)
ifeq ($(shell uname), SunOS)
LIBS_NODE+=-lsocket -lnsl
LIBS_SN+=-lsocket -lnsl
endif

APPS=node
APPS+=supernode

MANFILES=node.8.gz supernode.1.gz ntvl-v1.0.0.gz tunnel.1

help:
	@echo ""
	@echo "all 		- compile all programs"
	@echo "node		- compile node program"
	@echo "supernode	- compile supernode program"
	@echo "clean		- clean compile environment "
	@echo "install		- put executables in /usr/sbin and manuals on /usr/share/man"
	@echo "uninstall	- remove executables an manuals from directories"
	@echo ""

all: $(APPS) $(MANFILES)

node: node.c $(NTVL_LIB) ntvl_wire.h ntvl.h Makefile
	$(CC) $(CFLAGS) node.c $(NTVL_LIB) $(LIBS_NODE) -o node

test: test.c $(NTVL_LIB) ntvl_wire.h ntvl.h Makefile
	$(CC) $(CFLAGS) test.c $(NTVL_LIB) $(LIBS_NODE) -o test

supernode: sn.c $(NTVL_LIB) ntvl.h Makefile
	$(CC) $(CFLAGS) sn.c $(NTVL_LIB) $(LIBS_SN) -o supernode

benchmark: benchmark.c $(NTVL_LIB) ntvl_wire.h ntvl.h Makefile
	$(CC) $(CFLAGS) benchmark.c $(NTVL_LIB) $(LIBS_SN) -o benchmark

.c.o: ntvl.h ntvl_keyfile.h ntvl_transforms.h ntvl_wire.h twofish.h Makefile
	$(CC) $(CFLAGS) -c $<

%.gz : %
	gzip -c $< > $@

$(NTVL_LIB): $(NTVL_OBJS)
	ar rcs $(NTVL_LIB) $(NTVL_OBJS)
#	$(RANLIB) $@

version.o: Makefile
	$(CC) $(CFLAGS) -DNTVL_VERSION='"$(NTVL_VERSION)"' -DNTVL_OSNAME='"$(NTVL_OSNAME)"' -c version.c

clean:
	rm -rf $(NTVL_OBJS) $(NTVL_LIB) $(APPS) $(MANFILES) test *.dSYM *~

install: node supernode node.8.gz supernode.1.gz ntvl-v1.0.0.gz
	echo "MANDIR=$(MANDIR)"
	$(MKDIR) $(SBINDIR) $(MAN1DIR) $(MAN7DIR) $(MAN8DIR) $(RUNDIR)
	$(INSTALL_PROG) supernode $(SBINDIR)/
	$(INSTALL_PROG) node $(SBINDIR)/
	$(INSTALL_PROG) tunnel $(SBINDIR)/
	$(INSTALL_DOC) node.8.gz $(MAN8DIR)/
	$(INSTALL_DOC) supernode.1.gz $(MAN1DIR)/
	$(INSTALL_DOC) tunnel.1 $(MAN1DIR)/
	$(INSTALL_DOC) ntvl-v1.0.0.gz $(MAN7DIR)/

uninstall:
# SBINDIR and MAN?DIR preserved
	rm -f $(SBINDIR)/node
	rm -f $(SBINDIR)/supernode
	rm -f $(SBINDIR)/tunnel
	rm -f $(MAN8DIR)/node.8.gz
	rm -f $(MAN1DIR)/supernode.1.gz
	rm -f $(MAN1DIR)/tunnel.1.gz
	rm -f $(MAN7DIR)/ntvl-v1.0.0.gz 
