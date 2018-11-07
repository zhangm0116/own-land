
/***********************************************************************
文件名称：UDP_CONNECT.C
功    能：
编写时间:  2018.5
编 写 人： jerry
注    意：
***********************************************************************/

#include "udp_connect.h" 
#include "stdio.h"
#include "string.h" 
#include "lwip/tcp.h"
#include "lwip/memp.h"
#include "lwip/mem.h"
#include "lwip/pbuf.h"
#include "lwip/netifapi.h"
#include "math.h"  
#include "lwip/sockets.h"
#include "stm32f4xx_usart.h"
#include "interface.h"
#include "igmp.h"



mavlink_attitude_t  attitude;  
mavlink_attitude_t  attitude_last;

mavlink_status_t mav_return_status;
mavlink_message_t mav_message;
mavlink_attitude_quaternion_cov_t  attitude_gl;  

mavlink_gps_global_origin_t    gps_gl;

udp_connect_struct  udp_con_struct;
udp_connect_struct  udp_con_struct2;


static struct rt_thread   udp_fc_thread;
ALIGN(4)
static rt_uint8_t udp_fc_thread_stack[1024];


static struct rt_thread   udp_ground_thread;
ALIGN(4)
static rt_uint8_t udp_ground_thread_stack[1024];

static struct rt_thread   udp_rcvcgi_thread;
ALIGN(4)
static rt_uint8_t udp_recvcgi_thread_stack[512];


static struct rt_thread   udp_linkhub_thread;
ALIGN(4)
static rt_uint8_t udp_linkhub_thread_stack[512];


u8_t sendbuf[62];



		
struct rt_semaphore		sem_linkhub_tx;  
extern struct dev_status_t	dev_status_buf;
extern struct rt_mailbox mb_spi_dev;             

struct Quaternion_f{ 
	
	 float q1;
   float q2;
   float q3;
   float q4;
	
};
extern void UDP_linkhub_task(void *parament);
extern void UDP_fc_task(void *parament);
extern struct rt_semaphore		sem_attitude_change;	 
extern void UDP_ground_task(void *parament);
extern void UDP_recvcgi_task(void *parament);

//extern unsigned char  udp_tx_count;



u8_t tcp_demo_sendbuf[]="UDP demo send data\r\n";
u8_t truck_send_buf[100];
u8_t truck_len;

void udp_connect_init(void)
{
 int32_t		ret;
	
// 广播接受 
  ret =  rt_thread_init(&udp_fc_thread, "udp fc rev", UDP_fc_task, RT_NULL,
									&udp_fc_thread_stack[0], sizeof(udp_fc_thread_stack), 14, 5);
		if(ret == RT_EOK)
	{        
		rt_kprintf("udp fc Task init success \n");
		rt_thread_startup(&udp_fc_thread);
	}
	
	
  // CreateMulticastListen(); //组播接收
	
   //  udp_server_init(); 
  // UDP 发送62字节
	   ret =  rt_thread_init(&udp_ground_thread, "udp gd tx", UDP_ground_task, RT_NULL,
									&udp_ground_thread_stack[0], sizeof(udp_ground_thread_stack),13, 5);
		if(ret == RT_EOK)
	 {        
		rt_kprintf("udp ground Task init success \n");
		rt_thread_startup(&udp_ground_thread);
	 }
	 
	 	 ret =  rt_thread_init(&udp_linkhub_thread, "udp linkhub tx", UDP_linkhub_task, RT_NULL,
									&udp_linkhub_thread_stack[0], sizeof(udp_linkhub_thread_stack),15, 5);
		if(ret == RT_EOK)
	 {        
		rt_kprintf("udp linkhub Task init success \n");
		rt_thread_startup(&udp_linkhub_thread);
	 } 
	 
	   rt_sem_init(&sem_linkhub_tx, "sem_linkhub_tx", 0, RT_IPC_FLAG_PRIO);
	 
	   ret =  rt_thread_init(&udp_rcvcgi_thread, "udp rec cgi", UDP_recvcgi_task, RT_NULL,
									&udp_recvcgi_thread_stack[0], sizeof(udp_recvcgi_thread_stack), 15,5);
		if(ret == RT_EOK)
	{        
		rt_kprintf("udp recv cgi init success \n");
		rt_thread_startup(&udp_rcvcgi_thread);
	}
	 
}

