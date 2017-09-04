#define HW_TIMER_GLOBAL

#include "tiza_timer.h"
#include "os_timer.h"
#include "tiza_rtc.h"
#define HW_TIMER_DEBUG


RTC_TimeTypeDef   _RTC_Time={0};

//extern void TimerFunc(void);

// Time_ms���100ms, ��ʱ�����ʱ����㷽��:Tout=((reload+1)*(prescaler+1))/Ft us.
// ��ʱ��ʱ��84M����Ƶϵ��8400������84M/8400=10Khz�ļ���Ƶ�ʣ�����10��Ϊ1ms
void OpenTimer(uint8 Time_ms)
{	
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4,ENABLE);  ///ʹ��TIM4ʱ��
	
	TIM_TimeBaseInitStructure.TIM_Period = Time_ms*10-1; 	//�Զ���װ��ֵ
	TIM_TimeBaseInitStructure.TIM_Prescaler= 8400-1;  	//��ʱ����Ƶ
	TIM_TimeBaseInitStructure.TIM_CounterMode=TIM_CounterMode_Up; //���ϼ���ģʽ
	TIM_TimeBaseInitStructure.TIM_ClockDivision=TIM_CKD_DIV1; 
	
	TIM_TimeBaseInit(TIM4,&TIM_TimeBaseInitStructure);//��ʼ��TIM4
	
	TIM_ITConfig(TIM4,TIM_IT_Update,ENABLE); //����ʱ��4�����ж�
	TIM_Cmd(TIM4,ENABLE); //ʹ�ܶ�ʱ��4
	
	NVIC_InitStructure.NVIC_IRQChannel=TIM4_IRQn; //��ʱ��4�ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0x01; //��ռ���ȼ�1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=0x03; //�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

void CloseTimer(void)
{
	TIM_ITConfig(TIM4,TIM_IT_Update,DISABLE); //����ʱ��4�����ж�
	TIM_Cmd(TIM4,DISABLE); //ʹ�ܶ�ʱ��4
}
//static u32 SemTimer_count=0;
//��ʱ��4�жϷ�����(10MS)
void TIM4_IRQHandler(void)
{
	
	if(TIM_GetITStatus(TIM4,TIM_IT_Update)==SET) //����ж�
	{	
#if 0	//��ʱ����ʱ�� 1ms
			/*******************************************************
			*  TMR_SemPostΪϵͳ��������ʱ���ṩ��ʱ�䣺10ms;
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
		
		
		
				TMR_SemPost();    	//Ӳ����ʱ��10ms,�ж�һ�Σ�TMR_SemPostÿ10ms����һ�ź���

		
#endif
	
	}
	TIM_ClearITPendingBit(TIM4,TIM_IT_Update);  //����жϱ�־λ
}

