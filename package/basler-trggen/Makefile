# 
# Makefile for building out of tree kernel modules
#

# PetaLinux-related stuff
ifndef PETALINUX
$(error You must source the petalinux/settings.sh script before working with PetaLinux)
endif

-include modules.common.mk

.PHONY: all build clean help modules_copy_header modules_build install \
	udev_install modules_install_header modules_install check_rootfs \
	check_stagefs check_kernel

MODULE_HEADERS :=	uapi/misc/basler/trggen.h
KERNEL_BUILD :=		$(PROOT)/build/$(LINUX_KERNEL)

LOCALPWD :=	$(shell pwd)
obj-m +=	basler-trggen.o

all:	build install

build:	modules_copy_header modules_build

modules_copy_header: check_stagefs
	# Install headers
	mkdir -p $(STAGEDIR)/usr/include
	for h in $(MODULE_HEADERS); do \
		cp -a --parents $$h $(STAGEDIR)/usr/include; \
	done

modules_build: check_kernel
	$(MAKE) INSTANCE=$(LINUX_KERNEL) -C $(KERNEL_BUILD) \
		M=$(LOCALPWD) modules_only;

install: modules_install_header modules_install udev_install

modules_install_header: check_rootfs
	# Install headers
	mkdir -p $(TARGETDIR)/usr/include
	for h in $(MODULE_HEADERS); do \
		$(TARGETINST) -d $$h /usr/include/$$h; \
	done

modules_install: $(addprefix $(DIR),$(subst .o,.ko,$(obj-m))) check_kernel
	$(MAKE)	INSTANCE=$(LINUX_KERNEL) -C $(KERNEL_BUILD) \
		M=$(LOCALPWD) INSTALL_MOD_PATH=$(TARGETDIR) \
		modules_install_only;

udev_install: check_rootfs
	# Install udev stuff
	test -d $(TARGETDIR)/etc/udev/rules.d || mkdir -p $(TARGETDIR)/etc/udev/rules.d
	$(TARGETINST) -d 50-trggen.rules /etc/udev/rules.d/50-trggen.rules

check_rootfs:
	# Check if rootfs was already build
	if [ ! -f "$(TARGETDIR)/etc/timestamp" ]; then \
		echo "ERROR: Failed to build module ${INSTANCE} because rootfs hasn't been built."; \
		echo "ERROR: Please build rootfs with petalinux-build -c rootfs first."; \
		exit 255; \
	fi

check_stagefs:
	# Check if build root was already created
	if [ ! -f "$(STAGEDIR)/etc/timestamp" ]; then \
		echo "ERROR: Failed to build module ${INSTANCE} because rootfs hasn't been built."; \
		echo "ERROR: Please build rootfs with petalinux-build -c rootfs first."; \
		exit 255; \
	fi

check_kernel:
	if [ ! -f "$(PROOT)/build/$(LINUX_KERNEL)/link-to-kernel-build/Module.symvers" ]; then \
		echo "ERROR: Failed to install module ${INSTANCE} because kernel hasn't been built."; \
		echo "ERROR: Please build kernel with petalinux-build -c kernel first."; \
		exit 255; \
	fi

clean:
	$(MAKE)	INSTANCE=$(LINUX_KERNEL) -C $(KERNEL_BUILD) \
		M=$(LOCALPWD) clean

help:
	@echo ""
	@echo "Quick reference for various supported build targets for $(INSTANCE)."
	@echo "----------------------------------------------------"
	@echo "  clean                  clean out build objects"
	@echo "  all                    build $(INSTANCE) and install to rootfs host copy"
	@echo "  build                  build subsystem"
	@echo "  install                install built objects to rootfs host copy"

