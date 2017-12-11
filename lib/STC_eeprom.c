#include "STC_eeprom.h"
#include "Delay.h"

// /* 用户程序需要记忆的数组, 用户实际使用了n-1个数据，数组长度规整到
// 	2 4 8 16 32 64 上 */
// UINT8 Ttotal[16]	=
// {
// 	0x55,				/* 作为判别引导头使用，用户程序请不要修改它 */
// 	/* 用户保存记忆的数据 */
// 	0x01,				/* 用途说明....*/
// 	0x02,
// 	0x03,
// 	0x04,
// 	0x05,
// 	0x06,
// 	0x07,
// 	0x08,
// 	0x09,
// 	0x0a,
// 	0x0b,
// 	0x0c,
// 	0x0d,
// 	0x0e,
// 	0x0f,
// };

// UINT16 	timerForDelay,		/* 专供延时用的变量 */
//         i,					/* 循环变量			*/
//         EepromPtr;			/* eeprom读写指针	*/

/* --------------- 命令定义 --------------- */
#define ReadCmd			0x01	/* 字节读 	*/
#define ProgramCmd		0x02	/* 字节写 	*/
#define EraseCmd		0x03	/* 扇区擦除 */

/* 定义常量 */
#define Error   1
#define Ok      0

/* 定义Flash对应于20MHz晶振系统的操作等待时间 */
/* 时钟倍频时WaitTime用 0x00*/
// #define WaitTime	0x00 	//FOSC<40MHz
#define WaitTime	0x01 	//FOSC<20MHz
// #define WaitTime	0x02 	//FOSC<10MHz
// #define WaitTime	0x03 	//FOSC<5MHz


/* ================ 打开 ISP,IAP 功能 ================= */
void ISPIAPEnable(void)
{
	EA	=	0;							/* 关中断 		*/
	ISP_CONTR =	ISP_CONTR & 0x18;       /* 0001,1000	*/
	ISP_CONTR =	ISP_CONTR | WaitTime;	/* 写入硬件延时	*/
	ISP_CONTR =	ISP_CONTR | 0x80;       /* ISPEN=1		*/
}

/* =============== 关闭 ISP,IAP 功能 ================== */
void ISPIAPDisable(void)
{
	ISP_CONTR	=	ISP_CONTR & 0x7f;	/* ISPEN = 0 */
	ISP_TRIG	=	0x00;
	EA			=   1;			/* 开中断 */
}
/* ================ 公用的触发代码 ==================== */
void ISPGoOn(void)
{
	ISPIAPEnable();			/* 打开 ISP,IAP 功能	*/
	ISP_TRIG	=	0x46;		/* 触发ISP_IAP命令字节1	*/
	ISP_TRIG	=	0xb9;		/* 触发ISP_IAP命令字节2	*/
	NOP();
}

/* ==================== 字节读 ======================== */
UINT8 ISPReadByte(UINT16 byte_addr)
{
	ISP_ADDRH = (UINT8)(byte_addr >> 8);	/* 地址赋值	*/
	ISP_ADDRL = (UINT8)(byte_addr & 0x00ff);

	ISP_CMD   = ISP_CMD	& 0xf8;			/* 清除低3位 	*/
	ISP_CMD   = ISP_CMD	| ReadCmd;	/* 写入读命令	*/

	ISPGoOn();							/* 触发执行		*/
	ISPIAPDisable();				/* 关闭ISP,IAP功能	*/

	return (ISP_DATA);				/* 返回读到的数据	*/
}

/* ================== 扇区擦除 ======================== */
void ISPSectorErase(UINT16 sector_addr)
{
	UINT16 iSectorAddr;
	iSectorAddr = (sector_addr & 0xfe00); /* 取扇区地址 */
	ISP_ADDRH = (UINT8)(iSectorAddr >> 8);
	ISP_ADDRL = 0x00;

	ISP_CMD	= ISP_CMD & 0xf8;			/* 清空低3位 	*/
	ISP_CMD	= ISP_CMD | EraseCmd;	/* 擦除命令3 	*/

	ISPGoOn();							/* 触发执行		*/
	ISPIAPDisable();				/* 关闭ISP,IAP功能	*/

}

