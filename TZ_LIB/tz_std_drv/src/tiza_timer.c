#define HW_TIMER_GLOBAL

#include "tiza_timer.h"
#include "os_timer.h"
#include "tiza_rtc.h"
#define HW_TIMER_DEBUG


RTC_TimeTypeDef   _RTC_Time={0};

//extern void TimerFunc(void);

// Time_ms最大100ms, 定时器溢出时间计算方法:Tout=((reload+1)*(prescaler+1))/Ft us.
// 定时器时钟84M，分频系数8400，所以84M/8400=10Khz的计数频率，计数10次为1ms
void OpenTimer(uint8 Time_ms)
{	
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4,ENABLE);  ///使能TIM4时钟
	
	TIM_TimeBaseInitStructure.TIM_Period = Time_ms*10-1; 	//自动重装载值
	TIM_TimeBaseInitStructure.TIM_Prescaler= 8400-1;  	//定时器分频
	TIM_TimeBaseInitStructure.TIM_CounterMode=TIM_CounterMode_Up; //向上计数模式
	TIM_TimeBaseInitStructure.TIM_ClockDivision=TIM_CKD_DIV1; 
	
	TIM_TimeBaseInit(TIM4,&TIM_TimeBaseInitStructure);//初始化TIM4
	
	TIM_ITConfig(TIM4,TIM_IT_Update,ENABLE); //允许定时器4更新中断
	TIM_Cmd(TIM4,ENABLE); //使能定时器4
	
	NVIC_InitStructure.NVIC_IRQChannel=TIM4_IRQn; //定时器4中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0x01; //抢占优先级1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=0x03; //子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

void CloseTimer(void)
{
	TIM_ITConfig(TIM4,TIM_IT_Update,DISABLE); //允许定时器4更新中断
	TIM_Cmd(TIM4,DISABLE); //使能定时器4
}
//static u32 SemTimer_count=0;
//定时器4中断服务函数(10MS)
void TIM4_IRQHandler(void)
{
	
	if(TIM_GetITStatus(TIM4,TIM_IT_Update)==SET) //溢出中断
	{	
#if 0	//定时器基时间 1ms
			/*******************************************************
			*  TMR_SemPost为系统软件软件定时器提供基时间：10ms;
			********************************************************/
			TMR_SemPost_1MS();

			///////////////////////////////////////////////////////
#else
//		SemTimer_count++;
//		if(SemTimer_count>=0x50)
//		{
//				SemTimer_count=0;
//				//printf("\r\nApp_TaskIdleHook..........\r\n");
//#if __USE_RTC__==1
//				OS_CPU_SR     cpu_sr;
//				OS_ENTER_CRITICAL();
//				TM_RTC_GetDateTime(&RTC_time,TM_RTC_Format_BIN);
//				OS_EXIT_CRITICAL();
//#endif
//		}	
		
		
		
				TMR_SemPost();    	//硬件定时器10ms,中断一次，TMR_SemPost每10ms发送一信号量

		
#endif
	
	}
	TIM_ClearITPendingBit(TIM4,TIM_IT_Update);  //清除中断标志位
}

