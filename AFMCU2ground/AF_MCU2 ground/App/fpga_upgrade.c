#include "stm32f4xx.h"
#include "spi_bus.h"
#include "stm32f4_spi.h"
#include "spi_flash.h"


static struct rt_spi_configuration	config;
static struct rt_spi_device     	spi_device;
static struct stm32_spi_cs      	spi_cs;
struct spi_flash		  			*flash;

int32_t fpag_flash_init(void)
{
	GPIO_InitTypeDef		GPIO_InitStruct;
	rt_device_t				dev;
	int32_t					ret;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd  = GPIO_PuPd_UP;
	
	spi_cs.GPIOx = GPIOA;
	spi_cs.GPIO_Pin = GPIO_Pin_4;
	GPIO_InitStruct.GPIO_Pin = spi_cs.GPIO_Pin;
	GPIO_SetBits(spi_cs.GPIOx, spi_cs.GPIO_Pin);
	GPIO_Init(spi_cs.GPIOx, &GPIO_InitStruct);
	/* register device ad936x to spi1 */
	ret = rt_spi_bus_attach_device(&spi_device, "flash", "spi3",
									(void*)&spi_cs);

	if (ret < 0)
	{
		rt_kprintf("spi bus attach device failed!\n");
		return (-1);
	}

	dev = &(spi_device.parent);
	/* initial spi bus config */
	config.data_width = 8;
	config.max_hz = 8000000;
	config.mode = RT_SPI_MODE_0 | RT_SPI_MSB;
	dev->control(dev, 0, &config);
	
	flash = spi_flash_probe("flash");
	return 0;
}

int32_t fpga_erase(void)
{
	flash->erase(flash, 0, 0x1000000);
	
	return 0;
}

int fpga_upgrade(uint32_t offset, size_t len, void *buf)
{
	int ret;
	
	ret = flash->write(flash, offset, len, buf);
	
	return ret;
}

int if_fpga_erase(void)
{
	return fpga_erase();
}

int if_fpga_upgrade(unsigned int len, uint8_t *buf)
{
	static uint32_t offset = 0;
	
	fpga_upgrade(offset, len, buf);
	offset += len;
	return 0;
}
