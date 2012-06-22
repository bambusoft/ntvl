
NTVL_VERSION=1.0.0
NTVL_OSNAME=$(shell uname -s)

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

#NTVL_OPTION_AES?="yes"
NTVL_OPTION_AES=no

ifeq ($(NTVL_OPTION_AES), "yes")
    NTVL_DEFINES+="-DNTVL_HAVE_AES"
    LIBS_NODE_OPT+=-lcrypto
endif

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
SRCDIR=./src
INCDIR=$(SRCDIR)/include
DOCMANDIR=./docs/man
DSTDIR=./dist
ETCDIR=/etc/ntvl
LOGDIR=/var/log/ntvl
RUNDIR=/var/run/ntvl

INCLUDE=-I$(INCDIR)

CFLAGS+=$(DEBUG) $(OPTIMIZATION) $(WARN) $(OPTIONS) $(PLATOPTS) $(NTVL_DEFINES) $(INCLUDE)

NTVL_LIB=ntvl.a
NTVL_OBJS=ntvl.o ntvl_keyfile.o wire.o minilzo.o twofish.o \
         transform_null.o transform_tf.o transform_aes.o \
         tuntap_freebsd.o tuntap_netbsd.o tuntap_linux.o tuntap_osx.o version.o
NTVL_COBJS=$(addprefix $(SRCDIR)/, $(NTVL_OBJS) )

LIBS_NODE+=$(LIBS_NODE_OPT)
LIBS_SN=

#For OpenSolaris (Solaris too?)
ifeq ($(shell uname), SunOS)
LIBS_NODE+=-lsocket -lnsl
LIBS_SN+=-lsocket -lnsl
endif

APPS=node
APPS+=supernode
APPS+=tunnel
APPS+=ntvld

MANFILES=$(addprefix $(DOCMANDIR)/, node.8.gz supernode.1.gz ntvl-v1.0.0.gz tunnel.1.gz)

.PHONY: directories

all: directories $(APPS) $(MANFILES)

directories:
	@echo "Creating build directory"
	@$(MKDIR) $(DSTDIR)

node: $(SRCDIR)/node.c $(NTVL_LIB) $(INCDIR)/ntvl_wire.h $(INCDIR)/ntvl.h Makefile
	@echo "Compiling node"
	$(CC) $(CFLAGS) $(SRCDIR)/node.c $(NTVL_LIB) $(LIBS_NODE) -o $(DSTDIR)/node
	@echo "done..."

test: $(SRCDIR)/test.c $(NTVL_LIB) $(INCDIR)/ntvl_wire.h $(INCDIR)/ntvl.h Makefile
	@echo "Compiling test"
	$(CC) $(CFLAGS) $(SRCDIR)/test.c $(NTVL_LIB) $(LIBS_NODE) -o $(DSTDIR)/test

supernode: $(SRCDIR)/sn.c $(NTVL_LIB) $(INCDIR)/ntvl.h Makefile
	@echo "Compiling supernode"
	$(CC) $(CFLAGS) $(SRCDIR)/sn.c $(NTVL_LIB) $(LIBS_SN) -o $(DSTDIR)/supernode
	
tunnel: $(SRCDIR)/tunnel
	@cp $< $(DSTDIR)/$@
	@chmod a+x $(DSTDIR)/$@
	
ntvld: $(SRCDIR)/ntvld.c $(INCDIR)/minIni.h Makefile
	@echo "Compiling ntvld"
	$(CC) $(CFLAGS) $(SRCDIR)/minIni.c $(SRCDIR)/ntvld.c -o $(DSTDIR)/ntvld

benchmark: $(SRCDIR)/benchmark.c $(NTVL_LIB) $(INCDIR)/ntvl_wire.h $(INCDIR)/ntvl.h Makefile
	@echo "Compiling benchmark"
	$(CC) $(CFLAGS) $(SRCDIR)/benchmark.c $(NTVL_LIB) $(LIBS_SN) -o $(DSTDIR)/benchmark
	
.c.o: $(INCDIR)/ntvl.h $(INCDIR)/ntvl_keyfile.h $(INCDIR)/ntvl_transforms.h $(INCDIR)/ntvl_wire.h $(INCDIR)/twofish.h Makefile
	@echo "Compiling $@"
	$(CC) $(CFLAGS) -DNTVL_VERSION='"$(NTVL_VERSION)"' -DNTVL_OSNAME='"$(NTVL_OSNAME)"' -c $< 

%.gz : %
	@echo "Compressing $@"
	gzip -c $< > $@

$(NTVL_LIB): $(NTVL_COBJS)
	@echo "Compiling ${NTVL_LIB}"
	ar rcs $(NTVL_LIB) $(NTVL_OBJS)
#	$(RANLIB) $@

install: $(DSTDIR)/node $(DSTDIR)/supernode $(DSTDIR)/ntvld $(DOCMANDIR)/node.8.gz $(DOCMANDIR)/supernode.1.gz $(DOCMANDIR)/tunnel.1.gz $(DOCMANDIR)/ntvl-v1.0.0.gz
	@echo "MANDIR=$(MANDIR)"
	$(MKDIR) $(SBINDIR) $(MAN1DIR) $(MAN7DIR) $(MAN8DIR) $(ETCDIR) $(LOGDIR) $(RUNDIR)
	$(INSTALL_PROG) $(DSTDIR)/supernode $(SBINDIR)/
	$(INSTALL_PROG) $(DSTDIR)/node $(SBINDIR)/
	$(INSTALL_PROG) $(DSTDIR)/ntvld $(SBINDIR)/
	$(INSTALL_PROG) $(DSTDIR)/tunnel $(SBINDIR)/
	$(INSTALL_DOC) $(DOCMANDIR)/node.8.gz $(MAN8DIR)/
	$(INSTALL_DOC) $(DOCMANDIR)/supernode.1.gz $(MAN1DIR)/
	$(INSTALL_DOC) $(DOCMANDIR)/tunnel.1.gz $(MAN1DIR)/
	$(INSTALL_DOC) $(DOCMANDIR)/ntvl-v1.0.0.gz $(MAN7DIR)/
	$(INSTALL_DOC) config/ntvl-default.conf $(ETCDIR)/ntvl.conf
	@touch $(LOGDIR)/ntvl.log

clean:
	@echo "Cleaning ntvl building environment"
	@rm -rf $(DSTDIR) ./*.o $(NTVL_LIB) $(APPS) $(MANFILES) test *.dSYM *~

uninstall:
# SBINDIR and MAN?DIR preserved
	@echo "Uninstaling ntvl components"
	@rm -f $(SBINDIR)/node
	@rm -f $(SBINDIR)/supernode
	@rm -f $(SBINDIR)/ntvld 
	@rm -f $(SBINDIR)/tunnel
	@rm -f $(MAN8DIR)/node.8.gz
	@rm -f $(MAN1DIR)/supernode.1.gz
	@rm -f $(MAN1DIR)/tunnel.1.gz
	@rm -f $(MAN7DIR)/ntvl-v1.0.0.gz 
	@rm -rf $(ETCDIR) $(RUNDIR)
	@echo "Preserving $(SBINDIR), $(MAN1DIR), $(MAN7DIR), $(MAN8DIR)"
	
help:
	@echo ""
	@echo "all 		- compile all programs"
	@echo "node		- compile node program"
	@echo "supernode	- compile supernode program"
	@echo "ntvld		- compile ntvld program"
	@echo "clean		- clean compile environment "
	@echo "install		- put executables in /usr/sbin and manuals on /usr/share/man"
	@echo "uninstall	- remove executables an manuals from directories"
	@echo ""
