


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "stm32f4xx_conf.h"
//#include "timer.h"
#include "modem.h"
//#include "tz_gpio.h"




#define GSM_7BIT	0
#define GSM_8BIT	4
#define GSM_UCS2	8

#define MAX_7BITS_LEN 160
#define MAX_UCS2_LEN 	70
#define MAX_SMS_RETRY 3
#define PDU_BUF_SIZE	512



/* Sms data */
typedef struct 
{	
  bool pdu_sent;
  
  /* pdu data */
  u8 pdu[ PDU_BUF_SIZE ];

  /* Segment id */
  s32 seg_id;

  /* Message id */
  s32 msg_id;

  /* Are we sending (FALSE) or storing (TRUE) this sms data */
  bool storing;

} SMS_DataTypedef;



/* SMS parameters */
typedef struct {
	
  /* Service center address */
  u8 SCA[ 16 ];

  /* Remote tel number */
  u8 TPA[ 16 ];

  /* Type of address, tel number format 91 is international,
   * 81 is network defualt. */
  u8 TOA;

  /* User info identifier */
  u8 TP_PID;

  /* Code mode */
  u8 TP_DCS;

  /* Timestamp */
  u8 TP_SCTS[ 16 ];

  /* Sms message content */
  u8 TP_UD[ 161 ];

  /* Message length */
  s32   TP_UD_LEN;

  /* Sms index */
  u8 index;

  /* Is this storing sms parameter (TRUE) or submit (FALSE) */
  bool storing;

  /* Timestamp of the message */
  time_t timestamp;

  /* Message */
  ascii *message;

  /* Phone number */
  ascii *number;

} SMS_ParamsTypedef;


bool sms_txt_sent;
ascii str_msg[ MAX_7BITS_LEN ];
s32   str_len = 0;
const ascii* str_phoneNo;






/******************************************/
/*              内部函数[声明]            */
/******************************************/

static AtCmdHandler_Status SMS_Handler_SendTXT ( MDM_CmdTypedef *at, MDM_RspTypedef *atrsp );
extern AtCmdHandler_Status MDM_Utils_Handler   ( MDM_CmdTypedef *at, MDM_RspTypedef *atrsp );






/******************************************/
/*              内部变量[定义]            */
/******************************************/

extern MDM_DevTypedef *modem;

/* utility function*/
static MDM_CmdTypedef sms_commands[] 
= {
  /* Store */
  { SMS_STORE, "AT+CMGW=%s\r\n", 
  	1800, 3,
    SMS_Handler_SendTXT, SIM_READY, FALSE },

  /* Send sms */
  { SMS_SEND, "AT+CMGS=\"%s\"\r\n", 
  	900, 0,
    SMS_Handler_SendTXT, SIM_READY, FALSE },

  /* Init Select Message Service */
  { SMS_INIT, "AT+CSMS=128\r\n", 
  	100, 0,
    MDM_Utils_Handler, SIM_UNKNOWN, FALSE },

  { 0, NULL, 0, 0, NULL, SIM_UNKNOWN, FALSE }
}, *sms_command_p = sms_commands;







/**
 * @brief Checks if sms is coded in 7bits
 * @param str string to check
 * @return TRUE if sms is 7bits
 */
static bool SMS_Is7bits ( const ascii* str )
{
  s32 len = strlen ( ( const char* )str );
  s32 i = 0;
  
  for ( i = 0; i < len; i++ ) 
  {
    if ( ( str[ i ] & 0x80 ) != 0 )
    {
    	return FALSE;	
    }        
  }

  return TRUE;
}








/**
 * @brief Sends pdu to gsm modem (used for storing and sending sms)
 * @param ipc_data ipc data pointer
 * @param message message's contents
 * @param number recipient's number
 * @param store are we storing sms (TRUE) or sending (FALSE)
 */
extern void SMS_SendTXT ( void* ipc_data, const ascii *message, const ascii *number, bool store )
{
  /* read message from sms_db */
  if ( !message || !number || strlen( ( const char * )number ) == 0 ) 
  {
		/* Message needs contents and a number */
    return;
  }
  
  /* if it is not asccii 7bits */
  if ( SMS_Is7bits ( message ) )
  {
    str_len = strlen ( ( const char* )message ) + 1;
    sprintf ( ( char* )str_msg, "%s\x1A", message );
  
    /* check phone number format */
    str_phoneNo = number;
    sms_txt_sent = FALSE;
    MDM_PostAtCmdByID ( sms_command_p, SMS_SEND, ( const ascii* )str_phoneNo );
  }
}









/****************************************************
 *          C M G S toolbox           
 * This command sends an SM from the G24 to the network. The message reference value <mr> is 
 * returned to the G24 upon successful delivery of the message.
 * Valid <toda> will be any value between 128-255.
 * The header parameters in TEXT mode will be set according to CSMP settings.
 * The Set command validates the input parameters, sends the SM to network 
 * and reports the result of the operation to the G24.
 ****************************************************/


/**
 * @brief Directly send a sms message
 * @param ipc_data ipc data
 * @param message message's content
 * @param number recipient's phone number
 * @param want_report should report be sent
 */
