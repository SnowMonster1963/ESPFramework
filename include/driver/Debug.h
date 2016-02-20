/*
 * Debug.h
 *
 *  Created on: Dec 30, 2015
 *      Author: tsnow
 */

#ifndef INCLUDE_DRIVER_DEBUG_H_
#define INCLUDE_DRIVER_DEBUG_H_

extern "C"
{
#include <ets_sys.h>
#include <osapi.h>
#include <os_type.h>
#include <spi_flash.h>
extern int ets_uart_printf(const char *fmt, ...);
}

typedef uint8_t byte;

void dumpData(const byte *p,size_t len);

#endif /* INCLUDE_DRIVER_DEBUG_H_ */
