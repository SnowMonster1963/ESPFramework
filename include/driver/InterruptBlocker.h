/*
 * InterruptBlocker.h
 *
 *  Created on: Dec 31, 2015
 *      Author: tsnow
 */

#ifndef INCLUDE_DRIVER_INTERRUPTBLOCKER_H_
#define INCLUDE_DRIVER_INTERRUPTBLOCKER_H_
extern "C"
{
#include <ets_sys.h>
#include <osapi.h>
#include <os_type.h>
extern int ets_uart_printf(const char *fmt, ...);
}

class InterruptBlocker
{
private:
	uint32_t	sreg;

public:
	InterruptBlocker();
	~InterruptBlocker();
};



#endif /* INCLUDE_DRIVER_INTERRUPTBLOCKER_H_ */
