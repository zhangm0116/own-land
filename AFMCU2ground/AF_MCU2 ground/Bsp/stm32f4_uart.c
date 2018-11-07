#include "stm32f4xx.h"
#include "board.h"
#include "serial.h"

/*
 * USART DMA setting on STM32
 * USART1 Tx --> DMA2 Stream 7
 * USART1 Rx --> DMA2 Stream 2
 * USART2 Tx --> DMA1 Stream 6
 * USART2 Rx --> DMA1 Stream 5
 * USART3 Tx --> DMA1 Stream 3
 * USART3 Rx --> DMA1 Stream 1
 */

#ifdef RT_USING_UART1

#if defined(RT_UART1_INT_RX) && defined(RT_UART1_POLLING_TX)
struct stm32_serial_int_rx uart1_int_rx;
struct stm32_serial_device uart1 =
{
	.uart_device		= USART1,
	.rx_mode.int_rx		= &uart1_int_rx,
	.dma_tx				= RT_NULL
};
#endif

#if defined(RT_UART1_INT_RX) && defined(RT_UART1_DMA_TX)
struct stm32_serial_int_rx uart1_int_rx;
struct stm32_serial_dma_tx uart1_dma_tx;
struct stm32_serial_device uart1 =
{
	.uart_device		= USART1,
	.rx_mode.int_rx		= &uart1_int_rx,
	.dma_tx				= &uart1_dma_tx
};
#endif

#if defined(RT_UART1_DMA_RX) && defined(RT_UART1_DMA_TX)
struct stm32_serial_dma_rx uart1_dma_rx;
struct stm32_serial_dma_tx uart1_dma_tx;
struct stm32_serial_device uart1 =
{
	.uart_device		= USART1,
	.rx_mode.dma_rx		= &uart1_dma_rx,
	.dma_tx				= &uart1_dma_tx
};
#endif

struct rt_device uart1_device;
#endif

#ifdef RT_USING_UART2
#if defined(RT_UART2_INT_RX) && defined(RT_UART2_POLLING_TX)
struct stm32_serial_int_rx uart2_int_rx;
struct stm32_serial_device uart2 =
{
	.uart_device		= USART2,
	.rx_mode.int_rx		= &uart2_int_rx,
	.dma_tx				= RT_NULL
};
#endif
#if defined(RT_UART2_INT_RX) && defined(RT_UART2_DMA_TX)
struct stm32_serial_int_rx uart2_int_rx;
struct stm32_serial_dma_tx uart2_dma_tx;
struct stm32_serial_device uart2 =
{
	.uart_device		= USART2,
	.rx_mode.int_rx		= &uart2_int_rx,
	.dma_tx				= &uart2_dma_tx
};
#endif
#if defined(RT_UART2_DMA_RX) && defined(RT_UART2_DMA_TX)
struct stm32_serial_dma_rx uart2_dma_rx;
struct stm32_serial_dma_tx uart2_dma_tx;
struct stm32_serial_device uart2 =
{
	.uart_device		= USART2,
	.rx_mode.dma_rx		= &uart2_dma_rx,
	.dma_tx				= &uart2_dma_tx
};
#endif

struct rt_device uart2_device;
#endif

#ifdef RT_USING_UART3
#if defined(RT_UART3_INT_RX) && defined(RT_UART3_POLLING_TX)
struct stm32_serial_int_rx uart3_int_rx;
struct stm32_serial_device uart3 =
{
	.uart_device		= USART3,
	.rx_mode.int_rx		= &uart3_int_rx,
	.dma_tx				= RT_NULL
};
#endif
#if defined(RT_UART3_INT_RX) && defined(RT_UART3_DMA_TX)
struct stm32_serial_int_rx uart3_int_rx;
struct stm32_serial_dma_tx uart3_dma_tx;
struct stm32_serial_device uart3 =
{
	.uart_device		= USART3,
	.rx_mode.int_rx		= &uart3_int_rx,
	.dma_tx				= &uart3_dma_tx
};
#endif
#if defined(RT_UART3_DMA_RX) && defined(RT_UART3_DMA_TX)
struct stm32_serial_dma_rx uart3_dma_rx;
struct stm32_serial_dma_tx uart3_dma_tx;
struct stm32_serial_device uart3 =
{
	.uart_device		= USART3,
	.rx_mode.dma_rx		= &uart3_dma_rx,
	.dma_tx				= &uart3_dma_tx
};
#endif
struct rt_device uart3_device;
#endif

