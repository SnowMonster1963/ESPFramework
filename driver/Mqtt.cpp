/*
 * Mqtt.cpp
 *
 *  Created on: Jan 7, 2016
 *      Author: tsnow
 */

#include "driver/Mqtt.h"
#include "driver/Debug.h"

#define MQTT_CONNECT 	1 // Client request to connect to Server
#define MQTT_CONNACK 	2 // Connect Acknowledgment
#define MQTT_PUBLISH 	3 // Publish message
#define MQTT_PUBACK 	4 // Publish Acknowledgment
#define MQTT_PUBREC 	5 // Publish Received (assured delivery part 1)
#define MQTT_PUBREL 	6 // Publish Release (assured delivery part 2)
#define MQTT_PUBCOMP 	7 // Publish Complete (assured delivery part 3)
#define MQTT_SUBSCRIBE 	8 // Client Subscribe request
#define MQTT_SUBACK 	9 // Subscribe Acknowledgment
#define MQTT_UNSUBSCRIBE 10 // Client Unsubscribe request
#define MQTT_UNSUBACK 	11 // Unsubscribe Acknowledgment
#define MQTT_PINGREQ 	12 // PING Request
#define MQTT_PINGRESP 	13 // PING Response
#define MQTT_DISCONNECT 14 // Client is Disconnecting
// connect flags
#define CF_CLEAN (1 << 1)
#define CF_WILL (1 << 2)
#define CF_WILLQOS(qos) (qos << 3)
#define CF_WILLRETAIN (1 << 5)
#define CF_PASSWORD (1 << 6)
#define CF_USERNAME (1 << 7)

struct MQTT_Header
	{
	byte Retain :1;
	byte QoS :2;
	byte Dup :1;
	byte MessageType :4;

	byte RemainingLength[1];

	MQTT_Header()
		{
			MessageType = 0;
			Dup = 0;
			QoS = 0;
			Retain = 0;
			RemainingLength[0] = 0;
		}
	};

///////////////// Helper Functions //////////////////////////
LOCAL ICACHE_FLASH_ATTR size_t GetStringLen(const byte *data, size_t idx)
	{
		size_t ret = data[idx] * 256;
		ret += data[idx + 1];
		return ret;
	}

LOCAL ICACHE_FLASH_ATTR size_t ExtractString(const byte *data, size_t idx, char *buf, size_t buflen)
	{
		size_t sl = data[idx] * 256;
		sl += data[idx + 1];
		os_memset(buf, 0, buflen);
		os_memcpy(buf, data + idx + 2, sl);
		size_t ret = idx + sl + 2;
		return ret;
	}

LOCAL ICACHE_FLASH_ATTR size_t InsertString(byte *buf, size_t idx, const char *str)
	{
		size_t len = strlen(str);
		buf[idx++] = (byte) ((len >> 8) & 0xff);
		buf[idx++] = (byte) ((len) & 0xff);
		while (len > 0)
			{
				buf[idx++] = (byte) *str;
				str++;
				len--;
			}
		return idx;
	}

LOCAL ICACHE_FLASH_ATTR size_t FillRemainingLen(byte *buf, size_t rl)
	{
		size_t idx = 1;
		do
			{
				byte digit = rl % 128;
				rl /= 128;
				if (rl > 0)
					digit |= 0x80;
				buf[idx++] = digit;
			} while (rl > 0);

		return idx;
	}

LOCAL ICACHE_FLASH_ATTR size_t InsertMessageID(byte *data,size_t idx,size_t msgid)
	{
		data[idx] = (msgid >>8) & 0xff;
		data[idx+1] = msgid & 0xff;
		return idx + 2;
	}

LOCAL size_t ICACHE_FLASH_ATTR getRemainingLen(const byte *data, size_t &len)
	{
		size_t ret = 1;
		len = 0;
		size_t multiplier = 1;
		byte digit;
		do
			{
				digit = data[ret++];
				len += (digit & 0x7f) * multiplier;
				multiplier *= 128;

			} while ((digit & 0x80) != 0);
		return ret;
	}

LOCAL ICACHE_FLASH_ATTR size_t GetBufLen(size_t rl)
	{
		size_t ret = rl + 2;
		if (rl > 127)
			ret++;
		if (rl > 16383)
			ret++;
		if (rl > 2097151)
			ret++;

		return ret;
	}

