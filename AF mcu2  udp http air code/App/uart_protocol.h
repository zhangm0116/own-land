#ifndef _UART_CTL_H__
#define __UART_CTLH__

#define R_SUCCESS  		 		0x80
#define SET_FAILED				0xa0
#define GET_FAILED				0xa1
#define CHECKDATA_ERROR			0xa2
#define LEN_ERROR				0xa3
#define CMD_NOT_SUPPORT			0xa4
#define OUT_OF_RANGE  			0xa5

int uart_ctl_init(void);
void uart_ctl_thread_task(void* parameter);
#endif
