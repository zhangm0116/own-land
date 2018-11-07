
/***********************************************************************
文件名称：.C
功    能：
编写时间：
编 写 人：
注    意：
***********************************************************************/

#include "tcp_server.h" 
#include "stdio.h"
#include "string.h"  
#include "lwip/tcp_impl.h"



u8 tcp_server_recvbuf[TCP_SERVER_RX_BUFSIZE];	
const u8 tcp_server_sendbuf[]="TCP Server send data\r\n";


static struct rt_thread  tcppoll_thread;
ALIGN(4)
static rt_uint8_t tcppoll_thread_stack[1024];

			

//TCP Server 测试全局状态标记变量
//bit7:0,没有数据要发送;1,有数据要发送
//bit6:0,没有收到数据;1,收到数据了.
//bit5:0,没有客户端连接上;1,有客户端连接上了.
//bit4~0:保留

u8 tcp_server_flag;	 
 
 
//TCP Server 
int tcp_server_init(void)
{
	struct tcp_pcb *tcppcb,*tcppcbconn;  	
	struct ip_addr  ipaddr;      
	
	 err_t err;  
	rt_int32_t 		ret;
	unsigned char *tbuf; 

	  tbuf=mem_malloc(100);
    sprintf((char*)tbuf,"LOCAL IP:%d.%d.%d.%d \n",RT_LWIP_IPADDR0,RT_LWIP_IPADDR1,RT_LWIP_IPADDR2,RT_LWIP_IPADDR3);//IP
    rt_kprintf("%s",tbuf);
	   
	  sprintf((char*)tbuf,"Port:%d \n",TCP_SERVER_PORT);        //服务器端口号
	  rt_kprintf("%s",tbuf);
	  mem_free(tbuf);
	
	  IP4_ADDR(&ipaddr, RT_LWIP_IPADDR0, RT_LWIP_IPADDR1, RT_LWIP_IPADDR2, RT_LWIP_IPADDR3);
	
	  tcppcb=tcp_new();	
	 if(tcppcb)
	 { 
	   err=tcp_bind(tcppcb,&ipaddr,TCP_SERVER_PORT);	  //tcp
		if(err==ERR_OK)	
		{   
			tcppcbconn=tcp_listen(tcppcb); 			        //设置tcppcb进入监听状态
			tcp_accept(tcppcbconn,tcp_server_accept); 	//初始化LWIP的tcp_accept的回调函数
	   }
	 }
	 else {
  		      return  -1 ;
		          
		    }
	 
//		ret =  rt_thread_init(&tcppoll_thread, "tcppoll_task", tcppoll_thread_task, RT_NULL,
//										&tcppoll_thread_stack[0], sizeof(tcppoll_thread_stack), 21, 20);
//		if(ret == RT_EOK)
//		{
//			rt_kprintf("tcppoll Task init sucess \n");
//			rt_thread_startup(&tcppoll_thread);
//		}
				
		 return 0;
				
} 

err_t tcp_server_accept(void *arg,struct tcp_pcb *newpcb,err_t err)
{  

  	err_t ret_err;
	  struct tcp_server_struct *es; 
	
 	LWIP_UNUSED_ARG(arg);
	LWIP_UNUSED_ARG(err);
	
	tcp_setprio(newpcb,TCP_PRIO_MIN);
	
	es=(struct tcp_server_struct*)mem_malloc(sizeof(struct tcp_server_struct)); //分配内存
 	if(es!=NULL)
	{
		es->state=ES_TCPSERVER_ACCEPTED;  	 //接收连接
		es->pcb=newpcb;
		es->p=NULL;
		     
		tcp_arg(newpcb,es);                   
		tcp_recv(newpcb,tcp_server_recv);	    //初始化tcp_recv()的回调函数
		tcp_err(newpcb,tcp_server_error); 	  //初始化tcp_err()回调函数
		tcp_poll(newpcb,tcp_server_poll,1);   //初始化tcp_poll回调函数
		tcp_sent(newpcb,tcp_server_sent);   	//初始化发送回调函数
		
    es->remoteip[0]=es->pcb->remote_ip.addr&0xff;
		es->remoteip[1]=(es->pcb->remote_ip.addr>>8)&0xff;
		es->remoteip[2]=(es->pcb->remote_ip.addr>>16)&0xff;
		es->remoteip[3]=(es->pcb->remote_ip.addr>>24)&0xff;
		
		rt_kprintf("Tcp client %d.%d.%d.%d connected \n",es->remoteip[0],es->remoteip[1],es->remoteip[2],es->remoteip[3]);   //test 
		tcp_server_flag|=1<<5;  				       //标记有客户端连上了
		
	 
		ret_err=ERR_OK;
		
	}
	else ret_err=ERR_MEM; 
	return  ret_err;
	
}


