/*
 * HttpPageWiFi.cpp
 *
 *  Created on: May 17, 2016
 *      Author: tsnow
 */
#include <ATMqtt.h>
#include <CmdProcessor.h>
#include <WiFiManager.h>
#include <driver/utils.h>
#include "MainHttp.h"

MainHttpServerSocket * MainHttpServerSocket::m_pWiFiSock = NULL;
extern const char *enc[5];

const char *wifiPage = "HTTP/1.1 200 OK\r\n"
		"Server: Apache-Coyote/1.1\r\n"
		"Accept-Ranges: bytes\r\n"
		"ETag: W/\"925-1463435002939\"\r\n"
		"Last-Modified: Mon, 16 May 2016 21:43:22 GMT\r\n"
		"Content-Type: text/html\r\n"
		"Content-Length: $LENGTH$\r\n"
		"Date: Mon, 16 May 2016 21:46:15 GMT\r\n"
		"\r\n"
		"<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\" \"http://www.w3.org/TR/html4/loose.dtd\">\r\n"
		"<html>\r\n"
		"<link rel=\"stylesheet\" type=\"text/css\" href=\"css.css\" />\r\n"
		"<head>\r\n"
		"<meta http-equiv=\"Content-Type\" content=\"text/html; charset=ISO-8859-1\">\r\n"
		"<title>Connect to WiFi</title>\r\n"
		"</head>\r\n"
		"<body>\r\n"
		"\r\n"
		"<div class=\"wifi\">"
			"<h2>Update Device's Connection to Internet WiFi</h2>"
			"<form action=\"/set_wifi.html\" method=\"post\">"
				"<table>"
					"<thead>"
						"<tr>"
							"<td>Network</td>"
							"<td>Encryption</td>"
						"</tr>"
					"</thead>"
					"<tbody>"
						"$_row_ap$"
					"</tbody>"
				"</table>"
				"<div>Password:"
					"<input type=\"password\" name=\"pw\"></input>"
				"</div>"
				"<div>"
					"<input type=\"submit\" value=\"Choose Network\"></input>"
				"</div>"
			"</form>"
			"<div>\r\n"
				"<em>Note: Unless this Web Browser is connected directly to the\r\n"
					"Device's Access Point, changing the Device's WiFi connection will\r\n"
					"cause this connection to fail. </em>\r\n"
			"</div>\r\n"
		"</div>"
		"</body>\r\n"
		"</html>";

const char *rowFragment =
		"<tr>"
			"<td><input type=\"radio\" name=\"sel\" value=\"$AP$\" $_check$></input>$AP$</td>"
			"<td>$ENC$</td>"
		"</tr>";

void ICACHE_FLASH_ATTR MainHttpServerSocket::WiFiCallback(void *arg,STATUS status)
	{
		m_pWiFiSock->DoWiFiCallback((bss_info *)arg,status);
	}

void ICACHE_FLASH_ATTR MainHttpServerSocket::DoWiFiCallback(struct bss_info *bss,STATUS status)
	{
		m_pWiFiSock = NULL;
		int apcnt = 0;

		if(status == OK)
			{
				struct bss_info *p = bss;//->next.stqe_next;
				while(p != NULL)
					{
						apcnt++;
						p = p->next.stqe_next;
					}
				int buflen = os_strlen(wifiPage) + apcnt * (os_strlen(rowFragment) + 32) + 256;
				int fragslen = (apcnt+1) * (os_strlen(rowFragment) + 32);
				char *buf = new char[buflen];
				char *frags = new char[fragslen];
				os_memset(buf,0,buflen);
				os_strcpy(buf,wifiPage);
				os_memset(frags,0,fragslen);
				p = bss;//->next.stqe_next;
				struct station_config cfg;
				wifi_station_get_config(&cfg);
				char hn[33];
				os_memset(hn,0,sizeof(hn));
				os_memcpy(hn,cfg.ssid,32);
				while(p != NULL)
					{
						char frag[os_strlen(rowFragment) + 32];
						os_memcpy(frag,rowFragment,os_strlen(rowFragment)+1);
						char ssid[33];
						os_memset(ssid,0,sizeof(ssid));
						os_memcpy(ssid,p->ssid,p->ssid_len);
						if(os_strcmp(ssid,hn) == 0)
							str_replace(frag,"$_check$","checked");
						else
							str_replace(frag,"$_check$","");
						str_replace(frag,"$AP$",ssid);
						str_replace(frag,"$ENC$",enc[p->authmode]);
						os_strcat(frags,frag);
						p = p->next.stqe_next;
					}
				str_replace(buf,"$_row_ap$",frags);
				SetContentLength(buf);
				CloseOnSent();
				Send(buf);
				delete [] frags;
				delete [] buf;

			}

	}



void ICACHE_FLASH_ATTR MainHttpServerSocket::DoWiFiPage(const char *page, HttpParameters *paramarray, int params)
	{
		if(m_pWiFiSock == NULL)
			{
				m_pWiFiSock = this;
				struct scan_config config;
				os_memset(&config,0,sizeof(config));
				wifi_station_scan(&config,WiFiCallback);
			}
		else
			{
				// handle 'busy' page
			}

	}
