/**
 **********************************************************
 ******    Copyright(C), 2010-2016, NULL Co.,Ltd     ******
 **********************************************************

 *@Tittle		:	基于状态机的双按键多功能闹钟
 *@Version		:	v1.1
 *@Author		:	Liy
 *@Dat			:	2017-12-12 00:46:10
 *@Desctription	:	最多可设8组闹钟的时钟，可设置时、分、日期
 *					、年份。每组闹钟为在给定区间内动作相应IO口。
 *@History		:
 	#v1.0	2016-08-14 04:47:33
 		1. 时钟功能

 **********************************************************
 **********************************************************
 */


#include "main.h"

DS1302RTC rtcTime = { 0, 0, 12, 13, 8, 6, 16};

AppState curAppState = fsmTimeDisplay;

BOOL alarmInspect = FALSE;
BOOL alarmOn = FALSE;
UINT8 alarmAction = 0x00;
UINT16 fsmTimer = 0;
UINT8 alarmTimeNum = 0;
UINT8 th, tl;

struct ShortTime tmpTime = {0, 0};

void InitSys();
void InitAlarmTime();
void InitDs1302();
void InitIO();
void InitTimer0();
void GetTimeFromDS1302();
void UpdateScreen();
void AlarmWatchDog();
BOOL IsMinuteUpdate();
void AlarmActionController();
BOOL IsInAlarmTimeSpan(DS1302RTC *rtcTime);
void GenerateAlarmAction(DS1302RTC *rtcTime);
void KeyStateController(KeyMessage *keyMsg);
void AppStateController(KeyInfo *keyInfo);
void UpdateDisplayBuff();
void SaveAlarmTimeSetting();

void main()
{

	InitIO();
	InitSys();

	while (1)
	{
		if (curAppState == fsmTimeDisplay || curAppState == fsmDateDisplay
		        || curAppState == fsmYearDisplay)
		{
			AlarmWatchDog();
		}

		UpdateScreen();
	}
}

void InitSys()
{
	InitDigSeg(0x40);
	UARTInit();
	InitAlarmTime();
	InitDs1302();
	// Delayx10ms(50);
	GetTimeFromDS1302();
	UpdateScreen();


	alarmInspect = TRUE;

	th = (65536 - TIMESPAN) / 256;
	tl = (65536 - TIMESPAN) % 256;
	InitTimer0();
}

void InitAlarmTime()
{
	UINT8 tmp;
	tmp = ISPReadByte(ISP_EEPROM_ADDR);
	if (tmp != ISP_DATA_HEAD)	//不相等则说明该Flash扇区未写入有效闹钟数据，初始化该数据区
	{
		SaveAlarmTimeSetting();
	}
	ISPArrayRead(ISP_EEPROM_ADDR + 1, 4 * MAX_ALARM_NUM, (UINT8 *)alarmDict.ShortTimeArray);
    //ISPArrayRead(ISP_EEPROM_ADDR + 1, 4 * MAX_ALARM_NUM, (UINT8 *)dict.ShortTimeArray);
}

void InitIO()
{
    ALARM_IO = alarmAction;
	GPIO_RELAY = ~RELAY_ON;
}

void InitTimer0()
{
	EA = 1;
	ET0 = 1;
	TMOD &= 0xF0;		//清空定时器0的工作模式参数
	TMOD |= 0x01;		//设置定时器0的工作模式为模式1--16位定时器
	TH0 = th;		//设置定时高8位初值
	TL0 = tl;		//设置定时低8位初值
	TF0 = 0;		//清除TF0标志
	TR0 = 1;		//定时器0开始计时
}

void InitDs1302()
{
	UINT8 sec = DS1302SingleRead(0);
	if (sec & 0x80)	//判断是否在走时。未走时则初始化时间，调试时注释这句，以便用初始化值来设置时钟。
	{
		DS1302SetRTC(&rtcTime);	//突发方式写入
	}
}

