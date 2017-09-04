#ifndef __TIZA_HW_TIMER_H
#define __TIZA_HW_TIMER_H

#include "tiza_include.h"




extern RTC_TimeTypeDef _RTC_Time;

	#ifdef HW_TIMER_GLOBAL
		#define EXTERN_HW_TIMER
	#else
		#define EXTERN_HW_TIMER extern
	#endif

	//Ӳ����ʱ��
	EXTERN_HW_TIMER void OpenTimer(uint8 Time_ms);	// Time_ms���100ms
	EXTERN_HW_TIMER void CloseTimer(void);
	
	/*Ϋ���Լ����嶨ʱ���жϵ��õĺ���  void TimerFunc(void) */
	
#endif

