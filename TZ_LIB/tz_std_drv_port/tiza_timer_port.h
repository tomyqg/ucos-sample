


#ifndef __STM32_TIMER_H__
#define __STM32_TIMER_H__



//#include <stm32f10x_lib.h>
#include "stm32f4xx.h"
#include "stdbool.h"
#include "tiza_include.h"




/******************************************/
/*            定时器参数[配置]            */
/******************************************/


/* 定时器类型 */
#define TMR_TYPE_100MS  0   /* 100ms定时器 */
#define TMR_TYPE_10MS   1   /* 10ms 定时器 */
#define TMR_TYPE_1MS    2   /* 1ms 定时器 */



/****************/
/* 定时处理函数 */
/****************/

typedef void ( *tmr_procTriggerHdlr_t )( u8 ID );


typedef struct
{
  u8	                  TimerId;      /* 时钟ID */
  u8                    TimerType;    /* 时钟类型 */
  u32                   TimerValue;   /* 时间间隔 */
  tmr_procTriggerHdlr_t TimerHdlr;    /* 响应处理句柄 */
} tmr_t;







/******************************************/
/*           用户接口函数[声明]           */
/******************************************/

/* 注销用户分配的定时器 */
extern bool TMR_UnSubscribe ( tmr_t *tmr, tmr_procTriggerHdlr_t Timerhdlr, u8 TimerType );

/* 注册定时器 */
extern tmr_t *TMR_Subscribe ( bool cyclic, u32 TimerValue, u8 TimerType, tmr_procTriggerHdlr_t Timerhdlr );

































/******************************************/
/*           低层接口函数[声明]           */
/******************************************/

/* 软定时器单元低级初始化 */
extern void TMR_LowLevelIfInit ( void );

/* 软定时器100ms更新请求 */
extern void TMR_100msUpdateRequest ( void );

/* 软定时器10ms更新请求 */
extern void TMR_TickUpdateRequest ( void );

/* 软定时器1ms更新请求 */
extern void TMR_1MS_UpdateRequest ( void );
#endif	/* __STM32_TIMER_H__ */

