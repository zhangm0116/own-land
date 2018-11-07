/*
 * File      : queue.c
 * COPYRIGHT (C) 2017, TaiSync Development Team
 *
 * Change Logs:
 * Date           Author       Notes
 * 2017-12-28     hgz      	   the first version
 * …
*/
#include "queue.h"
#include "rtthread.h"

static unsigned char value[QUEUE_SIZE+1];

void InitQueue(Queue_uart *q)
{
    q->queuesize = QUEUE_SIZE;
    q->val  = value;
    q->tail = 0;
    q->head = 0;
}

int EnQueue(Queue_uart *q, unsigned char val)
{

    if ((q->tail+1)%(q->queuesize) == q->head)
    {
        rt_kprintf("queue full!\n");
        return -1;
    }
    else
    {
        q->val[q->tail]  = val;
        q->tail = (q->tail+1)%(q->queuesize);
        return 0;
			
    }
}

int DeQueue(Queue_uart *q, unsigned char *val)
{
    if(q->tail == q->head)
    {
        rt_kprintf("the queue is NULL\n");
        return -1;
    }
    else
    {
        *val  = q->val[q->head];
        q->head = (q->head+1) % q->queuesize;
        return 0;
    }
}

int IsQueueEmpty(Queue_uart *q)
{
    if(q->head == q->tail)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

int IsQueueFull(Queue_uart *q)
{
    if((q->tail+1)% q->queuesize == q->head)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}
