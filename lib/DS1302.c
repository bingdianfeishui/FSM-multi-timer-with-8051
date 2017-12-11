/**
 **********************************************************
 ******    Copyright(C), 2010-2016, NULL Co.,Ltd     ******
 **********************************************************

 *@Tittle		:	DS1302通用驱动。
 *@Version		:	v1.3
 *@Author		:	Liy
 *@Dat			:	2016-08-14 04:01:32
 *@Desctription	:	DS1302通用驱动，包括单字节时钟读写、RAM
 *					读写，突发模式时钟连续读写、RAM连续读写，
 *					时钟暂停与继续，涓流充电等。
 *			注意：	1. 未标识BCD的读写函数输入输出数据均为十
 *					进制编码，而非BCD编码。
 *					2. 时钟读写数据为24小时制。
 *@History		:
 *	#v1.3 	2016-08-14 04:01:42
 *		修复突发模式读取偶尔出错的bug，必须一次性读出8个字节。
 *	#v1.2 	2016-08-10 14:28:11
 *		修复突发模式无法写入的bug。必须一次性写入8字节。
 *	#v1.1 	2016-08-03 16:47:49
 *		采用typedef定义数据类型。
 *	#v1.0 	2016-08-03 01:32:13
 *		1. 完成RAM突发模式连续读功能。
 *		2. 将时钟寄存器单字节读写函数拆分为10进制和BCD两种。
 *		3. 完善DS1302控制功能，如时钟暂停与继续，涓流充电等。
 *	#v0.9	2016-08-02 14:03:15
 *		1. 实现单字节时钟、RAM读写，突发模式时钟连续读写。
 *		2. RAM突发模式连续写完成，RAM突发模式连续读未完成。
 *
 **********************************************************
 **********************************************************
 */


#include "DS1302.h"

#define DS1302_RAM_START_ADDR 0xC0 //写模式地址，读模式需+1
#define DS1302_RAM_END_ADDR 0xFC //写模式地址，读模式需+1
#define DS1302_BURST_WRITE_CLOCK_ADDR 0xBE	//突发模式时钟写地址，读取地址需+1
#define DS1302_BURST_WRITE_RAM_ADDR 0xFE	//突发模式RAM写地址，读取地址需+1
#define DS1302_MAX_RAM_BYTES	31

sbit DS1302_SCLK = P3 ^ 6;
sbit DS1302_CE 	= P3 ^ 5;
sbit DS1302_IO 	= P3 ^ 4;

//DS1302时钟寄存器地址，依次为秒、分、时、日、月、周、年、
//写保护、充电寄存器
UINT8 code ds1302RegAddr[] = {
	0x80, 0x82, 0x84, 0x86, 0x88, 0x8A, 0x8C, 0x8E, 0x90
//	  秒、 分、  时、  日、  月、  周、  年、写保护、充电寄存器
//	  0、   1、   2、   3、   4、   5、   6、   7、   8
};


/*
 **********************************************************
 *公共基础函数 Begin
 **********************************************************
 */

/**
 * 十进制转BCD码
 * @param dec 要转换的十进制数地址
 */
void Dec2BCD(UINT8 *dec)
{
	*dec = ((*dec / 10 % 10) << 4) | (*dec % 10);
}

/**
 * BCD码转十进制数
 * @param bcd 要转换的BCD码数据地址
 */
void BCD2Dec(UINT8 * bcd)
{
	*bcd = (*bcd >> 4) * 10 + (*bcd % 16);
}

/**
 * DS1302 通讯开始
 */
void DS1302Start()
{
	DS1302_IO = 0;
	Delay1us();
	DS1302_CE = 0;
	Delay1us();
	DS1302_SCLK = 0;
	Delay1us();
	DS1302_CE = 1;
}

/**
 * DS1302 通讯结束
 */
void DS1302End()
{
	DS1302_CE = 0;
	DS1302_IO = 0;	//普中V3.0开发板IO口未加上拉电阻，必须加这一句。
	//否则读出的某些数据随机为0xFF，原理未知。
	//或者在IO口加1K~10K上拉电阻则可不要这句
	Delay1us(); Delay1us(); Delay1us();
	DS1302_IO = 1;	//释放总线
	DS1302_SCLK = 1;	//释放总线
}

/**
 * 从DS1302读一个字节
 * @return UINT8,读出的数据
 */
void DS1302ByteRead(UINT8 *dataAddr)
{
	UINT8 i;

	for (i = 0; i < 8; i++)
	{
		DS1302_SCLK = 0;
		Delay1us();
		*dataAddr >>= 1;
		if (DS1302_IO == 1)
			*dataAddr |= 0x80;
		DS1302_SCLK = 1;
		Delay1us();

	}
}

/**
 * 向DS1302写一个字节
 * @param dat UINT8，需要写入的数据
 */
