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
#define PS2_RESET 0xFF

#define PORT_DISABLED		0
#define PORT_UNKNOWN		1
#define PORT_MOUSE 		2
#define PORT_MOUSE_SCROLL 	3
#define PORT_MOUSE_5		4
#define PORT_KEYBOARD		5

bool ps2_init_done = false;
int port1Mode = 0;
int port2Mode = 0;

inline void safe_write(uint8_t port, uint8_t data) {
	while(inportb(PS2_REGISTERPORT) & (1<<1)) continue;
	outportb(port, data);
}

inline uint8_t safe_read(uint8_t port) {
	while(!(inportb(PS2_REGISTERPORT) & 1)) continue;
	return inportb(port);
}
inline bool canRead(){
	return (inportb(PS2_REGISTERPORT) & 1);
}
inline void cleanInput(){
	if(canRead()){
		MODULE_INFO tty_writestring("Clearing garbage: "); 
		do { tty_printhex(inportb(0x60)); } while(canRead());
		tty_writechar('\n');
	}
}

void ps2_init(){
	// Information: http://wiki.osdev.org/"8042"_PS/2_Controller
	//		http://wiki.osdev.org/PS/2_Keyboard
	// TODO: Check if PS/2 ports exist
	// TODO: Timeouts on all outfull waits

	MODULE_INFO tty_writestring("init start\n");
	safe_write(PS2_REGISTERPORT, DISABLEPORT1);
	safe_write(PS2_REGISTERPORT, DISABLEPORT2);

	cleanInput();
	MODULE_INFO tty_writestring("Current config: ");
	safe_write(PS2_REGISTERPORT, READCONFIG);
	uint8_t config = safe_read(PS2_DATAPORT);
	for(uint8_t i = 0; i < 8; ++i) tty_writechar('0' + ((config&(1<<i))!= 0));
	port2Mode = config & (1<<5);
	if(port2Mode) tty_writestring(" Port2 enabled\n");
	else tty_writestring(" Port2 disabled\n");

	//TODO: Set interrupts off for both devices
	config &= ~(1 << 0);
	config &= ~(1 << 1);
	// Set translation off
	config &= ~(1 << 6);

	MODULE_INFO tty_writestring("Write new config ");
	for(uint8_t i = 0; i < 8; ++i) tty_writechar('0' + ((config&(1<<i))!= 0));
	tty_writechar('\n');
	safe_write(PS2_REGISTERPORT, WRITECONFIG);
	safe_write(PS2_DATAPORT, config);

	MODULE_INFO tty_writestring("Performing self-test \n");
	safe_write(PS2_REGISTERPORT, SELFTEST);
	uint8_t test = safe_read(PS2_DATAPORT);
	if(test == SELFTESTFAIL){
		//TODO: PS/2 has failed self-test
		MODULE_ERROR tty_writestring("Self-test failed\n");
		ps2_init_done = true;
		port1Mode = false;
		port2Mode = false;
		return;
	}else if(test != SELFTESTPASS){
		MODULE_ERROR tty_writestring("Self-test returned undefined response\n");
		//TODO: Unknown error has happened - Maybe buffers should be cleared before this
		ps2_init_done = true;
		port1Mode = false;
		port2Mode = false;
		return;
	}
	MODULE_SUCCESS tty_writestring("Self-test passed\n");

	if(port2Mode)
	{
		safe_write(PS2_REGISTERPORT, ENABLEPORT2);
		safe_write(PS2_REGISTERPORT, READCONFIG);
		port2Mode = !(safe_read(PS2_DATAPORT) & (1<<5));
		if(port2Mode){
			MODULE_INFO tty_writestring("Port2 exists\n");
			safe_write(PS2_REGISTERPORT, DISABLEPORT2);
		}
	}

	
	// Test if ports work
	// TODO: get specifics about port failures
	safe_write(PS2_REGISTERPORT, TESTPORT1);
	if(safe_read(PS2_DATAPORT) == PORTTESTPASS) port1Mode = PORT_UNKNOWN;
	else { MODULE_WARNING tty_writestring("Port1 failed test\n"); }
	if(port2Mode){
		safe_write(PS2_REGISTERPORT, TESTPORT2);
		if(safe_read(PS2_DATAPORT) != PORTTESTPASS){
			port2Mode = PORT_DISABLED;
			MODULE_WARNING tty_writestring("Port2 failed test\n");
		}
	}
	if(!port1Mode && !port2Mode){
		//TODO: Both ports don't work, throw and return
		MODULE_ERROR tty_writestring("Both ports are disabled, PS/2 support inactive\n");
		ps2_init_done = true;
		return;
	}

	MODULE_INFO tty_writestring("Finished port initialization, re-enabling ports\n");
	safe_write(PS2_REGISTERPORT, ENABLEPORT1);
	safe_write(PS2_REGISTERPORT, ENABLEPORT2);

	safe_write(PS2_DATAPORT, PS2_RESET);

	MODULE_INFO tty_writestring("Disabling scanning on port 1\n");
	int success = 0;
	int retries = 10;
	while(success != 0xFA) {
		cleanInput();
		safe_write(PS2_DATAPORT, DISABLESCANNING);		
		success = safe_read(PS2_DATAPORT);
		retries--;
		if(!retries){
			MODULE_WARNING tty_writestring("Unable to disable scanning\n");
			break;
		}
	}
	cleanInput();

	MODULE_INFO tty_writestring("Asking port 1 for device identification\n");
	success = false;
	do{
		safe_write(PS2_DATAPORT,PS2_IDENTIFY);
		if(safe_read(PS2_DATAPORT) == 0xFA) success = true;
	}while(!success);


	uint8_t identity = safe_read(PS2_DATAPORT);
	if(identity == 0xAB){
		//TODO:Keyboard
		cleanInput();
		MODULE_INFO tty_writestring("Port 1 is a keyboard\n");

		MODULE_INFO tty_writestring("Slowing typematic on port 1\n");
		bool success = false;
		do{
			cleanInput();
			safe_write(PS2_DATAPORT, 0xF3); // Set typematic
			safe_write(PS2_DATAPORT, 0xFE); // Slowest mode 11111 11 0
			if(safe_read(PS2_DATAPORT) == 0xFA) success = true;
		}while(!success);
		port1Mode = PORT_KEYBOARD;

		
	}else if(identity == 0x00){
		port1Mode = PORT_MOUSE;
		MODULE_INFO tty_writestring("Port 1 is a mouse\n");
	}else if(identity == 0x03){
		port1Mode = PORT_MOUSE;
		MODULE_INFO tty_writestring("Port 1 is a mouse with scrollwhell\n");
	}else if(identity == 0x04){
		port1Mode = PORT_MOUSE;
		MODULE_INFO tty_writestring("Port 1 is a 5 button mouse\n");
	}else{
		port1Mode = PORT_UNKNOWN;
		MODULE_WARNING tty_writef("Port 1 is unknown device code: %#2x", identity);
		while(canRead()) tty_writef("%#2x ", inportb(PS2_DATAPORT));
		tty_writechar('\n');
	}
	
	MODULE_INFO tty_writestring("Done, re-enabling scanning on port 1\n");
	do{
		safe_write(PS2_DATAPORT, ENABLESCANNING);
		if(safe_read(PS2_DATAPORT) == 0xFA) success = true;
	}while(!success);
}

