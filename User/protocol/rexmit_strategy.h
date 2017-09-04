

#ifndef __REXMIT_STATEGY_H__
#define __REXMIT_STATEGY_H__


#include "stm32f4xx.h"
#include "stdbool.h"

/******************************************/
/*              传输参数[配置]            */
/******************************************/

/* 定义数据输出端口 */
typedef bool ( *xmit_OutPortHdlr_f ) ( u8 *data, u16 size );

/* 定义错误码 */
#define XMIT_ERR_OK             (0)   /* 正常 */
#define XMIT_ERR_INTERNAL       (-1)  /* 内部错误 */
#define XMIT_ERR_NO_RES         (-2)  /* 无资源 */
#define XMIT_ERR_USER_PARAMS    (-3)  /* 参数错误 */
#define XMIT_ERR_INIT           (-4)  /* 未初始化 */
#define XMIT_ERR_HANDLE         (-5)  /* 句柄错误 */

/* 超时数据处理扩展 */
typedef bool ( *xmit_TimeoutHdlr_f ) ( s32 handle, u8 type, u8 *data, u16 size );








/******************************************/
/*              外部函数[声明]            */
/******************************************/

/* 初始化重传机制配置数据 */
extern void XMIT_InitTimeout ( xmit_OutPortHdlr_f xmitOP );

/* 启动超时机制 */
extern s32  XMIT_StartupTimeout ( u8 type, u8 retry, s16 timeout, u8* data, u16 size );

/* 取消超时机制 */
extern bool XMIT_CancelTimeout ( s32 handle );

/* 注册超时处理器 */
extern bool XMIT_SubscribeTimeoutHandler ( xmit_TimeoutHdlr_f xmitToHdlr );

/* 查询超时状态 */
extern void XMIT_QueryTimeoutState ( s32 handle );

/* 巡检超时机制 */
extern u32  XMIT_PollingTimeout ( void );

/* 清空错误码 */
extern bool XMIT_ClearLastErrCode ( void );

/* 获取错误码 */
extern s8   XMIT_GetLastErrCode ( void );


#endif  /* __REXMIT_STATEGY_H__ */

