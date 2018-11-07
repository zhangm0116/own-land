/*
 * File      : ethernetif.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006 - 2010, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2010-07-07     Bernard      fix send mail to mailbox issue.
 * 2011-07-30     mbbill       port lwIP 1.4.0 to RT-Thread
 * 2012-04-10     Bernard      add more compatible with RT-Thread.
 * 2012-11-12     Bernard      The network interface can be initialized
 *                             after lwIP initialization.
 * 2013-02-28     aozima       fixed list_tcps bug: ipaddr_ntoa isn't reentrant.
 */

/*
 * Copyright (c) 2001-2004 Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 *
 * Author: Adam Dunkels <adam@sics.se>
 *
 */

#include "rtthread.h"

#include "lwip/opt.h"
#include "lwip/debug.h"
#include "lwip/def.h"
#include "lwip/mem.h"
#include "lwip/pbuf.h"
#include "lwip/sys.h"
#include "lwip/netif.h"
#include "lwip/stats.h"
#include "lwip/tcpip.h"

#include "netif/etharp.h"
#include "netif/ethernetif.h"
#include "stm32f4x7_eth.h"

#define netifapi_netif_set_link_up(n)      netifapi_netif_common(n, netif_set_link_up, NULL)
#define netifapi_netif_set_link_down(n)    netifapi_netif_common(n, netif_set_link_down, NULL)

/**
 * Tx message structure for Ethernet interface
 */
 
 //unsigned int tcp_rcvbuf[100];
 
 
 ///test/////
#define IFNAME0 's'   
#define IFNAME1 't'  

#define BOARD_MAC_ADDR       0,0,0,0,0,1			//开发板MAC地址
 
 /* Ethernet Rx & Tx DMA Descriptors */
extern ETH_DMADESCTypeDef  DMARxDscrTab[ETH_RXBUFNB], DMATxDscrTab[ETH_TXBUFNB];

/* Ethernet Driver Receive buffers  */
extern uint8_t Rx_Buff[ETH_RXBUFNB][ETH_RX_BUF_SIZE]; 

/* Ethernet Driver Transmit buffers */
extern uint8_t Tx_Buff[ETH_TXBUFNB][ETH_TX_BUF_SIZE]; 

/* Global pointers to track current transmit and receive descriptors */
extern ETH_DMADESCTypeDef  *DMATxDescToSet;
extern ETH_DMADESCTypeDef  *DMARxDescToGet;

/* Global pointer for last received frame infos */
extern ETH_DMA_Rx_Frame_infos *DMA_RX_FRAME_infos;

 
 
 ////////////////////////////
struct eth_tx_msg
{
    struct netif 	*netif;
    struct pbuf 	*buf;
};


static struct rt_mailbox eth_tx_thread_mb;
static struct rt_thread eth_tx_thread;
#ifndef RT_LWIP_ETHTHREAD_PRIORITY
static char eth_tx_thread_mb_pool[32 * 4];
static char eth_tx_thread_stack[512];
#else
static char eth_tx_thread_mb_pool[RT_LWIP_ETHTHREAD_MBOX_SIZE * 4];
static char eth_tx_thread_stack[RT_LWIP_ETHTHREAD_STACKSIZE];
#endif

static struct rt_mailbox eth_rx_thread_mb;
static struct rt_thread eth_rx_thread;
#ifndef RT_LWIP_ETHTHREAD_PRIORITY
#define RT_ETHERNETIF_THREAD_PREORITY	0x90
static char eth_rx_thread_mb_pool[48 * 4];
static char eth_rx_thread_stack[1024];
#else  
#define RT_ETHERNETIF_THREAD_PREORITY	RT_LWIP_ETHTHREAD_PRIORITY
static char eth_rx_thread_mb_pool[RT_LWIP_ETHTHREAD_MBOX_SIZE * 4];
static char eth_rx_thread_stack[RT_LWIP_ETHTHREAD_STACKSIZE];
#endif


u8  udp_buf[100];

