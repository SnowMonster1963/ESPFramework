/*
 * Socket.cpp
 *
 *  Created on: Jan 5, 2016
 *      Author: tsnow
 */

#include "driver/Stream.h"
#include "driver/Socket.h"

ICACHE_FLASH_ATTR LOCAL Socket::Result MapErrorCode(err_t err)
{
	Socket::Result ret = Socket::Success;
	switch (err)
	{
	case ESPCONN_OK:				//  0    /* No error, everything OK. */
		ret = Socket::Success;
		break;
	case ESPCONN_MEM:				//  -1    /* Out of memory error.     */
	case ESPCONN_TIMEOUT:			//  -3    /* Timeout.                 */
	case ESPCONN_RTE:				//  -4    /* Routing problem.         */
		ret = Socket::Error;
		break;
	case ESPCONN_INPROGRESS:		//  -5    /* Operation in progress    */
		ret = Socket::Pending;
		break;
	case ESPCONN_MAXNUM:	//  -7	 /* Total number exceeds the set maximum*/
	case ESPCONN_ABRT:				//  -8    /* Connection aborted.      */
	case ESPCONN_RST:				//  -9    /* Connection reset.        */
	case ESPCONN_CLSD:				//  -10   /* Connection closed.       */
	case ESPCONN_CONN:				//  -11   /* Not connected.           */
	case ESPCONN_ARG:				//  -12   /* Illegal argument.        */
	case ESPCONN_IF:				//  -14	 /* UDP send error			 */
	case ESPCONN_ISCONN:			//  -15   /* Already connected.       */
	case ESPCONN_HANDSHAKE:			//  -28   /* ssl handshake failed	 */
	case ESPCONN_SSL_INVALID_DATA:	//  -61   /* ssl application invalid	 */
	default:
		ret = Socket::Error;
		break;
	}

	//ets_uart_printf("Error value is %d\n",(int)err);

	return ret;
}

ICACHE_FLASH_ATTR Socket::Socket()
{
	m_pConnection = new espconn;
	memset(m_pConnection, 0, sizeof(espconn));
	m_pConnection->proto.tcp = new esp_tcp; // union has same members at beginning as udp
	m_pConnection->reverse = this;
	m_pConnection->type = ESPCONN_INVALID;
	m_pConnection->state = ESPCONN_NONE;
	m_State = Unconnected;

	m_bOwnConnection = true;
}

ICACHE_FLASH_ATTR Socket::Socket(espconn *pConn)
{
	m_pConnection = pConn;
	m_pConnection->reverse = this;

	// need to check whether this is true
	m_pConnection->type = ESPCONN_INVALID;
	m_pConnection->state = ESPCONN_NONE;
	m_State = Unconnected;

	m_bOwnConnection = false;
}

ICACHE_FLASH_ATTR Socket::~Socket()
{
	if (m_bOwnConnection)
	{
		if (m_pConnection->proto.tcp != NULL)
			delete m_pConnection->proto.tcp;
		delete m_pConnection;
	}
	else
	{
		espconn_delete(m_pConnection);
	}
}

void ICACHE_FLASH_ATTR Socket::ConnectCallback(void *arg)
{
	espconn *pespcon = (espconn*) arg;
	Socket *p = (Socket *) pespcon->reverse;
	p->GotConnect();
}
void ICACHE_FLASH_ATTR Socket::ReconnectCallback(void *arg, sint8 err)
{
	espconn *pespcon = (espconn*) arg;
	Socket *p = (Socket *) pespcon->reverse;
	p->GotReconnect(err);
}
void ICACHE_FLASH_ATTR Socket::DisconnectCallback(void *arg)
{
	espconn *pespcon = (espconn*) arg;
	Socket *p = (Socket *) pespcon->reverse;
	p->GotDisconnect();
}
void ICACHE_FLASH_ATTR Socket::SentCallback(void *arg)
{
	espconn *pespcon = (espconn*) arg;
	Socket *p = (Socket *) pespcon->reverse;
	p->GotSent();
}
void ICACHE_FLASH_ATTR Socket::ReceiveCallback(void *arg, char *data, unsigned short len)
{
	espconn *pespcon = (espconn*) arg;
	Socket *p = (Socket *) pespcon->reverse;
	p->GotReceive((uint8_t *) data, len);
}
void ICACHE_FLASH_ATTR Socket::DnsFoundCallback(const char *host, ip_addr_t *ip, void *arg)
{
	espconn *pespcon = (espconn*) arg;
	Socket *p = (Socket *) pespcon->reverse;
	p->GotDnsFound(host, ip);
}

