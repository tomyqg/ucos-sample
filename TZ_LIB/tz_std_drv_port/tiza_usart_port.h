


#ifndef __STM32_USART_H__
#define __STM32_USART_H__



#include "stm32f4xx.h"
#include "tiza_include.h"


/******************************************/
/*              ���ڲ���[����]            */
/******************************************/

typedef u8 usart_Flow_e;


/* ��ѡ�����豸 */
#define DEV_USART1    0x00
#define DEV_USART2    0x01
#define DEV_USART3    0x02
#define DEV_USART4    0x03
#define DEV_USART5    0x04
#define DEV_USART_ALL 0xFF


/* ��ѡ�����豸 */
#define  SHELL_USART   DEV_USART1

#define  GPS_USART     DEV_USART4 

#define  GSM_USART     DEV_USART3 



/* �������Ͷ��� */
#define USART_InitType_None                         0x0000    /* ��ѡ�� */
#define USART_InitType_BaudRate                     0x0001    /* ������ */
#define USART_InitType_WordLength                   0x0002    /* ����λ */
#define USART_InitType_StopBits                     0x0004    /* ֹͣλ */
#define USART_InitType_Parity                       0x0008    /* ��żУ�� */
#define USART_InitType_HardwareFlowControl          0x0010    /* Ӳ������ */


/* ��ѡ���������� */
#define USART_BaudRate_9600bps                      9600
#define USART_BaudRate_38400bps                     38400
#define USART_BaudRate_115200bps                    115200
#define USART_BaudRate_Default                      115200



/* USART���ݴ��� */
typedef bool  ( * usart_procDataHdlr_f ) ( u16 DataSize, u8 *Data );






/******************************************/
/*           �û��ӿں���[����]           */
/******************************************/

/* ��������ͨ������ */
extern bool USARTx_Start ( usart_Flow_e Flow, usart_procDataHdlr_f DataHandler );

/* ֹͣ����ͨ�� */
extern void USART_Stop ( usart_Flow_e Flow );

/* ����ͨ������ */
extern bool USART_IOCtl ( usart_Flow_e Flow, ... );

/* д�봮��ͨ������ */
extern bool USART_WriteData ( usart_Flow_e Flow, u8 Data[], u16 DataLen );

/* ���ô���ͨ�ų�ʱ���(ms) */
extern bool USART_SetRxTimeoutByNms ( usart_Flow_e Flow, u32 RxTimeoutValue );


extern bool Shell_DataHandler( u16 DataSize, u8 * Data );

extern bool GPS_DataHandler( u16 DataSize, u8 * Data );

extern bool GSM_DataHandler( u16 DataSize, u8 * Data );

extern void GPS_ReceiveHandler(uint8_t data);

extern void GSM_ReceiveHandler(uint8_t data);

extern void SHELL_ReceiveHandler(uint8_t data);
















































/******************************************/
/*           �Ͳ�ӿں���[����]           */
/******************************************/

/* ���ڵ�Ԫ�ͼ���ʼ�� */
extern void USART_LowLevelIfInit ( usart_Flow_e Flow );

/* �������ݴ��� */
extern void USART_LowLevelDataProcess ( void );

/* ���ڵ�����Ϣ��ʾ */
//extern void USART_DebugInfo ( void );

#endif	/* __STM32_USART_H__ */


