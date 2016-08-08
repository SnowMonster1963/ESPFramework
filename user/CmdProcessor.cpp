/*
 * CmdProcessor.cpp
 *
 *  Created on: Feb 28, 2016
 *      Author: tsnow
 */
#include <WiFiManager.h>
#include <ATMqtt.h>
#include <CmdProcessor.h>

#define SendOK(x) ets_uart_printf("OK\r\n")
#define SendError(x) ets_uart_printf("Error\r\n")

/*
 * 		Command Base		Query				Set				Command
 * 		AX					n/a					n/a				Test command (for debugging)
 * 		AT					n/a					n/a				Tests 'alive'
 * 		AT+STATUS			n/a					n/a				Displays '+WIFI Connected' or '+WIFI Disconnected'
 * 																and '+MQTT Connected' or '+MQTT Disconnected'
 * 																and 'OK'
 * 		AT+TOPIC			AT+TOPIC=?			AT+TOPIC="topic","value"
 * 																Query returns all active topics
 * 																Format is: +AT+TOPIC="topic",xxx:data
 * 		AT+MQTT				AT+MQTT=?			AT+MQTT="server",port,"user","password","willtopic","willpayload",willqos,willretain,cleansession,keepalive
 * 		AT+MQCON			n/a					n/a				Connects to MQTT server
 * 		AT+MQDIS			n/a					n/a				Disconnects from MQTT server
 * 		AT+CLIENTID			AT+CLIENTID=?		AT+CLIENTID="client id"
 * 																Query returns '+AT+CLIENTID="client id"' & "OK"
 * 																Max length is 23 chars
 * 																If set to "", uses ESP Network ID
 * 		AT+AUTOCON			AT+AUTOCON=?		AT+AUTOCON=x	Set auto reconnect functionality, where 'x' is 0 (do not reconnect) or 1 (reconnect if connection lost)
 * 		AT+LOG				AT+LOG=?			AT+LOG=x		Set Logging of OS messages on (1) or off (0)
 */

ICACHE_FLASH_ATTR void ErrorMsg(const char *msg)
	{
		ets_uart_printf("Error - %s\r\n",msg);
	}

class CmdLineProcessor
	{
public:
	CmdLineProcessor();

	static void Parse(const char *pszLine);

	// commands
	static void DoAtCmd(const char *cmd);
	static void DoAtStatusCmd(const char *cmd);
	static void DoAtTopicCmd(const char *cmd);
	static void DoAtMqttCmd(const char *cmd);
	static void DoAtMqconCmd(const char *cmd);
	static void DoAtMqdisCmd(const char *cmd);
	static void DoAtClientidCmd(const char *cmd);
	static void DoAtAutoconnCmd(const char *cmd);
	static void DoAtLogCmd(const char *cmd);
	static void DoAtTestCmd(const char *cmd);

	// queries
	static void DoAtTopicQry(const char *cmd);
	static void DoAtMqttQry(const char *cmd);
	static void DoAtClientidQry(const char *cmd);
	static void DoAtAutoconnQry(const char *cmd);
	static void DoAtLogQry(const char *cmd);
	};


struct CmdTableItem
	{
	const char *str;
	void (*func)(const char *);
	};

CmdTableItem CmdTable[] =
		{
				{"AX\r\n",CmdLineProcessor::DoAtTestCmd},
				{"AT\r\n",CmdLineProcessor::DoAtCmd},
				{"AT+STATUS\r\n",CmdLineProcessor::DoAtStatusCmd},
				{"AT+TOPIC=?\r\n",CmdLineProcessor::DoAtTopicQry},
				{"AT+MQTT=?\r\n",CmdLineProcessor::DoAtMqttQry},
				{"AT+CLIENTID=?\r\n",CmdLineProcessor::DoAtClientidQry},
				{"AT+CLIENTID=\"",CmdLineProcessor::DoAtClientidCmd},
				{"AT+TOPIC=\"",CmdLineProcessor::DoAtTopicCmd},
				{"AT+MQTT=\"",CmdLineProcessor::DoAtMqttCmd},
				{"AT+MQCON\r\n",CmdLineProcessor::DoAtMqconCmd},
				{"AT+MQDIS\r\n",CmdLineProcessor::DoAtMqdisCmd},
				{"AT+AUTOCON=?\r\n",CmdLineProcessor::DoAtAutoconnQry},
				{"AT+AUTOCON=",CmdLineProcessor::DoAtAutoconnCmd},
				{"AT+LOG=?\r\n",CmdLineProcessor::DoAtLogQry},
				{"AT+LOG=",CmdLineProcessor::DoAtLogCmd},
		};