#define USART1_DR_Base  0x40011004
#define USART2_DR_Base  0x40004404
#define USART3_DR_Base  0x40004804

#define UART1_GPIO_TX		GPIO_Pin_6
#define UART1_TX_PIN_SOURCE GPIO_PinSource6
#define UART1_GPIO_RX		GPIO_Pin_7
#define UART1_RX_PIN_SOURCE GPIO_PinSource7
#define UART1_GPIO			GPIOB
#define UART1_TX_DMA		DMA2_Stream7
#define UART1_RX_DMA		DMA2_Stream2

#define UART2_GPIO_TX	    GPIO_Pin_5
#define UART2_TX_PIN_SOURCE GPIO_PinSource5
#define UART2_GPIO_RX	    GPIO_Pin_6
#define UART2_RX_PIN_SOURCE GPIO_PinSource6
#define UART2_GPIO	    	GPIOD
#define UART2_TX_DMA		DMA1_Stream6
#define UART2_RX_DMA		DMA1_Stream5

#define UART3_GPIO_TX		GPIO_Pin_8
#define UART3_TX_PIN_SOURCE GPIO_PinSource8
#define UART3_GPIO_RX		GPIO_Pin_9
#define UART3_RX_PIN_SOURCE GPIO_PinSource9
#define UART3_GPIO			GPIOD
#define UART3_TX_DMA		DMA1_Stream3
#define UART3_RX_DMA		DMA1_Stream1

/************************************UART bus init/register************************************/
static void RCC_Configuration(void)
{
#ifdef RT_USING_UART1
	/* Enable USART2 GPIO clocks */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	/* Enable USART2 clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
	/* DMA clock enable */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);
#endif

#ifdef RT_USING_UART2
	/* Enable USART2 GPIO clocks */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
	/* Enable USART2 clock */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
	/* DMA clock enable */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);
#endif

#ifdef RT_USING_UART3
	/* Enable USART3 GPIO clocks */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
	/* Enable USART3 clock */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
	/* DMA clock enable */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);
#endif
}

static void GPIO_Configuration(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;

#ifdef RT_USING_UART1
	/* Configure USART1 Rx/tx PIN */
	GPIO_InitStructure.GPIO_Pin = UART1_GPIO_RX | UART1_GPIO_TX;
	GPIO_Init(UART1_GPIO, &GPIO_InitStructure);

    /* Connect alternate function */
    GPIO_PinAFConfig(UART1_GPIO, UART1_TX_PIN_SOURCE, GPIO_AF_USART1);
    GPIO_PinAFConfig(UART1_GPIO, UART1_RX_PIN_SOURCE, GPIO_AF_USART1);
#endif

#ifdef RT_USING_UART2
	/* Configure USART2 Rx/tx PIN */
	GPIO_InitStructure.GPIO_Pin = UART2_GPIO_TX | UART2_GPIO_RX;
	GPIO_Init(UART2_GPIO, &GPIO_InitStructure);

    /* Connect alternate function */
    GPIO_PinAFConfig(UART2_GPIO, UART2_TX_PIN_SOURCE, GPIO_AF_USART2);
    GPIO_PinAFConfig(UART2_GPIO, UART2_RX_PIN_SOURCE, GPIO_AF_USART2);
#endif

#ifdef RT_USING_UART3
	/* Configure USART3 Rx/tx PIN */
	GPIO_InitStructure.GPIO_Pin = UART3_GPIO_RX | UART3_GPIO_RX;
	GPIO_Init(UART3_GPIO, &GPIO_InitStructure);

    /* Connect alternate function */
    GPIO_PinAFConfig(UART2_GPIO, UART3_TX_PIN_SOURCE, GPIO_AF_USART3);
    GPIO_PinAFConfig(UART2_GPIO, UART3_RX_PIN_SOURCE, GPIO_AF_USART3);
#endif
}

