/*
 * Stream.h
 *
 *  Created on: Dec 31, 2015
 *      Author: tsnow
 */

#ifndef INCLUDE_DRIVER_STREAM_H_
#define INCLUDE_DRIVER_STREAM_H_
extern "C"
{
#include <ets_sys.h>
#include <osapi.h>
#include <os_type.h>
#include <c_types.h>
#include "driver/uart_register.h"
#include "driver/uart.h"
#include "espmissingincludes.h"
extern int ets_uart_printf(const char *fmt, ...);
}

#include "driver/InterruptBlocker.h"
#include "driver/Process.h"
typedef uint8_t byte;

template<class T>
class RingBuffer
{
private:
	T *pBuf;
	size_t len;
	size_t head;
	size_t tail;
	T error_t;

public:
	RingBuffer(size_t size, T error)
	{
		len = size;
		pBuf = new T[len];
		memset(pBuf,0,len);

		head = tail = 0;
		error_t = error;
	}
	;

	~RingBuffer()
	{
		delete[] pBuf;
	}
	;

	size_t length()
	{
		InterruptBlocker ib;
		size_t ret = tail - head;
		if (tail < head)
			ret += len;

		return ret;
	}
	;

	T get()
	{
		InterruptBlocker ib;
		T ret = error_t;
		if (head != tail)
		{
			ret = pBuf[head];
			head = (head + 1) % len;
		}

		return ret;
	}
	;

	bool put(T x)
	{
		InterruptBlocker ib;
		bool ret = false;
		size_t nexttail = (tail + 1) % len;
		if (nexttail != head)
		{
			pBuf[tail] = x;
			tail = nexttail;
			ret = true;
		}

		return ret;
	}
	;

	void printStatus()
	{
		ets_uart_printf("head=%d, tail=%d, len=%d\r\n",head,tail,len);
	}
};

class Stream
{
public:
	Stream();
	virtual ~Stream();

	virtual int get() = 0;
	virtual void put(char c) = 0;
	virtual int length() = 0;

	void print(const char *p);
	void print(int x, int radix = 10);
	void println(const char *p);
	void println(int x, int radix = 10){print(x,radix);print("\r\n");};
	void send(const uint8_t *data,unsigned short len);

};
/*
typedef enum {
    FIVE_BITS = 0x0,
    SIX_BITS = 0x1,
    SEVEN_BITS = 0x2,
    EIGHT_BITS = 0x3
} UartBitsNum4Char;

typedef enum {
    ONE_STOP_BIT             = 0,
    ONE_HALF_STOP_BIT        = BIT2,
    TWO_STOP_BIT             = BIT2
} UartStopBitsNum;

typedef enum {
    NONE_BITS = 0,
    ODD_BITS   = 0,
    EVEN_BITS = BIT4
} UartParityMode;

typedef enum {
    STICK_PARITY_DIS   = 0,
    STICK_PARITY_EN    = BIT3 | BIT5
} UartExistParity;

typedef enum {
    BIT_RATE_9600     = 9600,
    BIT_RATE_19200   = 19200,
    BIT_RATE_38400   = 38400,
    BIT_RATE_57600   = 57600,
    BIT_RATE_74880   = 74880,
    BIT_RATE_115200 = 115200,
    BIT_RATE_230400 = 230400,
    BIT_RATE_460800 = 460800,
    BIT_RATE_921600 = 921600
} UartBautRate;

typedef enum {
    NONE_CTRL,
    HARDWARE_CTRL,
    XON_XOFF_CTRL
} UartFlowCtrl;

*/

class EOLProcess : public ProcessTask
	{
private:
	bool m_bRemoveBS;

public:
	EOLProcess(bool removebs=true);

	void  runTask(const void *p);
	virtual void OnLineSent(const char *pLine) = 0;
	};

class UARTStream: public Stream
{
private:
	RingBuffer<int> m_in;
	UartBautRate m_baud;
	UartBitsNum4Char m_bits;
	UartStopBitsNum m_stop_bits;
	UartParityMode m_parity;
	UartFlowCtrl m_flow_control;
	ProcessTask	*m_pEOLTask;

	static STATUS GlobalTransmit(char c);


public:
	UARTStream(UartFlowCtrl flow_control = NONE_CTRL,UartBautRate baud = BIT_RATE_115200, UartBitsNum4Char bits = EIGHT_BITS, UartStopBitsNum stop_bits = ONE_STOP_BIT, UartParityMode parity = NONE_BITS,size_t insize = 1024);
	~UARTStream();

	int get(){return m_in.get();};
	void put(char c);
	int length(){return m_in.length();};

	bool fillInput(char c);
	static void Handler(void *p);

	void SetEOLTask(ProcessTask *task){m_pEOLTask = task;};

};

extern UARTStream UART;


#endif /* INCLUDE_DRIVER_STREAM_H_ */