void InterruptTimer0() interrupt 1
{
	static UINT8 counter;	//每10ms计数
	static UINT8 keyMsg;

	TH0 = th;		//设置定时高8位初值
	TL0 = tl;		//设置定时低8位初值

	keyMsg = ReadKey();
	KeyStateController(&keyMsg);	//按键状态机控制器

	counter++;
	if (counter == TIMERCOUNTER)	//200ms
	{
		counter = 0;

		GetTimeFromDS1302();
	}

}

void GetTimeFromDS1302()
{
	// UINT8 i;
	if (curAppState == fsmTimeDisplay || curAppState == fsmDateDisplay
	        || curAppState == fsmYearDisplay)
	{
		//突发读取
		DS1302GetRTC(&rtcTime);

		// //逐字节读取
		// for (i = 0; i < DS1302_RTC_LENGTH; i++)
		// {
		// 	rtcTime.RTCArray[i] = DS1302SingleRead(i);
		// }

		if ((rtcTime.RTC.Hour != tmpTime.Hour) || (rtcTime.RTC.Minute != tmpTime.Minute))
		{
			alarmInspect = TRUE;
			tmpTime.Hour = rtcTime.RTC.Hour;
			tmpTime.Minute = rtcTime.RTC.Minute;
		}
	}

}

void UpdateScreen()
{
	UpdateDisplayBuff();
	Display4DigSeg(TRUE);
}

void AlarmWatchDog()
{
	if (alarmInspect)
	{
		alarmInspect = FALSE;
		//alarmOn = IsInAlarmTimeSpan(&rtcTime);
		GenerateAlarmAction(&rtcTime);
        AlarmActionController();
	}
}

void AlarmActionController()
{
    if(alarmOn)
        ALARM_IO = alarmAction;
	GPIO_RELAY = alarmOn ? RELAY_ON : ~RELAY_ON;
}

BOOL IsInAlarmTimeSpan(DS1302RTC *rtc)
{
	UINT8 i;
	UINT16 now, begin, end;
	for (i = 0; i < MAX_ALARM_NUM; i++)
	{
		if (rtc->RTC.Hour < alarmDict.AlarmTimeSetting[i].Begin.Hour
		        && rtc->RTC.Hour > alarmDict.AlarmTimeSetting[i].End.Hour)
			continue;

		now = rtc->RTC.Hour * 60 + rtc->RTC.Minute;
		begin = alarmDict.AlarmTimeSetting[i].Begin.Hour * 60 + alarmDict.AlarmTimeSetting[i].Begin.Minute;
		end = alarmDict.AlarmTimeSetting[i].End.Hour * 60 + alarmDict.AlarmTimeSetting[i].End.Minute;
		if (now >= begin && now < end)
			return 1;
	}
	return 0;
}

void GenerateAlarmAction(DS1302RTC *rtc){
    UINT8 i;
	UINT16 now, begin, end;
	for (i = 0; i < MAX_ALARM_NUM; i++)
	{
		if (rtc->RTC.Hour < alarmDict.AlarmTimeSetting[i].Begin.Hour
		        && rtc->RTC.Hour > alarmDict.AlarmTimeSetting[i].End.Hour)
			continue;

		now = rtc->RTC.Hour * 60 + rtc->RTC.Minute;
		begin = alarmDict.AlarmTimeSetting[i].Begin.Hour * 60 + alarmDict.AlarmTimeSetting[i].Begin.Minute;
		end = alarmDict.AlarmTimeSetting[i].End.Hour * 60 + alarmDict.AlarmTimeSetting[i].End.Minute;
		if (now >= begin && now < end)
			alarmAction |= 1 << i;
        else
            alarmAction &= ~(1 << i);
	}
}

