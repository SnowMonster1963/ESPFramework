/*
 * Socket.h
 *
 *  Created on: Jan 5, 2016
 *      Author: tsnow
 */

#ifndef INCLUDE_DRIVER_SOCKET_H_
#define INCLUDE_DRIVER_SOCKET_H_

extern "C"
{
#include <string.h>
#include <ets_sys.h>
#include <ip_addr.h>
#include <espconn.h>
#include <osapi.h>
#include <os_type.h>
#include <c_types.h>
#include <user_interface.h>
#include "espmissingincludes.h"
extern int ets_uart_printf(const char *fmt, ...);
}
#include "driver/Process.h"

class Socket
{
public:
	enum Result
	{
		Success,
		Pending,
		Busy,
		Error,
	};

	enum State
	{
		Unconnected,
		Sending,
		Connecting,
		Connected,
	};

private:
	bool	m_bOwnConnection;
	espconn	*m_pConnection;
	State	m_State;

	static void ConnectCallback(void *arg);
	static void ReconnectCallback(void *arg,sint8 err);
	static void DisconnectCallback(void *arg);
	static void SentCallback(void *arg);
	static void ReceiveCallback(void *arg,char *data,unsigned short len);
	static void DnsFoundCallback(const char *host,ip_addr_t *ip,void *param);


	void ProcessSendQueue();


protected:
	void GotConnect();
	void GotReconnect(sint8 err);
	void GotDisconnect();
	void GotSent();
	void GotReceive(uint8_t *data,unsigned short len);
	void GotDnsFound(const char *host,ip_addr_t *ip);

	virtual void OnConnect();
	virtual void OnReconnect(sint8 err);
	virtual void OnDisconnect();
	virtual void OnSent();
	virtual void OnReceive(uint8_t *data,unsigned short len);
	virtual void OnDnsFound(const char *host,ip_addr_t *ip);

public:
	Socket();
	Socket(espconn *pConn);

	bool IsConnected();

	State getState(){return m_State;};

	Result Connect(const char *host,int port);
	Result Connect(const ip_addr_t *ip,int port);
	Result Send(const uint8_t*data,unsigned short len);
	Result Send(const char *str)
	{
		unsigned short len = os_strlen(str);
		return Send((const uint8_t *)str,len);
	}

	virtual ~Socket();
};



#endif /* INCLUDE_DRIVER_SOCKET_H_ */
