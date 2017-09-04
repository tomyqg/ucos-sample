


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
/*        FLASH�洢���ƿ�[����]           */
/******************************************/

#define FLASH_OBJECT_BLOCK_SIZE		0x10

/* �洢�����ַ */
#define FLASH_OBJECT_ADDRESS( __file_base_address__ )  		( (__file_base_address__) + 0 )

/* ���ݳ��ȵ�ַ */
#define FLASH_LENGTH_ADDRESS( __file_base_address__ )  		( (__file_base_address__) + 2 )

/* CRCУ���ַ */
#define FLASH_CRCCHK_ADDRESS( __file_base_address__ )  		( (__file_base_address__) + 4 )

/* ���ݴ洢ƫ�Ƶ�ַ */
#define FLASH_BUFFER_ADDRESS( __file_base_address__ )  		( (__file_base_address__) + FLASH_OBJECT_BLOCK_SIZE )


#define MAX_FLASH_OBJECT_NUMBER   ( EEPROM_PAGE_NUMBER / 2 )
#define MAX_FLASH_OBJECT_SIZE     ( PAGE_SIZE - FLASH_OBJECT_BLOCK_SIZE )






/******************************************/
/*              �ڲ�����[����]            */
/******************************************/


/* FLASH������ */
static u16 NbrofObjects = 0;

struct FLASH_OBJ
{
  /* �洢����ַ */
  u32 MemoryBaseAddr;

  /* ���� */
  u16 Length;
} FlashObjects[ MAX_FLASH_OBJECT_NUMBER ];



/*	Begin
    Flash Read and Write Protect
	Modified by jitao
*/
static OS_EVENT* OS_EventFlashProt = NULL;





/* 
 * ����������FLASH������������
 * ���ò�������
 *          
 * ����ֵ  ����
 * 
 */
static void  OSFlash_DECL_PROTECT (void)
{
  OS_EventFlashProt = OSSemCreate(1);
}





/* 
 * ����������FLASH��������
 * ���ò�������
 *          
 * ����ֵ  ����
 * 
 */
static void  OSFlash_PROTECT (void)
{
  INT8U err = OS_ERR_NONE;

 	OSSemPend(OS_EventFlashProt, 0, &err);
}







/* 
 * ����������FLASH����ȥ����
 * ���ò�������
 *          
 * ����ֵ  ����
 * 
 */
static void  OSFlash_UNPROTECT (void)
{
 	OSSemPost(OS_EventFlashProt);
}

/*   End    */




/* 
 * ����������FLASH CRC����
 * ���ò�������
 *          
 * ����ֵ  ����
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
 * ����������FLASH����ע��
 * ���ò�������
 *          
 * ����ֵ  ����
 * 
 */
extern void FLASH_LowlLevelIfInit ( void )
{
  u32 FileBaseAddress;
  u16 ID;
  EEPROM_Status Status = EEPROM_IfInit ();

  OSFlash_DECL_PROTECT();

  /* E2PROM�Ƿ���ɳ�ʼ��? */
  if ( Status == EEPROM_COMPLETE )
  {
    /* ��մ洢�������ṹ */
    memset ( ( void *)&FlashObjects, 0, sizeof ( struct FLASH_OBJ ) );
    while ( TRUE )
    {	
      /* ������Ч����ҳ�� */
      FileBaseAddress = EEPROM_FindValidPage ( READ_FROM_VALID_PAGE );

      /* �Ƿ���ҽ���? */
      if ( EEPROM_NO_VALID_PAGE == FileBaseAddress )
      {
        /* �Ƿ�����û����ݶ���? */
        if ( NbrofObjects == 0 )
      	{
          /* ��ʽ��ȫ��ҳ�� */
        	EEPROM_FormatAllPages ();
        }  

#if FLASH_DEBUG == 1        
        printf ( "\r\n[FLASH EVENT] objects %u\r\n", NbrofObjects ); 
#endif  /* FLASH_DEBUG */ 
        return ;   
      }
      else
      {
        /* ҳ���Ƿ���Ч */
        bool pageHit = FALSE;

        /* ��ȡҳ��洢����ID */
        EEPROM_ReadVariable ( FLASH_OBJECT_ADDRESS ( FileBaseAddress ), ( u16* )&ID );
        if ( ID < MAX_FLASH_OBJECT_NUMBER )
        {
          u16 rawCRC = 0;
          u16 chkCRC = 0;

          /* ��ȡҳ���û����ݳ��� */
          EEPROM_ReadVariable ( FLASH_LENGTH_ADDRESS ( FileBaseAddress ), ( u16* )&FlashObjects[ ID ].Length );

          /* �û����ݳ����Ƿ���Ч? */
          if ( FlashObjects[ ID ].Length < MAX_FLASH_OBJECT_SIZE )
        	{
            /* ��ȡҳ��ԭʼCRC���� */
            EEPROM_ReadVariable ( FLASH_CRCCHK_ADDRESS ( FileBaseAddress ), ( u16* )&rawCRC );
            FlashObjects[ ID ].MemoryBaseAddr = ( u32 )FileBaseAddress;
  
            /* �Ը�ҳ���������У�� */
            chkCRC = FLASH_ObjectCRCCalculate ( ID );

#if FLASH_DEBUG == 1        
            printf ( "\r\n[FLASH EVENT] obj-Id %u, raw-crc %u, cal-crc %u\r\n", 
                    ID, rawCRC, chkCRC ); 
#endif  /* FLASH_DEBUG */

            if ( rawCRC == chkCRC )
            {
              /* �û��洢�������ۼ� */
              NbrofObjects++; 
              pageHit = TRUE;
            }      		
        	}

          /* �ͷŴ洢������Դ */
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

          /* ��ʽ����ҳ�� */
          EEPROM_FormatPage ( FileBaseAddress );       
        }
      }
    }  
  }
}







