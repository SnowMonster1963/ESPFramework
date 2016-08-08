/*
 * HttpPageSetAccess.cpp
 *
 *  Created on: May 14, 2016
 *      Author: tsnow
 */
/*
 * HttpPageAccess.cpp
 *
 *  Created on: May 14, 2016
 *      Author: tsnow
 */

#include <ATMqtt.h>
#include <CmdProcessor.h>
#include <WiFiManager.h>
#include <driver/utils.h>
#include "MainHttp.h"


const char *set_accessPage = "HTTP/1.1 200 OK\r\n"
		"Server: Apache-Coyote/1.1\r\n"
		"Accept-Ranges: bytes\r\n"
		"ETag: W/\"730-1463232859767\"\r\n"
		"Last-Modified: Sat, 14 May 2016 13:34:19 GMT\r\n"
		"Content-Type: text/html\r\n"
		"Content-Length: $LENGTH$\r\n"
		"Date: Sat, 14 May 2016 13:35:40 GMT\r\n"
		"\r\n"
		"<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\" \"http://www.w3.org/TR/html4/loose.dtd\">\r\n"
		"<html>\r\n"
		"<link rel=\"stylesheet\" type=\"text/css\" href=\"css.css\" />\r\n"
		"<head>\r\n"
		"<meta http-equiv=\"Content-Type\" content=\"text/html; charset=ISO-8859-1\">\r\n"
		"<title>Access Point Set</title>\r\n"
		"</head>\r\n"
		"<body>\r\n"
			"<h1>Access Point Set</h1>\r\n"
			"<div class=\"ap\">\r\n"
				"<h2>Access Point Details</h2>\r\n"
				"<form action=\"/\" method=\"post\">\r\n"
					"<div>\r\n"
						"SSID: $SSID$\r\n"
					"</div>\r\n"
					"<div>\r\n"
						"Password: ********\r\n"
					"</div>\r\n"
					"<div>\r\n"
						"Channel: $CHANNEL$\r\n"
					"</div>\r\n"
					"<div>\r\n"
						"Encryption: $ENCRYPTION$\r\n"
					"</div>\r\n"
					"<div>\r\n"
						"<input type=\"submit\" value=\"Back to Main Menu\"></input>\r\n"
					"</div>\r\n"
				"</form>\r\n"
			"</div>\r\n"
		"</body>\r\n"
		"</html>";

const char *setErrorPage = "HTTP/1.1 200 OK\r\n"
		"Server: Apache-Coyote/1.1\r\n"
		"Accept-Ranges: bytes\r\n"
		"ETag: W/\"730-1463232859767\"\r\n"
		"Last-Modified: Sat, 14 May 2016 13:34:19 GMT\r\n"
		"Content-Type: text/html\r\n"
		"Content-Length: $LENGTH$\r\n"
		"Date: Sat, 14 May 2016 13:35:40 GMT\r\n"
		"\r\n"
		"<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\" \"http://www.w3.org/TR/html4/loose.dtd\">\r\n"
		"<html>\r\n"
		"<link rel=\"stylesheet\" type=\"text/css\" href=\"css.css\" />\r\n"
		"<head>\r\n"
		"<meta http-equiv=\"Content-Type\" content=\"text/html; charset=ISO-8859-1\">\r\n"
		"<title>Error!</title>\r\n"
		"</head>\r\n"
		"<body>\r\n"
			"<h1>Access Point Unchanged</h1>\r\n"
			"<div class=\"ap\">\r\n"
				"<h2>Unable to Change Access Point</h2>\r\n"
				"<form action=\"/\" method=\"post\">\r\n"
					"<div>\r\n"
						"<input type=\"submit\" value=\"Back to Main Menu\"></input>\r\n"
					"</div>\r\n"
				"</form>\r\n"
			"</div>\r\n"
		"</body>\r\n"
		"</html>";

const char *enc[] = {
		"None",
		"WEP",
		"WPA_PSK",
		"WPA2_PSK",
		"WPA_WPA2_PSK"
};

void ICACHE_FLASH_ATTR MainHttpServerSocket::DoSetAccessPage(const char *page, HttpParameters *paramarray, int params)
	{
		struct softap_config config;

		os_memset(&config,0,sizeof(config));
		wifi_softap_get_config(&config);
		os_memset(&config.ssid,0,sizeof(config.ssid));
		os_memset(&config.password,0,sizeof(config.password));

		for(int i=0;i<params;i++)
			{
				if(os_strcmp(paramarray[i].parameter,"name") == 0)
					{
						size_t x = os_strlen(paramarray[i].value);
						os_memcpy(&config.ssid,paramarray[i].value,x);
						config.ssid_len = (uint8)x;
					}
				if(os_strcmp(paramarray[i].parameter,"pw") == 0)
					{
						os_memcpy(&config.password,paramarray[i].value,os_strlen(paramarray[i].value));
					}
				if(os_strcmp(paramarray[i].parameter,"ch") == 0)
					{
						config.channel = (uint8)atoi(paramarray[i].value);
					}
				if(os_strcmp(paramarray[i].parameter,"enc") == 0)
					{
						config.authmode = (AUTH_MODE)atoi(paramarray[i].value);
					}
			}

		bool b = wifi_softap_set_config(&config);
		if(b)
			{

			char buf[os_strlen(set_accessPage) + 256];
			os_strcpy(buf,set_accessPage);
			str_replace(buf,"$SSID$",(const char*)&config.ssid);
			char token[10];
			os_sprintf(token,"%d",(int)config.channel);
			str_replace(buf,"$CHANNEL$",token);
			str_replace(buf,"$ENCRYPTION$",enc[config.authmode]);
			SetContentLength(buf);
			CloseOnSent();
			Send(buf);
			}
		else
			{
				char buf[os_strlen(setErrorPage) + 256];
				SetContentLength(buf);
				CloseOnSent();
				Send(setErrorPage);
			}
	}








