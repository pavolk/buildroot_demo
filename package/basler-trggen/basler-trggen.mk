################################################################################
#
# basler-trggen
#
################################################################################

BASLER_TRGGEN_VERSION = 1.0
BASLER_TRGGEN_SITE = $(patsubst %/,%,$(BASLER_TRGGEN_PKGDIR))
BASLER_TRGGEN_SITE_METHOD = local
BASLER_TRGGEN_MODULE_MAKE_OPTS = EXTRA_CFLAGS="-I$(@D)"
BASLER_TRGGEN_INSTALL_STAGING = YES


define BASLER_TRGGEN_INSTALL_STAGING_CMDS
	$(INSTALL) -D -m 644 $(@D)/uapi/misc/basler/trggen.h \
		$(STAGING_DIR)/usr/include/uapi/misc/basler/trggen.h
endef

ifeq ($(BR2_PACKAGE_HAS_UDEV),y)
define BASLER_TRGGEN_SCRIPTS_INSTALL_UDEV_RULES
	$(INSTALL) -D -m 0644 $(@D)/50-trggen.rules \
		$(TARGET_DIR)/etc/udev/rules.d/50-trggen.rules
endef
endif

define BASLER_TRGGEN_INSTALL_TARGET_CMDS
	$(BASLER_TRGGEN_SCRIPTS_INSTALL_UDEV_RULES)
endef

define BASLER_TRGGEN_INSTALL_INIT_SYSV
endef

define BASLERT_TRGGEN_INSTALL_INIT_SYSTEMD
endef

$(eval $(kernel-module))
$(eval $(generic-package))
