#ifndef __DELAY_H__
#define __DELAY_H__

#include "Common.h"

#ifndef FSOC_12M_MOD_1T_STC12 	//如果没有定义1T模式，默认为12T
//定义默认设置：晶振12MHz，模式12T
#define FSOC_12M_MOD_12T
#endif

#define NOP()		_nop_()

/**
 * 晶振12MHz，12T模式下的延时。
 */
#ifdef FSOC_12M_MOD_12T

#define Delay1us()	NOP()
#define Delay2us()	NOP();NOP()
#define Delay5us()	NOP();NOP();NOP();NOP();NOP()


#elif defined(FSOC_12M_MOD_1T_STC12)
/**
 * STC12，晶振12MHz，1T模式下的延时。
 */
void Delay1us();
void Delay2us();
void Delay5us();
#else
// 必须在主程序中预先定义工作模式
#error "Neither FSOC_12M_MOD_12T nor FSOC_12M_MOD_1T_STC12 defined!"
#endif



void DelayX10us(UINT8 X);
void DelayX1ms(UINT8 X);
void DelayX10ms(UINT8 X);
void DelayX1s(UINT8 X);

#endif