//按键状态机控制器
void KeyStateController(KeyMessage *keyMsg)
{
	static fsmKeyState curKeyState = fsmKeyIdle;
	static KeyMessage tmpKey = msgKeyNone;

	static KeyInfo keyInfo;

	keyInfo.Key = msgKeyNone;	//每次都先复位
	keyInfo.Action = actNothing;

	switch (curKeyState)
	{
	case fsmKeyIdle :	//无按键
		if (*keyMsg != msgKeyNone)
			curKeyState = fsmKeyDown;
		break;
	case fsmKeyDown :	//按键按下
		if (*keyMsg == msgKeyNone)	//干扰
		{
			curKeyState = fsmKeyIdle;
		}
		else
		{
			curKeyState = fsmKeyTime;
			fsmTimer = 0;
			tmpKey = *keyMsg;
		}
		break;
	case fsmKeyTime :	//按键按下后计时
		if (*keyMsg == msgKeyNone )	//按键弹起，普通按键
		{
			curKeyState = fsmKeyIdle;
			keyInfo.Key = tmpKey;
			keyInfo.Action = actClick;
			// AppStateController(&keyInfo);
		}
		else if (*keyMsg != tmpKey)	//键值已变，重置为按下状态
		{
			curKeyState = fsmKeyDown;
		}
		else	//未弹起，开始计时
		{
			fsmTimer++;
			if (fsmTimer == KEY_LONG_TIME)
			{
				fsmTimer = 0;
				curKeyState = fsmKeyLongPress;
				keyInfo.Key = tmpKey;
				keyInfo.Action = actLongPress;
				// AppStateController(&keyInfo);
			}
		}

		break;
	case fsmKeyLongPress :	//计时达到长按阈值
		if (*keyMsg == msgKeyNone)
		{
			curKeyState = fsmKeyIdle;
		}
		else if (*keyMsg != tmpKey)
		{
			curKeyState = fsmKeyDown;
		}
		else
		{
			fsmTimer++;
			if (fsmTimer == KEY_CONTINUE_ACT)	//0.2s
			{
				fsmTimer = 0;
				keyInfo.Key = tmpKey;
				keyInfo.Action = actTick;	//连发状态每隔0.2秒认发送一次按键连击
			}
		}
		break;
	default:
		break;
	}

	AppStateController(&keyInfo); 	//程序状态机控制器
}

