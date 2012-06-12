#!/bin/bash

# This script makes a SRPM - a source RPM file which can be built into the
# appropriate distro specific RPM for any platform.
#
# To build the binary package:
# rpm -i ntvl-<ver>.src.rpm
# rpmbuild -bb ntvl.spec
#
# Look for the "Wrote:" line to see where the final RPM is.
#
# To run this script cd to the ntvl directory and run it as follows
# scripts/mk_SRPMS.sh
#

set -e

function exit_fail()
{
    echo "$1"
    exit 1
}

PACKAGE="ntvl"
PKG_VERSION="1.0.0"
PKG_AND_VERSION="${PACKAGE}-${PKG_VERSION}"

TEMPDIR="tmp"

SOURCE_MANIFEST="
README.md
node.c
lzoconf.h
lzodefs.h
Makefile
minilzo.c
minilzo.h
ntvl.c
ntvl.h
ntvl_keyfile.c
ntvl_keyfile.h
ntvl.spec
ntvl_transforms.h
ntvl_wire.h
sn.c
transform_aes.c
transform_null.c
transform_tf.c
tuntap_linux.c
tuntap_freebsd.c
tuntap_netbsd.c
tuntap_osx.c
twofish.c
twofish.h
version.c
wire.c
node.8
supernode.1
ntvl-v1.0.0
debian/changelog
debian/compat
debian/control
debian/copyright
debian/ntvl-node.docs
debian/ntvl-node.install
debian/ntvl-supernode.install
debian/ntvl-node.manpages
debian/ntvl-supernode.manpages
debian/README.Debian
debian/rules
docs/HACKING
docs/INSTALL
docs/LICENSE
docs/USAGE
"

BASE=`pwd`

for F in ${SOURCE_MANIFEST}; do
    test -e $F || exit_fail "Cannot find $F. Maybe you're in the wrong directory. Please execute from ntvl directory."; >&2
done

echo "Found critical files. Proceeding." >&2

if [ -d ${TEMPDIR} ]; then
    echo "Removing ${TEMPDIR} directory"
    rm -rf ${TEMPDIR} >&2
fi

mkdir ${TEMPDIR} >&2

pushd ${TEMPDIR} >&2

echo "Creating staging directory ${PWD}/${PKG_AND_VERSION}" >&2

if [ -d ${PKG_AND_VERSION} ] ; then
    echo "Removing ${PKG_AND_VERSION} directory"
    rm -rf ${PKG_AND_VERSION} >&2
fi

mkdir ${PKG_AND_VERSION}

pushd ${BASE} >&2

echo "Copying in files" >&2
for F in ${SOURCE_MANIFEST}; do
    cp --parents -a $F ${TEMPDIR}/${PKG_AND_VERSION}/
done

popd >&2

TARFILE="${PKG_AND_VERSION}.tar.gz"
echo "Creating ${TARFILE}" >&2
tar czf ${BASE}/${TARFILE} ${PKG_AND_VERSION}

popd >&2

rm -rf ${TEMPDIR} >&2

echo ${BASE}/${TARFILE}
