# Declare constants for the multiboot header.
.set ALIGN,    1<<0             # align loaded modules on page boundaries
.set MEMINFO,  1<<1             # provide memory map
.set FLAGS,    ALIGN | MEMINFO  # this is the Multiboot 'flag' field
.set MAGIC,    0x1BADB002       # 'magic number' lets bootloader find the header
.set CHECKSUM, -(MAGIC + FLAGS) # checksum of above, to prove we are multiboot

# Declare Storage for multiboot header
.section .multiboot
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM

# Stack must be 16 byte aligned
.section .bss
.align 16
stack_bottom:
.skip 16384 # 16 KiB stack
stack_top:


.section .text
# Setup GDT table
.global lgdt_core
.type lgdt_core, @function
lgdt_core:
	movl 4(%esp), %eax
	lgdt (%eax)
	ret


# Setup IDT table
.global lidt_core
.type lidt_core, @function
lidt_core:
	movl 4(%esp), %eax
	lidt (%eax)
	ret

.macro IRQ i
    .global irq\i
    .type irq\i, @function
    irq\i:
	pushal
	mov $0x20, %al
	mov $0x20, %dx
	outb %al, %dx
	push $\i
	call _irq_default
	pop %edx
	popal
	iret
.endm
.macro IRQS i
    .global irq\i
    .type irq\i, @function
    irq\i:
	pushal
	mov $0x20, %al
	mov $0x20, %dx
	out %al, %dx
	mov $0xA0, %dx
	out %al, %dx
	push $\i
	call _irq_default
	pop %edx
	popal
	iret
.endm
IRQ 1
IRQ 2
IRQ 3
IRQ 4
IRQ 5
IRQ 6
IRQ 7
IRQS 8
IRQS 9
IRQS 10
IRQS 11
IRQS 12
IRQS 13
IRQS 14
IRQS 15
.global irq_default
.type irq_default, @function
irq_default:
	pushal
	mov $0x20, %al
	mov $0x20, %dx
	out %al, %dx
	popal
	iret



.global _start
.type _start, @function
_start:

	# Set up stack using our previous pointer
	mov $stack_top, %esp


	call kernel_main

	# If the system has nothing more to do, put the computer into an
	# infinite loop.
	cli 
1:	hlt
	jmp 1b

# Set the size of the _start symbol to the current location '.' minus its start.
# This is useful when debugging or when you implement call tracing.
.size _start, . - _start


