#ifndef __KEY_H__
#define __KEY_H__

#include <common.h>


// #define KEY_IDLE		0x00
// #define KEY_DOWN		0x01
// #define KEY_TIME		0x02
// #define KEY_LONGPRESS	0x04

#define KEY_LONG_TIME	150 	//150*10ms, 1.5s
#define KEY_CONTINUE_ACT 20 	//20*10ms, 0.2s

typedef enum {
	fsmKeyIdle			= 0x00,
	fsmKeyDown			= 0x01,
	fsmKeyTime			= 0x02,
	fsmKeyLongPress		= 0x04
} fsmKeyState;

typedef enum {
	msgKeyNone 	= 0x00,
	msgKey1 	= 0x01,
	msgKey2 	= 0x02,
} KeyMessage;

KeyMessage ReadKey(UINT8* key);


#endif