static void NVIC_Configuration(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;

#ifdef RT_USING_UART1
#ifdef RT_UART1_INT_RX
	/* Enable the USART1 Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
#endif
#ifdef RT_UART1_DMA_RX
	/* Enable the DMA1 Channel2 Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = DMA2_Stream2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
#endif
#ifdef RT_UART1_DMA_TX
	/* Enable the DMA1 Channel2 Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = DMA2_Stream7_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
#endif
#endif

#ifdef RT_USING_UART2
#ifdef RT_UART2_INT_RX
	/* Enable the USART1 Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
#endif
#ifdef RT_UART2_DMA_RX
	/* Enable the DMA1 Channel2 Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Stream5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
#endif
#ifdef RT_UART2_DMA_TX
	/* Enable the DMA1 Channel2 Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Stream6_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
#endif
#endif

#ifdef RT_USING_UART3
#ifdef RT_UART3_INT_RX
	/* Enable the USART1 Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
#endif
#ifdef RT_UART3_DMA_RX
	/* Enable the DMA1 Channel2 Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Stream1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
#endif
#ifdef RT_UART3_DMA_TX
	/* Enable the DMA1 Channel2 Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Stream3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
#endif
#endif
}

static void DMA_Configuration(void)
{
#if defined(RT_UART1_DMA_RX) || defined(RT_UART1_DMA_TX) || defined(RT_UART2_DMA_RX) || \
	defined(RT_UART2_DMA_TX) || defined(RT_UART3_DMA_RX) || defined(RT_UART3_DMA_TX)
	DMA_InitTypeDef DMA_InitStructure;
#endif

#ifdef RT_USING_UART1
#if defined(RT_UART1_DMA_RX)
	DMA_DeInit(UART1_RX_DMA);
	DMA_StructInit(&DMA_InitStructure);
	
	DMA_InitStructure.DMA_Channel = DMA_Channel_4;
	DMA_InitStructure.DMA_PeripheralBaseAddr = USART1_DR_Base;
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
	
	DMA_Init(UART1_RX_DMA, &DMA_InitStructure);
	DMA_ITConfig(UART1_RX_DMA, DMA_IT_TC | DMA_IT_TE, ENABLE);
#endif
#if defined(RT_UART1_DMA_TX)
	DMA_DeInit(UART1_TX_DMA);
	DMA_StructInit(&DMA_InitStructure);
	
	DMA_InitStructure.DMA_Channel = DMA_Channel_4;
	DMA_InitStructure.DMA_PeripheralBaseAddr = USART1_DR_Base;
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
	
	DMA_Init(UART1_TX_DMA, &DMA_InitStructure);
	DMA_ITConfig(UART1_TX_DMA, DMA_IT_TC | DMA_IT_TE, ENABLE);
#endif
#endif

#ifdef RT_USING_UART2
#if defined(RT_UART2_DMA_RX)
	DMA_DeInit(UART2_RX_DMA);
	DMA_StructInit(&DMA_InitStructure);
	
	DMA_InitStructure.DMA_Channel = DMA_Channel_4;
	DMA_InitStructure.DMA_PeripheralBaseAddr = USART2_DR_Base;
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
	
	DMA_Init(UART2_RX_DMA, &DMA_InitStructure);
	DMA_ITConfig(UART2_RX_DMA, DMA_IT_TC | DMA_IT_TE, ENABLE);
#endif
#if defined(RT_UART2_DMA_TX)
	DMA_DeInit(UART2_TX_DMA);
	DMA_StructInit(&DMA_InitStructure);
	
	DMA_InitStructure.DMA_Channel = DMA_Channel_4;
	DMA_InitStructure.DMA_PeripheralBaseAddr = USART2_DR_Base;
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
	
	DMA_Init(UART2_TX_DMA, &DMA_InitStructure);
	DMA_ITConfig(UART2_TX_DMA, DMA_IT_TC | DMA_IT_TE, ENABLE);
	
#endif
#endif

#ifdef RT_USING_UART3
#if defined(RT_UART3_DMA_RX)
	DMA_DeInit(UART3_RX_DMA);
	DMA_StructInit(&DMA_InitStructure);
	
	DMA_InitStructure.DMA_Channel = DMA_Channel_4;
	DMA_InitStructure.DMA_PeripheralBaseAddr = USART3_DR_Base;
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
	
	DMA_Init(UART3_RX_DMA, &DMA_InitStructure);
	DMA_ITConfig(UART3_RX_DMA, DMA_IT_TC | DMA_IT_TE, ENABLE);
	
#endif
#if defined(RT_UART3_DMA_TX)
	DMA_DeInit(UART3_TX_DMA);
	DMA_StructInit(&DMA_InitStructure);
	
	DMA_InitStructure.DMA_Channel = DMA_Channel_4;
	DMA_InitStructure.DMA_PeripheralBaseAddr = USART3_DR_Base;
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
	
	DMA_Init(UART3_TX_DMA, &DMA_InitStructure);
	DMA_ITConfig(UART3_TX_DMA, DMA_IT_TC | DMA_IT_TE, ENABLE);
	
#endif
#endif
}

/* this function init uart and register uart bus */
void rt_hw_usart_init()
{
	USART_InitTypeDef USART_InitStructure;

	RCC_Configuration();

	GPIO_Configuration();

	NVIC_Configuration();

	DMA_Configuration();

	/* uart init */
#ifdef RT_USING_UART1
	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART1, &USART_InitStructure);
#if STM32_CONSOLE_USART == 1
	/* register uart1 */
	rt_hw_serial_register(&uart1_device, "uart1",
		RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX | RT_DEVICE_FLAG_STREAM,
		&uart1);

	/* enable interrupt */
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
#elif defined(RT_UART1_INT_RX) && defined(RT_UART1_POLLING_TX)
	/* register uart1 */
	rt_hw_serial_register(&uart1_device, "uart1",
		RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX | RT_DEVICE_FLAG_STREAM,
		&uart1);

	/* enable interrupt */
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
#elif defined(RT_UART1_DMA_RX) && defined(RT_UART1_DMA_TX)
	/* register uart1 */
	rt_hw_serial_register(&uart1_device, "uart1",
		RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_DMA_RX | RT_DEVICE_FLAG_DMA_TX,
		&uart1);
	
	uart1_dma_tx.dma_channel = UART1_TX_DMA;
	uart1_dma_rx.dma_channel = UART1_RX_DMA;
	
	/* Enable USART1 DMA Tx request */
	USART_DMACmd(USART1, USART_DMAReq_Tx , ENABLE);
	/* Enable USART1 DMA Rx request */
	USART_DMACmd(USART1, USART_DMAReq_Rx , ENABLE);
#elif defined(RT_UART1_INT_RX) && defined(RT_UART1_DMA_TX)
	/* register uart1 */
	rt_hw_serial_register(&uart1_device, "uart1",
		RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX | RT_DEVICE_FLAG_DMA_TX,
		&uart1);
	
	uart1_dma_tx.dma_channel = UART1_TX_DMA;
	
	/* Enable USART1 DMA Tx request */
	USART_DMACmd(USART1, USART_DMAReq_Tx , ENABLE);
	/* enable interrupt */
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
#endif
#endif

#ifdef RT_USING_UART2
	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART2, &USART_InitStructure);
