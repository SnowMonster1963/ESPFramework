/*
 * Socket.cpp
 *
 *  Created on: Jan 5, 2016
 *      Author: tsnow
 */

//#include <mem.h>
#include "driver/Stream.h"
#include "driver/Socket.h"
#include "driver/Process.h"
#define os_free(s)        vPortFree(s)
#define os_malloc(s)      pvPortMalloc(s)
#define os_calloc(s)      pvPortCalloc(s)
#define os_realloc(p, s)  pvPortRealloc(p,s)
#define os_zalloc(s)      pvPortZalloc(s)

ICACHE_FLASH_ATTR SocketDeleter::SocketDeleter() : UserTask<Socket,true>(Priority1Manager)
{

}

void ICACHE_FLASH_ATTR SocketDeleter::OnMessage(const Socket *sock)
	{
	}

SocketDeleter KillSocket;

ICACHE_FLASH_ATTR ListenManager::ListenManager()
	{
		for(int i=0;i<sizeof(m_entries)/sizeof(m_entries[0]);i++)
			{
				m_entries[i].Listener = NULL;
				m_entries[i].port = 0;
			}
	}


bool ICACHE_FLASH_ATTR ListenManager::AddListener(int port,Socket *sock)
	{
		// check to see if already listening
		for(int i=0;i<sizeof(m_entries)/sizeof(m_entries[0]);i++)
			{
				if(m_entries[i].port == port)
						return false;
			}

		// find a slot
		for(int i=0;i<sizeof(m_entries)/sizeof(m_entries[0]);i++)
			{
				if(m_entries[i].port == 0)
					{
						m_entries[i].Listener = sock;
						m_entries[i].port = port;
						return true;
					}
			}
		return false;
	}

void ICACHE_FLASH_ATTR ListenManager::RemoveListener(int port)
	{
		for(int i=0;i<sizeof(m_entries)/sizeof(m_entries[0]);i++)
			{
				if(m_entries[i].port == port)
					{
						m_entries[i].port = 0;
						m_entries[i].Listener = NULL;
					}
			}
	}

Socket * ICACHE_FLASH_ATTR ListenManager::FindListener(int port)
	{
		for(int i=0;i<sizeof(m_entries)/sizeof(m_entries[0]);i++)
			{
				if(m_entries[i].port == port)
					{
						return m_entries[i].Listener;
					}
			}

		return NULL;
	}

ListenManager Socket::m_ListenManager;

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

	//os_printf("Error value is %d\n",(int)err);

	return ret;
}

ICACHE_FLASH_ATTR Socket::Socket()
{
	m_pConnection = NULL;
//	memset(m_pConnection, 0, sizeof(espconn));
//	m_pConnection->proto.tcp = new esp_tcp; // union has same members at beginning as udp
//	m_pConnection->reverse = this;
//	m_pConnection->type = ESPCONN_INVALID;
//	m_pConnection->state = ESPCONN_NONE;
	m_State = Unconnected;

	m_bOwnConnection = false;
	m_Factory = NULL;
}

ICACHE_FLASH_ATTR Socket::Socket(espconn *pConn)
{
	m_pConnection = pConn;
	m_pConnection->reverse = this;

	m_bOwnConnection = false;
	m_Factory = NULL;
}

ICACHE_FLASH_ATTR Socket::~Socket()
{
	os_printf("Deleting 0x%08x with espconn 0x%08x\r\n",this,m_pConnection);
	FreeConnection();
}

void ICACHE_FLASH_ATTR Socket::AllocateConnection(bool isTCP)
	{
		if(m_pConnection == NULL)
			{
				m_pConnection = new espconn;// (espconn *)os_zalloc(sizeof(espconn));
				memset(m_pConnection, 0, sizeof(espconn));
				m_pConnection->proto.tcp = new esp_tcp; // union has same members at beginning as udp
				m_pConnection->reverse = this;
				m_pConnection->type = ESPCONN_INVALID;
				m_pConnection->state = ESPCONN_NONE;
				m_bOwnConnection = true;
//				if(isTCP)
//					m_pConnection->proto.tcp = new esp_tcp;//(esp_tcp *)os_zalloc(sizeof(esp_tcp));
//				else
//					m_pConnection->proto.udp = new esp_udp;//(esp_udp *)os_zalloc(sizeof(esp_udp));
			}
	}

void ICACHE_FLASH_ATTR Socket::FreeConnection()
	{
		if(m_pConnection)
			{
				if (m_bOwnConnection)
				{
					if (m_pConnection->proto.tcp != NULL)
						delete m_pConnection->proto.tcp;
					delete m_pConnection;
//					os_free(m_pConnection->proto.tcp);	//delete m_pConnection->proto.tcp;
//				os_free(m_pConnection);	//delete m_pConnection;
				}
				else
				{
					sint8 err = espconn_delete(m_pConnection);
					//os_printf("got err = %d from espconn_delete\r\n",(int)err);
				}
				m_pConnection = NULL;
			}
	}

