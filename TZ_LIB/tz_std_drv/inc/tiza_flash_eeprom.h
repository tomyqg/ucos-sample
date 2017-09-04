




#ifndef __EEPROM_H__
#define __EEPROM_H__


/* Includes */
#include <stdio.h>

#include "stm32f4xx.h"
#include "hardware_select_config.h"
#include "stdbool.h"
#include "stm32f4xx.h"



/******************************************/
/*             EEPROM参数[配置]           */
/******************************************/


typedef enum
{ 
  EEPROM_BUSY = 1,
  EEPROM_ERROR_PGS,
  EEPROM_ERROR_PGP,
  EEPROM_ERROR_PGA,
  EEPROM_ERROR_WRP,
  EEPROM_ERROR_PROGRAM,
  EEPROM_ERROR_OPERATION,
  EEPROM_COMPLETE,
/****************************/
  EEPROM_TIMEOUT,         /* EEPROM超时 */
  EEPROM_ERROR_ADDRESS,   /* EEPROM地址错误 */
  EEPROM_NO_VALID_PAGE,   /* EEPROM无效页面 */
	
/***************************/	
}EEPROM_Status;




//FLASH 起始地址
#define ADDR_FLASH_SECTOR_0     ((uint32)0x08000000) 	//扇区0, 16 Kbytes  
#define ADDR_FLASH_SECTOR_1     ((uint32)0x08004000) 	//扇区1, 16 Kbytes  
#define ADDR_FLASH_SECTOR_2     ((uint32)0x08008000) 	//扇区2, 16 Kbytes  
#define ADDR_FLASH_SECTOR_3     ((uint32)0x0800C000) 	//扇区3, 16 Kbytes  
#define ADDR_FLASH_SECTOR_4     ((uint32)0x08010000) 	//扇区4, 64 Kbytes  
#define ADDR_FLASH_SECTOR_5     ((uint32)0x08020000) 	//扇区5, 128 Kbytes  
#define ADDR_FLASH_SECTOR_6     ((uint32)0x08040000) 	//扇区6, 128 Kbytes  
#define ADDR_FLASH_SECTOR_7     ((uint32)0x08060000) 	//扇区7, 128 Kbytes  
#define ADDR_FLASH_SECTOR_8     ((uint32)0x08080000) 	//扇区8, 128 Kbytes  
#define ADDR_FLASH_SECTOR_9     ((uint32)0x080A0000) 	//扇区9, 128 Kbytes  
#define ADDR_FLASH_SECTOR_10    ((uint32)0x080C0000) 	//扇区10,128 Kbytes  
#define ADDR_FLASH_SECTOR_11    ((uint32)0x080E0000) 	//扇区11,128 Kbytes  





/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* DMA Stream parameters definitions. You can modify these parameters to select
   a different DMA Stream and/or channel.
   But note that only DMA2 Streams are capable of Memory to Memory transfers */
#define DMA_STREAM               DMA2_Stream0
#define DMA_CHANNEL              DMA_Channel_0
#define DMA_STREAM_CLOCK         RCC_AHB1Periph_DMA2 
#define DMA_STREAM_IRQ           DMA2_Stream0_IRQn
#define DMA_IT_TCIF              DMA_IT_TCIF0
#define DMA_STREAM_IRQHANDLER    DMA2_Stream0_IRQHandler

#define BUFFER_SIZE              32
#define TIMEOUT_MAX              10000 /* Maximum timeout value */









/* Device voltage range supposed to be [2.7V to 3.6V], the operation will 
   be done by word  */
#define VOLTAGE_RANGE           (uint8_t)VoltageRange_3

/* EEPROM emulation max data store space */
#define EEPROM_MAX_SIZE					( ( u32 )( EEPROM_END_ADDRESS - EEPROM_START_ADDRESS ) )

/* Define the number of page to be erased */
#define EEPROM_PAGE_NUMBER      ( ( u32 )EEPROM_MAX_SIZE / PAGE_SIZE )

/* Valid pages in read and write defines */
#define READ_FROM_VALID_PAGE    ( ( u8 )0x00 )
#define WRITE_IN_VALID_PAGE     ( ( u8 )0x01 )








/******************************************/
/*              外部函数[声明]            */
/******************************************/

extern EEPROM_Status EEPROM_IfInit ( void );
extern EEPROM_Status EEPROM_FormatPage ( u32 EraseVirtAddress );
extern EEPROM_Status EEPROM_FormatAllPages ( void );
extern EEPROM_Status EEPROM_WriteData ( u32 VirtAddress, u8 Data[], u32 DataLen );
extern EEPROM_Status EEPROM_ReadData ( u32 VirtAddress, u8 Data[], u16 DataLen );
extern u32 EEPROM_FindValidPage ( u8 Operation );
extern u16 EEPROM_ReadVariable ( u32 VirtAddress, u16* Data );
extern u16 EEPROM_WriteVariable ( u32 VirtAddress, u16 Data );

extern u16 FLASH_GetFlashSectorID(u32 addr);


#endif /* __EEPROM_H__ */