ICACHE_FLASH_ATTR MQTTSocket::MQTTSocket(MQTT_Connect_Params &parms, bool prepend_client_id) :
		m_parms(parms), msgs(20)
	{
		m_prepend_client_id = prepend_client_id;
		keepalive_timer = NULL;
		Connect(parms.host, parms.port);
	}

ICACHE_FLASH_ATTR void MQTTSocket::DoConnect()
	{
		size_t buflen = 2;	// header
		size_t rl = 12;	// version 3 variable header
		bool hasusername = false;
		bool haspassword = false;
		bool haswillpayload = false;
		bool haswilltopic = false;

		rl += strlen(m_parms.client_id) + 2;
		if (strlen(m_parms.password) > 0)
			{
				rl += strlen(m_parms.password) + 2;
				haspassword = true;
			}
		if (strlen(m_parms.username) > 0)
			{
				rl += strlen(m_parms.username) + 2;
				hasusername = true;
			}
		if (strlen(m_parms.willpayload) > 0)
			{
				rl += strlen(m_parms.willpayload) + 2;
				haswillpayload = true;
			}
		if (strlen(m_parms.willtopic) > 0)
			{
				rl += strlen(m_parms.willtopic) + 2;
				if(m_prepend_client_id)
					rl += strlen(m_parms.client_id) + 1;
				haswilltopic = true;
			}
		if (rl > 127)
			buflen++;
		if (rl > 16383)
			buflen++;
		if (rl > 2097151)
			buflen++;

		buflen += rl;
		byte buf[buflen];
		os_memset(buf, 0, buflen);
		MQTT_Header *ph = (MQTT_Header *) buf;
		ph->MessageType = MQTT_CONNECT;
		size_t idx = FillRemainingLen(buf, rl);
		idx = InsertString(buf, idx, "MQIsdp");
		buf[idx++] = 3;

		byte flags = 0;
		if (hasusername)
			flags |= CF_USERNAME;
		if (haspassword)
			flags |= CF_PASSWORD;
		if (haswilltopic)
			{
				flags |= CF_WILL | CF_WILLQOS(m_parms.willqos & 3);
				if (m_parms.willretain)
					flags |= CF_WILLRETAIN;
			}
		if (m_parms.cleansession)
			flags |= CF_CLEAN;

		buf[idx++] = flags;
		buf[idx++] = (m_parms.keepalive >> 8) & 0xff;
		buf[idx++] = (m_parms.keepalive) & 0xff;
		idx = InsertString(buf, idx, m_parms.client_id);
		if (haswilltopic)
			{
				if(m_prepend_client_id)
					{
						size_t x = strlen(m_parms.client_id) + 1;
						x += strlen(m_parms.willtopic);
						char preptopic[x+1];
						os_strcpy(preptopic,m_parms.client_id);
						os_strcat(preptopic,"/");
						os_strcat(preptopic,m_parms.willtopic);
						idx = InsertString(buf,idx,preptopic);
					}
				else
					idx = InsertString(buf, idx, m_parms.willtopic);
			}
		if (haswillpayload)
			idx = InsertString(buf, idx, m_parms.willpayload);
		if (hasusername)
			idx = InsertString(buf, idx, m_parms.username);
		if (haspassword)
			idx = InsertString(buf, idx, m_parms.password);

		if (idx != buflen)
			ets_uart_printf("Expected %d for idx but got %d\r\n", buflen, idx);

		Send(buf, buflen);

	}

ICACHE_FLASH_ATTR void MQTTSocket::OnConnect()
	{
		DoConnect();
	}

ICACHE_FLASH_ATTR void MQTTSocket::OnReconnect(sint8 err)
	{
		DoConnect();
	}

ICACHE_FLASH_ATTR void MQTTSocket::OnDisconnect()
	{
		ets_uart_printf("Disconnected!\r\n");
	}

ICACHE_FLASH_ATTR void MQTTSocket::GotDisconnect()
	{
		if(keepalive_timer != NULL)
			{
				keepalive_timer->Stop();
				delete keepalive_timer;
				keepalive_timer = NULL;
			}

		Socket::GotDisconnect();
	}