ICACHE_FLASH_ATTR CmdLineProcessor::CmdLineProcessor()
	{

	}

ICACHE_FLASH_ATTR void CmdLineProcessor::DoAtCmd(const char *cmd)
	{
		SendOK();
	}

ICACHE_FLASH_ATTR void CmdLineProcessor::DoAtStatusCmd(const char *cmd)
	{

		if(wifiproc != NULL && wifiproc->IsConnected())
			ets_uart_printf("+WIFI Connected\r\n");
		else
			ets_uart_printf("+WIFI Disconnected\r\n");

		if(pSock != NULL && pSock->getState() == Socket::Connected)
			ets_uart_printf("+MQTT Connected\r\n");
		else
			ets_uart_printf("+MQTT Disconnected\r\n");
		SendOK();

	}

ICACHE_FLASH_ATTR void CmdLineProcessor::DoAtTopicCmd(const char *cmd)
	{
		size_t len = os_strlen(cmd);
		char buf[len+1];
		os_strcpy(buf,cmd);

		if(pSock == NULL || pSock->getState() != Socket::Connected)
			{
				SendError();
				return;
			}

		// format: AT+TOPIC="topic","value"

		// topic
		char *p = os_strstr(buf,"\"");
		if(p == NULL)
			{
				ErrorMsg("Invalid Topic");
				return;
			}

		char *topic = p+1;
		p = os_strstr(topic,"\"");
		if(p == NULL)
			{
				ErrorMsg("Invalid Topic");
				return;
			}
		*p = 0;
		p += 2;

		// value
		if(*p != '\"')
			{
				ErrorMsg("Invalid Topic Value");
				return;
			}
		char *value = p+1;
		p = os_strstr(value,"\"");
		if(p == NULL)
			{
				ErrorMsg("Invalid Topic Value");
				return;
			}
		*p = 0;

		pSock->updateTopic(topic,value);
		SendOK();
	}

ICACHE_FLASH_ATTR void CmdLineProcessor::DoAtMqttCmd(const char *cmd)
	{
		size_t len = os_strlen(cmd);
		char buf[len+1];
		os_strcpy(buf,cmd);


		// format: AT+MQTT="server",port,"user","password","willtopic","willpayload",willqos,willretain,cleansession,keepalive

		// server
		char *p = os_strstr(buf,"\"");
		if(p == NULL)
			{
				ErrorMsg("Invalid Server");
				return;
			}

		char *server = p+1;
		p = os_strstr(server,"\"");
		if(p == NULL)
			{
				ErrorMsg("Invalid Server");
				return;
			}
		*p = 0;
		p += 2;


		// port
		int port = atoi(p);
		p = os_strstr(p,",");
		if(p == NULL || port == 0)
			{
				ErrorMsg("Invalid Port");
				return;
			}

		p++;

		// user
		if(*p != '\"')
			{
				ErrorMsg("Invalid User");
				return;
			}
		char *username = p+1;
		p = os_strstr(username,"\"");
		if(p == NULL)
			{
				ErrorMsg("Invalid User");
				return;
			}
		*p = 0;
		p += 2;

		// password
		if(*p != '\"')
			{
				ErrorMsg("Invalid Password");
				return;
			}
		char *password = p+1;
		p = os_strstr(password,"\"");
		if(p == NULL)
			{
				ErrorMsg("Invalid Password");
				return;
			}
		*p = 0;
		p += 2;


		// will topic
		if(*p != '\"')
			{
				ErrorMsg("Invalid Will Topic");
				return;
			}
		char *willtopic = p+1;
		p = os_strstr(willtopic,"\"");
		if(p == NULL)
			{
				ErrorMsg("Invalid Will Topic");
				return;
			}
		*p = 0;
		p += 2;

		// will payload
		if(*p != '\"')
			{
				ErrorMsg("Invalid Will Payload");
				return;
			}
		char *willpayload = p+1;
		p = os_strstr(willpayload,"\"");
		if(p == NULL)
			{
				ErrorMsg("Invalid Will Payload");
				return;
			}
		*p = 0;
		p += 2;

		// willqos
		int willqos = atoi(p);
		p = os_strstr(p,",");
		if(p == NULL || willqos > 2)
			{
				ErrorMsg("Invalid Will QOS");
				return;
			}

		p++;

		// willretain
		int willretain = atoi(p);
		p = os_strstr(p,",");
		if(p == NULL || willretain > 1)
			{
				ErrorMsg("Invalid Will Retain Flag");
				return;
			}

		p++;

		// cleansession
		int cleansession = atoi(p);
		p = os_strstr(p,",");
		if(p == NULL || cleansession > 1)
			{
				ErrorMsg("Invalid Clean Session Flag");
				return;
			}

		p++;

		// keepalive
		int keepalive = atoi(p);

		MQTT_Connect_Params mp;
		MQTT_Connect_Params *pmp = &mqttparms;
		os_memset(&mp,0,sizeof(mp));
		os_strcpy(mp.client_id,pmp->client_id);
		if(strlen(server) >= sizeof(mp.host))
			{
				ErrorMsg("Server Name too long");
				return;
			}
		os_strcpy(mp.host,server);
		mp.port = port;
		mp.cleansession = cleansession;
		mp.keepalive = keepalive;
		mp.willqos = willqos;
		mp.willretain = willretain;
		if(strlen(password) >= sizeof(mp.password))
			{
				ErrorMsg("Password too long");
				return;
			}
		os_strcpy(mp.password,password);

		if(strlen(username) >= sizeof(mp.username))
			{
				ErrorMsg("Username too long");
				return;
			}
		os_strcpy(mp.username,username);

		if(strlen(willpayload) >= sizeof(mp.willpayload))
			{
				ErrorMsg("Will Payload too long");
				return;
			}
		os_strcpy(mp.willpayload,willpayload);

		if(strlen(willtopic) >= sizeof(mp.willtopic))
			{
				ErrorMsg("Will Topic too long");
				return;
			}
		os_strcpy(mp.willtopic,willtopic);

		*pmp = mp;
		mqttparms.SaveData();
		SendOK();
	}

