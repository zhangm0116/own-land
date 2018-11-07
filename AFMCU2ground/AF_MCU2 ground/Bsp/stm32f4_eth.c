/*
 * STM32 Eth Driver for RT-Thread
 * Change Logs:
 * Date           Author       Notes
 * 2009-10-05     Bernard      eth interface driver for STM32F107 CL
 */
#include "rtthread.h"
#include "netif/ethernetif.h"
#include "lwipopts.h"
#include "stm32f4x7_eth.h"

#define MII_MODE
#define ETH_PHY_ADDRESS					0x05
#define netifGUARD_BLOCK_TIME		250
#define MAX_ADDR_LEN				6

/* Ethernet Rx & Tx DMA Descriptors */
extern ETH_DMADESCTypeDef  DMARxDscrTab[ETH_RXBUFNB], DMATxDscrTab[ETH_TXBUFNB];
/* Ethernet Receive buffers  */
extern uint8_t Rx_Buff[ETH_RXBUFNB][ETH_RX_BUF_SIZE];
/* Ethernet Transmit buffers */
extern uint8_t Tx_Buff[ETH_TXBUFNB][ETH_TX_BUF_SIZE];
/* Global pointers to track current transmit and receive descriptors */
extern ETH_DMADESCTypeDef  *DMATxDescToSet;
extern ETH_DMADESCTypeDef  *DMARxDescToGet;
/* Global pointer for last received frame infos */
extern ETH_DMA_Rx_Frame_infos *DMA_RX_FRAME_infos;

struct rt_stm32_eth
{
	/* inherit from ethernet device */
	struct eth_device parent;

	/* interface address info. */
	rt_uint8_t  dev_addr[MAX_ADDR_LEN];
};
static struct rt_stm32_eth stm32_eth_device;
static struct rt_semaphore tx_wait;


/* RT-Thread Device Interface */
/* initialize the interface */
static rt_err_t rt_stm32_eth_init(rt_device_t dev)
{
	int i;

	/* MAC address configuration */
	ETH_MACAddressConfig(ETH_MAC_Address0, (uint8_t*)&stm32_eth_device.dev_addr[0]);
	rt_kprintf("MAC Address:......%d.%d.%d.%d.%d.%d\r\n",stm32_eth_device.dev_addr[0],stm32_eth_device.dev_addr[1],stm32_eth_device.dev_addr[2],stm32_eth_device.dev_addr[3],stm32_eth_device.dev_addr[4],stm32_eth_device.dev_addr[5]);
	/* Initialize Tx Descriptors list: Chain Mode */
	ETH_DMATxDescChainInit(DMATxDscrTab, &Tx_Buff[0][0], ETH_TXBUFNB);
	/* Initialize Rx Descriptors list: Chain Mode  */
	ETH_DMARxDescChainInit(DMARxDscrTab, &Rx_Buff[0][0], ETH_RXBUFNB);

	/* Enable Ethernet Rx interrrupt */
	{
		for (i = 0; i < ETH_RXBUFNB; i++)
		{
			ETH_DMARxDescReceiveITConfig(&DMARxDscrTab[i], ENABLE);
		}
	}
	/* Enable MAC and DMA transmission and reception */
	ETH_Start();

	return RT_EOK;
}

static rt_err_t rt_stm32_eth_open(rt_device_t dev, rt_uint16_t oflag)
{
	return RT_EOK;
}

static rt_err_t rt_stm32_eth_close(rt_device_t dev)
{
	return RT_EOK;
}

static rt_size_t rt_stm32_eth_read(rt_device_t dev, rt_off_t pos, void* buffer, rt_size_t size)
{
	rt_set_errno(-RT_ENOSYS);
	return 0;
}

static rt_size_t rt_stm32_eth_write(rt_device_t dev, rt_off_t pos, const void* buffer, rt_size_t size)
{
	rt_set_errno(-RT_ENOSYS);
	return 0;
}

