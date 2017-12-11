/**
 **********************************************************
 ******    Copyright(C), 2010-2016, NULL Co.,Ltd     ******
 **********************************************************

 *@Tittle		:	I2C设备通用驱动
 *@Version		:	v1.2
 *@Author		:	Liy
 *@Dat			:	2016-08-03 16:48:45
 *@Desctription	:
 *				功能说明：
 *					1、实现I2C设备启动、停止、应答、非应答、等待应答、单字节及多字节读取或写入功能。
 *					2、多字节写入分连续页写及单字节依次写入两种方式；页写受设备页缓存大小限制，
 *						但速度快；单字节写入不受设备缓存大小限制，但速度较页写慢。
 *					2、写入无应答时自动重复执行I2C_MAX_REPEAT_TIMES次。
 *
 *				注意事项：
 *					1、不同晶振及机器周期下，请自行更改Delay2us()延时。
 *					2、GPIO_I2C_SDA/SCL管脚请自行修改。
 *					3、连续读取时，最大数据长度为16(I2C_MAX_BUFF_LENGTH)。
 *						[已取消。数组定义为xdata，即在扩展RAM上，对于无扩展RAM的单片机请自行修改(比如去掉xdata）。]
 *					4、对AT24CXX设备进行多字节连续写入(页写)时，一次写入数据长度不要超过设备页长度：
 *						AT24C01			 = 8
 *						AT24c02/04/08/16 = 16。
 *					5、对AT24CXX设备进行多字节写入后，需延时1个写周期（一般最大为10ms），以
 *						让设备将页缓存中的数据写入ROM，此期间设备无应答，写入完成后才能进行下一次读或写。
 * 					6、 所有延时均为2us。
 *      				可通过调整延时时间来改变总线传输速率。4.5~5.5V下，最小2us。低压系统最小5us。
 *
 *@History		:
 *	#v1.2
 *		数据类型采用typedef,便于移植。延时采用外部通用Delay.h。
 *	#v1.1
 * 		增加多字节数据一个个依次写入的功能，写入数据不再受设备缓存大小限制。
 * 	#v1.0:
 * 		实现基本功能，包括启动、停止、应答、非应答、单字节读写、多字节读和页写。

 **********************************************************
 **********************************************************
 */


#include "I2C.h"

#define I2C_MAX_REPEAT_TIMES 3 		//定义从机无应答时最大重复发送次数
#define I2C_MAX_BUFF_LENGTH 16 		//定义最大连续读取字节长度，单位char
#define I2C_DEV_WRITE_PAGE_SIZE 16 	//定义设备连续页写时的缓存字节数，单位char

/**
 * 定义I2C时钟总线和数据总线端口
 */
sbit GPIO_I2C_SDA = P2 ^ 0;
sbit GPIO_I2C_SCL = P2 ^ 1;

/*
 **********************************************************
 *内部函数
 **********************************************************
 */
/**
 * I2C开始信号。
 */
static void I2CStart(void)
{
	GPIO_I2C_SDA = 1;
	GPIO_I2C_SCL = 1;
	Delay2us();
	GPIO_I2C_SDA = 0;
	Delay2us();
	GPIO_I2C_SCL = 0;	//拉低(钳住)时钟总线，准备下一步操作
	Delay2us();
}

/**
 * I2C停止信号
 */
static void I2CStop(void)
{
	GPIO_I2C_SDA = 0;
	GPIO_I2C_SCL = 1;
	Delay2us();
	GPIO_I2C_SDA = 1;	//释放SDA总线
	Delay2us();
}

/**
 * 主机向从机发送应答讯号，读数据时用。
 * @param ark 0——发送应答ACK信号；1——发送非应答NACK信号
 */
static void I2CSendAck(BOOL ark)
{
	GPIO_I2C_SCL = 0;
	Delay2us();
	GPIO_I2C_SDA = ark;
	Delay2us();
	GPIO_I2C_SCL = 1;
	Delay2us();
	GPIO_I2C_SCL = 0;
	Delay2us();
}

