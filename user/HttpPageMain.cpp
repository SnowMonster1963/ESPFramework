/*
 * MainHttpPage.cpp
 *
 *  Created on: May 14, 2016
 *      Author: tsnow
 */

#include <ATMqtt.h>
#include <CmdProcessor.h>
#include <WiFiManager.h>
#include <driver/utils.h>
#include "MainHttp.h"

const char *mainPage = "HTTP/1.1 200 OK\r\n"
		"Server: Apache-Coyote/1.1\r\n"
		"Accept-Ranges: bytes\r\n"
		"ETag: W/\"902-1462299216733\"\r\n"
		"Last-Modified: Tue, 03 May 2016 18:13:36 GMT\r\n"
		"Content-Type: text/html\r\n"
		"Content-Length: $LENGTH$\r\n"
		"Date: Sat, 14 May 2016 10:12:45 GMT\r\n"
		"\r\n"
		"<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\" \"http://www.w3.org/TR/html4/loose.dtd\">\r\n"
		"<html>\r\n"
		"\r\n"
		"<link rel=\"stylesheet\" type=\"text/css\" href=\"css.css\" />\r\n"
		"\r\n"
		"\r\n"
		"<head>\r\n"
		"<meta http-equiv=\"Content-Type\" content=\"text/html; charset=ISO-8859-1\">\r\n"
		"<title>Update Your Device</title>\r\n"
		"</head>\r\n"
		"<head>\r\n"
		"<title>Device Setup - Main Menu</title>\r\n"
		"</head>\r\n"
		"<body>\r\n"
		"<h1>Main Menu</h1>\r\n"
		"<div class=\"menu\">\r\n"
		"<form action=\"access.html\" method=\"post\">\r\n"
		"<div class=\"button\">\r\n"
		"<input type=\"submit\" value=\"Set Access Point\"></input>\r\n"
		"</div>\r\n"
		"</form>\r\n"
		"<form action=\"wifi.html\" method=\"post\">\r\n"
		"<div class=\"button\">\r\n"
		"<input type=\"submit\" value=\"Connect to WiFi Network\"></input>\r\n"
		"</div>\r\n"
		"</form>\r\n"
		"<form action=\"mqtt.html\" method=\"post\">\r\n"
		"<div class=\"button\">\r\n"
		"<input type=\"submit\" value=\"Update Device Details\"></input>\r\n"
		"</div>\r\n"
		"</form>\r\n"
		"</div>\r\n"
		"</body>\r\n"
		"</html>";

void ICACHE_FLASH_ATTR MainHttpServerSocket::DoMainPage(const char *page, HttpParameters *paramarray, int params)
	{
		char buf[os_strlen(mainPage)+256];
		os_strcpy(buf,mainPage);
		SetContentLength(buf);
		CloseOnSent();
		Send(buf);
	}