static rt_err_t rt_stm32_eth_control(rt_device_t dev, rt_uint8_t cmd, void *args)
{
	switch (cmd)
	{
	case NIOCTL_GADDR:
		/* get mac address */
		if (args) rt_memcpy(args, stm32_eth_device.dev_addr, 6);
		else return -RT_ERROR;
		break;

	default:
		break;
	}

	return RT_EOK;
}

/* transmit packet. */
rt_err_t rt_stm32_eth_tx(rt_device_t dev, struct pbuf* p)
{
	rt_err_t ret;
	struct pbuf *q;
	uint32_t l = 0;
	u8 *buffer;

	if ((ret = rt_sem_take(&tx_wait, netifGUARD_BLOCK_TIME)) == RT_EOK)
	{
		buffer = (u8 *)(DMATxDescToSet->Buffer1Addr);
		for (q = p; q != NULL; q = q->next)
		{
			rt_memcpy((u8_t*)&buffer[l], q->payload, q->len);
			l = l + q->len;
		}
		if (ETH_Prepare_Transmit_Descriptors(l) == ETH_ERROR)
		{
			rt_kprintf("Tx Error\n");
		}
		rt_sem_release(&tx_wait);
	}
	else
	{
		rt_kprintf("Tx Timeout\n");
		return ret;
	}

	return RT_EOK;
}

/* reception packet. */
struct pbuf *rt_stm32_eth_rx(rt_device_t dev)
{
	struct pbuf *p, *q;
	u16_t len;
	uint32_t l = 0, i = 0;
	FrameTypeDef frame;
	u8 *buffer;
	__IO ETH_DMADESCTypeDef *DMARxNextDesc;

	p = RT_NULL;

	/* Get received frame */
	frame = ETH_Get_Received_Frame_interrupt();

	if (frame.length > 0)
	{
		/* check that frame has no error */
		if ((frame.descriptor->Status & ETH_DMARxDesc_ES) == (uint32_t)RESET)
		{
			//rt_kprintf("Get a frame %d buf = 0x%X, len= %d\n", framecnt++, frame.buffer, frame.length);
			/* Obtain the size of the packet and put it into the "len" variable. */
			len = frame.length;
			buffer = (u8 *)frame.buffer;

			/* We allocate a pbuf chain of pbufs from the pool. */
			p = pbuf_alloc(PBUF_RAW, len, PBUF_POOL);

			/* Copy received frame from ethernet driver buffer to stack buffer */
			if (p != NULL)
			{
				for (q = p; q != NULL; q = q->next)
				{
					rt_memcpy((u8_t*)q->payload, (u8_t*)&buffer[l], q->len);
					l = l + q->len;
				}
			}
		}

		/* Release descriptors to DMA */
		/* Check if received frame with multiple DMA buffer segments */
		if (DMA_RX_FRAME_infos->Seg_Count > 1)
		{
			DMARxNextDesc = DMA_RX_FRAME_infos->FS_Rx_Desc;
		}
		else
		{
			DMARxNextDesc = frame.descriptor;
		}

		/* Set Own bit in Rx descriptors: gives the buffers back to DMA */
		for (i = 0; i < DMA_RX_FRAME_infos->Seg_Count; i++)
		{
			DMARxNextDesc->Status = ETH_DMARxDesc_OWN;
			DMARxNextDesc = (ETH_DMADESCTypeDef *)(DMARxNextDesc->Buffer2NextDescAddr);
		}

		/* Clear Segment_Count */
		DMA_RX_FRAME_infos->Seg_Count = 0;


		/* When Rx Buffer unavailable flag is set: clear it and resume reception */
		if ((ETH->DMASR & ETH_DMASR_RBUS) != (u32)RESET)
		{
			/* Clear RBUS ETHERNET DMA flag */
			ETH->DMASR = ETH_DMASR_RBUS;

			/* Resume DMA reception */
			ETH->DMARPDR = 0;
		}
	}
	return p;
}

