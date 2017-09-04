#ifndef __DFZL_CAN_CONFIGURE_H__
#define __DFZL_CAN_CONFIGURE_H__



#include "stm32f4xx.h"
#include "stdbool.h"




/******************************************/
/*            ��������[����] :��������    */
/******************************************/

/* ���������� */
#ifndef DFZL_CAN_BUS_BPS
#define DFZL_CAN_BUS_BPS                 250/* kHZ */
#define CAN1_BUS_BPS                      ( DFZL_CAN_BUS_BPS )
#endif  /* DFZL_CAN_BUS_BPS */


/* ֡�������� */
#ifndef DFZL_CAN_IDE_TYPE     
#define DFZL_CAN_IDE_TYPE                 CAN_FRAME_EXT/* CAN_FRAME_EXT */
#define CAN_IDE_TYPE                     ( DFZL_CAN_IDE_TYPE )
#endif  /* DFZL_CAN_BUS_BPS */


/* ���������� */
#ifndef DFZL_CAN2_BUS_BPS
#define DFZL_CAN2_BUS_BPS                 250/* kHZ */
#define CAN2_BUS_BPS                        ( DFZL_CAN2_BUS_BPS )
#endif  /* DFZL_CAN_BUS_BPS */


/* ֡�������� */
#ifndef DFZL_CAN2_IDE_TYPE     
#define DFZL_CAN2_IDE_TYPE                CAN_FRAME_EXT/* CAN_FRAME_EXT */
#define CAN2_IDE_TYPE                     ( DFZL_CAN2_IDE_TYPE )
#endif  /* DFZL_CAN_BUS_BPS */


/* ���������ұ�� */
#ifndef NTYWK_CAN_CONTROLLER_ID            
#define NTYWK_CAN_CONTROLLER_ID            0x1800FFFF
#define CAN_CONTROLLER_ID                 ( NTYWK_CAN_CONTROLLER_ID )
#endif  /* DFZL_CAN_CONTROLLER_ID */


/* CAN�����ֶ�ռ�е��ֽ��� */
#ifndef DFZL_CAN_DATA_LENGTH_BYTES
#define DFZL_CAN_DATA_LENGTH_BYTES        2/* ��λ��byte */
//#define CAN_LENGTH_BYTES                  ( DFZL_CAN_DATA_LENGTH_BYTES )
#endif  /* DFZL_CAN_DATA_LENGTH_BYTES */
    


/* CAN���ջ��峤�� */
#ifndef DFZL_CAN_RECV_BUFFER_LENGTH
#define DFZL_CAN_RECV_BUFFER_LENGTH       256/* ��λ��byte */
//#define CAN_RECVED_BUFFER_LENGTH          ( JLKC_CAN_RECV_BUFFER_LENGTH )
#endif  /* DFZLCAN_RECV_BUFFER_LENGTH */



/* CAN���ͻ��峤�� */
#ifndef DFZL_CAN_SEND_BUFFER_LENGTH
#define DFZL_CAN_SEND_BUFFER_LENGTH       36/* ��λ��byte */
//#define CAN_TOSEND_BUFFER_LENGTH          ( JLKC_CAN_SEND_BUFFER_LENGTH )
#endif  /* DFZL_CAN_SEND_BUFFER_LENGTH */



/* CAN����ʱ���� */
#ifndef DFZL_CAN_SEND_TIME_INETRVAL
#define DFZL_CAN_SEND_TIME_INETRVAL       6/* (��λ��s) */
#endif  /* DFZL_CAN_SEND_TIME_INETRVAL */



/* CAN�����·����ݷ�����ʱ�� */
#ifndef DFZL_CAN_SEND_TOTAL_TIME
#define DFZL_CAN_SEND_TOTAL_TIME          5*60/* (��λ��s) */
#endif  /* DFZL_CAN_SEND_TOTAL_TIME */


/******************************************/
/*              �ⲿ����[����]            */
/******************************************/

extern void DFZL_CAN_Application_Init ( void );
extern void DFZL_CAN_BackupBufferReset ( void );
extern bool DFZL_CAN_RecvDataHdlr ( u32 FrameID, u16 DataSize, u8 *IPData );
extern void DFZL_CAN_ProcDataHdlr ( void );
extern bool DFZL_CAN_SetControllerParamsByServer ( u16 controllerID, u16 DataSize, u8 *IPData );

#endif  /* __JLKC_CAN_CONFIGURE_H__ */

