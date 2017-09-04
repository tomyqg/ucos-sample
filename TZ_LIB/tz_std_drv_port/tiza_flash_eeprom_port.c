


#include <stdio.h>
#include <string.h>

#include "tiza_flash_eeprom_port.h"
#include "tiza_flash_eeprom.h"
#include "ucos_ii.h"


	enum{
		FALSE = 0,
		TRUE = !FALSE
	};







#if __USE_FLASH__ > 0




/******************************************/
/*        FLASH存储控制块[定义]           */
/******************************************/

#define FLASH_OBJECT_BLOCK_SIZE		0x10

/* 存储对象地址 */
#define FLASH_OBJECT_ADDRESS( __file_base_address__ )  		( (__file_base_address__) + 0 )

/* 数据长度地址 */
#define FLASH_LENGTH_ADDRESS( __file_base_address__ )  		( (__file_base_address__) + 2 )

/* CRC校验地址 */
#define FLASH_CRCCHK_ADDRESS( __file_base_address__ )  		( (__file_base_address__) + 4 )

/* 数据存储偏移地址 */
#define FLASH_BUFFER_ADDRESS( __file_base_address__ )  		( (__file_base_address__) + FLASH_OBJECT_BLOCK_SIZE )


#define MAX_FLASH_OBJECT_NUMBER   ( EEPROM_PAGE_NUMBER / 2 )
#define MAX_FLASH_OBJECT_SIZE     ( PAGE_SIZE - FLASH_OBJECT_BLOCK_SIZE )






/******************************************/
/*              内部变量[定义]            */
/******************************************/


/* FLASH对象数 */
static u16 NbrofObjects = 0;

struct FLASH_OBJ
{
  /* 存储区基址 */
  u32 MemoryBaseAddr;

  /* 长度 */
  u16 Length;
} FlashObjects[ MAX_FLASH_OBJECT_NUMBER ];



/*	Begin
    Flash Read and Write Protect
	Modified by jitao
*/
static OS_EVENT* OS_EventFlashProt = NULL;





/* 
 * 功能描述：FLASH操作声明保护
 * 引用参数：无
 *          
 * 返回值  ：无
 * 
 */
static void  OSFlash_DECL_PROTECT (void)
{
  OS_EventFlashProt = OSSemCreate(1);
}





/* 
 * 功能描述：FLASH操作保护
 * 引用参数：无
 *          
 * 返回值  ：无
 * 
 */
static void  OSFlash_PROTECT (void)
{
  INT8U err = OS_ERR_NONE;

 	OSSemPend(OS_EventFlashProt, 0, &err);
}







/* 
 * 功能描述：FLASH操作去保护
 * 引用参数：无
 *          
 * 返回值  ：无
 * 
 */
static void  OSFlash_UNPROTECT (void)
{
 	OSSemPost(OS_EventFlashProt);
}

/*   End    */




/* 
 * 功能描述：FLASH CRC配置
 * 引用参数：无
 *          
 * 返回值  ：无
 * 
 */
