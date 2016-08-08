/*
 * MainHttp.h
 *
 *  Created on: May 5, 2016
 *      Author: tsnow
 */

#ifndef INCLUDE_MAINHTTP_H_
#define INCLUDE_MAINHTTP_H_

#include <driver/Http.h>

class MainHttpServerSocketFactory : public SocketFactory
	{
	public:
	MainHttpServerSocketFactory();

	Socket *CreateSocket(struct espconn *conn);
	};

class MainHttpServerSocket : public HttpServerSocket
	{
private:

	struct PageItem
		{
		const char *page;
		void (*func)(MainHttpServerSocket *, const char *,HttpParameters *,int);
		};

	static PageItem m_router[];
	static MainHttpServerSocket *m_pWiFiSock;

	static void WiFiCallback(void *arg,STATUS status);
	void DoWiFiCallback(struct bss_info *bss,STATUS status);

	static void RouteMainPage(MainHttpServerSocket *sock,const char *page,HttpParameters *paramarray,int params);
	static void RouteCssPage(MainHttpServerSocket *sock,const char *page,HttpParameters *paramarray,int params);
	static void RouteSetAccessPage(MainHttpServerSocket *sock,const char *page,HttpParameters *paramarray,int params);
	static void RouteAccessPage(MainHttpServerSocket *sock,const char *page,HttpParameters *paramarray,int params);
	static void RouteWiFiPage(MainHttpServerSocket *sock,const char *page,HttpParameters *paramarray,int params);
	static void RouteSetWiFiPage(MainHttpServerSocket *sock,const char *page,HttpParameters *paramarray,int params);
	static void RouteMqttPage(MainHttpServerSocket *sock,const char *page,HttpParameters *paramarray,int params);
	static void RouteSetMqttPage(MainHttpServerSocket *sock,const char *page,HttpParameters *paramarray,int params);

	void DoMainPage(const char *page,HttpParameters *paramarray,int params);
	void DoCssPage(const char *page,HttpParameters *paramarray,int params);
	void DoAccessPage(const char *page,HttpParameters *paramarray,int params);
	void DoSetAccessPage(const char *page,HttpParameters *paramarray,int params);
	void DoWiFiPage(const char *page,HttpParameters *paramarray,int params);
	void DoSetWiFiPage(const char *page,HttpParameters *paramarray,int params);
	void DoMqttPage(const char *page,HttpParameters *paramarray,int params);
	void DoSetMqttPage(const char *page,HttpParameters *paramarray,int params);

protected:
	virtual void OnPage(const char *page,HttpParameters *paramarray,int params);

public:
	MainHttpServerSocket();
	MainHttpServerSocket(espconn *conn);

	};


extern MainHttpServerSocketFactory MainHttpFactory;

extern const char *mainPage;
extern void SetContentLength(char *buf);

#endif /* INCLUDE_MAINHTTP_H_ */
