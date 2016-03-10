/*
 * ATMqtt.cpp
 *
 *  Created on: Feb 27, 2016
 *      Author: tsnow
 */

#include <ATMqtt.h>


ConfigManager<MQTT_Connect_Params> mqttparms;

// comment out define in ATMqtt.h to initialize flash with structure below
#ifndef ALREADY_SAVED
MQTT_Connect_Params staticmqttparms  =
	{
			"www.yourhost.com",	//const char host[128]; ,	//
			1883,			// uint16_t port;  14134
			"YourClientID",		// const char client_id[24];
			"yourusername",		// const char username[16];
			"yourpassword",		// const char password[16];
			"Active",		// const char willtopic[128];
			"No",			// const char willpayload[128];
			2,			// byte willqos;
			true,			// bool willretain;
			true,			// bool cleansession;
			10			// unsigned int keepalive;

		};
#endif



ICACHE_FLASH_ATTR MyMqtt::MyMqtt() : MQTTSocket(mqttparms)
	{
		os_memset(arry,0,sizeof(arry));
		topic_len = 0;
		autoreconnect = true;
	}

ICACHE_FLASH_ATTR void MyMqtt::printTopic(size_t idx)
	{
		ets_uart_printf("+AT+TOPIC=\"%s\",%d:",arry[idx].Topic,arry[idx].len);
		UART.send(arry[idx].Value,arry[idx].len);
		ets_uart_printf("\r\n");
	}

ICACHE_FLASH_ATTR void MyMqtt::printTopics()
	{
		for(size_t i=0;i<topic_len;i++)
			printTopic(i);
	}

ICACHE_FLASH_ATTR void MyMqtt::OnConnack(ConnAckCode x)
	{
		if(x == Accepted)
			{
				Publish(mqttparms.willtopic,"Yes");
				Subscribe("set/#");
				ets_uart_printf("Client ID:  %s\r\n",mqttparms.client_id);
				ets_uart_printf("+MQTT Connected\r\n");
			}
	}

ICACHE_FLASH_ATTR size_t MyMqtt::updateTopic(const char *Topic,const byte *payload,size_t len)
	{
		bool found = false;
		size_t ret = MAX_TOPICS;
		for(size_t i=0;i<topic_len;i++)
			{
				if(os_strcmp(Topic,arry[i].Topic) == 0)
					{
						found = true;
						if(arry[i].Value != NULL)
							{
								delete [] arry[i].Value;
								arry[i].Value = NULL;
							}
						if(len > 0)
							{
								arry[i].Value = new byte[len+1];
								os_memset(arry[i].Value,0,len+1);
								arry[i].len = len;
								os_memcpy(arry[i].Value,payload,len);
							}
						printTopic(i);
						ret = i;
					}
			}
		if(found == false && topic_len < MAX_TOPICS)
			{
				arry[topic_len].Topic = new char[strlen(Topic)+1];
				os_strcpy(arry[topic_len].Topic,Topic);
				arry[topic_len].Value = new byte[len+1];
				os_memset(arry[topic_len].Value,0,len+1);
				arry[topic_len].len = len;
				os_memcpy(arry[topic_len].Value,payload,len);
				printTopic(topic_len);
				ret = topic_len;
				topic_len++;
			}

		char buf[os_strlen(Topic) + 7];
		os_strcpy(buf,"value/");
		os_strcat(buf,Topic);
		Publish(buf,payload,len);

		return ret;
	}

ICACHE_FLASH_ATTR size_t MyMqtt::updateTopic(const char *Topic,const char *payload)
	{
		return updateTopic(Topic,(const byte *)payload,os_strlen(payload));
	}

ICACHE_FLASH_ATTR void MyMqtt::OnPublish(const char *topic, const byte *payload, size_t len)
	{
	if(os_strstr(topic,"set/") != NULL)
		{
			size_t idx = updateTopic(topic+4,payload,len);
		}

	}

ICACHE_FLASH_ATTR void MyMqtt::OnDisconnect()
	{
		ets_uart_printf("+MQTT Disconnected\r\n");
		if(autoreconnect == true)
			{
			Reconnect();
			}
	}

ICACHE_FLASH_ATTR Socket::Result MyMqtt::Disconnect()
	{
		autoreconnect = false;
		return MQTTSocket::Disconnect();
	}

ICACHE_FLASH_ATTR Socket::Result MyMqtt::Reconnect()
	{
		autoreconnect = true;
		return MQTTSocket::Reconnect();
	}

MyMqtt *pSock = NULL;