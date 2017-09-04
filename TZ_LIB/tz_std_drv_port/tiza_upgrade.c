
#include "tiza_upgrade.h"
#include "tiza_init.h"
#include "hardware_select_config.h"
#include "stm32f4xx_conf.h"
#include "tiza_flash_eeprom.h"
#include "tiza_wdg.h"
#include "string.h"
	enum{
		FALSE = 0,
		TRUE = !FALSE
	};




void UPGRADE_State_Display( void );
	
/* 16bit addresses data */
#define DATA_16BIT(_X_)           ( * ( u16 *)(_X_) ) 

/* 32bit addresses data */
#define DATA_32BIT(_X_)           ( * ( u32 *)(_X_) )

/* ����������� */
#define MAX_IMAGE_NAME_LENGTH     32	/* ��λ��byte */


/* System initialisation type */ 
typedef enum
{
  INIT_POWER_ON,              /* �����ϵ� */ 
  INIT_PIN_RST,               /* �ܽŸ�λ */
  INIT_SFT_RST,               /* �����λ */
  INIT_WDG_RST,               /* ���Ź���λ */ 
  INIT_LOWPWR_RST             /* �͹��ĸ�λ */
} InitType_e;



typedef struct
{
  /* �Ƿ������� */
  u32 isUpgrade;
  /* 0xAA ��ʾ��������
     0x55 ��ʾ�������� */

	/* �������С */
  u32 imageSize;
	
  /* ������� */
  u32 upgradeResult;
  /* 0xAA ��ʾ�����ɹ�
     0x55 ��ʾ����ʧ�� */
	


  /* �������� */
  ascii program[ MAX_IMAGE_NAME_LENGTH ];



  /* CRCУ�� */
  u16 fileCRC;

  /* ����λԭ�� */
  InitType_e sysResetCause;
} UPGRADE_DevTypedef;



typedef struct 
{
  /* RAM���ݲ�����ַ */
  vu32 Ram_address;

  /* FLASH���ݲ�����ַ */
  vu32 Flash_address;

  /* ���ݴ��䳤�� */
  u32 Buffer_Transfer_Size;
} APPFLASH_DevTypedef;



/******************************************/
/*              �ⲿ����[����]            */
/******************************************/

sys_InitType_e sysInitType = SYS_INIT_POWER_ON;





/******************************************/
/*              �ڲ�����[����]            */
/******************************************/

#define UPGRADE_PAGA_SIZE   1024*2

/* ҳ�洫�仺�� */
static u8  pageTransfer[ UPGRADE_PAGA_SIZE + 1024 ];

/* ҳ��д�볤�� */
static u32 pageWriteLen;

/* ҳ��д���ܳ��� */
static u32 pageWriteTotalLength;

/* �����ַ */
static u32 ApplicationAddress;

/* ������Ҫ��ҳ���� */
static u32 ApplicationPageNumber;

/* ���������� */
static UPGRADE_DevTypedef upgrader = { 0, 0, 0, "", 0, INIT_POWER_ON };

/* ��д���� */
static APPFLASH_DevTypedef adflash = { 0, 0, 0 };





extern bool CheckAppStackOverFlow(void);
extern bool CheckUsartStackOverFlow(void);
extern bool CheckTMRFastStackOverFlow(void);
extern bool CheckTMRSlowStackOverFlow(void);
extern bool CheckWorkStackOverFlow(void);
extern bool CheckRtcStackOverFlow(void);



/******************************************/
/*              �ڲ�����[����]            */
/******************************************/

static FLASH_Status UPGRADE_FormatApplicationRegions ( u32 PageEraseAddress, u16 PageNumber );
static FLASH_Status APPFLASH_WriteData ( u32 WriteAddress, u8 Data[], u32 DataLen );
static FLASH_Status APPFLASH_ReadData ( u32 ReadAddress, u8 Data[], u16 DataLen );

 bool UPGRADE_GetUpgradeConfigure ( void );
 bool UPGRADE_SetUpgradeConfigure ( void );
static void UPGRADE_QueryUpgradeResult ( void );
static u16  UPGRADE_CRCFileCalculate ( u32 imageSize );
static void UPGRADE_Finalise ( void );
static bool BufferCmp ( uc8 *pBuffer1, u8 *pBuffer2, u16 BufferLength );














