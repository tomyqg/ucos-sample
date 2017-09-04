#ifndef __SPI_FLASH_H
#define __SPI_FLASH_H

#include "stm32f4xx.h"
#include "os_cpu.h"
#include "gb_data_type.h"
#include "gb_data_type.h"
#include "stdbool.h"
/*调试模式*/
//////////////////////////////////////////////

#define  W25QXX_DMA               0


//////////////////////////////////////////////


//#ifdef REALDATA_UPLOAD_BUFFER_LEN

//#define EXFLASH_TX_BUFFER_LEN  REALDATA_UPLOAD_BUFFER_LEN
//#define EXFLASH_RX_BUFFER_LEN  REALDATA_UPLOAD_BUFFER_LEN
//#else
//#define EXFLASH_TX_BUFFER_LEN  1024
//#define EXFLASH_RX_BUFFER_LEN  1024
//#endif

//extern u8	TX_Buffer[EXFLASH_TX_BUFFER_LEN];
//extern u8 RX_Buffer[EXFLASH_RX_BUFFER_LEN];



/* Private typedef -----------------------------------------------------------*/

//W25q64相关使用配置
/*************************************************************************************/

#define SPI_FLASH_PageSize            	4096
#define SPI_FLASH_PerWritePageSize      4096

//第一个扇区的前8KB不使用
#define W25Q64_Start_Address            1024*32   

//W25Q64使用区结束地址
#define W25Q64_End_Address              8*1024*1024   //8M字节

//W25Q64 使用区大小
#define W25Q64_SafeArea_SIZE            W25Q64_End_Address-W25Q64_Start_Address
/*************************************************************************************/

/*备份参数*/

#define FLASH_VEHICLE_PARAMS 						8*1024

#define FLASH_GPRS_PARAMS 						  16*1024

#define FLASH_GPS_PARAMS 						20*1024

//盲区补偿，数据存储区32960
/*************************************************************************************/

 /*数据存储的配置地址*/
#define DATA_CONFIG_ADDRESS	 						W25Q64_Start_Address-1024*4

 /*数据存储的起始地址*/
#define DATA_STORAGE_ADDRESS	 				  W25Q64_Start_Address 

 /*数据存储的数据块大小*/
#define DATA_BLOCK_SIZE    							REALDATA_UPLOAD_BUFFER_LEN+24

 /*可存储数据量*/
#define MAX_DATA_STORAGE		     				W25Q64_SafeArea_SIZE/DATA_BLOCK_SIZE

/*************************************************************************************/




typedef struct FIFO_Type_FLASH  
{
	/*读取flash中是否有可读数据*/
		u16   offsetdata;	    //  0x8080:表示flash中已存储补发数据；!0x8080:flash中为存储可以补发的数据	初始化：0x0000
    bool  IsFinishSend;   //盲区发送完成
    u16   Depth;          // Fifo深度  
    u16   Head;           // Head为起始数据 
    u16   Tail;           // Tail-1为最后一个元素  
    u16   Counter;        // 数据计数器（数据条数） 
    u16   DataBytes;      // 每条数据的字节数  
 
}FIFO_FLASH_TYPE;

extern FIFO_FLASH_TYPE  FIFO_FlashBuf; //FIFO缓存
extern FIFO_FLASH_TYPE *pFlashBuf; //FIFO指针

extern void FIFO_FLASH_OffsetDataConfig_Clear(void);
extern u8 FIFO_FLASH_OffsetDataConfig_Read(u8* pdata,u16 len);
extern u8 FIFO_FLASH_OffsetDataConfig_Write(u8* pdata,u16 len);

extern void FIFO_flash_init(FIFO_FLASH_TYPE* pFIFO,u16 data_bytes,u16 flash_depth);
extern u8 FIFO_flash_IsEmpty(FIFO_FLASH_TYPE *pFIFO);
extern u8 FIFO_flash_AddOne(FIFO_FLASH_TYPE *pFIFO, void *pValue);
extern u8 FIFO_flash_GetOne(FIFO_FLASH_TYPE *pFIFO, void *pValue);
extern u8 FIFO_flash_IsFull(FIFO_FLASH_TYPE *pFIFO);

extern u8 STORAGE_FlashParams_WriteData(u8* pdata,u16 len,u32 address);
extern u8 STORAGE_FlashParams_ReadData(u8* pdata,u16 len,u32 address);

#endif 

