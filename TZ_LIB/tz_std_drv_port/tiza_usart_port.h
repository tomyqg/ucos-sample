


#ifndef __STM32_USART_H__
#define __STM32_USART_H__



#include "stm32f4xx.h"
#include "tiza_include.h"


/******************************************/
/*              串口参数[配置]            */
/******************************************/

typedef u8 usart_Flow_e;


/* 可选串口设备 */
#define DEV_USART1    0x00
#define DEV_USART2    0x01
#define DEV_USART3    0x02
#define DEV_USART4    0x03
#define DEV_USART5    0x04
#define DEV_USART_ALL 0xFF


/* 可选串口设备 */
#define  SHELL_USART   DEV_USART1

#define  GPS_USART     DEV_USART4 

#define  GSM_USART     DEV_USART3 



/* 配置类型定义 */
#define USART_InitType_None                         0x0000    /* 无选项 */
#define USART_InitType_BaudRate                     0x0001    /* 波特率 */
#define USART_InitType_WordLength                   0x0002    /* 数据位 */
#define USART_InitType_StopBits                     0x0004    /* 停止位 */
#define USART_InitType_Parity                       0x0008    /* 奇偶校验 */
#define USART_InitType_HardwareFlowControl          0x0010    /* 硬件流控 */


/* 可选波特率配置 */
#define USART_BaudRate_9600bps                      9600
#define USART_BaudRate_38400bps                     38400
#define USART_BaudRate_115200bps                    115200
#define USART_BaudRate_Default                      115200



/* USART数据处理 */
typedef bool  ( * usart_procDataHdlr_f ) ( u16 DataSize, u8 *Data );






/******************************************/
/*           用户接口函数[声明]           */
/******************************************/

/* 启动串口通信配置 */
extern bool USARTx_Start ( usart_Flow_e Flow, usart_procDataHdlr_f DataHandler );

/* 停止串口通信 */
extern void USART_Stop ( usart_Flow_e Flow );

/* 串口通信配置 */
extern bool USART_IOCtl ( usart_Flow_e Flow, ... );

/* 写入串口通信数据 */
extern bool USART_WriteData ( usart_Flow_e Flow, u8 Data[], u16 DataLen );

/* 设置串口通信超时间隔(ms) */
extern bool USART_SetRxTimeoutByNms ( usart_Flow_e Flow, u32 RxTimeoutValue );


extern bool Shell_DataHandler( u16 DataSize, u8 * Data );

extern bool GPS_DataHandler( u16 DataSize, u8 * Data );

extern bool GSM_DataHandler( u16 DataSize, u8 * Data );

extern void GPS_ReceiveHandler(uint8_t data);

extern void GSM_ReceiveHandler(uint8_t data);

extern void SHELL_ReceiveHandler(uint8_t data);
















































/******************************************/
/*           低层接口函数[声明]           */
/******************************************/

/* 串口单元低级初始化 */
extern void USART_LowLevelIfInit ( usart_Flow_e Flow );

/* 串口数据处理 */
extern void USART_LowLevelDataProcess ( void );

/* 串口调试信息显示 */
//extern void USART_DebugInfo ( void );

#endif	/* __STM32_USART_H__ */


