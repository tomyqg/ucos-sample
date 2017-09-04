#ifndef __WIP_TCP_H__
#define __WIP_TCP_H__





#include "stm32f4xx.h"

#include "wip_channel.h"





/******************************************/
/*               TCP����[����]            */
/******************************************/


/* TCP���ݴ��� */
typedef bool ( *tcp_procDataHdlr_f ) ( u16 DataSize, u8 *Data );

/* TCP�½��׽���֪ͨ */
typedef void ( *tcp_newSocketHdlr_f ) ( u32 address, u16 port );



/******************************************/
/*              �ⲿ����[����]            */
/******************************************/

/* ����TCP������ */
extern s8 wip_TCPServerCreate ( u16 Port, tcp_newSocketHdlr_f acceptNewSockHandler, void *ctx );

/* �����µĿͻ������� */
extern s8 wip_TCPServerAcceptNewClient ( tcp_procDataHdlr_f tcpDataHandler );

/* ����TCP�ͻ��� */
extern s8 wip_TCPClientCreate (ascii* serverIP, u16 serverPort, tcp_procDataHdlr_f tcpDataHandler );

/* ����TCP�¼������� */
extern bool wip_TCPCreateEventProcessor ( s8 sock_id, wip_eventHandler_f evHandler );

/* ��ȡTCP�׽���ѡ�� */
extern bool wip_TCPGetSockOption ( s8 sock_id, u32* address, u16* port );

/* �Ͽ�TCP���� */
extern bool wip_TCPDestroy ( s8 sock_id );

/* TCP���ݷ��� */
/* ����: ÿ�η��͵����������Ϊ1024bytes���������ɴ��� */
extern bool wip_TCPWriteData ( s8 sock_id, void *buffer, u16 bufsize );
extern bool wip_TCPWriteDataExt ( s8 sock_id, void *buffer, u16 bufsize );
extern bool wip_TCPWrite( s8 sock_id, void *buffer, u16 bufsize );


/* ����: ÿ�η��͵����̬������Ϊ4096bytes���������ɴ��� */
extern bool wip_TCPWriteStaticData ( s8 sock_id, void *staticBuffer, u16 bufsize );

//��ѯTCP�Ƿ�ɼ���д��
extern bool wip_TCPWriteState( s8 sock_id, u16 bufsize );

/* TCP���ݶ�ȡ */
extern u16  wip_TCPReadData ( s8 sock_id, void *buffer, u16 bufsize );

#endif  /* __WIP_TCP_H__ */

