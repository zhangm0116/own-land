#include "stm32f4xx.h"
#include "stm32f4_spi.h"
#include "board.h"
#include "rtthread.h"
#include "interface.h"
#include "uart_protocol.h"
#include "httpd.h"
#include "udp_connect.h"

#ifdef RT_USING_LWIP
#include "stm32f4_eth.h"
#include "stm32f4x7_eth.h"
#include "netif/ethernetif.h"
extern int lwip_system_init(void);
#endif

static struct rt_thread uart_ctl_thread;
ALIGN(4)
static rt_uint8_t uart_ctl_thread_stack[512];

static struct rt_thread uart_upgrade_thread;
ALIGN(4)
static rt_uint8_t uart_upgrade_thread_stack[2048];

extern void uart_upgrade_thread_task(void* parameter);
extern int32_t fpag_flash_init(void);

void init_thread_entry(void* parameter)
{
	uint32_t	timeout;
	int32_t		ret;
	
#ifdef RT_USING_LWIP
	/* reset Switch */
	Switch_reset();
	/* reset PHY */
	PHY_reset();
	
	rt_hw_stm32_eth_init();
	
	eth_system_device_init();
  
	lwip_system_init();
	
	udp_connect_init();
	
  httpd_init();
	
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
		rt_kprintf("uart protocl Task init sucess \n");
		rt_thread_startup(&uart_ctl_thread);
	}
	
	ret =  rt_thread_init(&uart_upgrade_thread, "uart_upgrade", uart_upgrade_thread_task, RT_NULL,
									&uart_upgrade_thread_stack[0], sizeof(uart_upgrade_thread_stack), 15, 5);
	if(ret == RT_EOK)
	{
		rt_kprintf("Upgrade Task init sucess \n");
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
