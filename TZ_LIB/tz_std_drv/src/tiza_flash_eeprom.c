
#include "tiza_flash_eeprom.h"
#include "stm32f4xx_flash.h"
#include "tiza_wdg.h"




	enum{
		FALSE = 0,
		TRUE = !FALSE
	};







/////////////////////////////////////////////////////////


#define EEPROM_DBG_NORMAL     0x01
#define EEPROM_DBG_WARNING    0x02

/* 定义EEPROM调试 */
#define EEPROM_DEBUG        0

/////////////////////////////////////////////////////////









#if EEPROM_END_ADDRESS < ( EEPROM_START_ADDRESS + PAGE_SIZE )
#error ***ERROR EEPROM ADDRESS CONFIGURE***
#else

/* Pages base addresses */
#define PAGE_BASE_ADDRESS(_X_)    ( ( u32 )( EEPROM_START_ADDRESS + (_X_) * PAGE_SIZE ) )

/* Pages end addresses */
#define PAGE_END_ADDRESS(_X_)     ( ( u32 )( EEPROM_START_ADDRESS + ( (_X_) + 1 ) * PAGE_SIZE - 1 ) )

/* Pages virtual addresses */
#define PAGE_VIRT_ADDRESS(_X_)    ( ( u32 )( (_X_) * PAGE_SIZE ) )

/* data contents virtual addresses */
#define DATA_VIRT_ADDRESS(_X_)    ( ( u32 )( (_X_) * PAGE_SIZE ) + 2 )

/* virtual addresse map to physical address */
#define VIRT_MAP(_X_)             ( ( u32 )( EEPROM_START_ADDRESS + (u32)(_X_) ) )

/* 8bit addresses data */
#define DATA_8BIT(_X_)            ( * ( u8  *)(_X_) )

/* 16bit addresses data */
#define DATA_16BIT(_X_)           ( * ( u16 *)(_X_) )

/* 32bit addresses data */
#define DATA_32BIT(_X_)           ( * ( u32 *)(_X_) )

/* Page status definitions */
/* PAGE is empty */
#define ERASED                    ( ( u16 )0xFFFF ) 

/* PAGE is marked to receive data */
#define RECEIVE_DATA              ( ( u16 )0x0000 )  

/* PAGE containing valid data */
#define VALID_PAGE                ( ( u16 )0xEEEE ) 
#endif  /* EEPROM_END_ADDRESS */



enum { 
  EEPROM_DIR_SRC = ( u32 )0x00,   /* 读FLASH */
  EEPROM_DIR_DST = ( u32 )0x40    /* 写FLASH */
};


typedef struct 
{
  /* 页面使用状态 */
  u16 PageStatus[ EEPROM_PAGE_NUMBER ];

  /* 页面写指针 */
  u32 WritePageIndex;

  /* 页面读指针 */
  u32 ReadPageIndex;

  /* RAM数据操作地址 */
  vu32 RAM_ADDRESS;

  /* FLASH数据操作地址 */
  vu32 FLASH_ADDRESS;

  /* 数据传输长度 */
  s32 Buffer_Transfer_Size;
} EEPROM_DevTypedef;






/******************************************/
/*              内部变量[定义]            */
/******************************************/

static EEPROM_DevTypedef eeprom = { {0}, 0, 0, 0, 0, 0 };





/******************************************/
/*              内部函数[声明]            */
/******************************************/

static void FLASH_DMAConfig ( u32 eepromDir );
static EEPROM_Status BufferCmp ( uc8 * pBuffer1, u8 * pBuffer2, s16 BufferLength );





//获取扇区编号
extern u16 FLASH_GetFlashSectorID(u32 addr)
{
	if(addr < ADDR_FLASH_SECTOR_1)return FLASH_Sector_0;
	else if(addr<ADDR_FLASH_SECTOR_2)return FLASH_Sector_1;
	else if(addr<ADDR_FLASH_SECTOR_3)return FLASH_Sector_2;
	else if(addr<ADDR_FLASH_SECTOR_4)return FLASH_Sector_3;
	else if(addr<ADDR_FLASH_SECTOR_5)return FLASH_Sector_4;
	else if(addr<ADDR_FLASH_SECTOR_6)return FLASH_Sector_5;
	else if(addr<ADDR_FLASH_SECTOR_7)return FLASH_Sector_6;
	else if(addr<ADDR_FLASH_SECTOR_8)return FLASH_Sector_7;
	else if(addr<ADDR_FLASH_SECTOR_9)return FLASH_Sector_8;
	else if(addr<ADDR_FLASH_SECTOR_10)return FLASH_Sector_9;
	else if(addr<ADDR_FLASH_SECTOR_11)return FLASH_Sector_10; 
	return FLASH_Sector_11;	
}



