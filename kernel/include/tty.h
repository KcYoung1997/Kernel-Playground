#ifndef _KERNEL_TTY_H
#define _KERNEL_TTY_H
 
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define MODULE_NONE2(X, Y) 	tty_writef("[%s:%d] ", X, Y);
#define MODULE_NONE MODULE_NONE2(__FILENAME__, __LINE__)
#define MODULE_INFO2(X, Y) 	tty_writef("[\033cB0%s:%d\033r] ", X, Y);
#define MODULE_INFO MODULE_INFO2(__FILENAME__, __LINE__)
#define MODULE_SUCCESS2(X, Y) 	tty_writef("[\033c20%s:%d\033r] ", X, Y);
#define MODULE_SUCCESS MODULE_SUCCESS2(__FILENAME__, __LINE__)
#define MODULE_WARNING2(X, Y) 	tty_writef("[\033cE0%s:%d\033r] ", X, Y);
#define MODULE_WARNING MODULE_WARNING2(__FILENAME__, __LINE__)
#define MODULE_ERROR2(X, Y) 	tty_writef("\033c0F[\033c40%s:%d\033c0F]\033r ", X, Y);
#define MODULE_ERROR MODULE_ERROR2(__FILENAME__, __LINE__)

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
