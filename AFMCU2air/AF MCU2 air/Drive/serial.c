#include "serial.h"
#include "rthw.h"
#include "string.h"


static void rt_serial_enable_rx_dma(DMA_Stream_TypeDef* dma_channel, uint32_t address, uint32_t size)
{
	RT_ASSERT(dma_channel != RT_NULL);

	DMA_Cmd(dma_channel, DISABLE);

	dma_channel->M0AR = address;

	dma_channel->NDTR = size;

	DMA_Cmd(dma_channel, ENABLE);
}

static void rt_serial_enable_tx_dma(DMA_Stream_TypeDef* dma_channel, uint32_t address, uint32_t size)
{
	RT_ASSERT(dma_channel != RT_NULL);

	DMA_Cmd(dma_channel, DISABLE);

	dma_channel->M0AR = address;

	dma_channel->NDTR = size;

	DMA_Cmd(dma_channel, ENABLE);
}

static rt_err_t rt_serial_init (rt_device_t dev)
{
	struct stm32_serial_device* uart = (struct stm32_serial_device*) dev->user_data;

	if (!(dev->flag & RT_DEVICE_FLAG_ACTIVATED))
	{
		if (dev->flag & RT_DEVICE_FLAG_INT_RX)
		{
			rt_memset(uart->rx_mode.int_rx->rx_buffer, 0,
				sizeof(uart->rx_mode.int_rx->rx_buffer));
			uart->rx_mode.int_rx->read_index = 0;
			uart->rx_mode.int_rx->save_index = 0;
		}

		if (dev->flag & RT_DEVICE_FLAG_DMA_TX)
		{
			RT_ASSERT(uart->dma_tx->dma_channel != RT_NULL);
			uart->dma_tx->list_head = uart->dma_tx->list_tail = RT_NULL;

			/* init data node memory pool */
			rt_mp_init(&(uart->dma_tx->data_node_mp), "dn",
				uart->dma_tx->data_node_mem_pool,
				sizeof(uart->dma_tx->data_node_mem_pool),
				sizeof(struct stm32_serial_data_node));
		}
		
		if (dev->flag & RT_DEVICE_FLAG_DMA_RX)
		{
			RT_ASSERT(uart->rx_mode.dma_rx->dma_channel != RT_NULL);
			rt_memset(uart->rx_mode.dma_rx->rx_dma_buffer, 0,
				sizeof(uart->rx_mode.dma_rx->rx_dma_buffer));
			uart->rx_mode.dma_rx->read_index = 0;
			uart->rx_mode.dma_rx->save_index = 0;
			
			rt_serial_enable_rx_dma(uart->rx_mode.dma_rx->dma_channel, (uint32_t)uart->rx_mode.dma_rx->rx_dma_buffer, UART_DMA_RX_SIZE);
		}

		/* Enable USART */
		USART_Cmd(uart->uart_device, ENABLE);

		dev->flag |= RT_DEVICE_FLAG_ACTIVATED;
	}

	return RT_EOK;
}

static rt_err_t rt_serial_open(rt_device_t dev, rt_uint16_t oflag)
{
	return RT_EOK;
}

static rt_err_t rt_serial_close(rt_device_t dev)
{
	return RT_EOK;
}