static u16 FLASH_ObjectCRCCalculate ( u16 ID )
{
	u32 i = 0;
  u32 m = 0;
  u32 n = 0;
  u32 addr = 0;
	u16 crc16_data = 0;
	union 
	{
		u16 u16_data;
		u8 buff[ 2 ];
	} long_data;

  addr = FLASH_BUFFER_ADDRESS( FlashObjects[ ID ].MemoryBaseAddr );
	for ( i = 0; i < FlashObjects[ ID ].Length; )
	{
    EEPROM_ReadVariable ( addr, ( u16* )&long_data.u16_data );
    addr += 2;
		for ( n = 0; n < 2; n++ )
		{
			if ( i >= FlashObjects[ ID ].Length )
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
 * 功能描述：FLASH操作注册
 * 引用参数：无
 *          
 * 返回值  ：无
 * 
 */
extern void FLASH_LowlLevelIfInit ( void )
{
  u32 FileBaseAddress;
  u16 ID;
  EEPROM_Status Status = EEPROM_IfInit ();

  OSFlash_DECL_PROTECT();

  /* E2PROM是否完成初始化? */
  if ( Status == EEPROM_COMPLETE )
  {
    /* 清空存储对象管理结构 */
    memset ( ( void *)&FlashObjects, 0, sizeof ( struct FLASH_OBJ ) );
    while ( TRUE )
    {	
      /* 查找有效数据页面 */
      FileBaseAddress = EEPROM_FindValidPage ( READ_FROM_VALID_PAGE );

      /* 是否查找结束? */
      if ( EEPROM_NO_VALID_PAGE == FileBaseAddress )
      {
        /* 是否存在用户数据对象? */
        if ( NbrofObjects == 0 )
      	{
          /* 格式化全部页面 */
        	EEPROM_FormatAllPages ();
        }  

#if FLASH_DEBUG == 1        
        printf ( "\r\n[FLASH EVENT] objects %u\r\n", NbrofObjects ); 
#endif  /* FLASH_DEBUG */ 
        return ;   
      }
      else
      {
        /* 页面是否有效 */
        bool pageHit = FALSE;

        /* 获取页面存储对象ID */
        EEPROM_ReadVariable ( FLASH_OBJECT_ADDRESS ( FileBaseAddress ), ( u16* )&ID );
        if ( ID < MAX_FLASH_OBJECT_NUMBER )
        {
          u16 rawCRC = 0;
          u16 chkCRC = 0;

          /* 获取页面用户数据长度 */
          EEPROM_ReadVariable ( FLASH_LENGTH_ADDRESS ( FileBaseAddress ), ( u16* )&FlashObjects[ ID ].Length );

          /* 用户数据长度是否有效? */
          if ( FlashObjects[ ID ].Length < MAX_FLASH_OBJECT_SIZE )
        	{
            /* 获取页面原始CRC数据 */
            EEPROM_ReadVariable ( FLASH_CRCCHK_ADDRESS ( FileBaseAddress ), ( u16* )&rawCRC );
            FlashObjects[ ID ].MemoryBaseAddr = ( u32 )FileBaseAddress;
  
            /* 对该页面进行数据校验 */
            chkCRC = FLASH_ObjectCRCCalculate ( ID );

#if FLASH_DEBUG == 1        
            printf ( "\r\n[FLASH EVENT] obj-Id %u, raw-crc %u, cal-crc %u\r\n", 
                    ID, rawCRC, chkCRC ); 
#endif  /* FLASH_DEBUG */

            if ( rawCRC == chkCRC )
            {
              /* 用户存储块数量累计 */
              NbrofObjects++; 
              pageHit = TRUE;
            }      		
        	}

          /* 释放存储管理资源 */
          if ( pageHit != TRUE )
          {
            FlashObjects[ ID ].Length = 0; 
            FlashObjects[ ID ].MemoryBaseAddr = 0;             
          }
        }
        
        if ( pageHit == FALSE )
        {

#if FLASH_DEBUG == 1        
          printf ( "\r\n[FLASH EVENT] format addr 0x%X, obj-Id %u\r\n", 
                   FileBaseAddress, ID ); 
#endif  /* FLASH_DEBUG */

          /* 格式化该页面 */
          EEPROM_FormatPage ( FileBaseAddress );       
        }
      }
    }  
  }
}







/* 
 * 功能描述：FLASH操作注册
 * 引用参数：(1)对象ID
 *          
 * 返回值  ：状态码
 * 
 */
extern bool FLASH_ObjectSubscribe ( u16 ID )
{
  bool result = FALSE;

  if ( ID < MAX_FLASH_OBJECT_NUMBER )
  {
		if ( ( FlashObjects[ ID ].Length == 0 )&&( FlashObjects[ ID ].MemoryBaseAddr == 0 ) )
		{
			u32 FileBaseAddress = EEPROM_FindValidPage ( WRITE_IN_VALID_PAGE );

			if ( FileBaseAddress != EEPROM_NO_VALID_PAGE )
			{
			  EEPROM_WriteVariable ( FLASH_OBJECT_ADDRESS ( FileBaseAddress ), ID );
			  FlashObjects[ ID ].Length = 0;
			  FlashObjects[ ID ].MemoryBaseAddr = FileBaseAddress;
        result = TRUE;
			}
		}
  }

  return result; 
}







/* 
 * 功能描述：FLASH操作注册
 * 引用参数：对象ID
 *          
 * 返回值  ：对象长度
 * 
 */
extern u16 FLASH_ObjectExist ( u16 ID )
{
  return FlashObjects[ ID ].Length;
}







/* 
 * 功能描述：FLASH对象擦除
 * 引用参数：对象ID
 * 
 * 返回值  ：状态码
 * 
 */
extern bool FLASH_EraseObject ( u16 ID )
{
	u16 tmpObjID;

  /* ID是否有效? */
  if ( MAX_FLASH_OBJECT_NUMBER <= ID )
  {
    return FALSE;
  }

  OSFlash_PROTECT ();

  /* 获取存储的对象编号 */
  EEPROM_ReadVariable ( FLASH_OBJECT_ADDRESS ( FlashObjects[ ID ].MemoryBaseAddr ), &tmpObjID );
  if ( tmpObjID == ID )
  {
    /* 格式化该页面 */
	  EEPROM_FormatPage ( FlashObjects[ ID ].MemoryBaseAddr );
  }
  {
  	FlashObjects[ ID ].Length = 0;
  	FlashObjects[ ID ].MemoryBaseAddr = 0;
  }

  OSFlash_UNPROTECT ();

  return TRUE;
}







/* 
 * 功能描述：FLASH全部擦除
 * 引用参数：无
 *          
 * 返回值  ：无
 * 
 */
extern bool FLASH_EraseAll ( void )
{
  u16 ID;

  OSFlash_PROTECT ();

  for ( ID = 0; ID < MAX_FLASH_OBJECT_NUMBER; ID++ )
  {
    FLASH_EraseObject ( ID );
  }

  OSFlash_UNPROTECT ();

  return TRUE;
}











/* 
 * 功能描述：FLASH数据读取
 * 引用参数：(1)对象ID
 *           (2)长度
 *           (3)数据内容
 *
 * 返回值  ：状态码
 * 
 */
extern bool FLASH_ReadData ( u16 ID, u16 Len, u8 *ReadData )
{
  if ( ( ReadData != NULL )&&( Len > 0 ) )
  {
    /* ID是否有效? 存储地址是否有效? */
  	if ( ( MAX_FLASH_OBJECT_NUMBER <= ID )||( FlashObjects[ ID ].MemoryBaseAddr == 0 ) )
  	{
  		return FALSE;
  	}
  	
		if ( FlashObjects[ ID ].Length >= Len )
		{
      OSFlash_PROTECT ();
			if ( EEPROM_COMPLETE == EEPROM_ReadData ( FLASH_BUFFER_ADDRESS ( FlashObjects[ ID ].MemoryBaseAddr ), ReadData, Len ) )
      {

#if FLASH_DEBUG == 1
        printf ( "\r\n[FLASH EVENT] read object %u, size %u\r\n", ID, Len );
#endif  /* FLASH_DEBUG */ 

		    OSFlash_UNPROTECT ();
        return TRUE;
      }

#if FLASH_DEBUG == 1   
      else
      {
        printf ( "\r\n[FLASH EVENT] fail to read object %u\r\n", ID ); 
      }
#endif  /* FLASH_DEBUG */ 

      OSFlash_UNPROTECT ();
		}
  }

  return FALSE;  
}







/* 
 * 功能描述：FLASH CRC配置
 * 引用参数：无
 *          
 * 返回值  ：无
 * 
 */
static u16 FLASH_DataCRCCalculate ( u8 Data[], u16 Size )
{
	u32 i = 0;
  u32 m = 0;
	u16 crc16_data = 0;

	for ( i = 0; i < Size; i++ )
	{
		crc16_data = crc16_data ^ ( u16 )( Data[ i ] << 8 );
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

  return crc16_data;
}







/* 
 * 功能描述：FLASH写用户数据
 * 引用参数：(1)对象ID
 *           (2)长度
 *           (3)数据内容
 *
 * 返回值  ：状态码
 * 
 */
extern bool FLASH_WriteData ( u16 ID, u16 Len, u8 *WriteData )
{
  /* 是否有效存储ID? */
  if ( ID < MAX_FLASH_OBJECT_NUMBER )
  {
    /* 检查输入的数据有效性? */
    if ( ( WriteData != NULL )&&( Len <= MAX_FLASH_OBJECT_SIZE && Len > 0 ) )
    {
      /* FIX-ME 是否应该分配了新的存储块再删除老的数据存储?? */
    	if ( FlashObjects[ ID ].Length != 0 )
  		{
        /* 是否已清空该对象? */
  			if ( FALSE == FLASH_EraseObject ( ID ) )
        {
          return FALSE;
        }

#if FLASH_DEBUG == 1
        printf ( "\r\n[FLASH EVENT] erase object %u\r\n", ID );
#endif  /* FLASH_DEBUG */ 
  		}

      OSFlash_PROTECT ();
  		if ( FlashObjects[ ID ].Length == 0 && FlashObjects[ ID ].MemoryBaseAddr == 0 )
  		{
        /* 重新分配存储空间 */
  			u32 FileBaseAddress = EEPROM_FindValidPage ( WRITE_IN_VALID_PAGE );
  
        /* 执行元数据更新 */
  			if ( FileBaseAddress != EEPROM_NO_VALID_PAGE &&
             EEPROM_COMPLETE == EEPROM_WriteVariable ( FLASH_OBJECT_ADDRESS ( FileBaseAddress ), ID ) )
  			{
			    FlashObjects[ ID ].Length = 0;
  			  FlashObjects[ ID ].MemoryBaseAddr = FileBaseAddress;
  			}
        else
        {
          OSFlash_UNPROTECT ();
          return FALSE;
        }
  		}

      /* 更新用户数据 */
  		if ( EEPROM_COMPLETE == EEPROM_WriteData ( FLASH_BUFFER_ADDRESS ( FlashObjects[ ID ].MemoryBaseAddr ), WriteData, Len ) )
      {
        u16 crc = FLASH_DataCRCCalculate ( WriteData, Len );

#if FLASH_DEBUG == 1        
        printf ( "\r\n[FLASH EVENT] write object %u, data size %ubytes\r\n", ID, Len ); 
#endif  /* FLASH_DEBUG */ 

        FlashObjects[ ID ].Length = Len;

        /* 存储配置元数据：长度和校验和 */
        EEPROM_WriteVariable ( FLASH_LENGTH_ADDRESS ( FlashObjects[ ID ].MemoryBaseAddr ), Len );
        EEPROM_WriteVariable ( FLASH_CRCCHK_ADDRESS ( FlashObjects[ ID ].MemoryBaseAddr ), crc );
		    OSFlash_UNPROTECT ();
        return TRUE;
      }

#if FLASH_DEBUG == 1   
      else
      {
        /* 等待数据清空 */
        printf ( "\r\n[FLASH EVENT] fail to write object %u\r\n", ID ); 
      }
#endif  /* FLASH_DEBUG */ 

	   OSFlash_UNPROTECT ();
    }
  }  

  return FALSE;
}

#endif