/* 
 * ����������FLASH NVIC����
 * ���ò�������
 *          
 * ����ֵ  ����
 * 
 */
extern void UPGRADE_LowLevelIfInit ( void )
{
  /* Enable DMA1 clock */
	RCC_AHB1PeriphClockCmd(DMA_STREAM_CLOCK, ENABLE);

  /* Unlock the Flash Program Erase controller */
  FLASH_Unlock ();

  /* Clear All pending flags */
  FLASH_ClearFlag ( FLASH_FLAG_BSY|FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR|FLASH_FLAG_PGPERR|FLASH_FLAG_PGSERR );
	
  UPGRADE_GetUpgradeConfigure ();
  UPGRADE_QueryUpgradeResult ();
	
	
}









/* 
 * ����������FLASH NVIC����
 * ���ò�������
 *          
 * ����ֵ  ����
 * 
 */
static void UPGRADE_QueryUpgradeResult ( void )
{
  if ( upgrader.upgradeResult == 0xAA && upgrader.imageSize > 0 )
  {
    sysInitType = SYS_INIT_DOWNLOAD_SUCCESS;  
  }
  else if ( upgrader.upgradeResult == 0x55 && upgrader.imageSize > 0 )
  {
    sysInitType = SYS_INIT_DOWNLOAD_ERROR; 
  }
  else
  {
    if ( upgrader.sysResetCause == INIT_WDG_RST ) 
    {
      sysInitType = SYS_INIT_REBOOT_FROM_EXCEPTION; 
    }
    else
    {
      sysInitType = SYS_INIT_POWER_ON; 
    }
  }




/////////////////////////////////////////////////////////////////
//          �˴��������Ҫ����ֹBOOT�����ظ���д */            //
/////////////////////////////////////////////////////////////////

  if ( upgrader.isUpgrade == 0xAA || upgrader.upgradeResult == 0xAA )
  {
    upgrader.isUpgrade = 0x55;
    upgrader.upgradeResult = 0x55;
    upgrader.imageSize = 0;
    UPGRADE_SetUpgradeConfigure ();  
  }
/////////////////////////////////////////////////////////////////

}







/* 
************************************************************************
 *������	��UPGRADE_Start
 *����		�����¹̼�ǰ���Թ̼�flash�Ĵ洢����в�����Ϊflash��д��BIN��׼��         
 *����ֵ 	��FLASH_COMPLETE�������ɹ�  FLASH_TIMEOUT����ʱ 
 *������  	��gl
 ************************************************************************
 */
extern bool UPGRADE_Start ( void )
{
	FLASH_Status res;
  pageWriteLen = 0;
  pageWriteTotalLength = 0;
  ApplicationAddress =APPLICATION_STORE_ADDRESS;
  ApplicationPageNumber=APP_STORE_PAGE_NUM;
	
	printf("\r\n  ApplicationPageNumber: %x \r\n",ApplicationPageNumber);

	res=UPGRADE_FormatApplicationRegions(ApplicationAddress, ApplicationPageNumber );

	printf("\r\n  FLASH_Status: %x \r\n",res);
  return ( res == FLASH_COMPLETE ) ? TRUE : FALSE;
}








/* 
 * ��������: ��ȡ��������������Ϣ
 * ���ò���:
 *          
 * ����ֵ  :
 * 
 */
 bool UPGRADE_GetUpgradeConfigure ( void )
{
	FLASH_Status res;
  //UPGRADE_State_Display();
	res=APPFLASH_ReadData ( APPLICATION_UPGRADE_CONFIG_ADDRESS,( u8 * )&upgrader, sizeof( UPGRADE_DevTypedef ));
	//UPGRADE_State_Display();
	return ( FLASH_COMPLETE == res) ? TRUE : FALSE;

}



void UPGRADE_State_Display( void )
{
	printf("\r\n fileCRC: %d  imageSize: %d  isUpgrade:%d  program:%s  sysResetCause:%d  upgradeResult:%d \r\n",
	upgrader.fileCRC,upgrader.imageSize,upgrader.isUpgrade ,upgrader.program,upgrader.sysResetCause,upgrader.upgradeResult);
}


