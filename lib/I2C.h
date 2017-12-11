#ifndef __I2C_H__
#define __I2C_H__

#include "common.h"
#include "Delay.h"

typedef struct
{
	UINT16 Length;
	UINT8* DataArray;
} I2CDataBuff;	//连续写入或读取时承载数据的数据缓存结构体

BOOL I2CIsBusy();
BOOL I2CDevSend1Byte(UINT8 devAddr, UINT8 dataAddr, UINT8 dat);
UINT8 I2CDevRead1Byte(UINT8 devAddr, UINT8 dataAddr);
BOOL I2CDevSendNBytes(UINT8 devAddr, UINT8 beginAddr, I2CDataBuff *dataBuff);
BOOL I2CDevSendNBytes1By1(UINT8 devAddr, UINT8 beginAddr, I2CDataBuff *dataBuff);
I2CDataBuff* I2CDevReadNBytes(UINT8 devAddr, UINT8 beginAddr, UINT8 length);

#endif
