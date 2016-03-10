/*
 * Mqtt.h
 *
 *  Created on: Jan 7, 2016
 *      Author: tsnow
 */

#ifndef INCLUDE_DRIVER_MQTT_H_
#define INCLUDE_DRIVER_MQTT_H_

#include "Array.h"
#include "driver/Socket.h"
#include "driver/Timer.h"

#ifndef byte
typedef unsigned char byte;
#endif

struct MQTT_Connect_Params
	{
	char host[50];
	uint16_t port;
	char client_id[24];
	char username[16];
	char password[16];
	char willtopic[32];
	char willpayload[32];
	int willqos;
	bool willretain;
	bool cleansession;
	unsigned int keepalive;

	};

class MQTTSocket;

class MQTTKeepAliveTimer : public PtrTimer<MQTTSocket>
{
public:
	MQTTKeepAliveTimer(MQTTSocket *sock,uint32_t delay) : PtrTimer<MQTTSocket>(sock,delay,true)
	{

	}

	void OnTime();
};

class MQTTSocket: public Socket
	{
	friend class MQTTKeepAliveTimer;

public:
	enum Qos
		{
		FireAndForget = 0, AcknowledgedDelivery, AssuredDelivery
		};

	enum ConnAckCode
		{
		Accepted = 0, WrongProtocol, RejectedID, ServerUnavailable, InvalidLogin, NotAuthorized
		};

	enum OpResult
		{
		Sent, Queued, Error
		};

private:
	class MQTTMessage
		{
	public:
		byte *msg;
		unsigned short len;
		bool handled;

	public:
		MQTTMessage(const byte *msg, unsigned short len)
			{
				this->msg = new byte[len];
				os_memcpy(this->msg, msg, len);
				this->len = len;
				handled = false;
			}
		;

		~MQTTMessage()
			{
				delete[] msg;
			}
		;
		};

	MQTT_Connect_Params &m_parms;
	bool m_prepend_client_id;
	Queue<MQTTMessage> msgs;
	size_t msgid;
	MQTTKeepAliveTimer *keepalive_timer;

	void DoConnect();
	void HandleMsgQueue();
	void QueueMessage(const byte *data, unsigned short len);
	void HandleConnAckEvent(const byte *data, unsigned short len);
	void HandlePublishEvent(const byte *data, unsigned short len);
	void HandlePubAckEvent(const byte *data, unsigned short len);
	size_t NextMessageID()
		{
			msgid++;
			if(msgid == 0)
				msgid++;
			return msgid;
		};

protected:
	void GotDisconnect();

	void OnConnect();
	void OnReconnect(sint8 err);
	void OnDisconnect();
	void OnSent();
	void OnReceive(uint8_t *data, unsigned short len);

	virtual void OnConnack(ConnAckCode result);
	virtual void OnPublish(const char *topic, const byte *payload, size_t len);


	void KeepAlive();

public:
	MQTTSocket(MQTT_Connect_Params &parms, bool prepend_client_id = true);

	// operations
	OpResult Subscribe(const char *topic, Qos qos = AcknowledgedDelivery);
	OpResult Publish(const char *topic, const byte *payload, size_t len, Qos qos = AcknowledgedDelivery, bool retain = true);
	OpResult Publish(const char *topic, const char *msg, Qos qos = AcknowledgedDelivery, bool retain = true)
		{
			return Publish(topic,(const byte *)msg,os_strlen(msg),qos,retain);
		}

	Socket::Result Send(const uint8_t*data,unsigned short len);
	virtual Socket::Result Reconnect();
	virtual Socket::Result Disconnect();

	};

#endif /* INCLUDE_DRIVER_MQTT_H_ */
