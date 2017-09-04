





#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "modem.h"








/* 
 * ��������: �����
 * ���ò���: (1)Ŀ��ָ��
 *           (2)ԭʼָ��
 *
 * ����ֵ  : ״̬��
 * 
 */
extern bool MDM_CmdCpy ( MDM_CmdTypedef *DstCmd, MDM_CmdTypedef *SrcCmd )
{
  if ( DstCmd && SrcCmd )
  {
    DstCmd->cmd_id = SrcCmd->cmd_id;        
    DstCmd->timeout = SrcCmd->timeout;
    DstCmd->retry_counter = SrcCmd->retry_counter;
    DstCmd->handler = SrcCmd->handler;
    DstCmd->required_state = SrcCmd->required_state;
    DstCmd->write_complete = SrcCmd->write_complete;
    if ( DstCmd->AtBuffer && SrcCmd->AtBuffer )
    {
      strcpy ( ( char * )DstCmd->AtBuffer, ( const char * )SrcCmd->AtBuffer );
    }
    //printf ( "\r\nread cmd %s\r\n", ( const char * )DstCmd->AtBuffer );
    return TRUE;    
  } 
  return FALSE; 
}







/* 
 * ��������: ��������
 * ���ò���:
 *          
 * ����ֵ  :
 * 
 */
extern bool MDM_CmdSet ( MDM_CmdTypedef *SrcCmd )
{
  if ( SrcCmd != NULL )
  {
    SrcCmd->cmd_id = 0;
    SrcCmd->timeout = 0;
    SrcCmd->retry_counter = 0;
    SrcCmd->handler = NULL;
    SrcCmd->required_state = SIM_UNKNOWN;
    SrcCmd->write_complete = FALSE;
    if ( SrcCmd->AtBuffer )
    {
      memset ( ( char * )SrcCmd->AtBuffer, 0, strlen ( ( const char * )SrcCmd->AtBuffer ) );
    }
    return TRUE;    
  } 
  return FALSE; 
}









/* 
 * ��������: ���Ų���
 * ���ò���:
 *          
 * ����ֵ  :
 * 
 */
extern ascii * MDM_StrTok ( MDM_RspTypedef *response, ascii *key )
{
  if ( response && key )
  {
    ascii *token = response->indicator;
    return token = ( ascii* )strtok ( ( char * )token, ( const char * )key );
  } 
  return NULL; 
}









/* 
 * ��������: �ַ�������
 * ���ò���:
 *          
 * ����ֵ  :
 * 
 */
extern ascii * MDM_StrStr ( MDM_RspTypedef *response, ascii *key )
{
  if ( response && key )
  {
    ascii *token = response->indicator;
    return token = ( ascii* )strstr ( ( const char * )token, ( const char * )key );
  } 
  return NULL; 
}








/* 
 * ��������: ����ӳ��Ϊ�ַ���
 * ���ò���:
 *          
 * ����ֵ  :
 * 
 */
extern ascii * MDM_Sym2Str ( const SymbolTable *symbolbase, s32 token )
{
  const SymbolTable *symbol = symbolbase;

  if ( symbol && token > 0 )
  {
    while ( symbol )
    {
      if ( symbol->symbol_token == token )
      {
        return symbol->symbol_name; 
      }
      else
      {
        symbol++; 
      }
    }
  }
   
  return NULL; 
}








/* 
 * ��������: ��������Ӧ��
 * ���ò���:
 *          
 * ����ֵ  :
 * 
 */
extern bool MDM_StrIsTerminalResponse ( MDM_RSP_ID RspID )
{
  return ( ( RspID >= MDM_OK )&&( RspID < MDM_LAST_TERMINAL ) ) ? TRUE : FALSE;
}









/* 
 * ��������: ��ȡ��ӦID
 * ���ò���:
 *          
 * ����ֵ  :
 * 
 */
extern MDM_RSP_ID MDM_GetResultCode ( MDM_RspTypedef *response )
{
	if ( response )
  {
    return response->Code;
  }
  else
  {
    return MDM_NO_STRING;
  }
}











/* 
 * ��������: �����������в����ض��ַ���
 * ���ò���:
 *          
 * ����ֵ  :
 * 
 */
static bool MDM_GetDataStr ( u8* Data, u16 DataSize, ascii *String )
{

#if 0
  /* ����Ƿ�����������Ч���� */
  if ( Data && DataSize > 0 && String )
  {
    s32 length = strlen ( ( const char* )String );
    s32 i = 0, j = 0;
    u8 *found = NULL;

    for ( j = 0; ( j < length )&&( length > 0 ); )
    {
      /* �趨Ԥ���ҵ��ַ� */
      found = ( u8* )( String + j );

      /* ������Դ�в��� */
      for ( i = 0; i < DataSize; i++ )
      {

        /* �Ƚϳɹ����������һ�ֱȽ� */
        if ( *found == *( Data + i ) )
        {
          j++;
          break;  
        }  
      }

      /* �ض��ַ�����ʧ�ܣ����˳� */
      if ( i >= DataSize )
      {
        return FALSE;
      }
    }

    if ( j >= length )
    {
      return TRUE;
    }
  }
#else
  if ( Data && DataSize > 0 && String )
  {
    s32 length = strlen ( ( const char* )String );
    s32 i = 0;
    s32 j = 0;
    u8 *found = NULL;

    /* ADD #001 ��ʼ�������� */
    if ( length <= 0 && DataSize < length )
    {
      return FALSE;
    }

    for ( j = 0; j < length; )
    {
      /* �趨Ԥ���ҵ��ַ� */
      found = ( u8* )( String + j );

      /* ������Դ�в��� */
      for ( ; i < DataSize; )
      {
        /* �Ƚϳɹ����������һ�ֱȽ� */
        if ( *found == *( Data + i ) )
        {
          j++;

          /* BUG #001 �Ƚ���һ��Ŀ�� */
          i++;
          break;
        }
        else
        {
          /* BUG #002 �Ƚ�ʧ��ʱ��ͷ��ʼ�Ƚ� */
          if ( j == 0 )
          {
            i++;
          }
          j = 0;
          break;
        }
      }

      /* �ض��ַ�����ʧ�ܣ����˳� */
      if ( i >= DataSize && j < length )
      {
        return FALSE;
      }
    }

    if ( j >= length )
    {
      return TRUE;
    }
  }
#endif
  
  return FALSE;
} 











                                                                                           

