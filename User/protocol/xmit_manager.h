


#ifndef __XMIT_MANAGER_H__
#define __XMIT_MANAGER_H__




#include "tiza_timer_port.h"

#include "stm32f4xx.h"



#define TCP_CONNECT_DEBUG 			 1

#define UDP_CONNECT_DEBUG        0

/* 连接IP地址 */
extern ascii mylink_address[];

/* 连接IP端口 */
extern u16   mylink_port;

/* 定义传输监视定时器 */
extern tmr_t *xmit_start_tmr;








/******************************************/
/*              外部函数[声明]            */
/******************************************/

/* 启动传输管理器 */
extern void XMIT_StartManager ( void );

/* 启动连接 */
extern void XMIT_Startup ( u8 ID );

/* 断开连接 */
extern void XMIT_Stop ( void );

/* 重连接 */
extern void XMIT_ConnectOnceAgain ( void );

/* 数据输出 */
extern bool XMIT_DataOutput ( u8 *data, u16 size );
/*tcp可写入状态查询*/
extern bool wip_TCPWriteCheck (u16 size );

/* TCP数据输出 */
extern bool TCP_DataOutput ( u8 *data, u16 size );


#endif  /* __XMIT_MANAGER_H__ */

