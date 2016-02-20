#ifndef ESPMISSINGINCLUDES_H
#define ESPMISSINGINCLUDES_H

#include <ets_sys.h>

//Missing function prototypes in include folders. Gcc will warn on these if we don't define 'em anywhere.
//MOST OF THESE ARE GUESSED! but they seem to swork and shut up the compiler.
//typedef struct espconn espconn;
/** callback prototype to inform about events for a espconn */
//typedef void (*espconn_recv_callback)(void *arg, char *pdata,
//		unsigned short len);
//typedef void (*espconn_callback)(void *arg, char *pdata, unsigned short len);
//typedef void (*espconn_connect_callback)(void *arg);
//typedef void (*espconn_reconnect_callback)(void *arg, sint8 err);
//typedef void (*espconn_sent_callback)(void *arg);
//typedef void* espconn_handle;
//typedef struct _esp_tcp
//{
//	int remote_port;
//	int local_port;
//	uint8 local_ip[4];
//	uint8 remote_ip[4];
//	espconn_connect_callback connect_callback;
//	espconn_reconnect_callback reconnect_callback;
//	espconn_connect_callback disconnect_callback;
//	espconn_connect_callback write_finish_fn;
//} esp_tcp;
//typedef struct _esp_udp
//{
//	int remote_port;
//	int local_port;
//	uint8 local_ip[4];
//	uint8 remote_ip[4];
//} esp_udp;
///** Protocol family and type of the espconn */
//enum espconn_type
//{
//	ESPCONN_INVALID = 0,
//	/* ESPCONN_TCP Group */
//	ESPCONN_TCP = 0x10,
//	/* ESPCONN_UDP Group */
//	ESPCONN_UDP = 0x20,
//};
///** Current state of the espconn. Non-TCP espconn are always in state
// ESPCONN_NONE! */
//enum espconn_state
//{
//	ESPCONN_NONE,
//	ESPCONN_WAIT,
//	ESPCONN_LISTEN,
//	ESPCONN_CONNECT,
//	ESPCONN_WRITE,
//	ESPCONN_READ,
//	ESPCONN_CLOSE
//};
//enum espconn_option
//{
//	ESPCONN_START = 0x00,
//	ESPCONN_REUSEADDR = 0x01,
//	ESPCONN_NODELAY = 0x02,
//	ESPCONN_COPY = 0x04,
//	ESPCONN_KEEPALIVE = 0x08,
//	ESPCONN_END
//};
//
//enum espconn_level
//{
//	ESPCONN_KEEPIDLE, ESPCONN_KEEPINTVL, ESPCONN_KEEPCNT
//};
//
///** A espconn descriptor */
//struct espconn
//{
//	/** type of the espconn (TCP, UDP) */
//	enum espconn_type type;
//	/** current state of the espconn */
//	enum espconn_state state;
//	union
//	{
//		esp_tcp *tcp;
//		esp_udp *udp;
//	} proto;
//	/** A callback function that is informed about events for this espconn */
//	espconn_recv_callback recv_callback;
//	espconn_sent_callback sent_callback;
//	uint8 link_cnt;
//	void *reverse; // reversed for customer use
//};
int atoi(const char *nptr);
void ets_install_putc1(void *routine);
void ets_isr_attach(int intr, void *handler, void *arg);
void ets_isr_mask(unsigned intr);
void ets_isr_unmask(unsigned intr);
int ets_memcmp(const void *s1, const void *s2, size_t n);
void *ets_memcpy(void *dest, const void *src, size_t n);
void *ets_memset(void *s, int c, size_t n);
int ets_sprintf(char *str, const char *format, ...)
		__attribute__ ((format (printf, 2, 3)));
int ets_str2macaddr(void *, void *);
int ets_strcmp(const char *s1, const char *s2);
char *ets_strcpy(char *dest, const char *src);
size_t ets_strlen(const char *s);
int ets_strncmp(const char *s1, const char *s2, int len);
char *ets_strncpy(char *dest, const char *src, size_t n);
char *ets_strstr(const char *haystack, const char *needle);
void ets_timer_arm_new(ETSTimer *a, int b, int c, int isMstimer);
void ets_timer_disarm(ETSTimer *a);
void ets_timer_setfn(ETSTimer *t, ETSTimerFunc *fn, void *parg);
void ets_update_cpu_frequency(int freqmhz);
int os_printf(const char *format, ...) __attribute__ ((format (printf, 1, 2)));
int os_snprintf(char *str, size_t size, const char *format, ...)
		__attribute__ ((format (printf, 3, 4)));
int os_printf_plus(const char *format, ...)
		__attribute__ ((format (printf, 1, 2)));
void pvPortFree(void *ptr);
void *pvPortMalloc(size_t xWantedSize);
void *pvPortZalloc(size_t);
void uart_div_modify(int no, unsigned int freq);
void vPortFree(void *ptr);
void *vPortMalloc(size_t xWantedSize);
uint8 wifi_get_opmode(void);
uint32 system_get_time();
int rand(void);
void ets_bzero(void *s, size_t n);
void ets_delay_us(int ms);
#endif