/* 
 * ��������: ��ȡ������ӦID
 * ���ò���:
 *          
 * ����ֵ  :
 * 
 */
extern MDM_RSP_ID MDM_StrGetID ( MDM_RspTypedef *response )
{

#if 1
{
			u8 *info = ( u8* )( response->indicator );
			u16 size = ( u16 )( response->totalsize );

			if ( info == NULL )
			{
				return  MDM_NO_STRING;
			}
			else if ( MDM_GetDataStr ( info, size, "OK" ) == TRUE )    
			{
				return  MDM_OK;
			}
			else if ( MDM_GetDataStr ( info, size, "BUSY" ) == TRUE )
			{
				return  MDM_BUSY;
			}
			else if ( MDM_GetDataStr ( info, size, "NO ANSWER" ) == TRUE )
			{
				return  MDM_NO_ANSWER;
			}
			else if ( MDM_GetDataStr ( info, size, "NO CARRIER" ) == TRUE )
			{
				return  MDM_NO_CARRIER;
			}
			else if ( MDM_GetDataStr ( info, size, "CONNECT" ) == TRUE )
			{
				return  MDM_CONNECT;
			}
			else if ( MDM_GetDataStr ( info, size, "ERROR" ) == TRUE )
			{
				return  MDM_ERROR;
			}
			else if ( MDM_GetDataStr ( info, size, "+CME ERROR:" ) == TRUE )
			{
				return  MDM_CME_ERROR;
			}
			else if ( MDM_GetDataStr ( info, size, "+CMS ERROR:" ) == TRUE )
			{
				return  MDM_CMS_ERROR;
			}
			else if ( MDM_GetDataStr ( info, size, "+CPIN:" ) == TRUE )
			{
				return  MDM_CPIN;
			}
			else if ( MDM_GetDataStr ( info, size, "RING" ) == TRUE )
			{
				return  MDM_RING;
			}
			else if ( MDM_GetDataStr ( info, size, "+CRING:" ) == TRUE )
			{
				return  MDM_CRING;
			}
			else if ( MDM_GetDataStr ( info, size, "+CMEE:" ) == TRUE )
			{
				return  MDM_CMEE;
			}
			else if ( MDM_GetDataStr ( info, size, "+CREG:" ) == TRUE )
			{
				return  MDM_CREG;
			}
			else if ( MDM_GetDataStr ( info, size, "+CGREG:" ) == TRUE )
			{
				return  MDM_CGREG;
			}
			else if ( MDM_GetDataStr ( info, size, "+CRC:" ) == TRUE )
			{
				return  MDM_CRC;
			}
			else if ( MDM_GetDataStr ( info, size, "+CGEV:" ) == TRUE )
			{
				return  MDM_CGEV;
			}
			else if ( MDM_GetDataStr ( info, size, "+CGEREP:" ) == TRUE )
			{
				return  MDM_CGEREP;
			}
			else
			{
				return  MDM_NO_STRING;
			}
}
#endif  /* �汾���� */


}










/* 
 * ��������: �������е�ʮ�������ַ���ת��������
 * ���ò���:
 *          
 * ����ֵ  :
 * 
 */
extern u32 MDM_String2Bytes ( const u8* Src, u8* Dst, u16 SrcLength )
{
  u32 i;

  for ( i = 0; i < SrcLength; i += 2 )
  {
    if ( *Src >= '0' && *Src <= '9' )
    {
      *Dst = ( *Src - '0' ) << 4;
    }
    else
    {
      *Dst = ( *Src - 'A' + 10 ) << 4;
    }
    Src++;
    
    if ( *Src >= '0' && *Src <= '9' )
    {
      *Dst |= *Src - '0';
    }
    else
    {
      *Dst |= *Src - 'A' + 10;
    }
    Src++;
    Dst++;
  }

  return SrcLength / 2;
}






/* 
 * ��������: �������е�ʮ��������ת�����ַ���
 * ���ò���:
 *          
 * ����ֵ  :
 * 
 */
extern u32 MDM_Bytes2String ( u8 *Dst, const u8 *Src, u16 SrcLength )
{
  const u8 TAB[] = "0123456789ABCDEF";
      
  if ( Dst && Src && SrcLength )
  {
    u32 i = 0;

    for ( i = 0; i < SrcLength; i++ )
    {
      *Dst++ = TAB[ *Src >> 4 ];
      *Dst++ = TAB[ *Src & 0x0F ];
      Src++;
    }
  
    *Dst = '\0';
    return SrcLength * 2;  
  }
  else
  {
    return 0;
  }
}



