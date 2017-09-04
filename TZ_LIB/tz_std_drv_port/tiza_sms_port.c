




#include <stdio.h>
#include <string.h>

#include "stm32f4xx.h"

#include "sim_interface.h"

#include "tiza_sms_port.h"






/* SMS接收号码 */
ascii fromPhone[ 15 ];





/******************************************/
/*           SMS短信息解析                */
/******************************************/



/* 
 * 功能描述：SMS事件处理
 * 引用参数：
 *         (1)来电号码；
 *         (2)短信相关信息,如时间和信息长度；
 *         (3)短信内容；
 * 返回值  ：
 * 
 */
extern void sim_read_message_reply ( void* ipc_data, const ascii *sender_number, const ascii *contents )
{
	
  if ( sender_number && contents )
  {
    if ( sender_number[ 0 ] == '+' )    /* 保存接收号码 */ 
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
 * 功能描述：SMS事件处理
 * 引用参数：
 *         (1)来电号码；
 *         (2)短信相关信息,如时间和信息长度；
 *         (3)短信内容；
 * 返回值  ：
 * 
 */
extern void sim_read_all_message_reply ( void* ipc_data, s16 index, const ascii *sender_number, const ascii *contents )
{
  if ( sender_number && contents )
  {
    if ( sender_number[ 0 ] == '+' )    /* 保存接收号码 */ 
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
 * 功能描述：输入信息提示
 * 引用参数：
 *         
 * 返回值  ：
 * 
 */
extern void sim_incoming_message ( s32 index )
{
  if ( index >= 0 )
  {
    lgsm_sim_list_preferred_message ( NULL, SMS_STAT_REC_UNREAD );
  }
}



