/*
	The hello world c++ demo
*/

#include <ATMqtt.h>
#include <CmdProcessor.h>
#include <WiFiManager.h>

#define DELAY 1000 /* milliseconds */



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



extern "C" void ICACHE_FLASH_ATTR user_init(void)
{
	do_global_ctors();
	// Configure the UART
	//uart_init(BIT_RATE_115200, BIT_RATE_115200);
	ets_uart_printf("System init...\r\n");

	MyWifiStatus *proc = MyWifiStatus::GetProcess();
	wifi.AttachWiFiEventProcess(proc);
#ifndef ALREADY_SAVED
	MQTT_Connect_Params *pmqttparms = &mqttparms;
	*pmqttparms = staticmqttparms;
	mqttparms.SaveData();
#endif

	// for now, we are going to overwrite the client ID to be the same as the WiFi Hostname
	if(os_strlen(mqttparms.client_id) == 0)
		{
			os_strcpy(mqttparms.client_id,wifi.GetHostName());
			mqttparms.SaveData();
		}
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
