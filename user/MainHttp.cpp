/*
 * MainHttp.cpp
 *
 *  Created on: May 5, 2016
 *      Author: tsnow
 */

#include <ATMqtt.h>
#include <CmdProcessor.h>
#include <WiFiManager.h>
#include "MainHttp.h"


MainHttpServerSocketFactory MainHttpFactory;

MainHttpServerSocket::PageItem MainHttpServerSocket::m_router[] =
		{
				{ "/",MainHttpServerSocket::RouteMainPage},
				{ "/css.css",MainHttpServerSocket::RouteCssPage},
				{ "/access.html",MainHttpServerSocket::RouteAccessPage},
				{ "/set_access.html",MainHttpServerSocket::RouteSetAccessPage},
				{ "/wifi.html",MainHttpServerSocket::RouteWiFiPage},
				{ "/set_wifi.html",MainHttpServerSocket::RouteSetWiFiPage},
				{ "/mqtt.html",MainHttpServerSocket::RouteMqttPage},
				{ "/set_mqtt.html",MainHttpServerSocket::RouteSetMqttPage},
		};

ICACHE_FLASH_ATTR MainHttpServerSocketFactory::MainHttpServerSocketFactory()
	{

	}

Socket * ICACHE_FLASH_ATTR MainHttpServerSocketFactory::CreateSocket(struct espconn *conn)
	{
		return new MainHttpServerSocket(conn);
	}

ICACHE_FLASH_ATTR MainHttpServerSocket::MainHttpServerSocket() :
		HttpServerSocket()
	{

	}

ICACHE_FLASH_ATTR MainHttpServerSocket::MainHttpServerSocket(espconn *conn) :
		HttpServerSocket(conn)
	{

	}

void ICACHE_FLASH_ATTR MainHttpServerSocket::OnPage(const char *page, HttpParameters *paramarray, int params)
	{
		HttpServerSocket::OnPage(page, paramarray, params);
		for(int i=0;i<sizeof(m_router)/sizeof(m_router[0]);i++)
			{
				if(os_strcmp(page,m_router[i].page) == 0)
					{
						m_router[i].func(this,page,paramarray,params);
						return;
					}
			}
		CloseOnSent();
		Send(mainPage);
	}

void ICACHE_FLASH_ATTR MainHttpServerSocket::RouteMainPage(MainHttpServerSocket *sock,
		const char *page, HttpParameters *paramarray, int params)
	{
		sock->DoMainPage(page,paramarray,params);
	}

void ICACHE_FLASH_ATTR MainHttpServerSocket::RouteCssPage(MainHttpServerSocket *sock,
		const char *page, HttpParameters *paramarray, int params)
	{
		sock->DoCssPage(page,paramarray,params);
	}
void ICACHE_FLASH_ATTR MainHttpServerSocket::RouteAccessPage(MainHttpServerSocket *sock,
		const char *page, HttpParameters *paramarray, int params)
	{
		sock->DoAccessPage(page,paramarray,params);
	}

void ICACHE_FLASH_ATTR MainHttpServerSocket::RouteSetAccessPage(MainHttpServerSocket *sock,
		const char *page, HttpParameters *paramarray, int params)
	{
		sock->DoSetAccessPage(page,paramarray,params);
	}

void ICACHE_FLASH_ATTR MainHttpServerSocket::RouteWiFiPage(MainHttpServerSocket *sock,
		const char *page, HttpParameters *paramarray, int params)
	{
		sock->DoWiFiPage(page,paramarray,params);
	}

void ICACHE_FLASH_ATTR MainHttpServerSocket::RouteSetWiFiPage(MainHttpServerSocket *sock,
		const char *page, HttpParameters *paramarray, int params)
	{
		sock->DoSetWiFiPage(page,paramarray,params);
	}

void ICACHE_FLASH_ATTR MainHttpServerSocket::RouteMqttPage(MainHttpServerSocket *sock,
		const char *page, HttpParameters *paramarray, int params)
	{
		sock->DoMqttPage(page,paramarray,params);
	}

void ICACHE_FLASH_ATTR MainHttpServerSocket::RouteSetMqttPage(MainHttpServerSocket *sock,
		const char *page, HttpParameters *paramarray, int params)
	{
		sock->DoSetMqttPage(page,paramarray,params);
	}

