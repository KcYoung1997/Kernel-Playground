#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <kernel/tty.h>
#include <kernel/keyboard.h>
#include <kernel/portb.h>
#include <string.h>

static unsigned long int next = 1;

int rand( void ) // RAND_MAX assumed to be 32767
{
    next = next * 1103515245 + 12345;
    return (unsigned int)(next / 65536) % 32768;
}
 
void srand( unsigned int seed )
{
    next = seed;
}

inline void print_hex(uint8_t in){
	uint8_t char1 = in >> 4;
	uint8_t char2 = in % 16;
	if(char1>9)
		terminal_putchar(char1-10 + 'A');
	else
		terminal_putchar(char1 % 10 + '0');
	if(char2>9)
		terminal_putchar(char2-10 + 'A');
	else
		terminal_putchar(char2 % 10 + '0');
}

inline uint8_t PIT_getclock(){
	return inportb(0x40);
}
void kernel_main(void) {
	/* Initialize terminal interface */
	terminal_initialize();
	//keyboard_initialize();
	uint8_t lastkey = 1;

//	terminal_reverse = true;
	srand(PIT_getclock() << 24 | PIT_getclock() << 16 | PIT_getclock() << 8 | PIT_getclock());
	while(true){

		terminal_setcolor(rand());
		uint8_t temp = rand();
		if(temp != '\n' && temp != '\t'&& temp != '\b' && temp != '\r')
			for(size_t i = rand() % 8; i < 8; ++i)
				terminal_putchar(temp);
		for(size_t i = 0; i < 1000000; i++) rand();
	}
}
