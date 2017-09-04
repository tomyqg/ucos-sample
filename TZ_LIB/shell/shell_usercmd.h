


#ifndef __TZ_USERCMD_H__
#define __TZ_USERCMD_H__



#include "stm32f4xx.h"








/******************************************/
/*              外部函数[声明]            */
/******************************************/

/* 用户指令初始化 */
extern void CMD_ApplicationIfInit ( void );

/* 延迟复位响应 */
extern void System_ResetDly ( u8 ID );


#endif  /* __TZ_USERCMD_H__ */

