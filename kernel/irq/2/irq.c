#include "kernel/tty.h"
#include "kernel/ps2.h"
#include "kernel/portb.h"
#include "kernel/cmos.h"
#include <stdint.h>

typedef struct {
	uint16_t offset_0_15;
	uint16_t selector;
	uint8_t zero;
	uint8_t type_attr;
	uint16_t offset_16_31;
} __attribute__((packed)) idt_descriptor;

#define IDT_32BIT_INTERRUPT_GATE	0xE
#define IDT_STORAGE_SEGMENT		0x20
#define IDT_DPL_3			0x60
#define IDT_PRESENT			0x80

#define INT_START asm volatile("pusha");
#define INT_END asm volatile("popa"); \
	asm volatile("iret");

static uint32_t idt_location = 0;
static uint32_t idtr_location = 0;
static uint16_t idt_size = 0x800;

static uint8_t test_success = 0;
static uint32_t test_timeout = 0x1000;

extern void _set_idtr();

void __idt_default_handler();
void __idt_test_handler();

static uint8_t __idt_setup = 0;

void mprint(){}
void panic(){}


void __idt_test_handler()
{
	INT_START;
	terminal_writestring("IDT test Handler\n");
	test_success = 1;
	INT_END;
}/*
#define PIC_MASTER_CMD 0x20
#define PIC_MASTER_DATA 0x21
#define PIC_SLAVE_CMD 0xA0
#define PIC_SLAVE_DATA 0xA1

#define PIC_CMD_EOI 0x20

void send_eoi(uint8_t irq)
{
	if(irq >= 8)
		outportb(PIC_SLAVE_CMD, PIC_CMD_EOI);
	outportb(PIC_MASTER_CMD, PIC_CMD_EOI);
}
void __idt_kbd_handler()
{
	INT_START;
	terminal_writestring("IDT kbd Handler\n");
	uint8_t scancode = 0;
	int i;
	for(i=10000;i<0;i--){
		if((inportb(0x64)&1)==0) continue;
		scancode=inportb(0x60);
		outportb(PIC_MASTER_CMD, PIC_CMD_EOI);
		break;
	}
	if(i>0) {
		terminal_writestring("got scancode=");
		terminal_printhex(scancode);
	} else terminal_writestring("spuorious irq");
	INT_END;
}*/
unsigned char tick = 0;
void __idt_tick_handler()
{
	++tick;
}

void idt_register_interrupt(uint8_t i, uint32_t callback)
{
	if(!__idt_setup) terminal_writestring("Invalid IDT!");
	*(uint16_t*)(idt_location + 8*i + 0) = (uint16_t)(callback & 0x0000ffff);
	*(uint16_t*)(idt_location + 8*i + 2) = (uint16_t)0x8;
	*(uint8_t*) (idt_location + 8*i + 4) = 0x00;
	*(uint8_t*) (idt_location + 8*i + 5) = 0x8e;//0 | IDT_32BIT_INTERRUPT_GATE | IDT_PRESENT;
	*(uint16_t*)(idt_location + 8*i + 6) = (uint16_t)((callback & 0xffff0000) >> 16);
	if(test_success) terminal_writestring("Registered INT\n");
	return;
}

void irq_inititalize()
{
	terminal_writestring("Starting IRQ\n");
	idt_location = 0x3000;
	idtr_location = 0x20F0;
	__idt_setup = 1;
	for(uint8_t i = 0; i < 255; i++)
	{
		idt_register_interrupt(i, (uint32_t)&__idt_default_handler);
	}
	idt_register_interrupt(0x2f, (uint32_t)&__idt_test_handler);
//	idt_register_interrupt(33, (uint32_t)&__idt_kbd_handler);
//	idt_register_interrupt(32, (uint32_t)&__idt_tick_handler);
	terminal_writestring("Registered all interrupts to default handler.\n");
	/* create IDTR now */
	*(uint16_t*)idtr_location = idt_size - 1;
	*(uint32_t*)(idtr_location + 2) = idt_location;

	mprint("IDTR.size = 0x%x IDTR.offset = 0x%x\n", *(uint16_t*)idtr_location, *(uint32_t*)(idtr_location + 2));
	_set_idtr();
	//for(;;)asm("hlt");
	mprint("IDTR set, testing link.\n");
	asm volatile("int $0x2f");
	while(test_timeout-- != 0)
	{
		if(test_success != 0)
		{
			mprint("Test succeeded, disabling INT#0x2F\n");
			idt_register_interrupt(0x2F, (uint32_t)&__idt_default_handler);
			break;
		}
	}
	if(!test_success)
		panic("IDT link is offline (timeout).");
	terminal_writestring("Ending IRQ\n");
	return;
}
