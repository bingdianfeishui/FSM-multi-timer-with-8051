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
#include "key.h"

DS1302RTC rtcTime;
UINT8 th, tl;
UINT8 datBuff[SEG_NUM] = {0, 1, 0, 2};

void InitIO();
void InitSys();
void InitClock();
void InitTimer0();
void UpdateScreen();
void StateControl(KeyMessage key);
void KeyPressed(KeyMessage key);
void KeyLongPressed(KeyMessage key);

sbit P20 = P2 ^ 0;
sbit P10 = P1 ^ 0;
void main()
{
	InitIO();
	InitSys();

	while (1)
	{
		UpdateScreen();

		DelayX10us(50);
	}
}


void InitIO()
{

}

void InitSys()
{
	InitDigSeg(DIG_SEG[0]);
	InitClock();

	InitTimer0();
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

void InitTimer0()
{
	th = (65536 - TIMESPAN) / 256;	//计算定时器初值
	tl = (65536 - TIMESPAN) % 256;

//	AUXR &= 0x7F;		//定时器时钟12T模式
	EA = 1;
	ET0 = 1;
	TMOD &= 0xF0;		//清空定时器0的工作模式参数
	TMOD |= 0x01;		//设置定时器0的工作模式为模式1,16位定时器
	TH0 = th;		//设置定时高8位初值
	TL0 = tl;		//设置定时低8位初值
	TF0 = 0;		//清除TF0标志
	TR0 = 1;		//定时器0开始计时
}

void InterruptTimer0() interrupt 1
{
	static UINT8 counter = 0;
	UINT8 keyValue;
	static KeyMessage keyMsg = msgKeyNone;

	TH0 = th;		//设置定时高8位初值
	TL0 = tl;		//设置定时低8位初值

	counter++;
	if (counter == TIMERCOUNTER)	//200ms
	{
		counter = 0;
		//KeyPressed(msgKey1);
	}

	keyMsg = ReadKey(&keyValue);
	StateControl(keyMsg);
}

void UpdateScreen()
{
	UINT8 i;
	for (i = 0; i < SEG_NUM; i++)
	{
		segBuff[i] = DIG_SEG[datBuff[i]];
	}

	Display4DigSeg();
}

void StateControl(KeyMessage key)
{
	static fsmKeyState curState = fsmKeyIdle;
	static KeyMessage tmpKey = msgKeyNone;
	static UINT16 keyTimer = 0;

	switch (curState)
	{
	case fsmKeyIdle :
		if (key != msgKeyNone)
			curState = fsmKeyDown;
		break;
	case fsmKeyDown :

		if (key == msgKeyNone)	//干扰
		{
			curState = fsmKeyIdle;
		}
		else
		{
			curState = fsmKeyTime;
			keyTimer = 0;
			tmpKey = key;
		}
		break;
	case fsmKeyTime :
		if (key == msgKeyNone )	//按键弹起，普通按键
		{
			curState = fsmKeyIdle;
			KeyPressed(tmpKey);
		}
		else if (key != tmpKey)	//键值已变，重置为按下状态
		{
			curState = fsmKeyDown;
		}
		else	//未弹起，开始计时
		{
			keyTimer++;
			if (keyTimer == KEY_LONG_TIME)
			{
				keyTimer = 0;
				curState = fsmKeyLongPress;
			}
		}

		break;
	case fsmKeyLongPress :
		if (key == msgKeyNone)
		{
			curState = fsmKeyIdle;
		}
		else if (key != tmpKey)
		{
			curState = fsmKeyDown;
		}
		else
		{
			keyTimer++;
			if (keyTimer == KEY_CONTINUE_ACT)	//0.2s
			{
				keyTimer = 0;
				KeyLongPressed(tmpKey);
			}
		}
		break;
	default:
		break;
	}
}

void KeyPressed(KeyMessage key)
{

	if (key == msgKey1)
	{
		datBuff[0]++;
		if (datBuff[0] == 10)
		{
			datBuff[0] = 0;
			datBuff[1]++;
			if (datBuff[1] == 10)
			{
				datBuff[1] = 0;
				datBuff[2]++;
				if (datBuff[2] == 10)
				{
					datBuff[2] = 0;
					datBuff[3]++;
					if (datBuff[3] == 10)
					{
						datBuff[0] = 0;
						datBuff[1] = 0;
						datBuff[2] = 0;
						datBuff[3] = 0;
					}
				}
			}
		}
	}
	else if (key == msgKey2)
	{
		datBuff[2]++;
		if (datBuff[2] == 10)
		{
			datBuff[2] = 0;
			datBuff[3]++;
			if (datBuff[3] == 10)
			{
				datBuff[0] = 0;
				datBuff[1] = 0;
				datBuff[2] = 0;
				datBuff[3] = 0;
			}
		}
	}
}

void KeyLongPressed(KeyMessage key)
{
	KeyPressed(key);
}
