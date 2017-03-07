#include <kernel/cmos.h>
#include <kernel/tty.h>

#define RTC_REG_SECOND 	0x00
#define RTC_REG_MINUTE 	0x02
#define RTC_REG_HOUR 	0x04
#define RTC_REG_DAY 	0x07
#define RTC_REG_MONTH 	0x08
#define RTC_REG_YEAR 	0x09

//Information: http://wiki.osdev.org/CMOS

bool cmos_init_done = false;
bool BCDmode;
bool AMPMmode;
struct time last_time;

struct time get_rtc_unsafe(){
	struct time ret;
	ret.second = get_rtc_register(RTC_REG_SECOND);
	ret.minute = get_rtc_register(RTC_REG_MINUTE);
	ret.hour = get_rtc_register(RTC_REG_HOUR);
	ret.day = get_rtc_register(RTC_REG_DAY);
	ret.month = get_rtc_register(RTC_REG_MONTH);
	ret.year = get_rtc_register(RTC_REG_YEAR);
	return ret;
}

struct time get_rtc(){
	struct time orig = get_rtc_unsafe();
	struct time correct;
	//Wait until we get the same value twice in a row while the CMOS is not updating
	do{ 
		orig = correct;
		correct = get_rtc_unsafe(); 
	}while(
			rtc_isupdating() ||
			correct.second  != orig.second ||
			correct.minute  != orig.minute ||
			correct.hour    != orig.hour ||
			correct.day     != orig.day ||
			correct.month   != orig.month ||
			correct.year    != orig.year
		);
	if(BCDmode){
		correct.second = (correct.second & 0x0F) + ((correct.second / 16) * 10);
		correct.minute = (correct.minute & 0x0F) + ((correct.minute / 16) * 10);
		correct.hour = ( (correct.hour & 0x0F) + (((correct.hour & 0x70) / 16) * 10) ) | (correct.hour & 0x80);
		correct.day = (correct.day & 0x0F) + ((correct.day / 16) * 10);
		correct.month = (correct.month & 0x0F) + ((correct.month / 16) * 10);
		correct.year = (correct.year & 0x0F) + ((correct.year / 16) * 10);
	}
	if (AMPMmode && (correct.hour & 0x80)) {
		correct.hour = ((correct.hour & 0x7F) + 12) % 24;
	}
	//TODO: Consult for position of century register inside init
	correct.century=20;
	last_time = correct;
	return correct;
}

void update_rtc(){
	get_rtc();
}

void waitSecond(uint32_t seconds){
	uint8_t start = get_rtc_register(RTC_REG_SECOND);
	while(seconds){
		asm volatile("hlt");
		uint8_t current = get_rtc_register(RTC_REG_SECOND);
		if(current != start){
			--seconds;
			start = current;
		}
	}
}

void cmos_init(){
	uint8_t flagsB = get_rtc_register(RTC_REG_STATUS_B);
	AMPMmode = !(flagsB & 0x2);
	BCDmode = !(flagsB & 0x4);
	//Select register B
	outportb(CMOS_ADDRESS_REG, RTC_REG_STATUS_B);
	//Write back flags, with IRQ enabled
	outportb(CMOS_DATA_REG, flagsB | 0x40);
	//
	uint8_t rate = 0x0F;			// rate must be above 2 and not over 15
	outportb(0x70, 0x8A);		// set index to register A, disable NMI
	char prev=inportb(0x71);	// get initial value of register A
	outportb(0x70, 0x8A);		// reset index to A
	outportb(0x71, (prev & 0xF0) | rate); //write only our rate to A. Note, rate is the bottom 4 bits.
}
