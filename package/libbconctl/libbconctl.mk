########################################################################
#
# libbconctl
#
################################################################################

LIBBCONCTL_VERSION = 1.0
LIBBCONCTL_SITE = $(patsubst %/,%,$(LIBBCONCTL_PKGDIR))
LIBBCONCTL_SITE_METHOD = local
LIBBCONCTL_INSTALL_STAGING = YES
LIBBCONCTL_INSTALL_TARGET = YES
LIBBCONCTL_DEPENDENCIES = basler-trggen

define LIBBCONCTL_BUILD_CMDS
	cp -f $(LIBBCONCTL_PKGDIR)/Makefile $(@D)
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) CC="$(TARGET_CC)" LD="$(TARGET_LD)"
endef

define LIBBCONCTL_INSTALL_STAGING_CMDS
	$(INSTALL) -D -m 644 $(@D)/basler/bconctl.h  $(STAGING_DIR)/usr/include/basler/bconctl.h
	$(INSTALL) -D -m 644 $(@D)/*.a $(STAGING_DIR)/usr/lib
endef


ifeq ($(BR2_PACKAGE_HAS_UDEV),y)
define LIBBCONCTL_SCRIPTS_INSTALL_UDEV_RULES
	$(INSTALL) $(@D)/50-gpio.rules $(TARGET_DIR)/etc/udev/rules.d/50-gpio.rules
	$(INSTALL) $(@D)/50-gpioled.rules $(TARGET_DIR)/etc/udev/rules.d/50-gpioled.rules
endef
endif

define LIBBCONCTL_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 644 $(@D)/*.so.* $(TARGET_DIR)/usr/lib
	$(INSTALL) -D $(@D)/setup_gpio $(TARGET_DIR)/usr/lib/bconctl/setup_gpio
	$(INSTALL) -D $(@D)/setup_leds $(TARGET_DIR)/usr/lib/bconctl/setup_leds
	$(LIBBCONCTL_SCRIPTS_INSTALL_UDEV_RULES)
endef

$(eval $(generic-package))
