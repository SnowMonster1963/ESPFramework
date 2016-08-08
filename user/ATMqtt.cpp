/*
 * ATMqtt.cpp
 *
 *  Created on: Feb 27, 2016
 *      Author: tsnow
 */

#include <ctype.h>
#include <ATMqtt.h>


ConfigManager<MQTT_Connect_Params> mqttparms;

// comment out define in ATMqtt.h to initialize flash with structure below
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

ICACHE_FLASH_ATTR MyMqtt::MyMqtt() : MQTTSocket(mqttparms)
	{
		os_memset(arry,0,sizeof(arry));
		topic_len = 0;
		autoreconnect = true;
		//pRetry = NULL;
	}

ICACHE_FLASH_ATTR void MyMqtt::printTopic(size_t idx)
	{
		if(idx < topic_len)
			{
				bool isstring = true;
				size_t i = 0;
				while(isstring == true && i < arry[idx].len)
					{
						int c = (int) arry[idx].Value[i];
						if(isspace(c) == 0 && isprint(c) == 0)
							isstring = false;
						i++;
					}

				if(isstring)
					{
						ets_uart_printf("+AT+TOPIC=\"%s\",\"",arry[idx].Topic);
						if(arry[idx].len > 0)
							UART.send(arry[idx].Value,arry[idx].len);
						ets_uart_printf("\"");

					}
				else
					{
						ets_uart_printf("+AT+TOPIC=\"%s\",%d:",arry[idx].Topic,arry[idx].len);
						if(arry[idx].len > 0)
							UART.send(arry[idx].Value,arry[idx].len);
					}
				ets_uart_printf("\r\n");
			}
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
				os_printf("Client ID:  %s\r\n",mqttparms.client_id);
				ets_uart_printf("+MQTT Connected\r\n");
				Subscribe("set/#");
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
								arry[i].len = 0;
							}
						if(len > 0)
							{
								arry[i].Value = new byte[len+1];
								os_memset(arry[i].Value,0,len+1);
								arry[i].len = len;
								os_memcpy(arry[i].Value,payload,len);
							}
						//printTopic(i);
						ret = i;

						// if topic value is null, remove the topic from list
						if(len == 0)
							{
								for(size_t n=i;n<(MAX_TOPICS-1);n++)
									{
										arry[n] = arry[n+1];
									}
								arry[MAX_TOPICS-1].Topic = NULL;
								arry[MAX_TOPICS-1].Value = NULL;
								arry[MAX_TOPICS-1].len = 0;
								topic_len--;
							}
					}
			}

		// only add non-null topics
		if(found == false && topic_len < MAX_TOPICS && len > 0)
			{
				arry[topic_len].Topic = new char[strlen(Topic)+1];
				os_strcpy(arry[topic_len].Topic,Topic);
				arry[topic_len].Value = new byte[len+1];
				os_memset(arry[topic_len].Value,0,len+1);
				arry[topic_len].len = len;
				os_memcpy(arry[topic_len].Value,payload,len);
				//printTopic(topic_len);
				ret = topic_len;
				topic_len++;
			}

		// acknowledge the topic was received
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
			printTopic(idx);
		}

	}

ICACHE_FLASH_ATTR void MyMqtt::OnDisconnect()
	{
		ets_uart_printf("+MQTT Disconnected\r\n");
		if(autoreconnect == true)
			{
			Reconnect();
//			if(pRetry == NULL)
//				pRetry = new MQTTRetryTimer(this,5000);
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


//ICACHE_FLASH_ATTR void MQTTRetryTimer::OnTime()
//	{
//		if(ptr->autoreconnect && ptr->IsConnected() == false)
//			ptr->Reconnect();
//	}

MyMqtt *pSock = NULL;
