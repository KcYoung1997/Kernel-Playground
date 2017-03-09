#include <kernel/tty.h>
#include <kernel/ps2.h>
#include <kernel/portb.h>
#include <kernel/cmos.h>
#include <kernel/memset.h>

#include <stdint.h>

//http://wiki.osdev.org/GDT#Structure
struct gdt_t {
	unsigned short limit_low;
	unsigned short base_low;
	unsigned char base_mid;
	unsigned char access_byte;
	unsigned char limit_high_flags;
	unsigned char base_high;
} __attribute__ ((packed));

#define entries 5
struct gdt_t gdt_entry[entries];

extern void lgdt_core(unsigned long gdtp);

void set_gdt(int num, unsigned int base, unsigned int limit, unsigned char access, unsigned char flags){
	gdt_entry[num].limit_low = limit;
	gdt_entry[num].limit_high_flags = (flags<<4) | ((limit>>16)&0x00FF);
	gdt_entry[num].access_byte = access;	
	gdt_entry[num].base_low = base;
	gdt_entry[num].base_mid = base>>16;
	gdt_entry[num].base_high = base>>24;
}


void gdt_init(void) {
	tty_writestring("[GDT] init start\n");

	struct gdt_p {
		unsigned short limit;
		unsigned long base;
	} __attribute__ ((packed));
	struct gdt_p gdtp;
	unsigned short size = sizeof(gdt_entry);
	gdtp.limit = (size * entries) - 1;
	gdtp.base = (unsigned int)&gdt_entry;
	
	memset(&gdt_entry, '\0', 256);

	// C code does not understand segmentation
	// so we will restrict memory access using paging instad
	// http://wiki.osdev.org/Segmentation#Notes_Regarding_C
	set_gdt(0, 0, 0, 0, 0); // Null segment.
	set_gdt(1, 0, 0xFFFFFFFF, 0x9A, 0xCF); // Code segment start at 0
	set_gdt(2, 0, 0xFFFFFFFF, 0x92, 0xCF); // Data segment
	set_gdt(3, 0, 0xFFFFFFFF, 0xFA, 0xCF); // Usermode code segment
	set_gdt(4, 0, 0xFFFFFFFF, 0xF2, 0xCF); // Usermode data segment

	tty_writestring("[GDT] calling lgdt\n");
	lgdt_core((unsigned long)&gdtp);

	tty_writestring("[GDT] init finished\n");
}
