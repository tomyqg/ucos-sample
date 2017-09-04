






#ifndef __OS_CAN_H__
#define __OS_CAN_H__
#include "includes.h"
#include "app_cfg.h"

/* ����CAN��������ջ��С */
#define  APP_TASK_CANREC_STK_SIZE                        512

/* ����CAN�������ȼ� */
#define  APP_TASK_CANREC_PRIO		                           _APP_TASK_CANREC_PRIO

/*
*****************************************************************
*
*CANģ���������
*
******************************************************************
*/
/*****************************************************************/
#define  OSCAN_DEBUG    1

#define  IN_BUFFER_SIZE           200       //�������
/*****************************************************************/






/* ������������������������������������������������
*
*  ���廷�ζ���
* 
������������������������������������������������ */
typedef struct    
{
   CanRxMsg *buffer;
   vu16 head;         //Unsigned short value of the head index.
   vu16 tail;         //Unsigned short value of the tail index.
   vu8  buffer_size;
}RING_T;



static void App_TaskCanRec(void *parg);



extern void  OSCAN_InitOS (void);
extern void   OSCAN_SemPost (void);

#if ( Sofe_Cache == 0 )
extern bool OSCAN_GetTaskActiveFalg( void );
#endif

extern void rng_enqueue(CanRxMsg msg, RING_T *ring);
extern CanRxMsg *rng_dequeue(RING_T *ring);



#endif    /* __OS_CAN_H__ */ 
