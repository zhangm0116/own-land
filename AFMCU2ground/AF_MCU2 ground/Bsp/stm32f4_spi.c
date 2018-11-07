#include "stm32f4xx.h"
#include "stm32f4_spi.h"

#ifdef RT_USING_SPI3
static struct stm32_spi_bus stm32_spi_bus_3;
#endif


/*
 * SPI DMA setting on STM32
 * SPI3 Tx --> DMA1 Stream 7
 * SPI3 Rx --> DMA1 Stream 0
 */
 
/*    SPI3     */
#define SPI3_GPIO_SCLK			GPIO_Pin_10
#define SPI3_SLCK_PIN_SOURCE	GPIO_PinSource10
#define SPI3_GPIO_MISO			GPIO_Pin_11
#define SPI3_MISO_PIN_SOURCE	GPIO_PinSource11
#define SPI3_GPIO_MOSI			GPIO_Pin_12
#define SPI3_MOSI_PIN_SOURCE	GPIO_PinSource12
#define SPI3_GPIO				GPIOC
#define SPI3_TX_DMA				DMA1_Stream7
#define SPI3_RX_DMA				DMA1_Stream0

#define SPI3_DR_ADDRESS			0x40003C0C

#define SPI_COMPLETE_DMA		(1<<2)
/****************************************spi bus ops functions********************************/
/**********************************************************************
 * this function configure spi clock.
 * param[in]  device pointer to struct rt_spi_device
 * param[in]  max_hz spi bus clock
 * return status of operate
 **********************************************************************/
rt_inline uint16_t get_spi_BaudRatePrescaler(struct rt_spi_device* device, uint32_t max_hz)
{
    RT_ASSERT(device != RT_NULL);
    uint16_t SPI_BaudRatePrescaler;
	struct stm32_spi_bus * stm32_spi_bus = (struct stm32_spi_bus *)device->bus;
	
	if(stm32_spi_bus->SPI == SPI1)
	{
		if(max_hz >= SystemCoreClock/4 && SystemCoreClock/4 <= 18000000)
		{
			SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;
		}
		else if(max_hz >= SystemCoreClock/8)
		{
			SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;
		}
		else if(max_hz >= SystemCoreClock/16)
		{
			SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_16;
		}
		else if(max_hz >= SystemCoreClock/32)
		{
			SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_32;
		}
		else if(max_hz >= SystemCoreClock/64)
		{
			SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_64;
		}
		else if(max_hz >= SystemCoreClock/128)
		{
			SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_128;
		}
		else if(max_hz >= SystemCoreClock/256)
		{
			SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;
		}
		else
		{
			/* min prescaler 256 */
			SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;
		}
	}
	else
	{
		if(max_hz >= SystemCoreClock/8 && SystemCoreClock/8 <= 18000000)
		{
			SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;
		}
		else if(max_hz >= SystemCoreClock/16)
		{
			SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;
		}
		else if(max_hz >= SystemCoreClock/32)
		{
			SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;
		}
		else if(max_hz >= SystemCoreClock/64)
		{
			SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_16;
		}
		else if(max_hz >= SystemCoreClock/128)
		{
			SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_32;
		}
		else if(max_hz >= SystemCoreClock/256)
		{
			SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_64;
		}
		else if(max_hz >= SystemCoreClock/512)
		{
			SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_128;
		}
		else
		{
			/* min prescaler 256 */
			SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;
		}
	}

    return SPI_BaudRatePrescaler;
}

/**********************************************************************
 * this function enable TX/RX DMA transceiving
 * param[in] spi_bus: pointer to struct stm32_spi_bus
 * param[in] send_addr: pointer to DMA send memory address
 * param[in] recv_addr: pointer to DMA recv memory address
 * param[in] size: send/recv memory buffer size
 **********************************************************************/
