#ifndef _KERNEL_CMOS_H
#define _KERNEL_CMOS_H

#include <stdint.h>
#include <stdbool.h>

#include <kernel/portb.h>

#define CMOS_ADDRESS_REG 	0x70
#define CMOS_DATA_REG 		0x71

#define RTC_REG_STATUS_A	0x0A
#define RTC_REG_STATUS_B	0x0B

struct time{
	uint8_t second;
	uint8_t minute;
	uint8_t hour;
	uint8_t day;
	uint8_t month;
	uint8_t year;
	uint8_t century;
};

extern bool cmos_init_done;
extern bool BCDmode;
extern bool AMPMmode;
extern struct time last_time;

inline uint8_t get_rtc_register(uint8_t reg){
      outportb(CMOS_ADDRESS_REG, reg);
      return inportb(CMOS_DATA_REG);
}
inline bool rtc_isupdating(){
      outportb(CMOS_ADDRESS_REG, RTC_REG_STATUS_A);
      return inportb(CMOS_DATA_REG) & 0x80;
}
struct time get_rtc();
void update_rtc();
void cmos_init();
void waitSecond(uint32_t seconds);

#endif
