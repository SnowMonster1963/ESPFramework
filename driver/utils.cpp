/*
 * utils.cpp
 *
 *  Created on: May 14, 2016
 *      Author: tsnow
 */

#include <ctype.h>
#include <driver/utils.h>

uint32_t ICACHE_FLASH_ATTR GetCheckSum(void *p,size_t len)
	{
		uint32_t ret = 0;
		byte *pb = (byte *)p;
		while(len)
			{
				ret <<= 1;
				ret += *pb;
				pb++;
				len--;
			}

		return ret;
	}


void ICACHE_FLASH_ATTR str_replace(char *buf,const char *from,const char *to)
	{
		size_t fromlen = os_strlen(from);
		size_t tolen = os_strlen(to);
		char *p;

		if(tolen == 0)  // we are just removing the 'from' string
			{
				p = os_strstr(buf,from);
				while(p != NULL)
					{
						os_memcpy(p,p+fromlen,os_strlen(p+fromlen)+1);
						p = os_strstr(p,from);
					}
				return;
			}
		if(fromlen > tolen) // simple case where the string you are replacing is longer than the new one
			{
				p = os_strstr(buf,from);
				while(p != NULL)
					{
						os_memcpy(p,to,tolen);
						os_memcpy(p+tolen,p+fromlen,os_strlen(p+fromlen)+1);
						p = os_strstr(p+tolen,from);
					}
				return;
			}
		if(fromlen == tolen) // simpler case where the string you are replacing is same length as the new one
			{
				p = os_strstr(buf,from);
				while(p != NULL)
					{
						os_memcpy(p,to,tolen);
						p = os_strstr(p+tolen,from);
					}
				return;
			}

		// fall through case where the string you are replacing is shorter than the new one
		p = os_strstr(buf,from);
		size_t diff = tolen - fromlen;
		while(p != NULL)
			{
				// need to make room
				size_t len = os_strlen(p+fromlen)+1;
				while(len > 0)
					{
						p[fromlen+len+diff] = p[fromlen+len];
						len--;
					}
				p[fromlen+len+diff] = p[fromlen+len];
				os_memcpy(p,to,tolen);
				p = os_strstr(p+tolen,from);
			}
		return;

	}

char * strstri(char *a,const char *b)
	{
		char c;

		while(*a)
			{
				c = toupper(*a);
				if(c == toupper(*b))
					{
						char *p = a;
						int idx = 1;
						bool m = true;
						while(m == true && b[idx] != 0)
							{
								if(toupper(p[idx]) != toupper(b[idx]))
									m = false;
								idx++;
							}
						if(m)
							return p;
					}
				a++;
			}

		return NULL;
	}
