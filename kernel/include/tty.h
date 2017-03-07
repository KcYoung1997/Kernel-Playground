#ifndef _KERNEL_TTY_H
#define _KERNEL_TTY_H
 
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define tty_clear tty_init

//Toggle to write bottom to top
bool tty_reverse;
//Toggle to false to not overwrite current color
bool tty_changecolor;
void tty_init(void);

void tty_writechar(char c);
void tty_write(const char* data, size_t size);
void tty_writestring(const char* data);
int tty_writef(const char* restrict format, ...);

void tty_cursorcolor(uint8_t color);
void tty_cursorposition(size_t x, size_t y);

void tty_printhex(uint8_t byte);
void tty_writeordinal(uint32_t num);
#endif
