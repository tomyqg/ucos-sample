#ifndef __STM32_LOW_POWER_H__
#define __STM32_LOW_POWER_H__


#include "stm32f4xx.h"
#include "stdbool.h"

#define LPWR_DEBUG      1

/******************************************/
/*              �͹���[����]              */
/******************************************/

/* �����Դģʽ */
#define LPWR_RUN_Mode         0x00    /* ����ģʽ */
#define LPWR_STOP_Mode        0x01    /* ֹͣģʽ */
#define LPWR_STANDBY_Mode     0x02    /* ����ģʽ */


/* ͣ��ģʽ״̬ */          
#define LPWR_STOP_STATE_IS_READY            0x01    /* ͣ��״̬��׼�� */
#define LPWR_STOP_STATE_IS_SLEEPING         0x02    /* ͣ��״̬ */      
#define LPWR_STOP_STATE_IS_RETURN_NORMAL    0x04    /* ����״̬�˳� */




/* ���Ѵ����� */
typedef void ( *lp_processHdlr_f ) ( u8 state );



/******************************************/
/*           �û��ӿں���[����]           */
/******************************************/

/* �����͹������� */
extern void LPWR_Start ( u8 LpType, lp_processHdlr_f lpHandler );

/* �ָ���������״̬ */
extern void LPWR_EnterRUNMode ( void );



/******************************************/
/*           �Ͳ�ӿں���[����]           */
/******************************************/

/* �͹��ĵ�Ԫ�ͼ���ʼ�� */
extern void LPWR_LowLevelIfInit ( void );

/* �Ƿ�ֹͣģʽ */
extern bool LPWR_IsSTOPMode ( void );

/* ��ȡ��Դ����״̬ */
extern u8   LPWR_GetPowerStatus ( void );

/* ���������ڻ��Ѵ��� */
extern void LPWR_CyclicWakeupHandler ( void );

#endif	/* __STM32_LOW_POWER_H__ */



