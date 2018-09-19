/*
 * File      : rtthread.h
 * COPYRIGHT (C) 2017, TaiSync Development Team
 *
 * Change Logs:
 * Date           Author       Notes
 * 2017-12-27     hgz      	   the first version
 * ?
*/
#include "rtthread.h"
#include "serial.h"
#include "uart_protocol.h"
#include "string.h"
#include "queue.h"
#include "public.h"
#include "interface.h"
#include "upgrade.h"



#define MAX_LENGTH     100


static uint16_t 				rd_index = 0;
static uint8_t 					rd_data[MAX_LENGTH];
static uint8_t 					tx_data[MAX_LENGTH];
static struct rt_semaphore		sem_rx;
static struct rt_semaphore		sem_queue;
struct upgrade_status			upgrade_table;
rt_device_t						uart1_dev;
Queue_uart						uart_queue;
rt_device_t						uart2_dev;


int write_data_opt(unsigned char val)
{
	rt_enter_critical();

	if (!IsQueueFull(&uart_queue))
	{
		EnQueue(&uart_queue, val);
		rt_exit_critical();
		rt_sem_release(&sem_queue);
		return 0;
	}
	else
	{
		rt_kprintf("Queue full !!!\n");
		rt_exit_critical();
		return -1;
		
	}
}

int write_cmd_wait(unsigned int timeout)
{
	if (rt_sem_take(&sem_queue, timeout) == -RT_ETIMEOUT)
	{
		return 0;
	}
	else
	{
		return 1;
	}
}

int32_t Receive_Byte(uint8_t *c, uint32_t timeout)
{
	int ret;
	ret = write_cmd_wait(timeout);
	if (ret == 1)
	{
		rt_enter_critical();
		if (!IsQueueEmpty(&uart_queue))
		{
			DeQueue(&uart_queue, c);
			rt_exit_critical();
			return 0;
		}
		else
		{
			rt_exit_critical();
			rt_kprintf("Queue empty !!\n");
			return -1;
		}
	}
	else
		return -1;
}

uint32_t Send_Byte(uint8_t c)
{
	rt_device_write(uart1_dev, 0, &c, 1);
	return 0;
}

unsigned char check_sum(void * buf, int len)
{
	unsigned char *data = buf;
	unsigned char sum = 0;
	int i;
	for (i = 0; i < len; i++)
	{
		sum += *data++;
	}
	sum = 0 - sum;
	return sum;
}

static rt_err_t uart_input(rt_device_t dev, rt_size_t size)
{
	
	rt_sem_release(&sem_rx);
	return 0;
	
}

int uart_ctl_init(void)
{

	uart1_dev = rt_device_find("uart2");
	if (uart1_dev != RT_NULL)
	{
		rt_device_open(uart1_dev, RT_DEVICE_OFLAG_RDWR);
		
	}
	else
	{     
		return (-1);
	}

	rt_device_set_rx_indicate(uart1_dev, uart_input); //定义接收的回调函数  uart_input
	rt_sem_init(&sem_rx, "sem_ctrl_rx", 0, RT_IPC_FLAG_PRIO);
	rt_sem_init(&sem_queue, "sem_queue", 0, RT_IPC_FLAG_PRIO);

	upgrade_table.flag = NORMAL_MODE;     

	InitQueue(&uart_queue);

	return 0;
}

static void len_error(void)
{
	uint8_t tdata[5] = { 0 };
	tdata[0] = 0x55;
	tdata[1] = 0xaa;
	tdata[2] = 0;
	tdata[3] = LEN_ERROR;
	tdata[4] = check_sum(&tx_data[0], 4);
	rt_device_write(uart1_dev, 0, tdata, 5);
	rt_kprintf("len error !\n");
	
}

