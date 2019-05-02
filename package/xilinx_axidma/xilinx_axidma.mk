################################################################################
#
# xilinx_axidma
#
################################################################################

XILINX_AXIDMA_VERSION = a994e5d
XILINX_AXIDMA_SITE = https://github.com/pavolk/xilinx_axidma.git
XILINX_AXIDMA_SITE_METHOD = git

#XILINX_AXIDMA_SITE = $(patsubst %/,%,$(XILINX_AXIDMA_PKGDIR))
#XILINX_AXIDMA_SITE_METHOD = local

XILINX_AXIDMA_INSTALL_STAGING = YES
XILINX_AXIDMA_INSTALL_TARGET = YES

XILINX_AXIDMA_MODULE_SUBDIRS = driver
XILINX_AXIDMA_MODULE_MAKE_OPTS = EXTRA_FLAGS="-I$(@D)/include"
XILINX_AXIDMA_POST_EXTRACT_HOOKS += REPLACE_KBUILD

# Replacing Kbuild (we ignore the driver/driver.mk for now)
# We're using our Makefile instead, to work with kernel-module environment.
define REPLACE_KBUILD
	cp $(XILINX_AXIDMA_PKGDIR)/driver/Makefile $(@D)/driver/Kbuild
endef

define XILINX_AXIDMA_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) CC="$(TARGET_CC)" LD="$(TARGET_LD)" library examples
endef

define XILINX_AXIDMA_INSTALL_STAGING_CMDS
	$(INSTALL) -D -m 644 $(@D)/include/*.h  $(STAGING_DIR)/usr/include
	$(INSTALL) -D -m 644 $(@D)/library/libaxidma.so $(STAGING_DIR)/usr/lib
endef

define XILINX_AXIDMA_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 644 $(@D)/library/libaxidma.so $(TARGET_DIR)/usr/lib
endef

$(eval $(kernel-module))
$(eval $(generic-package))
