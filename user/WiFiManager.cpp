/*
 * WiFiManager.cpp
 *
 *  Created on: Feb 28, 2016
 *      Author: tsnow
 */

#include <ATMqtt.h>
#include <CmdProcessor.h>
#include <WiFiManager.h>
#include <MainHttp.h>

MyWifiStatus *wifiproc;



HttpServerSocket server;

ICACHE_FLASH_ATTR MyWifiStatus::MyWifiStatus()
	{
		connected = false;
	}

ICACHE_FLASH_ATTR MyWifiStatus *MyWifiStatus::GetProcess()
	{
		if(wifiproc == NULL)
			wifiproc = new MyWifiStatus();

		return wifiproc;
	}

ICACHE_FLASH_ATTR bool MyWifiStatus::IsConnected()
	{
		return connected;
	}

ICACHE_FLASH_ATTR void MyWifiStatus::OnStationConnected(const char *ssid, uint8_t channel)
	{
		ets_uart_printf("+WIFI Connected\r\n");
//		os_printf("MyWifiStatus:  Connected to %s, channel %d\r\n", ssid, channel);
		connected = true;
	}

ICACHE_FLASH_ATTR void MyWifiStatus::OnStationDisconnected(const char *ssid, uint8_t reason)
	{
		ets_uart_printf("+WIFI Disconnected\r\n");
		connected = false;
	}

ICACHE_FLASH_ATTR void MyWifiStatus::OnStationAuthChange(AUTH_MODE oldmode, AUTH_MODE newmode)
	{
	}

ICACHE_FLASH_ATTR void MyWifiStatus::OnStationGotIP(ip_addr ip, ip_addr mask, ip_addr gateway)
	{
//		os_printf("MyWifiStatus:  ip:" IPSTR ", mask:" IPSTR ", gw:" IPSTR, IP2STR(&ip), IP2STR(&mask), IP2STR(&gateway));
//		os_printf("\r\n");
		pSock = new MyMqtt();
		if(server.IsListening() == false)
			server.Listen(&MainHttpFactory,80);
	}

ICACHE_FLASH_ATTR void MyWifiStatus::OnAccessClientConnected(const uint8_t *mac, uint8_t aid)
	{
		if(server.IsListening() == false)
			server.Listen(&MainHttpFactory,80);
	}

ICACHE_FLASH_ATTR void MyWifiStatus::OnAccessClientDisconnected(const uint8_t *mac, uint8_t aid)
	{
	}