void CreateMulticastListen(void)
{   
		 struct udp_pcb *multicast_udp;  
		 struct ip_addr  multicast_ipaddr;    //组播地址       	 
		 struct ip_addr  rtmaddr;   
		 err_t err_status;
	 //  igmp_init();
	
		 multicast_udp=udp_new();
	   IP4_ADDR(&multicast_ipaddr, 239, 0, 0, 1);
	   igmp_joingroup(IP_ADDR_ANY,&multicast_ipaddr);  //加入组播地址
	  if(!multicast_udp)
		{   
		   rt_kprintf("create multicast udp failed \r\n");
		   return;
	     
		}
		  err_status = udp_bind(multicast_udp,IP_ADDR_ANY,14555);
      if(err_status != ERR_OK){
				
        rt_kprintf("bind multicast udp failed \n\r");
				return;  
		 }  
	  
     udp_recv(multicast_udp,udp_recv_handle,NULL);  
	   rt_kprintf("udp  igmp packet init success\r\n ");
	 
}

void UDP_fc_task(void *parament)
{   
	
 	  struct sockaddr_in  client_addr;  
	  struct sockaddr_in  rtaddr;  
    int sock_fd,newconnect;                    /* client socked */  
    int i = 0;  
	  struct ip_addr  ipaddr;
	  unsigned char optval = 1; 
	
	  IP4_ADDR(&ipaddr, RT_LWIP_IPADDR0, RT_LWIP_IPADDR1, RT_LWIP_IPADDR2, RT_LWIP_IPADDR3);
    sock_fd = socket(AF_INET, SOCK_DGRAM, 0);   //UDP  
	 if(sock_fd==-1)
	 {
		 printf("init socket fc failed");
		 
	 }
		client_addr.sin_family = AF_INET;
		client_addr.sin_port = htons(14550);
		client_addr.sin_addr.s_addr =inet_addr("192.168.1.50");
	 // setsockopt(sock_fd, SOL_SOCKET,SO_REUSEADDR,(const void *)&optval, sizeof(optval) );  端口复用
	  bind(sock_fd,(struct sockaddr *)&client_addr, sizeof (client_addr)); 
	  
	  rtaddr.sin_family = AF_INET;  
		rtaddr.sin_port = htons(1455);  
		rtaddr.sin_addr.s_addr =inet_addr("192.168.1.255");
	 
   //connect(sock_fd,(struct sockaddr *)&rtaddr, sizeof (rtaddr));  // 关闭指定接收
	 
    //setsockopt(sock_fd,SOL_SOCKET,SO_BROADCAST,( void *)&optval,sizeof(optval));  //打开广播功能
	 
	  while(1){  
		 
 			
			udp_con_struct.udp_rcv_len=recv(sock_fd,&udp_con_struct.udp_rcv_buf,200,0);
		  for(i=0;i<udp_con_struct.udp_rcv_len;i++) 
		 {
		    	rt_kprintf("%x ",udp_con_struct.udp_rcv_buf[i]); 
		 }
      rt_kprintf("\n"); 	
		  mavlink_prase();
	   	message_hangle(&mav_message);  
		  rt_thread_delay(50);
		 
	}
}