ICACHE_FLASH_ATTR void CmdLineProcessor::DoAtMqconCmd(const char *cmd)
	{
		if(pSock == NULL)
			pSock = new MyMqtt();

		if(pSock->IsConnected() == false)
			{
				pSock->autoreconnect = true;
				pSock->Reconnect();
				SendOK();
			}
		else
			{
				ErrorMsg("Already Connected");
			}
	}

ICACHE_FLASH_ATTR void CmdLineProcessor::DoAtMqdisCmd(const char *cmd)
	{
		if(pSock != NULL)
			{
				pSock->autoreconnect = false;
				if(pSock->IsConnected())
					{
						pSock->Close();
						SendOK();
					}
				else
					{
						ErrorMsg("Not Connected");
					}
			}
		else
			{
				ErrorMsg("Not Connected");
			}
	}

ICACHE_FLASH_ATTR void CmdLineProcessor::DoAtClientidCmd(const char *cmd)
	{
		size_t len = os_strlen(cmd);
		char buf[len+1];
		os_strcpy(buf,cmd);


		// format: AT+CLIENTID="client id"

		// client ID
		char *p = os_strstr(buf,"\"");
		if(p == NULL)
			{
				SendError();
				return;
			}

		char *clientid = p+1;
		p = os_strstr(clientid,"\"");
		if(p == NULL)
			{
				SendError();
				return;
			}
		*p = 0;

		MQTT_Connect_Params *pmp = &mqttparms;
		len = os_strlen(clientid);
		if(len >= sizeof(pmp->client_id))
			{
				ErrorMsg("Client ID too long");
				return;
			}
		if(len > 0)
			os_strcpy(pmp->client_id,clientid);
		else
			os_strcpy(pmp->client_id,wifi.GetHostName());

		mqttparms.SaveData();
		if(pSock != NULL && pSock->IsConnected())
			pSock->Subscribe("set/#");
		SendOK();
	}

ICACHE_FLASH_ATTR void CmdLineProcessor::DoAtAutoconnCmd(const char *cmd)
	{
		int x = atoi(cmd + os_strlen("AT+AUTOCON="));
		if(x > 0)
			{
				pSock->autoreconnect = true;
				if(pSock->IsConnected() == false)
					pSock->Reconnect();
			}
		else
			{
				pSock->autoreconnect = false;
			}
		SendOK();
	}

ICACHE_FLASH_ATTR void CmdLineProcessor::DoAtAutoconnQry(const char *cmd)
	{
		ets_uart_printf("+AT+AUTOCON=%d\r\n", pSock->autoreconnect);
		SendOK();
	}


ICACHE_FLASH_ATTR void CmdLineProcessor::DoAtTopicQry(const char *cmd)
	{

		if(pSock != NULL)
			{
				pSock->printTopics();
				SendOK();
			}
		else
			SendError();
	}

