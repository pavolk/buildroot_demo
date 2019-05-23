########################################################################
#
# axidma_tests
#
################################################################################

AXIDMA_TESTS_VERSION = 1.0
AXIDMA_TESTS_SITE = $(patsubst %/,%,$(AXIDMA_TESTS_PKGDIR))
AXIDMA_TESTS_SITE_METHOD = local
AXIDMA_TESTS_INSTALL_TARGET = YES
AXIDMA_TESTS_DEPENDENCIES = xilinx_axidma

define AXIDMA_TESTS_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) CC="$(TARGET_CC)" LD="$(TARGET_LD)" all
endef

define AXIDMA_TESTS_INSTALL_TARGET_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) install PREFIX=$(TARGET_DIR)
endef

$(eval $(generic-package))