void UDP_linkhub_task(void *parament)
{   

	  struct sockaddr_in  client_addr ,rtaddr;  
	  int sock_fd; 
    struct ip_addr  ipaddr;
 

	  IP4_ADDR(&ipaddr, RT_LWIP_IPADDR0, RT_LWIP_IPADDR1, RT_LWIP_IPADDR2, RT_LWIP_IPADDR3);
    sock_fd = socket(AF_INET, SOCK_DGRAM, 0);      //UDP  
	 if(sock_fd==-1)
	 {     
		 printf("init socket ground failed");   
		 
		 
	 }
		client_addr.sin_family = AF_INET;
		client_addr.sin_port = htons(12005);
		client_addr.sin_addr.s_addr =inet_addr("192.168.1.50");
	  bind(sock_fd,(struct sockaddr *)&client_addr, sizeof (client_addr)); 
	 
	  rtaddr.sin_family = AF_INET;
		rtaddr.sin_port = htons(14200);
		rtaddr.sin_addr.s_addr =inet_addr("192.168.1.200");
	 
	 //  IP4_ADDR(&multicast_ipaddr, 239, 0, 0, 1);
	 
    //connect(sock_fd,(struct sockaddr *)&rtaddr, sizeof (rtaddr));
	 
	  while(1){ 
			      if(rt_sem_take(&sem_linkhub_tx,50)==RT_EOK)
			    {   
						  sendto(sock_fd,&truck_send_buf,truck_len,0,(struct sockaddr *)&rtaddr,sizeof (rtaddr));
						
             //	rt_thread_delay(150);  
					}	 
	 }

}
void UDP_ground_task(void *parament)
{   

	  struct sockaddr_in  client_addr ,rtaddr;  
	  int sock_fd; 
    struct ip_addr  ipaddr;
 

	  IP4_ADDR(&ipaddr, RT_LWIP_IPADDR0, RT_LWIP_IPADDR1, RT_LWIP_IPADDR2, RT_LWIP_IPADDR3);
    sock_fd = socket(AF_INET, SOCK_DGRAM, 0);      //UDP  
	 if(sock_fd==-1)
	 {     
		 printf("init socket ground failed");   
		 
	 }
		client_addr.sin_family = AF_INET;
		client_addr.sin_port = htons(14005);
		client_addr.sin_addr.s_addr =inet_addr("192.168.1.50");
	  bind(sock_fd,(struct sockaddr *)&client_addr, sizeof (client_addr)); 
	 
	  rtaddr.sin_family = AF_INET;
		rtaddr.sin_port = htons(14000);
		rtaddr.sin_addr.s_addr =inet_addr("192.168.1.103");
	 
	 //  IP4_ADDR(&multicast_ipaddr, 239, 0, 0, 1);
	 
    //connect(sock_fd,(struct sockaddr *)&rtaddr, sizeof (rtaddr));
	 
	  while(1){ 
			      if(rt_mb_recv(&mb_spi_dev,(rt_uint32_t*)&dev_status_buf,70)==RT_EOK)
			    {   
						  sendto(sock_fd,&dev_status_buf,62,0,(struct sockaddr *)&rtaddr,sizeof (rtaddr));
						
             //	rt_thread_delay(150);  
					}	 
	 }    

}   
void UDP_recvcgi_task(void *parament)
{   

	  struct sockaddr_in  client_addr ,rtaddr;  
	  int sock_fd; 
    struct ip_addr  ipaddr;
    uint8_t i=0,index=0;
	  uint32_t  value=0;
	  uint32_t addr_len=sizeof(struct sockaddr_in);
	  uint8_t result=0;
	  IP4_ADDR(&ipaddr, RT_LWIP_IPADDR0, RT_LWIP_IPADDR1, RT_LWIP_IPADDR2, RT_LWIP_IPADDR3);
    sock_fd = socket(AF_INET, SOCK_DGRAM, 0);      //UDP  
	 if(sock_fd==-1)
	 {     
		 printf("init socket ground failed");   
		 
		 
	 }
	 
		client_addr.sin_family = AF_INET;
		client_addr.sin_port = htons(11005);
		client_addr.sin_addr.s_addr =inet_addr("192.168.1.50");
	  bind(sock_fd,(struct sockaddr *)&client_addr, sizeof (client_addr)); 
	 
	  rtaddr.sin_family = AF_INET;
		rtaddr.sin_port = htons(11000);
		rtaddr.sin_addr.s_addr =inet_addr("192.168.1.103");
	 
	  while(1){ 
			    
			udp_con_struct2.udp_rcv_len=recvfrom(sock_fd,&udp_con_struct2.udp_rcv_buf,16,0,(struct sockaddr *)&rtaddr,&addr_len);
			 for(i=0;i<udp_con_struct2.udp_rcv_len;i++) 
		 {
		    	rt_kprintf("%x ",udp_con_struct2.udp_rcv_buf[i]); 
		 }
			 rt_kprintf("\r\n");  
		 
			for(i=0;i<udp_con_struct2.udp_rcv_len;i++)
		 {
		     if(udp_con_struct2.udp_rcv_buf[i]!=0)
				 {
					  index=i;
					  break; 
		     }
				 if(i==15) index=16;
		 }
		 switch(index)
		 {
		   case 0 :
				 value=udp_con_struct2.udp_rcv_buf[0]*1000+udp_con_struct2.udp_rcv_buf[1]*100+udp_con_struct2.udp_rcv_buf[2]*10 \
				       +udp_con_struct2.udp_rcv_buf[3];
			         result = if_set_freq(value, 1);
			         rt_kprintf("set txfreq result %d\n",result);
			 break;
		  case 4 :
				   value=udp_con_struct2.udp_rcv_buf[4]*1000+udp_con_struct2.udp_rcv_buf[5]*100+udp_con_struct2.udp_rcv_buf[6]*10 \
				       +udp_con_struct2.udp_rcv_buf[7];
			       result = if_set_freq(value, 0);
			       rt_kprintf("set rxfreq result %d\n",result);
			 break;
			 case 8:
				   value=udp_con_struct2.udp_rcv_buf[8]*10000+udp_con_struct2.udp_rcv_buf[9]*1000+udp_con_struct2.udp_rcv_buf[10]*100 \
				       +udp_con_struct2.udp_rcv_buf[11]*10+udp_con_struct2.udp_rcv_buf[12];
			      result = if_set_rf_power(value);
			     rt_kprintf("set txpower result %d\n",result);
			 break;
			 case 9:
				   value=udp_con_struct2.udp_rcv_buf[9]*1000+udp_con_struct2.udp_rcv_buf[10]*100 \
				       +udp_con_struct2.udp_rcv_buf[11]*10+udp_con_struct2.udp_rcv_buf[12];
			      result = if_set_rf_power(value);
			     rt_kprintf("set txpower result %d\n",result);
			 break;
			case 13:
				   value=udp_con_struct2.udp_rcv_buf[13]*10+udp_con_struct2.udp_rcv_buf[14];
				   result = if_set_downlink(value, 1);
			     rt_kprintf("set downlink result %d\n",result);
			 break;
				case 14:
				   value=udp_con_struct2.udp_rcv_buf[14];
				   result = if_set_downlink(value, 1);
			     rt_kprintf("set downlink result %d\n",result);
			 break;
				case 15:
				   value=udp_con_struct2.udp_rcv_buf[14];
				   result = if_set_antenna_sw(value);
			     rt_kprintf("set antenna result %d\n",result);
			 break;
				case 16:
				   value=0;
				   result = if_set_downlink(value, 1);
			     rt_kprintf("set downlink result %d\n",result);
			 break;
			default: break;
			 
		 }
	 }

}


