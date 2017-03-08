#include "kernel/tty.h"
#include "kernel/ps2.h"
#include "kernel/portb.h"


extern int default_isr(void);
extern int default_irq(void);

extern int irq_keyboard(void);

// From now on, memset is needed. 

void memset(void* bufptr, unsigned char value, int size) {
	unsigned char* buf = (unsigned char *)bufptr;
	for (int i = 0; i < size; i++) {
		buf[i] = value;
	}
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

void set_idt_entry(int num, unsigned long base, unsigned short selector, unsigned char flags) {
	idt_entry[num].base_low = base;
	idt_entry[num].base_high = (base >> 16);
	idt_entry[num].selector = selector;
	idt_entry[num].unused = 0;
	idt_entry[num].flags = flags;
}
void keyboard(void) {
	tty_writechar('@');
	if(!ps2_init_done) tty_writechar(inportb(0x60));
	tty_writechar(keyboard_read());
}
#define PIC_MASTER_CMD 0x20
#define PIC_MASTER_DATA 0x21
#define PIC_SLAVE_CMD 0xA0
#define PIC_SLAVE_DATA 0xA1

#define PIC_CMD_EOI 0x20
void irq_init(void) {
	tty_writestring("Beginning initialization\n");
	/* set up cascading mode */
	outportb(PIC_MASTER_CMD, 0x10 + 0x01);
	outportb(PIC_SLAVE_CMD,  0x10 + 0x01);
	/* Setup master's vector offset */
	outportb(PIC_MASTER_DATA, 0x20);
	/* Tell the slave its vector offset */
	outportb(PIC_SLAVE_DATA, 0x28);
	/* Tell the master that he has a slave */
	outportb(PIC_MASTER_DATA, 4);
	outportb(PIC_SLAVE_DATA, 2);
	/* Enabled 8086 mode */
	outportb(PIC_MASTER_DATA, 0x01);
	outportb(PIC_SLAVE_DATA, 0x01);

	tty_writestring("Resetting masks\n");
	outportb(PIC_MASTER_DATA, 0);
	outportb(PIC_SLAVE_DATA, 0);
	tty_writestring("Init done.\n");

	struct idt_p {
		unsigned short limit;
		unsigned long base;
	} __attribute__ ((packed));
	struct idt_p idtp;
	unsigned short size = sizeof(idt_entry);
	idtp.limit = (size * 256) - 1;
	idtp.base = (unsigned int)&idt_entry;
	
	memset(&idt_entry, '\0', 256);
	
	for (int i = 0; i < 32; i++) {
		set_idt_entry(i, (unsigned)irq_keyboard, 0x08, 0x8E);
	}
	set_idt_entry(32, (unsigned)irq_keyboard, 0x08, 0x8E);
	set_idt_entry(33, (unsigned)irq_keyboard, 0x08, 0x8E);
	for (int i = 34; i < 48; i++) {
		set_idt_entry(i, (unsigned)irq_keyboard, 0x08, 0x8E);
	}
	
	lidt_core((unsigned long)&idtp);
	
	tty_writestring("IRQ init_done\n");
}

