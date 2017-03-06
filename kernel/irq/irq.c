#include <kernel/tty.h>
#include <kernel/ps2.h>
#include <kernel/portb.h>

#include <stdint.h>

extern int irq_default(void);
extern int irq_test(void);
void _test(){
	terminal_setcolor(0xE0);
	terminal_writestring("IRQ test\n");
	terminal_setcolor(0x0F);
}

extern int irq_kbd(void);
void _irq_kbd(void) {
	int i;
	for(i=0;i<10000;i++){
		if(!(inportb(0x64)&1)) continue;
		while(inportb(0x64)&1){
			terminal_printhex(inportb(0x60));
			terminal_putchar(' ');
		}
		break;
	}
	if(i==1000) terminal_writestring("Spurios keyboard IRQ");
}

extern int irq_cmos(void);
void _irq_cmos(void) {
	terminal_writestring("CMOS tick");
}
extern int irq_default(void)
void _irq_default(uint8_t num) {
	terminal_writestring("IRQ: ");
	terminal_printhex(num);
	terminal_putchar('\n');
}
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

void make_idt_entry(int num, unsigned long base, unsigned short selector, unsigned char flags) {
	idt_entry[num].base_low = base;
	idt_entry[num].base_high = (base >> 16);
	idt_entry[num].selector = selector;
	idt_entry[num].unused = 0;
	idt_entry[num].flags = flags;
}

#define PIC_MASTER_CMD 0x20
#define PIC_MASTER_DATA 0x21
#define PIC_SLAVE_CMD 0xA0
#define PIC_SLAVE_DATA 0xA1

#define PIC_CMD_EOI 0x20
void irq_inititalize(void) {
	terminal_writestring("IRQ initialization\n");

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
	make_idt_entry(43, (unsigned)irq_cmos, 0x08, 0x8E);
	make_idt_entry(0x2f, (unsigned)irq_test, 0x08, 0x8E);
	
	lidt_core((unsigned long)&idtp);

	asm volatile("int $0x2f");
	
	terminal_writestring("IRQ initialized\n");

	terminal_writestring("PIC initialization\n");
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

	terminal_writestring("Resetting masks\n");
	outportb(PIC_MASTER_DATA, 0);
	outportb(PIC_SLAVE_DATA, 0);
	terminal_writestring("PIC initialized\n");

	asm volatile("sti");
}