static void GPIO_Configuration(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	/* Enable GPIOs clocks */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA
							| RCC_AHB1Periph_GPIOB
							| RCC_AHB1Periph_GPIOC
							| RCC_AHB1Periph_GPIOE, ENABLE);

	/* Enable SYSCFG clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

#ifdef MII_MODE
	SYSCFG_ETH_MediaInterfaceConfig(SYSCFG_ETH_MediaInterface_MII);
#elif defined RMII_MODE
	SYSCFG_ETH_MediaInterfaceConfig(SYSCFG_ETH_MediaInterface_RMII);
#endif

/************************************************************************
*	ETH_MDIO -------------------------> PA2
*	ETH_MDC --------------------------> PC1
*	ETH_MII_RX_CLK/ETH_RMII_REF_CLK---> PA1
*	ETH_MII_RX_DV/ETH_RMII_CRS_DV ----> PA7
*	ETH_MII_RXD0/ETH_RMII_RXD0 -------> PC4
*	ETH_MII_RXD1/ETH_RMII_RXD1 -------> PC5
*	ETH_MII_TX_EN/ETH_RMII_TX_EN -----> PB11
*	ETH_MII_TXD0/ETH_RMII_TXD0 -------> PB12
*	ETH_MII_TXD1/ETH_RMII_TXD1 -------> PB13

*	**** Just for MII Mode ****
*	ETH_MII_CRS ----------------------> PA0
*	ETH_MII_COL ----------------------> PA3
*	ETH_MII_TX_CLK -------------------> PC3
*	ETH_MII_RX_ER --------------------> PB10
*	ETH_MII_RXD2 ---------------------> PB0
*	ETH_MII_RXD3 ---------------------> PB1
*	ETH_MII_TXD2 ---------------------> PC2
*	ETH_MII_TXD3 ---------------------> PE2
***************************************************************************/
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	
	/* Configure PC1, PC4 and PC5 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_4 | GPIO_Pin_5;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource1, GPIO_AF_ETH);
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource4, GPIO_AF_ETH);
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource5, GPIO_AF_ETH);

	/* Configure PB11, PB12 and PB13 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource11, GPIO_AF_ETH);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource12, GPIO_AF_ETH);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource13, GPIO_AF_ETH);

	/* Configure PA1, PA2 and PA7 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_7;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource1, GPIO_AF_ETH);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_ETH);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource7, GPIO_AF_ETH);

#ifdef MII_MODE
	/* Configure PC2, PC3 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource2, GPIO_AF_ETH);
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource3, GPIO_AF_ETH);

	/* Configure PB0, PB1, PB10 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_10;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource0, GPIO_AF_ETH);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource1, GPIO_AF_ETH);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource10, GPIO_AF_ETH);

	/* Configure PA0, PA3 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_3;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource0, GPIO_AF_ETH);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_ETH);
	/* Configure PE2 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_Init(GPIOE, &GPIO_InitStructure);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource2, GPIO_AF_ETH);
#endif
}

static void NVIC_Configuration(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;

	/* Enable the Ethernet global Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = ETH_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

static void ETH_MACDMA_Config(void)
{
	ETH_InitTypeDef ETH_InitStructure;
	uint16_t  bcr;
	
	
	/* Enable ETHERNET clock  */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_ETH_MAC | RCC_AHB1Periph_ETH_MAC_Tx | RCC_AHB1Periph_ETH_MAC_Rx, ENABLE);

	/* Reset ETHERNET on AHB Bus */
	ETH_DeInit();

	/* Software reset */
	ETH_SoftwareReset();

	/* Wait for software reset */
	while (ETH_GetSoftwareResetStatus() == SET);

	ETH_StructInit(&ETH_InitStructure);

	ETH_InitStructure.ETH_AutoNegotiation = ETH_AutoNegotiation_Disable;
	ETH_InitStructure.ETH_Watchdog = ETH_Watchdog_Enable;
	ETH_InitStructure.ETH_Jabber = ETH_Jabber_Enable;
	ETH_InitStructure.ETH_InterFrameGap = ETH_InterFrameGap_96Bit;
	ETH_InitStructure.ETH_CarrierSense = ETH_CarrierSense_Enable;
	ETH_InitStructure.ETH_Speed = ETH_Speed_100M;
	ETH_InitStructure.ETH_ReceiveOwn = ETH_ReceiveOwn_Enable;
	ETH_InitStructure.ETH_LoopbackMode = ETH_LoopbackMode_Disable;
	ETH_InitStructure.ETH_Mode = ETH_Mode_FullDuplex;
	ETH_InitStructure.ETH_ChecksumOffload = ETH_ChecksumOffload_Disable;
	ETH_InitStructure.ETH_RetryTransmission = ETH_RetryTransmission_Disable;
	ETH_InitStructure.ETH_AutomaticPadCRCStrip = ETH_AutomaticPadCRCStrip_Disable;
	ETH_InitStructure.ETH_BackOffLimit = ETH_BackOffLimit_10;
	ETH_InitStructure.ETH_DeferralCheck = ETH_DeferralCheck_Disable;
	ETH_InitStructure.ETH_ReceiveAll = ETH_ReceiveAll_Disable;
	ETH_InitStructure.ETH_SourceAddrFilter = ETH_SourceAddrFilter_Disable;
	ETH_InitStructure.ETH_PassControlFrames = ETH_PassControlFrames_BlockAll;
	ETH_InitStructure.ETH_BroadcastFramesReception = ETH_BroadcastFramesReception_Enable;
	ETH_InitStructure.ETH_PromiscuousMode = ETH_PromiscuousMode_Disable;
	ETH_InitStructure.ETH_MulticastFramesFilter = ETH_MulticastFramesFilter_Perfect;
	ETH_InitStructure.ETH_UnicastFramesFilter = ETH_UnicastFramesFilter_Perfect;
	ETH_InitStructure.ETH_HashTableHigh = 0x0;
	ETH_InitStructure.ETH_HashTableLow = 0x0;
	ETH_InitStructure.ETH_PauseTime = 0x0;
	ETH_InitStructure.ETH_ZeroQuantaPause = ETH_ZeroQuantaPause_Disable;
	ETH_InitStructure.ETH_PauseLowThreshold = ETH_PauseLowThreshold_Minus4;
	ETH_InitStructure.ETH_UnicastPauseFrameDetect = ETH_UnicastPauseFrameDetect_Disable;
	ETH_InitStructure.ETH_ReceiveFlowControl = ETH_ReceiveFlowControl_Disable;
	ETH_InitStructure.ETH_TransmitFlowControl = ETH_TransmitFlowControl_Disable;
	ETH_InitStructure.ETH_VLANTagComparison = ETH_VLANTagComparison_16Bit;
	ETH_InitStructure.ETH_VLANTagIdentifier = 0x0;
	
	ETH_InitStructure.ETH_DropTCPIPChecksumErrorFrame = ETH_DropTCPIPChecksumErrorFrame_Enable;
	ETH_InitStructure.ETH_ReceiveStoreForward = ETH_ReceiveStoreForward_Enable;
	ETH_InitStructure.ETH_FlushReceivedFrame = ETH_FlushReceivedFrame_Enable;
	ETH_InitStructure.ETH_TransmitStoreForward = ETH_TransmitStoreForward_Enable;
	ETH_InitStructure.ETH_TransmitThresholdControl = ETH_TransmitThresholdControl_64Bytes;
	ETH_InitStructure.ETH_ForwardErrorFrames = ETH_ForwardErrorFrames_Disable;
	ETH_InitStructure.ETH_ForwardUndersizedGoodFrames = ETH_ForwardUndersizedGoodFrames_Disable;
	ETH_InitStructure.ETH_ReceiveThresholdControl = ETH_ReceiveThresholdControl_64Bytes;
	ETH_InitStructure.ETH_SecondFrameOperate = ETH_SecondFrameOperate_Enable;
	ETH_InitStructure.ETH_AddressAlignedBeats = ETH_AddressAlignedBeats_Enable;
	ETH_InitStructure.ETH_FixedBurst = ETH_FixedBurst_Enable;
	ETH_InitStructure.ETH_RxDMABurstLength = ETH_RxDMABurstLength_32Beat;
	ETH_InitStructure.ETH_TxDMABurstLength = ETH_TxDMABurstLength_32Beat;
	ETH_InitStructure.ETH_DescriptorSkipLength = 0x0;
	ETH_InitStructure.ETH_DMAArbitration = ETH_DMAArbitration_RoundRobin_RxTx_2_1;

	if (ETH_Init(&ETH_InitStructure, ETH_PHY_ADDRESS) == ETH_ERROR)
	{
		rt_kprintf("ETH init error, may be no link\n");
	}
	
	ETH_WritePHYRegister(0x05, PHY_BCR,0x2100);//100M duplex
	
	bcr = ETH_ReadPHYRegister(0x05, PHY_BCR);
	rt_kprintf("0x5 BCR :0x%x\n",bcr);
	/* Enable the Ethernet Rx Interrupt */
	ETH_DMAITConfig(ETH_DMA_IT_NIS | ETH_DMA_IT_R, ENABLE);
}