/**
 * 等待从机应答。
 * @return  BOOL, 0——从机应答ACK；1——从机非应答NACK
 */
static BOOL I2CWaitAck(void)
{
	UINT8 times = 0;
	GPIO_I2C_SDA = 1;
	GPIO_I2C_SCL = 1;
	Delay2us();
	while (GPIO_I2C_SDA == 1)
	{
		times++;
		if (times > 200)
		{
			I2CStop();
			return 1;
		}
	}
	GPIO_I2C_SCL = 0;	//拉低(钳住)时钟总线，准备下一步操作
	Delay2us();
	return 0;
}

/**
 * 从I2C总线读取1个字节
 * @return UINT8,读取到的数据
 */
static UINT8 I2CReadByte()
{
	UINT8 i, buff = 0;
	for (i = 0; i < 8; i++)
	{
		GPIO_I2C_SDA = 1;	//释放数据总线
		GPIO_I2C_SCL = 0;	//拉低时钟总线，等待从机改变SDA总线的值
		Delay2us();
		GPIO_I2C_SCL = 1;	//拉高时钟总线，准备读取SDA总线的值
		Delay2us();
		buff <<= 1;
		buff |= GPIO_I2C_SDA;
	}

	GPIO_I2C_SCL = 0;	//拉低(钳住)时钟总线，准备下一步操作
	Delay2us();
	return buff;
}

/**
 * 向I2C总线发送单个字节，从最高位开始发，并读取应答
 * @param  dat 要发送的8位数据
 * @return     BOOL,写入完成后，从机应答(ACK)返回0，从机非应答(NACK)或超时返回1
 */
static BOOL I2CSendByte(UINT8 dat)
{
	UINT8 i, tmp;
	for (i = 0; i < 8; i++)
	{
		GPIO_I2C_SCL = 0;
		if ((dat << i) & 0x80)	//发送单个位到SDA数据总线，从最高位开始发
			GPIO_I2C_SDA = 1;
		else
			GPIO_I2C_SDA = 0;

		Delay2us();
		GPIO_I2C_SCL = 1;
		Delay2us();
		GPIO_I2C_SCL = 0;
		Delay2us();
	}

	//检测从机应答
	GPIO_I2C_SDA = 1;
	Delay2us();
	GPIO_I2C_SCL = 1;
	while (GPIO_I2C_SDA == 1)
	{
		tmp++;
		if (tmp > 100)//等待超过100次循环认为超时，返回NACK
		{
			GPIO_I2C_SCL = 0;	//拉低(钳住)时钟总线，准备下一步操作
			return 1;
		}
	}

	GPIO_I2C_SCL = 0;	//拉低(钳住)时钟总线，准备下一步操作
	return 0;
}


/*
 **********************************************************
 *外部函数
 **********************************************************
 */
/**
 * 检测I2C总线是否被占用。SCL=0。
 * @return BOOL,0——空闲，1——占用
 */
BOOL I2CIsBusy()
{
	UINT8 i;
	UINT16 n = 10 * I2C_MAX_REPEAT_TIMES;

	for (i = 0; i < n; i++)	//多次重复延时检测SCL，提高准确性
	{
		if (~GPIO_I2C_SCL)	//若为1，则表示检测到SCL=0
		{
			Delay2us();
			if (~GPIO_I2C_SCL) return 1;	//延时再次检测，排除干扰。若还为1则返回总线忙。
		}
		Delay2us(); Delay2us();
	}
	return 0;
}

/**
 * 向I2C设备的指定位置写入1个字节的数据
 * @param devAddr  设备地址，如0xA0
 * @param dataAddr 要写入的数据地址
 * @param dat      要写入的8位数据
 * @return			写入状态，1——成功，0——失败
 */
