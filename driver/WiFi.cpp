/*
 * WiFi.cpp
 *
 *  Created on: Jan 3, 2016
 *      Author: tsnow
 */

#include "driver/WiFi.h"

WiFi wifi;

ICACHE_FLASH_ATTR WiFi::WiFi()
{
	m_userwifistatus = NULL;
	wifi_set_event_handler_cb(WifiEventCallback);
	m_mode = wifi_get_opmode();
	currtask = NULL;
}

bool ICACHE_FLASH_ATTR WiFi::IsStation()
{
	m_mode = wifi_get_opmode();
	return ((m_mode & STATION_MODE) != 0);
}

bool ICACHE_FLASH_ATTR WiFi::IsAccessPoint()
{
	m_mode = wifi_get_opmode();
	return ((m_mode & SOFTAP_MODE) != 0);
}

bool ICACHE_FLASH_ATTR WiFi::EnableStation(bool save)
{
	m_mode = wifi_get_opmode();
	m_mode |= STATION_MODE;
	bool ret = false;
	if (save)
		ret = wifi_set_opmode(m_mode);
	else
		ret = wifi_set_opmode_current(m_mode);

	return ret;
}

bool ICACHE_FLASH_ATTR WiFi::DisableStation(bool save)
{
	m_mode = wifi_get_opmode();
	m_mode &= ~STATION_MODE;
	bool ret = false;
	if (save)
		ret = wifi_set_opmode(m_mode);
	else
		ret = wifi_set_opmode_current(m_mode);

	return ret;
}

bool ICACHE_FLASH_ATTR WiFi::EnableAccessPoint(bool save)
{
	m_mode = wifi_get_opmode();
	m_mode |= SOFTAP_MODE;
	bool ret = false;
	if (save)
		ret = wifi_set_opmode(m_mode);
	else
		ret = wifi_set_opmode_current(m_mode);

	return ret;
}

bool ICACHE_FLASH_ATTR WiFi::DisableAccessPoint(bool save)
{
	m_mode = wifi_get_opmode();
	m_mode &= ~SOFTAP_MODE;
	bool ret = false;
	if (save)
		ret = wifi_set_opmode(m_mode);
	else
		ret = wifi_set_opmode_current(m_mode);

	return ret;
}

void ICACHE_FLASH_ATTR WiFi::ScanCallback(void *args, STATUS status)
{
	bss_info *info = (bss_info *) args;

	// docs say to ignore first one
	info = info->next.stqe_next;
	wifi.ProcessScan(info, status);
}

void ICACHE_FLASH_ATTR WiFi::ProcessScan(bss_info *info, STATUS status)
{
	if (!currtask)
		return;

	ScanTask::ScanMessage *msg = new ScanTask::ScanMessage;
	msg->mt = ScanTask::StatusMsg;
	msg->status = status;
	currtask->Post(msg);

	while (info)
	{
		msg = new ScanTask::ScanMessage;
		msg->mt = ScanTask::StationMsg;
		msg->station_info = info;

		currtask->Post(msg);

		info = info->next.stqe_next;
	}

	msg = new ScanTask::ScanMessage;
	msg->mt = ScanTask::DoneMsg;
	currtask->Post(msg);

}

bool ICACHE_FLASH_ATTR WiFi::ScanAccessPoints(ScanTask *task, const char *ssid, const char *bssid,
		uint8_t channel, bool showhidden)
{

	if (!task)
		return false;

	currtask = task;

	scan_config sc;
	sc.ssid = (uint8_t *) ssid;
	sc.bssid = (uint8_t *) bssid;
	sc.channel = channel;
	sc.show_hidden = showhidden;
	return wifi_station_scan(&sc, ScanCallback);

}

void ICACHE_FLASH_ATTR WiFi::WifiEventCallback(System_Event_t *event)
{
	wifi.ProcessWifiEvent(event);
}

void ICACHE_FLASH_ATTR WiFi::ProcessWifiEvent(System_Event_t *evt)
{
	os_printf("WiFi::ProcessWifiEvent:  event %x\n", evt->event);
	switch (evt->event)
	{
	case EVENT_STAMODE_CONNECTED:
		os_printf("WiFi::ProcessWifiEvent:  connect to ssid %s, channel %d\n",
				evt->event_info.connected.ssid,
				evt->event_info.connected.channel);
		break;
	case EVENT_STAMODE_DISCONNECTED:
		os_printf("WiFi::ProcessWifiEvent:  disconnect from ssid %s, reason %d\n",
				evt->event_info.disconnected.ssid,
				evt->event_info.disconnected.reason);
		break;
	case EVENT_STAMODE_AUTHMODE_CHANGE:
		os_printf("WiFi::ProcessWifiEvent:  mode: %d -> %d\n", evt->event_info.auth_change.old_mode,
				evt->event_info.auth_change.new_mode);
		break;
	case EVENT_STAMODE_GOT_IP:
		os_printf("WiFi::ProcessWifiEvent:  ip:" IPSTR ",mask:" IPSTR ",gw:" IPSTR,
				IP2STR(&evt->event_info.got_ip.ip),
				IP2STR(&evt->event_info.got_ip.mask),
				IP2STR(&evt->event_info.got_ip.gw));
		os_printf("\n");
		break;
	case EVENT_SOFTAPMODE_STACONNECTED:
		os_printf("WiFi::ProcessWifiEvent:  station: " MACSTR "join, AID = %d\n",
				MAC2STR(evt->event_info.sta_connected.mac),
				evt->event_info.sta_connected.aid);
		break;
	case EVENT_SOFTAPMODE_STADISCONNECTED:
		os_printf("WiFi::ProcessWifiEvent:  station: " MACSTR "leave, AID = %d\n",
				MAC2STR(evt->event_info.sta_disconnected.mac),
				evt->event_info.sta_disconnected.aid);
		break;
	default:
		break;
	}
	if(m_userwifistatus != NULL)
	{
		System_Event_t *nevt = new System_Event_t;
		os_memcpy(nevt,evt,sizeof(System_Event_t));

		m_userwifistatus->PostMessage(nevt);
	}
}

bool ICACHE_FLASH_ATTR WiFi::Connect(const char *ssid,const char *passwd)
{
    wifi_station_disconnect();
    bool ret = SetConnection(ssid,passwd);
    ret = ret == true ? wifi_station_connect() : ret;
    return ret;
}


bool ICACHE_FLASH_ATTR WiFi::SetConnection(const char *ssid,const char *passwd)
{
	station_config stationConf;
	os_memset(&stationConf,0,sizeof(station_config));
	os_strcpy((char *)stationConf.ssid,ssid);
	os_strcpy((char *)stationConf.password,passwd);
    ETS_UART_INTR_DISABLE();
    wifi_station_set_config(&stationConf);
    ETS_UART_INTR_ENABLE();
}

bool ICACHE_FLASH_ATTR WiFi::SetHostName(const char *hostname)
{
	return wifi_station_set_hostname((char *)hostname);
}

const char *ICACHE_FLASH_ATTR WiFi::GetHostName()
	{
		return wifi_station_get_hostname();
	}
