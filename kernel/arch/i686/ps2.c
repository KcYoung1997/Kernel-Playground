#include <kernel/portb.h>
#include <kernel/ps2.h>

#include <stdbool.h>

bool initialised;
bool port1Enabled;
bool port2Enabled;

inline bool outfull(){
	return inportb(KBD_REGISTERPORT) & (1<<1);
}

inline bool infull(){
	return inportb(KBD_REGISTERPORT) & 1;
}

inline void cleanin(){
	if(infull()){
		terminal_writestring("PS2: Clearing garbage: \n"); 
		do { terminal_putchar(inportb(0x60)); } while(infull());
		terminal_putchar('\n');
	}
}
void ps2_initialize(){
	// Information: http://wiki.osdev.org/"8042"_PS/2_Controller
	// TODO: Check if PS/2 ports exist
	// TODO: Timeouts on all outfull waits
	// Temporarily disable both PS/2 ports - waiting until ready to accept input
	while(outfull()) continue;
	outportb(KBD_REGISTERPORT, DISABLEPORT1);
	outportb(KBD_REGISTERPORT, DISABLEPORT2);
	terminal_writestring("PS2: Disabled PS/2 ports for diagnostics\n");
	//Flush output buffer by checking if there is any input then taking it without storing
	cleanin();
	// Request then read Controller configuration byte
	{
		while(outfull()) continue;
		outportb(KBD_REGISTERPORT, READCONFIG);
		uint8_t config = inportb(KDB_DATAPORT);
		terminal_writestring("PS2: Current config: ");
		for(size_t i = 0; i < 8; ++i) terminal_putchar('0');
		terminal_putchar('\n');
		// Get if two PS/2 devices exist (bit 5)
		port2Enabled = config & (1<<5);
		if(port2Enabled) terminal_writestring("PS2: Port2 enabled in config\n");
		else terminal_writestring("PS2: Port2 disabled in config\n");
		// Set interrupts off for both devices
		config &= ~(1 << 0);
		config &= ~(1 << 1);
		// Set translation off
		config &= ~(1 << 6);
		terminal_writestring("PS2: New config: ");
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
			terminal_writestring("PS2: Self-test failed\n");
			initialised = true;
			port1Enabled = false;
			port2Enabled = false;
			return;
		}else if(test != SELFTESTPASS){
			terminal_writestring("PS2: Self-test returned undefined response\n");
			//TODO: Unknown error has happened - Maybe buffers should be cleared before this
			initialised = true;
			port1Enabled = false;
			port2Enabled = false;
			return;
		}
		terminal_writestring("PS2: Self-test passed\n");
	}
	// Test for dual channel properly
	if(port2Enabled)
	{
		outportb(KBD_REGISTERPORT, ENABLEPORT2);
		port2Enabled = !(inportb(KDB_DATAPORT) & (1<<5));
		if(port2Enabled){
			terminal_writestring("PS2: Port2 enabled definitely\n");
			outportb(KBD_REGISTERPORT, DISABLEPORT2);
		}
	}
	// Test if ports work
	// TODO: get specifics about port failures
	outportb(KBD_REGISTERPORT, TESTPORT1);
	if(inportb(KDB_DATAPORT) == PORTTESTPASS) port1Enabled = true;
	else terminal_writestring("PS2: Port1 failed test\n");
	if(port2Enabled){
		outportb(KBD_REGISTERPORT, TESTPORT1);
		if(inportb(KDB_DATAPORT) != PORTTESTPASS){
			 port2Enabled = false;
			terminal_writestring("PS2: Port2 failed test\n");
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
	terminal_writestring("PS2: Finished initialization\n");
	// TODO: re-enable interrupts once implemented
	// TODO: Device resets when accessed
}
