#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>

#include <kernel/tty.h>
 
#include "vga.h"

 
static const size_t VGA_WIDTH = 80;
static const size_t VGA_HEIGHT = 25;
static uint16_t* const VGA_MEMORY = (uint16_t*) 0xB8000;

bool tty_reverse = false;
bool tty_changecolor = true;

size_t tty_row;
size_t tty_column;
uint8_t tty_color;
uint16_t* tty_buffer;

//TODO: This belongs in <string.h> when implemented
size_t strlen(const char* str) {
	size_t len = 0;
	while (str[len])
		len++;
	return len;
}
 
void tty_init(void) {
	tty_row = 0;
	tty_column = 0;
	tty_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
	tty_buffer = VGA_MEMORY;
	for (size_t y = 0; y < VGA_HEIGHT; y++) {
		for (size_t x = 0; x < VGA_WIDTH; x++) {
			const size_t index = y * VGA_WIDTH + x;
			tty_buffer[index] = vga_entry(' ', tty_color);
		}
	}
}
 
void tty_cursorcolor(uint8_t color) {
	tty_color = color;
}

void tty_cursorposition(size_t x, size_t y){
	tty_row = y;
	tty_column = x;
}
 
void tty_putentryat(unsigned char c, uint8_t color, size_t x, size_t y) {
	const size_t index = (tty_reverse ?  VGA_HEIGHT - y : y) * VGA_WIDTH + x;
	if(tty_changecolor){
		tty_buffer[index] = vga_entry(c, color);
	}else{
		tty_buffer[index] = (tty_buffer[index] & 0xff00) | c;
	}
}
 
void tty_scroll(size_t count) {
	const size_t shift = count * VGA_WIDTH;
	for (size_t y = count; y < VGA_HEIGHT; y++) {
		for (size_t x = 0; x < VGA_WIDTH; x++) {
			const size_t index = (tty_reverse ? VGA_HEIGHT - y : y) * VGA_WIDTH + x;
			tty_buffer[index+(tty_reverse?shift:-shift)] = tty_buffer[index];
			tty_buffer[index] = 0;
		}
	}
	--tty_row;
}


void tty_writechar(char c) {
	if(c == '\n' || c == '\r'){
		// Newline
		tty_column = 0;
		if (++tty_row == VGA_HEIGHT)
			tty_scroll(1);
	}else if(c == '\t'){
		// TAB
		do{
			tty_writechar(' ');
		}while(tty_column % 8);
	}else if(c == '\b'){
		//backspace
		--tty_column;
		tty_putentryat(0, 0, tty_column, tty_row);
	}else{
		tty_putentryat(c, tty_color, tty_column, tty_row);
		if (++tty_column == VGA_WIDTH) {
			tty_column = 0;
			if (++tty_row == VGA_HEIGHT)
				tty_scroll(1);
		}
	}
}
 
void tty_write(const char* data, size_t size) {
	for (size_t i = 0; i < size; i++)
	{
		if(data[i] == '\033'){
			//ANSI escape sequence https://en.wikipedia.org/wiki/ANSI_escape_code
			i++;
			if(data[i] == 'r') {
				tty_cursorcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
			}else if(data[i] == 'c') {
				tty_cursorcolor(vga_entry_color(
					data[i+1]<= '9'? data[i+1]-'0' :
					data[i+1]<='F'? data[i+1]-'A'+10:
							data[i+1]-'a'+10,
					data[i+2]<= '9'? data[i+2]-'0' :
					data[i+2]<='F'? data[i+2]-'A'+10:
							data[i+2]-'a'+10));
				i+=2;
			}
			continue;
		}
		tty_writechar(data[i]);
	}
}
 
void tty_writestring(const char* data) {
	tty_write(data, strlen(data));
}