err_t tcp_server_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
	err_t ret_err;
	u32_t data_len = 0;
	u32_t  len=0;
	u32_t  i=0;
	struct pbuf *q;
  struct tcp_server_struct *es;
	  
	LWIP_ASSERT("arg != NULL",arg != NULL);
	es=(struct tcp_server_struct *)arg;
	if(p==NULL)                            //从客户端接收到空数据
	{
		es->state=ES_TCPSERVER_CLOSING;       //需要关闭TCP 连接了
		es->p=p; 
		ret_err=ERR_OK;
		
	}else if(err!=ERR_OK)	
	{
		if(p)  pbuf_free(p);	
		ret_err=err;
	}else if(es->state==ES_TCPSERVER_ACCEPTED) 	//处于连接状态
	{
		if(p!=NULL)  
		{
			memset(tcp_server_recvbuf,0,TCP_SERVER_RX_BUFSIZE);  //数据接收缓冲区清零
			
			for(q=p;q!=NULL;q=q->next)  
			{
				
				if(q->len > (TCP_SERVER_RX_BUFSIZE-data_len)) memcpy(tcp_server_recvbuf+data_len,q->payload,(TCP_SERVER_RX_BUFSIZE-data_len));//拷贝数据
				else memcpy(tcp_server_recvbuf+data_len,q->payload,q->len);
				data_len += q->len;  	
				if(data_len > TCP_SERVER_RX_BUFSIZE) break; //超出TCP客户端接收数组,跳出	
				
				len=q->len;
			    
			}  
			
		 	 tcp_server_flag|=1<<6;	     //标记接收到数据了
			
			 
                             for(i=0;i<len;i++)
			 { 
 			    rt_kprintf("%d ",tcp_server_recvbuf[i]);
				 
			 }
			 
			tcp_recved(tpcb,p->tot_len); 
			pbuf_free(p);  	//释放内存
			ret_err=ERR_OK;
		}
	}else    //服务器关闭了
	{
		tcp_recved(tpcb,p->tot_len);  //用于获取接收数据,通知LWIP可以获取更多数据
		es->p=NULL;
		pbuf_free(p);           //释放内存
		ret_err=ERR_OK;
	}
	return ret_err;
	
}


//报错
void tcp_server_error(void *arg,err_t err)
{  
	
	LWIP_UNUSED_ARG(err);  
	rt_kprintf("tcp error:%x\r\n",(u32_t)arg);   //打印tcp的地址
	if(arg!=NULL)mem_free(arg);          
	
} 


err_t tcp_server_poll(void *arg, struct tcp_pcb *tpcb)
{
	err_t ret_err;
	struct tcp_server_struct *es; 
	es=(struct tcp_server_struct *)arg;   
	
	tcp_server_flag|=(1<<7);  	                  //test 
	
	if(es!=NULL)
	{
		if(tcp_server_flag&(1<<7))	                //判断是否有数据要发送
		{    
			es->p=pbuf_alloc(PBUF_TRANSPORT,strlen((char*)tcp_server_sendbuf),PBUF_POOL);  //申请内存
			pbuf_take(es->p,(char*)tcp_server_sendbuf,strlen((char*)tcp_server_sendbuf));  // 复制发送数据到PBUF中  
			tcp_server_senddata(tpcb,es); 		     //轮询的时候发送要发送的数据
			tcp_server_flag&=~(1<<7);  			       //清除数据发送标志位
			if(es->p!=NULL) pbuf_free(es->p); 	     //释放内存	
			
		}else if(es->state==ES_TCPSERVER_CLOSING)//需要关闭连接?执行关闭操作
		{
			 tcp_server_connection_close(tpcb,es);//关闭连接
		}
		ret_err=ERR_OK;
	}else
	{
		tcp_abort(tpcb);   //终止连接,删除pcb控制块
		ret_err=ERR_ABRT; 
	}
	return ret_err;
} 