/* ==================== 字节写 ======================== */
void ISPWriteByte(UINT16 byte_addr, UINT8 original_data)
{
	ISP_ADDRH =	(UINT8)(byte_addr >> 8); 	/* 取地址 	*/
	ISP_ADDRL =	(UINT8)(byte_addr & 0x00ff);

	ISP_CMD	 = ISP_CMD & 0xf8;				/* 清低3位	*/
	ISP_CMD  = ISP_CMD | ProgramCmd;		/* 写命令2	*/
	ISP_DATA = original_data;			/* 写入数据准备	*/

	ISPGoOn();							/* 触发执行		*/
	ISPIAPDisable();					/* 关闭IAP功能	*/
}

/* =================== 字节写并校验 =================== */
UINT8 ISPWriteByteVerify(UINT16 byte_addr, UINT8 original_data)
{
	ISP_ADDRH = (UINT8)(byte_addr >> 8); 	/* 取地址 	*/
	ISP_ADDRL = (UINT8)(byte_addr & 0xff);

	ISP_CMD  = ISP_CMD & 0xf8;				/* 清低3位	*/
	ISP_CMD  = ISP_CMD | ProgramCmd;		/* 写命令2	*/
	ISP_DATA = original_data;

	ISPGoOn();							/* 触发执行		*/

	/* 开始读，没有在此重复给地址，地址不会被自动改变 	*/
	ISP_DATA = 0x00;				/* 清数据传递寄存器	*/

	ISP_CMD = ISP_CMD & 0xf8;				/* 清低3位	*/
	ISP_CMD = ISP_CMD | ReadCmd;			/* 读命令1	*/

	ISP_TRIG	=	0x46;		/* 触发ISP_IAP命令字节1	*/
	ISP_TRIG	=	0xb9;		/* 触发ISP_IAP命令字节2 */
	NOP();					/* 延时	*/

	ISPIAPDisable();					/* 关闭IAP功能	*/

	if (ISP_DATA	== original_data) {		/* 读写数据校验	*/
		return	Ok;						/* 返回校验结果	*/
	}
	else {
		return	Error;
	}
}


/* ===================== 数组写入 ===================== */
UINT8 ISPArrayWrite(UINT16 begin_addr, UINT16 len, UINT8 *array)
{
	UINT16	i;
	UINT16	in_addr;

	/* 判是否是有效范围,此函数不允许跨扇区操作 */
	if (len > BytesPerSector) {
		return Error;
	}
	in_addr = begin_addr & 0x01ff;	 	/* 扇区内偏移量 */
	if ((in_addr + len) > BytesPerSector) {
		return Error;
	}

	in_addr = begin_addr;
	/* 逐个写入并校对 */
	ISPIAPEnable();					/* 打开IAP功能	*/
	for (i = 0; i < len; i++) {
		/* 写一个字节 */
		ISP_ADDRH = (UINT8)(in_addr >> 8);
		ISP_ADDRL = (UINT8)(in_addr & 0x00ff);
		ISP_DATA  = array[i];				/* 取数据	*/
		ISP_CMD   = ISP_CMD & 0xf8;			/* 清低3位 	*/
		ISP_CMD   = ISP_CMD | ProgramCmd;	/* 写命令2 	*/

		ISP_TRIG  = 0x46;		/* 触发ISP_IAP命令字节1 */
		ISP_TRIG  = 0xb9;		/* 触发ISP_IAP命令字节2 */
		NOP();

		/* 读回来 */
		ISP_DATA	=	0x00;

		ISP_CMD  = ISP_CMD & 0xf8;			/* 清低3位 	*/
		ISP_CMD  = ISP_CMD | ReadCmd;		/* 读命令1 	*/

		ISP_TRIG = 0x46;		/* 触发ISP_IAP命令字节1 */
		ISP_TRIG = 0xb9;		/* 触发ISP_IAP命令字节2 */
		NOP();

		/*  比较对错 */
		if (ISP_DATA != array[i]) {
			ISPIAPDisable();
			return Error;
		}
		in_addr++;					/* 指向下一个字节	*/
	}
	ISPIAPDisable();
	return	Ok;
}

