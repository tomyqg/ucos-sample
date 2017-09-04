
#ifndef __STM32_TMR_PORT_H__
#define __STM32_TMR_PORT_H__
#include "app_cfg.h"

/* ����1MS��ʱ�����ȼ� */
#define  APP_TASK_TMR_1MS_PRIO                             _APP_TASK_TMR_1MS_PRIO

/* ����10MS��ʱ�����ȼ� */
#define  APP_TASK_TMR_10MS_PRIO                            _APP_TASK_TMR_10MS_PRIO

/* ����100MS��ʱ�����ȼ� */
#define  APP_TASK_TMR_100MS_PRIO                            _APP_TASK_TMR_100MS_PRIO



/* ������ٶ�ʱ��ջ��С */
#define  APP_TASK_TMR_TICK_STK_SIZE                      1024

/* �������ٶ�ʱ��ջ��С */
#define  APP_TASK_TMR_SLOW_STK_SIZE                      512

/* ����1MS��ʱ��ջ��С */
#define  APP_TASK_TMR_1MS_STK_SIZE                       256







/******************************************/
/*           �Ͳ�ӿں���[����]           */
/******************************************/

extern void TMR_OSIfInit ( void );
extern void TMR_SemPost ( void );
extern void TMR_SemPost_1MS( void );



#endif	/* __STM32_TMR_PORT_H__ */

