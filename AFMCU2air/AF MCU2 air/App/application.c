#include "stm32f4xx.h"
#include "stm32f4_spi.h"
#include "board.h"
#include "rtthread.h"
#include "interface.h"
#include "uart_protocol.h"
#include "udp_connect.h" 
#include "timer.h" 
#include "httpd.h" 


#ifdef RT_USING_LWIP
#include "stm32f4_eth.h"
#include "stm32f4x7_eth.h"
#include "netif/ethernetif.h"
extern int lwip_system_init(void);
extern int tcp_server_init(void);
#endif

static struct rt_thread uart_ctl_thread;

ALIGN(4)
static rt_uint8_t uart_ctl_thread_stack[512];

static struct rt_thread   uart_upgrade_thread;
ALIGN(4)
static rt_uint8_t uart_upgrade_thread_stack[2048];


static struct rt_thread    antenna_swtich_thread;
ALIGN(4)
static rt_uint8_t  antenna_swtich_thread_stack[512];


struct rt_mailbox mb_spi_dev;
static char mb_pool[24];  




extern void uart_upgrade_thread_task(void* parameter);
extern int32_t fpag_flash_init(void);
extern void  antenna_swtich_task(void *parament);
extern struct rt_semaphore		sem_attitude_change;    

extern void udp_connect_init(void);

void init_thread_entry(void* parameter)
{
	uint32_t	timeout;
	int32_t		ret;
	
#ifdef RT_USING_LWIP
	/* reset Switch */
	Switch_reset();
	/* reset PHY */
	PHY_reset();
	 
	rt_hw_stm32_eth_init();          //将网口信息注册到rtt的设备管理中
	 
	eth_system_device_init();       //初始化网口发送和接受线程

	lwip_system_init();  
	
  udp_connect_init();
	
	httpd_init();
	
	//TIM3_Int_Init(5000-1,8400-1);  //500ms
      	  
	rt_kprintf("TCP/IP initialized!\n");
#endif   
  rt_thread_delay(3000);
		

	/* wait FPGA configure complete */
	timeout = 0;

 while (get_fpga_config_status() != 1)
	{
		rt_thread_delay(10);
		if (timeout++ >= 300)
		{
			rt_kprintf("FPGA Config Error...\n");
			goto err_handle;
			
		}
	}
 
	/* init spi interface */
	rt_hw_spi_init();
	
	ret = if_bus_init();
	
	if (ret < 0)
	{
		rt_kprintf("Interface init failed...\n");
		goto err_handle;
	}
	
	ret = fpag_flash_init();
	if (ret < 0)
	{
		rt_kprintf("FPGA Flash init failed...\n");
		goto err_handle;
	}
	

  ret = uart_ctl_init();
	if (ret < 0)
	{
		rt_kprintf("uart protocol init failed...\n");
		goto err_handle;
	}

	ret =  rt_thread_init(&uart_ctl_thread, "uart_ctl", uart_ctl_thread_task, RT_NULL,
									&uart_ctl_thread_stack[0], sizeof(uart_ctl_thread_stack), 15, 5);
	if(ret == RT_EOK)
	{        
		rt_kprintf("uart protocl Task init success \n");
		rt_thread_startup(&uart_ctl_thread);
	}
	
	rt_mb_init(&mb_spi_dev,"mbspi",&mb_pool[0], sizeof(mb_pool)/4,RT_IPC_FLAG_FIFO);     //将SPIDEV 读到的状态通过邮箱发送出去
	rt_sem_init(&sem_attitude_change, "sem_attitude_change", 0, RT_IPC_FLAG_PRIO);
	
	ret =  rt_thread_init(&antenna_swtich_thread, "ant_sw", antenna_swtich_task, RT_NULL,
									&antenna_swtich_thread_stack[0], sizeof(antenna_swtich_thread_stack), 11, 5);
		if(ret == RT_EOK)
	{        
		rt_kprintf("antenna swtich Task init success \n");
		rt_thread_startup(&antenna_swtich_thread);
	}
	ret =  rt_thread_init(&uart_upgrade_thread, "uart_upgrade", uart_upgrade_thread_task, RT_NULL,
									&uart_upgrade_thread_stack[0], sizeof(uart_upgrade_thread_stack), 15, 5);
	
	if(ret == RT_EOK)
	{          
		 rt_kprintf("Upgrade Task init success \n");  
	   rt_thread_startup(&uart_upgrade_thread);  
	}

	
err_handle:
		return;
}


int rt_application_init()
{
    rt_thread_t tid;

    tid = rt_thread_create("init",init_thread_entry, RT_NULL, 2048, 8, 20);

    if (tid != RT_NULL)
	{   
		rt_thread_startup(tid);
		
	}
    return 0;  
	   
}