/* 
 * 功能描述：Restore the pages to a known good state in case of page's status
 *           corruption after a power loss
 * 引用参数：无
 *          
 * 返回值  ：- Flash error code: on write Flash error
 *           - FLASH_COMPLETE: on success
 * 
 */
extern EEPROM_Status EEPROM_IfInit ( void )
{
  u16 PageIndex;

#if EEPROM_DEBUG == 1
		printf ( "\r\n\r\n[E2PROM] init start\r\n" );
#endif  /* EEPROM_DEBUG */

  /* Enable DMA1 clock */
  RCC_AHB1PeriphClockCmd ( RCC_AHB1Periph_DMA1, ENABLE );

  /* configuration Flash Program NVIC controller */
//  FLASH_NvicConfig ();

  /* Unlock the Flash Program Erase controller */
  FLASH_Unlock ();


  /* Clear All pending flags */
  FLASH_ClearFlag ( FLASH_FLAG_BSY|FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR|FLASH_FLAG_PGPERR|FLASH_FLAG_PGSERR );


#if EEPROM_DEBUG == 1
		printf ( "\r\npage ( ID  PHY-ADDR ) STAT\r\n" );
#endif  /* EEPROM_DEBUG */

  for( PageIndex = 0; PageIndex < EEPROM_PAGE_NUMBER; PageIndex++ )
  {
    /* Get Page status */
    eeprom.PageStatus[ PageIndex ] = DATA_16BIT( PAGE_BASE_ADDRESS ( PageIndex ) );  
    
#if EEPROM_DEBUG == 1
		printf ( "page ( %2d 0x%X ) stat 0x%X\r\n", 
             PageIndex, 
             PAGE_BASE_ADDRESS ( PageIndex ), 
             eeprom.PageStatus[ PageIndex ] );
#endif  /* EEPROM_DEBUG */

    /* Check for invalid header states and repair if necessary */
    switch ( eeprom.PageStatus[ PageIndex ] )
    {
      case ERASED:  /* PAGE is empty */
        break;
  
      case RECEIVE_DATA:  /* PAGE is marked to receive data */
        break;
  
      case VALID_PAGE:  /* PAGE containing valid data */
        break;
  
      default:  /* Any other state -> format eeprom */
      {
          /* Erase this Page and set as valid page */
          EEPROM_Status FlashStatus = EEPROM_FormatPage ( PAGE_VIRT_ADDRESS ( PageIndex ) );

          /* If erase/program operation was failed, a Flash error code is returned */
          if ( FlashStatus != EEPROM_COMPLETE )
          {
            return FlashStatus;
          }
      }
      break;
    }
  }

  return EEPROM_COMPLETE;
}








/* 
 * 功能描述：Erases PAGE X and writes VALID_PAGE header to PAGE X
 * 引用参数：无
 *          
 * 返回值  ：Status of the last operation (Flash write or erase) done during
 *           EEPROM formating
 * 
 */
extern EEPROM_Status EEPROM_FormatPage ( u32 EraseVirtAddress )
{
  EEPROM_Status FlashStatus = EEPROM_COMPLETE;

  u32 Address;
  u16 pageIdx;
	u8  Sector_id;
  #define EepromEraseTimeout             ((u32)0x00000FFF)
  bool retry = FALSE;

#if EEPROM_DEBUG == 1
  printf ( "\r\n\r\n[E2PROM] do format page, " );
#endif  /* EEPROM_DEBUG */

  /* Check the Address of the page to be erased */
  if ( EraseVirtAddress >= EEPROM_MAX_SIZE )
  {
#if EEPROM_DEBUG == 1
    printf ( "virt addr 0x%X error***\r\n", EraseVirtAddress );
#endif  /* EEPROM_DEBUG */
    return ( EEPROM_ERROR_ADDRESS );
  }

__RETRY_FROAMT_LABLE__:
  FLASH_ClearFlag ( FLASH_FLAG_BSY|FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR|FLASH_FLAG_PGPERR|FLASH_FLAG_PGSERR );


  FLASH_WaitForLastOperation();

  /* Erase Page */
  pageIdx = EraseVirtAddress / PAGE_SIZE;
  Address = PAGE_BASE_ADDRESS( pageIdx );
  Sector_id=FLASH_GetFlashSectorID(Address);

  WDG_KickCmd ();
	FlashStatus=(EEPROM_Status)FLASH_EraseSector(Sector_id,VOLTAGE_RANGE);
  WDG_KickCmd ();
#if EEPROM_DEBUG == 1
  printf ( "format page ( %2d 0x%X ) result 0x%X,", 
           pageIdx, 
           Address, 
           FlashStatus );
#endif  /* EEPROM_DEBUG */

  /* If erase operation was failed, a Flash error code is returned */
  if ( FlashStatus != EEPROM_COMPLETE )
  {
    if ( retry == TRUE )
    {
#if EEPROM_DEBUG == 1
      printf ( "error erase***\r\n" );
#endif  /* EEPROM_DEBUG */
      return FlashStatus;    
    }
    else
    {
      retry = TRUE;
      goto __RETRY_FROAMT_LABLE__;  
    }
  }
  else
  {
#if EEPROM_DEBUG == 1
    printf ( " stat ERASED,\t" );
#endif  /* EEPROM_DEBUG */
    eeprom.PageStatus[ pageIdx ] = ERASED;
  }

  /* Set Page as valid page: Write VALID_PAGE at Page X base address */
  FlashStatus = ( EEPROM_Status )FLASH_ProgramHalfWord ( Address, VALID_PAGE );
  if ( FlashStatus != EEPROM_COMPLETE )
  {
#if EEPROM_DEBUG == 1
    printf ( "error foramt***\r\n" );
#endif  /* EEPROM_DEBUG */
    return FlashStatus;
  }
  else
  {
#if EEPROM_DEBUG == 1
    printf ( " stat VALID_PAGE,\t" );
#endif  /* EEPROM_DEBUG */
    eeprom.PageStatus[ pageIdx ] = VALID_PAGE;
  }

#if EEPROM_DEBUG == 1
  printf ( "EEPROM_COMPLETE\r\n" );  
#endif  /* EEPROM_DEBUG */


  /* If program operation was failed, a Flash error code is returned */
  return FlashStatus;
}








