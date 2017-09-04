
#ifndef __TZ_LOW_POWER_PROC_H__
#define __TZ_LOW_POWER_PROC_H__

#include "stdbool.h"
#include "stm32f4xx.h"
#include "tiza_gprs_protocol.h"

/******************************************/
/*        TZ-LOW POWER����[����]          */
/******************************************/

/* GSM���ܿ��� */
#ifndef GSM_FUNCTION_IS_OFF
#define GSM_FUNCTION_IS_OFF       1
#endif

/* LED���ܿ��� */
#ifndef LED_FUNCTION_IS_OFF
#define LED_FUNCTION_IS_OFF       1
#endif

/* CAN���ܿ��� */
#ifndef CAN_FUNCTION_IS_OFF
#define CAN_FUNCTION_IS_OFF       1
#endif

/* GPS���ܿ��� */
#ifndef GPS_FUNCTION_IS_OFF
#define GPS_FUNCTION_IS_OFF       1
#endif


/*ȫ�ֱ���*/

extern u32 IsLPWR_Counter;

extern s8 gsmring_io_handler;


/******************************************/
/*           LOW POWER����[����]          */
/******************************************/

#define WAKEUP_Type_None                        0x00      /* �������ͣ��� */
#define WAKEUP_Type_Calling                     0x01      /* �������ͣ��ⲿ���� */
#define WAKEUP_Type_ACC_ON                      0x02      /* �������ͣ�ACC�� */
#define WAKEUP_Type_Active                      0x04      /* �������ͣ����ļ���ָ�� */
#define WAKEUP_Type_Power_off                   0x08      /* �������ͣ��ϵ� */
#define WAKEUP_Type_Data_Cycling_Transfer       0x10      /* �������ͣ����������ݴ��� */





/******************************************/
/*              �ⲿ����[����]            */
/******************************************/

/* ǳ�����Ѿ��� */
extern bool SLEEP_TYPEI_IsReady ( void );

/* �Ƿ��˳�ǳ���� */
extern bool SLEEP_TYPEI_IsNeedExit ( void );

/* �Ƿ���ǳ����״̬ */
extern void SLEEP_TYPEI_IsSleeping ( void );

extern void TmrMonitorHdlr ( u8 ID );

extern void Modem_Stop(sys_status_t states);
extern void Modem_Restart(sys_status_t states);

#endif  /* __TZ_LOW_POWER_PROC_H__ */

