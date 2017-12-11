#ifndef __MAIN_H__
#define __MAIN_H__

#include <intrins.h>
#include <common.h>
#include <DS1302.h>
#include <Delay.h>
#include <STC_eeprom.h>
#include "Display595.h"
#include "key.h"
#include "ISP.h"

//系统参数
#ifndef FOSC
#define FOSC 11059200 			//晶振频率，Hz
#endif
#ifndef TMCU
#define TMCU 6					//机器周期模式，12T、6T、1T
#endif

#define TIMESPAN (FOSC/TMCU/100-10) 	//定时器定时n个机器周期。12MHz/12T模式下，为10000个机器周期，即10ms
#define TIMERCOUNTER 20 		//定时器中断重复次数。12MHz/12T模式下20次*10ms，共200ms。count为unsigned char，此值不得超过255


#define MAX_ALARM_NUM 	8 		//闹钟设定最大数量
#define RELAY_ON 0 				//继电器动作时的电平
#define KEY_FREE_TIME	1500 	//无按键空闲时间1500*10ms, 15s
#define FLASH_LEVEL 100 		//设置界面闪烁速度控制,12T模式时建议50
#define ISP_EEPROM_ADDR 0x2000L	//ISP起始扇区
#define ISP_DATA_HEAD  0xAA		//EEPROM数据区头字节校验
/**
 * 共阴极数码管段码
 * 共阳极数码管使用时需取反
 */
UINT8 code DIG_SEG[] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07,
                        //0  ,   1 ,   2 ,   3 ,  4  ,  5  ,  6  ,  7  ,
                        0x7F, 0x6F, 0x77, 0x7C, 0x39, 0x5E, 0x79, 0x71,
                        //8  ,   9 ,  A  ,   B ,  C  ,  D  ,  E  ,  F ,
                        0x80 , 0x00, 0x40
                        //小数点, 空, 短横线
                       };
UINT8 code DIG_POS[] = {0x01, 0x02, 0x04, 0x08};
UINT8 segBuff[SEG_NUM];


struct ShortTime
{
	UINT8 Hour;
	UINT8 Minute;
};

typedef struct
{
	struct ShortTime Begin;
	struct ShortTime End;
} AlarmTime;

union alarmDict_u
{
	AlarmTime AlarmTimeSetting[MAX_ALARM_NUM];
	struct ShortTime ShortTimeArray[MAX_ALARM_NUM * 2];

} /*xdata 实机运行时取消注释*/ alarmDict = {
	0, 10, 0, 11,
	0, 8, 0, 9,
	0, 10, 0, 13,
	0, 0, 0, 0,
	0, 0, 0, 0,
    0, 0, 0, 0,
	0, 0, 0, 0,
    0, 0, 0, 0
};


typedef enum {
	fsmTimeDisplay	= 0x00,	//默认状态，显示时间
	fsmDateDisplay,
	fsmYearDisplay,
	fsmAlarmTimeDisplay,

	fsmMinuteSetting,
	fsmHourSetting,
	fsmDaySetting,
	fsmMonthSetting,
	fsmWeekSetting,
	fsmYearSetting,

	fsmAlarmMinuteSetting,
	fsmAlarmHourSetting,

} AppState;

sbit GPIO_RELAY = P3 ^ 7;
#define ALARM_IO  P1
#endif
