
#ifndef __STM32_TMR_PORT_H__
#define __STM32_TMR_PORT_H__
#include "app_cfg.h"

/* 定义1MS定时器优先级 */
#define  APP_TASK_TMR_1MS_PRIO                             _APP_TASK_TMR_1MS_PRIO

/* 定义10MS定时器优先级 */
#define  APP_TASK_TMR_10MS_PRIO                            _APP_TASK_TMR_10MS_PRIO

/* 定义100MS定时器优先级 */
#define  APP_TASK_TMR_100MS_PRIO                            _APP_TASK_TMR_100MS_PRIO



/* 定义快速定时器栈大小 */
#define  APP_TASK_TMR_TICK_STK_SIZE                      1024

/* 定义慢速定时器栈大小 */
#define  APP_TASK_TMR_SLOW_STK_SIZE                      512

/* 定义1MS定时器栈大小 */
#define  APP_TASK_TMR_1MS_STK_SIZE                       256







/******************************************/
/*           低层接口函数[声明]           */
/******************************************/

extern void TMR_OSIfInit ( void );
extern void TMR_SemPost ( void );
extern void TMR_SemPost_1MS( void );



#endif	/* __STM32_TMR_PORT_H__ */

