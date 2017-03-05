
#include <stdint.h>
#include <stdbool.h>

#include <kernel/keyboard.h>
#include <kernel/portb.h>
#include <kernel/tty.h>

#define KDB_DATAPORT 0x60
#define KBD_REGISTERPORT 0x64

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

bool initialised = false;
bool port1Enabled = false;
bool port2Enabled = false;
uint8_t keys[16];
enum keyPS2{
	F9 = 0x1, 	F5 = 0x3,	F3 = 0x4, 	F1 = 0x5, 	F2 = 0x6, 	F12 = 0x7, 
	F10 = 0x9, 	F8 = 0x0A, 	F6 = 0x0B, 	F4 = 0x0C, 	TAB = 0x0D, 	GRAVE = 0x0E,
	LALT = 0x11, 	LSHFT = 0x12, 	LCTRL = 0x14, 	Q = 0x15, 	ONE = 0x16, 	Z = 0x1A,
	S = 0x1B, 	A = 0x1C, 	W = 0x1D, 	TWO = 0x1E, 	C = 0x21, 	X = 0x22, 	
	D = 0x23,	E = 0x24, 	FOUR = 0x25, 	THREE = 0x26, 	SPACE = 0x29, 	V = 0x2A,
	F = 0x2B,	T = 0x2C, 	R = 0x2D, 	FIVE = 0x2E, 	N = 0x31, 	B = 0x32,
	H = 0x33,	G = 0x34, 	Y = 0x35, 	SIX = 0x36,	M = 0x3A, 	J = 0x3B,
	U = 0x3C, 	SEVEN = 0x3D, 	EIGTH = 0x3E,	COMMA = 0x41, 	K = 0x42, 	I = 0x43,
	O = 0x44, 	NINE = 0x46, 	ZERO = 0x49, 	FWDSLASH = 0x4A,L = 0x4B,	SEMICOLON = 0x4C,
	P = 0x4D,	MINUS = 0x4E,	APOSTRAPHE = 0x52,		LSQBRACKET = 0x54,	
	EQUALS = 0x55,	CAPS = 0x58,	RSHFT = 0x59,	ENTER = 0x5A,	RSQBRACKET = 0x5B,	
	BACKSLASH = 0x5D,BKSP = 0x66,	KP1 = 0x69,	KP4 = 0x6B,	KP7 = 0x6C,	KP0 = 0x70,
	KPDOT = 0x71,	KP2 = 0x72,	KP5 = 0x73,	KP6 = 0x74,	KP8 = 0x75,	ESC = 0x76,
	NUM = 0x77,	F11 = 0x78,	KPPLUS = 0x79,	KP3 = 0x7A,	KPMINUS = 0x7B,	KPMULTI = 0x7C,
	KP9 = 0x7D,	SCROLL = 0x7E,	F7 = 0x83
};
/*enum keyKernel{
	A = 0x1, 	B = 0x2, 	C = 0x3, 	D = 0x4, 	E = 0x5,	F = 0x6,
	G = 0x7,	H = 0x8,	I = 0x9,	J = 0xA,	K = 0xB		L = 0xC
	M = 0xD,	N = 0xE,	O = 0xF,	P = 0xB,	Q = 		R = 
	S = 		T = 		U = 		V = 		W = 		X = 
	Y = 		Z = 
}*/

inline bool outfull(){
	return inportb(KBD_REGISTERPORT) & (1<<1);
}

inline bool infull(){
	return inportb(KBD_REGISTERPORT) & 1;
}

inline void cleanin(){
	if(infull()){
		terminal_writestring("KEYBOARD: Clearing garbage: \n"); 
		do { terminal_putchar(inportb(0x60)); } while(infull());
		terminal_putchar('\n');
	}
}


void nextExtended(){

}
void nextUp(){
	uint8_t key = inportb(0x60);
}
bool next(){
	if(infull()){
		// Get key
		uint8_t key = inportb(0x60);
		bool keyDown = true;
		// If the key is part of the extended spec, use extended function
		if(key == 0xE0) nextExtended();
		else {
			// If key was released, set keydown to false
			// and get the next key to release it
			if(key == 0xF0){
				keyDown = false;
				key = inportb(0x60);
			}
			
			
		}
		return true;
	}
	else return false;
}
void readall(){
	while(next());
}

