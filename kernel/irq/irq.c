#include "kernel/tty.h"
#include "kernel/ps2.h"
#include "kernel/portb.h"

// From now on, memset is needed. 

void memset(void* bufptr, unsigned char value, int size) {
	unsigned char* buf = (unsigned char *)bufptr;
	for (int i = 0; i < size; i++) {
		buf[i] = value;
	}
}	

#define INT_START asm volatile("pusha");
#define INT_END asm volatile("popa"); \
	asm volatile("iret");

extern int irq_test();
void _test(){
	terminal_writestring("TEST");
}

extern int irq_kbd();
void _irq_kbd(){
	int i;
	for(i = 0; i < 10000; i++){
		if(!(inportb(0x64&1))) continue;
		terminal_writestring("Got scancode");
		terminal_putchar(inportb(0x60));
		terminal_putchar('\n');
	}
	if(i==10000) terminal_writestring("Spurios keyboard IRQ");
}




void irq_default()
{
	INT_START;
	terminal_writestring("TEST");
	INT_END;
}

struct idt_t {
	unsigned short base_low;
	unsigned short selector;
	unsigned char unused;
	unsigned char flags;
	unsigned short base_high;
} __attribute__ ((packed));

struct idt_t idt_entry[256];

extern void lidt_core(unsigned long idtp);

void make_idt_entry(int num, unsigned long base, unsigned short selector, unsigned char flags) {
	idt_entry[num].base_low = base;
	idt_entry[num].selector = selector;
	idt_entry[num].unused = 0;
	idt_entry[num].flags = flags;
	idt_entry[num].base_high = (base >> 16);
}
unsigned char tick = 0;
void irq_inititalize(void) {
	struct idt_p {
		unsigned short limit;
		unsigned long base;
	} __attribute__ ((packed));
	struct idt_p idtp;
	unsigned short size = sizeof(idt_entry);
	idtp.limit = (size * 256) - 1;
	idtp.base = (unsigned int)&idt_entry;
	
	memset(&idt_entry, '\0', 256);
	
	for (int i = 0; i < 256; i++) {
		make_idt_entry(i, (unsigned)irq_default, 0x08, 0x8E);
	}
	make_idt_entry(33, (unsigned)irq_kbd, 0x08, 0x8E);
	make_idt_entry(0x2f, (unsigned)irq_test, 0x08, 0x8E);
	
	
	lidt_core((unsigned long)&idtp);
	
	asm volatile("int $0x2f");
//	asm volatile("int $33");

	terminal_writestring("IRQ initialized\n");
}

