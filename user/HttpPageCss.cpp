/*
 * CssHttpPage.cpp
 *
 *  Created on: May 14, 2016
 *      Author: tsnow
 */

#include <ATMqtt.h>
#include <CmdProcessor.h>
#include <WiFiManager.h>
#include <driver/utils.h>
#include "MainHttp.h"

const char *cssPage = "HTTP/1.1 200 OK\r\n"
		"Server: Apache-Coyote/1.1\r\n"
		"Accept-Ranges: bytes\r\n"
		"ETag: W/\"849-1462299259813\"\r\n"
		"Last-Modified: Tue, 03 May 2016 18:14:19 GMT\r\n"
		"Content-Type: text/css\r\n"
		"Content-Length: $LENGTH$\r\n"
		"\r\n"
		"body {\r\n"
		"background-color: linen;\r\n"
		"font-family: Arial;\r\n"
		"}\r\n"
		"\r\n"
		"table {\r\n"
		"margin: 10px;\r\n"
		"padding: 10px;\r\n"
		"border: 1px solid black;\r\n"
		"padding: 10px;\r\n"
		"}\r\n"
		"\r\n"
		"thead {\r\n"
		"font-weight: bold;\r\n"
		"border: 1px solid black;\r\n"
		"}\r\n"
		"\r\n"
		"td {\r\n"
		"margin: 10px;\r\n"
		"padding: 10px;\r\n"
		"}\r\n"
		"\r\n"
		"div {\r\n"
		"margin: 10px;\r\n"
		"padding: 10px;\r\n"
		"float: left;\r\n"
		"background-color: linen;\r\n"
		"font-family: Arial;\r\n"
		"}\r\n"
		"\r\n"
		"div.wifi {\r\n"
		"width: 400px;\r\n"
		"border: 1px solid black;\r\n"
		"clear: left;\r\n"
		"height: 500px;\r\n"
		"}\r\n"
		"\r\n"
		"div.menu {\r\n"
		"width: 200px;\r\n"
		"height: 150px;\r\n"
		"clear: left;\r\n"
		"border: 1px solid black;\r\n"
		"align: center;\r\n"
		"}\r\n"
		"\r\n"
		"div.ap {\r\n"
		"border: 1px solid black;\r\n"
		"width: 1100px;\r\n"
		"}\r\n"
		"\r\n"
		"div.mqtt {\r\n"
		"border: 1px solid black;\r\n"
		"width: 660px;\r\n"
		"height: 500px;\r\n"
		"}\r\n"
		"\r\n"
		"div.button {\r\n"
		"width: 100px;\r\n"
		"height: 0px;\r\n"
		"}\r\n"
		"\r\n"
		"div {\r\n"
		"margin: 10px;\r\n"
		"padding: 10px;\r\n"
		"}\r\n"
		"\r\n"
		"div.button {\r\n"
		"display: block;\r\n"
		"width: 75%;\r\n"
		"}\r\n";


void SetContentLength(char *buf)
	{
		int contentlen = 0;

		char *p = os_strstr(buf,"\r\n\r\n");
		if(p != NULL)
			{
				contentlen = os_strlen(p)-4;
			}

		char szlen[16];
		os_sprintf(szlen,"%d",contentlen);
		str_replace(buf,"$LENGTH$",szlen);

	}

void ICACHE_FLASH_ATTR MainHttpServerSocket::DoCssPage(const char *page, HttpParameters *paramarray, int params)
	{
		char buf[os_strlen(cssPage)+256];
		os_strcpy(buf,cssPage);

		SetContentLength(buf);
		CloseOnSent();
		Send(buf);
	}




