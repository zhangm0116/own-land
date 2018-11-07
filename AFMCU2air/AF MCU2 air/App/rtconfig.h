/* RT-Thread config file */
#ifndef __RTTHREAD_CFG_H__
#define __RTTHREAD_CFG_H__

#define AIR_MODULE

#define RT_LWIP_IGMP  

/* RT_NAME_MAX*/
#define RT_NAME_MAX	   32

/* RT_ALIGN_SIZE*/
#define RT_ALIGN_SIZE	8

/* PRIORITY_MAX */
#define RT_THREAD_PRIORITY_MAX	32

/* Tick per Second */
#define RT_TICK_PER_SECOND	1000

#define RT_DEBUG

/* Using Software Timer */
/* #define RT_USING_TIMER_SOFT */
#define RT_TIMER_THREAD_PRIO		4
#define RT_TIMER_THREAD_STACK_SIZE	512
#define RT_TIMER_TICK_PER_SECOND	10



/* SECTION: IPC */
/* Using Semaphore*/
#define RT_USING_SEMAPHORE

/* Using Mutex */
#define RT_USING_MUTEX

/* Using Event */
#define RT_USING_EVENT

/* Using MailBox */
#define RT_USING_MAILBOX

/* Using Message Queue */
#define RT_USING_MESSAGEQUEUE

/* SECTION: Memory Management */
/* Using Memory Pool Management*/
#define RT_USING_MEMPOOL

/* Using Dynamic Heap Management */
#define RT_USING_HEAP

/* Using Small MM */
#define RT_USING_SMALL_MEM

#define RT_USING_MINILIBC

/* SECTION: Device System */
/* Using Device System */
#define RT_USING_DEVICE

#define RT_USING_UART1
#define RT_UART1_INT_RX
#define RT_UART1_POLLING_TX

#define RT_USING_UART2
#define RT_UART2_INT_RX
#define RT_UART2_POLLING_TX

#define RT_USING_SPI3
//#define RT_SPI3_DMA

#define RT_USING_I2C

/* SECTION: Console options */
#define RT_USING_CONSOLE
/* the buffer size of console*/
#define RT_CONSOLEBUF_SIZE	128

/* SECTION: finsh, a C-Express shell */
#define RT_USING_FINSH
#define FINSH_USING_MSH
#define FINSH_USING_MSH_ONLY

/* Using symbol table */
#define FINSH_USING_SYMTAB
#define FINSH_USING_DESCRIPTION

/* SECTION: lwip, a lighwight TCP/IP protocol stack */
#define RT_USING_LWIP
/* LwIP uses RT-Thread Memory Management */
#define RT_LWIP_USING_RT_MEM
/* Enable ICMP protocol*/
#define RT_LWIP_ICMP            //组播协议
/* Enable UDP protocol*/
#define RT_LWIP_UDP
/* Enable TCP protocol*/
#define RT_LWIP_TCP

/* the number of simulatenously active TCP connections*/
#define RT_LWIP_TCP_PCB_NUM	5

/* ip address of target*/
#define RT_LWIP_IPADDR0	192
#define RT_LWIP_IPADDR1	168
#define RT_LWIP_IPADDR2	1
#define RT_LWIP_IPADDR3 50 

/* gateway address of target*/
#define RT_LWIP_GWADDR0	192
#define RT_LWIP_GWADDR1	168
#define RT_LWIP_GWADDR2	1 
#define RT_LWIP_GWADDR3	1

/* mask address of target*/
#define RT_LWIP_MSKADDR0	255
#define RT_LWIP_MSKADDR1	255
#define RT_LWIP_MSKADDR2	255
#define RT_LWIP_MSKADDR3	0


#define  UDP_FC_PORT  14558 
#define  UDP_TRUCK_PORT  14000


//远端客户端地址   
#define REMOTE_IPADDR0	192
#define REMOTE_IPADDR1	168
#define REMOTE_IPADDR2	1
#define REMOTE_IPADDR3	121

//远端客户端端口号   

#define  UDP_REMOTE_PORT  8085

//远端客户端地址  
#define REMOTE_IPADDR4	192
#define REMOTE_IPADDR5	168
#define REMOTE_IPADDR6	1
#define REMOTE_IPADDR7	221

#define  UDP_REMOTE_PORT2  8090  


/* tcp thread options */
#define RT_LWIP_TCPTHREAD_PRIORITY		12
#define RT_LWIP_TCPTHREAD_MBOX_SIZE		32
#define RT_LWIP_TCPTHREAD_STACKSIZE		3072

/* ethernet if thread options */
#define RT_LWIP_ETHTHREAD_PRIORITY		10
#define RT_LWIP_ETHTHREAD_MBOX_SIZE		32
#define RT_LWIP_ETHTHREAD_STACKSIZE		2048

/* TCP sender buffer space */
#define RT_LWIP_TCP_SND_BUF	8192
/* TCP receive window. */
#define RT_LWIP_TCP_WND		8192

//#define RT_LWIP_DEBUG
#endif