/* 
**********************************************************************
*
 * ������  : UPGRADE_SetUpgradeConfigure
 * ����    : �Թ̼�������־λ��ʽ�������0xFF����д��ṹ��upgrader�е�������Ϣ        
 * ����ֵ  : 1��flash��д��������Ϣ�ɹ���0��flash��д��������Ϣʧ��
 * ������  ��gl
 *************************************************************************
 */
 bool UPGRADE_SetUpgradeConfigure ( void )
{

  UPGRADE_FormatApplicationRegions( APPLICATION_UPGRADE_CONFIG_ADDRESS,1);

  return ( FLASH_COMPLETE == APPFLASH_WriteData (APPLICATION_UPGRADE_CONFIG_ADDRESS, 
         ( u8 * )&upgrader, sizeof( UPGRADE_DevTypedef ) ) ) ? TRUE : FALSE;
}







/* 
 * ������������ʽ������洢����
 * ���ò�������
 *          
 * ����ֵ  ����
 * 
 */
static FLASH_Status UPGRADE_FormatApplicationRegions ( u32 PageEraseAddress, u16 PageNumber )
{
  FLASH_Status FlashStatus = FLASH_GetStatus ();
  u32 EraseCounter;
	u16 Sector_id=0;
  if ( FlashStatus != FLASH_COMPLETE )
  {
		FLASH_ClearFlag ( FLASH_FLAG_BSY|FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR|FLASH_FLAG_PGPERR|FLASH_FLAG_PGSERR );
    FlashStatus = FLASH_WaitForLastOperation ();
  }

	Sector_id=FLASH_GetFlashSectorID(PageEraseAddress);
  for ( EraseCounter = 0; ( EraseCounter < PageNumber )&&( FlashStatus == FLASH_COMPLETE ); EraseCounter++ )
  {
		WDG_KickCmd (); 
    /* Erase the FLASH pages */
    FlashStatus = FLASH_EraseSector((Sector_id + ( EraseCounter * 0x08 ) ),VOLTAGE_RANGE);

    /* If erase operation was failed, a Flash error code is returned */
    if ( FlashStatus != FLASH_COMPLETE )
    {
      return FlashStatus;
    }    
  }

  /* If program operation was completed */
  return FLASH_COMPLETE;
}







/* 
 * ����������FLASH NVIC����
 * ���ò�������
 *          
 * ����ֵ  ����
 * 
 */
extern void UPGRADE_InstallApplication ( void )
{

  UPGRADE_Finalise ();

  if ( UPGRADE_SetUpgradeConfigure () == TRUE )
  {
		
		printf("\r\n upgrader.isUpgrade: %x \r\n",upgrader.isUpgrade);

#if SYSTEM_RESTART_DEBUG == 1
    printf( "\r\n[SYS RESET] upgrade type\r\n" );
#endif  /* SYSTEM_RESTART_DEBUG */


#if 0
		printf("FtpToFlash_DataCount:0x%x\r\n",FtpToFlash_DataCount);
	   
		for(count=0;count<(FtpToFlash_DataCount/2);count++)
		{
			printf("0x%02x\t",*(u16*)(APPLICATION_STORE_ADDRESS + 2*count));
		}
		printf("\r\nBIN�ļ���ӡ���\r\n");
#else
		printf("\r\n�̼�������� --> ׼����λ\r\n");
    System_Reset();
#endif
  } 
}












/* 
 * ����������FLASH NVIC����
 * ���ò�������
 *          
 * ����ֵ  ����
 * 
 */
static u16 UPGRADE_CRCFileCalculate ( u32 imageSize )
{
	u32 i = 0, m = 0, n = 0;
	u16 crc16_data = 0;
	u32 read_flash_destination;
	union 
	{
		u32 u32_data;
		u8 buff[ 4 ];
	} long_data;
	
	read_flash_destination = APPLICATION_STORE_ADDRESS;	
	for ( i = 0; i < imageSize; )
	{
		long_data.u32_data = DATA_32BIT ( read_flash_destination );
		read_flash_destination += 4;

		for ( n = 0; n < 4; n++ )
		{
			if ( i >= upgrader.imageSize )
			{
				break;
			}
      else
      {
        i++;
      }
			crc16_data = crc16_data ^ ( u16 )( long_data.buff[ n ] << 8 );
			for ( m = 0; m < 8; m++ ) 
			{
				if ( crc16_data & 0x8000 ) 
        {
          crc16_data = crc16_data << 1 ^ 0x1021;
        }
				else 
        {
          crc16_data = crc16_data << 1;
        }
			}
		}
	}

  return crc16_data;
}
















