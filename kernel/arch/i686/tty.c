#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#include <kernel/tty.h>
 
#include "vga.h"
 
static const size_t VGA_WIDTH = 80;
static const size_t VGA_HEIGHT = 25;
static uint16_t* const VGA_MEMORY = (uint16_t*) 0xB8000;

bool terminal_reverse = false;
bool terminal_changecolor = true;

static size_t terminal_row;
static size_t terminal_column;
static uint8_t terminal_color;
static uint16_t* terminal_buffer;

//TODO: This belongs in <string.h> when implemented
size_t strlen(const char* str) {
	size_t len = 0;
	while (str[len])
		len++;
	return len;
}
 
void terminal_initialize(void) {
	terminal_row = 0;
	terminal_column = 0;
	terminal_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
	terminal_buffer = VGA_MEMORY;
	for (size_t y = 0; y < VGA_HEIGHT; y++) {
		for (size_t x = 0; x < VGA_WIDTH; x++) {
			const size_t index = y * VGA_WIDTH + x;
			terminal_buffer[index] = vga_entry(' ', terminal_color);
		}
	}
}
 
void terminal_setcolor(uint8_t color) {
	terminal_color = color;
}

void terminal_setcursor(size_t x, size_t y){
	terminal_row = y;
	terminal_column = x;
}
 
void terminal_putentryat(unsigned char c, uint8_t color, size_t x, size_t y) {
	const size_t index = (terminal_reverse ?  VGA_HEIGHT - y : y) * VGA_WIDTH + x;
	if(terminal_changecolor){
		terminal_buffer[index] = vga_entry(c, color);
	}else{
		terminal_buffer[index] = (terminal_buffer[index] & 0xff00) | c;
	}
}
 
void terminal_scroll(size_t count) {
	const size_t shift = count * VGA_WIDTH;
	for (size_t y = count; y < VGA_HEIGHT; y++) {
		for (size_t x = 0; x < VGA_WIDTH; x++) {
			const size_t index = (terminal_reverse ? VGA_HEIGHT - y : y) * VGA_WIDTH + x;
			terminal_buffer[index+(terminal_reverse?shift:-shift)] = terminal_buffer[index];
			terminal_buffer[index] = 0;
		}
	}
	--terminal_row;
}
 
void terminal_putchar(char c) {
	if(c == '\n' || c == '\r'){
		// Newline
		terminal_column = 0;
		if (++terminal_row == VGA_HEIGHT)
			terminal_scroll(1);
	}else if(c == '\t'){
		// TAB
		do{
			terminal_putchar(' ');
		}while(terminal_column % 8);
	}else if(c == '\b'){
		//backspace
		--terminal_column;
		terminal_putentryat(0, 0, terminal_column, terminal_row);
	}else{
		terminal_putentryat(c, terminal_color, terminal_column, terminal_row);
		if (++terminal_column == VGA_WIDTH) {
			terminal_column = 0;
			if (++terminal_row == VGA_HEIGHT)
				terminal_scroll(1);
		}
	}
}
 
void terminal_write(const char* data, size_t size) {
	for (size_t i = 0; i < size; i++)
		terminal_putchar(data[i]);
}
 
void terminal_writestring(const char* data) {
	terminal_write(data, strlen(data));
}
