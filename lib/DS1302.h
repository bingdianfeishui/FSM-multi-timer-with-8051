#ifndef __DS1302_H__
#define __DS1302_H__

#include "common.h"
#include "Delay.h"

#define DS1302_RTC_LENGTH 7

typedef union
{
	struct {
		UINT8 Second;
		UINT8 Minute;
		UINT8 Hour;
		UINT8 Day;
		UINT8 Month;
		UINT8 Week;
		UINT8 Year;
	} RTC;
	UINT8 RTCArray[DS1302_RTC_LENGTH];
} DS1302RTC;

extern UINT8 DS1302SingleRead(UINT8 regNum);
extern void DS1302SingleWrite(UINT8 regNum, UINT8 dat);
extern void DS1302GetRTC(DS1302RTC *rtc);
extern void DS1302SetRTC(DS1302RTC *rtc);
extern void DS1302SetRTCArray(UINT8 *rtcArray);
extern void DS1302GetRTCArray(UINT8 *rtcArray);


extern UINT8 DS1302RAMSingleRead(UINT8 ramNum);
extern void DS1302RAMSingleWrite(UINT8 ramNum, UINT8 dat);
extern void DS1302RAMBurstRead(UINT8* buff);
extern void DS1302RAMBurstWrite(UINT8* buff);


extern void DS1302WriteEnable(BOOL enable);
extern void DS1302TrickleChargeController(BOOL enable, BOOL oneDiode, UINT8 resistor);
extern BOOL  DS1302IsHalt();
extern void DS1302ClockHalt(BOOL enable);
#endif
