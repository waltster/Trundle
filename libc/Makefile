HOST?=
HOST_ARCH?=i386

C_FLAGS?=-O2 -g
CPP_FLAGS?=
LD_FLAGS?=
LIBS?=

DEST_DIR?=
PREFIX?=/usr/local
EXEC_PREFIX?=$(PREFIX)
INCLUDE_DIR?=$(PREFIX)/include
LIB_DIR?=$(EXEC_PREFIX)/lib
ARCH_DIR=arch/$(HOST_ARCH)

C_FLAGS:=$(C_FLAGS) -ffreestanding -Wall -Wextra
CPP_FLAGS:=$(CPP_FLAGS) -Iinclude -I$(DEST_DIR)$(INCLUDE_DIR)
LIBK_C_FLAGS:=$(C_FLAGS)
LIBK_CPP_FLAGS:=$(CPP_FLAGS) -I$(DEST_DIR)$(INCLUDE_DIR)

include $(ARCH_DIR)/make.config

C_FLAGS:=$(C_FLAGS) $(ARCH_C_FLAGS)
CPP_FLAGS:=$(CPP_FLAGS) $(ARCH_CPP_FLAGS)
LIBK_C_FLAGS:=$(LIBK_C_FLAGS) $(KERNEL_ARCH_C_FLAGS)
LIBK_CPP_FLAGS:=$(LIBK_CPP_FLAGS) $(KERNEL_ARCH_CPP_FLAGS)

FREE_OBJS=$(ARCH_FREE_OBJS) \
stdio/printf.o \
stdio/putchar.o \
stdio/puts.o \
stdlib/abort.o \
string/memcmp.o \
string/memcpy.o \
string/memmove.o \
string/memset.o \
string/strlen.o \
string/strcmp.o \
string/strcpy.o

HOSTED_OBJS=$(ARCH_HOSTED_OBJS)
OBJS=$(FREE_OBJS) $(HOSTED_OBJS)
LIBK_OBJS=$(FREE_OBJS:.o=.libk.o)

BINARIES=libk.a

.PHONY: all clean install install-headers install-libs
.SUFFIXES: .o .libk.o .c .s

all: $(BINARIES)

libc.a: $(OBJS)
	$(AR) rcs $@ $(OBJS)

libk.a: $(LIBK_OBJS)
	$(AR) rcs $@ $(LIBK_OBJS)

.c.o:
	$(CC) -MD -c $< -o $@ -std=gnu11 -D__is_libc $(C_FLAGS) $(CPP_FLAGS)

.S.o:
	$(CC) -MD -c $< -o $@ $(C_FLAGS) -D__is_libc $(CPP_FLAGS)

.c.libk.o:
	$(CC) -MD -c $< -o $@ -std=gnu11 -D__is_libk $(LIBK_C_FLAGS) $(LIBK_CPP_FLAGS)

.S.libk.o:
	$(CC) -MD -c $< -o $@ -D__is_libk $(LIBK_C_FLAGS) $(LIBK_CPP_FLAGS)

clean:
	rm -f $(BINARIES) *.a
	rm -f $(OBJS) $(LIBK_OBJS) *.o */*.o */*/*.o
	rm -f $(OBJS:.o=.d) $(LIBK_OBJS:.o=.d) *.d */*.d */*/*.d

install: install-headers install-libs

install-headers:
	mkdir -p $(DEST_DIR)$(INCLUDE_DIR)
	cp -R include/* $(DEST_DIR)$(INCLUDE_DIR)

install-libs: $(BINARIES)
	mkdir -p $(DEST_DIR)$(LIB_DIR)
	cp $(BINARIES) $(DEST_DIR)$(LIB_DIR)

-include $(OBJS:.o=.d)
-include $(LIBK_OBJS:.o=.d)