void DS1302ByteWrite(UINT8 dat)
{
	UINT8 i;

	for (i = 0; i < 8; i++)
	{
		DS1302_SCLK = 0;
		Delay1us();
		DS1302_IO = (dat & 0x01);
		dat >>= 1;
		DS1302_SCLK = 1;
		Delay1us();
	}
}

/**
 * DS1302寄存器单字节写入,BCD码
 * @param  regNum UINT8,寄存器地址序号，为DS1302RegAddr[]数组下标
 * @param dat    UINT8,要写入的数据,BCD码
 */
void DS1302SingleWriteBCD(UINT8 regNum, UINT8 dat)
{
	DS1302WriteEnable(1);

	DS1302Start();
	DS1302ByteWrite(ds1302RegAddr[regNum]);
	DS1302ByteWrite(dat);
	DS1302End();

	DS1302WriteEnable(0);
}

UINT8 DS1302SingleReadBCD(UINT8 regNum)
{
	UINT8 buff = 0;
	DS1302WriteEnable(1);

	DS1302Start();
	DS1302ByteWrite(ds1302RegAddr[regNum] + 1);	//写入读取寄存器地址
	DS1302ByteRead(&buff);
	DS1302End();

	DS1302WriteEnable(0);
	return buff;
}
/*
 **********************************************************
 *公共基础函数 End
 **********************************************************
 */


/*
 **********************************************************
 *时钟寄存器读写函数 Begin
 **********************************************************
 */
/**
 * DS1302寄存器单字节读取
 * @param  regNum UINT8,寄存器地址序号，为DS1302RegAddr[]数组下标
 * @return        UINT8,读出的单字节数据。
 *                对于RTC寄存器，则转换为十进制再返回。
 *                对于秒寄存器，需注意_bool7是否为1。
 */
UINT8 DS1302SingleRead(UINT8 regNum)
{
	UINT8 buff = 0;
	buff = DS1302SingleReadBCD(regNum);

	if (regNum <= 6) BCD2Dec(&buff); //对于RTC寄存器，读出为BCD码，转换为十进制

	return buff;
}

/**
 * DS1302寄存器单字节写入,十进制
 * @param  regNum UINT8,寄存器地址序号，为DS1302RegAddr[]数组下标
 * @param dat    UINT8,要写入的数据,十进制
 */
void DS1302SingleWrite(UINT8 regNum, UINT8 dat)
{
	if (regNum <= 6) Dec2BCD(&dat);	//对于RTC寄存器，写入需为BCD码，先转换为BCD
	DS1302SingleWriteBCD(regNum, dat);
}


/**
 * 突发方式读取DS1302时钟数据
 * @param rtc DS1302RTC union,返回数据保存指针，读取到的数据为十进制
 */
void DS1302GetRTC(DS1302RTC *rtc)
{
	DS1302GetRTCArray(rtc->RTCArray);
}

/**
 * 突发方式写入DS1302时钟数据
 * @param rtc DS1302RTC union,待写入数据保存指针，时钟数据为十进制
 */
void DS1302SetRTC(DS1302RTC *rtc)
{
	DS1302SetRTCArray(rtc->RTCArray);
	// UINT8 i;
	// DS1302WriteEnable(1);

	// DS1302Start();
	// DS1302ByteWrite(DS1302_BURST_WRITE_CLOCK_ADDR);	//突发模式时钟写入
	// for (i = 0; i < DS1302_RTC_LENGTH; i++)
	// {
	// 	Dec2BCD(&(rtc->RTCArray[i]));
	// 	DS1302ByteWrite(rtc->RTCArray[i]);
	// }
	// DS1302End();

	// DS1302WriteEnable(0);
}

/**
 * 突发方式写入DS1302时钟数据
 * @param rtcArray 待写入的数组指针，时钟数据为十进制，数组长度7。
 */
void DS1302SetRTCArray(UINT8 *rtcArray)
{
	UINT8 i;
	DS1302WriteEnable(1);

	DS1302Start();
	DS1302ByteWrite(DS1302_BURST_WRITE_CLOCK_ADDR);	//突发模式时钟写入
	for (i = 0; i < DS1302_RTC_LENGTH; i++)
	{
		Dec2BCD(&(rtcArray[i]));
		DS1302ByteWrite(rtcArray[i]);
	}
	DS1302ByteWrite(0x80);	//打开写保护。写入第8个字节。突发模式必须一次性写入8字节。凑数
	DS1302End();
}

/**
 * 突发方式读出DS1302时钟数据
 * @param rtcArray 保存读出数据的数组指针，时钟数据为十进制，数组长度7。
 */