/* 
 * 功能描述：
 * 引用参数：无
 *          
 * 返回值  ：无
 * 
 */
extern EEPROM_Status EEPROM_FormatAllPages ( void )
{
  EEPROM_Status FlashStatus = EEPROM_COMPLETE;
  u32 EraseCounter;
	u16 Sector_id;
  #define EepromEraseTimeout             ((u32)0x00000FFF)

#if EEPROM_DEBUG == 1
  printf ( "\r\n\r\n[E2PROM] do format all page\r\n" );
#endif  /* EEPROM_DEBUG */

  FLASH_ClearFlag ( FLASH_FLAG_BSY|FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR|FLASH_FLAG_PGPERR|FLASH_FLAG_PGSERR );
  FLASH_WaitForLastOperation();
  for ( EraseCounter = 0; ( EraseCounter < EEPROM_PAGE_NUMBER )&&( FlashStatus == FLASH_COMPLETE ); EraseCounter++ )
  {
    /* 喂狗操作 */
 		WDG_KickCmd (); 
		Sector_id=FLASH_GetFlashSectorID(PAGE_BASE_ADDRESS( EraseCounter ));
		/* Erase the FLASH pages */
		WDG_KickCmd ();
		FlashStatus=( EEPROM_Status )FLASH_EraseSector(Sector_id,VOLTAGE_RANGE);
    WDG_KickCmd ();
    /* If erase operation was failed, a Flash error code is returned */
    if ( FlashStatus != EEPROM_COMPLETE )
    {
#if EEPROM_DEBUG == 1
      printf ( ", format sector( %2d 0x%X ) result %u\r\n", 
               EraseCounter, 
               PAGE_BASE_ADDRESS( EraseCounter ), 
               FlashStatus );
#endif  /* EEPROM_DEBUG */
      return FlashStatus;
    }
    else
    {
      eeprom.PageStatus[ EraseCounter ] = ERASED;
    } 

    /* Set Page as valid page: Write VALID_PAGE at Page X base address */
    FlashStatus = ( EEPROM_Status )FLASH_ProgramHalfWord ( PAGE_BASE_ADDRESS ( EraseCounter ), VALID_PAGE );   

    /* If erase operation was failed, a Flash error code is returned */
    if ( FlashStatus != EEPROM_COMPLETE )
    {

#if EEPROM_DEBUG == 1
      printf ( ", err Write VALID_PAGE sector ( %2d 0x%X ) result %u****\r\n", 
               EraseCounter, 
               PAGE_BASE_ADDRESS( EraseCounter ), 
               FlashStatus );
#endif  /* EEPROM_DEBUG */
      return FlashStatus;
    }
    else
    {

#if EEPROM_DEBUG == 1
      printf ( " EEPROM_FormatAllPages is OK ! Write VALID_PAGE sector ( %2d 0x%X ) result %u\r\n", 
               EraseCounter, 
               PAGE_BASE_ADDRESS( EraseCounter ), 
               FlashStatus );
#endif  /* EEPROM_DEBUG */			

      eeprom.PageStatus[ EraseCounter ] = VALID_PAGE;
    }     
  }

  /* If program operation was completed */
  return EEPROM_COMPLETE;
}






