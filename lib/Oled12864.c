/**
 ***********************************************************************
 ************      Copyright(C), 2010-2016, 吐泡泡的虾      ************
 ***********************************************************************

 *@标题			:	Oled12864-SSD1306显示驱动
 *@版本			:	v1.0
 *@作者			:	吐泡泡的虾
 *@更新日期		:	2016-10-06 00:32:27
 *@硬件			:	STC90C516RD+
 *@晶振			:	12.0MHz
 *@说明			:	4-wires SPI驱动方式
 *@版本历史		:
 	#v1.0	[Press F5 to insert time]
 		1. 1.0
 ***********************************************************************
 ***********************************************************************
 */


#include "Oled12864.h"
#include "Fonts.h"

sbit Oled_CS   = P1 ^ 0;
sbit Oled_DC   = P1 ^ 1;
sbit Oled_RES  = P1 ^ 2;
sbit Oled_SDIN = P1 ^ 3;
sbit Oled_SCLK = P1 ^ 4;

#define Oled_CS_Set()  		Oled_CS=1
#define Oled_CS_Reset()  	Oled_CS=0

#define Oled_DC_Set()  		Oled_DC=1
#define Oled_DC_Reset()  	Oled_DC=0

#define Oled_RES_Set()  	Oled_RES=1
#define Oled_RES_Reset()  	Oled_RES=0

#define Oled_SCLK_Set()  	Oled_SCLK=1
#define Oled_SCLK_Reset()  	Oled_SCLK=0

#define Oled_SDIN_Set()  	Oled_SDIN=1
#define Oled_SDIN_Reset()  	Oled_SDIN=0


static void SpiSendByte(UINT8 dat)
{
	UINT8 i;
	Oled_CS_Reset();
	for (i = 0; i < 8; i++)
	{
		Oled_SCLK_Reset();
		if (dat & 0x80)
			Oled_SDIN_Set();
		else
			Oled_SDIN_Reset();
		Oled_SCLK_Set();
		dat <<= 1;
	}
	Oled_CS_Set();
}

static UINT8 SpiReadByte()
{
	UINT8 dat, i;
	Oled_CS_Reset();
	for (i = 0; i < 8; i++)
	{
		dat <<= 1;
		Oled_SCLK_Set();
		dat |= Oled_SDIN;
		Oled_SCLK_Reset();
	}
	Oled_CS_Set();
	return dat;
}

void OledSendCmd(UINT8 cmd)
{
	Oled_DC_Reset();
	SpiSendByte(cmd);
	Oled_DC_Set();
}

void OledSendData(UINT8 dat)
{
	Oled_DC_Set();
	SpiSendByte(dat);
	Oled_DC_Set();
}

UINT8 OledReadData()
{
	UINT8 dat;
	Oled_DC_Reset();
	dat = SpiReadByte();
	Oled_DC_Set();
	return dat;
}

void OledSetPageAndCol(UINT8 page, UINT8 col)
{
	OledSendCmd(0xB0 + page);
	OledSendCmd((col >> 4) | 0x10);
	OledSendCmd(col & 0x0f);
}

void Oled12864Init()
{
	Oled_RES_Set();
	Oled_RES_Reset();
	Oled_RES_Set();
	/*
	OledSendCmd(0xAE);//--turn off oled panel
	OledSendCmd(0x00);//---set low column address
	OledSendCmd(0x10);//---set high column address
	OledSendCmd(0x40);//--set start line address  Set Mapping RAM Display Start Line (0x00~0x3F)
	OledSendCmd(0x81);//--set contrast control register
	OledSendCmd(0xCF); // Set SEG Output Current Brightness
	OledSendCmd(0xA1);//--Set SEG/Column Mapping     0xa0左右反置 0xa1正常
	OledSendCmd(0xC8);//Set COM/Row Scan Direction   0xc0上下反置 0xc8正常
	OledSendCmd(0xA6);//--set normal display
	OledSendCmd(0xA8);//--set multiplex ratio(1 to 64)
	OledSendCmd(0x3f);//--1/64 duty
	OledSendCmd(0xD3);//-set display offset	Shift Mapping RAM Counter (0x00~0x3F)
	OledSendCmd(0x00);//-not offset
	OledSendCmd(0xd5);//--set display clock divide ratio/oscillator frequency
	OledSendCmd(0x80);//--set divide ratio, Set Clock as 100 Frames/Sec
	OledSendCmd(0xD9);//--set pre-charge period
	OledSendCmd(0xF1);//Set Pre-Charge as 15 Clocks & Discharge as 1 Clock
	OledSendCmd(0xDA);//--set com pins hardware configuration
	OledSendCmd(0x12);
	OledSendCmd(0xDB);//--set vcomh
	OledSendCmd(0x40);//Set VCOM Deselect Level
	OledSendCmd(0x20);//-Set Page Addressing Mode (0x00/0x01/0x02)
	OledSendCmd(0x02);//
	OledSendCmd(0x8D);//--set Charge Pump enable/disable
	OledSendCmd(0x14);//--set(0x10) disable
	OledSendCmd(0xA4);// Disable Entire Display On (0xa4/0xa5)
	OledSendCmd(0xA6);// Disable Inverse Display On (0xa6/a7)
	OledSendCmd(0xAF);//--turn on oled panel
	*/

	OledSendCmd(0xAE); //--turn off oled panel
	OledSendCmd(0x00); //---set low column address
	OledSendCmd(0x10); //---set high column address
	OledSendCmd(0x40); //--set start line address  Set Mapping RAM Display Start Line (0x00~0x3F)
	OledSendCmd(0x81); //--set contrast control register
	OledSendCmd(0xCF); // Set SEG Output Current Brightness
	OledSendCmd(0xA1); //--Set SEG/Column Mapping     0xa0左右反置 0xa1正常
	OledSendCmd(0xC8); //Set COM/Row Scan Direction   0xc0上下反置 0xc8正常
	OledSendCmd(0xA6); //--set normal display
	OledSendCmd(0xA8); //--set multiplex ratio(1 to 64)
	OledSendCmd(0x3f); //--1/64 duty
	OledSendCmd(0xD3); //-set display offset	Shift Mapping RAM Counter (0x00~0x3F)
	OledSendCmd(0x00); //-not offset
	OledSendCmd(0xd5); //--set display clock divide ratio/oscillator frequency
	OledSendCmd(0x80); //--set divide ratio, Set Clock as 100 Frames/Sec
	OledSendCmd(0xD9); //--set pre-charge period
	OledSendCmd(0xF1); //Set Pre-Charge as 15 Clocks & Discharge as 1 Clock
	OledSendCmd(0xDA); //--set com pins hardware configuration
	OledSendCmd(0x12);
	OledSendCmd(0xDB); //--set vcomh
	OledSendCmd(0x40); //Set VCOM Deselect Level
	OledSendCmd(0x20); //-Set Page Addressing Mode (0x00/0x01/0x02)
	OledSendCmd(0x02); //
	OledSendCmd(0x8D); //--set Charge Pump enable/disable
	OledSendCmd(0x14); //--set(0x10) disable
	OledSendCmd(0xA4); // Disable Entire Display On (0xa4/0xa5)
	OledSendCmd(0xA6); // Disable Inverse Display On (0xa6/a7)
	OledSendCmd(0xAF); //--turn on oled panel

	Oled12864DisplayControl(true);
	Oled12864Clear();
	// OledSetPageAndCol(0, 0);
}

