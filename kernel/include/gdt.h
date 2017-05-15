#ifndef _KERNEL_GDT_H
#define _KERNEL_GDT_H
void set_gdt(int num, unsigned int base, unsigned int limit, unsigned char access, unsigned char flags);
void gdt_init(void);
#endif
