#ifndef _TIMER_H
#define _TIMER_H
#include "rtthread.h"
#include "stm32f4xx.h"	

//typedef unsigned short     int uint16_t;


void TIM3_Int_Init(u16 arr,u16 psc);

#endif
 