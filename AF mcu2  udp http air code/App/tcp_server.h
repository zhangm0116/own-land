

#ifndef __TCP_SERVER_H
#define __TCP_SERVER_H

#include "lwip/debug.h"
#include "lwip/stats.h"
#include "lwip/tcp.h"
#include "lwip/memp.h"
#include "lwip/mem.h"
#include "lwip/pbuf.h"
#include "lwip/netifapi.h"
#include "lwip/tcpip.h"
#include "netif/etharp.h"



//////////////////////////////////////////////////////////////////////////////////	 
//修改信息
//无
////////////////////////////////////////////////////////////////////////////////// 	   
 
#define TCP_SERVER_RX_BUFSIZE	200		//定义tcp server最大接收数据长度
#define TCP_SERVER_PORT  8086   

typedef unsigned char  u8;
typedef unsigned short int  u16;  
//tcp服务器连接状态
enum tcp_server_states
{
	ES_TCPSERVER_NONE = 0,		//没有连接
	ES_TCPSERVER_ACCEPTED,		//有客户端连接上了 
	ES_TCPSERVER_CLOSING,		//即将关闭连接
	
};   



//LWIP回调函数使用的结构体
struct tcp_server_struct
{
	u8 state;               //当前连接状
	struct tcp_pcb *pcb;    //指向当前的pcb
	struct pbuf *p;         //指向接收/或传输的pbuf  
	u8  remoteip[4];        //连接到的客户端IP
	
}; 

void tcp_server_test(void);//TCP Server测试函数
err_t tcp_server_accept(void *arg,struct tcp_pcb *newpcb,err_t err);
err_t tcp_server_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);
void tcp_server_error(void *arg,err_t err);
err_t tcp_server_poll(void *arg, struct tcp_pcb *tpcb);
err_t tcp_server_sent(void *arg, struct tcp_pcb *tpcb, u16_t len);
void tcp_server_senddata(struct tcp_pcb *tpcb, struct tcp_server_struct *es);
void tcp_server_connection_close(struct tcp_pcb *tpcb, struct tcp_server_struct *es);

void tcppoll_thread_task(void* parameter);
void lwip_periodic_handle(void);

#endif 