static rt_size_t rt_serial_read (rt_device_t dev, rt_off_t pos, void* buffer, rt_size_t size)
{
	uint8_t* ptr;
	int32_t err_code;
	struct stm32_serial_device* uart;

	ptr = buffer;
	err_code = RT_EOK;
	uart = (struct stm32_serial_device*)dev->user_data;

	if (dev->flag & RT_DEVICE_FLAG_INT_RX)
	{
		/* interrupt mode Rx */
		while (size)
		{
			rt_base_t level;
			
			/* disable interrupt */
			level = rt_hw_interrupt_disable();

			if (uart->rx_mode.int_rx->read_index != uart->rx_mode.int_rx->save_index)
			{
				/* read a character */
				*ptr++ = uart->rx_mode.int_rx->rx_buffer[uart->rx_mode.int_rx->read_index];
				size--;

				/* move to next position */
				uart->rx_mode.int_rx->read_index ++;
				if (uart->rx_mode.int_rx->read_index >= UART_INT_RX_BUFFER_SIZE)
					uart->rx_mode.int_rx->read_index = 0;
			}
			else
			{
				/* set error code */
				err_code = -RT_EEMPTY;

				/* enable interrupt */
				rt_hw_interrupt_enable(level);
				break;
			}

			/* enable interrupt */
			rt_hw_interrupt_enable(level);
		}
	}
	else if(dev->flag & RT_DEVICE_FLAG_DMA_RX)
	{
		/* dma mode Rx */
		if(size != UART_DMA_RX_SIZE)
		{
			return 0;
		}
		rt_base_t level;
		
		/* disable interrupt */
		level = rt_hw_interrupt_disable();
		
		if (uart->rx_mode.dma_rx->read_index != uart->rx_mode.dma_rx->save_index)
		{
			memcpy(ptr, uart->rx_mode.dma_rx->rx_dma_buffer + uart->rx_mode.dma_rx->read_index, UART_DMA_RX_SIZE);
			
			ptr += UART_DMA_RX_SIZE;
			
			/* move to next position */
			uart->rx_mode.dma_rx->read_index += UART_DMA_RX_SIZE;
			if (uart->rx_mode.dma_rx->read_index >= UART_INT_RX_BUFFER_SIZE)
			{
				uart->rx_mode.dma_rx->read_index = 0;
			}
		}
		else
		{
			/* set error code */
			err_code = -RT_EEMPTY;

			/* enable interrupt */
			rt_hw_interrupt_enable(level);
		}
		/* enable interrupt */
		rt_hw_interrupt_enable(level);
	}
	else
	{
		
		/* polling mode */
		while ((uint32_t)ptr - (uint32_t)buffer < size)
		{
			while (uart->uart_device->SR & USART_FLAG_RXNE)
			{
				*ptr = uart->uart_device->DR & 0xff;
				ptr ++;
			}
		}
	}

	/* set error code */
	rt_set_errno(err_code);
	return (uint32_t)ptr - (uint32_t)buffer;
}

static rt_size_t rt_serial_write (rt_device_t dev, rt_off_t pos, const void* buffer, rt_size_t size)
{
	uint8_t* ptr;
	int32_t err_code;
	struct stm32_serial_device* uart;

	err_code = RT_EOK;
	ptr = (uint8_t*)buffer;
	uart = (struct stm32_serial_device*)dev->user_data;

	if (dev->flag & RT_DEVICE_FLAG_INT_TX)
	{
		/* interrupt mode Tx, does not support */
		RT_ASSERT(0);
	}
	else if (dev->flag & RT_DEVICE_FLAG_DMA_TX)
	{
		/* DMA mode Tx */

		/* allocate a data node */
		struct stm32_serial_data_node* data_node = (struct stm32_serial_data_node*)
			rt_mp_alloc (&(uart->dma_tx->data_node_mp), RT_WAITING_FOREVER);
		if (data_node == RT_NULL)
		{
			/* set error code */
			err_code = -RT_ENOMEM;
		}
		else
		{
			uint32_t level;

			/* fill data node */
			data_node->data_ptr 	= ptr;
			data_node->data_size 	= size;

			/* insert to data link */
			data_node->next = RT_NULL;

			/* disable interrupt */
			level = rt_hw_interrupt_disable();

			data_node->prev = uart->dma_tx->list_tail;
			if (uart->dma_tx->list_tail != RT_NULL)
				uart->dma_tx->list_tail->next = data_node;
			uart->dma_tx->list_tail = data_node;

			if (uart->dma_tx->list_head == RT_NULL)
			{
				/* start DMA to transmit data */
				uart->dma_tx->list_head = data_node;

				/* Enable DMA Channel */
				rt_serial_enable_tx_dma(uart->dma_tx->dma_channel,
					(rt_uint32_t)uart->dma_tx->list_head->data_ptr,
					uart->dma_tx->list_head->data_size);
			}

			/* enable interrupt */
			rt_hw_interrupt_enable(level);
		}
	}
	else
	{
		/* polling mode */
		if (dev->flag & RT_DEVICE_FLAG_STREAM)
		{
			/* stream mode */
			while (size)
			{
				if (*ptr == '\n')
				{
					while (!(uart->uart_device->SR & USART_FLAG_TXE));
					uart->uart_device->DR = '\r';
				}

				while (!(uart->uart_device->SR & USART_FLAG_TXE));
				uart->uart_device->DR = (*ptr & 0x1FF);

				++ptr; --size;
			}
		}
		else
		{
			/* write data directly */
			while (size)
			{
				while (!(uart->uart_device->SR & USART_FLAG_TXE));
				uart->uart_device->DR = (*ptr & 0x1FF);

				++ptr; --size;
			}
		}
	}

	/* set error code */
	rt_set_errno(err_code);

	return (uint32_t)ptr - (uint32_t)buffer;
}