void Oled12864Clear()
{
	UINT8 page, col;
	for (page = 0; page < OLED12864_MAX_PAGE; page++)
	{
		OledSendCmd(0xB0 + page);	//设置页地址（0~7）
		OledSendCmd(0x00);			//设置显示位置—列低地址
		OledSendCmd(0x10);			//设置显示位置—列高地址
		for (col = 0; col < OLED12864_MAX_COL; col++)
		{
			OledSendData(0x00);
		}
	}
}

void Oled12864DisplayControl(BOOL onOff)
{
	OledSendCmd(0X8D);  //SET DCDC命令
	if (onOff)
	{
		OledSendCmd(0X14);  //DCDC ON
		OledSendCmd(0XAF);  //DISPLAY ON
	}
	else
	{
		OledSendCmd(0X10);  //DCDC OFF
		OledSendCmd(0XAE);  //DISPLAY OFF
	}
}

void OledDisplayChar(UINT8* page, UINT8* col, UINT8* index, BOOL isCn)
{
	// static UINT8 enLen = sizeof(enFonts) / sizeof(enFonts[0]);
	// static UINT8 cnLen = sizeof(cnFonts) / sizeof(cnFonts[0]);

	BOOL isMatch;
	UINT8 charNum, j;
	const UINT8 halfCount = isCn ? 16 : 8;
	const UINT16 maxLen = isCn ? cnLen : enLen;

	for (charNum = 0; charNum < maxLen;)
	{
		isMatch = isCn ? (*index == cnFonts[charNum].Index[0] && *(index + 1) == cnFonts[charNum].Index[1] )
		          : (*index == enFonts[charNum].Index);
		if (isMatch)
		{
			OledSetPageAndCol(*page, *col);
			for (j = 0; j < halfCount * 2; j++)
			{
				if (j == halfCount)
					OledSetPageAndCol(*page + 1, *col);

				OledSendData(isCn ? cnFonts[charNum].Mask[j] : enFonts[charNum].Mask[j]);
			}
			(*col) += halfCount;
			break;
		}

		if (++charNum == maxLen)	//匹配到最后一个还没找到
		{
			OledSetPageAndCol(*page, *col);
			for (j = 0; j < halfCount * 2; j++)
			{
				if (j == halfCount)
					OledSetPageAndCol(*page + 1, *col);

				OledSendData(0xff);
			}
			(*col) += halfCount;
		}
	}
}

void Oled12864DisplayEn(UINT8 page, UINT8 col, UINT8* enIndex)
{
	while (*enIndex != '\0')
	{
		OledDisplayChar(&page, &col, enIndex, false);
		enIndex++;
	}
}

void Oled12864DisplayCn(UINT8 page, UINT8 col, UINT8* cnIndex)
{
	while (*cnIndex != '\0')
	{
		OledDisplayChar(&page, &col, cnIndex, true);

		cnIndex++;
	}
}

void Oled12864DisplayString(UINT8 page, UINT8 col, UINT8* str)
{
	while (*str != '\0')
	{
		if (*str > 0xA0)	//汉字区位码大于0xA0
		{
			OledDisplayChar(&page, &col, str, true);
			str += 2;
		}
		else	//ASCII字符
		{
			OledDisplayChar(&page, &col, str, false);
			str++;
		}
	}
}

void Oled12864DisplayPic(UINT8 beginPage, UINT8 beginCol, UINT8 picX, UINT8 picY, UINT8* pic)
{
	UINT8 page, col;
	UINT8* pt;
	for (page = 0; page < (picY + 7) / 8; page++)
	{
		if (beginPage + page > 8)
			break;

		pt = pic + page * picX;
		OledSetPageAndCol(beginPage + page, beginCol);
		for (col = 0; col < picX; col++)
		{
			if (beginCol + col > 128)
				break;
			OledSendData(*pt++);
		}
	}
}