/* 
 * 功能描述：Returns the last stored variable data, if found, which correspond to
 *           the passed virtual address
 * 引用参数：VirtAddress: Variable virtual address
 *           Data: Global variable contains the read variable value
 *          
 * 返回值  ：Success or error status:
 *           - 0: if variable was found
 *           - 1: if the variable was not found
 *           - EEPROM_NO_VALID_PAGE: if no valid page was found
 *           - EEPROM_ERROR_ADDRESS: if error page address was used
 */
extern u16 EEPROM_ReadVariable ( u32 VirtAddress, u16 * Data )
{
  u16 ReadStatus = 1;
  u32 Address = 0;

#if EEPROM_DEBUG == 3
  printf ( "\r\n\r\n[E2PROM] do read variable\r\n" );
#endif  /* EEPROM_DEBUG */

  /* Get active Page for read operation */
  if ( VirtAddress >= EEPROM_MAX_SIZE )
  {
    return ( u16 )EEPROM_ERROR_ADDRESS;
  }
  else
  {
  	u16 ValidPage = VirtAddress / PAGE_SIZE;

	  /* Check if there is no valid page */
	  if ( eeprom.PageStatus[ ValidPage ] == ERASED )
	  {
#if EEPROM_DEBUG == 1
      printf ( "\r\n[E2PROM] format page ( %2d 0x%X ) ERASED\t", 
               ValidPage, 
               PAGE_BASE_ADDRESS( ValidPage ) );
#endif  /* EEPROM_DEBUG */
	    return EEPROM_NO_VALID_PAGE;
	  }

	  /* Get the valid Page Address */
    Address = ( u32 )VIRT_MAP( VirtAddress );
  
    /* Verify if Address content is 0xFFFF */
    *Data = DATA_16BIT( Address );
    ReadStatus = 0;
  }
	  
  /* Return ReadStatus value: (0: variable exist, 1: variable doesn't exist) */
  return ReadStatus;
}






/* 
 * 功能描述：Find valid Page for write or read operation
 * 引用参数：Operation: operation to achieve on the valid page.
 *           This parameter can be one of the following values:
 *              READ_FROM_VALID_PAGE: read operation from valid page
 *              WRITE_IN_VALID_PAGE: write operation from valid page
 *          
 * 返回值  ：Valid page address or EEPROM_NO_VALID_PAGE in case
 *           of no valid page was found
 */
extern u32 EEPROM_FindValidPage ( u8 Operation )
{
  u32 pageNumber = 0;

#if EEPROM_DEBUG == 1
  printf ( "\r\n\r\n[E2PROM] do find valid page\r\n" );
#endif  /* EEPROM_DEBUG */    

	/* Write or read operation */
	switch ( Operation )
	{
	  case WRITE_IN_VALID_PAGE:   /* Write operation */
		{
			bool __RESERACH_VALID_PAGE = FALSE;

#if EEPROM_DEBUG == 1
      printf ( "\r\nops WRITE_IN_VALID_PAGE\r\n" );
#endif  /* EEPROM_DEBUG */

			while ( TRUE ) 
			{								
				for ( ; eeprom.WritePageIndex < EEPROM_PAGE_NUMBER; eeprom.WritePageIndex++ )
				{
					if ( ( eeprom.PageStatus[ eeprom.WritePageIndex ] == VALID_PAGE )||
               ( eeprom.PageStatus[ eeprom.WritePageIndex ] == ERASED ) )
					{
						pageNumber = eeprom.WritePageIndex;
            eeprom.WritePageIndex++;

#if EEPROM_DEBUG == 1
            printf ( "\r\nget write page id %u write-ptr %u\r\n", 
                     pageNumber,
                     eeprom.WritePageIndex );
#endif  /* EEPROM_DEBUG */

            return DATA_VIRT_ADDRESS ( pageNumber );
					}
				}

				if ( eeprom.WritePageIndex >= EEPROM_PAGE_NUMBER )
				{
					eeprom.WritePageIndex = 0;  /* 页索引置位 */
				}

				if ( __RESERACH_VALID_PAGE != TRUE )
				{
					__RESERACH_VALID_PAGE = TRUE;
				}		
				else
				{
					break;
				}
			}
		}
		break;

	  case READ_FROM_VALID_PAGE:  /* Read operation */
#if EEPROM_DEBUG == 1
      printf ( "\r\nops READ_FROM_VALID_PAGE\r\n" );
#endif  /* EEPROM_DEBUG */

			for ( ; eeprom.ReadPageIndex < EEPROM_PAGE_NUMBER; eeprom.ReadPageIndex++ )
			{
#if EEPROM_DEBUG == 1
				printf ( "\r\nREAD_FROM_VALID_PAGE eeprom.ReadPageIndex: %d\r\n", eeprom.ReadPageIndex);
				printf ( "\r\nREAD_FROM_VALID_PAGE eeprom.PageStatus[ eeprom.ReadPageIndex ]: %x\r\n", eeprom.PageStatus[ eeprom.ReadPageIndex ]);
#endif  /* EEPROM_DEBUG */				
				
				if ( eeprom.PageStatus[ eeprom.ReadPageIndex ] == RECEIVE_DATA )
				{
					pageNumber = eeprom.ReadPageIndex;
          eeprom.ReadPageIndex++;

#if EEPROM_DEBUG == 1
          printf ( "\r\nget read page id %u read-ptr %u\r\n", 
                   pageNumber,
                   eeprom.ReadPageIndex );
#endif  /* EEPROM_DEBUG */

					return DATA_VIRT_ADDRESS ( pageNumber );
				}
			}

			if ( eeprom.ReadPageIndex >= EEPROM_PAGE_NUMBER )
			{
				eeprom.ReadPageIndex = 0;  /* 页索引置位 */
			}
		  break;

	  default:
		  break;
	}

#if EEPROM_DEBUG == 1
  printf ( "\r\nerr find EEPROM_NO_VALID_PAGE!****\r\n" );
#endif  /* EEPROM_DEBUG */

  return EEPROM_NO_VALID_PAGE;
}






