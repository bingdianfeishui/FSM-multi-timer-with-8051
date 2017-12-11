#include "key.h"

sbit K1 = P2 ^ 0;
sbit K2 = P2 ^ 1;

KeyMessage ReadKey(UINT8* key)
{
	KeyMessage  keyMsg = msgKeyNone;
	*key = 0;
	if (K1 == 0)
	{
		*key += (UINT8)msgKey1;
		keyMsg = msgKey1;
	}
	if (K2 == 0)
	{
		*key += (UINT8)msgKey2;
		keyMsg = (keyMsg == msgKeyNone) ? msgKey2 : msgKeyNone;
	}
	return keyMsg;
}
