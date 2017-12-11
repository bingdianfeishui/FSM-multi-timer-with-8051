#ifndef __COMMON_H__
#define __COMMON_H__
#include <intrins.h>

#if !(defined(__REG51_H__)||defined(__REG52_H__))
#if defined(STC12)
#include <STC12C5A60S2.H>
#elif defined(STC15)
#include <STC15.H>
#else
#include <STC90C5xAD.H>
//#include "reg52.h"
#endif
#endif

typedef bit 			BOOL;
typedef char 			INT8;
typedef unsigned char 	UINT8;
typedef unsigned char 	BYTE;
typedef int 			INT16;
typedef unsigned int 	UINT16;
typedef unsigned int 	WORD;
typedef long 			INT32;
typedef unsigned long 	UINT32;
typedef float			FLOAT32;

#define TRUE 1
#define true 1
#define FALSE 0
#define false 0

#endif
