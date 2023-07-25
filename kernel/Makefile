C_FLAGS?=-O2 -g
CPP_FLAGS?=
LD_FLAGS?=
LIBS?=

DEST_DIR?=
PREFIX?=/usr/local
EXEC_PREFIX?=$(PREFIX)
BOOT_DIR?=$(EXEC_PREFIX)/boot
INCLUDE_DIR?=$(PREFIX)/include
ARCH_DIR=arch/$(HOST_ARCH)

C_FLAGS:=$(C_FLAGS) -ffreestanding -Wall -Wextra
CPP_FLAGS:=$(CPP_FLAGS) -D__is_kernel -Iinclude -I$(DEST_DIR)$(INCLUDE_DIR)
LD_FLAGS:=$(LD_FLAGS)
LIBS:=$(LIBS) -nostdlib -lgcc -L$(DEST_DIR)$(PREFIX)/lib -lk

include $(ARCH_DIR)/make.config

C_FLAGS:=$(C_FLAGS) $(KENREL_ARCH_C_FLAGS)
CPP_FLAGS:=$(CPP_FLAGS) $(KERNEL_ARCH_CPP_FLAGS)
LD_FLAGS:=$(LD_FLAGS) $(KERNEL_ARCH_LD_FLAGS)
LIBS:=$(LIBS) $(KERNEL_ARCH_LIBS)

KERNEL_OBJS=$(KERNEL_ARCH_OBJS) kernel/kernel.o kernel/vmm.o kernel/liballoc.o \
	kernel/device.o kernel/driver/vfs.o kernel/driver/initrd.o
OBJS=$(ARCH_DIR)/crti.o $(ARCH_DIR)/crtbegin.o $(KERNEL_OBJS) \
	$(ARCH_DIR)/crtend.o $(ARCH_DIR)/crtend.o $(ARCH_DIR)/crtn.o
LINK_LIST=$(LD_FLAGS) $(ARCH_DIR)/crti.o $(ARCH_DIR)/crtbegin.o $(KERNEL_OBJS) \
	$(LIBS) $(ARCH_DIR)/crtend.o $(ARCH_DIR)/crtn.o

all: myos.kernel

myos.kernel: $(OBJS) $(ARCH_DIR)/linker.ld
	echo "OBJS: $(OBJS)"
	$(CC) -T $(ARCH_DIR)/linker.ld -o $@ $(C_FLAGS) $(LINK_LIST)

$(ARCH_DIR)/crtbegin.o $(ARCH_DIR)/crtend.o:
	OBJ=`$(CC) $(C_FLAGS) $(LD_FLAGS) -print-file-name=$(@F)` && cp "$$OBJ" $@

.c.o:
	$(CC) -MD -c $< -o $@ -std=gnu11 $(C_FLAGS) $(CPP_FLAGS)

.S.o:
	$(CC) -MD -c $< -o $@ $(C_FLAGS) $(CPP_FLAGS)

clean:
	rm -f myos.kernel
	rm -f $(OBJS) *.o */*.o */*/*.o
	rm -f $(OBJS:.o=.d) *.d */*.d */*/*.d

install: install-headers install-kernel

install-headers:
	mkdir -p $(DEST_DIR)$(INCLUDE_DIR)
	cp -R include/* $(DEST_DIR)$(INCLUDE_DIR)

install-kernel: myos.kernel
	mkdir -p $(DEST_DIR)$(BOOT_DIR)
	cp myos.kernel $(DEST_DIR)$(BOOT_DIR)

-include $(OBJS:.o=.d)
