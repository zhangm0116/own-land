#include "rthw.h"
#include "rtthread.h"
#include "stm32f4xx.h"
#include "board.h"

extern int  rt_application_init(void);
#ifdef RT_USING_FINSH
extern void finsh_system_init(void);
extern void finsh_set_device(const char* device);
#endif

#ifdef __CC_ARM
extern int Image$$RW_IRAM1$$ZI$$Limit;
#define STM32_SRAM_BEGIN    (&Image$$RW_IRAM1$$ZI$$Limit)
#elif __ICCARM__
#pragma section="HEAP"
#define STM32_SRAM_BEGIN    (__segment_end("HEAP"))
#else
extern int __bss_end;
#define STM32_SRAM_BEGIN    (&__bss_end)
#endif

/*******************************************************************************
* Function Name  : assert_failed
* Description    : Reports the name of the source file and the source line number
*                  where the assert error has occurred.
* Input          : - file: pointer to the source file name
*                  - line: assert error line source number
* Output         : None
* Return         : None
*******************************************************************************/
void assert_failed(u8* file, u32 line)
{
	rt_kprintf("\n\r Wrong parameter value detected on\r\n");
	rt_kprintf("       file  %s\r\n", file);
	rt_kprintf("       line  %d\r\n", line);

	while (1) ;
}

/**
 * This function will startup RT-Thread RTOS.
 */
void rtthread_startup(void)
{
	/* init board */
	rt_hw_board_init();

	/* show version */
	//rt_show_version();
	
#ifdef AIR_MODULE	
	
	rt_kprintf("\n\n#################################################################\n");
  rt_kprintf("Build time:		%s  %s by TaiSync Co.,Ltd\n",__DATE__,__TIME__);
  rt_kprintf("Device :		Wireless 2.4GHz AIR IT\n");
	rt_kprintf("Version :		TDD_10M_10M_ETH_1.1.0\n");
	rt_kprintf("#################################################################\n\n");
#else
	rt_kprintf("\n\n#################################################################\n");
  rt_kprintf("Build time:		%s  %s by TaiSync Co.,Ltd\n",__DATE__,__TIME__);
  rt_kprintf("Device :		Wireless 2.4GHz GROUND IT\n");
	rt_kprintf("Version :		TDD_10M_10M_ETH_1.1.0\n");
	rt_kprintf("#################################################################\n\n");
	
#endif	
	/* init tick */
	rt_system_tick_init();

	/* init kernel object */
	rt_system_object_init();

	/* init timer system */ 
	rt_system_timer_init();

   rt_system_heap_init((void*)STM32_SRAM_BEGIN, (void*)STM32_SRAM_END); //初始化堆空间  

	/* init scheduler system */
	rt_system_scheduler_init();

	/* init application */
	rt_application_init();  

#ifdef RT_USING_FINSH
	/* init finsh */
	
	finsh_system_init();
	finsh_set_device(CONSOLE_DEVICE);
	rt_kprintf("finsh init done\n");
#endif

    /* init timer thread */
  rt_system_timer_thread_init(); //软件定时器

	/* init idle thread */
	rt_thread_idle_init();//空闲任务初始化

	/* start scheduler */
	rt_system_scheduler_start();

	/* never reach here */
	return ;        
}



int main(void)		
{
	/* disable interrupt first */// 临界区
	rt_hw_interrupt_disable();

	/* startup RT-Thread RTOS */
	rtthread_startup();

	return 0;
	
}
