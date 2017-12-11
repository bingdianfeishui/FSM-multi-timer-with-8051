/**
 **********************************************************
 ******    Copyright(C), 2010-2016, NULL Co.,Ltd     ******
 **********************************************************

 *@Tittle		:	状态机按键测试1
 *@Version		:	v1.0
 *@Author		:	Liy
 *@Dat			:	2016-08-07 21:54:22
 *@Desctription	:	[desctription]
 *@History		:


 **********************************************************
 **********************************************************
 */

#include "lesson10_1.h"

DS1302RTC rtcTime;

sbit K0 = P3 ^ 0;

void InitIO();
void InitSys();
void InitClock();

void main()
{



	InitIO();
	InitSys();


	while (1)
	{

		DS1302GetRTC(&rtcTime);	//突发读取
		segBuff[0] = DIG_SEG[ (rtcTime.RTC.Minitue) % 10] | ((rtcTime.RTC.Second % 2 == 0) ? DIG_SEG[SEG_DOT] : 0x00);
		segBuff[1] = DIG_SEG[ (rtcTime.RTC.Minitue) / 10] | ((rtcTime.RTC.Second % 2 == 0) ? DIG_SEG[SEG_DOT] : 0x00);
		segBuff[2] = DIG_SEG[rtcTime.RTC.Hour % 10] ;
		segBuff[3] = DIG_SEG[rtcTime.RTC.Hour / 10] ;

		Display4DigSeg();

		DelayX10us(50);
	}
}


void InitIO()
{

}

void InitSys()
{
	InitDigSeg(0x00);
	InitClock();
}

void InitClock()
{
	UINT8 sec;
	DS1302RTC now = {10, 28, 19, 10, 8, 3, 16};

	// DS1302TrickleChargeController(1, 1, 2);//打开涓流充电，2个二极管，8Kohm电阻
	sec = DS1302SingleRead(0);
	if (sec & 0x80)	//初始化时间，调试时取消这句
	{
		DS1302SetRTC(&now);	//突发方式写入
	}
}
