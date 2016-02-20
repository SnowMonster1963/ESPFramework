/*
 * InterruptBlocker.cpp
 *
 *  Created on: Dec 31, 2015
 *      Author: tsnow
 */

#include "driver/InterruptBlocker.h"


InterruptBlocker::InterruptBlocker()
{
	__asm__ __volatile__("rsil %0,15" : "=a" (sreg));
}

InterruptBlocker::~InterruptBlocker()
{
	__asm__ __volatile__("wsr %0,ps; isync" :: "a" (sreg) : "memory") ;
}