static void spi_dma_init(struct stm32_spi_bus * spi_bus, const void * send_addr, void * recv_addr, uint32_t size)
{
    static uint8_t dummy0 = 0xff;
    uint8_t dummy1;
	uint32_t reg_val;
    
    RT_ASSERT(spi_bus != RT_NULL);
	
	/* DMA RX */
	spi_bus->DMA_Channel_RX->NDTR = size;
	if(recv_addr != RT_NULL)
	{
		spi_bus->DMA_Channel_RX->M0AR = (uint32_t)recv_addr;
		reg_val = spi_bus->DMA_Channel_RX->CR;
		spi_bus->DMA_Channel_RX->CR = reg_val | DMA_SxCR_MINC;
	}
	else
	{
		spi_bus->DMA_Channel_RX->M0AR = (uint32_t)(&dummy1);
		reg_val = spi_bus->DMA_Channel_RX->CR;
		spi_bus->DMA_Channel_RX->CR = reg_val & (~DMA_SxCR_MINC);
	}

	/* DMA TX */
	spi_bus->DMA_Channel_TX->NDTR = size;
	if(send_addr != RT_NULL)
	{
		spi_bus->DMA_Channel_TX->M0AR = (uint32_t)send_addr;
		reg_val = spi_bus->DMA_Channel_TX->CR;
		spi_bus->DMA_Channel_TX->CR = reg_val | DMA_SxCR_MINC;
	}
	else
	{
		spi_bus->DMA_Channel_TX->M0AR = (uint32_t)(&dummy0);
		reg_val = spi_bus->DMA_Channel_TX->CR;
		spi_bus->DMA_Channel_TX->CR = reg_val & (~DMA_SxCR_MINC);
	}
	
	DMA_ITConfig(spi_bus->DMA_Channel_RX, DMA_IT_TC, ENABLE);
	DMA_ITConfig(spi_bus->DMA_Channel_TX, DMA_IT_TC, ENABLE);
	
	SPI_I2S_DMACmd(spi_bus->SPI, SPI_I2S_DMAReq_Rx, ENABLE);
	DMA_Cmd(spi_bus->DMA_Channel_RX, ENABLE);
	DMA_Cmd(spi_bus->DMA_Channel_TX, ENABLE);
	SPI_I2S_DMACmd(spi_bus->SPI, SPI_I2S_DMAReq_Tx, ENABLE);
}
/**********************************************************************
 * this function configure spi control.
 * param[in] device: pointer to struct rt_spi_device
 * param[in] configuration: pointer to struct rt_spi_configuration
 * return status of operate
 **********************************************************************/
static int32_t configure(struct rt_spi_device* device, struct rt_spi_configuration* configuration)
{
	RT_ASSERT(device != RT_NULL);
    RT_ASSERT(device->bus != RT_NULL);
    struct stm32_spi_bus * stm32_spi_bus = (struct stm32_spi_bus *)device->bus;
    SPI_InitTypeDef SPI_InitStructure;
	
	/* data_width */
    if(configuration->data_width <= 8)
    {
        SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
    }
    else if(configuration->data_width <= 16)
    {
        SPI_InitStructure.SPI_DataSize = SPI_DataSize_16b;
    }
    else
    {
        return -RT_EIO;
    }
    /* baudrate */
    SPI_InitStructure.SPI_BaudRatePrescaler = get_spi_BaudRatePrescaler(device, configuration->max_hz);
    /* CPOL */
    if(configuration->mode & RT_SPI_CPOL)
    {
        SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
    }
    else
    {
        SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
    }
    /* CPHA */
    if(configuration->mode & RT_SPI_CPHA)
    {
        SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
    }
    else
    {
        SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
    }
    /* MSB or LSB */
    if(configuration->mode & RT_SPI_MSB)
    {
        SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
    }
    else
    {
        SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_LSB;
    }
    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
    SPI_InitStructure.SPI_NSS  = SPI_NSS_Soft;
	SPI_InitStructure.SPI_CRCPolynomial = 7;
	
	SPI_Cmd(stm32_spi_bus->SPI, DISABLE);

    SPI_Init(stm32_spi_bus->SPI, &SPI_InitStructure);
	SPI_CalculateCRC(stm32_spi_bus->SPI, DISABLE);
	
    SPI_Cmd(stm32_spi_bus->SPI, ENABLE);

    return RT_EOK;
}
/**********************************************************************
 * spi transfer function.
 * param[in] device: pointer to struct rt_spi_device
 * param[in] message: pointer to struct rt_spi_message
 * return status of the operation
 **********************************************************************/