static rt_err_t rt_serial_control (rt_device_t dev, rt_uint8_t cmd, void *args)
{
	struct stm32_serial_device* uart;

	RT_ASSERT(dev != RT_NULL);

	uart = (struct stm32_serial_device*)dev->user_data;
	switch (cmd)
	{
	case RT_DEVICE_CTRL_SUSPEND:
		/* suspend device */
		dev->flag |= RT_DEVICE_FLAG_SUSPENDED;
		USART_Cmd(uart->uart_device, DISABLE);
		break;

	case RT_DEVICE_CTRL_RESUME:
		/* resume device */
		dev->flag &= ~RT_DEVICE_FLAG_SUSPENDED;
		USART_Cmd(uart->uart_device, ENABLE);
		break;
	}

	return RT_EOK;
}

int32_t rt_hw_serial_register(rt_device_t device, const char* name, uint32_t flag, struct stm32_serial_device *serial)
{
	RT_ASSERT(device != RT_NULL);

	if (flag & RT_DEVICE_FLAG_INT_TX)
	{
		RT_ASSERT(0);
	}

	device->type 		= RT_Device_Class_Char;
	device->rx_indicate = RT_NULL;
	device->tx_complete = RT_NULL;
	device->init 		= rt_serial_init;
	device->open		= rt_serial_open;
	device->close		= rt_serial_close;
	device->read 		= rt_serial_read;
	device->write 		= rt_serial_write;
	device->control 	= rt_serial_control;
	device->user_data	= serial;

	/* register a character device */
	return rt_device_register(device, name, RT_DEVICE_FLAG_RDWR | flag);
}

void rt_hw_serial_isr(rt_device_t device)
{
	struct stm32_serial_device* uart = (struct stm32_serial_device*) device->user_data;

	if(USART_GetITStatus(uart->uart_device, USART_IT_RXNE) != RESET)
	{
		/* interrupt mode receive */
		RT_ASSERT(device->flag & RT_DEVICE_FLAG_INT_RX);

		/* save on rx buffer */
		while (uart->uart_device->SR & USART_FLAG_RXNE)
		{
			rt_base_t level;

			/* disable interrupt */
			level = rt_hw_interrupt_disable();

			/* save character */
			uart->rx_mode.int_rx->rx_buffer[uart->rx_mode.int_rx->save_index] = uart->uart_device->DR & 0xff;
			uart->rx_mode.int_rx->save_index ++;
			if (uart->rx_mode.int_rx->save_index >= UART_INT_RX_BUFFER_SIZE)
				uart->rx_mode.int_rx->save_index = 0;

			/* if the next position is read index, discard this 'read char' */
			if (uart->rx_mode.int_rx->save_index == uart->rx_mode.int_rx->read_index)
			{
				uart->rx_mode.int_rx->read_index ++;
				if (uart->rx_mode.int_rx->read_index >= UART_INT_RX_BUFFER_SIZE)
					uart->rx_mode.int_rx->read_index = 0;
			}

			/* enable interrupt */
			rt_hw_interrupt_enable(level);
		}

		/* invoke callback */
		if (device->rx_indicate != RT_NULL)
		{
			rt_size_t rx_length;

			/* get rx length */
			rx_length = uart->rx_mode.int_rx->read_index > uart->rx_mode.int_rx->save_index ?
				UART_INT_RX_BUFFER_SIZE - uart->rx_mode.int_rx->read_index + uart->rx_mode.int_rx->save_index :
				uart->rx_mode.int_rx->save_index - uart->rx_mode.int_rx->read_index;

			device->rx_indicate(device, rx_length);
		}
	}
	
	if(USART_GetFlagStatus(uart->uart_device, USART_FLAG_ORE) != RESET)
	{
		USART_ReceiveData(uart->uart_device);
	}
}

