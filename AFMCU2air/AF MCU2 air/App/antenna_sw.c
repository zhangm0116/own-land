/***********************************************************************
文件名称：antenna_swtich.C
功    能：
编写时间:  2018.5
编 写 人： jerry
注    意：
***********************************************************************/



#include "stm32f4xx.h"
#include "stm32f4_spi.h"
#include "board.h"
#include "rtthread.h"
#include "interface.h"
#include "uart_protocol.h"
#include "udp_connect.h"
//#include "math.h"

extern  mavlink_attitude_t  attitude;
struct rt_semaphore		sem_attitude_change;    //切换天线的信号量
//struct antenna_data_t read_antenna_data;

extern struct rt_mailbox mb_spi_dev;
struct dev_status_t	dev_status_buf;


void  antenna_swtich_task(void *parament)
{  
	
	
	while(1) 
	{   

			 
		if_get_dev_status(&dev_status_buf, sizeof(struct dev_status_t));  
		if((rt_sem_take(&sem_attitude_change,0)==RT_EOK)) // 无等待获取
		{
			if(attitude.roll>0&attitude.roll<0.9)
			{
			    
					if((dev_status_buf.B_RSSI-dev_status_buf.A_RSSI)>2)
					{
								//B =3  A=1  
						if(dev_status_buf.current_ant!=1)  if_set_antenna_sw(1); 
						else ;
					}
					else if_set_antenna_sw(3); 
			}
			else if(attitude.roll<0&&attitude.roll>(-0.9)) 
			{
			   if((dev_status_buf.A_RSSI-dev_status_buf.B_RSSI)>2)
					{
								//B =3  A=1  
						if(dev_status_buf.current_ant!=3)  if_set_antenna_sw(3); 
						else ;
					}
					else if_set_antenna_sw(1); 
		  }
		 else 
			 {
					if((dev_status_buf.A_RSSI-dev_status_buf.B_RSSI)>=2)
					{
							//B =3  A=1  
					if(dev_status_buf.current_ant!=3)  if_set_antenna_sw(3); 
					else ;
					
				 }
				else if((dev_status_buf.B_RSSI-dev_status_buf.A_RSSI)>=2)
				 {   
					if(dev_status_buf.current_ant!=1)   if_set_antenna_sw(1); 
					else ;
					
				 }
				else ; 
		   } 
		 }	 

		if((attitude.roll==0)&&(attitude.pitch==0)&&(attitude.yaw==0)) //imu没有数据
		{
		
			if((dev_status_buf.A_RSSI-dev_status_buf.B_RSSI)>=2)
				{
						//B =3  A=1  
				if(dev_status_buf.current_ant!=3)  if_set_antenna_sw(3); 
				else ;
				
			 }
			else if((dev_status_buf.B_RSSI-dev_status_buf.A_RSSI)>=2)
			 {   
				if(dev_status_buf.current_ant!=1)   if_set_antenna_sw(1); 
				else ;
				
			 }
			else ;  
			 
  	}
   	if((dev_status_buf.A_RSSI>=85)&&(dev_status_buf.B_RSSI>=85))
		 {
		   
			 if(dev_status_buf.A_RSSI>=dev_status_buf.B_RSSI)  if_set_antenna_sw(3);
       else if_set_antenna_sw(1);			 
		 }
		 else ;

//			rt_kprintf("TX  mode   %d\n",read_antenna_data.Tx_mode);			
//			rt_kprintf("A Rssi   %d\n",read_antenna_data.A_Antenna_Rssi);
//			rt_kprintf("B Rssi   %d\n",read_antenna_data.B_Antenna_Rssi);
//			rt_kprintf("current antenna %d\n",read_antenna_data.Current_Antenna);
//			rt_kprintf("antenna  status %d\n",read_antenna_data.ant_status);
//		  rt_kprintf("fre num    %d\n",dev_status_buf.rf_rx_freq);
//			rt_kprintf("antenna test \n");
//			rt_kprintf("antenna test \n");
//			rt_kprintf("antenna test \n");
//			
		 
			rt_mb_send(&mb_spi_dev, (rt_uint32_t)&dev_status_buf);
      rt_thread_delay(100);
		
	}
}