static int32_t xfer(struct rt_spi_device* device, struct rt_spi_message* message)
{
	RT_ASSERT(device != RT_NULL);
    RT_ASSERT(device->bus != RT_NULL);
    struct stm32_spi_bus			*stm32_spi_bus = (struct stm32_spi_bus *)device->bus;
    struct rt_spi_configuration		*config = &device->config;
    SPI_TypeDef						*SPI = stm32_spi_bus->SPI;
    struct stm32_spi_cs				*stm32_spi_cs = device->parent.user_data;
    uint32_t						size = message->length;
	int32_t 						timeout;
	
    if(config->data_width != 8)
    {
        rt_kprintf("spi drive not support 16bit date width!\n");
        return (-1);
    }
    
    /* take CS */
    if(message->cs_take)
    {
        GPIO_ResetBits(stm32_spi_cs->GPIOx, stm32_spi_cs->GPIO_Pin);
    }
 
    if((size >= 32) && (stm32_spi_bus->dma_en == 1))/*DMA*/
    {
        spi_dma_init(stm32_spi_bus, message->send_buf, message->recv_buf, message->length);
        rt_sem_take(&(stm32_spi_bus->complete), RT_WAITING_FOREVER);
    }
	else if(size >= 1) /*polling*/
    {
        const uint8_t * send_ptr = message->send_buf;
        uint8_t * recv_ptr = message->recv_buf;
        
        while(size--)
        {
            uint8_t data = 0xFF;

            if(send_ptr != RT_NULL)
            {
                data = *send_ptr++;
            }
			
			timeout = 0xFFFFF;
            while (SPI_I2S_GetFlagStatus(SPI, SPI_I2S_FLAG_TXE) == RESET && --timeout > 0);
            SPI_I2S_SendData(SPI, data);
			
			timeout = 0xFFFFF;
            while (SPI_I2S_GetFlagStatus(SPI, SPI_I2S_FLAG_RXNE) == RESET && --timeout > 0);
            data = SPI_I2S_ReceiveData(SPI);

            if(recv_ptr != RT_NULL)
            {
                *recv_ptr++ = data;
            }
        }
    }
    /* release CS */
    if(message->cs_release)
    {
        GPIO_SetBits(stm32_spi_cs->GPIOx, stm32_spi_cs->GPIO_Pin);
    }
    return message->length;
}

/* spi bus operation instance*/
static struct rt_spi_ops stm32_spi_ops =
{
    configure,
    xfer
};
/************************************spi bus init/register************************************/
static void RCC_Configuration()
{
#ifdef RT_USING_SPI3
	/* Enable USART2 GPIO clocks */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
	/* Enable SPI1 clock */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI3, ENABLE);
	/* DMA clock enable */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);
#endif
}

static void GPIO_Configuration()
{
#ifdef RT_USING_SPI3
    GPIO_InitTypeDef GPIO_InitStructure;
	
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
#endif
	
#ifdef RT_USING_SPI3
	/* Connect alternate function */
	GPIO_PinAFConfig(SPI3_GPIO, SPI3_SLCK_PIN_SOURCE, GPIO_AF_SPI3);
	GPIO_PinAFConfig(SPI3_GPIO, SPI3_MISO_PIN_SOURCE, GPIO_AF_SPI3);
	GPIO_PinAFConfig(SPI3_GPIO, SPI3_MOSI_PIN_SOURCE, GPIO_AF_SPI3);
	
	/* Configure SPI1 SLCK/MISO/MOSI PIN */
	GPIO_InitStructure.GPIO_Pin = SPI3_GPIO_SCLK | SPI3_GPIO_MISO | SPI3_GPIO_MOSI;
	GPIO_Init(SPI3_GPIO, &GPIO_InitStructure);
#endif
}

static void NVIC_Configuration()
{
#if (defined(RT_USING_SPI3) && defined(RT_SPI3_DMA))
	NVIC_InitTypeDef NVIC_InitStructure;
#endif
#if defined(RT_USING_SPI3) && defined(RT_SPI3_DMA)
	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Stream7_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Stream0_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
#endif
}