#if STM32_CONSOLE_USART == 2
	/* register uart2 */
	rt_hw_serial_register(&uart2_device, "uart2",
		RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX | RT_DEVICE_FLAG_STREAM,
		&uart2);

	/* enable interrupt */
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
#elif defined(RT_UART2_INT_RX) && defined(RT_UART2_POLLING_TX)
	/* register uart2 */
	rt_hw_serial_register(&uart2_device, "uart2",
		RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX | RT_DEVICE_FLAG_STREAM,
		&uart2);

	/* enable interrupt */
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
#elif defined(RT_UART2_DMA_RX) && defined(RT_UART2_DMA_TX)
	/* register uart2 */
	rt_hw_serial_register(&uart2_device, "uart2",
		RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_DMA_RX | RT_DEVICE_FLAG_DMA_TX,
		&uart2);
	
	uart2_dma_tx.dma_channel = UART2_TX_DMA;
	uart2_dma_rx.dma_channel = UART2_RX_DMA;
	
	/* Enable USART2 DMA Tx request */
	USART_DMACmd(USART2, USART_DMAReq_Tx , ENABLE);
	/* Enable USART2 DMA Rx request */
	USART_DMACmd(USART2, USART_DMAReq_Rx , ENABLE);
#elif defined(RT_UART2_INT_RX) && defined(RT_UART2_DMA_TX)
	/* register uart2 */
	rt_hw_serial_register(&uart2_device, "uart2",
		RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX | RT_DEVICE_FLAG_DMA_TX,
		&uart2);
	
	uart2_dma_tx.dma_channel = UART2_TX_DMA;

	/* Enable USART1 DMA Tx request */
	USART_DMACmd(USART2, USART_DMAReq_Tx , ENABLE);
	/* enable interrupt */
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
#endif
#endif

