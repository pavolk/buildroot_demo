########################################################################
#
# bcongrab
#
################################################################################

BCONGRAB_VERSION = 1.0
BCONGRAB_SITE = $(patsubst %/,%,$(BCONGRAB_PKGDIR))
BCONGRAB_SITE_METHOD = local
BCONGRAB_INSTALL_TARGET = YES
BCONGRAB_DEPENDENCIES = pylonsdk libbconadapterzynq

define BCONGRAB_BUILD_CMDS
	cp -f $(BCONGRAB_PKGDIR)/Makefile $(@D)
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) CC="$(TARGET_CC)" CXX="$(TARGET_CXX)" LD="$(TARGET_LD)" PYLON_ROOT=$(STAGING_DIR)/opt/pylon5
endef

define BCONGRAB_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 555 $(@D)/bcongrab $(TARGET_DIR)/usr/bin
endef

$(eval $(generic-package))
