/*
 * Http.cpp
 *
 *  Created on: May 3, 2016
 *      Author: tsnow
 */
#include "driver/Stream.h"
#include "driver/Process.h"
#include "driver/Http.h"
#include "driver/utils.h"


ICACHE_FLASH_ATTR HttpServerSocket::HttpServerSocket() : Socket()
	{
		m_CloseOnSent = false;
		m_request = NULL;
		m_remaining = 0;
	}

ICACHE_FLASH_ATTR HttpServerSocket::HttpServerSocket(espconn *conn) : Socket(conn)
	{
		m_CloseOnSent = false;
		m_request = NULL;
		m_remaining = 0;
	}

char ICACHE_FLASH_ATTR parseHex(char *p)
	{
		char ret = 0;
		char x = *p;
		x -= '0';
		if(x > 9)
			x -= 7;
		ret = (x & 0x0f) << 4;
		x = *(p+1);
		x -= '0';
		if(x > 9)
			x -= 7;
		ret |= x & 0x0f;

		return ret;
	}

LOCAL ICACHE_FLASH_ATTR void decode(char *buf)
	{
		while(*buf)
			{
				if(*buf == '%')
					{
						*buf = parseHex(buf+1);
						os_memcpy(buf+1,buf+3,strlen(buf+3)+1);
					}
				buf++;
			}
	}

LOCAL ICACHE_FLASH_ATTR char *ParseLine(char *buf)
	{
		char *ret = buf;
		while(*ret != 0 && *ret != '\r' && *ret != '\n')
			{
				ret++;
			}

		if(*ret == 0)
			return NULL;

		int cnt = 2;
		while(cnt > 0 && (*ret == '\r' || *ret == '\n'))
			{
				*ret = 0;
				ret++;
				cnt--;
			}

		if(*ret == 0)
			return NULL;

		return ret;
	}

void ICACHE_FLASH_ATTR HttpServerSocket::ProcessPage(char *page)
	{
		char *pHead;
		char *pNext;
		bool isPost = false;
		HttpParameters parms[MAX_PARMS];
		int parmcnt = 0;
		char empty[1];

		empty[0] = 0;

		os_printf("Http Request:\r\n%s\r\n",page);

		pHead = page;
		pNext = ParseLine(pHead);
		int ln = 0;
		os_printf("Line %d: '%s'\r\n",ln,pHead);

		if(strstr(pHead,"POST") != NULL)
			isPost = true;

		if(isPost)
			page = pHead + 5;
		else
			page = pHead + 4;

		char *p = strstr(page," ");
		if(p)
			*p = 0;

		os_printf("Page is %s\r\n",page);

		p = strstr(page,"?");
		// if there's a ?, this is a GET
		if(p)
			{
				*p = 0;
				p++;
				while(*p)
					{
						parms[parmcnt].parameter = p;
						parms[parmcnt].value = empty;
						char *psep = strstr(p,"=");
						if(psep)
							{
								*psep=0;
								psep++;
								parms[parmcnt].value = psep;
								p = psep;
							}
						psep = strstr(p,"&");
						if(psep)
							{
								*psep = 0;
								p = psep + 1;
							}
						else
							p += strlen(p);
						parmcnt++;
					}

				OnPage(page,parms,parmcnt);
				return;
			}
		else
			{
			// check POST - find empty line
			pHead = pNext;
			pNext = ParseLine(pNext);
			while(strlen(pHead) > 0 && pNext != NULL)
				{
					pHead = pNext;
					pNext = ParseLine(pHead);
					os_printf("Line %d: '%s'\r\n",ln,pHead);
					ln++;
				}

			os_printf("found break\r\n");
			while(pNext != NULL)
				{
					pHead = pNext;
					pNext = ParseLine(pHead);
					os_printf("Line %d: '%s'\r\n",ln,pHead);
					ln++;

					if(strlen(pHead))
						{
						p = pHead;
						while(*p)
							{
								parms[parmcnt].parameter = p;
								parms[parmcnt].value = empty;
								char *psep = strstr(p,"=");
								if(psep)
									{
										*psep=0;
										psep++;
										parms[parmcnt].value = psep;
										p = psep;
									}
								psep = strstr(p,"&");
								if(psep)
									{
										*psep = 0;
										p = psep + 1;
									}
								else
									p += strlen(p);
								parmcnt++;
							}
						}
				}
			}

		for(int i=0;i<parmcnt;i++)
			{
				decode(parms[i].parameter);
				decode(parms[i].value);
			}

		OnPage(page,parms,parmcnt);
	}

void ICACHE_FLASH_ATTR HttpServerSocket::OnReceive(uint8_t *data,unsigned short len)
	{
		if(m_request != NULL)
			{
				os_printf("OnReceive: m_request is not null\r\n");
				size_t rlen = os_strlen(m_request);
				char *buf = new char[rlen + len + 1];
				os_memset(buf,0,rlen + len + 1);
				os_memcpy(buf,m_request,rlen);
				os_memcpy(buf+rlen,data,len);
				delete [] m_request;
				m_request = buf;
			}
		else
			{
				os_printf("OnReceive: m_request is null\r\n");
				char *buf = new char[len + 1];
				os_memset(buf,0,len + 1);
				os_memcpy(buf,data,len);
				m_request = buf;
			}

		size_t cl = 0;
		os_printf("OnRecieve m_request:\r\n%s\r\n",m_request);

		char *p = strstri(m_request,"Content-Length: ");
		if(p)
			cl = atoi(p+16);

		p = os_strstr(m_request,"\r\n\r\n");
		if(p != NULL)
			{
				if((os_strlen(p) - 4) >= cl)
					{
						ProcessPage(m_request);
						delete [] m_request;
						m_request = NULL;
					}
			}
		else
			os_printf("No crlfcrlf\r\n");

	}

void ICACHE_FLASH_ATTR HttpServerSocket::OnSent()
	{
		if(m_CloseOnSent == true)
			Close();
	}

void ICACHE_FLASH_ATTR HttpServerSocket::OnPage(const char *page,HttpParameters *paramarray,int params)
	{
		os_printf("Page '%s' requested\r\n",page);
		for(int i=0;i<params;i++)
			os_printf("Parameter %d: '%s' = '%s'\r\n",i+1,paramarray[i].parameter,paramarray[i].value);
	}

void ICACHE_FLASH_ATTR HttpServerSocket::CloseOnSent(bool willClose)
	{
		m_CloseOnSent = willClose;
	}