extern void SMS_Command_SendMessage ( void* ipc_data, const ascii *message, const ascii *number, bool want_report )
{
  SMS_SendTXT ( ipc_data, message, number, FALSE );
}






/**
 * @brief Processes sms error responses.
 * @param modem modem whose error to process
 * @param scanner to get error from
 * @return TRUE if next command can be prosessed, false if command should be retried
 */
static AtCmdHandler_Status SMS_ProcessErrorResponse ( u16 errorCode )
{
  ascii reason[ 128 ];

  switch ( errorCode ) 
  {
    case 41:	/* check network status */
    case 331:	/* No network services */
      strcpy ( ( char* )reason, "No network service" );
      break;
            
    case 42:	/* network congestion retry 3 times */
      return AT_HANDLER_RETRY;

    case 1:
    case 96:
      strcpy ( ( char* )reason, "Destination address is uncorrect or missing" );
      /* need check destinate address */
      break;
            
    case 330:/* SMSC centre No uncorrect */
      strcpy ( ( char* )reason, "SMS service centre number is uncorrect or missing" );
      break;
            
    case 304:
      return AT_HANDLER_RETRY;

    default:
      sprintf ( ( char* )reason, "Error code is %ld\n", errorCode );
      break;
  } 

  return AT_HANDLER_DONE_ERROR;
}






/**
 * @brief General TXT handler for sms related reply handling
 * @param modem whose replies to handle
 * @return TRUE if replies were recognized
 */
extern AtCmdHandler_Status SMS_Handler_TXT ( MDM_CmdTypedef *at, MDM_RspTypedef *response )
{
  MDM_RSP_ID code = MDM_GetResultCode ( response );
  ascii *token = NULL;

  switch ( code )
  {
	  case MDM_OK:
#if 0
	    /* Send storing sms reply */
	    if ( data->storing )
	    {
	      /* Check that we have a valid message index returned
	       * with +CMGW */
	      if ( data->msg_id < 0 ) 
	      {
					/* "FAILED TO GET WRITTEN MESSAGE'S INDEX" */
	      } 
	      else 
	      {
          modem->sim->store_message_reply ( NULL, data->msg_id );
	      }
	    } 
	    else	/* Send sending sms reply */
#endif
	    {
        if ( modem && modem->sms && modem->sms->send_message_reply ) 
	      {
          modem->sms->send_message_reply ( NULL );
	      }

	      if ( modem && modem->sms && modem->sms->message_sent )
        {
          modem->sms->message_sent ( TRUE, NULL );
        }            
	    }

	    return AT_HANDLER_DONE;      
	
	  case MDM_ERROR:
      token = MDM_StrStr ( response, MDM_Sym2Str ( symbol_p, MDM_CMGW ) );
      if ( token )
      {
        return AT_HANDLER_NEED_MORE;    
      }

      token = MDM_StrStr ( response, MDM_Sym2Str ( symbol_p, MDM_CMS_ERROR ) );
      if ( token )
      {
        SMS_ProcessErrorResponse ( atoi ( ( const char* )token ) );
        return AT_HANDLER_DONE_ERROR;
      }

      if ( modem && modem->sms && modem->sms->message_sent ) 
      {
        /* Add-ons error code */ 
        modem->sms->message_sent ( FALSE, "SOME ERROR OCCURRED" );
	    }  
	    return AT_HANDLER_DONE_ERROR;        
  }

  return AT_HANDLER_DONT_UNDERSTAND;
}




extern u8 sms_sent_reset = 0;
extern	time_t Rtc_new_time;

#define RTC_ALM_COUNT  (((RTC->ALRH)<<16) + RTC->ALRL)
#define RTC_COUNT  (((RTC->CNTH)<<16) + RTC->CNTL)

//extern s32 GPIO_ReadPINlevel ( s32 Handle );

/**
 * @brief Handler for sending sms at command's replies.
 * @return TRUE if replies were recognized
 */
static AtCmdHandler_Status SMS_Handler_SendTXT ( MDM_CmdTypedef *at, MDM_RspTypedef *response )
{
  MDM_RSP_ID code = MDM_GetResultCode ( response );
  //u8 i = 0;
  //u32 bkp_time[10] = {0};

	/* If txt was sent, 
     use a different handler to handle reply to pdu */
	if ( sms_txt_sent == TRUE )
	{
	 	return SMS_Handler_TXT ( at, response );
	}

  if ( code == MDM_ERROR )
  {	 
      return AT_HANDLER_DONE_ERROR;
	  
  }
  else if ( MDM_StrStr ( response, ">" ) )
  {
    if ( modem && modem->serial && modem->serial->write_data )
    {
      /* Modem is ready to receive txt */
      /* Directly write txt to modem */
      modem->serial->write_data ( AT_Flow_Type, ( u8 * )str_msg, str_len );
  
      /* Mark that txt was sent */
      sms_txt_sent = TRUE;
   
	 
    }

    /* Wait for ok to the txt */
    return AT_HANDLER_NEED_MORE;  
  }
	
	return AT_HANDLER_DONT_UNDERSTAND;
}







