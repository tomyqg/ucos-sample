#ifndef __STM32_USART_PORT_H__
#define __STM32_USART_PORT_H__

#include "includes.h"
#include "app_cfg.h"
#include "tiza_usart_port.h"


///////////////////////////////////////////////////////

#define USART_TASK_DEBUG    0

///////////////////////////////////////////////////////

/* ���崮�ڽ������ȼ� */
#define  APP_TASK_USART_RX_PRIO                            _APP_TASK_USART_RX_PRIO

/* ���崮�ڽ�������ջ��С */
#define  APP_TASK_USART_RX_STK_SIZE                        512




/******************************************/
/*           �Ͳ�ӿں���[����]           */
/******************************************/

extern void TZ_UART_ApplicationIfInit(void);

extern void USART_OSIfInit ( void );
extern void USART_SemPend ( usart_Flow_e Flow );
extern void USART_SemPost ( usart_Flow_e Flow );
extern bool USART_SemPendExt ( usart_Flow_e Flow, u32 timeout );
extern void USART_SemPost2 ( void );

#if __USE_USART__ == 1
#define UASRT_RX_EVENT_TRIGGER()   
#endif  /* __USE_USART__ */



#endif	/* __STM32_USART_PORT_H__ */

