#include <kernel/tty.h>
#include <kernel/ps2.h>
#include <kernel/portb.h>
#include <kernel/cmos.h>

#include <stdint.h>

extern int irq_test(void);
void _test(){
	tty_cursorcolor(0xE0);
	tty_writestring("IRQ test\n");
	tty_cursorcolor(0x0F);
}

extern int irq_kbd(void);
void _irq_kbd(void) {
	int i;
	for(i=0;i<10000;i++){
		if(!(inportb(0x64)&1)) continue;
		while(inportb(0x64)&1){
			tty_writef("%#x ", inportb(0x60));

		}
		break;
	}
	if(i==1000) tty_writestring("Spurios keyboard IRQ");
}


const char * days[7] = {  "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
const char * months[12] = {  "January","February","March","April","May","June","July","August","September","October","November","December" };
void printTime(){
	struct time current = get_rtc();
	tty_cursorposition(36,1);

	int d = current.day;
	int y = (current.century*100)+current.year;
	int m = current.month;	
	int weekday  = (d += d< 3 ? y-- : y - 2, 23*m/9 + d + 4 + y/4- y/100 + y/400)%7;
	tty_writef("%02d:%02d:%02d", current.hour, current.minute, current.second);
	tty_cursorposition(28,2);
	tty_writef("%s the %t of %s", days[weekday], current.day, months[current.month-1]);
}
extern int irq_cmos(void);
void _irq_cmos(void) {
	outportb(0x70, 0x0C);	// select register C
	inportb(0x71);		// just throw away contents
	printTime();
	asm volatile("cli");
	uint8_t rate = 0x0F;			// rate must be above 2 and not over 15
	outportb(0x70, 0x8A);		// set index to register A, disable NMI
	char prev=inportb(0x71);	// get initial value of register A
	outportb(0x70, 0x8A);		// reset index to A
	outportb(0x71, (prev & 0xF0) | rate); //write only our rate to A. Note, rate is the bottom 4 bits.
	asm volatile("sti");
}

extern int irq_default(void);
void _irq_default(uint8_t num) {
	tty_writestring("IRQ: ");
	tty_printhex(num);
	tty_writechar('\n');
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
void irq_init(void) {
	tty_writestring("IRQ initialization\n");

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
	make_idt_entry(40, (unsigned)irq_cmos, 0x08, 0x8E);
	make_idt_entry(0x2f, (unsigned)irq_test, 0x08, 0x8E);
	
	lidt_core((unsigned long)&idtp);

	asm volatile("int $0x2f");
	
	tty_writestring("IRQ init_done\n");

	tty_writestring("PIC initialization\n");
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
	tty_writestring("PIC init_done\n");
}