/* 
 * 功能描述：Writes/upadtes variable data in EEPROM
 * 引用参数：VirtAddress: Variable virtual address
 *           Data: 16 bit data to be written
 *
 * 返回值  ：Success or error status:
 *           - FLASH_COMPLETE: on success
 *           - PAGE_FULL: if valid page is full
 *           - EEPROM_NO_VALID_PAGE: if no valid page was found
 *           - Flash error code: on write Flash error
 */
extern u16 EEPROM_WriteVariable ( u32 VirtAddress, u16 Data )
{
  #define EepromEraseTimeout             ((u32)0x00000FFF)
  FLASH_Status FlashStatus = FLASH_GetStatus ();

  FLASH_ClearFlag ( FLASH_FLAG_BSY|FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR|FLASH_FLAG_PGPERR|FLASH_FLAG_PGSERR );


  FLASH_WaitForLastOperation();

#if EEPROM_DEBUG == 1
  printf ( "\r\n\r\n[E2PROM] do write variable, " );
#endif  /* EEPROM_DEBUG */

  /* Get active Page for write operation */
  if ( VirtAddress >= EEPROM_MAX_SIZE )
  {
#if EEPROM_DEBUG == 1
    printf ( "VirtAddress 0x%X err*****\r\n", VirtAddress );
#endif  /* EEPROM_DEBUG */
    return ( u16 )EEPROM_ERROR_ADDRESS;  
  }
  else
  {
    /* Write the variable virtual address and value in the EEPROM */
    /* Get the valid Page start Address */
    u32 Address = ( u32 )VIRT_MAP( VirtAddress );
  	u16 PageIndex = VirtAddress / PAGE_SIZE;
    EEPROM_Status flashStatus = EEPROM_COMPLETE;

#if EEPROM_DEBUG == 1
    printf ( "page ( %2d 0x%X ), user data 0x%X,\t", 
              PageIndex, 
              eeprom.FLASH_ADDRESS, 
              Data );
#endif  /* EEPROM_DEBUG */

	  /* Check if there is no valid page */
	  if ( eeprom.PageStatus[ PageIndex ] != RECEIVE_DATA )
	  {
      /* Set Page as valid page: Write VALID_PAGE at Page X base address */
      flashStatus = ( EEPROM_Status )FLASH_ProgramHalfWord ( PAGE_BASE_ADDRESS( PageIndex ), RECEIVE_DATA );

      if ( flashStatus == FLASH_COMPLETE )
      {
        eeprom.PageStatus[ PageIndex ] = RECEIVE_DATA; 
#if EEPROM_DEBUG == 1
        printf ( "stat RECEIVE_DATA, " );
#endif  /* EEPROM_DEBUG */

      }
      else
      {
#if EEPROM_DEBUG == 1
        if ( eeprom.PageStatus[ PageIndex ] == ERASED )
        {
          printf ( "stat ERASED, program RECEIVE_DATA err***\r\n" );
        }
        else
        {
          printf ( "stat VALID_PAGE, program RECEIVE_DATA err***\r\n" );  
        }
#endif  /* EEPROM_DEBUG */
        return flashStatus;  
      }
	  }
        
    /* Verify if Address content is 0xFFFF */
    if ( ( DATA_16BIT( Address ) ) == 0xFFFF )
    {
      /* Set variable data */
      flashStatus = ( EEPROM_Status )FLASH_ProgramHalfWord( Address, Data );
  
      /* If program operation was failed, a Flash error code is returned */
      if ( flashStatus != EEPROM_COMPLETE )
      {
#if EEPROM_DEBUG == 1
        printf ( "write data err***\r\n" );
#endif  /* EEPROM_DEBUG */
        return flashStatus;
      }
    }
    else
    {

#if EEPROM_DEBUG == 1
      printf ( "write protect err***\r\n" );
#endif  /* EEPROM_DEBUG */
      return EEPROM_ERROR_WRP;
    }  
  }

#if EEPROM_DEBUG == 1
  printf ( "write OK\r\n" );
#endif  /* EEPROM_DEBUG */

  /* Return last operation status */
  return EEPROM_COMPLETE;
}







