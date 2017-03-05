# Items taken in from environment variables if exist, else set to their default value
HOST?=
ARCH?=
# Default CFLAGS, optimisation level 2 and leave symbols in the compiled objects:
CFLAGS?=-O2 -g
CPPFLAGS?=
OSNAME?=kOS
OUTDIR?=bin
SYSROOT?=../sysroot
BOOTDIR?=boot
INCLUDEDIR?=usr/include

PREFIX?=/usr/local
INCLUDEDIR?=$(PREFIX)/include
LIBDIR?=$(PREFIX)/lib

CFLAGS:=$(CFLAGS) -ffreestanding -Wall -Wextra
CPPFLAGS:=$(CPPFLAGS) -D__is_libc -Iinclude
LIBK_CFLAGS:=$(CFLAGS)
LIBK_CPPFLAGS:=$(CPPFLAGS) -D__is_libk

ARCHDIR = arch/$(ARCH)

# Get extra flags from architecture based config file
#include $(ARCHDIR)/make.config
#CFLAGS:=$(CFLAGS) $(KERNEL_ARCH_CFLAGS)
#CPPFLAGS:=$(CPPFLAGS) $(KERNEL_ARCH_CPPFLAGS)
#LDFLAGS:=$(LDFLAGS) $(KERNEL_ARCH_LDFLAGS)
#LIBS:=$(LIBS) $(KERNEL_ARCH_LIBS)

FREEOBJS=\
stdio/printf.o \
stdio/putchar.o \
stdio/puts.o \
stdlib/abort.o \
string/memcmp.o \
string/memcpy.o \
string/memmove.o \
string/memset.o \
string/strlen.o \

HOSTEDOBJS=\
#$(ARCH_HOSTEDOBJS) \

OBJS=\
$(FREEOBJS) \
$(HOSTEDOBJS) \

LIBK_OBJS=$(FREEOBJS:.o=.libk.o)

#BINARIES=libc.a libk.a
BINARIES=libk.a

.SUFFIXES: .o .libk.o .c .S

#############################

.PHONY: all
all: $(BINARIES)

.PHONY: clean
clean:
	find . -name "*.a" -type f -delete
	find . -name "*.o" -type f -delete
	find . -name "*.d" -type f -delete

.PHONY: install
install: install-headers install-libs

.PHONY: install-headers
install-headers:
	mkdir -p $(DESTDIR)/$(INCLUDEDIR)
	cp -R --preserve=timestamps include/. $(DESTDIR)/$(INCLUDEDIR)/.

.PHONY: install-libs
install-libs: $(BINARIES)
	mkdir -p $(DESTDIR)/$(LIBDIR)
	cp $(BINARIES) $(DESTDIR)/$(LIBDIR)

#############################

libc.a: $(OBJS)
	$(AR) rcs $@ $(OBJS)

libk.a: $(LIBK_OBJS)
	echo $(LIBK_OBJS)
	echo $(OBJS)
	$(AR) rcs $@ $(LIBK_OBJS)

.c.o:
	$(CC) -MD -c $< -o $@ -std=gnu11 $(CFLAGS) $(CPPFLAGS)

.c.S:
	$(CC) -MD -c $< -o $@ $(CFLAGS) $(CPPFLAGS)

.c.libk.o:
	$(CC) -MD -c $< -o $@ -std=gnu11 $(LIBK_CFLAGS) $(LIBK_CPPFLAGS)

.S.libk.o:
	$(CC) -MD -c $< -o $@ $(LIBK_CFLAGS) $(LIBK_CPPFLAGS)



-include $(OBJS:.o=.d)
-include $(LIBK_OBJS:.o=.d)

