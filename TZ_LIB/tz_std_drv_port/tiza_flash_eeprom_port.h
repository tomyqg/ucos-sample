









#ifndef __FLASH_H__
#define __FLASH_H__



#include "stm32f4xx_flash.h"

#include "stdbool.h"



/******************************************/
/*             FLASH参数[配置]            */
/******************************************/




/******************************************/
/*              外部函数[声明]            */
/******************************************/

/* FLASH存储对象是否存在 */
extern u16  FLASH_ObjectExist ( u16 ID );

/* FLASH存储对象数据读取 */
extern bool FLASH_ReadData ( u16 ID, u16 Len, u8 *ReadData );

/* FLASH存储对象数据写入 */
extern bool FLASH_WriteData ( u16 ID, u16 Len, u8 *WriteData );

/* 擦除全部FALSH存储对象 */
extern bool FLASH_EraseAll ( void );

/* 擦除特定FLASH存储对象 */
extern bool FLASH_EraseObject ( u16 ID );











































































#ifndef __USE_FLASH__
#define __USE_FLASH__		  1
#endif  /* __USE_FLASH__ */


/******************************************/
/*            低级接口函数[声明]          */
/******************************************/

/*  */
extern void FLASH_LowlLevelIfInit ( void );


#endif  /* __FLASH_H__ */

