# Items taken in from environment variables if exist, else set to their default value
HOST?=
ARCH?=
# Default CFLAGS, optimisation level 2 and leave symbols in the compiled objects:
CFLAGS?=-O2 -g
CPPFLAGS?=
OSNAME?=kOS
OUTDIR?=bin
SYSROOT?=../sysroot

PREFIX?=/usr/local
INCLUDEDIR?=$(PREFIX)/include
LIBDIR?=$(PREFIX)/lib

# Add mandatory options to CFLAGS, adds extra warnings:
CFLAGS:=$(CFLAGS) -Wall -Wextra -ffreestanding
CPPFLAGS:=$(CPPFLAGS) -D__is_kernel -Iinclude

ARCHDIR = arch/$(ARCH)

GRUBDIR = $(OUTDIR)/grub
OUTBIN = $(OUTDIR)/$(OSNAME).bin
OUTISO?= $(OUTDIR)/$(OSNAME).iso

include $(ARCHDIR)/make.config

CFLAGS:=$(CFLAGS) $(KERNEL_ARCH_CFLAGS)
CPPFLAGS:=$(CPPFLAGS) $(KERNEL_ARCH_CPPFLAGS)
LDFLAGS:=$(LDFLAGS) $(KERNEL_ARCH_LDFLAGS)
LIBS:=$(LIBS) $(KERNEL_ARCH_LIBS)

OBJS:= $(KERNEL_ARCH_OBJS) \
kernel/kernel.o \
 
CRTI_OBJ=$(ARCHDIR)/crti.o
CRTN_OBJ=$(ARCHDIR)/crtn.o
 
OBJ_LINK_LIST:=$(CRTI_OBJ) \
$(shell $(CC) $(CFLAGS) -print-file-name=crtbegin.o) \
$(CRTBEGIN_OBJ) \
$(OBJS) \
$(shell $(CC) $(CFLAGS) -print-file-name=crtend.o) \
$(CRTN_OBJ)

INTERNAL_OBJS:=$(CRTI_OBJ) \
$(OBJS) \
$(CRTN_OBJ)

.SUFFIXES: .cpp .c .o .s

define grubcfg
menuentry "$(OSNAME)" {
	multiboot /boot/$(OSNAME).bin
}
endef
export grubcfg

#########################################
.PHONY: all
all: $(OUTBIN)

.PHONY: grub
grub: $(OUTISO)

.PHONY: clean
clean:
	find . -name "*.o" -type f -delete
	find . -name "*.d" -type f -delete
	rm -rf $(OUTDIR)/*

.PHONY: run
run: $(OUTBIN)
	qemu-system-i386 -kernel $(OUTBIN)

.PHONY: install
install: install-headers install-kernel

.PHONY: install-headers
install-headers:
	mkdir -p $(SYSROOT)/$(INCLUDEDIR)
	cp -R --preserve=timestamps include/. $(SYSROOT)/$(INCLUDEDIR)/kernel

# Move kernel file to $(SYSROOT)
.PHONY: install-kernel 
install-kernel: $(OUTBIN)
	mkdir -p $(SYSROOT)/$(BOOTDIR)
	cp $(OUTBIN) $(SYSROOT)/$(BOOTDIR)

##########################################

$(OUTBIN): $(INTERNAL_OBJS) $(ARCHDIR)/linker.ld
	mkdir -p $(OUTDIR)
	$(CC) $(CFLAGS) -T $(ARCHDIR)/linker.ld -o $@ -ffreestanding -nostdlib $(OBJ_LINK_LIST)
	grub-file --is-x86-multiboot $@

.s.o:
	$(CC) -MD -c $< -o $@ $(CFLAGS) $(CPPFLAGS)

.c.o:
	$(CC) -MD -c $< -o $@ $(CFLAGS) $(CPPFLAGS)

.cpp.o:
	$(CC) -MD -c $< -o $@ $(CFLAGS) $(CPPFLAGS)


$(OUTISO): $(OUTBIN)
	mkdir -p $(OUTDIR)
	mkdir -p $(GRUBDIR)/boot/grub
	cp $(OUTBIN) $(GRUBDIR)/boot/$(OSNAME).bin
	@echo "$$grubcfg" > $(GRUBDIR)/boot/grub/grub.conf
	echo $(OUTISO)
	grub-mkrescue -o $(OUTISO) $(GRUBDIR)

-include $(OBJS:.o=.d)