#ifdef RT_USING_UART3
	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART3, &USART_InitStructure);
#if STM32_CONSOLE_USART == 3
	/* register uart3 */
	rt_hw_serial_register(&uart3_device, "uart3",
		RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX | RT_DEVICE_FLAG_STREAM,
		&uart3);

	/* enable interrupt */
	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
#elif defined(RT_UART3_INT_RX) && defined(RT_UART3_POLLING_TX)
	/* register uart3 */
	rt_hw_serial_register(&uart3_device, "uart3",
		RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX | RT_DEVICE_FLAG_STREAM,
		&uart3);

	/* enable interrupt */
	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
#elif defined(RT_UART3_DMA_RX) && defined(RT_UART3_DMA_TX)
	/* register uart3 */
	rt_hw_serial_register(&uart3_device, "uart3",
		RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_DMA_RX | RT_DEVICE_FLAG_DMA_TX,
		&uart3);
	
	uart3_dma_tx.dma_channel = UART3_TX_DMA;
	uart3_dma_rx.dma_channel = UART3_RX_DMA;
	
	/* Enable USART3 DMA Tx request */
	USART_DMACmd(USART3, USART_DMAReq_Tx , ENABLE);
	/* Enable USART3 DMA Rx request */
	USART_DMACmd(USART3, USART_DMAReq_Rx , ENABLE);
#elif defined(RT_UART3_INT_RX) && defined(RT_UART3_DMA_TX)
	/* register uart3 */
	rt_hw_serial_register(&uart3_device, "uart3",
		RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX | RT_DEVICE_FLAG_DMA_TX,
		&uart3);
	
	uart3_dma_tx.dma_channel = UART3_TX_DMA;
	
	/* Enable USART1 DMA Tx request */
	USART_DMACmd(USART3, USART_DMAReq_Tx , ENABLE);
	/* enable interrupt */
	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
#endif
#endif
}

