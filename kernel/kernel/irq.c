#include <kernel/tty.h>
#include <kernel/ps2.h>
#include <kernel/portb.h>
#include <kernel/cmos.h>
#include <kernel/memset.h>

#include <stdint.h>

extern int irq_kbd(void);
extern int irq1(void);
extern int irq2(void);
extern int irq3(void);
extern int irq4(void);
extern int irq5(void);
extern int irq6(void);
extern int irq7(void);
extern int irq8(void);
extern int irq9(void);
extern int irq10(void);
extern int irq11(void);
extern int irq12(void);
extern int irq13(void);
extern int irq14(void);
extern int irq15(void);
int (*irqnum[15])(void) = { irq1,irq2,irq3,irq4,irq5,irq6,irq7,irq8,irq9,irq10,irq11,irq12,irq13,irq14,irq15 };
void (*irqfunc[15])(void) = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };

int set_irq_func(int irq, void (*func)(void)) {
	if(irq < 1 || irq > 15) {
		//TODO: ERRNO
		return -1;
	}
	irqfunc[irq-1] = func;
	return 0;
}

void clear_irq_func(int irq) {
	irqfunc[irq-1] = 0;
}

extern int irq_default(void);
void _irq_default(int num) {
//	tty_writef("IRQ:%d\n",num);
	if(num==8) 
	{
		//http://www.walshcomptech.com/ohlandl/config/cmos_bank_0.html#Hex_00C
		outportb(0x70, 0x0C);	// select register C
		if(inportb(0x71)&(1<<6)) {
			//If periodic interrupt (the one we care about
			if(irqfunc[7]) irqfunc[7]();
		}
	}
	if(irqfunc[num-1])irqfunc[num-1]();

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
	idt_entry[num].selector = selector;
	idt_entry[num].unused = 0;
	idt_entry[num].flags = flags;
	idt_entry[num].base_high = (base >> 16);
}


void irq_init(void) {
	MODULE_INFO tty_writestring("init start\n");

	struct idt_p {
		unsigned short limit;
		unsigned long base;
	} __attribute__ ((packed));
	struct idt_p idtp;
	unsigned short size = sizeof(idt_entry);
	idtp.limit = (size * 256) - 1;
	idtp.base = (unsigned int)&idt_entry;
	
	memset(&idt_entry, '\0', 256);
	
	// 0-1Fh are reserved for CPU http://wiki.osdev.org/PIC#Protected_Mode
	for (int i = 0; i < 33; i++) {
		set_idt_entry(i, (unsigned)irq_default, 0x08, 0x8E);
	}
	// 0x20-0x2E are mapped to IRQs by PIC init
	for(int i = 0; i < 15; i++) {
		set_idt_entry(33+i, (unsigned)irqnum[i], 0x08, 0x8E);
	}
	for (int i = 47; i < 256; i++) {
		set_idt_entry(i, (unsigned)irq_default, 0x08, 0x8E);
	}
	
	lidt_core((unsigned long)&idtp);

	MODULE_INFO tty_writestring("init complete\n");
}