/* 
******************************************************************************
 * ������	��UPGRADE_Finalise
 * ����		���Խṹ��upgrader���ã����ù̼�������Ϣ      
 * ����ֵ  	����
 * ������	��gl
 ******************************************************************************
 */
static void UPGRADE_Finalise ( void )
{
  if ( pageWriteLen > 0 )
  {
    APPFLASH_WriteData ( ApplicationAddress, pageTransfer, pageWriteLen );
  } 

  /* ����������Ϣ */ 
//  upgrader.isUpgrade = 0xAA;
//  upgrader.upgradeResult = 0x55;
//  upgrader.imageSize = pageWriteTotalLength;
//  upgrader.fileCRC = UPGRADE_CRCFileCalculate( pageWriteTotalLength );
	
  upgrader.isUpgrade =0xAAAAAAAA;
  upgrader.upgradeResult = 0x55;
  upgrader.imageSize = pageWriteTotalLength;
  upgrader.fileCRC = UPGRADE_CRCFileCalculate( pageWriteTotalLength );
	
}






/* 
**********************************************************************
 * ������	��UPGRADE_WriteData
 * ����		����FTP���صĻ������ݣ�����APP��flash��ַ        
 * ����ֵ  	����
 * ������	��gl
 ************************************************************************
 */

      

extern bool UPGRADE_WriteData ( u8 Data[], u32 DataLen )
{
  if ( Data && DataLen > 0 )
  {    
    memcpy ( ( u8* )&pageTransfer[ pageWriteLen ], Data, DataLen );
    pageWriteLen += DataLen;
    pageWriteTotalLength += DataLen;
    WDG_KickCmd();
    if ( pageWriteLen >= UPGRADE_PAGA_SIZE )
    {
      u8 *PagePointer = pageTransfer;
       
			APPFLASH_WriteData ( ApplicationAddress, pageTransfer, UPGRADE_PAGA_SIZE );

      /* Check if the transmitted and received data are equal */
      if (!BufferCmp ( ( uc8 *)ApplicationAddress, ( u8 *)pageTransfer, UPGRADE_PAGA_SIZE ) )
      { 
        return FALSE;
      }

      ApplicationAddress += UPGRADE_PAGA_SIZE; 
      pageWriteLen       -= UPGRADE_PAGA_SIZE;
      memcpy ( PagePointer, &pageTransfer[ UPGRADE_PAGA_SIZE ], pageWriteLen );
    }
    return TRUE;
  }
  else
  {
    return FALSE;
  }
}









/* 
 * ����������FLASH���ݱȽ�
 * ���ò�������
 *          
 * ����ֵ  ����
 * 
 */
static FLASH_Status APPFLASH_WriteData ( u32 WriteAddress, u8 Data[], u32 DataLen )
{
  FLASH_Status FlashStatus = FLASH_COMPLETE;

  /* Get active Page for write operation */
  if ( WriteAddress >= APPLICATION_STORE_ADDRESS )
  {
	  /* Check if there is no valid page */
    if ( Data && DataLen > 0 )
    {
      u32 i;
        
  	  /* Get the valid Page end Address */
      adflash.Flash_address = WriteAddress;
      adflash.Ram_address = ( u32 )Data;
      adflash.Buffer_Transfer_Size = DataLen;

#if UPGRADE_DEBUG == 1
      printf ( "\r\nMain program flash address %X RAM address %X size %u",
               adflash.Flash_address, 
               adflash.Ram_address, 
               adflash.Buffer_Transfer_Size  );
#endif  /* UPGRADE_DEBUG */ 
      

    	for ( i = 0; i < adflash.Buffer_Transfer_Size; i += 2 )
    	{
    		FlashStatus = FLASH_ProgramHalfWord ( adflash.Flash_address, DATA_16BIT ( adflash.Ram_address ) );
        if ( FlashStatus == FLASH_COMPLETE )
        {
						/* Check the corectness of written data */
						if ( DATA_16BIT( adflash.Flash_address ) != DATA_16BIT( adflash.Ram_address ) )
						{
							return FLASH_ERROR_PGP;
						}
						else
						{
							adflash.Flash_address += 2;
							adflash.Ram_address   += 2;
						}
        }
        else
        {
          return FlashStatus;
        }
    	}        
    }
    else
    {
      FlashStatus = FLASH_ERROR_PGP;
    }
  }

  return FlashStatus;
}






