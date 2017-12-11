#ifndef __OLED12864_H__
#define __OLED12864_H__
#include <common.h>

#define OLED12864_MAX_COL  128
#define OLED12864_MAX_PAGE (64/8)

UINT8 OledReadData();
void Oled12864Init();
void Oled12864Clear();
void Oled12864DisplayControl(BOOL onOff);
void Oled12864DisplayEn(UINT8 page, UINT8 col, UINT8* enIndex);
void Oled12864DisplayCn(UINT8 page, UINT8 col, UINT8* cnIndex);
void Oled12864DisplayString(UINT8 page, UINT8 col, UINT8* str);
void Oled12864DisplayPic(UINT8 beginPage, UINT8 beginCol, UINT8 picX, UINT8 picY, UINT8* pic);


static void SpiSendByte(UINT8 dat);
static UINT8 SpiReadByte();
static void OledSendCmd(UINT8 cmd);
static void OledSendData(UINT8 dat);
static void OledSetPageAndCol(UINT8 page, UINT8 col);
static void OledDisplayChar(UINT8* page, UINT8* col, UINT8* index, BOOL isCn);
#endif
