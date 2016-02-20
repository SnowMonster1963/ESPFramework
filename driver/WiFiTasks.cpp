/*
 * WiFiScanTask.cpp
 *
 *  Created on: Jan 3, 2016
 *      Author: tsnow
 */

#include "driver/WiFi.h"


ICACHE_FLASH_ATTR ScanTask::ScanTask() : ProcessTask(Priority0Manager)
{

}

ICACHE_FLASH_ATTR void ScanTask::runTask(const void *p)
{
	ScanMessage *msg = (ScanMessage *)p;
	switch(msg->mt)
	{
	case StatusMsg:
		OnStatus(msg->status);
		break;
	case StationMsg:
	{
		const bss_info *bi = msg->station_info;
		char ssid[bi->ssid_len + 1];
		memset(ssid,0,sizeof(ssid));
		os_strncpy(ssid,(const char *)bi->ssid,sizeof(ssid)-1);
		char bssid[sizeof(bi->bssid) + 1];
		os_strncpy(bssid,(const char *)bi->bssid,sizeof(bssid)-1);
		OnStation(ssid,bssid,bi->channel,bi->authmode,bi->rssi,bi->is_hidden,bi);
	}
		break;
	case DoneMsg:
		break;
	}

	delete msg;
}


ICACHE_FLASH_ATTR void WiFiStatusProcess::OnMessage(const System_Event_t *evt)
{
	OnEventMessage(evt);

	switch (evt->event)
	{
	case EVENT_STAMODE_CONNECTED:
		OnStationConnected((const char *)evt->event_info.connected.ssid, evt->event_info.connected.channel);
		break;
	case EVENT_STAMODE_DISCONNECTED:
		OnStationDisconnected((const char *)evt->event_info.disconnected.ssid, evt->event_info.disconnected.reason);
		break;
	case EVENT_STAMODE_AUTHMODE_CHANGE:
		OnStationAuthChange((AUTH_MODE)evt->event_info.auth_change.old_mode, (AUTH_MODE)evt->event_info.auth_change.new_mode);
		break;
	case EVENT_STAMODE_GOT_IP:
		OnStationGotIP(evt->event_info.got_ip.ip,
				evt->event_info.got_ip.mask,
				evt->event_info.got_ip.gw);
		break;
	case EVENT_SOFTAPMODE_STACONNECTED:
		OnAccessClientConnected(evt->event_info.sta_connected.mac, evt->event_info.sta_connected.aid);
		break;
	case EVENT_SOFTAPMODE_STADISCONNECTED:
		OnAccessClientDisconnected(evt->event_info.sta_disconnected.mac,evt->event_info.sta_disconnected.aid);
		break;
	default:
		break;
	}
}
