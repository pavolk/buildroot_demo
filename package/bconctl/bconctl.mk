########################################################################
#
# bconctl
#
################################################################################

BCONCTL_VERSION = 1.0
BCONCTL_SITE = $(patsubst %/,%,$(BCONCTL_PKGDIR))
BCONCTL_SITE_METHOD = local
BCONCTL_INSTALL_TARGET = YES
BCONCTL_DEPENDENCIES = libbconctl

define BCONCTL_BUILD_CMDS
	cp -f $(BCONCTL_PKGDIR)/Makefile $(@D)
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) CC="$(TARGET_CC)" CXX="$(TARGET_CXX)" LD="$(TARGET_LD)"
endef

define BCONCTL_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 555 $(@D)/bconctl $(TARGET_DIR)/usr/bin
endef


$(eval $(generic-package))
