#ifndef __SPI_FLASH_H
#define __SPI_FLASH_H

#include "stm32f4xx.h"
#include "os_cpu.h"
#include "gb_data_type.h"
#include "gb_data_type.h"
#include "stdbool.h"
/*����ģʽ*/
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

//W25q64���ʹ������
/*************************************************************************************/

#define SPI_FLASH_PageSize            	4096
#define SPI_FLASH_PerWritePageSize      4096

//��һ��������ǰ8KB��ʹ��
#define W25Q64_Start_Address            1024*32   

//W25Q64ʹ����������ַ
#define W25Q64_End_Address              8*1024*1024   //8M�ֽ�

//W25Q64 ʹ������С
#define W25Q64_SafeArea_SIZE            W25Q64_End_Address-W25Q64_Start_Address
/*************************************************************************************/

/*���ݲ���*/

#define FLASH_VEHICLE_PARAMS 						8*1024

#define FLASH_GPRS_PARAMS 						  16*1024

#define FLASH_GPS_PARAMS 						20*1024

//ä�����������ݴ洢��32960
/*************************************************************************************/

 /*���ݴ洢�����õ�ַ*/
#define DATA_CONFIG_ADDRESS	 						W25Q64_Start_Address-1024*4

 /*���ݴ洢����ʼ��ַ*/
#define DATA_STORAGE_ADDRESS	 				  W25Q64_Start_Address 

 /*���ݴ洢�����ݿ��С*/
#define DATA_BLOCK_SIZE    							REALDATA_UPLOAD_BUFFER_LEN+24

 /*�ɴ洢������*/
#define MAX_DATA_STORAGE		     				W25Q64_SafeArea_SIZE/DATA_BLOCK_SIZE

/*************************************************************************************/




typedef struct FIFO_Type_FLASH  
{
	/*��ȡflash���Ƿ��пɶ�����*/
		u16   offsetdata;	    //  0x8080:��ʾflash���Ѵ洢�������ݣ�!0x8080:flash��Ϊ�洢���Բ���������	��ʼ����0x0000
    bool  IsFinishSend;   //ä���������
    u16   Depth;          // Fifo���  
    u16   Head;           // HeadΪ��ʼ���� 
    u16   Tail;           // Tail-1Ϊ���һ��Ԫ��  
    u16   Counter;        // ���ݼ����������������� 
    u16   DataBytes;      // ÿ�����ݵ��ֽ���  
 
}FIFO_FLASH_TYPE;

extern FIFO_FLASH_TYPE  FIFO_FlashBuf; //FIFO����
extern FIFO_FLASH_TYPE *pFlashBuf; //FIFOָ��

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