BOOL I2CDevSend1Byte(UINT8 devAddr, UINT8 dataAddr, UINT8 dat)
{
	BOOL ack = 0;
	UINT8 repeat = 0;

	do
	{
		I2CStart();
		ack = I2CSendByte(devAddr);
	} while (ack && ++repeat <= I2C_MAX_REPEAT_TIMES);	//当从机返回非应答NACK时，重复发送
	if (repeat == I2C_MAX_REPEAT_TIMES) { I2CStop(); return 0;}	//超过最大重复次数则停止I2C总线并返回

	repeat = 0;
	do
	{
		ack = I2CSendByte(dataAddr);
	} while (ack && ++repeat <= I2C_MAX_REPEAT_TIMES);
	if (repeat == I2C_MAX_REPEAT_TIMES) { I2CStop(); return 0;}

	repeat = 0;
	do
	{
		ack = I2CSendByte(dat);
	} while (ack && ++repeat == I2C_MAX_REPEAT_TIMES);
	if (repeat == I2C_MAX_REPEAT_TIMES) { I2CStop(); return 0;}

	I2CStop();
	return 1;
}

/**
 * 从I2C设备中读取指定位置的单个字节的数据
 * @param  devAddr  要读取的I2C设备地址
 * @param  dataAddr 数据存储地址
 * @return          UINT8,读取到的单字节数据
 */
UINT8 I2CDevRead1Byte(UINT8 devAddr, UINT8 dataAddr)
{
	UINT8 dat, repeat = 0;
	BOOL ack = 0;

	do
	{
		I2CStart();
		ack = I2CSendByte(devAddr);
	} while (ack && ++repeat <= I2C_MAX_REPEAT_TIMES);	//当从机返回非应答NACK时，重复发送
	if (repeat == I2C_MAX_REPEAT_TIMES) { I2CStop(); return 0xFF;}	//超过最大重复次数则停止I2C总线并返回无效数据0xFF

	repeat = 0;
	do
	{
		ack = I2CSendByte(dataAddr);
	} while (ack && ++repeat <= I2C_MAX_REPEAT_TIMES);
	if (repeat == I2C_MAX_REPEAT_TIMES) { I2CStop(); return 0xFF;}

	repeat = 0;
	do
	{
		I2CStart();
		ack = I2CSendByte(devAddr | 0x01);	//R/W =1, 读取方向
	} while (ack && ++repeat <= I2C_MAX_REPEAT_TIMES);	//当从机返回非应答NACK时，重复发送
	if (repeat == I2C_MAX_REPEAT_TIMES) { I2CStop(); return 0xFF;}	//超过最大重复次数则停止I2C总线并返回

	dat = I2CReadByte();
	I2CSendAck(1);	//不再继续读取，发送NACK
	I2CStop();
	return dat;
}

/**
 * 向I2C设备的指定起始位置发送多字节数据，一次性连续写入(页写)。单词发送字节数不得超过I2C_DEV_WRITE_PAGE_SIZE。
 * 注意：
 * 		1、对于AT24CXX，发送的数据长度不得超过页长度，
 * 		否则地址指针将回到指定起始位置重新开始，导致数据被覆写。
 * 	 	2、对于AT24CXX，发送完成之后需延时一个写周期(一般为10ms)等待设备写入。
 * @param devAddr   设备地址
 * @param beginAddr 数据存储起始位置
 * @param *dataBuff 数据缓存指针
 * @return          BOOL,发送成功标识，1——成功,0——失败
 */
BOOL I2CDevSendNBytes(UINT8 devAddr, UINT8 beginAddr, I2CDataBuff *dataBuff)
{
	BOOL ack = 0;
	UINT8 i,  repeat = 0;

	if (dataBuff->Length > I2C_DEV_WRITE_PAGE_SIZE)	//检测单次写入数据长度
		dataBuff->Length = I2C_DEV_WRITE_PAGE_SIZE;

	do
	{
		I2CStart();
		ack = I2CSendByte(devAddr);
	} while (ack == 1 && ++repeat <= I2C_MAX_REPEAT_TIMES);
	if (repeat ==  I2C_MAX_REPEAT_TIMES) { I2CStop(); return 0;}

	repeat = 0;
	do
	{
		ack = I2CSendByte(beginAddr);
	} while (ack == 1 && ++repeat > I2C_MAX_REPEAT_TIMES);
	if (repeat > I2C_MAX_REPEAT_TIMES) { I2CStop(); return 0;}

	for (i = 0; i < (*dataBuff).Length; i++)
	{
		do
		{
			ack = I2CSendByte(*((*dataBuff).DataArray + i));
		} while (ack == 1 && ++repeat <= I2C_MAX_REPEAT_TIMES);
		if (repeat == I2C_MAX_REPEAT_TIMES) { I2CStop(); return 0;}
	}

	I2CStop();
	return 1;
}

