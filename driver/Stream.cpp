/*
 * Stream.cpp
 *
 *  Created on: Dec 31, 2015
 *      Author: tsnow
 */

#include "driver/Stream.h"
extern "C" {
void uart_div_modify(int no, unsigned int freq);
void ets_install_putc1(void *routine);
void ets_isr_unmask(unsigned intr);
}


UARTStream UART;

ICACHE_FLASH_ATTR Stream::Stream()
{

}


ICACHE_FLASH_ATTR Stream::~Stream()
{

}


ICACHE_FLASH_ATTR void Stream::print(const char *p)
{
	while(*p)
		put(*p++);
}


ICACHE_FLASH_ATTR char *itoa(int n, char *buf,int radix)
{
	int idx = 0;
	const char *chars = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	if(radix > strlen(chars))
		return buf;

	do
	{
		int digit = n % radix;
		buf[idx++] = chars[digit];
		buf[idx] = 0;
		n = n/radix;
	} while (n > 0);

	n = strlen(buf);
	for(idx = 0;idx < n;idx++)
	{
		char c = buf[idx];
		buf[idx] = buf[n-idx-1];
		buf[n-idx-1] = c;
	}
	return buf;
}

ICACHE_FLASH_ATTR void Stream::print(int x,int radix)
{
	char buf[32];
	itoa(x,buf,radix);
	print(buf);
}

ICACHE_FLASH_ATTR void Stream::println(const char *p)
{
	print(p);
	print("\r\n");
}

ICACHE_FLASH_ATTR UARTStream::UARTStream(UartFlowCtrl flow_control,UartBautRate baud, UartBitsNum4Char bits, UartStopBitsNum stop_bits, UartParityMode parity ,size_t insize) : m_in(insize,-1)
{
	m_baud = baud;
	m_bits = bits;
	m_stop_bits = stop_bits;
	m_parity = parity;
	m_flow_control = flow_control;
	m_pEOLTask = NULL;

    ETS_UART_INTR_ATTACH((void *)UARTStream::Handler,  this);
    PIN_PULLUP_DIS(PERIPHS_IO_MUX_U0TXD_U);
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_U0TXD_U, FUNC_U0TXD);
    uart_div_modify(0, UART_CLK_FREQ / (m_baud));
    WRITE_PERI_REG(UART_CONF0(0),    (m_parity == NONE_BITS ? 0 : (BIT3 | BIT5))
                   | m_parity
                   | (m_stop_bits << UART_STOP_BIT_NUM_S)
                   | (m_bits << UART_BIT_NUM_S));


    //clear rx and tx fifo,not ready
    SET_PERI_REG_MASK(UART_CONF0(0), UART_RXFIFO_RST | UART_TXFIFO_RST);
    CLEAR_PERI_REG_MASK(UART_CONF0(0), UART_RXFIFO_RST | UART_TXFIFO_RST);

    //set rx fifo trigger
    WRITE_PERI_REG(UART_CONF1(0), (1 & UART_RXFIFO_FULL_THRHD) << UART_RXFIFO_FULL_THRHD_S);

    //clear all interrupt
    WRITE_PERI_REG(UART_INT_CLR(0), 0xffff);
    //enable rx_interrupt
    SET_PERI_REG_MASK(UART_INT_ENA(0), UART_RXFIFO_FULL_INT_ENA);
    ETS_UART_INTR_ENABLE();
    //os_install_putc1((void *)GlobalTransmit);

}

ICACHE_FLASH_ATTR UARTStream::~UARTStream()
{

}

ICACHE_FLASH_ATTR void UARTStream::put(char c)
{
    while (true)
	{
		uint32 fifo_cnt = READ_PERI_REG(UART_STATUS(0)) & (UART_TXFIFO_CNT<<UART_TXFIFO_CNT_S);
		if ((fifo_cnt >> UART_TXFIFO_CNT_S & UART_TXFIFO_CNT) < 126) {
			break;
		}
	}

	WRITE_PERI_REG(UART_FIFO(0) , c);

}

ICACHE_FLASH_ATTR bool UARTStream::fillInput(char c)
{
		if(c == '\n' && m_pEOLTask != NULL)
			{
			m_pEOLTask->Post(NULL);
			}
	return m_in.put((int)c);
}

ICACHE_FLASH_ATTR void UARTStream::Handler(void *p)
{
	UARTStream *strm = (UARTStream *) p;
    uint8 RcvChar;

    if (UART_RXFIFO_FULL_INT_ST != (READ_PERI_REG(UART_INT_ST(0)) & UART_RXFIFO_FULL_INT_ST)) {
        return;
    }

    WRITE_PERI_REG(UART_INT_CLR(0), UART_RXFIFO_FULL_INT_CLR);

    while (READ_PERI_REG(UART_STATUS(0)) & (UART_RXFIFO_CNT << UART_RXFIFO_CNT_S)) {
        RcvChar = READ_PERI_REG(UART_FIFO(0)) & 0xFF;

        strm->fillInput((char)RcvChar);
    }
}

ICACHE_FLASH_ATTR STATUS UARTStream::GlobalTransmit(char c)
{
	UART.put(c);
	return OK;
}

ICACHE_FLASH_ATTR void Stream::send(const uint8_t *data,unsigned short len)
{
	while(len > 0)
	{
		char c = (char) *data++;
		put(c);
		len--;
	}
}

EOLProcess::EOLProcess(bool removebs) : ProcessTask(Priority1Manager)
{
	m_bRemoveBS = removebs;
};


void  EOLProcess::runTask(const void *p)
	{
		byte buf[UART.length() + 1];
		os_memset(buf,0,sizeof(buf));
		int idx = 0;
		int c = UART.get();
		while(c >= 0)
			{
				if(m_bRemoveBS && idx > 0 && (c == 8 || c == 0x7f))	// back space or delete
					{
						idx--;
						buf[idx] = 0;
					}
				else
					{
					buf[idx++] = (char) c;
					}
				c = UART.get();
			}
		OnLineSent((const char*)buf);
	}
