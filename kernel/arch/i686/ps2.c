#include <kernel/portb.h>
#include <kernel/ps2.h>
#include <kernel/tty.h>

#include <stdint.h>


#define PS2_DATAPORT 0x60
#define PS2_REGISTERPORT 0x64

#define DISABLEPORT1 0xAD
#define DISABLEPORT2 0xA7

#define ENABLEPORT1 0xAE
#define ENABLEPORT2 0xA8

#define TESTPORT1 0xAB
#define TESTPORT2 0xA9
#define PORTTESTPASS 0x00

#define READCONFIG 0x20
#define WRITECONFIG 0x60

#define SELFTEST 0xAA
#define SELFTESTPASS 0x55
#define SELFTESTFAIL 0xFC

#define ENABLESCANNING 0xF4
#define DISABLESCANNING 0xF5
#define PS2_IDENTIFY 0xF2

#define PORT_DISABLED		0
#define PORT_UNKNOWN		1
#define PORT_MOUSE 		2
#define PORT_MOUSE_SCROLL 	3
#define PORT_MOUSE_5		4
#define PORT_KEYBOARD		5

bool initialized;
int port1Mode = 0;
int port2Mode = 0;

inline bool canRead(){
	return (inportb(PS2_REGISTERPORT) & 1);
}
inline bool canWrite(){
	return !(inportb(PS2_REGISTERPORT) & (1<<1));
}
inline void cleanInput(){
	if(canRead()){
		terminal_writestring("PS2: Clearing garbage: "); 
		do { terminal_printhex(inportb(0x60)); } while(canRead());
		terminal_putchar('\n');
	}
}

