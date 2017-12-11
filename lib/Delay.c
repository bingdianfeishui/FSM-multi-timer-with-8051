/**
 **********************************************************
 ******    Copyright(C), 2010-2016, NULL Co.,Ltd     ******
 **********************************************************

 *@Tittle		:	通用延时函数
 *@Version		:	v1.2
 *@Author		:	Liy
 *@Dat			:	2016-09-03 01:52:20
 *@Desctription	:	延时函数库
 *@History		:
 *	#v1.2 	2016-09-03 01:52:36
 *		1. 增加STC12 1T单片机12MHz晶振下的延时函数,未精确计算
 *	#v1.1 	2016-08-10 15:03:41
 *		1. 删除固定延时函数，仅保留X倍延时；
 *		2. 优化X倍延时函数的时间，所有X倍延时函数误差控制为5us。
 *	#v1.0	2016-08-03 16:44:18
 *		1. 完成12MHz、12T模式下常用延时函数

 **********************************************************
 **********************************************************
 */


#include "Delay.h"

/**
 * 晶振12MHz,12T模式下延时
 */
#ifdef FSOC_12M_MOD_12T
/**
 * 晶振12MHz,12T模式下延时10*X us,固定误差5us。
 * X最大值255
 */
void DelayX10us(UINT8 X)
{
	UINT8 i;
	do
	{
		_nop_();
		i = 3;
		while (--i);
	} while (--X);
}

/**
 * 晶振12MHz,12T模式下延时1*X ms,固定误差5us。
 * X最大值255
 */
void DelayX1ms(UINT8 X)
{
	UINT8 i, j;
	do
	{
		i = 2;
		j = 240;
		do
		{
			while (--j);
		} while (--i);
	} while (--X);
}

/**
 * 晶振12MHz,12T模式下延时10*X ms,固定误差5us。
 * 周期数N=((2+(114*2+2)+(256*2+2)*19+2))*X+5
 * X最大值255
 */
void DelayX10ms(UINT8 X)
{
	UINT8 i, j;
	do
	{
		i = 20;
		j = 114;
		do
		{
			while (--j);
		} while (--i);
	} while (--X);
}

/**
 * 晶振12MHz,12T模式下延时10*X s,固定误差5us。
 * 周期数N=(1+3+(((123*2+2)*1+(256*2+2)*153) +2 ) + ((256*2+2)*256+2)*7+2)*X+5
 * X最大值255
 */
void DelayX1s(UINT8 X)
{
	UINT8 i, j, k;

	do
	{
		NOP();
		i = 8;
		j = 154;
		k = 123;
		do
		{
			do
			{
				while (--k);
			} while (--j);
		} while (--i);
	} while (--X);
}

#endif


/**
 * 晶振12MHz,STC12 1T模式下延时
 */
#ifdef FSOC_12M_MOD_1T_STC12

void Delay1us()		//@12.000MHz
{
	NOP(); NOP();
}

void Delay2us()		//@12.000MHz
{
	UINT8 i;

	i = 3;
	while (--i);
}

void Delay5us()		//@12.000MHz
{
	UINT8 i;

	i = 12;
	while (--i);
}

/**
 * STC12单片机,晶振12MHz,1T模式下延时10*X us
 * X最大值255
 */
void DelayX10us(UINT8 X)
{
	UINT8 i;
	do
	{
		i = 29;
		while (--i);
	} while (--X);
}

/**
 * STC12单片机,晶振12MHz,1T模式下延时1*X ms
 * X最大值255
 */
void DelayX1ms(UINT8 X)
{
	UINT8 i, j;
	do
	{
		NOP();
		NOP();
		i = 12;
		j = 178;
		do
		{
			while (--j);
		} while (--i);
	} while (--X);
}

/**
 * STC12单片机,晶振12MHz,1T模式下延时10*X ms
 * 周期数N=((2+(114*2+2)+(256*2+2)*19+2))*X+5
 * X最大值255
 */
void DelayX10ms(UINT8 X)
{
	UINT8 i, j;
	do
	{
		NOP();
		NOP();
		i = 117;
		j = 193;
		do
		{
			while (--j);
		} while (--i);
	} while (--X);
}

/**
 * STC12单片机,晶振12MHz,1T模式下延时10*X s
 * 周期数N=(1+3+(((123*2+2)*1+(256*2+2)*153) +2 ) + ((256*2+2)*256+2)*7+2)*X+5
 * X最大值255
 */
void DelayX1s(UINT8 X)
{
	UINT8 i, j, k;

	do
	{
		i = 46;
		j = 153;
		k = 255;
		do
		{
			do
			{
				while (--k);
			} while (--j);
		} while (--i);
	} while (--X);
}

#endif
