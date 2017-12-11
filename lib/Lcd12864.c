/**
 **********************************************************
 ******    Copyright(C), 2010-2016, NULL Co.,Ltd     ******
 **********************************************************

 *@Tittle		:	Lcd12864-ST7565无字库液晶显示屏驱动程序
 *@Version		:	v1.0
 *@Author		:	吐泡泡的虾
 *@Dat			:	2016-09-01 17:09:44
 *@Desctription	:	适用于ST7565驱动的无字库Lcd12864液晶屏，
 *					8080总线模式，并行
 *
 *@History		:


 **********************************************************
 **********************************************************
 */


#include "Lcd12864.h"
#include "Fonts.h"


sbit Lcd_WR  = P2 ^ 7;
sbit Lcd_RD  = P2 ^ 5;
sbit Lcd_RS  = P2 ^ 6;
sbit Lcd_A0  = P2 ^ 6;

sbit Lcd_CS  = P3 ^ 2;
sbit Lcd_RES = P3 ^ 3;

sbit Lcd_D0  = P0 ^ 0;
sbit Lcd_D1  = P0 ^ 1;
sbit Lcd_D2  = P0 ^ 2;
sbit Lcd_D3  = P0 ^ 3;
sbit Lcd_D4  = P0 ^ 4;
sbit Lcd_D5  = P0 ^ 5;
sbit Lcd_D6  = P0 ^ 6;
sbit Lcd_D7  = P0 ^ 7;

#ifdef LCD_ST7565_SPI
sbit Lcd_SCL = P0 ^ 6;
sbit Lcd_SI  = P0 ^ 7;
#else
#define LcdData P0
#endif

#ifdef LCD_ST7565_SPI
void St7565SendByte(UINT8 dat)
{
	UINT8 i;
	for (i = 0; i < 8; i++)
	{
		Lcd_SCL = 0;
		Lcd_SI = (dat & 0x80) ? 1 : 0;
		Lcd_SCL = 1;
		dat <<= 1;
	}
}

// SPI方式不能读取
// UINT8 St7565ReadByte()
// {
// 	UINT8 dat = 0;

// 	return dat;
// }
#else
void St7565SendByte(UINT8 dat)
{
	LcdData = dat;
}

UINT8 St7565ReadByte()
{
	UINT8 dat = LcdData;
	return dat;
}
#endif


void LcdSendCmd(UINT8 cmd)
{
	Lcd_CS = 0;	//片选
	Lcd_RD = 1;	//读失能
	Lcd_A0 = 0;	//指令数据
	Lcd_WR = 0;	//写使能

	St7565SendByte(cmd);

	Lcd_WR = 1;
}

void LcdSendData(UINT8 dat)
{
	Lcd_CS = 0;	//片选
	Lcd_RD = 1;	//读失能
	Lcd_A0 = 1;	//显示数据
	Lcd_WR = 0;	//写使能

	St7565SendByte(dat);

	Lcd_WR = 1;
}

#ifndef LCD_ST7565_SPI
BOOL LcdIsBusy()
{
	return (St7565ReadByte() & 0x80) ? true : false;
}
#endif

void LcdOnOff(BOOL on)
{
	if (on)
		LcdSendCmd(0xAF);
	else
		LcdSendCmd(0xAE);
}

void LcdSetPage(UINT8 page)
{
	LcdSendCmd(0xB0 | page);
}

void LcdSetStartLine(UINT8 line)
{
	LcdSendCmd(0x40 | line);
}

void LcdSetColumn(UINT8 col)
{
	LcdSendCmd(0x10 | (col >> 4 & 0x0f));
	LcdSendCmd(0x00 | (col & 0x0f));
}

void LcdSetPageAndCol(UINT8 page, UINT8 col)
{
	LcdSetPage(page);
	LcdSetColumn(col);
}

void LcdClearScreen()
{
	UINT8 page, col;
	for (page = 0; page < 8; page++)
	{
		LcdSetPageAndCol(page, 0);
		for (col = 0; col < 128; col++)
		{
			LcdSendData(0x00);
		}
	}
}

