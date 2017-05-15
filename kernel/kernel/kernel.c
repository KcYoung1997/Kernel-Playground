#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <kernel/tty.h>
#include <kernel/ps2.h>
#include <kernel/portb.h>
#include <kernel/cmos.h>
#include <kernel/irq.h>
#include <kernel/gdt.h>
#include <kernel/pic.h>
#include <kernel/tss.h>

#include <cpuid.h>

#define CPUID_NAME_STRING_1 0x80000002
#define CPUID_NAME_STRING_2 0x80000003
#define CPUID_NAME_STRING_3 0x80000004

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

inline uint8_t PIT_getclock(){
	return inportb(0x40);
}
void exitSplash(void) {
	tty_clear();
	for(;;){
		char c = keyboard_read();
		if(c) { if(c=='1') return; else tty_writechar(c); };
	}

}

const char * days[7] = {  "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
const char * months[12] = {  "January","February","March","April","May","June","July","August","September","October","November","December" };
void printTime(){
	struct time current = get_rtc();
	tty_cursorposition(36,1);

	int d = current.day;
	int y = (current.century*100)+current.year;
	int m = current.month;	
	int weekday  = (d += d< 3 ? y-- : y - 2, 23*m/9 + d + 4 + y/4- y/100 + y/400)%7;
	tty_writef("%02d:%02d:%02d", current.hour, current.minute, current.second);
	tty_cursorposition(28,2);
	tty_writef("%s the %t of %s", days[weekday], current.day, months[current.month-1]);
	tty_cursorposition(35,3);
}

void splash(void) {
	set_irq_func(1, exitSplash);
	set_irq_func(8,printTime);
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

static int ticks;
void sleep_countdown(void) { ticks--; }
void sleep(int _ticks) {
	ticks = _ticks;
	set_irq_func(8, sleep_countdown);
	while(ticks>0) tty_writef("");
}

void kernel_main(void) {
	tty_init();
	ps2_init();
	cmos_init();
	irq_init();
	gdt_init();
	pic_init();
	tss_init();

	uint32_t ret[12];
	for(int i = 0; i < 3; i++){
		uint32_t eax, ebx, ecx, edx;

  		__cpuid (0x80000002+i, eax, ebx, ecx, edx);
		*(ret+(i*4)) = eax;
		*(ret+(i*4)+1) = ebx;
		*(ret+(i*4)+2) = ecx;
		*(ret+(i*4)+3) = edx;
	}
	tty_writestring((char*)ret);
	tty_writechar(*"\n");


	// Enable interrupts
	asm volatile("sti");
	tty_writestring("Entering splash screen in: ");
	for(int i = 5; i > 0; i--) {
		tty_writef("%d... ", i);
		sleep(4);
	}
	splash();
	for(;;) {asm volatile("hlt");}
}
