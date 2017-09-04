#ifndef __STM32_LOW_POWER_H__
#define __STM32_LOW_POWER_H__


#include "stm32f4xx.h"
#include "stdbool.h"

#define LPWR_DEBUG      1

/******************************************/
/*              低功耗[配置]              */
/******************************************/

/* 定义电源模式 */
#define LPWR_RUN_Mode         0x00    /* 正常模式 */
#define LPWR_STOP_Mode        0x01    /* 停止模式 */
#define LPWR_STANDBY_Mode     0x02    /* 待机模式 */


/* 停机模式状态 */          
#define LPWR_STOP_STATE_IS_READY            0x01    /* 停机状态已准备 */
#define LPWR_STOP_STATE_IS_SLEEPING         0x02    /* 停机状态 */      
#define LPWR_STOP_STATE_IS_RETURN_NORMAL    0x04    /* 待机状态退出 */




/* 唤醒处理函数 */
typedef void ( *lp_processHdlr_f ) ( u8 state );



/******************************************/
/*           用户接口函数[声明]           */
/******************************************/

/* 启动低功耗配置 */
extern void LPWR_Start ( u8 LpType, lp_processHdlr_f lpHandler );

/* 恢复正常运行状态 */
extern void LPWR_EnterRUNMode ( void );



/******************************************/
/*           低层接口函数[声明]           */
/******************************************/

/* 低功耗单元低级初始化 */
extern void LPWR_LowLevelIfInit ( void );

/* 是否停止模式 */
extern bool LPWR_IsSTOPMode ( void );

/* 获取电源运行状态 */
extern u8   LPWR_GetPowerStatus ( void );

/* 假休眠周期唤醒处理 */
extern void LPWR_CyclicWakeupHandler ( void );

#endif	/* __STM32_LOW_POWER_H__ */



