########################################################################
#
# libbconadapterzynq
#
################################################################################

LIBBCONADAPTERZYNQ_VERSION = 1.0
LIBBCONADAPTERZYNQ_SITE = $(patsubst %/,%,$(LIBBCONADAPTERZYNQ_PKGDIR))
LIBBCONADAPTERZYNQ_SITE_METHOD = local
LIBBCONADAPTERZYNQ_INSTALL_TARGET = YES
LIBBCONADAPTERZYNQ_DEPENDENCIES = basler-vsrc pylonsdk

define LIBBCONADAPTERZYNQ_BUILD_CMDS
	cp -f $(LIBBCONADAPTERZYNQ_PKGDIR)/Makefile $(@D)
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) CC="$(TARGET_CC)" CXX="$(TARGET_CXX)" LD="$(TARGET_LD)" PYLON_ROOT=$(STAGING_DIR)/opt/pylon5
endef

define LIBBCONADAPTERZYNQ_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 644 $(@D)/*.so $(TARGET_DIR)/usr/lib
endef


$(eval $(generic-package))
