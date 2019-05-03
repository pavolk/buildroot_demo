ifdef PETALINUX
    include libs.common.mk
    INSTALL_CMD := $(TARGETINST) -d
    SYMLINK_CMD := $(TARGETINST) -s
else
    INSTALL_CMD := install -D
    SYMLINK_CMD := ln -sf
endif

LIB          := libbconctl
LIB_MAJOR    := 1
LIB_MINOR    := 0
LIB_SUBMINOR := 0
LIB_VERSION  := $(LIB_MAJOR).$(LIB_MINOR).$(LIB_SUBMINOR)
LIB_SONAME   := $(LIB).so.$(LIB_MAJOR)
LIB_OBJS     := libbconctl-trggen.o libbconctl-campower.o libbconctl-camaddress.o libbconctl-led.o
SHLIB_OBJS   := $(LIB_OBJS:.o=.os)
LIB_HEADERS  := basler/bconctl.h
LIB_SOURCES  := $(LIB_OBJS:.o=.c) $(LIB_HEADERS) Makefile setup_gpio setup_leds 50-gpio.rules 50-gpioled.rules

CFLAGS       := $(CFLAGS) -Werror -Wall

.PHONY: all build build_copy_header install install_header \
        check_rootfs install_udev clean help

all: build

build: build_copy_header $(LIB).a $(LIB).so.$(LIB_VERSION)
	# Install libraries to STAGEDIR when set
	if [ -n "$(STAGEDIR)" ]; then \
		mkdir -p $(STAGEDIR)/usr/lib; \
		rsync -a *.a $(STAGEDIR)/usr/lib/; \
		rsync -a *.so.* $(STAGEDIR)/usr/lib/; \
	fi

build_copy_header:
	# Install headers to STAGEDIR when set
	if [ -n "$(STAGEDIR)" ]; then \
		mkdir -p $(STAGEDIR)/usr/include; \
		for h in $(LIB_HEADERS); do \
			cp -a --parents $$h $(STAGEDIR)/usr/include; \
		done; \
	fi

$(LIB).a: $(LIB_OBJS)
	$(AR) rcs $@ $^

$(LIB).so.$(LIB_VERSION): $(SHLIB_OBJS)
	$(CC) -shared -Wl,-soname=$(LIB_SONAME) -o $@ $^

%.o: %.c $(LIB_HEADERS)
	$(CC) -c $(CFLAGS) -fno-PIC -o $@ $<

%.os: %.c $(LIB_HEADERS)
	$(CC) -c $(CFLAGS) -fPIC -o $@ $<

install: install_header install_udev $(LIB).a $(LIB).so.$(LIB_VERSION)
	# Install static lib files
	$(INSTALL_CMD) $(LIB).a /usr/lib/$(LIB).a

	# Install shared lib files
	$(INSTALL_CMD) $(LIB).so.$(LIB_VERSION) /usr/lib/$(LIB).so.$(LIB_VERSION)

	# Install shared lib symlinks
	# Cannot use ldconfig here, native ldconfig chokes on ARM binaries!
	$(SYMLINK_CMD) $(LIB).so.$(LIB_VERSION) /usr/lib/$(LIB_SONAME)
	$(SYMLINK_CMD) $(LIB).so.$(LIB_VERSION) /usr/lib/$(LIB).so

ifdef PETALINUX
	# Install source files
	for file in $(LIB_SOURCES); do \
		$(INSTALL_CMD) $$file /opt/bcon/sources/$(LIB)/$$file; \
	done
endif

install_header: check_rootfs
	# Install headers
	for h in $(LIB_HEADERS); do \
		$(INSTALL_CMD) $$h /usr/include/$$h; \
	done

install_udev: check_rootfs
	# Install udev stuff
	$(INSTALL_CMD) setup_gpio /usr/lib/bconctl/setup_gpio
	$(INSTALL_CMD) setup_leds /usr/lib/bconctl/setup_leds
	$(INSTALL_CMD) 50-gpio.rules /etc/udev/rules.d/50-gpio.rules
	$(INSTALL_CMD) 50-gpioled.rules /etc/udev/rules.d/50-gpioled.rules

check_rootfs:
ifdef PETALINUX
	# Check if rootfs has already been built
	if [ ! -f "$(TARGETDIR)/etc/timestamp" ]; then \
		echo "ERROR: Failed to build module ${INSTANCE} because rootfs hasn't been built."; \
		echo "ERROR: Please build rootfs with petalinux-build -c rootfs first."; \
		exit 255; \
	fi
endif

clean:
	rm -f $(LIB).a $(LIB).so.$(LIB_VERSION) $(SHLIB_OBJS) $(LIB_OBJS)

help:
	@echo ""
	@echo "Quick reference for various supported build targets for $(INSTANCE)."
	@echo "----------------------------------------------------"
	@echo "  clean                  clean out build objects"
	@echo "  all                    build $(INSTANCE)"
	@echo "  build                  build subsystem"
	@echo "  install                install built objects to rootfs host copy"