//lwIP tcp_sent的回调函数(当从远端主机接收到ACK信号后发送数据)

err_t tcp_server_sent(void *arg, struct tcp_pcb *tpcb, u16_t len)
{
	struct tcp_server_struct *es;
	LWIP_UNUSED_ARG(len); 
	es = (struct tcp_server_struct *) arg;
	if(es->p)tcp_server_senddata(tpcb,es);    
	return ERR_OK;
} 



void tcp_server_senddata(struct tcp_pcb *tpcb, struct tcp_server_struct *es)
{
	struct pbuf *ptr;
	u16 plen;
	err_t wr_err=ERR_OK;
	 while((wr_err==ERR_OK)&&es->p&&(es->p->len<=tcp_sndbuf(tpcb))) 
	 {
		ptr=es->p;
		wr_err=tcp_write(tpcb,ptr->payload,ptr->len,1);   //将要发送的数据加入发送缓冲队列中
		if(wr_err==ERR_OK)
		{ 
			plen=ptr->len;
			es->p=ptr->next;			    
			if(es->p)pbuf_ref(es->p);	  
			pbuf_free(ptr);
			tcp_recved(tpcb,plen); 		//更新tcp窗口大小
		}else if(wr_err==ERR_MEM)es->p=ptr;
		tcp_output(tpcb);         //将发送缓冲队列中的数据发送出去  
		
	 }
} 


//关闭tcp连接
void tcp_server_connection_close(struct tcp_pcb *tpcb, struct tcp_server_struct *es)
{
	tcp_close(tpcb);
	tcp_arg(tpcb,NULL);
	tcp_sent(tpcb,NULL);
	tcp_recv(tpcb,NULL);
	tcp_err(tpcb,NULL);
	tcp_poll(tpcb,NULL,0);
	if(es)mem_free(es); 
	tcp_server_flag&=~(1<<5);//标记连接断开了
	
	
}

void tcppoll_thread_task(void* parameter) 
{  
	 for(; ;)  
	 { 
		lwip_periodic_handle();
		 
	 }
	   rt_thread_delay(300);
    
}




u32_t TCPTimer=0;			//TCP查询计时器
u32_t ARPTimer=0;			//ARP查询计时器
u32_t lwip_localtime;		//lwip本地时间计数器,单位:ms

//LWIP轮询任务
void lwip_periodic_handle(void)
{
	
#if LWIP_TCP
	//每250ms调用一次tcp_tmr()函数
  if (lwip_localtime - TCPTimer >= TCP_TMR_INTERVAL)
  {
    TCPTimer =  lwip_localtime;
    tcp_tmr();
	//	rt_kprintf(" tick1 \n");
  }
#endif

  //ARP每5s周期性调用一次
  if ((lwip_localtime - ARPTimer) >= ARP_TMR_INTERVAL)
  {
    ARPTimer =  lwip_localtime;
    etharp_tmr();
	//	rt_kprintf(" tick2\n");
		
  }

	
#if LWIP_DHCP //如果使用DHCP的话
  //每500ms调用一次dhcp_fine_tmr()
  if (lwip_localtime - DHCPfineTimer >= DHCP_FINE_TIMER_MSECS)
  {
    DHCPfineTimer =  lwip_localtime;
    dhcp_fine_tmr();
    if ((lwipdev.dhcpstatus != 2)&&(lwipdev.dhcpstatus != 0XFF))
    { 
     // lwip_dhcp_process_handle();  //DHCP处理
    }
  }

  //每60s执行一次DHCP粗糙处理
  if (lwip_localtime - DHCPcoarseTimer >= DHCP_COARSE_TIMER_MSECS)
  {
    DHCPcoarseTimer =  lwip_localtime;
		dhcp_coarse_tmr();  
		
  }  
#endif
}




