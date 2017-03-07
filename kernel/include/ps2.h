#ifndef _KERNEL_PS2_H
#define _KERNEL_PS2_H
#include <stdbool.h>

extern bool ps2_init_done;
extern int port1Mode;
extern int port2Mode;

void ps2_init();
char keyboard_read();
#endif
