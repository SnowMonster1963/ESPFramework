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


const char *accessPage = "HTTP/1.1 200 OK\r\n"
		"Server: Apache-Coyote/1.1\r\n"
		"Accept-Ranges: bytes\r\n"
		"ETag: W/\"1619-1462299902292\"\r\n"
		"Last-Modified: Tue, 03 May 2016 18:25:02 GMT\r\n"
		"Content-Type: text/html\r\n"
		"Content-Length: $LENGTH$\r\n"
		"\r\n"
		"<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\" \"http://www.w3.org/TR/html4/loose.dtd\">\r\n"
		"<html>\r\n"
		"<link rel=\"stylesheet\" type=\"text/css\" href=\"css.css\" />\r\n"
		"<head>\r\n"
		"<meta http-equiv=\"Content-Type\" content=\"text/html; charset=ISO-8859-1\">\r\n"
		"<title>Set Your Device's Access Point</title>\r\n"
		"</head>\r\n"
		"<body>\r\n"
		"<h1>Configure the Device's Access Point</h1>\r\n"
		"<div class=\"ap\">\r\n"
		"<h2>Update Your Device's Access Point Details</h2>\r\n"
		"<form action=\"/set_access.html\" method=\"post\">\r\n"
		"<div>\r\n"
		"SSID: <input type=\"text\" maxlength=\"32\" name=\"name\" value=\"$SSID$\"></input>\r\n"
		"</div>\r\n"
		"<div>\r\n"
		"Password:<input type=\"password\" maxlength=\"64\" name=\"pw\" value=\"$SSPW$\"></input>\r\n"
		"</div>\r\n"
		/*
		"<div>\r\n"
		"Channel: <select name=\"ch\">\r\n"
		"<option value=\"1\"$_selch1$>1</option>\r\n"
		"<option value=\"2\"$_selch2$>2</option>\r\n"
		"<option value=\"3\"$_selch3$>3</option>\r\n"
		"<option value=\"4\"$_selch4$>4</option>\r\n"
		"<option value=\"5\"$_selch5$>5</option>\r\n"
		"<option value=\"6\"$_selch6$>6</option>\r\n"
		"<option value=\"7\"$_selch7$>7</option>\r\n"
		"<option value=\"8\"$_selch8$>8</option>\r\n"
		"<option value=\"9\"$_selch9$>9</option>\r\n"
		"<option value=\"10\"$_selch10$>10</option>\r\n"
		"<option value=\"11\"$_selch11$>11</option>\r\n"
		"<option value=\"10\"$_selch12$>12</option>\r\n"
		"<option value=\"11\"$_selch13$>13</option>\r\n"
		"</select>\r\n"
		"</div>\r\n"
		*/
		"<div>\r\n"
		"Encryption <select name=\"enc\">\r\n"
		"<option value=\"0\"$_selenc0$>None</option>\r\n"
		//"<option value=\"1\"$_selenc1$>WEP</option>\r\n"
		"<option value=\"2\"$_selenc2$>WPA_PSK</option>\r\n"
		"<option value=\"3\"$_selenc3$>WPA2_PSK</option>\r\n"
		"<option value=\"4\"$_selenc4$>WPA_WPA2_PSK</option>\r\n"
		"</select>\r\n"
		"</div>\r\n"
		"<div>\r\n"
		"<input type=\"submit\" value=\"Update Access Point\"></input>\r\n"
		"</div>\r\n"
		"</form>\r\n"
		"</div>\r\n"
		"</body>\r\n"
		"</html>";

void ICACHE_FLASH_ATTR MainHttpServerSocket::DoAccessPage(const char *page, HttpParameters *paramarray, int params)
	{
		struct softap_config config;

		os_memset(&config,0,sizeof(config));
		wifi_softap_get_config(&config);

		char buf[os_strlen(accessPage) + 256];
		os_strcpy(buf,accessPage);
		str_replace(buf,"$SSID$",(const char*)&config.ssid);
		str_replace(buf,"$SSPW$",(const char*)&config.password);
		char token[128];
		for(uint8 i = 1;i < 14;i++)
			{
				os_sprintf(token,"$_selch%d$",(int)i);
				if(config.channel == i)
					str_replace(buf,token," selected");
				else
					str_replace(buf,token,"");

			}

		for(int i = 0;i < 5;i++)
			{
				os_sprintf(token,"$_selenc%d$",i);
				int x = (int)config.authmode;
				if(x == i)
					str_replace(buf,token," selected");
				else
					str_replace(buf,token,"");

			}
		SetContentLength(buf);
		CloseOnSent();
		Send(buf);
	}




