/*
 * File      : interface.c
 * COPYRIGHT (C) 2017, TaiSync Development Team
 *
 * Change Logs:
 * Date           Author       Notes
 * 2017-12-28     hgz      	   the first version
 * 2018-01-16     hgz      	   add interface API
 * …
*/

#include "rtthread.h"
#include "interface.h"
#include "stm32f4xx.h"
#include "spi_bus.h"
#include "stm32f4_spi.h"
#include "public.h"
#include "mcu_api.h"

static struct rt_spi_device 		*device;
static struct rt_spi_configuration	config;
static struct rt_spi_device     	spi_device;
static struct stm32_spi_cs      	spi_cs;

extern int mcu_antenna_sw(struct rt_spi_device *spi, uint8_t antenna_sw);
int if_bus_init()
{
	GPIO_InitTypeDef		GPIO_InitStruct;
	rt_device_t				dev;
	int32_t					ret;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);

	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd  = GPIO_PuPd_UP;
	
	spi_cs.GPIOx = GPIOE;    
	spi_cs.GPIO_Pin = GPIO_Pin_13;     
	GPIO_InitStruct.GPIO_Pin = spi_cs.GPIO_Pin;     
	GPIO_SetBits(spi_cs.GPIOx, spi_cs.GPIO_Pin);   
	GPIO_Init(spi_cs.GPIOx, &GPIO_InitStruct);
	/* register device ad936x to spi1 */
	ret = rt_spi_bus_attach_device(&spi_device, "baseband", "spi3",
									(void*)&spi_cs);

	if (ret < 0)
	{
		rt_kprintf("spi bus attach device failed!\n");
		return (-1);
	}

	dev = &(spi_device.parent);
	/* initial spi bus config */
	config.data_width = 8;
	config.max_hz = 100000;
	config.mode = RT_SPI_MODE_0 | RT_SPI_MSB;
	dev->control(dev, 0, &config);
	
	device = &spi_device;
	return 0;
}

int if_set_freq(uint32_t val, int ch)
{	
	int32_t ret;
	
	ret = mcu_set_freq(device, val, ch);
	
	return ret;
	
}

int if_set_rf_power(uint32_t val)
{
	int32_t ret;
	
	ret = mcu_set_vga(device, val);
  
	return ret;
}

int if_set_downlink(uint32_t val, uint8_t ch)
{
	int32_t ret;
	
	ret = mcu_set_mode(device, val, ch);
	
	return ret;
	
}

int if_get_dev_status(void *buf, uint32_t len)
{
	int32_t ret;

  mcu_get_status(device, buf, len);

	return ret;
	
}

int32_t mcu_erase(void)
{
	mcu_flash_erase(device, 0, 0x1D000);
	
	return 0;
}

int mcu_upgrade(uint32_t offset, size_t len, void *buf)
{
	int ret;
	ret = mcu_flash_write(device, offset, len, buf);
	
	return ret;
}

int if_mcu_erase(void)
{
	return mcu_erase();
}

int if_mcu_upgrade(unsigned int len, uint8_t *buf)
{
	static uint32_t	offset = 0;
	
	mcu_upgrade(offset, len, buf);
	offset += len;
	return 0;
}

void MCU1_upgrade_enable(uint8_t status) //bootloader upgrade enable pin
{
	if(status == 1)
	{
		GPIO_ResetBits(GPIOE, GPIO_Pin_13);
	}
	else if(status == 0)
	{
		GPIO_SetBits(GPIOE, GPIO_Pin_13);
	}
}
int if_set_antenna_sw(uint32_t val)
{
	int32_t ret;
	
	ret = mcu_antenna_sw(device, val);
	
	return ret;
}

void MCU1_reset(void)
{
	GPIO_InitTypeDef		GPIO_InitStruct;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);

	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd  = GPIO_PuPd_UP;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9;
	
	GPIO_SetBits(GPIOE, GPIO_Pin_9);
	GPIO_Init(GPIOE, &GPIO_InitStruct);
	GPIO_ResetBits(GPIOE, GPIO_Pin_9);
	rt_thread_delay(10);
	GPIO_SetBits(GPIOE, GPIO_Pin_9);
	
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
	GPIO_Init(GPIOE, &GPIO_InitStruct);
	
}