void ICACHE_FLASH_ATTR Socket::ConnectCallback(void *arg)
{
	espconn *pespcon = (espconn*) arg;
	Socket *p = m_ListenManager.FindListener(pespcon->proto.tcp->local_port);
	if(p == NULL)
		p = (Socket *) pespcon->reverse;
	p->GotConnect(pespcon);
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

void ICACHE_FLASH_ATTR Socket::GotConnect(espconn *new_connection)
{
	//os_printf("Got Connect socket=0x%08x, conn=0x%08x, newconn=0x%08x\n",this,m_pConnection,new_connection);
	// do some initialization stuff
	if(m_Factory == NULL)	// this socket connected
		{
		//os_printf("In GotConnect - non server socket\r\n");
		m_State = Connected;
		espconn_regist_disconcb(m_pConnection, DisconnectCallback);
		espconn_regist_recvcb(m_pConnection, ReceiveCallback);
		espconn_regist_sentcb(m_pConnection, SentCallback);

		OnConnect();
		}
	else					// this socket is listening to new connection
		{
		//os_printf("In GotConnect - server socket\r\n");
		GotListenConnect(new_connection);
		}
}

void ICACHE_FLASH_ATTR Socket::GotReconnect(sint8 err)
{
	m_State = Connecting;
	OnReconnect(err);
}

void ICACHE_FLASH_ATTR Socket::GotDisconnect()
{
//		if(m_Factory == NULL)	// this socket connected
//			{
//				os_printf("In GotDisconnect - non server socket\r\n");
//			}
//		else					// this socket is listening to new connection
//			{
//			os_printf("In GotDisconnect - server socket\r\n");
//			}
	m_State = Unconnected;
	OnDisconnect();
	//FreeConnection();
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

void ICACHE_FLASH_ATTR Socket::GotListenConnect(espconn *new_connection)
{
	// do some initialization stuff
	//os_printf("Creating socket local port = %d, remote port = %d\r\n",new_connection->proto.tcp->local_port,new_connection->proto.tcp->remote_port);
	Socket *pSock = m_Factory->CreateSocket(new_connection);
	//os_printf("Created socket 0x%08x\r\n",pSock);

	pSock->GotConnect(new_connection);
}

void ICACHE_FLASH_ATTR Socket::OnConnect()
{
	os_printf("Got Connected!\n");
}

void ICACHE_FLASH_ATTR Socket::OnReconnect(sint8 err)
{
	os_printf("Got Reconnected!\n");
}
void ICACHE_FLASH_ATTR Socket::OnDisconnect()
{
	os_printf("Got Disconnected!\n");
}
void ICACHE_FLASH_ATTR Socket::OnSent()
{
	os_printf("Got Sent!\n");
}
void ICACHE_FLASH_ATTR Socket::OnReceive(uint8_t *data, unsigned short len)
{
	os_printf("Got Receive!\n");
	UART.send(data,len);
}
void ICACHE_FLASH_ATTR Socket::OnDnsFound(const char *host, ip_addr_t *ip)
{
	//os_printf("Got DNS!\n");
}

Socket::Result ICACHE_FLASH_ATTR Socket::Connect(const char *host, int port)
{
	ip_addr_t ip;

	ip.addr = ipaddr_addr(host);	// if it's an x.x.x.x, we skip host lookup
	if (ip.addr == IPADDR_NONE || ip.addr == IPADDR_ANY)
	{
		AllocateConnection();
		m_State = Connecting;
		m_pConnection->proto.tcp->remote_port = port;
		//os_printf("Looking up Hostname!\n");
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
	AllocateConnection();
	m_pConnection->type = ESPCONN_TCP;
	m_pConnection->proto.tcp->local_port = espconn_port();
	m_pConnection->proto.tcp->remote_port = port;
	os_memcpy(m_pConnection->proto.tcp->remote_ip, ip, 4);

	m_pConnection->reverse = this;

	espconn_regist_connectcb(m_pConnection, ConnectCallback);
	espconn_regist_reconcb(m_pConnection, ReconnectCallback);
	//os_printf("Connecting!\n");
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

Socket::Result ICACHE_FLASH_ATTR Socket::Close()
{
	return MapErrorCode(espconn_disconnect(m_pConnection));
}

bool ICACHE_FLASH_ATTR Socket::IsConnected()
{
	return (m_pConnection != NULL && m_pConnection->state == ESPCONN_CONNECT);
}

bool ICACHE_FLASH_ATTR Socket::IsListening()
{
	return (m_pConnection != NULL && m_pConnection->state == ESPCONN_LISTEN);
}

Socket::Result ICACHE_FLASH_ATTR Socket::Listen(SocketFactory *factory, int port,uint32 timeout)
{
		if(this->m_ListenManager.AddListener(port, this))
			{
			AllocateConnection(true);
			m_pConnection->type = ESPCONN_TCP;
			m_pConnection->state = ESPCONN_NONE;
			m_pConnection->proto.tcp->local_port = port;
			m_Factory = factory;

			//espconn_regist_connectcb(m_pConnection,ListenConnectCallback);
			espconn_regist_connectcb(m_pConnection, ConnectCallback);
			err_t err = espconn_accept(m_pConnection);
			if(err != 0)
				return MapErrorCode(err);
			err = espconn_regist_time(m_pConnection,timeout,0);
			return MapErrorCode(err);
			}
		return Error;
}


ICACHE_FLASH_ATTR SocketFactory::SocketFactory()
	{

	}

ICACHE_FLASH_ATTR SocketFactory::~SocketFactory()
	{

	}
