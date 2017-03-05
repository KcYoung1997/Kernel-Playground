# Kernel-Playground
A project x86 kernel built for fun and testing purposes. Supports being run via [QEMU](http://www.qemu-project.org) or smiliar, or bootloaded from any bootloader that supports multiboot 2 ([GRUB2](www.gnu.org/software/grub/manual/grub.html#Overview) or similar).
## Requirements
* A [cross-compiler](http://wiki.osdev.org/GCC_Cross-Compiler) that supports building C to i686-elf executabled (GCC)
    * OSDev wiki has a [list of prebuilt toolchains](http://wiki.osdev.org/GCC_Cross-Compiler#Prebuilt_Toolchains) for multiple operating systems.
* An emulator or device that supports VGA (Most modern graphics cards still do)