void ICACHE_FLASH_ATTR Socket::GotConnect()
{
	// do some initialization stuff
	m_State = Connected;
	espconn_regist_disconcb(m_pConnection, DisconnectCallback);
	espconn_regist_recvcb(m_pConnection, ReceiveCallback);
	espconn_regist_sentcb(m_pConnection, SentCallback);

	OnConnect();
}

void ICACHE_FLASH_ATTR Socket::GotReconnect(sint8 err)
{
	m_State = Connecting;
	OnReconnect(err);
}

void ICACHE_FLASH_ATTR Socket::GotDisconnect()
{
	m_State = Unconnected;
	OnDisconnect();
}

void ICACHE_FLASH_ATTR Socket::GotSent()
{
	m_State = Connected;
	OnSent();
}

void ICACHE_FLASH_ATTR Socket::GotReceive(uint8_t *data, unsigned short len)
{
	OnReceive(data, len);
}

void ICACHE_FLASH_ATTR Socket::GotDnsFound(const char *host, ip_addr_t *ip)
{
	OnDnsFound(host, ip);
	if (ip != NULL)
		Connect(ip, m_pConnection->proto.tcp->remote_port);
}

void ICACHE_FLASH_ATTR Socket::OnConnect()
{
	ets_uart_printf("Got Connected!\n");
}

void ICACHE_FLASH_ATTR Socket::OnReconnect(sint8 err)
{
	ets_uart_printf("Got Reconnected!\n");
}
void ICACHE_FLASH_ATTR Socket::OnDisconnect()
{
	ets_uart_printf("Got Disconnected!\n");
}
void ICACHE_FLASH_ATTR Socket::OnSent()
{
	ets_uart_printf("Got Sent!\n");
}
void ICACHE_FLASH_ATTR Socket::OnReceive(uint8_t *data, unsigned short len)
{
	ets_uart_printf("Got Receive!\n");
	UART.send(data,len);
}
void ICACHE_FLASH_ATTR Socket::OnDnsFound(const char *host, ip_addr_t *ip)
{
	ets_uart_printf("Got DNS!\n");
}

Socket::Result ICACHE_FLASH_ATTR Socket::Connect(const char *host, int port)
{
	ip_addr_t ip;

	ip.addr = ipaddr_addr(host);	// if it's an x.x.x.x, we skip host lookup
	if (ip.addr == IPADDR_NONE || ip.addr == IPADDR_ANY)
	{
		m_State = Connecting;
		m_pConnection->proto.tcp->remote_port = port;
		ets_uart_printf("Looking up Hostname!\n");
		err_t err = espconn_gethostbyname(m_pConnection, host, &ip,
				DnsFoundCallback);
		return MapErrorCode(err);
	}
	else
	{
		return Connect(&ip, port);
	}
}

Socket::Result ICACHE_FLASH_ATTR Socket::Connect(const ip_addr_t *ip, int port)
{
	m_pConnection->type = ESPCONN_TCP;
	m_pConnection->proto.tcp->local_port = espconn_port();
	m_pConnection->proto.tcp->remote_port = port;
	os_memcpy(m_pConnection->proto.tcp->remote_ip, ip, 4);

	m_pConnection->reverse = this;

	espconn_regist_connectcb(m_pConnection, ConnectCallback);
	espconn_regist_reconcb(m_pConnection, ReconnectCallback);
	ets_uart_printf("Connecting!\n");
	return MapErrorCode(espconn_connect(m_pConnection));
}

Socket::Result ICACHE_FLASH_ATTR Socket::Send(const uint8_t *data,unsigned short len)
{
	if(m_State == Sending)
		return Busy;
	if(m_State != Connected)
		return Error;

	m_State = Sending;
	err_t err = espconn_send(m_pConnection,(uint8_t *)data,len);
	return MapErrorCode(err);
}

bool ICACHE_FLASH_ATTR Socket::IsConnected()
{
	return (m_pConnection->state == ESPCONN_CONNECT);
}
