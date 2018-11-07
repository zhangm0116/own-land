/*
 * File      : upgrade.c
 * COPYRIGHT (C) 2017, TaiSync Development Team
 *
 * Change Logs:
 * Date           Author       Notes
 * 2017-12-28     hgz      	   the first version
 * ?
*/
#include "stm32f4xx.h"
#include "board.h"
#include "public.h"
#include "interface.h"
#include "ymodem.h"

#define ENABLE			1
#define DISABLE			0

extern struct rt_mailbox				mb;
extern struct upgrade_status			upgrade_table;
extern struct firmware_upgrade_status	upgrade_status;
uint8_t				uart_buffer[1024 + 16];

void uart_upgrade_thread_task(void* parameter)
{
	int32_t size;

	while (1)
	{
		if (upgrade_table.flag == UPGRADE_MODE)
		{
			rt_kprintf("upgrade run ...\n");
			size = Ymodem_Receive(&uart_buffer[0]);
			if ((size != 0) && (size != -1))
			{
				upgrade_table.flag = NORMAL_MODE;
				rt_kprintf("transfer done! file size is %d\n", size);
			}

		}

		rt_thread_delay(200);
	}
}
