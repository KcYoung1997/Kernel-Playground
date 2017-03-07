#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <kernel/tty.h>
#include <kernel/ps2.h>
#include <kernel/portb.h>
#include <kernel/cmos.h>
#include <kernel/irq.h>


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
		tty_writechar(char1-10 + 'A');
	else
		tty_writechar(char1 % 10 + '0');
	if(char2>9)
		tty_writechar(char2-10 + 'A');
	else
		tty_writechar(char2 % 10 + '0');
}

inline uint8_t PIT_getclock(){
	return inportb(0x40);
}
void exitSplash(void) {
	tty_clear();
	for(;;){
		char c = keyboard_read();
		if(c) tty_writechar(c);
	}

}
void splash(void) {
	tty_clear();
	uint8_t* vga = (uint8_t*) 0xB8000;
	srand(PIT_getclock() << 24 | PIT_getclock() << 16 | PIT_getclock() << 8 | PIT_getclock());
	tty_writestring("\n\n\n\n           kkkkkkkk                OOOOOOOOO        SSSSSSSSSSSSSSS\n           k::::::k              OO:::::::::OO    SS:::::::::::::::S\n           k::::::k            OO:::::::::::::OO S:::::SSSSSS::::::S\n           k::::::k           O:::::::OOO:::::::OS:::::S     SSSSSSS\n            k:::::k    kkkkkkkO::::::O   O::::::OS:::::S\n            k:::::k   k:::::k O:::::O     O:::::OS:::::S\n            k:::::k  k:::::k  O:::::O     O:::::O S::::SSSS\n            k:::::k k:::::k   O:::::O     O:::::O  SS::::::SSSSS\n            k::::::k:::::k    O:::::O     O:::::O    SSS::::::::SS\n            k:::::::::::k     O:::::O     O:::::O       SSSSSS::::S\n            k:::::::::::k     O:::::O     O:::::O            S:::::S\n            k::::::k:::::k    O::::::O   O::::::O            S:::::S\n           k::::::k k:::::k   O:::::::OOO:::::::OSSSSSSS     S:::::S\n           k::::::k  k:::::k   OO:::::::::::::OO S::::::SSSSSS:::::S\n           k::::::k   k:::::k    OO:::::::::OO   S:::::::::::::::SS\n           kkkkkkkk    kkkkkkk     OOOOOOOOO      SSSSSSSSSSSSSSS\n\n                       \xC9\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xBB\n                       \xBA Press any key to continue... \xBA\n                       \xC8\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xBC");
	uint8_t last = 0;
	while(true){
		uint8_t temp = (((rand()%0x0E)+1)<< 4);
		while(temp==last) temp = rand();
		for(size_t i = 0; i < 24; ++i)
		{
		 	vga[i*160 +1] = temp;
			for(size_t i = 0; i < 2000000; ++i)  rand();
		}
		for(size_t i = 0; i < 79; ++i)
		{
		 	vga[i*2 +80*24*2 + 1] = temp;
			for(size_t i = 0; i < 2000000; ++i)  rand();
		}
		for(size_t i = 24; i >0; --i)  
		{
		 	vga[i*160 +159] = temp;
			for(size_t i = 0; i < 2000000; ++i)  rand();
		}
		for(size_t i = 80; i > 0; --i)
		{
		 	vga[i*2 +1] = temp;
			for(size_t i = 0; i < 2000000; ++i)  rand();
		}
		last = temp;
	}
}


void kernel_main(void) {
	tty_init();
	ps2_init();
	cmos_init();
	irq_init();
	// Enable interrupts
	asm volatile("sti");
	splash();
	for(;;) {asm volatile("hlt");}
}
