################################################################################
#
# xilinx-video
#
################################################################################

XILINX_VIDEO_VERSION = 1.0
XILINX_VIDEO_SITE = $(patsubst %/,%,$(XILINX_VIDEO_PKGDIR))
XILINX_VIDEO_SITE_METHOD = local

$(eval $(kernel-module))
$(eval $(generic-package))
