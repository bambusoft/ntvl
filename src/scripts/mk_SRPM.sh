#!/bin/sh

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

test -f ${TARFILE}

echo "Building SRPM"
# -ts means build source RPM from tarfile
rpmbuild -ts ${TARFILE}

echo "Done"