/* 
 * ����������FLASH����ע��
 * ���ò�����(1)����ID
 *          
 * ����ֵ  ��״̬��
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
 * ����������FLASH����ע��
 * ���ò���������ID
 *          
 * ����ֵ  �����󳤶�
 * 
 */
extern u16 FLASH_ObjectExist ( u16 ID )
{
  return FlashObjects[ ID ].Length;
}







/* 
 * ����������FLASH�������
 * ���ò���������ID
 * 
 * ����ֵ  ��״̬��
 * 
 */
extern bool FLASH_EraseObject ( u16 ID )
{
	u16 tmpObjID;

  /* ID�Ƿ���Ч? */
  if ( MAX_FLASH_OBJECT_NUMBER <= ID )
  {
    return FALSE;
  }

  OSFlash_PROTECT ();

  /* ��ȡ�洢�Ķ����� */
  EEPROM_ReadVariable ( FLASH_OBJECT_ADDRESS ( FlashObjects[ ID ].MemoryBaseAddr ), &tmpObjID );
  if ( tmpObjID == ID )
  {
    /* ��ʽ����ҳ�� */
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
 * ����������FLASHȫ������
 * ���ò�������
 *          
 * ����ֵ  ����
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
 * ����������FLASH���ݶ�ȡ
 * ���ò�����(1)����ID
 *           (2)����
 *           (3)��������
 *
 * ����ֵ  ��״̬��
 * 
 */
extern bool FLASH_ReadData ( u16 ID, u16 Len, u8 *ReadData )
{
  if ( ( ReadData != NULL )&&( Len > 0 ) )
  {
    /* ID�Ƿ���Ч? �洢��ַ�Ƿ���Ч? */
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
 * ����������FLASH CRC����
 * ���ò�������
 *          
 * ����ֵ  ����
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
 * ����������FLASHд�û�����
 * ���ò�����(1)����ID
 *           (2)����
 *           (3)��������
 *
 * ����ֵ  ��״̬��
 * 
 */
extern bool FLASH_WriteData ( u16 ID, u16 Len, u8 *WriteData )
{
  /* �Ƿ���Ч�洢ID? */
  if ( ID < MAX_FLASH_OBJECT_NUMBER )
  {
    /* ��������������Ч��? */
    if ( ( WriteData != NULL )&&( Len <= MAX_FLASH_OBJECT_SIZE && Len > 0 ) )
    {
      /* FIX-ME �Ƿ�Ӧ�÷������µĴ洢����ɾ���ϵ����ݴ洢?? */
    	if ( FlashObjects[ ID ].Length != 0 )
  		{
        /* �Ƿ�����ոö���? */
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
        /* ���·���洢�ռ� */
  			u32 FileBaseAddress = EEPROM_FindValidPage ( WRITE_IN_VALID_PAGE );
  
        /* ִ��Ԫ���ݸ��� */
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

      /* �����û����� */
  		if ( EEPROM_COMPLETE == EEPROM_WriteData ( FLASH_BUFFER_ADDRESS ( FlashObjects[ ID ].MemoryBaseAddr ), WriteData, Len ) )
      {
        u16 crc = FLASH_DataCRCCalculate ( WriteData, Len );

#if FLASH_DEBUG == 1        
        printf ( "\r\n[FLASH EVENT] write object %u, data size %ubytes\r\n", ID, Len ); 
#endif  /* FLASH_DEBUG */ 

        FlashObjects[ ID ].Length = Len;

        /* �洢����Ԫ���ݣ����Ⱥ�У��� */
        EEPROM_WriteVariable ( FLASH_LENGTH_ADDRESS ( FlashObjects[ ID ].MemoryBaseAddr ), Len );
        EEPROM_WriteVariable ( FLASH_CRCCHK_ADDRESS ( FlashObjects[ ID ].MemoryBaseAddr ), crc );
		    OSFlash_UNPROTECT ();
        return TRUE;
      }

#if FLASH_DEBUG == 1   
      else
      {
        /* �ȴ�������� */
        printf ( "\r\n[FLASH EVENT] fail to write object %u\r\n", ID ); 
      }
#endif  /* FLASH_DEBUG */ 

	   OSFlash_UNPROTECT ();
    }
  }  

  return FALSE;
}

#endif