void DS1302GetRTCArray(UINT8 *rtcArray)
{
	UINT8 i;
	DS1302WriteEnable(1);

	DS1302Start();
	DS1302ByteWrite(DS1302_BURST_WRITE_CLOCK_ADDR + 1);	//突发模式时钟读取
	for (i = 0; i < DS1302_RTC_LENGTH; i++)
	{
		DS1302ByteRead(&(rtcArray[i]));
		BCD2Dec(&(rtcArray[i]));
	}
	DS1302ByteRead(&i);	//读出第8个字节，凑数。否则读出的数据可能出错
	DS1302End();
	rtcArray[0] = rtcArray[0] & 0x7F;
	DS1302WriteEnable(0);
}
/*
 **********************************************************
 *时钟寄存器读写函数 End
 **********************************************************
 */


/*
 **********************************************************
 *RAM读写函数 Begin
 **********************************************************
 */
/**
 * DS1302 RAM单字节读取
 * @param  ramNum UINT8,范围0-31，需要读取的内存字节序号
 * @return        UINT8,读取到的RAM数据，十六进制
 */
UINT8 DS1302RAMSingleRead(UINT8 ramNum)
{
	UINT8 buff = 0;
	DS1302WriteEnable(1);

	DS1302Start();
	DS1302ByteWrite(DS1302_RAM_START_ADDR + ramNum * 2 + 1);	//写入读取寄存器地址
	DS1302ByteRead(&buff);
	DS1302End();

	DS1302WriteEnable(0);
	return buff;
}

/**
 * DS1302 RAM单字节写入
 * @param ramNum UINT8,范围0-31，需要写入的内存字节序号
 * @param dat    UINT8,待写入到RAM的数据，十六进制
 */
void DS1302RAMSingleWrite(UINT8 ramNum, UINT8 dat)
{
	DS1302WriteEnable(1);

	DS1302Start();
	DS1302ByteWrite(DS1302_RAM_START_ADDR + ramNum * 2);
	DS1302ByteWrite(dat);
	DS1302End();

	DS1302WriteEnable(0);
}

/**
 * DS1302 RAM突发模式连续读取31字节
 * @param buff UINT8*,保存读取数据的数组地址
 */
void DS1302RAMBurstRead(UINT8* buff)
{
	UINT8 i;
	DS1302WriteEnable(1);
	DS1302Start();
	DS1302ByteWrite(DS1302_BURST_WRITE_RAM_ADDR + 1);	//突发模式RAM读取
	for (i = 0; i < DS1302_MAX_RAM_BYTES; i++)
	{
		DS1302ByteRead(buff + i);
	}
	DS1302End();
	DS1302WriteEnable(0);
}

/**
 * DS1302 RAM突发模式连续写入31字节
 * @param buff UINT8*,要写入的数组地址
 */
void DS1302RAMBurstWrite(UINT8* buff)
{
	UINT8 i;
	DS1302WriteEnable(1);
	DS1302Start();
	DS1302ByteWrite(DS1302_BURST_WRITE_RAM_ADDR);	//突发模式RAM写入
	for (i = 0; i < DS1302_MAX_RAM_BYTES; i++)
	{
		DS1302ByteWrite(buff[i]);
	}
	DS1302End();
	DS1302WriteEnable(0);
}
/*
 **********************************************************
 *RAM读写函数 End
 **********************************************************
 */

/*
 **********************************************************
 *DS1302控制函数 End
 **********************************************************
 */
/**
 * DS1302写保护控制
 * @param enable 1——允许写入；0——禁止写入。
 */
void DS1302WriteEnable(BOOL enable)
{
	DS1302Start();
	Delay1us();
	DS1302ByteWrite(ds1302RegAddr[7]);
	DS1302ByteWrite(enable ? 0x00 : 0x80);
	DS1302End();
}

/**
 * DS1302涓流充电控制
 * @param enable   充电使能控制,1——使能充电；0——关闭充电。
 * @param twoDiode 是否使用两个二极管。1——两个二极管;0——一个二极管。
 * @param resistor 电阻编号1、2、3。1——2KΩ电阻,2——4KΩ电阻,3——8KΩ电阻。
 */
void DS1302TrickleChargeController(BOOL enable, BOOL twoDiode, UINT8 resistor)
{
	UINT8 charger = 0x00;
	if (enable)
	{
		charger = (0xA0 | (((UINT8)twoDiode + 1) << 2) | resistor);
	}
	DS1302SingleWrite(8, charger);
}

/**
 * DS1302 时钟是否在计时
 * @return BOOL，1——正在计时；0——未在计时
 */
BOOL  DS1302IsHalt()
{
	UINT8 dat = DS1302SingleRead(0);
	return (dat & 0x80);	//最高位为0则表示在计时
}

/**
 * DS1302 计时停止控制
 * @param enable BOOL, 1——停止计时;0——使能计时
 */
void DS1302ClockHalt(BOOL enable)
{
	UINT8 dat = DS1302SingleReadBCD(0);	//BCD码

	//dat &= 0x7F;
	dat = (dat & 0x7F) | ((UINT8)enable << 7);
	DS1302SingleWriteBCD(0, dat);	//最高位置0后,然后或上enable位
}

/*
 **********************************************************
 *DS1302控制函数 End
 **********************************************************
 */