ICACHE_FLASH_ATTR void MQTTSocket::HandleMsgQueue()
	{
		if (msgs.length() > 0 && getState() == Connected)
			{
				MQTTMessage *p = msgs.Peek();
				if (p->handled == false)
					{
						Socket::Result r = Send(p->msg, p->len);
						if (r == Success)
							p->handled = true;
					}
				else
					{
						p = msgs.Pop();
						delete p;
						HandleMsgQueue();
					}
			}
	}

ICACHE_FLASH_ATTR void MQTTSocket::OnSent()
	{
		//ets_uart_printf("Sent Data\r\n");
		HandleMsgQueue();
	}

ICACHE_FLASH_ATTR void MQTTSocket::HandlePublishEvent(const byte *data, unsigned short len)
	{
		MQTT_Header *ph = (MQTT_Header *) data;
		Qos qos = (Qos) ph->QoS;
		unsigned short msgid = 0;
		size_t rl;
		size_t idx = getRemainingLen(data, rl);

		size_t topicsize = GetStringLen(data, idx);
		char topic[topicsize + 1];
		idx = ExtractString(data, idx, topic, topicsize + 1);
		rl -= topicsize + 2;
		if(qos != FireAndForget)
			{
				msgid = data[idx++] << 8;
				msgid |= data[idx++];
				rl -= 2;
			}
		size_t offset = 0;

		// strip off client id and '/' chars
		if(this->m_prepend_client_id)
			offset = os_strlen(this->m_parms.client_id) + 1;

		byte ack[4];
		memset(ack,0,sizeof(ack));
		ph = (MQTT_Header *)ack;
		switch(qos)
			{
		case FireAndForget:
			break;
		case AcknowledgedDelivery:
			ph->MessageType = MQTT_PUBACK;
			ph->RemainingLength[0] = 2;
			InsertMessageID(ack,2,msgid);
			Send(ack,sizeof(ack));
			break;
		case AssuredDelivery:
			ph->MessageType = MQTT_PUBREC;
			ph->RemainingLength[0] = 2;
			InsertMessageID(ack,2,msgid);
			Send(ack,sizeof(ack));
			break;
			}
		this->OnPublish(topic + offset, data + idx, rl);
	}

ICACHE_FLASH_ATTR void MQTTSocket::HandleConnAckEvent(const byte *data, unsigned short len)
	{
		if(keepalive_timer == NULL && m_parms.keepalive > 0)
			keepalive_timer = new MQTTKeepAliveTimer(this,m_parms.keepalive * 1000);

		this->OnConnack((ConnAckCode) data[3]);
	}

ICACHE_FLASH_ATTR void MQTTSocket::HandlePubAckEvent(const byte *data, unsigned short len)
	{
	}

ICACHE_FLASH_ATTR void MQTTSocket::OnReceive(uint8_t *data, unsigned short len)
	{
//		ets_uart_printf("Receiving Data from MQTT Server:\r\n");
//		dumpData(data,len);
		MQTT_Header *ph = (MQTT_Header *) data;
		switch (ph->MessageType)
			{
		//case MQTT_CONNECT:		// 	1 // Client request to connect to Server
		case MQTT_CONNACK:		// 	2 // Connect Acknowledgment
			HandleConnAckEvent(data,len);
			break;
		case MQTT_PUBLISH:		// 	3 // Publish message
			HandlePublishEvent(data, len);
			break;
		case MQTT_PUBACK:		// 	4 // Publish Acknowledgment
		case MQTT_PUBREC:		// 	5 // Publish Received (assured delivery part 1)
		case MQTT_PUBREL:		// 	6 // Publish Release (assured delivery part 2)
		case MQTT_PUBCOMP:		// 	7 // Publish Complete (assured delivery part 3)
		case MQTT_SUBSCRIBE:	// 	8 // Client Subscribe request
		case MQTT_SUBACK:		// 	9 // Subscribe Acknowledgment
		case MQTT_UNSUBSCRIBE:	// 10 // Client Unsubscribe request
		case MQTT_UNSUBACK:		// 	11 // Unsubscribe Acknowledgment
		case MQTT_PINGREQ:		// 	12 // PING Request
		case MQTT_PINGRESP:		// 	13 // PING Response
		case MQTT_DISCONNECT:	// 14 // Client is Disconnecting
		default:
			break;
			}
	}

ICACHE_FLASH_ATTR void MQTTSocket::QueueMessage(const byte *data,unsigned short len)
	{
		MQTTMessage *p = new MQTTMessage(data,len);
		msgs.Push(p);
	}

