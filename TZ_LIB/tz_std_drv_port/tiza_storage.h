
#ifndef __TZ_STORAGE_H__
#define __TZ_STORAGE_H__


#include "stm32f4xx.h"
#include "stdbool.h"


/* 使能盲区数据补偿 */
#ifndef TZ_USE_STORAGE_MASS_DATA
#define TZ_USE_STORAGE_MASS_DATA                0

#if TZ_USE_STORAGE_MASS_DATA == 0
#define TZ_USE_STORAGE_WORKPARAM                0
#define TZ_USE_STORAGE_UNIVERSAL                0
#else
#define TZ_USE_STORAGE_UNIVERSAL                1   /* 默认支持通用工作参数 */
#define TZ_USE_STORAGE_WORKPARAM                0
#endif  /* TZ_USE_STORAGE_MASS_DATA == 1 */

#endif




#define __TZ_USE_FLASH__  2   /* FLASH type */


/* 定义程序参数存储类别 */
#define TZ_USE_STORAGE   __TZ_USE_FLASH__



#if TZ_USE_STORAGE == __TZ_USE_FLASH__
#define STORAGE_WriteData           FLASH_WriteData
#define STORAGE_ReadData            FLASH_ReadData
#define STORAGE_ObjectExist         FLASH_ObjectExist
#endif  /* __TZ_USE_FLASH__ */						    




/******************************************/
/*              外部函数[声明]            */
/******************************************/

/* 存储对象是否存在 */
extern u16  STORAGE_ObjectExist ( u16 ID );

/* 存储对象数据读取 */
extern bool STORAGE_ReadData ( u16 ID, u16 Len, u8 *ReadData );

/* 存储对象数据更新 */
extern bool STORAGE_WriteData ( u16 ID, u16 Len, u8 *WriteData );





#endif	/* __TZ_STORAGE_H__ */


