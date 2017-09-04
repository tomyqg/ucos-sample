


#ifndef __STM32_TIMER_H__
#define __STM32_TIMER_H__



//#include <stm32f10x_lib.h>
#include "stm32f4xx.h"
#include "stdbool.h"
#include "tiza_include.h"




/******************************************/
/*            ��ʱ������[����]            */
/******************************************/


/* ��ʱ������ */
#define TMR_TYPE_100MS  0   /* 100ms��ʱ�� */
#define TMR_TYPE_10MS   1   /* 10ms ��ʱ�� */
#define TMR_TYPE_1MS    2   /* 1ms ��ʱ�� */



/****************/
/* ��ʱ������ */
/****************/

typedef void ( *tmr_procTriggerHdlr_t )( u8 ID );


typedef struct
{
  u8	                  TimerId;      /* ʱ��ID */
  u8                    TimerType;    /* ʱ������ */
  u32                   TimerValue;   /* ʱ���� */
  tmr_procTriggerHdlr_t TimerHdlr;    /* ��Ӧ������ */
} tmr_t;







/******************************************/
/*           �û��ӿں���[����]           */
/******************************************/

/* ע���û�����Ķ�ʱ�� */
extern bool TMR_UnSubscribe ( tmr_t *tmr, tmr_procTriggerHdlr_t Timerhdlr, u8 TimerType );

/* ע�ᶨʱ�� */
extern tmr_t *TMR_Subscribe ( bool cyclic, u32 TimerValue, u8 TimerType, tmr_procTriggerHdlr_t Timerhdlr );

































/******************************************/
/*           �Ͳ�ӿں���[����]           */
/******************************************/

/* ��ʱ����Ԫ�ͼ���ʼ�� */
extern void TMR_LowLevelIfInit ( void );

/* ��ʱ��100ms�������� */
extern void TMR_100msUpdateRequest ( void );

/* ��ʱ��10ms�������� */
extern void TMR_TickUpdateRequest ( void );

/* ��ʱ��1ms�������� */
extern void TMR_1MS_UpdateRequest ( void );
#endif	/* __STM32_TIMER_H__ */

