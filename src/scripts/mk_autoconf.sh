#!/bin/sh
NTVL_DIR=../../../ntvl
NTVL="ntvl"
NTVL_VERSION="1.0.0"
NTVL_MAIL="collab@bambusoft.com"
echo "Generating configuration package"
autoscan $NTVL_DIR
mv $NTVL_DIR/configure.scan $NTVL_DIR/configure.ac
sed -i 	-e "s/FULL-PACKAGE-NAME/$NTVL/" \
		-e "s/VERSION/$NTVL_VERSION/" \
		-e "s/BUG-REPORT-ADDRESS/$NTVL_MAIL/" \
		$NTVL_DIR/configure.ac
autoconf $NTVL_DIR/configure.ac > $NTVL_DIR/configure
chmod +x $NTVL_DIR/configure
