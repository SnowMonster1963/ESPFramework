/*
 * utils.h
 *
 *  Created on: May 14, 2016
 *      Author: tsnow
 */

#ifndef INCLUDE_DRIVER_UTILS_H_
#define INCLUDE_DRIVER_UTILS_H_


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
typedef uint8_t byte;

extern void str_replace(char *buf,const char *from,const char *to);
extern char * strstri(char *a,const char *b);
extern uint32_t GetCheckSum(void *p,size_t len);
#endif /* INCLUDE_DRIVER_UTILS_H_ */
