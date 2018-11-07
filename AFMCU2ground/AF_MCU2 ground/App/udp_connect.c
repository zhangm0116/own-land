
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
//#include "igmp.h"


static struct rt_thread   udp_fc_thread;
ALIGN(4)
static rt_uint8_t udp_fc_thread_stack[512];


static struct rt_thread   udp_ground_thread;
ALIGN(4)
static rt_uint8_t udp_ground_thread_stack[512];


u8_t sendbuf[62];
uint8_t  AirTxBuf[16];
struct rt_semaphore		sem_cgi_set; 
udp_connect_struct  udp_con_struct;


extern struct dev_status_t	dev_status_buf;
extern void UDP_fc_task(void *parament);
extern void UDP_ground_task(void *parament);

u8_t tcp_demo_sendbuf[]="UDP send data\r\n";

void udp_connect_init(void)
{
 int32_t		ret;
	
//  接收天空端状态包 
  ret =  rt_thread_init(&udp_fc_thread, "udp fc rev", UDP_fc_task, RT_NULL,
									&udp_fc_thread_stack[0], sizeof(udp_fc_thread_stack), 13, 5);
		if(ret == RT_EOK)
	{        
		rt_kprintf("udp fc Task init success \n");
		rt_thread_startup(&udp_fc_thread);
	}
	  //將cgi参数发给天空端
  	ret =  rt_thread_init(&udp_ground_thread, "udp gd tx", UDP_ground_task, RT_NULL,
									&udp_ground_thread_stack[0], sizeof(udp_ground_thread_stack),13, 5);
		if(ret == RT_EOK)
	 {        
		rt_kprintf("udp ground Task init success \n");
		rt_thread_startup(&udp_ground_thread);
	 }
  //cgi 参数设置完成信号量
	 
	 rt_sem_init(&sem_cgi_set, "sem_cgi_set", 0, RT_IPC_FLAG_PRIO);

}
void UDP_fc_task(void *parament)
{   
	
 	  struct sockaddr_in  client_addr;  
	  struct sockaddr_in  rtaddr;  
    int sock_fd,newconnect;                    /* client socked */  
    int i = 0;  
	  uint32_t addr_len=sizeof(struct sockaddr_in);
	 // struct ip_addr  ipaddr;
	  unsigned char optval = 1; 
	
	  //IP4_ADDR(&ipaddr, RT_LWIP_IPADDR0, RT_LWIP_IPADDR1, RT_LWIP_IPADDR2, RT_LWIP_IPADDR3);
    sock_fd = socket(AF_INET, SOCK_DGRAM, 0);   //UDP  
	 if(sock_fd==-1)
	 {
		 rt_kprintf("init socket fc failed");
		 
	 }
		client_addr.sin_family = AF_INET;
		client_addr.sin_port = htons(14000);
		client_addr.sin_addr.s_addr =inet_addr("192.168.1.103");
	  bind(sock_fd,(struct sockaddr *)&client_addr, sizeof (client_addr)); 
	  
	  rtaddr.sin_family = AF_INET;  
		rtaddr.sin_port = htons(14005);  
		rtaddr.sin_addr.s_addr =inet_addr("192.168.1.50");
   // connect(sock_fd,(struct sockaddr *)&rtaddr, sizeof (rtaddr));  // 关闭指定接收
	 
    // setsockopt(sock_fd,SOL_SOCKET,SO_BROADCAST,( void *)&optval,sizeof(optval));  //打开广播功能
	 
	  while(1){  
			
			
	    udp_con_struct.udp_rcv_len=recvfrom(sock_fd,&udp_con_struct.udp_rcv_buf,62,0,(struct sockaddr *)&rtaddr,&addr_len);
			
			 for(i=0;i<udp_con_struct.udp_rcv_len;i++)   
		 {
		    	rt_kprintf("%x ",udp_con_struct.udp_rcv_buf[i]); 
		 }
      rt_kprintf("\n"); 	
		 
		 
//		  mavlink_prase();
//	   	message_hangle(&mav_message);  
		   rt_thread_delay(50);
		 
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
		 rt_kprintf("init socket rcvcgi failed");   
		 
		 
	 }
		client_addr.sin_family = AF_INET;
		client_addr.sin_port = htons(11000);
		client_addr.sin_addr.s_addr =inet_addr("192.168.1.103");
	  bind(sock_fd,(struct sockaddr *)&client_addr, sizeof (client_addr)); 
	 
	  rtaddr.sin_family = AF_INET;
		rtaddr.sin_port = htons(11005);
		rtaddr.sin_addr.s_addr =inet_addr("192.168.1.50");
	 
	 //  IP4_ADDR(&multicast_ipaddr, 239, 0, 0, 1);
	 
    //connect(sock_fd,(struct sockaddr *)&rtaddr, sizeof (rtaddr));
	  while(1){ 
             if(rt_sem_take(&sem_cgi_set,50)==RT_EOK)
			    {   
						  sendto(sock_fd,&AirTxBuf,16,0,(struct sockaddr *)&rtaddr,sizeof (rtaddr));
						 	rt_kprintf("udp rcvcgi send"); 
             //	rt_thread_delay(150);  
					}	 
	 }

}


