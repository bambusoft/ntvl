#############################################################
#
# ntvl
#
#############################################################
NTVL_VERSION = 1.0.0
NTVL_SOURCE = bambusoft-ntvl-$(NTVL_VERSION).tar.gz
NTVL_SITE = http://ntvl.bambusoft.mx/downloads

define NTVL_BUILD_CMDS
	$(SED) 's/OPTIONS=/OPTIONS=-static/' $(@D)/Makefile
	$(MAKE1) -C $(@D)
endef

define NTVL_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 0644 $(@D)/config/ntvl-node-default.conf $(TARGET_DIR)/etc/ntvl/ntvl.conf
	$(INSTALL) -D -m 0755 $(@D)/dist/ntvld $(TARGET_DIR)/sbin/ntvld
	$(INSTALL) -D -m 0755 $(@D)/dist/node $(TARGET_DIR)/sbin/edge
	$(INSTALL) -D -m 0755 $(@D)/dist/supernode $(TARGET_DIR)/sbin/supernode
	$(INSTALL) -D -m 0755 $(@D)/dist/tunnel $(TARGET_DIR)/sbin/tunnel
endef

$(eval $(call GENTARGETS))
