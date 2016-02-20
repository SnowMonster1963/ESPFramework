/*
 * New.cpp
 *
 *  Created on: Dec 29, 2015
 *      Author: tsnow
 */
#include "driver/New.h"

// =============================================================================================
// C includes and declarations
// =============================================================================================

// =============================================================================================
// These methods shall be defined anywhere.
// They are required for C++ compiler
// =============================================================================================
void *operator new(size_t size)
{
   return os_malloc(size);
}

void *operator new[](size_t size)
{
   return os_malloc(size);
}

void operator delete(void * ptr)
{
   os_free(ptr);
}

void operator delete[](void * ptr)
{
   os_free(ptr);
}

extern "C" void abort()
{
   while(true); // enter an infinite loop and get reset by the WDT
}

extern "C" void __cxa_pure_virtual(void) {
  abort();
}

extern "C" void __cxa_deleted_virtual(void) {
  abort();
}


