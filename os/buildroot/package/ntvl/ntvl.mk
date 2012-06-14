#############################################################
#
# ntvl
#
#############################################################
NTVL_VERSION = 1.0.0-RC
NTVL_SOURCE = bambusoft-ntvl-$(NTVL_VERSION).tar.gz
NTVL_SITE = http://ntvl.bambusoft.mx/downloads

define NTVL_BUILD_CMDS
	$(SED) 's/########/OPTIONS=-static/' $(@D)/Makefile
	$(MAKE1) -C $(@D)
endef

define NTVL_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 0755 $(@D)/edge $(TARGET_DIR)/sbin/edge
	$(INSTALL) -D -m 0755 $(@D)/supernode $(TARGET_DIR)/sbin/supernode
	$(INSTALL) -D -m 0755 $(@D)/tunnel $(TARGET_DIR)/sbin/tunnel
endef

$(eval $(call GENTARGETS))
