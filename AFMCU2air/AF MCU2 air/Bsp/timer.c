#include "timer.h"
#include "stm32f4xx.h"
#include "interface.h"


unsigned char  udp_tx_count=0;

extern struct dev_status_t	 dev_status_send;	
void TIM3_Int_Init(u16 arr,u16 psc)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);  ///使能TIM3时钟
	
  TIM_TimeBaseInitStructure.TIM_Period = arr; 	
	TIM_TimeBaseInitStructure.TIM_Prescaler=psc;  
	TIM_TimeBaseInitStructure.TIM_CounterMode=TIM_CounterMode_Up; //向上计数模式
	TIM_TimeBaseInitStructure.TIM_ClockDivision=TIM_CKD_DIV1; 
	
	TIM_TimeBaseInit(TIM3,&TIM_TimeBaseInitStructure);     
	
	TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE);   
	TIM_Cmd(TIM3,ENABLE);    
	
	NVIC_InitStructure.NVIC_IRQChannel=TIM3_IRQn;  
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0x01; //抢占优先级1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=0x03; //子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
}


//定时器3中断服务函数

void TIM3_IRQHandler(void)
{
	
	rt_interrupt_enter();
	
	if(TIM_GetITStatus(TIM3,TIM_IT_Update)==SET) //溢出中断
	{
		
		  udp_tx_count++;

	}
	TIM_ClearITPendingBit(TIM3,TIM_IT_Update);  //清除中断标志位
	
	rt_interrupt_leave();
	
}
