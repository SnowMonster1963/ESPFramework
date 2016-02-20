/*
	The hello world c++ demo
*/

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

#define DELAY 1000 /* milliseconds */

ConfigManager<MQTT_Connect_Params> mqttparms;

// comment out next line to initialize flash with structure below
#define ALREADY_SAVED
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

// =============================================================================================
// C includes and declarations
// =============================================================================================
extern "C"
{
#include "driver/uart.h"
// declare lib methods
extern int ets_uart_printf(const char *fmt, ...);
extern UartDevice UartDev;
}//extern "C"




// =============================================================================================
// Pointers to the constructors of the global objects
// (defined in the linker script eagle.app.v6.ld)
// =============================================================================================

extern void (*__init_array_start)(void);
extern void (*__init_array_end)(void);

// Initialyzer of the global objects
static void ICACHE_FLASH_ATTR do_global_ctors(void)
{
    void (**p)(void);
    for (p = &__init_array_start; p != &__init_array_end; ++p)
            (*p)();
}

// =============================================================================================
// User code
// =============================================================================================


extern "C" void ICACHE_FLASH_ATTR user_rf_pre_init(void)
{
}

#define MAX_TOPICS 20

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

	public:
	MyMqtt();
	void OnConnack(ConnAckCode x);
	void OnPublish(const char *topic, const byte *payload, size_t len);
	void printTopics();
	size_t updateTopic(const char *Topic,const byte *data,size_t len);
	size_t updateTopic(const char *Topic,const char *data);
	};

ICACHE_FLASH_ATTR MyMqtt::MyMqtt() : MQTTSocket(mqttparms)
	{
		os_memset(arry,0,sizeof(arry));
		topic_len = 0;
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

MyMqtt *pSock = NULL;

class MyWifiStatus : public WiFiStatusProcess
{
public:
	MyWifiStatus(){};

protected:
	void OnStationConnected(const char *ssid,uint8_t channel)
	{
		ets_uart_printf("MyWifiStatus:  Connected to %s, channel %d\r\n",ssid,channel);
	};
	void OnStationDisconnected(const char *ssid,uint8_t reason){};
	void OnStationAuthChange(AUTH_MODE oldmode, AUTH_MODE newmode){};
	void OnStationGotIP(ip_addr ip, ip_addr mask, ip_addr gateway)
	{
		ets_uart_printf("MyWifiStatus:  ip:" IPSTR ", mask:" IPSTR ", gw:" IPSTR,
		IP2STR(&ip),
		IP2STR(&mask),
		IP2STR(&gateway));
		ets_uart_printf("\r\n");
		pSock = new MyMqtt();
	};
	void OnAccessClientConnected(const uint8_t *mac,uint8_t aid){};
	void OnAccessClientDisconnected(const uint8_t *mac,uint8_t aid){};
};


class MyTimer : public Timer
{
public:
	MyTimer() : Timer(DELAY,true){saved = false;};
	~MyTimer(){};

	bool saved;

	void OnTime()
	{
		if(pSock != NULL && pSock->getState() == Socket::Connected)
		{
			static int ctr = 0;
			char buf[256];
			os_sprintf(buf,"Hello world - %d!",ctr++);
			pSock->Publish("test",buf);
		}

	}
};


LOCAL MyTimer *pTimer = NULL;

class MyEOLProcess : public EOLProcess
	{
public:
	MyEOLProcess();
	void OnLineSent(const char *pszLine);
	};

ICACHE_FLASH_ATTR MyEOLProcess::MyEOLProcess()
	{

	}

ICACHE_FLASH_ATTR void MyEOLProcess::OnLineSent(const char *pszLine)
	{
		//dumpData((byte *)pszLine,strlen(pszLine));

		// AT\r\n - Send "OK\r\n"
		if(os_strcmp(pszLine,"AT\r\n") == 0)
			{
			ets_uart_printf("OK\r\n");
			}
		else if(os_strcmp(pszLine,"AT+STATUS\r\n") == 0) // AT+STATUS - return Connected or Disconnected
			{
				if(pSock->IsConnected() )
					ets_uart_printf("Connected\r\nOK\r\n");
				else
					ets_uart_printf("Disconnected\r\nOK\r\n");
			}
		else if(os_strcmp(pszLine,"AT+TOPIC=?\r\n") == 0) // AT+TOPIC=? - print topics and "OK\r\n"
			{
				pSock->printTopics();
				ets_uart_printf("OK\r\n");
			}
		else if(os_strstr(pszLine,"AT+TOPIC=\"") != NULL) // AT+TOPIC="topic","value" - prints OK
			{
				char buf[os_strlen(pszLine) + 1];
				os_strcpy(buf,pszLine);
				char *topic = strstr(buf,"\"");
				if(topic != NULL)
					{
						topic++;
						char *data = os_strstr(topic,"\"");
						if(data != NULL)
							{
								*data = 0;
								data += 3;
								char *pend = os_strstr(data,"\"");
								if(pend != NULL)
									{
										*pend = 0;
										if(pSock->updateTopic(topic,data) < MAX_TOPICS)
											ets_uart_printf("OK\r\n");
										else
											ets_uart_printf("Too Many Topics\r\n");
									}
								else
									ets_uart_printf("Error\r\n");
							}
						else
							ets_uart_printf("Error\r\n");
					}
				else
					ets_uart_printf("Error\r\n");
			}
		else
			{
			dumpData((byte *)pszLine,strlen(pszLine));
			}
	}

EOLProcess *eolp;

extern "C" void ICACHE_FLASH_ATTR user_init(void)
{
	do_global_ctors();
	// Configure the UART
	//uart_init(BIT_RATE_115200, BIT_RATE_115200);
	ets_uart_printf("System init...\r\n");

	MyWifiStatus *proc = new MyWifiStatus();
	wifi.AttachWiFiEventProcess(proc);
#ifndef ALREADY_SAVED
	MQTT_Connect_Params *pmqttparms = &mqttparms;
	*pmqttparms = staticmqttparms;
	mqttparms.SaveData();
#endif

	// for now, we are going to overwrite the client ID to be the same as the WiFi Hostname
	os_strcpy(mqttparms.client_id,wifi.GetHostName());
	char buf[256];

	if(wifi.IsStation())
		ets_uart_printf("In Station Mode...\r\n");
	else
		wifi.EnableStation();

	if(wifi.IsAccessPoint())
		ets_uart_printf("Is Access Point...\r\n");
	else
		wifi.EnableAccessPoint();

	pTimer = new MyTimer();
	eolp = new MyEOLProcess();
	UART.SetEOLTask(eolp);

	ets_uart_printf("System init done.\r\nReady\r\n");
}
