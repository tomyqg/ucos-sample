
#ifndef __TZ_LOW_POWER_PROC_H__
#define __TZ_LOW_POWER_PROC_H__

#include "stdbool.h"
#include "stm32f4xx.h"
#include "tiza_gprs_protocol.h"

/******************************************/
/*        TZ-LOW POWER控制[配置]          */
/******************************************/

/* GSM功能控制 */
#ifndef GSM_FUNCTION_IS_OFF
#define GSM_FUNCTION_IS_OFF       1
#endif

/* LED功能控制 */
#ifndef LED_FUNCTION_IS_OFF
#define LED_FUNCTION_IS_OFF       1
#endif

/* CAN功能控制 */
#ifndef CAN_FUNCTION_IS_OFF
#define CAN_FUNCTION_IS_OFF       1
#endif

/* GPS功能控制 */
#ifndef GPS_FUNCTION_IS_OFF
#define GPS_FUNCTION_IS_OFF       1
#endif


/*全局变量*/

extern u32 IsLPWR_Counter;

extern s8 gsmring_io_handler;


/******************************************/
/*           LOW POWER参数[配置]          */
/******************************************/

#define WAKEUP_Type_None                        0x00      /* 唤醒类型：无 */
#define WAKEUP_Type_Calling                     0x01      /* 唤醒类型：外部来电 */
#define WAKEUP_Type_ACC_ON                      0x02      /* 唤醒类型：ACC开 */
#define WAKEUP_Type_Active                      0x04      /* 唤醒类型：中心激活指令 */
#define WAKEUP_Type_Power_off                   0x08      /* 唤醒类型：断电 */
#define WAKEUP_Type_Data_Cycling_Transfer       0x10      /* 唤醒类型：周期性数据传输 */





/******************************************/
/*              外部函数[声明]            */
/******************************************/

/* 浅休眠已就绪 */
extern bool SLEEP_TYPEI_IsReady ( void );

/* 是否退出浅休眠 */
extern bool SLEEP_TYPEI_IsNeedExit ( void );

/* 是否在浅休眠状态 */
extern void SLEEP_TYPEI_IsSleeping ( void );

extern void TmrMonitorHdlr ( u8 ID );

extern void Modem_Stop(sys_status_t states);
extern void Modem_Restart(sys_status_t states);

#endif  /* __TZ_LOW_POWER_PROC_H__ */