#if 0
/* 
 * 功能描述：FLASH NVIC配置
 * 引用参数：无
 *          
 * 返回值  ：无
 * 
 */
static void FLASH_NvicConfig ( void )
{
  NVIC_InitTypeDef NVIC_InitStructure;

  /* FLASH IRQ Channel configuration */
  NVIC_InitStructure.NVIC_IRQChannel = FLASH_IRQChannel;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = FLASH_PreemptionPriority;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = FLASH_SubPriority;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init ( ( NVIC_InitTypeDef* )&NVIC_InitStructure );

  /* Enable DMA1 channel6 IRQ Channel */
  NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel6_IRQChannel;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = DMA1_Channel6_PreemptionPriority;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = DMA1_Channel6_SubPriority;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init ( ( NVIC_InitTypeDef* )&NVIC_InitStructure );
}
#endif







/* 
 * 功能描述: FLASH DMA传输请求
 * 引用参数：无
 *          
 * 返回值  ：无
 * 
 */
static void FLASH_DMAConfig ( u32 eepromDir )
{
	DMA_InitTypeDef  DMA_InitStructure;
  __IO uint32_t    Timeout = TIMEOUT_MAX;
    
  /* Enable DMA clock */
  RCC_AHB1PeriphClockCmd(DMA_STREAM_CLOCK, ENABLE);
  
  /* Reset DMA Stream registers (for debug purpose) */
  DMA_DeInit(DMA_STREAM);

  /* Check if the DMA Stream is disabled before enabling it.
     Note that this step is useful when the same Stream is used multiple times:
     enabled, then disabled then re-enabled... In this case, the DMA Stream disable
     will be effective only at the end of the ongoing data transfer and it will 
     not be possible to re-configure it before making sure that the Enable bit 
     has been cleared by hardware. If the Stream is used only once, this step might 
     be bypassed. */
  while (DMA_GetCmdStatus(DMA_STREAM) != DISABLE)
  {
  }

  /* Configure DMA Stream */
  DMA_InitStructure.DMA_Channel = DMA_CHANNEL;  
	if(eepromDir==EEPROM_DIR_SRC)
	{
		DMA_InitStructure.DMA_PeripheralBaseAddr = ( u32 )eeprom.FLASH_ADDRESS;
		DMA_InitStructure.DMA_Memory0BaseAddr =  ( u32 )eeprom.RAM_ADDRESS;
	
	}
  else
	{
		DMA_InitStructure.DMA_PeripheralBaseAddr = ( u32 )eeprom.RAM_ADDRESS; 
		DMA_InitStructure.DMA_Memory0BaseAddr = ( u32 )eeprom.FLASH_ADDRESS;
	}

  DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToMemory;
  DMA_InitStructure.DMA_BufferSize = eeprom.Buffer_Transfer_Size;
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
	
  DMA_Init(DMA_STREAM, &DMA_InitStructure);
    
  /* Enable DMA Stream Transfer Complete interrupt */
  //DMA_ITConfig(DMA_STREAM, DMA_IT_TC, ENABLE);

  /* DMA Stream enable */
  DMA_Cmd(DMA_STREAM, ENABLE);

  /* Check if the DMA Stream has been effectively enabled.
     The DMA Stream Enable bit is cleared immediately by hardware if there is an 
     error in the configuration parameters and the transfer is no started (ie. when
     wrong FIFO threshold is configured ...) */
  Timeout = TIMEOUT_MAX;
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
	
}






/* 
 * 功能描述：FLASH数据比较
 * 引用参数：无
 *          
 * 返回值  ：无
 * 
 */
