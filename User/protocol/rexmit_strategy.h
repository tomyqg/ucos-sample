

#ifndef __REXMIT_STATEGY_H__
#define __REXMIT_STATEGY_H__


#include "stm32f4xx.h"
#include "stdbool.h"

/******************************************/
/*              �������[����]            */
/******************************************/

/* ������������˿� */
typedef bool ( *xmit_OutPortHdlr_f ) ( u8 *data, u16 size );

/* ��������� */
#define XMIT_ERR_OK             (0)   /* ���� */
#define XMIT_ERR_INTERNAL       (-1)  /* �ڲ����� */
#define XMIT_ERR_NO_RES         (-2)  /* ����Դ */
#define XMIT_ERR_USER_PARAMS    (-3)  /* �������� */
#define XMIT_ERR_INIT           (-4)  /* δ��ʼ�� */
#define XMIT_ERR_HANDLE         (-5)  /* ������� */

/* ��ʱ���ݴ�����չ */
typedef bool ( *xmit_TimeoutHdlr_f ) ( s32 handle, u8 type, u8 *data, u16 size );








/******************************************/
/*              �ⲿ����[����]            */
/******************************************/

/* ��ʼ���ش������������� */
extern void XMIT_InitTimeout ( xmit_OutPortHdlr_f xmitOP );

/* ������ʱ���� */
extern s32  XMIT_StartupTimeout ( u8 type, u8 retry, s16 timeout, u8* data, u16 size );

/* ȡ����ʱ���� */
extern bool XMIT_CancelTimeout ( s32 handle );

/* ע�ᳬʱ������ */
extern bool XMIT_SubscribeTimeoutHandler ( xmit_TimeoutHdlr_f xmitToHdlr );

/* ��ѯ��ʱ״̬ */
extern void XMIT_QueryTimeoutState ( s32 handle );

/* Ѳ�쳬ʱ���� */
extern u32  XMIT_PollingTimeout ( void );

/* ��մ����� */
extern bool XMIT_ClearLastErrCode ( void );

/* ��ȡ������ */
extern s8   XMIT_GetLastErrCode ( void );


#endif  /* __REXMIT_STATEGY_H__ */

