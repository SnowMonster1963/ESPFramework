/*
 * Http.h
 *
 *  Created on: May 3, 2016
 *      Author: tsnow
 */

#ifndef INCLUDE_HTTP_H_
#define INCLUDE_HTTP_H_

#include "driver/Socket.h"

#define MAX_PARMS 20

struct HttpParameters
	{
	char *parameter;
	char *value;
	};


class HttpServerSocket : public Socket
	{
private:
	bool m_CloseOnSent;
	char *m_request;
	size_t	m_remaining;


protected:
	void OnReceive(uint8_t *data,unsigned short len);
	void OnSent();
	virtual void ProcessPage(char *page);
	virtual void OnPage(const char *page,HttpParameters *paramarray,int params);

public:
	HttpServerSocket();
	HttpServerSocket(espconn *conn);
	void CloseOnSent(bool willClose = true);

	};


#endif /* INCLUDE_HTTP_H_ */
