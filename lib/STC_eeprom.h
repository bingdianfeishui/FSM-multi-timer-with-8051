#ifndef __STC_EEPROM_H__
#define __STC_EEPROM_H__

#include "common.h"
#include "Delay.h"

#if defined(__REG51_H__)||defined(__REG52_H__)
sfr ISP_DATA = 0xe2; //寄存器申明
sfr ISP_ADDRH = 0xe3;
sfr ISP_ADDRL = 0xe4;
sfr ISP_CMD = 0xe5;
sfr ISP_TRIG = 0xe6;
sfr ISP_CONTR = 0xe7;
#endif

/* STC90C52RD+的flash空间从0x2000~0xf3ff 共10个扇区，每扇区512字节	*/
#define BaseAddr		0x2000
#define EndAddr	 		0xf3ff
/* ------------- 定义扇区大小 ------------- */
#define BytesPerSector		512


UINT8 ISPReadByte(UINT16 byte_addr);
void ISPWriteByte(UINT16 byte_addr, UINT8 original_data);
void ISPSectorErase(UINT16 sector_addr);
UINT8 ISPWriteByteVerify(UINT16 byte_addr, UINT8 original_data);
UINT8 ISPArrayWrite(UINT16 begin_addr, UINT16 len, UINT8 *array);
void ISPArrayRead(UINT16 begin_addr, UINT8 len, UINT8 *array);

#endif