ICACHE_FLASH_ATTR void CmdLineProcessor::DoAtMqttQry(const char *cmd)
	{
		MQTT_Connect_Params *pmqttparms = &mqttparms;
		// format: +AT+MQTT="server",port,"user","password","willtopic","willpayload",willqos,willretain,cleansession,keepalive
		ets_uart_printf("+AT+MQTT=\"%s\",%d,\"%s\",\"%s\",\"%s\",\"%s\",%d,%d,%d,%d\r\n"
				,pmqttparms->host
				,pmqttparms->port
				,pmqttparms->username
				,pmqttparms->password
				,pmqttparms->willtopic
				,pmqttparms->willpayload
				,pmqttparms->willqos
				,pmqttparms->willretain
				,pmqttparms->cleansession
				,pmqttparms->keepalive
				);
		SendOK();
	}

ICACHE_FLASH_ATTR void CmdLineProcessor::DoAtClientidQry(const char *cmd)
	{
		MQTT_Connect_Params *pmqttparms = &mqttparms;
		// format: +AT+CLIENTID="client id"
		ets_uart_printf("+AT+CLIENTID=\"%s\"\r\n"
				,pmqttparms->client_id
				);
		SendOK();
	}

ICACHE_FLASH_ATTR void CmdLineProcessor::Parse(const char *cmd)
	{
		for(int i=0;i<sizeof(CmdTable)/sizeof(CmdTable[0]);i++)
			{
				if(os_strstr(cmd,CmdTable[i].str) != NULL)
					{
						CmdTable[i].func(cmd);
						return;
					}
			}
		SendError();
	}

ICACHE_FLASH_ATTR void CmdLineProcessor::DoAtLogQry(const char *cmd)
	{
		ets_uart_printf("+AT+LOG=%d\r\n", (int)system_get_os_print());
		SendOK();
	}

ICACHE_FLASH_ATTR void CmdLineProcessor::DoAtLogCmd(const char *cmd)
	{
		int x = atoi(cmd + os_strlen("AT+LOG="));
		if(x > 0)
			{
			    system_set_os_print(1);
			}
		else
			{
			    system_set_os_print(0);
			}
		SendOK();
	}

ICACHE_FLASH_ATTR void CmdLineProcessor::DoAtTestCmd(const char *cmd)
	{
		struct softap_config cfg;
		/*
		 * struct softap_config {
			uint8 ssid[32];
			uint8 password[64];
			uint8 ssid_len;	// Note: Recommend to set it according to your ssid
			uint8 channel;	// Note: support 1 ~ 13
			AUTH_MODE authmode;	// Note: Don't support AUTH_WEP in softAP mode.
			uint8 ssid_hidden;	// Note: default 0
			uint8 max_connection;	// Note: default 4, max 4
			uint16 beacon_interval;	// Note: support 100 ~ 60000 ms, default 100
		};
		 */

		ets_uart_printf("wifi_softap_dhcps_status = %d\r\n",wifi_softap_dhcps_status());
		ets_uart_printf("wifi_get_phy_mode = %d\r\n",wifi_get_phy_mode());

		wifi_softap_get_config(&cfg);
		ets_uart_printf("softap_config.ssid = %s\r\n",(char*)cfg.ssid);
		ets_uart_printf("softap_config.password = %s\r\n",(char*)cfg.password);
		ets_uart_printf("softap_config.ssid_len = %d\r\n",(int)cfg.ssid_len);
		ets_uart_printf("softap_config.channel = %d\r\n",(int)cfg.channel);
		ets_uart_printf("softap_config.authmode = %d\r\n",(int)cfg.authmode);
		ets_uart_printf("softap_config.ssid_hidden = %d\r\n",(int)cfg.ssid_hidden);
		ets_uart_printf("softap_config.max_connection = %d\r\n",(int)cfg.max_connection);
		ets_uart_printf("softap_config.beacon_interval = %d\r\n",(int)cfg.beacon_interval);
		cfg.channel = 1;

		wifi_softap_set_config(&cfg);
		wifi_set_phy_mode(PHY_MODE_11G);

		SendOK();
	}



ICACHE_FLASH_ATTR MyEOLProcess::MyEOLProcess()
	{

	}

ICACHE_FLASH_ATTR void MyEOLProcess::OnLineSent(const char *pszLine)
	{
		CmdLineProcessor::Parse(pszLine);
	}

EOLProcess *eolp;


