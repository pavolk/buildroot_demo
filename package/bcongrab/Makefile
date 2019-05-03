ifdef PETALINUX
    include apps.common.mk
    INSTALL_CMD := $(TARGETINST) -d
else
    INSTALL_CMD := install -D
endif

APP = bcongrab

# Add any other object files to this list below
APP_OBJS    := bcongrab.o
APP_HEADER  := HardwareTriggerConfiguration.h BconTriggerGenerator.h
APP_SOURCES := $(APP_OBJS:.o=.cpp) $(APP_HEADER) Makefile

# Installation directory for pylon
PYLON_ROOT := /opt/pylon5

CXXFLAGS := $(CXXFLAGS) -Werror -Wall -Wno-unknown-pragmas
CPPFLAGS := $(CPPFLAGS) -I. $(shell $(STAGEDIR)$(PYLON_ROOT)/bin/pylon-config --cflags)
LDLIBS   := $(shell $(STAGEDIR)$(PYLON_ROOT)/bin/pylon-config --libs) -lbconctl

# Find pylon libs at run time.
LDFLAGS  := $(LDFLAGS) -Wl,-rpath=$(PYLON_ROOT)/lib

# Find pylon second level library dependencies at link time
LDFLAGS  := $(LDFLAGS) -Wl,-rpath-link=$(STAGEDIR)$(PYLON_ROOT)/lib

# libstdc++.so needs libm.so.6 as second level dependency when linking our executable.
# In our sysroot usr/lib/arm-linux-gnueabihf/libm.so is linked absolutely to /lib/arm-linux-gnueabihf/libm.so.6,
# which is not available at link time, only at run time. Two possible workarounds:
#LDFLAGS  := $(LDFLAGS) -Wl,--allow-shlib-undefined
LDFLAGS  := $(LDFLAGS) -Wl,-rpath-link=$(STAGEDIR)/lib/$(TARGETARCH)

all: build

build: $(APP)

$(APP): $(APP_OBJS)
	$(CXX) $(LDFLAGS) -o $@ $(APP_OBJS) $(LDLIBS)

clean:
	-rm -f $(APP) *.elf *.gdb *.o

.PHONY: install image

install: $(APP)
	$(INSTALL_CMD) $(APP) /usr/bin/$(APP)
ifdef PETALINUX
	for file in $(APP_SOURCES); do \
		$(INSTALL_CMD) $$file /opt/bcon/sources/$(APP)/$$file; \
	done
endif

%.o: %.cpp $(APP_HEADER)
	$(CXX) -c $(CXXFLAGS) $(CPPFLAGS) -o $@ $<

help:
	@echo ""
	@echo "Quick reference for various supported build targets for $(INSTANCE)."
	@echo "----------------------------------------------------"
	@echo "  clean                  clean out build objects"
	@echo "  all                    build $(INSTANCE)"
	@echo "  build                  build subsystem"
	@echo "  install                install built objects to rootfs host copy"
