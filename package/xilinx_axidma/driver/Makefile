
AXIDMA_FILES = axi_dma.c axidma_chrdev.c axidma_dma.c axidma.h axidma_of.c
AXIDMA_MODULE_NAME = axidma

obj-m += $(AXIDMA_MODULE_NAME).o
$(AXIDMA_MODULE_NAME)-objs = $(patsubst %.c,%.o,$(filter %.c,$(AXIDMA_FILES)))

ccflags-y = $(EXTRA_FLAGS) -Werror -ggdb
