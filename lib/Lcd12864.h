#ifndef __LCD12864_H__
#define  __LCD12864_H__

#include <Common.h>
#include <Delay.h>

void LcdSendCmd(UINT8 cmd);
void LcdSendData(UINT8 dat);
void Lcd12864Init();
void LcdClearScreen();
void LcdSetPageAndCol(UINT8 page, UINT8 col);
void LcdSetPage(UINT8 page);
void LcdSetColumn(UINT8 col);
void LcdOnOff(BOOL on);
void LcdDisplayCn(UINT8 page, UINT8 col, UINT8* cnIndex);
void LcdDisplayEn(UINT8 page, UINT8 col, UINT8* enIndex);
void LcdDisplayStr(UINT8 page, UINT8 col, UINT8* str);
void LcdDisplayPic(UINT8 beginPage, UINT8 beginCol, UINT8 picX, UINT8 picY, UINT8* pic);
#endif