static EEPROM_Status BufferCmp ( uc8 * pBuffer1, u8 * pBuffer2, s16 BufferLength )
{
  while ( BufferLength > 0 )
  {
    if ( *pBuffer1 != *pBuffer2 )
    {
      printf("\r\n  pBuffer1: %x  pBuffer2: %x  \r\n",*pBuffer1,*pBuffer2);
			return EEPROM_ERROR_PGS;
    }
    else
    {
      pBuffer1++;
      pBuffer2++;
      BufferLength--;    
    }
  }

  return EEPROM_COMPLETE;  
}







/* 
 * 功能描述：FLASH数据比较
 * 引用参数：无
 *          
 * 返回值  ：无
 * 
 */
extern EEPROM_Status EEPROM_ReadData ( u32 VirtAddress, u8 Data[], u16 DataLen )
{
  EEPROM_Status FlashStatus = EEPROM_COMPLETE;
  u16 ValidPage = VirtAddress / PAGE_SIZE;

#if EEPROM_DEBUG == 1
  printf ( "\r\n[E2PROM] do read page data\r\n" );
#endif  /* EEPROM_DEBUG */

  /* Get active Page for read operation */
  if ( ( ( VirtAddress+ DataLen ) >= EEPROM_MAX_SIZE )||
       ( ( VirtAddress+ DataLen ) >= PAGE_END_ADDRESS( ValidPage ) ) )
  {
#if EEPROM_DEBUG == 1
    printf ( "\r\nread length err***\r\n" );
#endif  /* EEPROM_DEBUG */
    return EEPROM_ERROR_ADDRESS;
  }
  else
  {
	  /* Check if there is no valid page */
	  if ( eeprom.PageStatus[ ValidPage ] != RECEIVE_DATA )
	  {
#if EEPROM_DEBUG == 1
      printf ( "\r\n read VirtAddress addr 0x%X, page id %u, stat 0x%X EEPROM_NO_VALID_PAGE\r\n",
               VirtAddress, 
               ValidPage,
               eeprom.PageStatus[ ValidPage ] );
#endif  /* EEPROM_DEBUG */
	    return  EEPROM_NO_VALID_PAGE;
	  }
    else if ( Data != NULL && DataLen > 0 )
    {
  	  /* Get the valid Page end Address */
      eeprom.FLASH_ADDRESS = VIRT_MAP ( VirtAddress );
      eeprom.RAM_ADDRESS = ( u32 )Data;
      eeprom.Buffer_Transfer_Size = DataLen;

#if EEPROM_DEBUG == 1
      printf ( "\r\nread page ( %u 0x%X ), ram addr 0x%X, size %u\r\n",
                ValidPage, 
                eeprom.FLASH_ADDRESS, 
                eeprom.RAM_ADDRESS,
                eeprom.Buffer_Transfer_Size );
#endif  /* EEPROM_DEBUG */

      /* Enable DMA1 Channel6 Transfer Complete / error interrupt */
      FLASH_DMAConfig ( EEPROM_DIR_SRC );
        
      /* Wait the end of transmission */
      while ( DMA_GetCurrDataCounter ( DMA_STREAM ) != 0 )
      {
      	if ( DMA_GetFlagStatus ( DMA_STREAM,DMA_IT_TEIF0 ) == SET )
      	{
					printf("\r\n[DMA EVENT]: DMA2_Stream0  error \r\n");
      		break;	/* 通道 0传输错误 */
      	}
      }
      
      /* Check if the transmitted and received data are equal */
      FlashStatus = BufferCmp ( ( uc8 *)eeprom.FLASH_ADDRESS, ( u8 *)eeprom.RAM_ADDRESS, eeprom.Buffer_Transfer_Size );    
		}
    else
    {
      FlashStatus = EEPROM_ERROR_PGS;
    }
  }
  return FlashStatus;
}







/* 
 * 功能描述：FLASH数据比较
 * 引用参数：无
 *          
 * 返回值  ：无
 * 
 */
