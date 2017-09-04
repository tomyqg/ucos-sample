






#ifndef __WIP_UDP_H__
#define __WIP_UDP_H__



#include "stm32f4xx.h"

#include "wip_channel.h"

#include "stdbool.h"

/******************************************/
/*               UDP参数[配置]            */
/******************************************/







/******************************************/
/*              外部函数[声明]            */
/******************************************/

/* 创建UDP连接 */
extern s8 wip_UDPCreateOpts ( ascii* remoteIP, u16 remotePort, wip_eventHandler_f evHandler, void *ctx );

/* UDP数据发送 */
extern bool wip_UDPWriteData ( s8 sock_id, void *buffer, u16 bufsize );

/* UDP数据读取 */
extern u16  wip_UDPReadData ( s8 sock_id, void *buffer, u16 bufsize );

/* UDP连接断开 */
extern bool wip_UDPDestroy ( s8 sock_id );



#endif  /* __WIP_UDP_H__ */




