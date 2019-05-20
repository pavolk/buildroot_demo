ifdef PETALINUX
include apps.common.mk
INSTALL_CMD := $(TARGETINST) -d
SYMLINK_CMD := $(TARGETINST) -s
else
INSTALL_CMD := install -D
SYMLINK_CMD := ln -sf
endif

# Name of application
APP = bconctl

# Add any other object files to this list below
APP_OBJS    := bconctl.o bconctlCommandLineParser.o
APP_HEADER  := bconctlCommandLineParser.h bconctlParameters.h bconctlVersion.h
APP_SOURCES := $(APP_OBJS:.o=.cpp) $(APP_HEADER) Makefile bconctl-power.service

# Additional compiler and linker flags
CXXFLAGS := $(CXXFLAGS) -Werror -Wall -Wno-unknown-pragmas
LDLIBS   := -lbconctl

# libstdc++.so needs libm.so.6 as second level dependency when linking our executable.
# In our sysroot usr/lib/arm-linux-gnueabihf/libm.so is linked absolutely to /lib/arm-linux-gnueabihf/libm.so.6,
# which is not available at link time, only at run time. Two possible workarounds:
#LDFLAGS  := $(LDFLAGS) -Wl,--allow-shlib-undefined
LDFLAGS  := $(LDFLAGS) -Wl,-rpath-link=$(STAGEDIR)/lib/$(TARGETARCH)

.PHONY: install

all: build

build: $(APP)

%.o: %.cpp $(APP_HEADER)
	$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) -o $@ $<
	
$(APP): $(APP_OBJS)
	$(CXX) $(LDFLAGS) -o $@ $(APP_OBJS) $(LDLIBS)

clean:
	rm -f $(APP) *.elf *.gdb *.o

install: $(APP)
	$(INSTALL_CMD) $(APP) /usr/bin/$(APP)
	
ifdef PETALINUX
	for s in $(APP_SOURCES); do \
		$(INSTALL_CMD) $$s /opt/bcon/sources/$(APP)/$$s; \
	done
endif

	$(INSTALL_CMD) bconctl-power.service /etc/systemd/system/bconctl-power.service
	$(SYMLINK_CMD) ../bconctl-power.service /etc/systemd/system/multi-user.target.wants/bconctl-power.service
	
help:
	@echo ""
	@echo "Quick reference for various supported build targets for $(INSTANCE)."
	@echo "----------------------------------------------------"
	@echo "  clean                  clean out build objects"
	@echo "  all                    build $(INSTANCE)"
	@echo "  build                  build subsystem"
	@echo "  install                install built objects to rootfs host copy"
