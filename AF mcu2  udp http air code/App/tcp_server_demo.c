
/***********************************************************************
�ļ����ƣ�.C
��    �ܣ�
��дʱ�䣺
�� д �ˣ�
ע    �⣺
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

			

//TCP Server ����ȫ��״̬��Ǳ���
//bit7:0,û������Ҫ����;1,������Ҫ����
//bit6:0,û���յ�����;1,�յ�������.
//bit5:0,û�пͻ���������;1,�пͻ�����������.
//bit4~0:����

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
	   
	  sprintf((char*)tbuf,"Port:%d \n",TCP_SERVER_PORT);        //�������˿ں�
	  rt_kprintf("%s",tbuf);
	  mem_free(tbuf);
	
	  IP4_ADDR(&ipaddr, RT_LWIP_IPADDR0, RT_LWIP_IPADDR1, RT_LWIP_IPADDR2, RT_LWIP_IPADDR3);
	
	  tcppcb=tcp_new();	
	 if(tcppcb)
	 { 
	   err=tcp_bind(tcppcb,&ipaddr,TCP_SERVER_PORT);	  //tcp
		if(err==ERR_OK)	
		{   
			tcppcbconn=tcp_listen(tcppcb); 			        //����tcppcb�������״̬
			tcp_accept(tcppcbconn,tcp_server_accept); 	//��ʼ��LWIP��tcp_accept�Ļص�����
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
	
	es=(struct tcp_server_struct*)mem_malloc(sizeof(struct tcp_server_struct)); //�����ڴ�
 	if(es!=NULL)
	{
		es->state=ES_TCPSERVER_ACCEPTED;  	 //��������
		es->pcb=newpcb;
		es->p=NULL;
		     
		tcp_arg(newpcb,es);                   
		tcp_recv(newpcb,tcp_server_recv);	    //��ʼ��tcp_recv()�Ļص�����
		tcp_err(newpcb,tcp_server_error); 	  //��ʼ��tcp_err()�ص�����
		tcp_poll(newpcb,tcp_server_poll,1);   //��ʼ��tcp_poll�ص�����
		tcp_sent(newpcb,tcp_server_sent);   	//��ʼ�����ͻص�����
		
    es->remoteip[0]=es->pcb->remote_ip.addr&0xff;
		es->remoteip[1]=(es->pcb->remote_ip.addr>>8)&0xff;
		es->remoteip[2]=(es->pcb->remote_ip.addr>>16)&0xff;
		es->remoteip[3]=(es->pcb->remote_ip.addr>>24)&0xff;
		
		rt_kprintf("Tcp client %d.%d.%d.%d connected \n",es->remoteip[0],es->remoteip[1],es->remoteip[2],es->remoteip[3]);   //test 
		tcp_server_flag|=1<<5;  				       //����пͻ���������
		
	 
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
	if(p==NULL)                            //�ӿͻ��˽��յ�������
	{
		es->state=ES_TCPSERVER_CLOSING;       //��Ҫ�ر�TCP ������
		es->p=p; 
		ret_err=ERR_OK;
		
	}else if(err!=ERR_OK)	
	{
		if(p)  pbuf_free(p);	
		ret_err=err;
	}else if(es->state==ES_TCPSERVER_ACCEPTED) 	//��������״̬
	{
		if(p!=NULL)  
		{
			memset(tcp_server_recvbuf,0,TCP_SERVER_RX_BUFSIZE);  //���ݽ��ջ���������
			
			for(q=p;q!=NULL;q=q->next)  
			{
				
				if(q->len > (TCP_SERVER_RX_BUFSIZE-data_len)) memcpy(tcp_server_recvbuf+data_len,q->payload,(TCP_SERVER_RX_BUFSIZE-data_len));//��������
				else memcpy(tcp_server_recvbuf+data_len,q->payload,q->len);
				data_len += q->len;  	
				if(data_len > TCP_SERVER_RX_BUFSIZE) break; //����TCP�ͻ��˽�������,����	
				
				len=q->len;
			    
			}  
			
		 	 tcp_server_flag|=1<<6;	     //��ǽ��յ�������
			
			 
                             for(i=0;i<len;i++)
			 { 
 			    rt_kprintf("%d ",tcp_server_recvbuf[i]);
				 
			 }
			 
			tcp_recved(tpcb,p->tot_len); 
			pbuf_free(p);  	//�ͷ��ڴ�
			ret_err=ERR_OK;
		}
	}else    //�������ر���
	{
		tcp_recved(tpcb,p->tot_len);  //���ڻ�ȡ��������,֪ͨLWIP���Ի�ȡ��������
		es->p=NULL;
		pbuf_free(p);           //�ͷ��ڴ�
		ret_err=ERR_OK;
	}
	return ret_err;
	
}


//����
void tcp_server_error(void *arg,err_t err)
{  
	
	LWIP_UNUSED_ARG(err);  
	rt_kprintf("tcp error:%x\r\n",(u32_t)arg);   //��ӡtcp�ĵ�ַ
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
		if(tcp_server_flag&(1<<7))	                //�ж��Ƿ�������Ҫ����
		{    
			es->p=pbuf_alloc(PBUF_TRANSPORT,strlen((char*)tcp_server_sendbuf),PBUF_POOL);  //�����ڴ�
			pbuf_take(es->p,(char*)tcp_server_sendbuf,strlen((char*)tcp_server_sendbuf));  // ���Ʒ������ݵ�PBUF��  
			tcp_server_senddata(tpcb,es); 		     //��ѯ��ʱ����Ҫ���͵�����
			tcp_server_flag&=~(1<<7);  			       //������ݷ��ͱ�־λ
			if(es->p!=NULL) pbuf_free(es->p); 	     //�ͷ��ڴ�	
			
		}else if(es->state==ES_TCPSERVER_CLOSING)//��Ҫ�ر�����?ִ�йرղ���
		{
			 tcp_server_connection_close(tpcb,es);//�ر�����
		}
		ret_err=ERR_OK;
	}else
	{
		tcp_abort(tpcb);   //��ֹ����,ɾ��pcb���ƿ�
		ret_err=ERR_ABRT; 
	}
	return ret_err;
} 

//lwIP tcp_sent�Ļص�����(����Զ���������յ�ACK�źź�������)

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
		wr_err=tcp_write(tpcb,ptr->payload,ptr->len,1);   //��Ҫ���͵����ݼ��뷢�ͻ��������
		if(wr_err==ERR_OK)
		{ 
			plen=ptr->len;
			es->p=ptr->next;			    
			if(es->p)pbuf_ref(es->p);	  
			pbuf_free(ptr);
			tcp_recved(tpcb,plen); 		//����tcp���ڴ�С
		}else if(wr_err==ERR_MEM)es->p=ptr;
		tcp_output(tpcb);         //�����ͻ�������е����ݷ��ͳ�ȥ  
		
	 }
} 


//�ر�tcp����
void tcp_server_connection_close(struct tcp_pcb *tpcb, struct tcp_server_struct *es)
{
	tcp_close(tpcb);
	tcp_arg(tpcb,NULL);
	tcp_sent(tpcb,NULL);
	tcp_recv(tpcb,NULL);
	tcp_err(tpcb,NULL);
	tcp_poll(tpcb,NULL,0);
	if(es)mem_free(es); 
	tcp_server_flag&=~(1<<5);//������ӶϿ���
	
	
}

void tcppoll_thread_task(void* parameter) 
{  
	 for(; ;)  
	 { 
		lwip_periodic_handle();
		 
	 }
	   rt_thread_delay(300);
    
}




u32_t TCPTimer=0;			//TCP��ѯ��ʱ��
u32_t ARPTimer=0;			//ARP��ѯ��ʱ��
u32_t lwip_localtime;		//lwip����ʱ�������,��λ:ms

//LWIP��ѯ����
void lwip_periodic_handle(void)
{
	
#if LWIP_TCP
	//ÿ250ms����һ��tcp_tmr()����
  if (lwip_localtime - TCPTimer >= TCP_TMR_INTERVAL)
  {
    TCPTimer =  lwip_localtime;
    tcp_tmr();
	//	rt_kprintf(" tick1 \n");
  }
#endif

  //ARPÿ5s�����Ե���һ��
  if ((lwip_localtime - ARPTimer) >= ARP_TMR_INTERVAL)
  {
    ARPTimer =  lwip_localtime;
    etharp_tmr();
	//	rt_kprintf(" tick2\n");
		
  }

	
#if LWIP_DHCP //���ʹ��DHCP�Ļ�
  //ÿ500ms����һ��dhcp_fine_tmr()
  if (lwip_localtime - DHCPfineTimer >= DHCP_FINE_TIMER_MSECS)
  {
    DHCPfineTimer =  lwip_localtime;
    dhcp_fine_tmr();
    if ((lwipdev.dhcpstatus != 2)&&(lwipdev.dhcpstatus != 0XFF))
    { 
     // lwip_dhcp_process_handle();  //DHCP����
    }
  }

  //ÿ60sִ��һ��DHCP�ֲڴ���
  if (lwip_localtime - DHCPcoarseTimer >= DHCP_COARSE_TIMER_MSECS)
  {
    DHCPcoarseTimer =  lwip_localtime;
		dhcp_coarse_tmr();  
		
  }  
#endif
}




