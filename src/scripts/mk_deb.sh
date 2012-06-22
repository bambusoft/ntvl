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

set -x

BASE=`pwd`

TARFILE=`${BASE}/scripts/mk_tar.sh`
TEMPDIR="build_deb"

test -f ${TARFILE}

echo "Building .deb"

if [ -d ${TEMPDIR} ]; then
    echo "Removing ${TEMPDIR} directory"
    rm -rf ${TEMPDIR} >&2
fi

mkdir ${TEMPDIR}

pushd ${TEMPDIR}

tar xzf ${TARFILE} #At original location

cd ntvl*

dpkg-buildpackage -rfakeroot

popd

echo "Done"
