#ifndef __QUEUE_H__
#define __QUEUE_H__

#define QUEUE_SIZE 				(1024 +16)

typedef struct queue
{
    int queuesize;
    int head, tail;
    unsigned char *val;
}Queue_uart;

void InitQueue(Queue_uart *q);
int IsQueueEmpty(Queue_uart *q);
int IsQueueFull(Queue_uart *q);
int EnQueue(Queue_uart *q, unsigned char val);
int DeQueue(Queue_uart *q, unsigned char *val);
#endif
