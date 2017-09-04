


#ifndef __XMIT_MANAGER_H__
#define __XMIT_MANAGER_H__




#include "tiza_timer_port.h"

#include "stm32f4xx.h"



#define TCP_CONNECT_DEBUG 			 1

#define UDP_CONNECT_DEBUG        0

/* ����IP��ַ */
extern ascii mylink_address[];

/* ����IP�˿� */
extern u16   mylink_port;

/* ���崫����Ӷ�ʱ�� */
extern tmr_t *xmit_start_tmr;








/******************************************/
/*              �ⲿ����[����]            */
/******************************************/

/* ������������� */
extern void XMIT_StartManager ( void );

/* �������� */
extern void XMIT_Startup ( u8 ID );

/* �Ͽ����� */
extern void XMIT_Stop ( void );

/* ������ */
extern void XMIT_ConnectOnceAgain ( void );

/* ������� */
extern bool XMIT_DataOutput ( u8 *data, u16 size );
/*tcp��д��״̬��ѯ*/
extern bool wip_TCPWriteCheck (u16 size );

/* TCP������� */
extern bool TCP_DataOutput ( u8 *data, u16 size );


#endif  /* __XMIT_MANAGER_H__ */