/* 
********************************************************************
 * ������	��BufferCmp
 * ����		����FTP��������������Flash�����ݽ��бȽ�          
 * ����ֵ  	��1��OK  0��error
 * ������	��gl
 *************************************************************************
 */
static bool BufferCmp ( uc8 *pBuffer1, u8 *pBuffer2, u16 BufferLength )
{

#if UPGRADE_DEBUG == 1
  u16 dataCounter = 0;
#endif  /* UPGRADE_DEBUG */ 

  while ( BufferLength > 0 )
  {
    if ( *pBuffer1 != *pBuffer2 )
    {

#if UPGRADE_DEBUG == 1
      printf ( "\r\nError found at %u byets", dataCounter );
#endif  /* UPGRADE_DEBUG */ 
      
      return FALSE;
    }
    else
    {
      pBuffer1++;
      pBuffer2++;
      BufferLength--; 

#if UPGRADE_DEBUG == 1
      dataCounter++;
#endif  /* UPGRADE_DEBUG */ 
 
    }
  }

  return TRUE;  
}



/* 
 * ����������FLASH���ݱȽ�
 * ���ò�������
 *          
 * ����ֵ  ����
 * 
 */
static FLASH_Status APPFLASH_ReadData ( u32 ReadAddress, u8 Data[], u16 DataLen )
{
    FLASH_Status FlashStatus = FLASH_COMPLETE;

  /* Get active Page for read operation */
  if ( ReadAddress >= APPLICATION_STORE_ADDRESS )
  {
    if ( Data && DataLen > 0 )
    {
  	  /* Get the valid Page end Address */
      adflash.Flash_address = ReadAddress;
      adflash.Ram_address = ( u32 )Data;
      adflash.Buffer_Transfer_Size = DataLen;
  
      /* Enable DMA1 Channel6 Transfer Complete/error interrupt */
      {
      	DMA_InitTypeDef  DMA_InitStructure;
      
        /* DMA1 channel6 configuration */
        DMA_DeInit(DMA_STREAM);
        DMA_InitStructure.DMA_PeripheralBaseAddr = ( u32 )adflash.Flash_address;
        DMA_InitStructure.DMA_Memory0BaseAddr = ( u32 )adflash.Ram_address;
				
        DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToMemory;
        DMA_InitStructure.DMA_BufferSize = adflash.Buffer_Transfer_Size;
        DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Enable;
        DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
        DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
        DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
        DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
        DMA_InitStructure.DMA_Priority = DMA_Priority_High;

				DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;         
				DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
				DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
				DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
							
        DMA_Init ( DMA_STREAM, ( DMA_InitTypeDef* )&DMA_InitStructure );

        /* Enable DMA1 Channel6 transfer */
        DMA_Cmd ( DMA_STREAM, ENABLE );
      }
    
      /* Wait the end of transmission */
			__IO uint32_t    Timeout = TIMEOUT_MAX;
      while ((DMA_GetCmdStatus(DMA_STREAM) != ENABLE) && (Timeout-- > 0))
			{		
			
			}   
			/* Check if a timeout condition occurred */
			if (Timeout == 0)
			{
				/* Manage the error: to simplify the code enter an infinite loop */
				while (1)
				{
					printf("\r\n  DMA_GetCmdStatus  Error \r\n");
					break;
				}
			}
      
      /* Check if the transmitted and received data are equal */
      if ( !BufferCmp ( ( uc8 *)adflash.Flash_address, ( u8 *)adflash.Ram_address, adflash.Buffer_Transfer_Size ) )
      {
        FlashStatus = FLASH_ERROR_PGP; 
      }    
    }
    else
    {
      FlashStatus = FLASH_ERROR_PGP;
    }
  }

  return FlashStatus;
}