//UDP  server  init       
void udp_server_init(void)
{  
 	err_t err;
	struct udp_pcb *udppcb_fc;  
	struct udp_pcb *udppcb_track;  
	struct ip_addr  ipaddr;           	 
	struct ip_addr  rtmaddr;   
 	u8_t  *tbuf;
 
  tbuf=mem_malloc(100);	
	if(tbuf==NULL)return ;	
	sprintf((char*)tbuf,"LOCAL IP:%d.%d.%d.%d \n",RT_LWIP_IPADDR0,RT_LWIP_IPADDR1,RT_LWIP_IPADDR2,RT_LWIP_IPADDR3);//IP
	rt_kprintf("%s",tbuf);  
 	sprintf((char*)tbuf,"UDP FC Port:%d",UDP_FC_PORT);
	rt_kprintf("%s",tbuf);  
	sprintf((char*)tbuf,"UDP TRUCK Port:%d",UDP_TRUCK_PORT);
	rt_kprintf("%s",tbuf); 
	mem_free(tbuf);
	udppcb_fc=udp_new();

	       
	if(udppcb_fc)
	{ 
		  IP4_ADDR(&ipaddr, RT_LWIP_IPADDR0, RT_LWIP_IPADDR1, RT_LWIP_IPADDR2, RT_LWIP_IPADDR3);
		  IP4_ADDR(&rtmaddr,REMOTE_IPADDR0, REMOTE_IPADDR1,REMOTE_IPADDR2, REMOTE_IPADDR3);
		
      err=udp_bind(udppcb_fc,&ipaddr,UDP_FC_PORT);   
		
			if(err==ERR_OK)	    
			{
				udp_recv(udppcb_fc,udp_recv_handle,NULL); 
			  udp_connect(udppcb_fc,&rtmaddr,UDP_REMOTE_PORT);
				
			}   
			else ;  
		
//			IP4_ADDR(&rtmaddr,REMOTE_IPADDR4, REMOTE_IPADDR5,REMOTE_IPADDR6, REMOTE_IPADDR7);
//			 err=udp_bind(udppcb_track,&ipaddr,UDP_TRUCK_PORT);   
//			if(err==ERR_OK)	    
//			{   
//			 	udp_recv(udppcb_track,udp_recv_handle1,NULL); 
//			 	udp_connect(udppcb_track,&rtmaddr,UDP_REMOTE_PORT2);
//				
//	    }
//			else ;
			
     }  
} 