void rt_hw_stm32_eth_init(void)
{
	u32_t  sn0=0;
	GPIO_Configuration();
	NVIC_Configuration();
	ETH_MACDMA_Config();

	sn0=*(vu32 *)(0x1FFF7A10);
	stm32_eth_device.dev_addr[0] = 0x00;
	stm32_eth_device.dev_addr[1] = 0x0e;
	stm32_eth_device.dev_addr[2] = 0xc6;
	stm32_eth_device.dev_addr[3]=(sn0>>16)&0XFF;   
	stm32_eth_device.dev_addr[4]=(sn0>>8)&0XFF;
	stm32_eth_device.dev_addr[5]=sn0&0XFF;
	
	
//	stm32_eth_device.dev_addr[3] = 0xc3;
//	stm32_eth_device.dev_addr[4] = 0x1a;
//	stm32_eth_device.dev_addr[5] = 0x88;

	stm32_eth_device.parent.parent.init = rt_stm32_eth_init;
	stm32_eth_device.parent.parent.open = rt_stm32_eth_open;
	stm32_eth_device.parent.parent.close = rt_stm32_eth_close;
	stm32_eth_device.parent.parent.read = rt_stm32_eth_read;
	stm32_eth_device.parent.parent.write = rt_stm32_eth_write;
	stm32_eth_device.parent.parent.control = rt_stm32_eth_control;
	stm32_eth_device.parent.parent.user_data = RT_NULL;

	stm32_eth_device.parent.eth_rx = rt_stm32_eth_rx;
	stm32_eth_device.parent.eth_tx = rt_stm32_eth_tx;

	/* init tx semaphore */
	rt_sem_init(&tx_wait, "tx_wait", 1, RT_IPC_FLAG_FIFO);

	/* register eth device */
	eth_device_init(&(stm32_eth_device.parent), "eth0");
	
}

/* interrupt service routine */
void ETH_IRQHandler(void)
{
	/* Frame received */
	if (ETH_GetDMAFlagStatus(ETH_DMA_FLAG_R) == SET)
	{
		rt_err_t result;

		ETH_DMAClearITPendingBit(ETH_DMA_IT_R);

		/* a frame has been received */
		result = eth_device_ready(&(stm32_eth_device.parent));

		if (result != RT_EOK)
		{
			rt_kprintf("RX err =%d\n", result);
		}
	}
	if (ETH_GetDMAITStatus(ETH_DMA_IT_T) == SET) /* packet transmission */
	{
		ETH_DMAClearITPendingBit(ETH_DMA_IT_T);
	}

	ETH_DMAClearITPendingBit(ETH_DMA_IT_NIS);
}