static err_t   ethernetif_linkoutput(struct netif *netif, struct pbuf *p)
{  
	
    struct eth_tx_msg msg;
    struct eth_device* enetif;

    enetif = (struct eth_device*)netif->state;

    /* send a message to eth tx thread */
    msg.netif = netif;
    msg.buf   = p;
    if (rt_mb_send(&eth_tx_thread_mb, (rt_uint32_t) &msg) == RT_EOK)   
    {
        /* waiting for ack */
        rt_sem_take(&(enetif->tx_ack), RT_WAITING_FOREVER);
    }

    return ERR_OK;
		
		
		
}

static err_t eth_netif_device_init(struct netif *netif)
{
    struct eth_device *ethif;

    ethif = (struct eth_device*)netif->state;
    if (ethif != RT_NULL)
    {
        rt_device_t device;

        /* get device object */
        device = (rt_device_t) ethif;
        if (rt_device_init(device) != RT_EOK)
        {
            return ERR_IF;
        }

        /* copy device flags to netifflags */
        netif->flags = ethif->flags;

        /* set default netif */
        if (netif_default == RT_NULL)
            netif_set_default(ethif->netif);

#if LWIP_DHCP
        if (ethif->flags & NETIF_FLAG_DHCP)
        {
            /* if this interface uses DHCP, start the DHCP client */
            dhcp_start(ethif->netif);
        }
        else
#endif
        {
            /* set interface up */
            netif_set_up(ethif->netif);
        }

#ifdef LWIP_NETIF_LINK_CALLBACK
        netif_set_link_up(ethif->netif);
#endif

        return ERR_OK;
    }

    return ERR_IF;
}


/* Keep old drivers compatible in RT-Thread */
rt_err_t eth_device_init_with_flag(struct eth_device *dev, char *name, rt_uint8_t flags)
{
    struct netif* netif;

    netif = (struct netif*) rt_malloc (sizeof(struct netif));
    if (netif == RT_NULL)
    {
        rt_kprintf("malloc netif failed\n");
        return -RT_ERROR;
    }
    rt_memset(netif, 0, sizeof(struct netif));

    /* set netif */
    dev->netif = netif;
    /* device flags, which will be set to netif flags when initializing */
    dev->flags = flags;
    /* link changed status of device */
    dev->link_changed = 0x00;    //初始状态  连接 
    dev->parent.type = RT_Device_Class_NetIf;
    /* register to RT-Thread device manager */
    rt_device_register(&(dev->parent), name, RT_DEVICE_FLAG_RDWR);
    rt_sem_init(&(dev->tx_ack), name, 0, RT_IPC_FLAG_FIFO);

    /* set name */
    netif->name[0] = name[0];
    netif->name[1] = name[1];

    /* set hw address to 6 */
    netif->hwaddr_len 	= 6;
    /* maximum transfer unit */
    netif->mtu	= ETHERNET_MTU;

    /* get hardware MAC address */
    rt_device_control(&(dev->parent), NIOCTL_GADDR, netif->hwaddr);

    /* set output */
    netif->output = etharp_output;
    netif->linkoutput	= ethernetif_linkoutput;

    /* if tcp thread has been started up, we add this netif to the system */
    if (rt_thread_find("tcpip")!= RT_NULL)
    {   
        struct ip_addr ipaddr, netmask, gw;
       
#if !LWIP_DHCP
			
        IP4_ADDR(&ipaddr, RT_LWIP_IPADDR0, RT_LWIP_IPADDR1, RT_LWIP_IPADDR2, RT_LWIP_IPADDR3);
        IP4_ADDR(&gw, RT_LWIP_GWADDR0, RT_LWIP_GWADDR1, RT_LWIP_GWADDR2, RT_LWIP_GWADDR3);
        IP4_ADDR(&netmask, RT_LWIP_MSKADDR0, RT_LWIP_MSKADDR1, RT_LWIP_MSKADDR2, RT_LWIP_MSKADDR3);
#else
        IP4_ADDR(&ipaddr, 0, 0, 0, 0);
        IP4_ADDR(&gw, 0, 0, 0, 0);        
        IP4_ADDR(&netmask, 0, 0, 0, 0);
#endif

        netifapi_netif_add(netif, &ipaddr, &netmask, &gw, dev, eth_netif_device_init, tcpip_input);
    }

    return RT_EOK;
}


