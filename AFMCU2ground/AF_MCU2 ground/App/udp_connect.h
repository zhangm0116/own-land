#ifndef __UDP_CONNECT_H
#define __UDP_CONNECT_H


#include "lwip/pbuf.h"
#include "lwip/udp.h"
#include "lwip/tcp.h"
 

#define UDP_RX_BUFSIZE		200	 

typedef struct _udp_connect_struct{

	u8_t  udp_rcv_buf[UDP_RX_BUFSIZE];
  u32_t  udp_rcv_len;	
	u8_t   romoteip[4];
	u32_t  romoteport;  


}udp_connect_struct;  

#define  UDP_AIR_PORT  14005
#define  UDP_GROUND_PORT  14000


//远端客户端地址   
#define REMOTE_IPADDR0	192
#define REMOTE_IPADDR1	168
#define REMOTE_IPADDR2	1
#define REMOTE_IPADDR3	50

//远端客户端端口号   

//define  UDP_REMOTE_PORT  8085


void udp_connect_init(void);
void udp_server_init(void);
void udp_senddata(struct udp_pcb *upcb);
void udp_connection_close(struct udp_pcb *upcb);
void udp_recv_handle(void *arg,struct udp_pcb *upcb,struct pbuf *p,struct ip_addr *addr,u16_t port);
void udp_recv_handle1(void *arg,struct udp_pcb *upcb,struct pbuf *p,struct ip_addr *addr,u16_t port);

#endif

