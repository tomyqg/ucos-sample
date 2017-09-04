




#ifndef __STM32_WORKER_PORT_H__
#define __STM32_WORKER_PORT_H__


#include "stm32f4xx.h"
#include "defines.h"
#include "app_cfg.h"

/* ���幤���������ȼ� */
#define  APP_TASK_WORKER_PRIO                              _APP_TASK_WORKER_PRIO

/* ���幤������ջ��С */
#define  APP_TASK_WORKER_STK_SIZE                          1024



/******************************************/
/*           �Ͳ�ӿں���[����]           */
/******************************************/

extern void WORKER_OSIfInit ( void );
extern void WORKER_SemPost ( void );




#endif	/* __STM32_WORKER_PORT_H__ */