rt_err_t eth_device_init(struct eth_device * dev, char *name)
{
    rt_uint8_t flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP;

#if LWIP_DHCP
    /* DHCP support */
    flags |= NETIF_FLAG_DHCP;
#endif

#if LWIP_IGMP
    /* IGMP support */
    flags |= NETIF_FLAG_IGMP;
#endif
    
	
    return eth_device_init_with_flag(dev, name, flags);
}

rt_err_t eth_device_ready(struct eth_device* dev)
{
    if (dev->netif)
		{ /* post message to Ethernet thread */
		       // dev->link_changed=1;      //test 
			
        return rt_mb_send(&eth_rx_thread_mb, (rt_uint32_t)dev);}
    else
        return ERR_OK; /* netif is not initialized yet, just return. */
		
}

rt_err_t eth_device_linkchange(struct eth_device* dev, rt_bool_t up)
{
    rt_uint32_t level;

    RT_ASSERT(dev != RT_NULL);

    level = rt_hw_interrupt_disable();
	
    dev->link_changed = 0x01;
    if (up == RT_TRUE)
			
        dev->link_status = 0x01;
    else
        dev->link_status = 0x00;
    rt_hw_interrupt_enable(level);

    /* post message to ethernet thread */
    return rt_mb_send(&eth_rx_thread_mb, (rt_uint32_t)dev); 
		
		
}

/* Ethernet Tx Thread */
static void eth_tx_thread_entry(void* parameter)
{
    struct eth_tx_msg* msg;

    while (1)
    {
        if (rt_mb_recv(&eth_tx_thread_mb, (rt_uint32_t*)&msg, RT_WAITING_FOREVER) == RT_EOK)
        {
            struct eth_device* enetif;
             
            RT_ASSERT(msg->netif != RT_NULL);
            RT_ASSERT(msg->buf   != RT_NULL);
           // rt_kprintf("aa");
					
					
            enetif = (struct eth_device*)msg->netif->state;//将接收到的状态传给
            if (enetif != RT_NULL)
            {   
                /* call driver's interface */
                if (enetif->eth_tx(&(enetif->parent), msg->buf) != RT_EOK)//调用eth_tx发送函数发送数据包
                {
                    rt_kprintf("transmit eth packet failed\n");
									
									
									
                }
            }

            /* send ACK */
            rt_sem_release(&(enetif->tx_ack));
        }
    }
}


/* Ethernet Rx Thread */  
static void eth_rx_thread_entry(void* parameter)
{
    struct eth_device* device;
      u32_t i=0;
    while (1)
    {     //  
        if (rt_mb_recv(&eth_rx_thread_mb, (rt_uint32_t*)&device, RT_WAITING_FOREVER) == RT_EOK)     //把device 的
        {
            struct pbuf   *p;
             
            /* check link status */
            if (device->link_changed)
            {  
							  
							 // rt_kprintf("link status");  //test
							
							  int status;
                rt_uint32_t level;

                level = rt_hw_interrupt_disable();
							
                status = device->link_status;
                device->link_changed = 0x00;
							
                rt_hw_interrupt_enable(level);

                if (status)
                    netifapi_netif_set_link_up(device->netif);//
                else
                    netifapi_netif_set_link_down(device->netif);
            }

            /* receive all of buffer */
            while (1)
            {
							
							  
                p = device->eth_rx(&(device->parent));        //  调用 （rt_stm32_eth_rx）
                if (p != RT_NULL)
                {   			  
                   
//                           memcpy(udp_buf,p->payload,p->len);
//										   	   for(i=0;i<p->len;i++)     rt_kprintf("%x ",udp_buf[i]);
//											      rt_kprintf("\n");      
//                  


									/* notify to upper layer */
                    if( device->netif->input(p, device->netif) != ERR_OK )   //调用input函数从网卡接收一包数据
                    {    
                        LWIP_DEBUGF(NETIF_DEBUG, ("ethernetif_input: Input error\n"));
                        pbuf_free(p);
                        p = NULL;
											
                    }
										else {
											  
											       //   rt_kprintf("link status");
										      
								   }
						
                }
                else break;
            }
        }
        else
        {
            LWIP_ASSERT("Should not happen!\n",0);
        }
    }
}


