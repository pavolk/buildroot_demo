################################################################################
#
# basler-vsrc
#
################################################################################

BASLER_VSRC_VERSION = 1.0
BASLER_VSRC_SITE = $(patsubst %/,%,$(BASLER_VSRC_PKGDIR))
BASLER_VSRC_SITE_METHOD = local

$(eval $(kernel-module))
$(eval $(generic-package))