void keyboard_initialize(){
	// Information: http://wiki.osdev.org/"8042"_PS/2_Controller
	// TODO: Check if PS/2 ports exist
	// TODO: Timeouts on all outfull waits
	// Temporarily disable both PS/2 ports - waiting until ready to accept input
	while(outfull()) continue;
	outportb(KBD_REGISTERPORT, DISABLEPORT1);
	outportb(KBD_REGISTERPORT, DISABLEPORT2);
	terminal_writestring("KEYBOARD: Disabled PS/2 ports for diagnostics\n");
	//Flush output buffer by checking if there is any input then taking it without storing
	cleanin();
	// Request then read Controller configuration byte
	{
		while(outfull()) continue;
		outportb(KBD_REGISTERPORT, READCONFIG);
		uint8_t config = inportb(KDB_DATAPORT);
		terminal_writestring("KEYBOARD: Current config: ");
		for(size_t i = 0; i < 8; ++i) terminal_putchar('0');
		terminal_putchar('\n');
		// Get if two PS/2 devices exist (bit 5)
		port2Enabled = config & (1<<5);
		if(port2Enabled) terminal_writestring("KEYBOARD: Port2 enabled in config\n");
		else terminal_writestring("KEYBOARD: Port2 disabled in config\n");
		// Set interrupts off for both devices
		config &= ~(1 << 0);
		config &= ~(1 << 1);
		// Set translation off
		config &= ~(1 << 6);
		terminal_writestring("KEYBOARD: New config: ");
		for(size_t i = 0; i < 8; ++i) terminal_putchar('0');
		terminal_putchar('\n');
		// Write back the modified configuration byte
		while(outfull()) continue;
		outportb(KBD_REGISTERPORT, WRITECONFIG);
		outportb(KDB_DATAPORT, config);
	}
	// Perform controller self-test
	while(outfull()) continue;
	outportb(KBD_REGISTERPORT, SELFTEST);
	{
		while(!infull()) continue;
		uint8_t test = inportb(KDB_DATAPORT);
		if(test == SELFTESTFAIL){
			//TODO: PS/2 has failed self-test
			terminal_writestring("KEYBOARD: Self-test failed\n");
			initialised = true;
			port1Enabled = false;
			port2Enabled = false;
			return;
		}else if(test != SELFTESTPASS){
			terminal_writestring("KEYBOARD: Self-test returned undefined response\n");
			//TODO: Unknown error has happened - Maybe buffers should be cleared before this
			initialised = true;
			port1Enabled = false;
			port2Enabled = false;
			return;
		}
		terminal_writestring("KEYBOARD: Self-test passed\n");
	}
	// Test for dual channel properly
	if(port2Enabled)
	{
		outportb(KBD_REGISTERPORT, ENABLEPORT2);
		port2Enabled = !(inportb(KDB_DATAPORT) & (1<<5));
		if(port2Enabled){
			terminal_writestring("KEYBOARD: Port2 enabled definitely\n");
			outportb(KBD_REGISTERPORT, DISABLEPORT2);
		}
	}
	// Test if ports work
	// TODO: get specifics about port failures
	outportb(KBD_REGISTERPORT, TESTPORT1);
	if(inportb(KDB_DATAPORT) == PORTTESTPASS) port1Enabled = true;
	else terminal_writestring("KEYBOARD: Port1 failed test\n");
	if(port2Enabled){
		outportb(KBD_REGISTERPORT, TESTPORT1);
		if(inportb(KDB_DATAPORT) != PORTTESTPASS){
			 port2Enabled = false;
			terminal_writestring("KEYBOARD: Port2 failed test\n");
		}
	}
	if(!port1Enabled && !port2Enabled){
		//TODO: Both ports don't work, throw and return
		initialised = true;
		return;
	}
	// Re-enable both PS/2 ports
	outportb(KBD_REGISTERPORT, ENABLEPORT1);
	outportb(KBD_REGISTERPORT, ENABLEPORT2);
	terminal_writestring("KEYBOARD: Finished initialization\n");
	// TODO: re-enable interrupts once implemented
	// TODO: Device resets when accessed
}
