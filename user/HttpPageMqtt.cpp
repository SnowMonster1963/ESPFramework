/*
 * HttpPageMqtt.cpp
 *
 *  Created on: May 16, 2016
 *      Author: tsnow
 */


#include <ATMqtt.h>
#include <CmdProcessor.h>
#include <WiFiManager.h>
#include <driver/utils.h>
#include "MainHttp.h"

const char *mqttPage = "HTTP/1.1 200 OK\r\n"
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
		"<body>\r\n"
		"<h1>Device Setup</h1>\r\n"
		"<div class=\"mqtt\">"
			"<h2>Update MQTT Settings</h2>"
			"<form action=\"/set_mqtt.html\" method=\"post\">"
				"<div>MQTT Broker: <input type=\"text\" maxlength=\"49\" name=\"broker\" value=\"$MQTTHost$\"></input></div>"
				"<div>Port: <input type=\"text\" name=\"port\" value=\"$MQTTPort$\"></input></div>"
				"<div>Client ID: <input type=\"text\" maxlength=\"23\" name=\"cid\" value=\"$CLIENT$\"></input></div>"
				"<div>MQTT Username: <input type=\"text\" maxlength=\"15\" name=\"un\" value=\"$MQTTUSER$\"></input></div>"
				"<div>MQTT Password: <input type=\"password\" maxlength=\"15\" name=\"pw\" value=\"$MQTTPASSWORD$\"></input></div>"
				"<div>Will Topic: <input type=\"text\" maxlength=\"31\" name=\"wt\" value=\"$WILLTOPIC$\"></input></div>"
				"<div>Will Message: <input type=\"text\" maxlength=\"31\" name=\"wpl\" value=\"$WILLPAYLOAD$\"></input></div>"
				"<div>Will Quality of Service: "
					"<select\tname=\"qos\">"
						"<option value=\"0\" $_selqos0$>QoS 0 - Fire and Forget</option>"
						"<option value=\"1\" $_selqos1$>QoS 1 - Acknowledged Delivery</option>"
						"<option value=\"2\" $_selqos2$>QoS 2 - Assured Delivery</option>"
					"</select>"
				"</div> "
				"<div>Retain Will?"
					"<select name=\"wr\">"
						"<option value=\"0\" $_selwr0$>No</option>"
						"<option value=\"1\" $_selwr1$>Yes</option>"
					"</select>"
				"</div>"
				"<div>Clean Session?"
					"<select name=\"cs\">"
						"<option value=\"0\" $_selcs0$>No</option>"
						"<option value=\"1\" $_selcs1$>Yes</option>"
					"</select>"
				"</div>"
				"<div>Keep Alive: <input type=\"text\" name=\"ka\" value=\"$KA$\"></input></div>"
				"<div>"
					"<input type=\"submit\" value=\"Update MQTT Settings\"></input>"
				"</div>"
			"</form>"
		"</div>"
		"</body>\r\n"
		"</html>";

void ICACHE_FLASH_ATTR MainHttpServerSocket::DoMqttPage(const char *page, HttpParameters *paramarray, int params)
	{
		char buf[os_strlen(mqttPage)+256];
		char tok[16];

		os_strcpy(buf,mqttPage);

		MQTT_Connect_Params *pmqtt = (MQTT_Connect_Params *)&mqttparms;
		str_replace(buf,"$MQTTHost$",pmqtt->host);
		os_sprintf(tok,"%d",pmqtt->port);
		str_replace(buf,"$MQTTPort$",tok);
		str_replace(buf,"$CLIENT$",pmqtt->client_id);
		str_replace(buf,"$MQTTUSER$",pmqtt->username);
		str_replace(buf,"$MQTTPASSWORD$",pmqtt->password);
		str_replace(buf,"$WILLTOPIC$",pmqtt->willtopic);
		str_replace(buf,"$WILLPAYLOAD$",pmqtt->willpayload);
		os_sprintf(tok,"%u",pmqtt->keepalive);
		str_replace(buf,"$KA$",tok);

		for(int i=0;i<3;i++)
			{
				os_sprintf(tok,"$_selqos%d$",i);
				if(pmqtt->willqos == i)
					{
						str_replace(buf,tok,"selected");
					}
				else
					{
						str_replace(buf,tok,"");
					}
			}

		for(int i=0;i<2;i++)
			{
				os_sprintf(tok,"$_selcs%d$",i);
				int n = pmqtt->cleansession ? 1 : 0;
				if(n == i)
					{
						str_replace(buf,tok,"selected");
					}
				else
					{
						str_replace(buf,tok,"");
					}

				os_sprintf(tok,"$_selwr%d$",i);
				n = pmqtt->willretain ? 1 : 0;
				if(n == i)
					{
						str_replace(buf,tok,"selected");
					}
				else
					{
						str_replace(buf,tok,"");
					}
			}

		SetContentLength(buf);
		CloseOnSent();
		Send(buf);
	}