void udp_recv_handle(void *arg,struct udp_pcb *upcb,struct pbuf *p,struct ip_addr *addr,u16_t port)
{
	u32_t data_len = 0;
	struct pbuf *q; 
	u32_t  i=0;		

	
	if(p!=NULL) 
	{
		 memset(&udp_con_struct.udp_rcv_buf[0],0,UDP_RX_BUFSIZE);   
		for(q=p;q!=NULL;q=q->next)        
		{
			     
			if(q->len > (UDP_RX_BUFSIZE-data_len)) 
			memcpy(&udp_con_struct.udp_rcv_buf[0]+data_len,q->payload,(UDP_RX_BUFSIZE-data_len));   //拷贝数据
			else memcpy(&udp_con_struct.udp_rcv_buf[0]+data_len,q->payload,q->len);
			data_len += q->len;  	
			if(data_len > UDP_RX_BUFSIZE)         break;  
		  udp_con_struct.udp_rcv_len=q->len;
		}
      
	  upcb->remote_ip=*addr; 		  		                    
		upcb->remote_port=port;  		  	                    
		udp_con_struct.romoteip[0]=upcb->remote_ip.addr&0xff; 	  	//IADDR4
		udp_con_struct.romoteip[1]=(upcb->remote_ip.addr>>8)&0xff; //IADDR3   
		udp_con_struct.romoteip[2]=(upcb->remote_ip.addr>>16)&0xff;//IADDR2  
		udp_con_struct.romoteip[3]=(upcb->remote_ip.addr>>24)&0xff;//IADDR1   
	  //test	
		rt_kprintf("UDP client %d.%d.%d.%d connected \r\n",udp_con_struct.romoteip[0],udp_con_struct.romoteip[1],udp_con_struct.romoteip[2],udp_con_struct.romoteip[3]);   //test 
	 	for(i=0;i<udp_con_struct.udp_rcv_len;i++)
			 {    
 			    rt_kprintf("%x ",udp_con_struct.udp_rcv_buf[i]);
				      
			 }
			rt_kprintf("\r\n"); 
	//////		 
    mavlink_prase();
	 	message_hangle(&mav_message);    
    pbuf_free(p);  
			 
	}else
	{
		  udp_disconnect(upcb); 	
	} 
} 

//UDP服务器发送数据
void udp_senddata(struct udp_pcb *upcb)
{
	struct pbuf *ptr;
	ptr=pbuf_alloc(PBUF_TRANSPORT,strlen((char*)tcp_demo_sendbuf),PBUF_POOL);
	if(ptr)
	{
		ptr->payload=(void*)tcp_demo_sendbuf; 
		udp_send(upcb,ptr);	                    
		pbuf_free(ptr);    
	
	} 
} 

void udp_connection_close(struct udp_pcb *upcb)
{
	udp_disconnect(upcb); 
	udp_remove(upcb);	                   

	
}

void mavlink_prase(void)
{     u8_t i=0;
	
	 for(i=0;i<udp_con_struct.udp_rcv_len;i++)
      { 
				mavlink_frame_char(MAVLINK_COMM_0,udp_con_struct.udp_rcv_buf[i],&mav_message,&mav_return_status);
			}

//			 rt_kprintf("payload msg is :");    //test  
//			 for(i=0;i<mav_message.len;i++)
//			{  
//				rt_kprintf("data :%d",mav_message.payload64 [i]);	    
//			}   
			
			rt_kprintf("\n");
			rt_kprintf( "sys id is %d \n",mav_message.sysid);
			rt_kprintf("comp id is %d \n ",mav_message.compid);
		  rt_kprintf(" msg id is %d \n",mav_message.msgid);
			

}