#ifdef RT_USING_UART1
void USART1_IRQHandler()
{
	/* enter interrupt */
	rt_interrupt_enter();

	rt_hw_serial_isr(&uart1_device);

	/* leave interrupt */
	rt_interrupt_leave();
}
#endif

#ifdef RT_USING_UART2
void USART2_IRQHandler()
{
	/* enter interrupt */
	rt_interrupt_enter();

	rt_hw_serial_isr(&uart2_device);

	/* leave interrupt */
	rt_interrupt_leave();
}
#endif

#ifdef RT_USING_UART3
void USART3_IRQHandler()
{
	/* enter interrupt */
	rt_interrupt_enter();

	rt_hw_serial_isr(&uart3_device);

	/* leave interrupt */
	rt_interrupt_leave();
}
#endif

#ifdef RT_UART1_DMA_RX
void DMA2_Stream2_IRQHandler()
{
	/* enter interrupt */
	rt_interrupt_enter();
	
	if(DMA_GetITStatus(UART1_RX_DMA, DMA_IT_TCIF2))
	{
		rt_hw_serial_dma_rx_isr(&uart1_device);
		DMA_ClearITPendingBit(UART1_RX_DMA, DMA_IT_TCIF2);
	}
	
	/* leave interrupt */
	rt_interrupt_leave();
}
#endif

#ifdef RT_UART1_DMA_TX
void DMA2_Stream7_IRQHandler()
{
	/* enter interrupt */
	rt_interrupt_enter();
    
	if(DMA_GetITStatus(UART1_TX_DMA, DMA_IT_TCIF7))
	{
		rt_hw_serial_dma_tx_isr(&uart1_device);
		DMA_ClearITPendingBit(UART1_TX_DMA, DMA_IT_TCIF7);
	}
    
	/* leave interrupt */
	rt_interrupt_leave();
}
#endif

#ifdef RT_UART2_DMA_RX
void DMA1_Stream5_IRQHandler()
{
	/* enter interrupt */
	rt_interrupt_enter();
	
	if(DMA_GetITStatus(UART2_RX_DMA, DMA_IT_TCIF5))
	{
		rt_hw_serial_dma_rx_isr(&uart2_device);
		DMA_ClearITPendingBit(UART2_RX_DMA, DMA_IT_TCIF5);
	}
	
	/* leave interrupt */
	rt_interrupt_leave();
}
#endif

#ifdef RT_UART2_DMA_TX
void DMA1_Stream6_IRQHandler()
{
		/* enter interrupt */
	rt_interrupt_enter();
    
	if(DMA_GetITStatus(UART2_TX_DMA, DMA_IT_TCIF6))
	{
		rt_hw_serial_dma_tx_isr(&uart2_device);
		DMA_ClearITPendingBit(UART2_TX_DMA, DMA_IT_TCIF6);
	}
    
	/* leave interrupt */
	rt_interrupt_leave();
}
#endif


#ifdef RT_UART3_DMA_RX
void DMA1_Stream1_IRQHandler()
{
	/* enter interrupt */
	rt_interrupt_enter();
	
	if(DMA_GetITStatus(UART3_RX_DMA, DMA_IT_TCIF1))
	{
		rt_hw_serial_dma_rx_isr(&uart3_device);
		DMA_ClearITPendingBit(UART3_RX_DMA, DMA_IT_TCIF1);
	}
	
	/* leave interrupt */
	rt_interrupt_leave();
}
#endif

#ifdef RT_UART3_DMA_TX
void DMA1_Stream3_IRQHandler()
{
	/* enter interrupt */
	rt_interrupt_enter();
    
	if(DMA_GetITStatus(UART3_TX_DMA, DMA_IT_TCIF3))
	{
		rt_hw_serial_dma_tx_isr(&uart3_device);
		DMA_ClearITPendingBit(UART3_TX_DMA, DMA_IT_TCIF3);
	}
    
	/* leave interrupt */
	rt_interrupt_leave();
}
#endif
