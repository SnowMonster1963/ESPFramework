/*
 * WiFi.h
 *
 *  Created on: Jan 3, 2016
 *      Author: tsnow
 */

#ifndef INCLUDE_DRIVER_WIFI_H_
#define INCLUDE_DRIVER_WIFI_H_

extern "C"
{
#include <string.h>
#include <ets_sys.h>
#include <osapi.h>
#include <os_type.h>
#include <c_types.h>
#include <user_interface.h>
#include "espmissingincludes.h"
extern int ets_uart_printf(const char *fmt, ...);
}
#include "driver/Process.h"

class WiFi;

class ScanTask : public ProcessTask
{
	friend class WiFi;

protected:
	void runTask(const void *p);
	enum ScanMessageType
	{
		StatusMsg,
		StationMsg,
		DoneMsg,
	};

	struct ScanMessage
	{
		ScanMessageType mt;
		const bss_info	*station_info;
		STATUS		status;
	};

public:
	ScanTask();

	virtual void OnStatus(STATUS status) = 0;
	virtual void OnStation(const char *ssid,const char *bssid,uint8_t channel,AUTH_MODE authmode,sint8 SignalStrenth,bool ishidden,const bss_info *bssdetail) = 0;
	virtual void OnComplete() = 0;
};

class WiFiStatusProcess : public UserTask<const System_Event_t,true>
{
public:
	WiFiStatusProcess() : UserTask(Priority0Manager){};

protected:
	void OnMessage(const System_Event_t *event);

	virtual void OnEventMessage(const System_Event_t *event){};// don't do anything, but give user option to look directly at event
	virtual void OnStationConnected(const char *ssid,uint8_t channel) = 0;
	virtual void OnStationDisconnected(const char *ssid,uint8_t reason) = 0;
	virtual void OnStationAuthChange(AUTH_MODE oldmode, AUTH_MODE newmode) = 0;
	virtual void OnStationGotIP(ip_addr ip, ip_addr mask, ip_addr gateway) = 0;
	virtual void OnAccessClientConnected(const uint8_t *mac,uint8_t aid) = 0;
	virtual void OnAccessClientDisconnected(const uint8_t *mac,uint8_t aid) = 0;
};

class WiFi
{
private:
	uint8_t			m_mode;	// station or AP or both
	ScanTask		*currtask;
	WiFiStatusProcess *m_userwifistatus;

	static void ScanCallback(void *args,STATUS status);
	static void WifiEventCallback(System_Event_t *event);

	void ProcessScan(bss_info *info,STATUS status);
	void ProcessWifiEvent(System_Event_t *event);

public:
	WiFi();

	void AttachWiFiEventProcess(WiFiStatusProcess *proc){m_userwifistatus = proc;};

	// Station Operations
	bool IsStation();
	bool EnableStation(bool save=true);
	bool DisableStation(bool save=true);
	bool SetConnection(const char *ssid,const char *passwd);
	bool Connect(){return wifi_station_connect();};
	bool Connect(const char *ssid,const char *passwd);
	bool Disconnect(){return wifi_station_disconnect();};
	bool ScanAccessPoints(ScanTask *task,const char *ssid=NULL,const char *bssid=NULL,uint8_t channel=0,bool showhidden=true);


	// AP Operations
	bool IsAccessPoint();
	bool EnableAccessPoint(bool save=true);
	bool DisableAccessPoint(bool save=true);
	const char *GetHostName();
	bool SetHostName(const char *hostname);
};


extern WiFi wifi;

#endif /* INCLUDE_DRIVER_WIFI_H_ */
