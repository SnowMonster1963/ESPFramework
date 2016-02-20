/*
 * Debug.cpp
 *
 *  Created on: Dec 30, 2015
 *      Author: tsnow
 */
#include "driver/Debug.h"

void ByteToNybble(byte b, char *buf)
{
	byte x = b & 0xf;
	char c = '0' + x;
	if (x > 9)
		c += 7;
	buf[0] = c;
	buf[1] = 0;
}

void ByteToHex(byte b, char *buf)
{
	ByteToNybble(b >> 4, buf);
	ByteToNybble(b, buf + 1);
}

void WordToHex(uint16_t x, char *buf)
{
	byte b = x >> 8;
	ByteToHex(b, buf);
	b = x & 0xff;
	ByteToHex(b, buf + 2);
}

void dumpData(const byte *p, size_t len)
{
	size_t i;
	char hex[80];
	char chars[20];
	for (i = 0; i < len; i++)
	{
		if((i%16) == 0)
		{
			WordToHex(i,hex);
			strcat(hex,": ");
			memset(chars,0,sizeof(chars));
		}

		char bytechars[4];
		ByteToHex(p[i],bytechars);
		strcat(hex,bytechars);
		if(p[i] >= 0x20 && p[i] < 0x7f)
			bytechars[0] = p[i];
		else
			bytechars[0] = '.';
		bytechars[1] = 0;
		strcat(chars,bytechars);
		strcat(hex," ");

		if(((i+1)%4) == 0)
		{
			strcat(hex," ");
		}

		if(((i+1) % 16) == 0)
			ets_uart_printf("%s /*%s*/\r\n",hex,chars);
	}
	if((i%16) != 0)
	{
		while((i%16) != 0)
		{
			strcat(hex,"   ");
			//strcat(chars," ");
			if(((i+1)%4) == 0)
				strcat(hex," ");
			i++;
		}
		ets_uart_printf("%s /*%s*/\r\n",hex,chars);
	}
}
