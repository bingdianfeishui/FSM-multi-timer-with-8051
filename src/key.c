#include "key.h"

sbit K1 = P2 ^ 0;
sbit K2 = P2 ^ 1;

KeyMessage ReadKey2(UINT8* key)
{
	KeyMessage  keyMsg = msgKeyNone;
	*key = 0;
	if (K1 == 0)
	{
		*key += (UINT8)msgKeyFunc;
		keyMsg = msgKeyFunc;
	}
	if (K2 == 0)
	{
		*key += (UINT8)msgKeySet;
		keyMsg = (keyMsg == msgKeyNone) ? msgKeySet : msgKeyNone;
	}
	return keyMsg;
}

KeyMessage ReadKey()
{
	KeyMessage  keyMsg = msgKeyNone;

	if (K1 == 0)
	{
		keyMsg = msgKeyFunc;
	}
	if (K2 == 0)
	{
		keyMsg = (keyMsg == msgKeyNone) ? msgKeySet : msgKeyNone;
	}
	return keyMsg;
}