//程序状态机控制器
void AppStateController(KeyInfo *keyInfo)
{
	static UINT16 freeTime = 0;
	static BOOL changedFlag = FALSE;



	if (keyInfo->Key == msgKeyNone)	//无按键
	{
		//不在时间显示状态且无按键状态超过空闲时间
		if (curAppState != fsmTimeDisplay && ++freeTime >= KEY_FREE_TIME)
		{
			freeTime = 0;
			curAppState = fsmTimeDisplay;
			return;
		}
	}
	else	//有按键则清零
		freeTime = 0;


	switch (curAppState)
	{
	case (fsmTimeDisplay) :	//默认状态，时间显示状态
		if (keyInfo->Key == msgKeySet && keyInfo->Action == actClick)
			curAppState = fsmDateDisplay;
		else if (keyInfo->Key == msgKeyFunc && keyInfo->Action == actLongPress)
		{	curAppState = fsmMinuteSetting;
			changedFlag = FALSE;
		}
		break;
	case (fsmDateDisplay) :	//日期显示状态
		if (keyInfo->Key == msgKeySet )
		{
			if (keyInfo->Action == actClick)
				curAppState = fsmYearDisplay;
			else if (keyInfo->Action == actLongPress)	//长按设置键返回时间显示状态
				curAppState = fsmTimeDisplay;
		}
		else if (keyInfo->Key == msgKeyFunc && keyInfo->Action == actLongPress)
		{	curAppState = fsmDaySetting;
			changedFlag = FALSE;
		}
		break;
	case (fsmYearDisplay) :	//年份显示状态
		if (keyInfo->Key == msgKeySet )
		{
			if (keyInfo->Action == actClick)
			{
				curAppState = fsmAlarmTimeDisplay;
				alarmTimeNum = 0;		//闹钟序号归0
			}
			else if (keyInfo->Action == actLongPress)	//长按设置键返回时间显示状态
				curAppState = fsmTimeDisplay;
		}
		else if (keyInfo->Key == msgKeyFunc && keyInfo->Action == actLongPress)
		{	curAppState = fsmYearSetting;
			changedFlag = FALSE;
		}
		break;
	case (fsmAlarmTimeDisplay) :	//闹钟设置显示状态
		if (keyInfo->Key == msgKeySet )
		{
			if (keyInfo->Action == actClick)
			{
				if (++alarmTimeNum == 2 * MAX_ALARM_NUM)	//切换到下一个闹钟。
					//到达最后一个闹钟设置，重置为时间显示状态
				{	curAppState = fsmTimeDisplay;
					alarmTimeNum = 0;		//闹钟序号归0
				}
			}
			else if (keyInfo->Action == actLongPress)	//长按设置键返回时间显示状态
				curAppState = fsmTimeDisplay;
		}
		else if (keyInfo->Key == msgKeyFunc && keyInfo->Action == actLongPress)
		{	curAppState = fsmAlarmMinuteSetting;
			changedFlag = FALSE;
		}
		break;

	case (fsmMinuteSetting) :	//分钟设置
		if (keyInfo->Key == msgKeySet )	//不管短按还是连发，都累加。长按逻辑已在按键状态机中处理
		{
			changedFlag = TRUE;
			rtcTime.RTC.Minute ++;
			if (rtcTime.RTC.Minute >= 60)
				rtcTime.RTC.Minute = 0;
		}
		else if (keyInfo->Key == msgKeyFunc)	//短按切换，长按保存并切换回显示状态
		{
			if (keyInfo->Action == actClick)
				curAppState = fsmHourSetting;
			else if (keyInfo->Action == actLongPress)
			{
				curAppState = fsmTimeDisplay;	//切换到时间显示
				if (changedFlag)
				{
					DS1302SingleWrite(0, 0);	//重置到0秒
					DS1302SingleWrite(1, rtcTime.RTC.Minute);
					DS1302SingleWrite(2, rtcTime.RTC.Hour);
					changedFlag = FALSE;
					alarmInspect = TRUE;
				}
			}
		}
		break;
	case (fsmHourSetting) :	//小时设置
		if (keyInfo->Key == msgKeySet )	//不管短按还是连发，都累加
		{
			changedFlag = TRUE;
			rtcTime.RTC.Hour ++;
			if (rtcTime.RTC.Hour >= 24)
				rtcTime.RTC.Hour = 0;
		}
		else if (keyInfo->Key == msgKeyFunc)
		{
			if (keyInfo->Action == actLongPress)	//长按保存，并切换到时间显示
			{
				curAppState = fsmTimeDisplay;	//切换到时间显示
				if (changedFlag)
				{
					DS1302SingleWrite(0, 0);	//重置到0秒
					DS1302SingleWrite(1, rtcTime.RTC.Minute);
					DS1302SingleWrite(2, rtcTime.RTC.Hour);
					changedFlag = FALSE;
					alarmInspect = TRUE;
				}
			}
			else if (keyInfo->Action == actClick)	//短按回到分钟设置
				curAppState = fsmMinuteSetting;
		}
		break;
	case (fsmDaySetting) :	//日期的日设置
		if (keyInfo->Key == msgKeySet )
		{
			changedFlag = TRUE;
			rtcTime.RTC.Day ++;
			if (rtcTime.RTC.Day >= 31)
				rtcTime.RTC.Day = 1;
		}
		else if (keyInfo->Key == msgKeyFunc)	//短按切换，长按保存并切换回日期显示
		{
			if (keyInfo->Action == actClick)
				curAppState = fsmMonthSetting;
			else if (keyInfo->Action == actLongPress)
			{
				curAppState = fsmDateDisplay;	//切换回日期显示
				if (changedFlag)
				{	DS1302SingleWrite(3, rtcTime.RTC.Day);
					DS1302SingleWrite(4, rtcTime.RTC.Month);
					changedFlag = FALSE;
				}
			}
		}
		break;
	case (fsmMonthSetting) :	//日期的月设置
		if (keyInfo->Key == msgKeySet )	//不管短按还是连发，都累加
		{
			changedFlag = TRUE;
			rtcTime.RTC.Month ++;
			if (rtcTime.RTC.Month >= 13)
				rtcTime.RTC.Month = 1;
		}
		else if (keyInfo->Key == msgKeyFunc)
		{
			if (keyInfo->Action == actLongPress)	//长按保存，并切换回日期显示
			{
				curAppState = fsmDateDisplay;	//切换回日期显示
				if (changedFlag)
				{
					DS1302SingleWrite(3, rtcTime.RTC.Day);
					DS1302SingleWrite(4, rtcTime.RTC.Month);
					changedFlag = FALSE;
				}
			}
			else if (keyInfo->Action == actClick)	//短按回到日设置
				curAppState = fsmDaySetting;
		}
		break;
	case (fsmYearSetting) :	//年设置
		if (keyInfo->Key == msgKeySet )	//不管短按还是连发，都累加
		{
			changedFlag = TRUE;
			rtcTime.RTC.Year ++;
			if (rtcTime.RTC.Year >= 100)
				rtcTime.RTC.Year = 0;
		}
		else if (keyInfo->Key == msgKeyFunc)
		{
			if (keyInfo->Action == actLongPress)	//长按保存，并切换回年份显示
			{
				curAppState = fsmYearDisplay;	//切换回年份显示
				alarmTimeNum = 0;		//闹钟序号归0
				if (changedFlag)
				{	DS1302SingleWrite(6, rtcTime.RTC.Year);
					changedFlag = FALSE;
				}
			}
		}
		break;

	case (fsmAlarmMinuteSetting) :	//闹钟时间的分钟设置
		if (keyInfo->Key == msgKeySet )
		{
			changedFlag = TRUE;
			alarmDict.ShortTimeArray[alarmTimeNum].Minute ++;
			if (alarmDict.ShortTimeArray[alarmTimeNum].Minute >= 60)
				alarmDict.ShortTimeArray[alarmTimeNum].Minute = 0;
		}
		else if (keyInfo->Key == msgKeyFunc)	//短按切换，长按保存并切换回闹钟显示状态
		{
			if (keyInfo->Action == actClick)
				curAppState = fsmAlarmHourSetting;
			else if (keyInfo->Action == actLongPress)
			{
				curAppState = fsmAlarmTimeDisplay;

				SaveAlarmTimeSetting();
				changedFlag = FALSE;
				alarmInspect = TRUE;
			}
		}
		break;
	case (fsmAlarmHourSetting) :	//闹钟时间的小时设置
		if (keyInfo->Key == msgKeySet )
		{
			changedFlag = TRUE;
			alarmDict.ShortTimeArray[alarmTimeNum].Hour ++;
			if (alarmDict.ShortTimeArray[alarmTimeNum].Hour >= 24)
				alarmDict.ShortTimeArray[alarmTimeNum].Hour = 0;
		}
		else if (keyInfo->Key == msgKeyFunc)	//短按切换，长按保存并切换回闹钟显示状态
		{
			if (keyInfo->Action == actClick)
				curAppState = fsmAlarmMinuteSetting;
			else if (keyInfo->Action == actLongPress)
			{
				curAppState = fsmAlarmTimeDisplay;

				SaveAlarmTimeSetting();
				changedFlag = FALSE;
				alarmInspect = TRUE;
			}
		}
		break;
	default:
		break;
	}
}

