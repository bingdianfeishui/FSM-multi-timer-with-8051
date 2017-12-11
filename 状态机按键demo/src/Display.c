#include "Display.h"

sbit SCLK_595 = P0 ^ 0;		//P3^6: 595的移位寄存器时钟管脚 SCK
sbit RCLK_595 = P0 ^ 1;		//P3^5: 595的输出寄存器时钟管脚 RCK
sbit DS_595 = P0 ^ 2;		//P3^4: 595的数据输入管脚



static void InputTo595(UINT8 *buff, UINT8 len);
static void OutputFrom595();

void Display4DigSeg()
{
	UINT8 buff[2], i;
	for (i = 0; i < SEG_NUM; i++)
	{
		buff[0] = DIG_POS[i];
		buff[1] = ~segBuff[i];
		InputTo595(buff, 2);
		OutputFrom595();
	}
}

void InitDigSeg(UINT8 segCode)
{
	UINT8 buff[2], i;
	for (i = 0; i < SEG_NUM; i++)
	{
		buff[0] = DIG_POS[i];
		buff[1] = ~segCode;
		InputTo595(buff, 2);
		OutputFrom595();
	}
}

/**
 * 将datacode数组输入级联的595芯片，最后一个元素先输入，从低位到高位顺序输入
 * @param buff 输入数组
 * @param len      要输入的数组元素个数，从数组第一个元素开始计
 */
void InputTo595(UINT8 *buff, UINT8 len)
{
	UINT8 i, j;

	for (j = len; j > 0; j--)
	{
		for (i = 0; i < 8; i++)
		{
			DS_595 = (buff[j - 1] & 0x80) ;
			buff[j - 1] <<= 1;
			SCLK_595 = 0;
			SCLK_595 = 1;
			SCLK_595 = 0;
		}
	}
}
void OutputFrom595()
{
	RCLK_595 = 0;
	RCLK_595 = 1;
	RCLK_595 = 0;
}

