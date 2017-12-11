#ifndef __MAIN_H__
#define __MAIN_H__

#include <reg52.h>
#include <intrins.h>
#include <common.h>
#include <DS1302.h>
#include <Delay.h>
#include "Display.h"


//系统参数
#define FOSC 12000000 			//晶振频率，Hz
#define TSOC 12					//机器周期模式，12T、6T、1T
#define TIMESPAN (FOSC/TSOC/100-10) 	//定时器定时n个机器周期。12MHz/12T模式下，为10000个机器周期，即10ms
#define TIMERCOUNTER 100 		//定时器中断重复次数。12MHz/12T模式下20次*10ms，共200ms。count为unsigned char，此值不得超过255


/**
 * 共阴极数码管段码
 * 共阳极数码管使用时需取反
 */
UINT8 code DIG_SEG[] = {  0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07,
                          //0  ,   1 ,   2 ,   3 ,  4  ,  5  ,  6  ,  7  ,
                          0x7F, 0x6F, 0x77, 0x7C, 0x39, 0x5E, 0x79, 0x71,
                          //8  ,   9 ,  A  ,   B ,  C  ,  D  ,  E  ,  F ,
                          0x80 , 0x00, 0x40
                          //DOT , NULL, LINE
                       };
UINT8 code DIG_POS[8] = {0x01, 0x02, 0x04, 0x08, 0x01, 0x02, 0x04, 0x08}; //{0x80, 0x40, 0x20, 0x10, 0x80, 0x40, 0x20, 0x10};

UINT8 segBuff[SEG_NUM];


#endif
