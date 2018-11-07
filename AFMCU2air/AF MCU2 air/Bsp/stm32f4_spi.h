#ifndef _STM32F4_SPI_H_
#define _STM32F4_SPI_H_

#include "stm32f4xx.h"
#include "spi_bus.h"

struct stm32_spi_bus
{
    struct rt_spi_bus       parent;
    SPI_TypeDef 			*SPI;
    DMA_Stream_TypeDef 		*DMA_Channel_TX;
    DMA_Stream_TypeDef 		*DMA_Channel_RX;
    struct rt_semaphore		complete;
	int						dma_en;
};

struct stm32_spi_cs
{
    GPIO_TypeDef 	*GPIOx;
    uint16_t 		GPIO_Pin;
};

void rt_hw_spi_init(void);
#endif