static uint8_t fKeys[12] = { 0x5, 0x6, 0x4, 0xC, 0x3, 0xB, 0x83, 0x0A, 0x1, 0x9, 0x78, 0x7 };
static uint8_t alphabetic[26] = {0x1C,0x32,0x21,0x23,0x24,0x2B,0x34,0x33,0x43,0x3B,0x42,0x4B,0x3A,0x31,0x44,0x4D,0x15,0x2D,0x1B,0x2C,0x3C,0x2A,0x1D,0x22,0x35,0x1A};
static uint8_t numeric[10] = {0x45,0x16,0x1E,0x26,0x25,0x2E,0x36,0x3D,0x3E,0x46};

int capital = 0; //32 if capital as 'a'-'A'== 32
char keyboard_read() {
	for(int i = 0; i < 10000; i++) if(canRead()) break;
	if(!canRead()) return 0;
	uint8_t keyCode = inportb(0x60);
	if(keyCode == 0xF0) inportb(0x60);
	if(keyCode == 0xE0)if(inportb(0x60) == 0xF0)inportb(0x60);
	for(uint8_t i = 0; i < 26; ++i){
		if(alphabetic[i] == keyCode) return 'a' + i - capital;
	}
	for(uint8_t i = 0; i < 10; ++i){
		if(numeric[i] == keyCode) return '0' + i;
	}
	if(keyCode==0x29) return ' ';
	if(keyCode==0x66) return '\b';
	if(keyCode==0x0D) return '\t';
	if(keyCode==0x5A) return '\n';
	if(keyCode==0x58) { capital = capital>0? 0 : 32; return 0; }
	if(keyCode==fKeys[0]) tty_cursorcolor(0x0F); //White on black
	if(keyCode==fKeys[1]) tty_cursorcolor(0x02); //Green on black
	if(keyCode==fKeys[2]) tty_cursorcolor(0x1E); //Green on black
	if(keyCode==fKeys[3]) tty_cursorcolor(0x02); //Green on black
	if(keyCode==fKeys[4]) tty_cursorcolor(0x02); //Green on black
	if(keyCode==fKeys[5]) tty_cursorcolor(0x02); //Green on black
	if(keyCode==fKeys[6]) tty_cursorcolor(0x02); //Green on black
	if(keyCode==fKeys[7]) tty_cursorcolor(0x02); //Green on black
	if(keyCode==fKeys[8]) tty_cursorcolor(0x02); //Green on black
	if(keyCode==fKeys[9]) tty_cursorcolor(0x02); //Green on black
	if(keyCode==fKeys[10]) tty_cursorcolor(0x02); //Green on black
	if(keyCode==fKeys[11]) tty_cursorcolor(0x02); //Green on black
	if(keyCode==0x76) tty_clear(); //ESC == clear

	return 0;
}


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
