#ifndef __WIP_TCP_H__
#define __WIP_TCP_H__





#include "stm32f4xx.h"

#include "wip_channel.h"





/******************************************/
/*               TCP参数[配置]            */
/******************************************/


/* TCP数据处理 */
typedef bool ( *tcp_procDataHdlr_f ) ( u16 DataSize, u8 *Data );

/* TCP新建套接字通知 */
typedef void ( *tcp_newSocketHdlr_f ) ( u32 address, u16 port );



/******************************************/
/*              外部函数[声明]            */
/******************************************/

/* 创建TCP服务器 */
extern s8 wip_TCPServerCreate ( u16 Port, tcp_newSocketHdlr_f acceptNewSockHandler, void *ctx );

/* 接受新的客户端连接 */
extern s8 wip_TCPServerAcceptNewClient ( tcp_procDataHdlr_f tcpDataHandler );

/* 创建TCP客户端 */
extern s8 wip_TCPClientCreate (ascii* serverIP, u16 serverPort, tcp_procDataHdlr_f tcpDataHandler );

/* 创建TCP事件处理器 */
extern bool wip_TCPCreateEventProcessor ( s8 sock_id, wip_eventHandler_f evHandler );

/* 获取TCP套接字选项 */
extern bool wip_TCPGetSockOption ( s8 sock_id, u32* address, u16* port );

/* 断开TCP连接 */
extern bool wip_TCPDestroy ( s8 sock_id );

/* TCP数据发送 */
/* 警告: 每次发送的最大数据量为1024bytes，否则会造成错误！ */
extern bool wip_TCPWriteData ( s8 sock_id, void *buffer, u16 bufsize );
extern bool wip_TCPWriteDataExt ( s8 sock_id, void *buffer, u16 bufsize );
extern bool wip_TCPWrite( s8 sock_id, void *buffer, u16 bufsize );


/* 警告: 每次发送的最大静态数据量为4096bytes，否则会造成错误！ */
extern bool wip_TCPWriteStaticData ( s8 sock_id, void *staticBuffer, u16 bufsize );

//查询TCP是否可继续写入
extern bool wip_TCPWriteState( s8 sock_id, u16 bufsize );

/* TCP数据读取 */
extern u16  wip_TCPReadData ( s8 sock_id, void *buffer, u16 bufsize );

#endif  /* __WIP_TCP_H__ */

