#ifndef _KERNEL_PS2_H
#define _KERNEL_PS2_H
#include <stdbool.h>

extern bool initialized;
extern int port1Mode;
extern int port2Mode;

void ps2_initialize();
#endif
