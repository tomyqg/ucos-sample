






#ifndef __WIP_UDP_H__
#define __WIP_UDP_H__



#include "stm32f4xx.h"

#include "wip_channel.h"

#include "stdbool.h"

/******************************************/
/*               UDP����[����]            */
/******************************************/







/******************************************/
/*              �ⲿ����[����]            */
/******************************************/

/* ����UDP���� */
extern s8 wip_UDPCreateOpts ( ascii* remoteIP, u16 remotePort, wip_eventHandler_f evHandler, void *ctx );

/* UDP���ݷ��� */
extern bool wip_UDPWriteData ( s8 sock_id, void *buffer, u16 bufsize );

/* UDP���ݶ�ȡ */
extern u16  wip_UDPReadData ( s8 sock_id, void *buffer, u16 bufsize );

/* UDP���ӶϿ� */
extern bool wip_UDPDestroy ( s8 sock_id );



#endif  /* __WIP_UDP_H__ */