/**
 * 向I2C设备的指定起始位置发送多字节数据，单个字节写入，重复多次。
 * 注意：
 * 		1、对于AT24CXX，发送的数据长度不得超过页长度，
 * 		否则地址指针将回到指定起始位置重新开始，导致数据被覆写。
 * 	 	2、对于AT24CXX，发送完成之后需延时等待设备写入。
 * @param devAddr   设备地址
 * @param beginAddr 数据存储起始位置
 * @param *dataBuff 数据缓存指针
 * @return          BOOL,发送成功标识，1——成功,0——失败
 */
BOOL I2CDevSendNBytes1By1(UINT8 devAddr, UINT8 beginAddr, I2CDataBuff *dataBuff)
{
	BOOL result = 0;
	UINT8 i, repeat = 0;
	for (i = 0; i < dataBuff->Length; i++)
	{
		do
		{
			result = I2CDevSend1Byte(devAddr, beginAddr + i, dataBuff->DataArray[i]);
		} while (result == 0 && ++repeat <= I2C_MAX_REPEAT_TIMES);
		if (repeat ==  I2C_MAX_REPEAT_TIMES) { return 0;}
	}
	return 1;
}

/**
 * 从I2C设备中指定位置连续读取多个字节数据，最长不要超过16字节(I2C_MAX_BUFF_LENGTH)。
 * @param  devAddr   设备地址
 * @param  beginAddr 数据起始位置
 * @param  length    要读取的字节数
 * @return           I2CDataBuff*，读出的数据缓存结构体指针
 */
I2CDataBuff* I2CDevReadNBytes(UINT8 devAddr, UINT8 beginAddr, UINT8 length)
{
	UINT8 /*xdata*/ tmpArray[I2C_MAX_BUFF_LENGTH];	//在扩展RAM上分配I2C_MAX_BUFF_LENGTH个字节的内存
	UINT8 i, repeat, tmp = 0;
	BOOL ack = 0;
	I2CDataBuff tmpBuff = {0, tmpArray};	//读入数据前初始化长度Length=0

	if (length > I2C_MAX_BUFF_LENGTH)	//检测单次读取数据长度
		length = I2C_MAX_BUFF_LENGTH;

	do
	{
		I2CStart();
		ack = I2CSendByte(devAddr);
	} while (ack == 1 && ++repeat <= I2C_MAX_REPEAT_TIMES);
	if (repeat == I2C_MAX_REPEAT_TIMES) { I2CStop(); return &tmpBuff;}

	repeat = 0;
	do
	{
		ack = I2CSendByte(beginAddr);
	} while (ack == 1 && ++repeat <= I2C_MAX_REPEAT_TIMES);
	if (repeat == I2C_MAX_REPEAT_TIMES) { I2CStop(); return &tmpBuff;}

	repeat = 0;
	do
	{
		I2CStart();
		ack = I2CSendByte(devAddr | 0x01);
	} while (ack == 1 && ++repeat <= I2C_MAX_REPEAT_TIMES);
	if (repeat == I2C_MAX_REPEAT_TIMES) { I2CStop(); return &tmpBuff;}

	for (i = 0; i < length; i++)
	{
		tmpArray[i] = I2CReadByte();
		I2CSendAck(!(i < length - 1));
		Delay2us();
	}

	tmpBuff.Length = length;

	I2CStop();

	return &tmpBuff;
}