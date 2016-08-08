/*
 * HttpPageSetMqtt.cpp
 *
 *  Created on: May 16, 2016
 *      Author: tsnow
 */

#include <ATMqtt.h>
#include <CmdProcessor.h>
#include <WiFiManager.h>
#include <driver/utils.h>
#include "MainHttp.h"


const char *set_mqttPage = "HTTP/1.1 200 OK\r\n"
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
		"<title>MQTT Settings Updated</title>\r\n"
		"</head>\r\n"
		"<body>\r\n"
		"\r\n"
			"<div class=\"mqtt\">\r\n"
				"<h2>MQTT Settings Updated</h2>\r\n"
				"<div>MQTT Broker: $MQTTHost$</div>\r\n"
				"<div>Port: $MQTTPort$</div>\r\n"
				"<div>Client ID: $CLIENT$</div>\r\n"
				"<div>MQTT Username: $MQTTUSER$</div>\r\n"
				"<div>MQTT Password: **********</div>\r\n"
				"<div>Will Topic: $WILLTOPIC$</div>\r\n"
				"<div>Will Message: $WILLPAYLOAD$</div>\r\n"
				"<div>Will Quality of Service: $QOS$</div>\r\n"
				"<div>Retain Will? $RW$</div>\r\n"
				"<div>Clean Session? $CS$</div>\r\n"
				"<div>Keep Alive: $KA$</div>\r\n"
				"<form action=\"/\" method=\"post\">\r\n"
					"<div>\r\n"
						"<input type=\"submit\" value=\"Go Back To Main Menu\"></input>\r\n"
					"</div>\r\n"
				"</form>\r\n"
			"</div>\r\n"
		"</body>\r\n"
		"</html>";

const char*qosstrs[] =
		{
				"QoS 0 - Fire and Forget",
				"QoS 1 - Acknowledged Delivery",
				"QoS 2 - Assured Delivery"
		};

const char *yesno[] = {"No","Yes"};

void ICACHE_FLASH_ATTR MainHttpServerSocket::DoSetMqttPage(const char *page, HttpParameters *paramarray, int params)
	{
		char buf[os_strlen(set_mqttPage)+256];
		char tok[16];

		os_strcpy(buf,set_mqttPage);
		MQTT_Connect_Params *pmqtt = (MQTT_Connect_Params *)&mqttparms;

		for(int i=0;i<params;i++)
			{
				if(os_strcmp(paramarray[i].parameter,"broker") == 0)
					{
						str_replace(buf,"$MQTTHost$",paramarray[i].value);
						os_memset(pmqtt->host,0,sizeof(pmqtt->host));
						os_strcpy(pmqtt->host,paramarray[i].value);
					}
				if(os_strcmp(paramarray[i].parameter,"port") == 0)
					{
						str_replace(buf,"$MQTTPort$",paramarray[i].value);
						pmqtt->port = atoi(paramarray[i].value);
					}
				if(os_strcmp(paramarray[i].parameter,"cid") == 0)
					{
						str_replace(buf,"$CLIENT$",paramarray[i].value);
						os_memset(pmqtt->client_id,0,sizeof(pmqtt->client_id));
						os_strcpy(pmqtt->client_id,paramarray[i].value);
					}
				if(os_strcmp(paramarray[i].parameter,"un") == 0)
					{
						str_replace(buf,"$MQTTUSER$",paramarray[i].value);
						os_memset(pmqtt->username,0,sizeof(pmqtt->username));
						os_strcpy(pmqtt->username,paramarray[i].value);
					}
				if(os_strcmp(paramarray[i].parameter,"pw") == 0)
					{
						os_memset(pmqtt->password,0,sizeof(pmqtt->password));
						os_strcpy(pmqtt->password,paramarray[i].value);
					}
				if(os_strcmp(paramarray[i].parameter,"wt") == 0)
					{
						str_replace(buf,"$WILLTOPIC$",paramarray[i].value);
						os_memset(pmqtt->willtopic,0,sizeof(pmqtt->willtopic));
						os_strcpy(pmqtt->willtopic,paramarray[i].value);
					}
				if(os_strcmp(paramarray[i].parameter,"wpl") == 0)
					{
						str_replace(buf,"$WILLPAYLOAD$",paramarray[i].value);
						os_memset(pmqtt->willpayload,0,sizeof(pmqtt->willpayload));
						os_strcpy(pmqtt->willpayload,paramarray[i].value);
					}
				if(os_strcmp(paramarray[i].parameter,"qos") == 0)
					{
						int v = atoi(paramarray[i].value);
						str_replace(buf,"$QOS$",qosstrs[v%3]);
						pmqtt->willqos = v;
					}
				if(os_strcmp(paramarray[i].parameter,"wr") == 0)
					{
						int v = atoi(paramarray[i].value) % 2;
						str_replace(buf,"$RW$",yesno[v]);
						pmqtt->willretain = v != 0;
					}
				if(os_strcmp(paramarray[i].parameter,"cs") == 0)
					{
						int v = atoi(paramarray[i].value) % 2;
						str_replace(buf,"$CS$",yesno[v]);
						pmqtt->cleansession = v != 0;
					}
				if(os_strcmp(paramarray[i].parameter,"ka") == 0)
					{
						int v = atoi(paramarray[i].value);
						str_replace(buf,"$KA$",qosstrs[v%3]);
						pmqtt->keepalive = v;
					}
			}

		mqttparms.SaveData();

		SetContentLength(buf);
		CloseOnSent();
		Send(buf);
	}