void Lcd12864Init()
{
	Lcd_RES = 0;
	Delay1us();
	Lcd_CS  = 0;
	Lcd_RES = 1;

	//--软件初始化--//
	LcdSendCmd(0xE2);  //reset
	Delay1us();

	//--列显示方向，表格第8个命令，0xA0（左->右）方向，正常方向，(0xA1为反方向)--//
	LcdSendCmd(0xA0);  //ADC select segment direction

	//--行显示方向，表格第15个命令，0xC8(上->下)方向，(0xC0为反向)--//
	LcdSendCmd(0xC8);  //Common direction

	//--高电平显示还是低电平显示，表格第9个命令，0xA6为设置字体为黑色，背景为白色---//
	//--0xA7为设置字体为白色，背景为黑色---//
	LcdSendCmd(0xA6);  //reverse display

	//--显示所有，表格第10个命令，0xA4像素正常显示，0xA5像素全开--//
	LcdSendCmd(0xA4);  //normal display

	//--偏压设置，表格第11个命令，0xA3偏压为1/7,0xA2偏压为1/9--//
	LcdSendCmd(0xA2);  //bias set 1/9

	//--增压设置，表格第20个命令，这个是个双字节的命令，0xF800选择增压为4X;--//
	//--0xF801,选择增压为4X，其实效果差不多--//
	LcdSendCmd(0xF8);  //Boost ratio set
	LcdSendCmd(0x03);  //x4

	//--背景电压调节，表格第18个命令，这个是个双字节命令，高字节为0X81，低字节可以--//
	//--选择从0x01到0X3F。用来设置背景光对比度。---//
	LcdSendCmd(0x81);  //V0 a set
	LcdSendCmd(0x20);

	//--电阻率调节，表格第17个命令，选择调节电阻率--//
	LcdSendCmd(0x25);  //Ra/Rb set

	//--表格第16个命令，电源设置。--//
	LcdSendCmd(0x2F);
	Delay1us();

	//--表格第2个命令，设置显示开始位置--//
	LcdSendCmd(0x40);  //start line

	// LcdSendCmd(0xAF);
	LcdOnOff(true);

	LcdClearScreen();
}

void LcdDisplayChar(UINT8* page, UINT8* col, UINT8* index, BOOL isCn)
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
			LcdSetPageAndCol(*page, *col);
			for (j = 0; j < halfCount * 2; j++)
			{
				if (j == halfCount)
					LcdSetPageAndCol(*page + 1, *col);

				LcdSendData(isCn ? cnFonts[charNum].Mask[j] : enFonts[charNum].Mask[j]);
			}
			(*col) += halfCount;
			break;
		}

		if (++charNum == maxLen)	//匹配到最后一个还没找到
		{
			LcdSetPageAndCol(*page, *col);
			for (j = 0; j < halfCount * 2; j++)
			{
				if (j == halfCount)
					LcdSetPageAndCol(*page + 1, *col);

				LcdSendData(0xff);
			}
			(*col) += halfCount;
		}
	}
}

void LcdDisplayCn(UINT8 page, UINT8 col, UINT8* cnIndex)
{

	while (*cnIndex != '\0')
	{
		LcdDisplayChar(&page, &col, cnIndex, true);

		cnIndex++;
	}
}

void LcdDisplayEn(UINT8 page, UINT8 col, UINT8* enIndex)
{
	while (*enIndex != '\0')
	{
		LcdDisplayChar(&page, &col, enIndex, false);
		enIndex++;
	}
}

void LcdDisplayStr(UINT8 page, UINT8 col, UINT8* str)
{
	while (*str != '\0')
	{
		if (*str > 0xA0)	//汉字区位码大于0xA0
		{
			LcdDisplayChar(&page, &col, str, true);
			str += 2;
		}
		else	//ASCII字符
		{
			LcdDisplayChar(&page, &col, str, false);
			str++;
		}
	}
}

void LcdDisplayPic(UINT8 beginPage, UINT8 beginCol, UINT8 picX, UINT8 picY, UINT8* pic)
{
	UINT8 page, col;
	UINT8* pt;
	for (page = 0; page < (picY + 7) / 8; page++)
	{
		if (beginPage + page > 8)
			break;

		pt = pic + page * picX;
		LcdSetPageAndCol(beginPage + page, beginCol);
		for (col = 0; col < picX; col++)
		{
			if (beginCol + col > 128)
				break;
			LcdSendData(*pt++);
		}
	}
}