void uart_ctl_thread_task(void* parameter)
{
	struct dev_status_t	rd_bb_status;
	uint8_t		  ch;
	uint32_t		value, cmd_rd;
	int					result;
	uint8_t 	  checkdata;
	uint8_t		  offset = 0;
  char *str;
	
	for (;;)
	{
		if (rt_sem_take(&sem_rx, 10) == RT_EOK)  //收到数据
		{   
			while (rt_device_read(uart1_dev, 0, &ch, 1) == 1)
			{  
				if (upgrade_table.flag == UPGRADE_MODE)
				{
					 write_data_opt(ch);
					}
				else
				{  
					rd_data[rd_index] = ch;
					rd_index++;

					if (rd_index > MAX_LENGTH)
					{
						rt_kprintf("rd_index out of range %d\n", rd_index);
						rd_index = 0;
					}

					if (rd_data[0] != 0x55)
					{
						rd_index = 0;
					}

					if (rd_index > 1)
					{
						if (rd_data[1] != 0xaa)
							rd_index = 0;
					}

					if (rd_index > rd_data[2] + 4)
					{
						rd_index = 0;
						offset = rd_data[2] + 4;
						checkdata = check_sum(&rd_data[0], rd_data[2] + 4);
						if (checkdata != rd_data[offset]) //校验和错误
						{
							tx_data[0] = 0x55;
							tx_data[1] = 0xaa;
							tx_data[2] = 0;
							tx_data[3] = CHECKDATA_ERROR;
							tx_data[4] = check_sum(&tx_data[0], 4);
							rt_device_write(uart1_dev, 0, tx_data, 5);
							rt_kprintf("check data error !\n");  
							
						}
						else
						{   
        
							
							switch (rd_data[3])
							{
							case SET_TX_FREQ:
								if (rd_data[2] > 4)//data len max is 4bytes
								{
									len_error();
								}
								else
								{  
									
									
									value = 0;
									if (rd_data[2] == 1)
										value = rd_data[4];
									else if (rd_data[2] == 2)
										value = rd_data[4] << 8 | rd_data[5];
									else if (rd_data[2] == 3)
										value = rd_data[4] << 16 | rd_data[5] << 8 | rd_data[6];
									else if (rd_data[2] == 4)
										value = rd_data[4] << 24 | rd_data[5] << 16 | rd_data[6] << 8 | rd_data[7];

									result = if_set_freq(value, 1);
									
//									rt_kprintf("%4d\n",value);  //test 
//									rt_kprintf("777");
//									rt_kprintf("%d\n",result);

									tx_data[0] = 0x55;
									tx_data[1] = 0xaa;
									tx_data[2] = result & 0xff;
									tx_data[3] = check_sum(&tx_data[0], 3);
									rt_device_write(uart1_dev, 0, tx_data, 4);
									
									
								}
								break;
							case SET_RX_FREQ:
								if (rd_data[2] > 4)//data len max is 4bytes
								{
									len_error();
								}
								else
								{
									value = 0;
									if (rd_data[2] == 1)
										value = rd_data[4];
									else if (rd_data[2] == 2)
										value = rd_data[4] << 8 | rd_data[5];
									else if (rd_data[2] == 3)
										value = rd_data[4] << 16 | rd_data[5] << 8 | rd_data[6];
									else if (rd_data[2] == 4)
										value = rd_data[4] << 24 | rd_data[5] << 16 | rd_data[6] << 8 | rd_data[7];
                   
//								  rt_kprintf("%d\n",value);
//									rt_kprintf("777"); 
									result = if_set_freq(value, 0); 
                  tx_data[0] = 0x55;
									tx_data[1] = 0xaa;
									tx_data[2] = result & 0xff;
									tx_data[3] = check_sum(&tx_data[0], 3);
									rt_device_write(uart1_dev, 0, tx_data, 4);
									
						
								}
								break;
							case SET_RF_POWER:  //发射功率？
								if (rd_data[2] > 4)
								{
									len_error();
								}
								else
								{
									value = 0;
									if (rd_data[2] == 1)
										value = rd_data[4];
									else if (rd_data[2] == 2)
										value = rd_data[4] << 8 | rd_data[5];
									else if (rd_data[2] == 3)
										value = rd_data[4] << 16 | rd_data[5] << 8 | rd_data[6];
									else if (rd_data[2] == 4)
										value = rd_data[4] << 24 | rd_data[5] << 16 | rd_data[6] << 8 | rd_data[7];
                 
//                  rt_kprintf("%4d\n",value);
//									rt_kprintf("777");
//									
									result = if_set_rf_power(value);
									tx_data[0] = 0x55;
									tx_data[1] = 0xaa;
									tx_data[2] = result & 0xff;
									tx_data[3] = check_sum(&tx_data[0], 3);
									rt_device_write(uart1_dev, 0, tx_data, 4);
								}
								break;
							case SET_BB_DOWNLINK:
								if (rd_data[2] > 4)
								{
									len_error();
								}
								else
								{
									value = 0;
									if (rd_data[2] == 1)
										value = rd_data[4];
									else if (rd_data[2] == 2)
										value = rd_data[4] << 8 | rd_data[5];
									else if (rd_data[2] == 3)
										value = rd_data[4] << 16 | rd_data[5] << 8 | rd_data[6];
									else if (rd_data[2] == 4)
										value = rd_data[4] << 24 | rd_data[5] << 16 | rd_data[6] << 8 | rd_data[7];
									
									
									
#ifdef AIR_MODULE	
									result = if_set_downlink(value, 1);
#else
									result = if_set_downlink(value, 0);
#endif									
									tx_data[0] = 0x55;
									tx_data[1] = 0xaa;
									tx_data[2] = result & 0xff;
									tx_data[3] = check_sum(&tx_data[0], 3);
									rt_device_write(uart1_dev, 0, tx_data, 4);
								}
								break;
					
							case GET_DEV_STATUS:
#ifdef AIR_MODULE								
								tx_data[0] = 0x55;
								tx_data[1] = 0xaa;
								tx_data[2] = sizeof(struct dev_status_t) + 5;
								tx_data[3] = 0x80;
								if_get_dev_status(&rd_bb_status, sizeof(struct dev_status_t));

								memcpy(&tx_data[4], &rd_bb_status, sizeof(struct dev_status_t));
               
								tx_data[sizeof(struct dev_status_t) + 4] = check_sum(&tx_data[0], sizeof(struct dev_status_t) + 4);
                    
								rt_device_write(uart2_dev, 0, tx_data, sizeof(struct dev_status_t) + 5);
#else
								tx_data[0] = 0x55;
								tx_data[1] = 0xaa;
								tx_data[2] = sizeof(struct dev_status_t) + 5;
								tx_data[3] = 0x80;
								if_get_dev_status(&rd_bb_status, sizeof(struct dev_status_t));

								memcpy(&tx_data[4], &rd_bb_status, sizeof(struct dev_status_t));
								tx_data[sizeof(struct dev_status_t) + 4] = check_sum(&tx_data[0], sizeof(struct dev_status_t) + 4);

								rt_device_write(uart2_dev, 0, tx_data, sizeof(struct dev_status_t) + 5);
#endif							
								break;
							case MCU_FLASH_ERASE:
								rt_kprintf("Start erase mcu flash...\n");
							
								MCU1_upgrade_enable(1);  //  启动方式
								MCU1_reset();
							
								rt_thread_delay(500);
								/* release upgrade enable */
								MCU1_upgrade_enable(0);
							     
								rt_thread_delay(1000);
								/* mcu erase */
								cmd_rd = 0;
								if_mcu_erase();
								cmd_rd = 1;
								GPIO_ResetBits(GPIOB, GPIO_Pin_11);
								rt_thread_delay(10);
								GPIO_SetBits(GPIOB, GPIO_Pin_11);	
								rt_kprintf("Done\n");
								tx_data[0] = 0x55;
								tx_data[1] = 0xaa;
								tx_data[2] = 0;	
								tx_data[3] = 0x80;
								tx_data[4] = check_sum(&tx_data[0],4);
								rt_device_write(uart1_dev,0,tx_data,5);
								break;	
							case MCU_FLASH_PROG:
								cmd_rd = 1;
								rt_kprintf("program mcu flash...\n");
								tx_data[0] = 0x55;
								tx_data[1] = 0xaa;
								tx_data[2] = 0;
								tx_data[3] = 0x80; 
								tx_data[4] = check_sum(&tx_data[0],4);
								rt_device_write(uart1_dev,0,tx_data,5);
								upgrade_table.flag = UPGRADE_MODE;
								upgrade_table.type = UP_MCU1;
								break;
							case FPGA_FLASH_ERASE:
								rt_kprintf("start erase fpga flash......\n");
								cmd_rd = 0;
								if_fpga_erase();
								cmd_rd = 1;
								rt_kprintf("Done\n");
								break;
							case FPGA_FLASH_PROG:
								cmd_rd = 1;
								rt_kprintf("program fpga flash...\n");
								upgrade_table.flag = UPGRADE_MODE;
								upgrade_table.type = UP_FPGA;
								break;	
							case GET_TYPE:
								tx_data[0] = 0x55;
								tx_data[1] = 0xaa;
								tx_data[2] = 1;
								tx_data[3] = 0x80;
#ifdef AIR_MODULE
								tx_data[4] = 0;
#else
								tx_data[4] = 1;
#endif							
								tx_data[5] = check_sum(&tx_data[0], 5);
								rt_device_write(uart1_dev, 0, tx_data, 6);
								break;
							
							case SET_ANTENNA_SW:
								if (rd_data[2] > 4)
								{
									len_error();
								}
								else
								{
									value = 0;
									if (rd_data[2] == 1)
										value = rd_data[4];
									else if (rd_data[2] == 2)
										value = rd_data[4] << 8 | rd_data[5];
									else if (rd_data[2] == 3)
										value = rd_data[4] << 16 | rd_data[5] << 8 | rd_data[6];
									else if (rd_data[2] == 4)
										value = rd_data[4] << 24 | rd_data[5] << 16 | rd_data[6] << 8 | rd_data[7];


									result = if_set_antenna_sw(value);
									tx_data[0] = 0x55;
									tx_data[1] = 0xaa;
									tx_data[2] = result & 0xff;
									tx_data[3] = check_sum(&tx_data[0], 3);
									rt_device_write(uart2_dev, 0, tx_data, 4);
								}
								break;					
							
							
							
							default:
								tx_data[0] = 0x55;
								tx_data[1] = 0xaa;
								tx_data[2] = 0;
								tx_data[3] = CMD_NOT_SUPPORT;
								tx_data[4] = check_sum(&tx_data[0], 4);
								rt_device_write(uart1_dev, 0, tx_data, 5);
								break;
							}
						}
					}
				}
			}
		} 
		
		rt_thread_delay(20);
	}
}