void ps2_initialize(){
	// Information: http://wiki.osdev.org/"8042"_PS/2_Controller
	//		http://wiki.osdev.org/PS/2_Keyboard
	// TODO: Check if PS/2 ports exist
	// TODO: Timeouts on all outfull waits

	terminal_writestring("PS2: Disabling PS/2 ports for diagnostics\n");
	while(!canWrite()) continue;
	outportb(PS2_REGISTERPORT, DISABLEPORT1);
	outportb(PS2_REGISTERPORT, DISABLEPORT2);

	cleanInput();
	terminal_writestring("PS2: Read config: ");
	while(!canWrite()) continue;
	outportb(PS2_REGISTERPORT, READCONFIG);
	while(!canRead()) continue;
	uint8_t config = inportb(PS2_DATAPORT);
	terminal_writestring("PS2: Current config: ");
	for(uint8_t i = 0; i < 8; ++i) terminal_putchar('0');
	terminal_putchar('\n');

	port2Mode = config & (1<<5);
	if(port2Mode) terminal_writestring("PS2: Port2 enabled in config\n");
	else terminal_writestring("PS2: Port2 disabled in config\n");

	// Set interrupts off for both devices
	config &= ~(1 << 0);
	config &= ~(1 << 1);
	// Set translation off
	config &= ~(1 << 6);
	terminal_writestring("PS2: New config: ");
	for(uint8_t i = 0; i < 8; ++i) terminal_putchar('0');
	terminal_putchar('\n');

	terminal_writestring("PS2: Write config\n");
	while(!canWrite()) continue;
	outportb(PS2_REGISTERPORT, WRITECONFIG);
	outportb(PS2_DATAPORT, config);

	terminal_writestring("PS2: Performing self-test \n");
	while(!canWrite()) continue;
	outportb(PS2_REGISTERPORT, SELFTEST);
		while(!canRead()) continue;
	uint8_t test = inportb(PS2_DATAPORT);
	if(test == SELFTESTFAIL){
		//TODO: PS/2 has failed self-test
		terminal_writestring("PS2: Self-test failed\n");
		initialized = true;
		port1Mode = false;
		port2Mode = false;
		return;
	}else if(test != SELFTESTPASS){
		terminal_writestring("PS2: Self-test returned undefined response\n");
		//TODO: Unknown error has happened - Maybe buffers should be cleared before this
		initialized = true;
		port1Mode = false;
		port2Mode = false;
		return;
	}
	terminal_writestring("PS2: Self-test passed\n");

	if(port2Mode)
	{
		while(!canWrite()) continue;
		outportb(PS2_REGISTERPORT, ENABLEPORT2);
		while(!canWrite()) continue;
		outportb(PS2_REGISTERPORT, READCONFIG);
		while(!canRead()) continue;
		port2Mode = !(inportb(PS2_DATAPORT) & (1<<5));
		if(port2Mode){
			terminal_writestring("PS2: Port2 enabled definitely\n");
			while(!canWrite()) continue;
			outportb(PS2_REGISTERPORT, DISABLEPORT2);
		}
	}

	
	// Test if ports work
	// TODO: get specifics about port failures
	while(!canWrite()) continue;
	outportb(PS2_REGISTERPORT, TESTPORT1);
	while(!canRead()) continue;
	if(inportb(PS2_DATAPORT) == PORTTESTPASS) port1Mode = PORT_UNKNOWN;
	else terminal_writestring("PS2: Port1 failed test\n");
	if(port2Mode){
		while(!canWrite()) continue;
		outportb(PS2_REGISTERPORT, TESTPORT1);
		while(!canRead()) continue;
		if(inportb(PS2_DATAPORT) != PORTTESTPASS){
			 port2Mode = PORT_DISABLED;
			terminal_writestring("PS2: Port2 failed test\n");
		}
	}
	if(!port1Mode && !port2Mode){
		//TODO: Both ports don't work, throw and return
		initialized = true;
		return;
	}

	terminal_writestring("PS2: Finished port initialization, re-enabling ports\n");
	while(!canWrite()) continue;
	outportb(PS2_REGISTERPORT, ENABLEPORT1);
	while(!canWrite()) continue;
	outportb(PS2_REGISTERPORT, ENABLEPORT2);

	terminal_writestring("PS2: Disabling scanning on port 1\n");
	bool success = false;
	do{
		while(!canWrite()) continue;
		cleanInput();
		outportb(PS2_DATAPORT, DISABLESCANNING);
		while(!canRead()) continue;
		if(inportb(PS2_DATAPORT) == 0xFA) success = true;
	}while(!success);
	cleanInput();

	terminal_writestring("PS2: Asking port 1 for device identification\n");
	success = false;
	do{
		while(!canWrite()) continue;
		outportb(PS2_DATAPORT,PS2_IDENTIFY);
		while(!canRead()) continue;
		if(inportb(PS2_DATAPORT) == 0xFA) success = true;
	}while(!success);


	while(!canRead()) continue;
	uint8_t identity = inportb(PS2_DATAPORT);
	if(identity == 0xAB){
		//TODO:Keyboard
		cleanInput();
		terminal_writestring("PS2: Port 1 is a keyboard\n");
		port1Mode = PORT_KEYBOARD;
	}else if(identity == 0x00){
		port1Mode = PORT_MOUSE;
		terminal_writestring("PS2: Port 1 is a mouse\n");
	}else if(identity == 0x03){
		port1Mode = PORT_MOUSE;
		terminal_writestring("PS2: Port 1 is a mouse with scrollwhell\n");
	}else if(identity == 0x04){
		port1Mode = PORT_MOUSE;
		terminal_writestring("PS2: Port 1 is a 5 button mouse\n");
	}else{
		port1Mode = PORT_UNKNOWN;
		terminal_writestring("PS2: Port 1 is unknown device code: ");
		terminal_printhex(identity);
		while(canRead())	terminal_printhex(inportb(PS2_DATAPORT));
		terminal_putchar('\n');
	}
	
	terminal_writestring("PS2: Re-enabling scanning on port 1\n");
	do{
		while(!canWrite()) continue;
		outportb(PS2_DATAPORT, ENABLESCANNING);
		while(!canRead()) continue;
		if(inportb(PS2_DATAPORT) == 0xFA) success = true;
	}while(!success);
}


