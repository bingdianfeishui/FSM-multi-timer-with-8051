#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#include <intrins.h>
#include <Delay.h>
#include <common.h>

#define SEG_DOT 16
#define SEG_NULL 17
#define SEG_DASH 18

#define SEG_NUM 4

extern UINT8 segBuff[];
extern UINT8 code DIG_SEG[];
extern UINT8 code DIG_POS[];

void Display4DigSeg(BOOL onOff);
void InitDigSeg(UINT8 segCode);

#endif
