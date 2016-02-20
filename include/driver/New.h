/*
 * New.h
 *
 *  Created on: Dec 29, 2015
 *      Author: tsnow
 */

#ifndef INCLUDE_DRIVER_NEW_H_
#define INCLUDE_DRIVER_NEW_H_

extern "C" {
#include <ets_sys.h>
#include <osapi.h>
#include <os_type.h>
#include <gpio.h>

}
extern "C"
{
void *pvPortMalloc( size_t xWantedSize );
void vPortFree( void *pv );
void *pvPortZalloc(size_t size);

#define os_malloc   pvPortMalloc
#define os_free     vPortFree
#define os_zalloc   pvPortZalloc

}//extern "C"

void *operator new(size_t size);
void *operator new[](size_t size);
void operator delete(void * ptr);
void operator delete[](void * ptr);
extern "C" void __cxa_pure_virtual(void) __attribute__ ((__noreturn__));
extern "C" void __cxa_deleted_virtual(void) __attribute__ ((__noreturn__));


#endif /* INCLUDE_DRIVER_NEW_H_ */