static void DMA_Configuration(void)
{
#if (defined(RT_USING_SPI3) && defined(RT_SPI3_DMA))	
	DMA_InitTypeDef DMA_InitStructure;
#endif
	
#if defined(RT_USING_SPI3) && defined(RT_SPI3_DMA)
	DMA_DeInit(SPI3_TX_DMA);
	DMA_StructInit(&DMA_InitStructure);
	
	DMA_InitStructure.DMA_Channel = DMA_Channel_7;
	DMA_InitStructure.DMA_PeripheralBaseAddr = SPI3_DR_ADDRESS;
	DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_Priority = DMA_Priority_Low;
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_1QuarterFull;
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	
	DMA_Init(SPI3_TX_DMA, &DMA_InitStructure);
	
	DMA_DeInit(SPI3_RX_DMA);
	DMA_StructInit(&DMA_InitStructure);
	
	DMA_InitStructure.DMA_Channel = DMA_Channel_0;
	DMA_InitStructure.DMA_PeripheralBaseAddr = SPI3_DR_ADDRESS;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_Priority = DMA_Priority_Low;
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_1QuarterFull;
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	
	DMA_Init(SPI3_RX_DMA, &DMA_InitStructure);
#endif	
}

/* this function init struct stm32_spi_bus and register spi bus */
void rt_hw_spi_init()
{
	RCC_Configuration();
	GPIO_Configuration();
	NVIC_Configuration();
	DMA_Configuration();

#ifdef RT_USING_SPI3	
	stm32_spi_bus_3.SPI = SPI3;
    stm32_spi_bus_3.DMA_Channel_TX = SPI3_TX_DMA;
    stm32_spi_bus_3.DMA_Channel_RX = SPI3_RX_DMA;
#if defined(RT_SPI3_DMA)
	stm32_spi_bus_3.dma_en         = 1;
#else
	stm32_spi_bus_3.dma_en         = 0;
#endif
	SPI_I2S_DeInit(SPI3);
	rt_sem_init(&(stm32_spi_bus_3.complete), "sem0", 0, RT_IPC_FLAG_PRIO);
    rt_spi_bus_register(&(stm32_spi_bus_3.parent), "spi3", &stm32_spi_ops);
#endif
}

#if defined(RT_USING_SPI3) && defined(RT_SPI3_DMA)
void DMA1_Stream7_IRQHandler() /*SPI 1 TX DMA*/
{
    /* enter interrupt */
    rt_interrupt_enter();
    
    if (DMA_GetITStatus(SPI3_TX_DMA, DMA_IT_TCIF7))
    {
        DMA_ClearITPendingBit(SPI3_TX_DMA, DMA_IT_TCIF7);
    }
    
    /* leave interrupt */
    rt_interrupt_leave();
}

void DMA1_Stream0_IRQHandler() /*SPI 1 RX DMA*/
{
    /* enter interrupt */
    rt_interrupt_enter();
    
    if (DMA_GetITStatus(SPI3_RX_DMA, DMA_IT_TCIF0))
    {
        DMA_ClearITPendingBit(SPI3_RX_DMA, DMA_IT_TCIF0);
		DMA_ITConfig(stm32_spi_bus_3.DMA_Channel_RX, DMA_IT_TC, DISABLE);
		DMA_ITConfig(stm32_spi_bus_3.DMA_Channel_TX, DMA_IT_TC, DISABLE);
		DMA_Cmd(stm32_spi_bus_3.DMA_Channel_TX, DISABLE);
		DMA_Cmd(stm32_spi_bus_3.DMA_Channel_RX, DISABLE);
        SPI_I2S_DMACmd(stm32_spi_bus_3.SPI, SPI_I2S_DMAReq_Rx, DISABLE);
        SPI_I2S_DMACmd(stm32_spi_bus_3.SPI, SPI_I2S_DMAReq_Tx, DISABLE);
        rt_sem_release(&(stm32_spi_bus_3.complete));
    }
    /* leave interrupt */
    rt_interrupt_leave();
}
#endif
