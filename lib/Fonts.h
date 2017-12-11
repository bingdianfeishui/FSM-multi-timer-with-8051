
#ifndef __FONTS_H__
#define __FONTS_H__

#include "Common.h"

typedef struct
{
	UINT8 Index[2];
	UINT8 Mask[32];
} CnFonts;

typedef struct
{
	UINT8 Index;
	UINT8 Mask[16];
} EnFonts;

extern UINT16 cnLen;
//取模方式： 纵向取模，字节倒序
extern CnFonts code cnFonts[];

extern UINT16 enLen;
//取模方式： 纵向取模，字节倒序
extern EnFonts code enFonts[];

#endif

