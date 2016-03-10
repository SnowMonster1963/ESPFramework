/*
 * WiFiManager.h
 *
 *  Created on: Feb 28, 2016
 *      Author: tsnow
 */

#ifndef INCLUDE_WIFIMANAGER_H_
#define INCLUDE_WIFIMANAGER_H_
#include <ets_sys.h>
#include <osapi.h>
#include <os_type.h>
#include <gpio.h>
#include "driver/Timer.h"
#include "driver/ConfigManager.h"
#include "driver/Debug.h"
#include "driver/Stream.h"
#include "driver/Process.h"
#include "driver/WiFi.h"
#include "driver/Socket.h"
#include "driver/Mqtt.h"

class MyWifiStatus : public WiFiStatusProcess
{
private:
	bool connected;
	MyWifiStatus();

public:
	static MyWifiStatus *GetProcess();
	bool IsConnected();

protected:
	void OnStationConnected(const char *ssid,uint8_t channel);
	void OnStationDisconnected(const char *ssid,uint8_t reason);
	void OnStationAuthChange(AUTH_MODE oldmode, AUTH_MODE newmode);
	void OnStationGotIP(ip_addr ip, ip_addr mask, ip_addr gateway);
	void OnAccessClientConnected(const uint8_t *mac,uint8_t aid);
	void OnAccessClientDisconnected(const uint8_t *mac,uint8_t aid);
};


extern MyWifiStatus *wifiproc;

#endif /* INCLUDE_WIFIMANAGER_H_ */
