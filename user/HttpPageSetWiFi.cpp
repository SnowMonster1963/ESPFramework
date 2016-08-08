/*
 * HttpPageSetWiFi.cpp
 *
 *  Created on: May 18, 2016
 *      Author: tsnow
 */

#include <ATMqtt.h>
#include <CmdProcessor.h>
#include <WiFiManager.h>
#include <driver/utils.h>
#include "MainHttp.h"


const char *set_wifiPage = "HTTP/1.1 200 OK\r\n"
		"Server: Apache-Coyote/1.1\r\n"
		"Accept-Ranges: bytes\r\n"
		"ETag: W/\"579-1463569690604\"\r\n"
		"Last-Modified: Wed, 18 May 2016 11:08:10 GMT\r\n"
		"Content-Type: text/html\r\n"
		"Content-Length: $LENGTH$\r\n"
		"Date: Wed, 18 May 2016 11:11:32 GMT\r\n"
		"\r\n"
		"<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\" \"http://www.w3.org/TR/html4/loose.dtd\">\r\n"
		"<html>\r\n"
		"<link rel=\"stylesheet\" type=\"text/css\" href=\"css.css\" />\r\n"
		"<head>\r\n"
		"<meta http-equiv=\"Content-Type\" content=\"text/html; charset=ISO-8859-1\">\r\n"
		"<title>WiFi Connection Updated</title>\r\n"
		"</head>\r\n"
		"<body>\r\n"
		"\r\n"
			"<div class=\"wifi\">\r\n"
				"<h2>Wifi Connection Updated</h2>\r\n"
				"<form action=\"/\" method=\"post\">\r\n"
					"<div>\r\n"
						"You are now connected to $WIFI$\r\n"
					"</div>\r\n"
					"<div>\r\n"
						"<input type=\"submit\" value=\"Back to Main Menu\"></input>\r\n"
					"</div>\r\n"
				"</form>\r\n"
			"</div>\r\n"
		"</body>\r\n"
		"</html>";

const char *set_wifiErrorPage = "HTTP/1.1 200 OK\r\n"
		"Server: Apache-Coyote/1.1\r\n"
		"Accept-Ranges: bytes\r\n"
		"ETag: W/\"579-1463569690604\"\r\n"
		"Last-Modified: Wed, 18 May 2016 11:08:10 GMT\r\n"
		"Content-Type: text/html\r\n"
		"Content-Length: $LENGTH$\r\n"
		"Date: Wed, 18 May 2016 11:11:32 GMT\r\n"
		"\r\n"
		"<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\" \"http://www.w3.org/TR/html4/loose.dtd\">\r\n"
		"<html>\r\n"
		"<link rel=\"stylesheet\" type=\"text/css\" href=\"css.css\" />\r\n"
		"<head>\r\n"
		"<meta http-equiv=\"Content-Type\" content=\"text/html; charset=ISO-8859-1\">\r\n"
		"<title>WiFi Connection Update Failed</title>\r\n"
		"</head>\r\n"
		"<body>\r\n"
		"\r\n"
			"<div class=\"wifi\">\r\n"
				"<h2>Wifi Connection Update Failed</h2>\r\n"
				"<form action=\"/wifi.html\" method=\"post\">\r\n"
					"<div>\r\n"
						"Unable to connect to $WIFI$ - check the password\r\n"
					"</div>\r\n"
					"<div>\r\n"
						"<input type=\"submit\" value=\"Back Connect to WiFi Network\"></input>\r\n"
					"</div>\r\n"
				"</form>\r\n"
				"<form action=\"/\" method=\"post\">\r\n"
					"<div>\r\n"
						"<input type=\"submit\" value=\"Back to Main Menu\"></input>\r\n"
					"</div>\r\n"
				"</form>\r\n"
			"</div>\r\n"
		"</body>\r\n"
		"</html>";

void ICACHE_FLASH_ATTR MainHttpServerSocket::DoSetWiFiPage(const char *page, HttpParameters *paramarray, int params)
	{

		char *ssid;
		char *password;

		for(int i=0;i<params;i++)
			{
				if(os_strcmp(paramarray[i].parameter,"sel") == 0)
					{
						ssid = paramarray[i].value;
					}
				if(os_strcmp(paramarray[i].parameter,"pw") == 0)
					{
						password = paramarray[i].value;
					}
			}

		bool b = wifi.Connect(ssid,password);
		if(b)
			{
				char buf[os_strlen(set_wifiPage) + 256];
				os_strcpy(buf,set_wifiPage);
				str_replace(buf,"$WIFI$",ssid);
				SetContentLength(buf);
				CloseOnSent();
				Send(buf);
			}
		else
			{
				char buf[os_strlen(set_wifiErrorPage) + 256];
				os_strcpy(buf,set_wifiPage);
				str_replace(buf,"$WIFI$",ssid);
				SetContentLength(buf);
				CloseOnSent();
				Send(buf);
			}
	}












