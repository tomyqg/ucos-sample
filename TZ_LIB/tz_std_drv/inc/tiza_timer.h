#ifndef __TIZA_HW_TIMER_H
#define __TIZA_HW_TIMER_H

#include "tiza_include.h"




extern RTC_TimeTypeDef _RTC_Time;

	#ifdef HW_TIMER_GLOBAL
		#define EXTERN_HW_TIMER
	#else
		#define EXTERN_HW_TIMER extern
	#endif

	//硬件定时器
	EXTERN_HW_TIMER void OpenTimer(uint8 Time_ms);	// Time_ms最大100ms
	EXTERN_HW_TIMER void CloseTimer(void);
	
	/*潍柴自己定义定时器中断调用的函数  void TimerFunc(void) */
	
#endif

