





#ifndef __STM32_UPGRADE_H__
#define __STM32_UPGRADE_H__

#include "stm32f4xx.h"
#include "stdbool.h"

/* 升级服务调试选项 */
// <e>UPGRADE controller debug Definitions
// ===================
//   <i>Enable UPGRADE controller debug function
#define UPGRADE_DEBUG           0


/******************************************/
/*            UPGRADE参数[配置]           */
/******************************************/






/******************************************/
/*           用户接口函数[声明]           */
/******************************************/

/* 启动升级功能 */
extern bool UPGRADE_Start ( void );

/* 写入升级数据 */
extern bool UPGRADE_WriteData ( u8 Data[], u32 DataLen );

/* 安装用户更新程序 */
extern void UPGRADE_InstallApplication ( void );

extern u32 FtpToFlash_DataCount;





extern  bool UPGRADE_GetUpgradeConfigure ( void );

extern bool UPGRADE_SetUpgradeConfigure(void);

































/******************************************/
/*           低层接口函数[声明]           */
/******************************************/

/* 升级单元低级初始化 */
extern void UPGRADE_LowLevelIfInit ( void );





#endif	/* __STM32_UPGRADE_H__ */
