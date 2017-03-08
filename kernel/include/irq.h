#ifndef _KERNEL_IRQ_H
#define _KERNEL_IRQ_H
int set_irq_func(int irq, void (*func)(void));
void clear_irq_func(int irq);
void irq_init();
#endif
