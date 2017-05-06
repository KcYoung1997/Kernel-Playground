#include <kernel/tty.h>
#include <kernel/portb.h>

#define PIC_MASTER_CMD 0x20
#define PIC_MASTER_DATA 0x21
#define PIC_SLAVE_CMD 0xA0
#define PIC_SLAVE_DATA 0xA1

#define PIC_MASTER_OFFSET 0x20
#define PIC_SLAVE_OFFSET 0x28

#define PIC_CMD_EOI 0x20

void pic_init(void) {
	MODULE_INFO tty_writestring("init start\n");
	// Send both PICs the init (cascade) command http://wiki.osdev.org/PIC#Initialisation
	outportb(PIC_MASTER_CMD, 0x10 + 0x01);
	outportb(PIC_SLAVE_CMD,  0x10 + 0x01); 
	// Set both PICs vector (IRQ) offsets
	outportb(PIC_MASTER_DATA, PIC_MASTER_OFFSET);
	outportb(PIC_SLAVE_DATA, PIC_SLAVE_OFFSET);
	// Tel the master theres a slave at IRQ 2 (0000 0100)
	outportb(PIC_MASTER_DATA, 4);
	// Tell slave its cascade identity
	outportb(PIC_SLAVE_DATA, 2);
	/* Enabled 8086 mode */
	outportb(PIC_MASTER_DATA, 0x01);
	outportb(PIC_SLAVE_DATA, 0x01);

	// Reset masks, enables all PIT lines
	outportb(PIC_MASTER_DATA, 0);
	outportb(PIC_SLAVE_DATA, 0);
	MODULE_INFO tty_writestring("init finished\n");

}