int tty_writef(const char* restrict format, ...) {
	va_list parameters;
	va_start(parameters, format);
 
	size_t written = 0;
	while(*format != '\0'){
		//If not special format or is "%%"
		if(format[0] != '%' || format[1] == '%'){
			//Skip first %
			if(format[0] == '%') format++;
			size_t amount = 1;
			//Increment until % found or EOF
			while(format[amount] && format[amount] != '%')
				amount++;
			tty_write(format, amount);
			format += amount;
			written += amount;
			continue;
		}
		//store start of format and increment
//		const char* format_begin = format++;
		//FLAGS
		format++;
		int flag = 0;
		for(;format;format++){
			#define LEFTJUSTIFY 1
			if(*format == '-') 	flag |= LEFTJUSTIFY;
			#define PREPENDPLUS 2
			else if(*format == '+') flag |= PREPENDPLUS;
			#define PREPENDNONE 4
			else if(*format == ' ')	flag |= PREPENDNONE;
			#define PREPENDLONG 8
			else if(*format == '#')	flag |= PREPENDLONG;
			else if(*format == '0')	flag |= 16;
			else{break;}
		}
		//WIDTH
		int width = 0;
		if(*format == '*') { width = va_arg(parameters, int); format++;}
		else for(;*format >= '0' && *format <= '9';format++){
			width = width*10 + *format-'0';
		}
		width = width==0 ? 1 : width;
		//PRECISION
		int precision = -1;
		if(*format == '.') {
			if(*format == '*') { precision = va_arg(parameters, int); format++;}
			else for(precision = 0;*format >= '0' && *format <= '9';format++){
				precision = precision*10 + *format-'0';
			}
		}
		//TODO:LENGTH
		//SPRCIFIER
		if(*format == 'd' || *format == 'i') {
			//TODO flags (DONE: prepend)
			//Signed int
			//Get next int
			int num = va_arg(parameters, int);
			//If its 0, just write them
			if(num<0) {
				if(!(flag&PREPENDNONE)) {
					tty_writechar('-');
				}
			} else {
				if(flag&PREPENDPLUS) {
					tty_writechar('+');
				}				
			}
			char digits[10];
			//Setup width digits with '0'
			for(int i = 0; i < width; i++, num /=10) 
				digits[10-i] = '0' + num % 10;
			//keep going until no more digits
			for(;width<10 && num > 0; width++, num /=10) 
				digits[10-width] = '0' + num % 10;
			tty_write(((const char*)digits)+11-width,width);
			written += width;
		} else	if(*format == 'u' ) {
			//TODO flags (DONE: prepend)
			//Unsigned int - Dec
			//Get next uint
			unsigned int num = va_arg(parameters, unsigned int);
			//If its 0, just write it
			if(flag&PREPENDPLUS) {
				tty_writechar('+');
			}
			char digits[10];
			//Setup width digits with '0'
			for(int i = 0; i < width; i++, num /=10) 
				digits[10-i] = '0' + num % 10;
			//keep going until no more digits
			for(;width>0 && num > 0; width++, num /=10) 
				digits[10-width] = '0' + num % 10;
			tty_write(((const char*)digits)+11-width,width);
			written += width;
		} else	if(*format == 'o') {
			//Unsigned int = oct

		} else	if(*format == 'c') {
			//Character
		} else	if(*format == 's') {
			//char*
			const char* str = va_arg(parameters, const char*);
			int len = strlen(str);
			written += len;
			tty_write(str,len);
		} else	if(*format == 'p') {
			//Pointer address
		} else	if(*format == 'n') {
			//Nothing to print
			//Place written count in va_arg
		} else	if(*format == 't') {
			//ordinal
			unsigned int num = va_arg(parameters, unsigned int);
			written += tty_writef("%u", num) + 2;
			switch(num % 100)
			{
			case 11:
			case 12:
			case 13:
			    tty_writestring("th");
			}

			switch(num % 10)
			{
			case 1:
			    tty_writestring("st");
			case 2:
			    tty_writestring("nd");
			case 3:
			    tty_writestring("rd");
			default:
			    tty_writestring("th");
			}

		} else { 
			//options with capitals
			//int capital = 0;
			if(*format == 'x' || *format == 'X') {
				//TODO flags, capitals
				//Unsigned int - hex
				unsigned int num = va_arg(parameters, unsigned int);
				char digits[8];
				for(int i = 0; i < 8; i++, num /= 0x10){
					uint8_t res = num % 0x10;
					if(res<10) {
						digits[7-i] = '0'+res;
					}
					else {
						// Dirty hack for capitalisation
						// *format-23 is: 'a' if 'x', 'A' if 'X'
						digits[7-i] = res + (*format)-33;
					}
				}
				int written = 0;
				if(flag&PREPENDLONG){
					tty_write("0x",2);
					written += 2;
				}
				int skip = 0;
				while(digits[skip] == '0' && skip<8-width) skip++;
				tty_write(((const char*)(digits))+skip, 8-skip);
				written += 8-skip;
//		format+=2;
			} else	if(*format == 'f') {
				//Double - floating point
			} else	if(*format == 'e') {
				//Double - scientific notation
			} else	if(*format == 'g') {
				//Double - shortest notation %e | %f
			} else {
				/*TODO: Throw incorrect param*/ 
			}
		}
		format++;
	}

 
	va_end(parameters);
	return written;
}



void tty_printhex(uint8_t byte){
	tty_writechar('0');
	tty_writechar('x');
	tty_writechar(byte>=160? 'A'+(byte/16)-10 : '0'+(byte/16));
	byte %= 16;
	tty_writechar(byte>=10? 'A'+byte-10 : '0'+byte);
}

void tty_writeordinal(uint32_t num) {
	switch(num % 100)
	{
	case 11:
	case 12:
	case 13:
	    tty_writestring("th");
	}

	switch(num % 10)
	{
	case 1:
	    tty_writestring("st");
	case 2:
	    tty_writestring("nd");
	case 3:
	    tty_writestring("rd");
	default:
	    tty_writestring("th");
	}
}
