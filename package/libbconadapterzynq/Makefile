ifdef PETALINUX
    include libs.common.mk
    INSTALL_CMD := $(TARGETINST) -d
else
    INSTALL_CMD := install -D
endif

# The shared librarys to build
LIB_MMAP   := libBconAdapterZynqMMAP
PYLON_ROOT := /opt/pylon5

CPPFLAGS   := $(CPPFLAGS) -I. $(shell $(STAGEDIR)/$(PYLON_ROOT)/bin/pylon-config --cflags-only-I)
CFLAGS     := $(CFLAGS) -fPIC -fvisibility=hidden -Wall -Werror -fno-strict-aliasing
CXXFLAGS   := $(CXXFLAGS) -fPIC -fvisibility=hidden -Wall -Werror
LDFLAGS    := $(LDFLAGS) -shared -Wl,--no-undefined
LDLIBS     := -lpthread

LIB_HEADER    := MyBconAdapterLogging.h MyBconAdapterZynqSimpleMutex.h \
                 MyBconAdapterZynqMemoryFunc.h MyBconAdapterZynqStream.h
LIB_SRC_C     := MyBconAdapterEnumerator.c MyBconAdapterI2CConnection.c \
                 MyBconAdapterLibrary.c MyBconAdapterLogging.c
LIB_SRC_CPP   := MyBconAdapterZynqStream.cpp \
                 MyBconAdapterZynqMemoryMMAP.cpp
LIB_OBJS_SHRD := $(LIB_SRC_C:.c=.o) MyBconAdapterZynqStream.o
LIB_OBJS_MMAP := MyBconAdapterZynqMemoryMMAP.o
LIB_SOURCES   := $(LIB_SRC_C) $(LIB_SRC_CPP) $(LIB_HEADER) Makefile

.PHONY: all build install clean help

# Makefile for Bcon Adapter sample library
all: build

build: $(LIB_MMAP).so
	# Install libraries to STAGEDIR when set
	if [ -n "$(STAGEDIR)" ]; then \
		mkdir -p $(STAGEDIR)/opt/bcon/lib; \
		rsync -a *.so $(STAGEDIR)/opt/bcon/lib/; \
	fi

# Rules for building 
%.o: %.c $(LIB_HEADER)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

%.o: %.cpp $(LIB_HEADER)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $<

$(LIB_MMAP).so: $(LIB_OBJS_SHRD) $(LIB_OBJS_MMAP)
	$(CXX) $(LDFLAGS) -o $@ $^ $(LDLIBS)

install: install_environment install_x11_environment $(LIB_MMAP).so
	# Install lib files
	$(INSTALL_CMD) $(LIB_MMAP).so /opt/bcon/lib/$(LIB_MMAP).so

ifdef PETALINUX
	# Install source files
	for file in $(LIB_SOURCES); do \
		$(INSTALL_CMD) $$file /opt/bcon/sources/libbconadapterzynq/$$file; \
	done
endif

install_environment:
	# Install BCON defaults into environment
	if grep -q -e "^BCON_ADAPTER_I2C_DEVICES=.*" "$(TARGETDIR)/etc/environment"; then \
		sed -i -e "s@^BCON_ADAPTER_I2C_DEVICES=.*@BCON_ADAPTER_I2C_DEVICES=\"/dev/i2c-0:60\"@g" "$(TARGETDIR)/etc/environment"; \
	else \
		echo "" >> "$(TARGETDIR)/etc/environment"; \
		echo "# BCON default settings" >> "$(TARGETDIR)/etc/environment"; \
		echo "BCON_ADAPTER_I2C_DEVICES=\"/dev/i2c-0:60\"" >> "$(TARGETDIR)/etc/environment"; \
	fi

	if grep -q -e "^BCON_ADAPTER_LIB=.*" "$(TARGETDIR)/etc/environment"; then \
		sed -i -e "s@^BCON_ADAPTER_LIB=.*@BCON_ADAPTER_LIB=\"/opt/bcon/lib/$(LIB_MMAP).so\"@g" "$(TARGETDIR)/etc/environment"; \
	else \
		echo "BCON_ADAPTER_LIB=\"/opt/bcon/lib/$(LIB_MMAP).so\"" >> "$(TARGETDIR)/etc/environment"; \
	fi

install_x11_environment:
	# Install BCON defaults into x11 environment
	if [ -f "$(TARGETDIR)/etc/X11/Xsession.d/90environment" ]; then \
		if grep -q -e "^export BCON_ADAPTER_I2C_DEVICES=.*" "$(TARGETDIR)/etc/X11/Xsession.d/90environment"; then \
			sed -i -e "s@^export BCON_ADAPTER_I2C_DEVICES=.*@export BCON_ADAPTER_I2C_DEVICES=\"/dev/i2c-0:60\"@g" "$(TARGETDIR)/etc/X11/Xsession.d/90environment"; \
		else \
			echo "" >> "$(TARGETDIR)/etc/X11/Xsession.d/90environment"; \
			echo "# BCON default settings" >> "$(TARGETDIR)/etc/X11/Xsession.d/90environment"; \
			echo "export BCON_ADAPTER_I2C_DEVICES=\"/dev/i2c-0:60\"" >> "$(TARGETDIR)/etc/X11/Xsession.d/90environment"; \
		fi; \
		\
		if grep -q -e "^export BCON_ADAPTER_LIB=.*" "$(TARGETDIR)/etc/X11/Xsession.d/90environment"; then \
			sed -i -e "s@^export BCON_ADAPTER_LIB=.*@export BCON_ADAPTER_LIB=\"/opt/bcon/lib/$(LIB_MMAP).so\"@g" "$(TARGETDIR)/etc/X11/Xsession.d/90environment"; \
		else \
			echo "export BCON_ADAPTER_LIB=\"/opt/bcon/lib/$(LIB_MMAP).so\"" >> "$(TARGETDIR)/etc/X11/Xsession.d/90environment"; \
		fi; \
	fi

clean:
	rm -rf *.o *.so *.so.* *.a

help:
	@echo ""
	@echo "Quick reference for various supported build targets for $(INSTANCE)."
	@echo "----------------------------------------------------"
	@echo "  clean                  clean out build objects"
	@echo "  all                    build $(INSTANCE)"
	@echo "  build                  build subsystem"
	@echo "  install                install built objects to rootfs host copy"
