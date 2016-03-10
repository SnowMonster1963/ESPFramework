/*
 * ATMqtt.h
 *
 *  Created on: Feb 27, 2016
 *      Author: tsnow
 */

#ifndef INCLUDE_ATMQTT_H_
#define INCLUDE_ATMQTT_H_

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

#define MAX_TOPICS 20

// Comment next line if you want to use the values stored in the static MQTTParms
#define ALREADY_SAVED

struct MqttTopic
	{
	char *Topic;
	byte *Value;
	size_t len;
	};

class MyMqtt : public MQTTSocket
	{
private:
	MqttTopic	arry[MAX_TOPICS];
	size_t		topic_len;

	void printTopic(size_t idx);

protected:
	void OnDisconnect();


public:
	MyMqtt();
	void OnConnack(ConnAckCode x);
	void OnPublish(const char *topic, const byte *payload, size_t len);
	void printTopics();
	size_t updateTopic(const char *Topic,const byte *data,size_t len);
	size_t updateTopic(const char *Topic,const char *data);
	Socket::Result Disconnect();
	Socket::Result Reconnect();

	bool		autoreconnect;
	};


extern MyMqtt *pSock;
extern MQTT_Connect_Params staticmqttparms;
extern ConfigManager<MQTT_Connect_Params> mqttparms;

#endif /* INCLUDE_ATMQTT_H_ */
