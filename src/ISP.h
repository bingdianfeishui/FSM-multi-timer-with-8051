/******************************************************************************
* 文    件: ISP.h
* 原作者: 李锋源
* 修  改: ZhnJa
* 创建日期: 2011-7-15
* 修改日期: 2013-8-01
******************************************************************************/
#ifndef	__ISP_H__
#define __ISP_H__

#include <common.h>
//系统配置
#define Self_Define_ISP_Download			//ISP下载
#define ICPCODE		0x12,0x34,0x56,0x78,0x90,0xAB,0xCD,0xEF
#define FSOC 11059200 			//晶振频率，Hz
#define TSOC 6					//机器周期模式，12T、6T、1T
#define BAUD 9600UL	//9600b/s

#define TH1_NUM (FSOC/TSOC/BAUD/16) //波特率倍增


sfr IAP_CONTR = 0xE7;
//函数声明

//串口
void UARTInit(void);

void SendByte(UINT8 c);
void SendStr(INT8 *s);

void delay1s(void);


#endif
