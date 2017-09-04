




#include <stdio.h>
#include <string.h>

#include "stm32f4xx.h"

#include "sim_interface.h"

#include "tiza_sms_port.h"






/* SMS���պ��� */
ascii fromPhone[ 15 ];





/******************************************/
/*           SMS����Ϣ����                */
/******************************************/



/* 
 * ����������SMS�¼�����
 * ���ò�����
 *         (1)������룻
 *         (2)���������Ϣ,��ʱ�����Ϣ���ȣ�
 *         (3)�������ݣ�
 * ����ֵ  ��
 * 
 */
extern void sim_read_message_reply ( void* ipc_data, const ascii *sender_number, const ascii *contents )
{
	
  if ( sender_number && contents )
  {
    if ( sender_number[ 0 ] == '+' )    /* ������պ��� */ 
    {
  		strcpy ( ( char * )fromPhone, ( const char * )&sender_number[ 3 ] );
    }
  	else 
    if ( sender_number[ 0 ] == '8' && sender_number[ 1 ] == '6' )
    {
      strcpy ( ( char * )fromPhone, ( const char * )&sender_number[ 2 ] ); 
    }
    else   
    {
  		strcpy ( ( char * )fromPhone, (  const char * )sender_number );
    }

//    ProtocolParser ( contents, strlen ( ( const char * )contents ) );  
  }
}







/* 
 * ����������SMS�¼�����
 * ���ò�����
 *         (1)������룻
 *         (2)���������Ϣ,��ʱ�����Ϣ���ȣ�
 *         (3)�������ݣ�
 * ����ֵ  ��
 * 
 */
extern void sim_read_all_message_reply ( void* ipc_data, s16 index, const ascii *sender_number, const ascii *contents )
{
  if ( sender_number && contents )
  {
    if ( sender_number[ 0 ] == '+' )    /* ������պ��� */ 
    {
  		strcpy ( ( char * )fromPhone, ( const char * )&sender_number[ 3 ] );
    }
  	else 
    if ( sender_number[ 0 ] == '8' && sender_number[ 1 ] == '6' )
    {
      strcpy ( ( char * )fromPhone, ( const char * )&sender_number[ 2 ] ); 
    }
    else   
    {
  		strcpy ( ( char * )fromPhone, (  const char * )sender_number );
    }
    //ProtocolParser ( contents, strlen ( ( const char * )contents ) );  
  } 

  lgsm_sim_delete_all_messages ( NULL );
}








/* 
 * ����������������Ϣ��ʾ
 * ���ò�����
 *         
 * ����ֵ  ��
 * 
 */
extern void sim_incoming_message ( s32 index )
{
  if ( index >= 0 )
  {
    lgsm_sim_list_preferred_message ( NULL, SMS_STAT_REC_UNREAD );
  }
}



