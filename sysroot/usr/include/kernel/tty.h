#ifndef _KERNEL_TTY_H
#define _KERNEL_TTY_H
 
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define terminal_clear terminal_initialize

//Toggle to write bottom to top
bool terminal_reverse;
//Toggle to false to not overwrite current color
bool terminal_changecolor;
void terminal_initialize(void);
void terminal_putchar(char c);
void terminal_write(const char* data, size_t size);
void terminal_writestring(const char* data);
void terminal_setcolor(uint8_t color);
void terminal_setcursor(size_t x, size_t y);
void terminal_printhex(uint8_t byte);
void terminal_writeordinal(uint32_t num);
#endif
