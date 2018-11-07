#ifndef __SERIAL_H__
#define __SERIAL_H__

#include "stm32f4xx.h"
#include "rtthread.h"

#define UART_INT_RX_BUFFER_SIZE		512
#define UART_TX_DMA_NODE_SIZE		4
#define UART_DMA_RX_SIZE			256
#define UART_RX_DMA_NODE_SIZE		4
#define UART_DMA_RX_BUFFER_SZIE		(UART_DMA_RX_SIZE * UART_RX_DMA_NODE_SIZE)

/* data node for Tx/Rx Mode */
struct stm32_serial_data_node
{
	uint8_t		*data_ptr;
	uint32_t	data_size;
	struct stm32_serial_data_node *next, *prev;
};

struct stm32_serial_dma_tx
{
	/* DMA Channel */
	DMA_Stream_TypeDef* dma_channel;

	/* data list head and tail */
	struct stm32_serial_data_node *list_head, *list_tail;

	/* data node memory pool */
	struct rt_mempool data_node_mp;
	uint8_t data_node_mem_pool[UART_TX_DMA_NODE_SIZE *
		(sizeof(struct stm32_serial_data_node))];
};

struct stm32_serial_dma_rx
{
	/* DMA Channel */
	DMA_Stream_TypeDef* dma_channel;
	
	uint8_t	rx_dma_buffer[UART_DMA_RX_BUFFER_SZIE];
	uint32_t read_index, save_index;
};

struct stm32_serial_int_rx
{
	uint8_t  rx_buffer[UART_INT_RX_BUFFER_SIZE];
	uint32_t read_index, save_index;
};

struct stm32_serial_device
{
	USART_TypeDef* uart_device;

	/* rx structure */
	union
	{
		struct stm32_serial_int_rx* int_rx;
		struct stm32_serial_dma_rx* dma_rx;
	}rx_mode;

	/* tx structure */
	struct stm32_serial_dma_tx* dma_tx;
};

int32_t rt_hw_serial_register(rt_device_t device, const char* name, uint32_t flag, struct stm32_serial_device *serial);

void rt_hw_serial_isr(rt_device_t device);
void rt_hw_serial_dma_tx_isr(rt_device_t device);
void rt_hw_serial_dma_rx_isr(rt_device_t device);

#endif
