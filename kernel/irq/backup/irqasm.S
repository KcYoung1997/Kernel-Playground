
.global lidt_core
.global default_isr
.global default_irq

.global irq_keyboard
.extern keyboard

lidt_core:
	movl 4(%esp), %eax
	lidt (%eax)
	ret
default_isr:
	iret
	ret
default_irq:
	mov $0x20, %al
	outb %al, $0x20
	iret
	ret

irq_keyboard:
	push %eax
	mov $0x60, %dx
	inb %dx, %al 

	call keyboard	

	mov $0x20, %al
	outb %al, $0x20
	pop %eax
	iret
	ret