void UpdateDisplayBuff()
{
	static UINT16 flashCounter = 0;
	static UINT8 tmp = 0;
	static BOOL flashFlag = TRUE;

	switch (curAppState)
	{
	case (fsmTimeDisplay) :
		segBuff[0] = DIG_SEG[rtcTime.RTC.Minute % 10] | (rtcTime.RTC.Second % 2 ? DIG_SEG[SEG_DOT] : DIG_SEG[SEG_NULL]);
		segBuff[1] = DIG_SEG[rtcTime.RTC.Minute / 10] | (rtcTime.RTC.Second % 2 ? DIG_SEG[SEG_DOT] : DIG_SEG[SEG_NULL]);
		segBuff[2] = DIG_SEG[rtcTime.RTC.Hour % 10];
		tmp = rtcTime.RTC.Hour / 10;
		segBuff[3] = DIG_SEG[tmp == 0 ? SEG_NULL : tmp];
		break;
	case (fsmDateDisplay) :
		segBuff[0] = DIG_SEG[rtcTime.RTC.Day % 10] | DIG_SEG[SEG_DOT];
		segBuff[1] = DIG_SEG[rtcTime.RTC.Day / 10] ;
		segBuff[2] = DIG_SEG[rtcTime.RTC.Month % 10];
		tmp = rtcTime.RTC.Month / 10;
		segBuff[3] = DIG_SEG[tmp == 0 ? SEG_NULL : tmp];
		break;
	case (fsmYearDisplay) :
		segBuff[0] = DIG_SEG[rtcTime.RTC.Year % 10];
		segBuff[1] = DIG_SEG[rtcTime.RTC.Year / 10] ;
		segBuff[2] = DIG_SEG[0];
		segBuff[3] = DIG_SEG[2];
		break;
	case (fsmAlarmTimeDisplay) :
		segBuff[0] = DIG_SEG[alarmDict.ShortTimeArray[alarmTimeNum].Minute % 10] | (alarmTimeNum % 2 ? DIG_SEG[SEG_DOT] : DIG_SEG[SEG_NULL]);
		segBuff[1] = DIG_SEG[alarmDict.ShortTimeArray[alarmTimeNum].Minute / 10] | (alarmTimeNum % 2 ? DIG_SEG[SEG_NULL] : DIG_SEG[SEG_DOT]);
		segBuff[2] = DIG_SEG[alarmDict.ShortTimeArray[alarmTimeNum].Hour % 10];
		tmp = alarmDict.ShortTimeArray[alarmTimeNum].Hour / 10;
		segBuff[3] = DIG_SEG[tmp == 0 ? SEG_NULL : tmp];
		break;

	case (fsmMinuteSetting):
		segBuff[0] = flashFlag ? DIG_SEG[SEG_NULL] : (DIG_SEG[rtcTime.RTC.Minute % 10] | DIG_SEG[SEG_DOT]);
		segBuff[1] = flashFlag ? DIG_SEG[SEG_NULL] : (DIG_SEG[rtcTime.RTC.Minute / 10] | DIG_SEG[SEG_DOT]);
		segBuff[2] = DIG_SEG[rtcTime.RTC.Hour % 10];
		tmp = rtcTime.RTC.Hour / 10;
		segBuff[3] = DIG_SEG[tmp == 0 ? SEG_NULL : tmp];
		break;
	case (fsmHourSetting):
		segBuff[0] = DIG_SEG[rtcTime.RTC.Minute % 10] | DIG_SEG[SEG_DOT];
		segBuff[1] = DIG_SEG[rtcTime.RTC.Minute / 10] | DIG_SEG[SEG_DOT];
		segBuff[2] = flashFlag ? DIG_SEG[SEG_NULL] : DIG_SEG[rtcTime.RTC.Hour % 10];
		tmp = rtcTime.RTC.Hour / 10;
		segBuff[3] = flashFlag ? DIG_SEG[SEG_NULL] : DIG_SEG[tmp == 0 ? SEG_NULL : tmp];
		break;

	case (fsmDaySetting):
		segBuff[0] = flashFlag ? DIG_SEG[SEG_NULL] : (DIG_SEG[rtcTime.RTC.Day % 10] | DIG_SEG[SEG_DOT]);
		segBuff[1] = flashFlag ? DIG_SEG[SEG_NULL] : DIG_SEG[rtcTime.RTC.Day / 10] ;
		segBuff[2] = DIG_SEG[rtcTime.RTC.Month % 10];
		tmp = rtcTime.RTC.Month / 10;
		segBuff[3] = DIG_SEG[tmp == 0 ? SEG_NULL : tmp];
		break;
	case (fsmMonthSetting):
		segBuff[0] = DIG_SEG[rtcTime.RTC.Day % 10] | DIG_SEG[SEG_DOT];
		segBuff[1] = DIG_SEG[rtcTime.RTC.Day / 10] ;
		segBuff[2] = flashFlag ? DIG_SEG[SEG_NULL] : DIG_SEG[rtcTime.RTC.Month % 10];
		tmp = rtcTime.RTC.Month / 10;
		segBuff[3] = flashFlag ? DIG_SEG[SEG_NULL] : DIG_SEG[tmp == 0 ? SEG_NULL : tmp];
		break;
	case (fsmYearSetting) :
		segBuff[0] = flashFlag ? DIG_SEG[SEG_NULL] : DIG_SEG[rtcTime.RTC.Year % 10];
		segBuff[1] = flashFlag ? DIG_SEG[SEG_NULL] : DIG_SEG[rtcTime.RTC.Year / 10] ;
		segBuff[2] = DIG_SEG[0];
		segBuff[3] = DIG_SEG[2];
		break;

	case (fsmAlarmMinuteSetting):
		segBuff[0] = flashFlag ? DIG_SEG[SEG_NULL] : (DIG_SEG[alarmDict.ShortTimeArray[alarmTimeNum].Minute % 10] | (alarmTimeNum % 2 ? DIG_SEG[SEG_DOT] : DIG_SEG[SEG_NULL]));
		segBuff[1] = flashFlag ? DIG_SEG[SEG_NULL] : (DIG_SEG[alarmDict.ShortTimeArray[alarmTimeNum].Minute / 10] | (alarmTimeNum % 2 ? DIG_SEG[SEG_NULL] : DIG_SEG[SEG_DOT]));
		segBuff[2] = DIG_SEG[alarmDict.ShortTimeArray[alarmTimeNum].Hour % 10];
		tmp = alarmDict.ShortTimeArray[alarmTimeNum].Hour / 10;
		segBuff[3] = DIG_SEG[tmp == 0 ? SEG_NULL : tmp];
		break;
	case (fsmAlarmHourSetting):
		segBuff[0] = DIG_SEG[alarmDict.ShortTimeArray[alarmTimeNum].Minute % 10] | (alarmTimeNum % 2 ? DIG_SEG[SEG_DOT] : DIG_SEG[SEG_NULL]);
		segBuff[1] = DIG_SEG[alarmDict.ShortTimeArray[alarmTimeNum].Minute / 10] | (alarmTimeNum % 2 ? DIG_SEG[SEG_NULL] : DIG_SEG[SEG_DOT]);
		segBuff[2] = flashFlag ? DIG_SEG[SEG_NULL] : DIG_SEG[alarmDict.ShortTimeArray[alarmTimeNum].Hour % 10];
		tmp = alarmDict.ShortTimeArray[alarmTimeNum].Hour / 10;
		segBuff[3] = flashFlag ? DIG_SEG[SEG_NULL] : DIG_SEG[tmp == 0 ? SEG_NULL : tmp];
		break;
	default:
		break;
	}

	if (++flashCounter == FLASH_LEVEL)
	{
		flashCounter = 0;
		if (flashFlag )
			flashFlag = FALSE;
		else
			flashFlag = TRUE;
	}
}

void SaveAlarmTimeSetting()
{
	ISPSectorErase(ISP_EEPROM_ADDR);
	ISPWriteByte(ISP_EEPROM_ADDR, ISP_DATA_HEAD);
	ISPArrayWrite(ISP_EEPROM_ADDR + 1, 4 * MAX_ALARM_NUM, (UINT8 *)alarmDict.ShortTimeArray);
}
