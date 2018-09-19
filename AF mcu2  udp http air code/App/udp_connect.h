#ifndef __UDP_CONNECT_H
#define __UDP_CONNECT_H


#include "lwip/pbuf.h"
#include "lwip/udp.h"
#include "lwip/tcp.h"
#include "mavlink.h"   

#define UDP_RX_BUFSIZE		100	 

typedef struct _udp_connect_struct{

	u8_t   udp_rcv_buf[UDP_RX_BUFSIZE];
  u32_t  udp_rcv_len;	
	u8_t   romoteip[4];
	u32_t  romoteport;  

	

}udp_connect_struct;  


void CreateMulticastListen(void);
void  message_hangle(mavlink_message_t *msg);
void quaternion_to_euler(void);
void mavlink_prase(void);
void udp_server_init(void);
void udp_senddata(struct udp_pcb *upcb);
void udp_connection_close(struct udp_pcb *upcb);
void udp_recv_handle(void *arg,struct udp_pcb *upcb,struct pbuf *p,struct ip_addr *addr,u16_t port);
void udp_recv_handle1(void *arg,struct udp_pcb *upcb,struct pbuf *p,struct ip_addr *addr,u16_t port);

#endif