//UDP  server  init       
void udp_server_init(void)
{  
 	err_t err;
	struct udp_pcb *udppcb_fc;  
//	struct udp_pcb *udppcb_track;  
	struct ip_addr  ipaddr;           	 
	struct ip_addr  rtmaddr;   
 	u8_t  *tbuf;
 
  tbuf=mem_malloc(100);	
	if(tbuf==NULL)return ;	
	sprintf((char*)tbuf,"LOCAL IP:%d.%d.%d.%d \n",RT_LWIP_IPADDR0,RT_LWIP_IPADDR1,RT_LWIP_IPADDR2,RT_LWIP_IPADDR3);//IP
	rt_kprintf("%s",tbuf);  
 	sprintf((char*)tbuf,"UDP AIR Port:%d",UDP_AIR_PORT);
	rt_kprintf("%s",tbuf);  
	sprintf((char*)tbuf,"UDP GROUND Port:%d",UDP_GROUND_PORT);
	rt_kprintf("%s",tbuf); 
	mem_free(tbuf);
	udppcb_fc=udp_new();

	       
	if(udppcb_fc)
	{ 
		  IP4_ADDR(&ipaddr, RT_LWIP_IPADDR0, RT_LWIP_IPADDR1, RT_LWIP_IPADDR2, RT_LWIP_IPADDR3);
		  IP4_ADDR(&rtmaddr,REMOTE_IPADDR0, REMOTE_IPADDR1,REMOTE_IPADDR2, REMOTE_IPADDR3);
		
      err=udp_bind(udppcb_fc,&ipaddr,UDP_GROUND_PORT);   
		
			if(err==ERR_OK)	    
			{  
				udp_recv(udppcb_fc,udp_recv_handle,NULL); 
		  //  udp_connect(udppcb_fc,&rtmaddr,UDP_AIR_PORT);  
	       
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

		rt_kprintf("air   data  send"); 
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
      
	  upcb->remote_ip=*addr; 		  		                    //记录远程主机的IP地址
		upcb->remote_port=port;  		  	                    //记录远程主机的端口号
		udp_con_struct.romoteip[0]=upcb->remote_ip.addr&0xff; 	  	//IADDR4
		udp_con_struct.romoteip[1]=(upcb->remote_ip.addr>>8)&0xff; //IADDR3   
		udp_con_struct.romoteip[2]=(upcb->remote_ip.addr>>16)&0xff;//IADDR2  
		udp_con_struct.romoteip[3]=(upcb->remote_ip.addr>>24)&0xff;//IADDR1   
	  /////test	
		rt_kprintf("UDP client %d.%d.%d.%d connected \n",udp_con_struct.romoteip[0],udp_con_struct.romoteip[1],udp_con_struct.romoteip[2],udp_con_struct.romoteip[3]);   //test 
	 	for(i=0;i<udp_con_struct.udp_rcv_len;i++)
			 {    
 			    rt_kprintf("%x ",udp_con_struct.udp_rcv_buf[i]);
				      
			 }
			rt_kprintf("\n"); 
	//////		 
//    mavlink_prase();
//	 	message_hangle(&mav_message);    
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

/***************
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
//		  id30_num++; 
//		 if(id30_num>2000) id30_num=0;
//		 rt_kprintf("id30 count: %d\n", id30_num);
		 rt_sem_release(&sem_attitude_change);
     if((attitude_last.pitch!=attitude.pitch)||(attitude_last.roll!=attitude.roll)||(attitude_last.yaw!=attitude.yaw))
		 {  
			
			 	attitude_last.pitch=attitude.pitch;
			  attitude_last.roll=attitude.roll;
		    attitude_last.yaw=attitude.yaw;
		
		 }
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
********************/
