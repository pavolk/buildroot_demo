
BINARIES=axidma_vidin_test axidma_vidout_test
LDLIBS += -laxidma
CFLAGS += -g

.PHONY: all clean install

all: clean $(BINARIES)

install: $(BINARIES)
	install -D -m 555 $(BINARIES)  $(PREFIX)/usr/bin
	mkdir -p $(PREFIX)/usr/share/axidma_tests
	install -D -m 555 $(@D)/*.sh $(PREFIX)/usr/share/axidma_tests

clean:
	$(RM) *.o $(BINARIES)