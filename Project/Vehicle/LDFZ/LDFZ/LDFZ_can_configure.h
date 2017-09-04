#ifndef __DFZL_CAN_CONFIGURE_H__
#define __DFZL_CAN_CONFIGURE_H__



#include "stm32f4xx.h"
#include "stdbool.h"




/******************************************/
/*            东风汽车[配置] :中联底盘    */
/******************************************/

/* 波特率设置 */
#ifndef DFZL_CAN_BUS_BPS
#define DFZL_CAN_BUS_BPS                 250/* kHZ */
#define CAN1_BUS_BPS                      ( DFZL_CAN_BUS_BPS )
#endif  /* DFZL_CAN_BUS_BPS */


/* 帧类型设置 */
#ifndef DFZL_CAN_IDE_TYPE     
#define DFZL_CAN_IDE_TYPE                 CAN_FRAME_EXT/* CAN_FRAME_EXT */
#define CAN_IDE_TYPE                     ( DFZL_CAN_IDE_TYPE )
#endif  /* DFZL_CAN_BUS_BPS */


/* 波特率设置 */
#ifndef DFZL_CAN2_BUS_BPS
#define DFZL_CAN2_BUS_BPS                 250/* kHZ */
#define CAN2_BUS_BPS                        ( DFZL_CAN2_BUS_BPS )
#endif  /* DFZL_CAN_BUS_BPS */


/* 帧类型设置 */
#ifndef DFZL_CAN2_IDE_TYPE     
#define DFZL_CAN2_IDE_TYPE                CAN_FRAME_EXT/* CAN_FRAME_EXT */
#define CAN2_IDE_TYPE                     ( DFZL_CAN2_IDE_TYPE )
#endif  /* DFZL_CAN_BUS_BPS */


/* 控制器厂家编号 */
#ifndef NTYWK_CAN_CONTROLLER_ID            
#define NTYWK_CAN_CONTROLLER_ID            0x1800FFFF
#define CAN_CONTROLLER_ID                 ( NTYWK_CAN_CONTROLLER_ID )
#endif  /* DFZL_CAN_CONTROLLER_ID */


/* CAN长度字段占有的字节数 */
#ifndef DFZL_CAN_DATA_LENGTH_BYTES
#define DFZL_CAN_DATA_LENGTH_BYTES        2/* 单位：byte */
//#define CAN_LENGTH_BYTES                  ( DFZL_CAN_DATA_LENGTH_BYTES )
#endif  /* DFZL_CAN_DATA_LENGTH_BYTES */
    


/* CAN接收缓冲长度 */
#ifndef DFZL_CAN_RECV_BUFFER_LENGTH
#define DFZL_CAN_RECV_BUFFER_LENGTH       256/* 单位：byte */
//#define CAN_RECVED_BUFFER_LENGTH          ( JLKC_CAN_RECV_BUFFER_LENGTH )
#endif  /* DFZLCAN_RECV_BUFFER_LENGTH */



/* CAN发送缓冲长度 */
#ifndef DFZL_CAN_SEND_BUFFER_LENGTH
#define DFZL_CAN_SEND_BUFFER_LENGTH       36/* 单位：byte */
//#define CAN_TOSEND_BUFFER_LENGTH          ( JLKC_CAN_SEND_BUFFER_LENGTH )
#endif  /* DFZL_CAN_SEND_BUFFER_LENGTH */



/* CAN发送时间间隔 */
#ifndef DFZL_CAN_SEND_TIME_INETRVAL
#define DFZL_CAN_SEND_TIME_INETRVAL       6/* (单位：s) */
#endif  /* DFZL_CAN_SEND_TIME_INETRVAL */



/* CAN中心下发数据发送总时间 */
#ifndef DFZL_CAN_SEND_TOTAL_TIME
#define DFZL_CAN_SEND_TOTAL_TIME          5*60/* (单位：s) */
#endif  /* DFZL_CAN_SEND_TOTAL_TIME */


/******************************************/
/*              外部函数[声明]            */
/******************************************/

extern void DFZL_CAN_Application_Init ( void );
extern void DFZL_CAN_BackupBufferReset ( void );
extern bool DFZL_CAN_RecvDataHdlr ( u32 FrameID, u16 DataSize, u8 *IPData );
extern void DFZL_CAN_ProcDataHdlr ( void );
extern bool DFZL_CAN_SetControllerParamsByServer ( u16 controllerID, u16 DataSize, u8 *IPData );

#endif  /* __JLKC_CAN_CONFIGURE_H__ */