ICACHE_FLASH_ATTR MQTTSocket::OpResult MQTTSocket::Subscribe(const char *topic, Qos qos)
	{
		OpResult ret = Error;
		size_t topicsize = os_strlen(topic);
		if(this->m_prepend_client_id)
			topicsize += os_strlen(this->m_parms.client_id) + 1;

		char szTopic[topicsize+1];
		os_memset(szTopic,0,topicsize+1);
		if(this->m_prepend_client_id)
			{
				os_strcat(szTopic,m_parms.client_id);
				os_strcat(szTopic,"/");
			}
		os_strcat(szTopic,topic);
		size_t rl = os_strlen(szTopic) + 2;
		if(qos != FireAndForget)
			rl += 3;

		size_t buflen = GetBufLen(rl);
		byte data[buflen];
		os_memset(data,0,buflen);
		MQTT_Header *ph = (MQTT_Header *)data;
		ph->MessageType = MQTT_SUBSCRIBE;
		ph->QoS = qos;

		size_t idx = FillRemainingLen(data,rl);
		if(qos != FireAndForget)
			{
			idx = InsertMessageID(data,idx,NextMessageID());
			}
		idx = InsertString(data,idx,szTopic);
		data[idx++] = (byte)qos;


		if(getState() == Connected)
			{
				Socket::Result r = Send(data,buflen);
				if(r == Success)
					ret = Sent;
			}
		else
			{
				QueueMessage(data,buflen);
				ret = Queued;
			}

		return ret;
	}

ICACHE_FLASH_ATTR MQTTSocket::OpResult MQTTSocket::Publish(const char *topic, const byte *payload, size_t len, Qos qos, bool retain)
	{
		OpResult ret = Error;
		size_t topicsize = os_strlen(topic);
		if(this->m_prepend_client_id)
			topicsize += os_strlen(this->m_parms.client_id) + 1;

		char szTopic[topicsize+1];
		os_memset(szTopic,0,topicsize+1);
		if(this->m_prepend_client_id)
			{
				os_strcat(szTopic,m_parms.client_id);
				os_strcat(szTopic,"/");
			}
		os_strcat(szTopic,topic);
		size_t rl = os_strlen(szTopic) + 2;
		if(qos != FireAndForget)
			rl += 2;

		rl += len;

		size_t buflen = GetBufLen(rl);
		byte data[buflen];
		os_memset(data,0,buflen);
		MQTT_Header *ph = (MQTT_Header *)data;
		ph->MessageType = MQTT_PUBLISH;
		ph->QoS = qos;
		ph->Retain = retain;

		size_t idx = FillRemainingLen(data,rl);
		idx = InsertString(data,idx,szTopic);
		if(qos != FireAndForget)
			idx = InsertMessageID(data,idx,NextMessageID());

		os_memcpy(data+idx,payload,len);

		if(getState() == Connected)
			{
				Socket::Result r = Send(data,buflen);
				if(r == Success)
					ret = Sent;
			}
		else
			{
				QueueMessage(data,buflen);
				ret = Queued;
			}

		return ret;
	}

ICACHE_FLASH_ATTR void MQTTSocket::OnConnack(ConnAckCode x)
	{
	}

ICACHE_FLASH_ATTR void MQTTSocket::OnPublish(const char *topic, const byte *payload, size_t len)
	{
	char buf[len+1];
	os_memset(buf,0,len+1);
	os_memcpy(buf,payload,len);
	ets_uart_printf("Topic: '%s'\r\nMessage: '%s'\r\n",topic,buf);

	}

ICACHE_FLASH_ATTR Socket::Result MQTTSocket::Send(const uint8_t*data,unsigned short len)
	{
//		ets_uart_printf("Sending Data to MQTT Server:\r\n");
//		dumpData(data,len);

		return Socket::Send(data,len);

	}

ICACHE_FLASH_ATTR void MQTTSocket::KeepAlive()
	{
		MQTT_Header hdr;
		os_memset(&hdr,0,sizeof(hdr));
		hdr.MessageType = MQTT_PINGREQ;
		Send((const byte *)&hdr,sizeof(hdr));
	}

ICACHE_FLASH_ATTR void MQTTKeepAliveTimer::OnTime()
	{
		if(this->ptr != NULL)
			ptr->KeepAlive();
	}