void rt_hw_serial_dma_tx_isr(rt_device_t device)
{
	uint32_t level;
	struct stm32_serial_data_node* data_node;
	struct stm32_serial_device* uart = (struct stm32_serial_device*) device->user_data;

	/* DMA mode transmit */
	RT_ASSERT(device->flag & RT_DEVICE_FLAG_DMA_TX);

	/* get the first data node */
	data_node = uart->dma_tx->list_head;
	RT_ASSERT(data_node != RT_NULL);

	/* invoke call to notify tx complete */
	if (device->tx_complete != RT_NULL)
		device->tx_complete(device, data_node->data_ptr);

	/* disable interrupt */
	level = rt_hw_interrupt_disable();

	/* remove list head */
	uart->dma_tx->list_head = data_node->next;
	if (uart->dma_tx->list_head == RT_NULL) /* data link empty */
		uart->dma_tx->list_tail = RT_NULL;

	/* enable interrupt */
	rt_hw_interrupt_enable(level);

	/* release data node memory */
	rt_mp_free(data_node);

	if (uart->dma_tx->list_head != RT_NULL)
	{
		/* transmit next data node */
		rt_serial_enable_tx_dma(uart->dma_tx->dma_channel,
			(rt_uint32_t)uart->dma_tx->list_head->data_ptr,
			uart->dma_tx->list_head->data_size);
	}
	else
	{
		/* no data to be transmitted, disable DMA */
		DMA_Cmd(uart->dma_tx->dma_channel, DISABLE);
	}
}

void rt_hw_serial_dma_rx_isr(rt_device_t device)
{
	rt_base_t level;
	struct stm32_serial_device* uart = (struct stm32_serial_device*) device->user_data;
	
	/* DMA mode receive */
	RT_ASSERT(device->flag & RT_DEVICE_FLAG_DMA_RX);
	
	/* disable interrupt */
	level = rt_hw_interrupt_disable();
	
	uart->rx_mode.dma_rx->save_index += UART_DMA_RX_SIZE;
	if (uart->rx_mode.dma_rx->save_index >= UART_DMA_RX_BUFFER_SZIE)
	{
		uart->rx_mode.dma_rx->save_index = 0;
	}
	
	/* if the next position is read index, discard this 'read char' */
	if (uart->rx_mode.dma_rx->save_index == uart->rx_mode.dma_rx->read_index)
	{
		uart->rx_mode.dma_rx->read_index += UART_DMA_RX_SIZE;
		if (uart->rx_mode.dma_rx->read_index >= UART_DMA_RX_BUFFER_SZIE)
			uart->rx_mode.dma_rx->read_index = 0;
	}
	
	rt_serial_enable_rx_dma(uart->rx_mode.dma_rx->dma_channel, (uint32_t)(uart->rx_mode.dma_rx->rx_dma_buffer + uart->rx_mode.dma_rx->save_index), UART_DMA_RX_SIZE);
	/* enable interrupt */
	rt_hw_interrupt_enable(level);
	
	/* invoke callback */
	if (device->rx_indicate != RT_NULL)
	{
		device->rx_indicate(device, UART_DMA_RX_SIZE);
	}
}