int eth_system_device_init(void)
{
    rt_err_t result = RT_EOK;

    /* initialize Rx thread.
     * initialize mailbox and create Ethernet Rx thread */
    result = rt_mb_init(&eth_rx_thread_mb, "erxmb",
                        &eth_rx_thread_mb_pool[0], sizeof(eth_rx_thread_mb_pool)/4,
                        RT_IPC_FLAG_FIFO);
    RT_ASSERT(result == RT_EOK);

    result = rt_thread_init(&eth_rx_thread, "erx", eth_rx_thread_entry, RT_NULL,
                            &eth_rx_thread_stack[0], sizeof(eth_rx_thread_stack),
                            RT_LWIP_ETHTHREAD_PRIORITY, 16);
    RT_ASSERT(result == RT_EOK);
														
    result = rt_thread_startup(&eth_rx_thread);
    RT_ASSERT(result == RT_EOK);

    /* initialize Tx thread */
    /* initialize mailbox and create Ethernet Tx thread */
    result = rt_mb_init(&eth_tx_thread_mb, "etxmb",
                        &eth_tx_thread_mb_pool[0], sizeof(eth_tx_thread_mb_pool)/4,
                        RT_IPC_FLAG_FIFO);
    RT_ASSERT(result == RT_EOK);

    result = rt_thread_init(&eth_tx_thread, "etx", eth_tx_thread_entry, RT_NULL,
                            &eth_tx_thread_stack[0], sizeof(eth_tx_thread_stack),
                            RT_ETHERNETIF_THREAD_PREORITY, 16);
    RT_ASSERT(result == RT_EOK);

    result = rt_thread_startup(&eth_tx_thread);
    RT_ASSERT(result == RT_EOK);
	
	return 0;
}
INIT_DEVICE_EXPORT(eth_system_device_init);





void list_if(void)
{
    rt_ubase_t index;
    struct netif * netif;

    rt_enter_critical();

    netif = netif_list;

    while( netif != RT_NULL )
    {
        rt_kprintf("network interface: %c%c%s\n",
                   netif->name[0],
                   netif->name[1],
                   (netif == netif_default)?" (Default)":"");
        rt_kprintf("MTU: %d\n", netif->mtu);
        rt_kprintf("MAC: ");
        for (index = 0; index < netif->hwaddr_len; index ++)
            rt_kprintf("%02x ", netif->hwaddr[index]);
        rt_kprintf("\nFLAGS:");
        if (netif->flags & NETIF_FLAG_UP) rt_kprintf(" UP");
        else rt_kprintf(" DOWN");
        if (netif->flags & NETIF_FLAG_LINK_UP) rt_kprintf(" LINK_UP");
        else rt_kprintf(" LINK_DOWN");
        if (netif->flags & NETIF_FLAG_DHCP) rt_kprintf(" DHCP");
        if (netif->flags & NETIF_FLAG_POINTTOPOINT) rt_kprintf(" PPP");
        if (netif->flags & NETIF_FLAG_ETHARP) rt_kprintf(" ETHARP");
        if (netif->flags & NETIF_FLAG_IGMP) rt_kprintf(" IGMP");
        rt_kprintf("\n");
        rt_kprintf("ip address: %s\n", ipaddr_ntoa(&(netif->ip_addr)));
        rt_kprintf("gw address: %s\n", ipaddr_ntoa(&(netif->gw)));
        rt_kprintf("net mask  : %s\n", ipaddr_ntoa(&(netif->netmask)));
        rt_kprintf("\r\n");
         
        netif = netif->next;
    }

#if LWIP_DNS
    {
        struct ip_addr ip_addr;

        for(index=0; index<DNS_MAX_SERVERS; index++)
        {
            ip_addr = dns_getserver(index);
            rt_kprintf("dns server #%d: %s\n", index, ipaddr_ntoa(&(ip_addr)));
        }
    }
#endif /**< #if LWIP_DNS */

    rt_exit_critical();
}