extern EEPROM_Status EEPROM_WriteData ( u32 VirtAddress, u8 Data[], u32 DataLen )
{
  u16 PageIndex = VirtAddress / PAGE_SIZE;
  FLASH_Status FlashStatus = FLASH_GetStatus ();

#if EEPROM_DEBUG == 1
  printf ( "\r\n\r\n[E2PROM] do write page data," );
#endif  /* EEPROM_DEBUG */

  FLASH_ClearFlag ( FLASH_FLAG_BSY|FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR|FLASH_FLAG_PGPERR|FLASH_FLAG_PGSERR );


  FLASH_WaitForLastOperation();

  /* Get active Page for write operation */
  if ( ( ( VirtAddress+ DataLen ) >= EEPROM_MAX_SIZE )||
       ( ( VirtAddress+ DataLen ) >= PAGE_END_ADDRESS( PageIndex ) ) )
  {

#if EEPROM_DEBUG == 1
    printf ( "write length err***\r\n" );
#endif  /* EEPROM_DEBUG */
    return EEPROM_ERROR_ADDRESS;
  }
  else
  {
	  /* Check if there is no valid page */
	  if ( eeprom.PageStatus[ PageIndex ] != RECEIVE_DATA )
	  {

#if EEPROM_DEBUG == 1
      printf ( "write VirtAddress addr 0x%X page id %u, stat 0x%X EEPROM_NO_VALID_PAGE\r\n",
               VirtAddress, 
               PageIndex, 
               eeprom.PageStatus[ PageIndex ] );
#endif  /* EEPROM_DEBUG */

      
      return EEPROM_NO_VALID_PAGE; 
	  }
    else if ( Data != NULL && DataLen > 0 )
    {
      u32 i;
        
  	  /* Get the valid Page end Address */
      eeprom.FLASH_ADDRESS = VIRT_MAP ( VirtAddress );
      eeprom.RAM_ADDRESS = ( u32 )Data;
      eeprom.Buffer_Transfer_Size = DataLen;

#if EEPROM_DEBUG == 1
      printf ( "write page ( %u 0x%X ), ram addr 0x%X, size %u,", 
                PageIndex, 
                eeprom.FLASH_ADDRESS, 
                eeprom.RAM_ADDRESS,
                eeprom.Buffer_Transfer_Size );
#endif  /* EEPROM_DEBUG */

    	for ( i = 0; i < eeprom.Buffer_Transfer_Size; i += 2 )
    	{
    		FlashStatus = FLASH_ProgramHalfWord ( eeprom.FLASH_ADDRESS, DATA_16BIT ( eeprom.RAM_ADDRESS ) );
        if ( FlashStatus == EEPROM_COMPLETE )
        {
          /* Check the corectness of written data */
      		if ( DATA_16BIT( eeprom.FLASH_ADDRESS ) != DATA_16BIT( eeprom.RAM_ADDRESS ) )
      		{

#if EEPROM_DEBUG == 1
            printf ( "write page id %u addr err@ 0x%X\r\n",
                     PageIndex,  
                     eeprom.FLASH_ADDRESS );
#endif  /* EEPROM_DEBUG */

  FLASH_ClearFlag ( FLASH_FLAG_BSY|FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR|FLASH_FLAG_PGPERR|FLASH_FLAG_PGSERR );

      			return EEPROM_ERROR_PGS;
      		}
          else
          {
            eeprom.FLASH_ADDRESS += 2;
      		  eeprom.RAM_ADDRESS += 2;
          }
        }
        else
        {

#if EEPROM_DEBUG == 1
          printf ( "program page id %u addr err@ 0x%X, stat %u\r\n",
                   PageIndex, 
                   eeprom.FLASH_ADDRESS,
                   FlashStatus );
#endif  /* EEPROM_DEBUG */

  FLASH_ClearFlag ( FLASH_FLAG_BSY|FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR|FLASH_FLAG_PGPERR|FLASH_FLAG_PGSERR );


          return ( EEPROM_Status )FlashStatus;
        }
    	}        
    }
    else
    {
      return EEPROM_ERROR_PGS;
    }
  }

  return ( EEPROM_Status )FlashStatus;
}







/* 
 * 功能描述：FLASH中断处理
 * 引用参数：无
 *          
 * 返回值  ：无
 * 
 */
extern void FLASH_IRQHandler ( void )
{
  FlagStatus Status = RESET;  

  /* FLASH忙标志位 */
  Status = FLASH_GetFlagStatus ( FLASH_FLAG_BSY  );
  if ( Status == SET )
  {
    return ;
  }

  /* FLASH操作结束标志位  */
  Status = FLASH_GetFlagStatus ( FLASH_FLAG_EOP );
  if ( Status == SET )
  {
    return ;
  }
	 /* flash编程对准误差错误标志*/
  Status = FLASH_GetFlagStatus ( FLASH_FLAG_PGAERR );
  if ( Status == SET )
  {
    return ;
  }
  /* FLASH编写并行错误标志位 */
  Status = FLASH_GetFlagStatus ( FLASH_FLAG_PGPERR );
  if ( Status == SET )
  {
    return ;
  }
	
	  /* FLASH编写顺序错误标志位 */
  Status = FLASH_GetFlagStatus ( FLASH_FLAG_PGSERR );
  if ( Status == SET )
  {
    return ;
  }

  /* FLASH页面写保护错误标志位 */
  Status = FLASH_GetFlagStatus ( FLASH_FLAG_WRPERR );
  if ( Status == SET )
  {
    return ;
  }                  

  /* FLASH选择字节错误标志位  */
  Status = FLASH_GetFlagStatus ( FLASH_FLAG_OPERR );
  if ( Status == SET )
  {
    return ;
  } 
}



