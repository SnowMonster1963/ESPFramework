/*
 * Timer.h
 *
 *  Created on: Dec 29, 2015
 *      Author: tsnow
 */

#ifndef INCLUDE_TIMER_H_
#define INCLUDE_TIMER_H_
extern "C"
{
#include <ets_sys.h>
#include <osapi.h>
#include <os_type.h>
#include <gpio.h>
}

class Timer
{
private:
	os_timer_t timer;
	static void ICACHE_FLASH_ATTR callback(void *p);

	uint32_t m_delay;
	bool m_repeat;

public:
	Timer(uint32_t delay, bool repeat = true);
	virtual ~Timer()
	{
		Stop();
	}
	;
	void Start();
	void Stop();
	virtual void OnTime() = 0;
};

template<class T>
class PtrTimer : public Timer
	{
	protected:
		T	*ptr;

	public:
		PtrTimer(T *p,uint32_t delay,bool repeat = true) : Timer(delay,repeat)
		{
			ptr = p;
		};

	};
#endif /* INCLUDE_TIMER_H_ */
