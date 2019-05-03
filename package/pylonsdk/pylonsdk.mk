########################################################################
#
# pylonsdk
#
################################################################################

PYLONSDK_VERSION = 5.0.8.10089
PYLONSDK_SITE = $(patsubst %/,%,$(PYLONSDK_PKGDIR))
PYLONSDK_SOURCE = pylonsdk-$(PYLONSDK_VERSION).tar.gz
PYLONSDK_SITE_METHOD = file
PYLONSDK_INSTALL_STAGING = YES

define PYLONSDK_INSTALL_STAGING_CMDS
	$(INSTALL) -D $(@D)/bin/pylon-config $(STAGING_DIR)/opt/pylon5/bin/pylon-config
	cp -r $(@D)/include $(STAGING_DIR)/opt/pylon5
	cp -av -r $(@D)/lib $(STAGING_DIR)/opt/pylon5
endef

define PYLONSDK_INSTALL_TARGET_CMDS
	cp -av $(@D)/lib/*.so $(TARGET_DIR)/usr/lib
	cp -av -r $(@D)/share/pylon $(TARGET_DIR)/usr/share
endef

$(eval $(generic-package))