void  message_hangle(mavlink_message_t *msg) 
{  
    switch (msg->msgid) { 
    case MAVLINK_MSG_ID_GPS_GLOBAL_ORIGIN:   
			  memcpy(&gps_gl,&mav_message.payload64,mav_message.len);
//test
//			rt_kprintf("gps  time_usec: %d  \n",gps_gl.time_usec );
//			rt_kprintf("gps  altitude:  %d  \n",gps_gl.altitude);
//			rt_kprintf("gps  latitude:  %d  \n",gps_gl.latitude); 
//			rt_kprintf("gps  longitude: %d  \n",gps_gl.longitude); 
//		
    	break;
	  case MAVLINK_MSG_ID_ATTITUDE_QUATERNION_COV :
				
			  memcpy(&attitude_gl,&mav_message.payload64,mav_message.len);   
		
//      rt_kprintf("attitude time_usec: %d  \n",attitude_gl.time_usec);  
//			rt_kprintf("attitude rollspeed: %d  \n",attitude_gl.rollspeed); 
//			printf("attitude  Q: %f ,%f ,%f,%f \n",attitude_gl.q [0],attitude_gl.q [1],attitude_gl.q [2],attitude_gl.q [3]); 
//			rt_kprintf("attitude pitchspeed: %d \n",attitude_gl.pitchspeed ); 
//			printf("attitude yawspeed: %f \n",attitude_gl.yawspeed); 
//			printf("attitude covariance : %f,%f,%f,%f,%f,%f,%f,%f,%f \n",attitude_gl.covariance [0]  /
//			attitude_gl.covariance [1],attitude_gl.covariance [2],attitude_gl.covariance [3],attitude_gl.covariance [4] /
//			attitude_gl.covariance [5],attitude_gl.covariance [6],attitude_gl.covariance [7],attitude_gl.covariance [8]); 
		
			quaternion_to_euler();
		  
		 break;
		case MAVLINK_MSG_ID_ATTITUDE :
			
		  memcpy(&attitude,&mav_message.payload64,mav_message.len);
		  printf("attitude roll: %f  \r\n",attitude.roll);
			printf("attitude pitch: %f  \r\n",attitude.pitch );
		  printf("attitude yaw: %f  \r\n",attitude.yaw);
    
     if((attitude_last.pitch!=attitude.pitch)||(attitude_last.roll!=attitude.roll)||(attitude_last.yaw!=attitude.yaw))
		 {  
			
			 	attitude_last.pitch=attitude.pitch;
			  attitude_last.roll=attitude.roll;
		    attitude_last.yaw=attitude.yaw;
			  rt_sem_release(&sem_attitude_change);
		
		 }
//		 	    truck_len=mavlink_msg_to_send_buffer(truck_send_buf,msg);
//		      rt_sem_release(&sem_linkhub_tx);
	 
      break;
		case  MAVLINK_MSG_ID_GLOBAL_POSITION_INT:
			    truck_len=mavlink_msg_to_send_buffer(truck_send_buf,msg);
		      rt_sem_release(&sem_linkhub_tx);
	    break;
		
		   default: break;
	
}}

void quaternion_to_euler(void) 
{      
	     
	     struct Quaternion_f *q;
       double *roll=0;
			 double *pitch=0; 
			 double *yaw=0;
	
	    q->q1=attitude_gl.q[0];
			q->q2=attitude_gl.q[1];
			q->q3=attitude_gl.q[2]; 
			q->q4=attitude_gl.q[3];
     
	    *roll = (atan2f(2.0f*(q->q1*q->q2 + q->q3*q->q4), 1 - 2.0f*(q->q2*q->q2 + q->q3*q->q3)));
			*pitch = asin(2.0f*(q->q1*q->q3 - q->q4*q->q2));
	    *yaw = atan2f(2.0f*(q->q1*q->q4 + q->q2*q->q3), 1 - 2.0f*(q->q3*q->q3 + q->q4*q->q4));
			*roll = *roll*180/3.1415926;     
			*pitch = *pitch*180/3.1415926;
			*yaw = *yaw*180/3.1415926;
	    
			printf("roll  is : %f  \n",*roll);
			printf("pitch is : %f  \n",*pitch);
			printf("yaw   is : %f  \n",*yaw);
			

}