/* ========================= 扇区读出 ========================= */
/* 程序对地址没有作有效性判断，请调用方事先保证他在规定范围内	*/
void ISPArrayRead(UINT16 begin_addr, UINT8 len, UINT8 *array)
{
	UINT16 iSectorAddr;
	UINT16 i;
	iSectorAddr = begin_addr;	// & 0xfe00; 		/* 取扇区地址 	*/

	ISPIAPEnable();
	for (i = 0; i < len; i++)
	{
		ISP_ADDRH =	(UINT8)(iSectorAddr >> 8);
		ISP_ADDRL =	(UINT8)(iSectorAddr & 0x00ff);

		ISP_CMD   =	ISP_CMD	& 0xf8;				/* 清低3位 	*/
		ISP_CMD   =	ISP_CMD	| ReadCmd;		/* 读命令1 	*/
		ISP_DATA = 0;
		ISP_TRIG = 0x46;			/* 触发ISP_IAP命令字节1 */
		ISP_TRIG = 0xb9;			/* 触发ISP_IAP命令字节2 */
		NOP();

		array[i]	=	ISP_DATA;
		iSectorAddr++;
	}
	ISPIAPDisable();						/* 关闭IAP功能	*/
}


// /* ==============================================================
//  从eeprom中读取数据
//  ============================================================== */
// void DataRestore()
// {
// 	EepromPtr = BaseAddr;				/* 指向eeprom的起始点	*/
// 	while (EepromPtr < EndAddr)			/* 在eeprom的可用区域内	*/
// 	{
// 		if (ISPReadByte(EepromPtr) == 0x55) /* 找到了上一次有效纪录	*/
// 		{
// 			break;						/*	寻找完成			*/
// 		}
// 		EepromPtr += 0x10;				/* 指向下一个小区		*/
// 	}
// 	if (EepromPtr >= EndAddr)			/* 如果照遍都没有,是新片*/
// 	{
// 		EepromPtr = BaseAddr;			/* 指向eeprom的起始点	*/
// 		for (i = 0; i < 90; i++)
// 		{
// 			SectorErase(EepromPtr + 0x200 * i);	/* 全部扇区擦除		*/
// 		}
// 		while (ISPArrayWrite(EepromPtr, 0x10, Ttotal))	/* 写默认值	*/
// 		{	/* 写入失败才运行的部分	*/
// 			ISPWriteByte(EepromPtr, 0);	/* 该单元已经失效		*/
// 			if (EepromPtr < EndAddr)
// 			{
// 				EepromPtr += 0x10;		/* 换一块新的小区		*/
// 			}
// 			else
// 			{
// 				P1 = 0;					/* 指示芯片内eeprom全坏	*/
// 				EA = 0;					/* 不再做任何事			*/
// 				while (1);				/* 死机					*/
// 			}
// 		}
// 	}
// 	ISPArrayRead(EepromPtr, 16);
// }

// /* ==============================================================
//  将需要记忆的数据保存到eeprom
//  ============================================================== */
// void DataSave()
// {
// 	UINT16	wrPtr;									/* 临时指针		*/

// NextArea:
// 	ISPWriteByteVerify(EepromPtr, 0);		/* 将原来的标记清除	*/
// 	wrPtr = EepromPtr & 0xfe00;	/* 上一个扇区的起始地址	*/
// 	EepromPtr += 0x10;						/* 目标存入地址		*/

// 	/* ------------------ 判断是否启用新的扇区 ---------------- */
// 	if ((EepromPtr & 0x1ff) == 0)
// 	{
// 		SectorErase(wrPtr);			/* 将上一个扇区擦除，备用	*/
// 		if (EepromPtr >= EndAddr)		/* 已经用完了最后一个区域	*/
// 		{
// 			EepromPtr = BaseAddr;					/* 从头开始	*/
// 		}
// 	}
// 	/* -------------------- 数据存入前的准备 ------------------ */
// 	/* 。。。。。。。。。。。。。。转移、处理					*/
// 	Ttotal[0] = 0x55;						/* 重申启用标记		*/
// 	if (ISPArrayWrite(EepromPtr, 0x10, Ttotal))
// 	{	/* 数据写入，如果有错换一块	*/
// 		goto NextArea;
// 	}
// }
