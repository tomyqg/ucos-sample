




#ifndef __STM32_WORKER_PORT_H__
#define __STM32_WORKER_PORT_H__


#include "stm32f4xx.h"
#include "defines.h"
#include "app_cfg.h"

/* 定义工作队列优先级 */
#define  APP_TASK_WORKER_PRIO                              _APP_TASK_WORKER_PRIO

/* 定义工作队列栈大小 */
#define  APP_TASK_WORKER_STK_SIZE                          1024



/******************************************/
/*           低层接口函数[声明]           */
/******************************************/

extern void WORKER_OSIfInit ( void );
extern void WORKER_SemPost ( void );




#endif	/* __STM32_WORKER_PORT_H__ */