static err_t low_level_output(struct netif *netif, struct pbuf *p)
{
  struct pbuf *q;
  int framelength =0;
	
  u8_t *buffer =  (u8_t *)(DMATxDescToSet->Buffer1Addr);
  
  /* copy frame from pbufs to driver buffers */
  for(q = p; q != NULL; q = q->next) 
  {
    memcpy((u8_t*)&buffer[framelength], q->payload, q->len);
	  framelength = framelength + q->len;
  }
  
  /* Note: padding and CRC for transmitted frame 
     are automatically inserted by DMA */
      
  /* Prepare transmit descriptors to give to DMA*/ 
  ETH_Prepare_Transmit_Descriptors(framelength);

  return ERR_OK;
	
	
}
 

static void low_level_init(struct netif *netif)
{
#ifdef CHECKSUM_BY_HARDWARE
  int i; 
#endif
  uint8_t macaddress[6]={BOARD_MAC_ADDR};	
  /* set MAC hardware address length */
  netif->hwaddr_len = ETHARP_HWADDR_LEN;

  /* set MAC hardware address */
  netif->hwaddr[0] =  macaddress[0];
  netif->hwaddr[1] =  macaddress[1];
  netif->hwaddr[2] =  macaddress[2];
  netif->hwaddr[3] =  macaddress[3];
  netif->hwaddr[4] =  macaddress[4];
  netif->hwaddr[5] =  macaddress[5];
  
  /* initialize MAC address in ethernet MAC */ 
  ETH_MACAddressConfig(ETH_MAC_Address0, netif->hwaddr); 

  /* maximum transfer unit */
  netif->mtu = 1500;

  /* device capabilities */
  /* don't set NETIF_FLAG_ETHARP if this device is not an ethernet one */
  netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_LINK_UP;

  /* Initialize Tx Descriptors list: Chain Mode */
  ETH_DMATxDescChainInit(DMATxDscrTab, &Tx_Buff[0][0], ETH_TXBUFNB);
  /* Initialize Rx Descriptors list: Chain Mode  */
  ETH_DMARxDescChainInit(DMARxDscrTab, &Rx_Buff[0][0], ETH_RXBUFNB);
  
#ifdef CHECKSUM_BY_HARDWARE
  /* Enable the TCP, UDP and ICMP checksum insertion for the Tx frames */
  for(i=0; i<ETH_TXBUFNB; i++)
    {
      ETH_DMATxDescChecksumInsertionConfig(&DMATxDscrTab[i], ETH_DMATxDesc_ChecksumTCPUDPICMPFull);
    }
#endif

   /* Note: TCP, UDP, ICMP checksum checking for received frame are enabled in DMA config */

  /* Enable MAC and DMA transmission and reception */
  ETH_Start();

}

err_t ethernetif_init(struct netif *netif)
{
  LWIP_ASSERT("netif != NULL", (netif != NULL));
  
#if LWIP_NETIF_HOSTNAME
  /* Initialize interface hostname */
  netif->hostname = "lwip";
#endif /* LWIP_NETIF_HOSTNAME */

  netif->name[0] = IFNAME0;
  netif->name[1] = IFNAME1;
  /* We directly use etharp_output() here to save a function call.
   * You can instead declare your own function an call etharp_output()
   * from it if you have to do some checks before sending (e.g. if link
   * is available...) */
  netif->output = etharp_output;
  netif->linkoutput = low_level_output;

  /* initialize the hardware */
  low_level_init(netif);
	

  return ERR_OK;
}
