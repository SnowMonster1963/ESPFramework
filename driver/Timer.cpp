/*
 * Timer.cpp
 *
 *  Created on: Dec 29, 2015
 *      Author: tsnow
 */
#include "driver/Timer.h"
extern "C" {
void ets_timer_disarm(ETSTimer *ptimer);
void ets_timer_setfn(ETSTimer *ptimer, ETSTimerFunc *pfunction, void *parg);
void ets_timer_arm_new(ETSTimer *ptimer,uint32_t milliseconds, bool repeat_flag, bool);
}

void ICACHE_FLASH_ATTR Timer::callback(void *p)
{
	Timer *t = (Timer *) p;
	t->OnTime();
}


Timer::Timer(uint32_t delay, bool repeat)
{
	m_delay = delay;
	m_repeat = repeat;
	Stop();
	os_timer_setfn(&timer, (os_timer_func_t *)callback, (void *)this);
	Start();
}

void Timer::Start()
{
	os_timer_arm(&timer, m_delay, m_repeat);
}

void Timer::Stop()
{
	os_timer_disarm(&timer);
}
