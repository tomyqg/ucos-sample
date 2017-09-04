#include "stm32f4xx.h"



#ifndef __STM32_WDG_H__
#define __STM32_WDG_H__






/******************************************/
/*               WDG参数[配置]            */
/******************************************/

/* 可选看门狗类型 */
#define WDG_TYPE_EXTERNAL         0x01			/* 外部看门狗 */
#define WDG_TYPE_INDEPENDENCY     0x02			/* 独立看门狗 */
#define WDG_TYPE_WINDOWS          0x03			/* 窗口看门狗 */










/******************************************/
/*           低层接口函数[声明]           */
/******************************************/

/* 看门狗配置 */
extern void WDG_configuration ( void );


/* 喂狗 */
extern void WDG_KickCmd ( void );




#endif	/* __STM32_WDG_H__ */

