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
	uint8_t* vga = (uint8_t*) 0xB8000;
//	terminal_reverse = true;
	srand(PIT_getclock() << 24 | PIT_getclock() << 16 | PIT_getclock() << 8 | PIT_getclock());
	terminal_writestring("\n\n\n\n           kkkkkkkk                OOOOOOOOO        SSSSSSSSSSSSSSS\n           k::::::k              OO:::::::::OO    SS:::::::::::::::S\n           k::::::k            OO:::::::::::::OO S:::::SSSSSS::::::S\n           k::::::k           O:::::::OOO:::::::OS:::::S     SSSSSSS\n            k:::::k    kkkkkkkO::::::O   O::::::OS:::::S\n            k:::::k   k:::::k O:::::O     O:::::OS:::::S\n            k:::::k  k:::::k  O:::::O     O:::::O S::::SSSS\n            k:::::k k:::::k   O:::::O     O:::::O  SS::::::SSSSS\n            k::::::k:::::k    O:::::O     O:::::O    SSS::::::::SS\n            k:::::::::::k     O:::::O     O:::::O       SSSSSS::::S\n            k:::::::::::k     O:::::O     O:::::O            S:::::S\n            k::::::k:::::k    O::::::O   O::::::O            S:::::S\n           k::::::k k:::::k   O:::::::OOO:::::::OSSSSSSS     S:::::S\n           k::::::k  k:::::k   OO:::::::::::::OO S::::::SSSSSS:::::S\n           k::::::k   k:::::k    OO:::::::::OO   S:::::::::::::::SS\n           kkkkkkkk    kkkkkkk     OOOOOOOOO      SSSSSSSSSSSSSSS\n\n                       \xC9\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xBB\n                       \xBA Press any key to continue... \xBA\n                       \xC8\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xBC");
	while(true){
		uint8_t temp = ((rand()%8)<<4) | ((rand()%8)+8);
		for(size_t i = 0; i < 80*25; ++i){
			vga[i*2+1] = temp;
		}
		for(size_t i = 0; i < 100000000; i++) rand();
	}
}
