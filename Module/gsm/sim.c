





#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "modem.h"
#include "at_error.h"






typedef struct 
{
  ascii imsi_code[ 16 ];
  ascii cnum_code[ 20 ];
} SIM_InfoTypedef;


/* Common routines */
#define IS_AF(c)  ((c >= 'A') && (c <= 'F'))
#define IS_af(c)  ((c >= 'a') && (c <= 'f'))
#define IS_09(c)  ((c >= '0') && (c <= '9'))
#define ISVALIDHEX(c)  IS_AF(c) || IS_af(c) || IS_09(c)
#define ISVALIDDEC(c)  IS_09(c)
#define CONVERTDEC(c)  (c - '0')



/******************************************/
/*              外部函数[声明]            */
/******************************************/

extern void SMS_SendTXT ( void* ipc_data, const ascii *message, const ascii *number, bool store );




/******************************************/
/*              内部函数[声明]            */
/******************************************/

static AtCmdHandler_Status SIM_Handler_GetSIMinfo               ( MDM_CmdTypedef *at, MDM_RspTypedef *atrsp );
static AtCmdHandler_Status SIM_Handler_QueryPINstatus           ( MDM_CmdTypedef *at, MDM_RspTypedef *atrsp );
static AtCmdHandler_Status SIM_Handler_QueryIMSI                ( MDM_CmdTypedef *at, MDM_RspTypedef *atrsp );
static AtCmdHandler_Status SIM_Handler_PIN                      ( MDM_CmdTypedef *at, MDM_RspTypedef *atrsp );
static AtCmdHandler_Status SIM_Handler_Read                     ( MDM_CmdTypedef *at, MDM_RspTypedef *atrsp );
static AtCmdHandler_Status SIM_Handler_List                     ( MDM_CmdTypedef *at, MDM_RspTypedef *atrsp );
static AtCmdHandler_Status SIM_Handler_Delete                   ( MDM_CmdTypedef *at, MDM_RspTypedef *atrsp );
static AtCmdHandler_Status SIM_Handler_DeleteAllMessages        ( MDM_CmdTypedef *at, MDM_RspTypedef *atrsp );
static AtCmdHandler_Status SIM_Handler_QueryServiceCenter       ( MDM_CmdTypedef *at, MDM_RspTypedef *atrsp );
static AtCmdHandler_Status SIM_Handler_SetServiceCenter         ( MDM_CmdTypedef *at, MDM_RspTypedef *atrsp );
static AtCmdHandler_Status SIM_Handler_GetSubscriberNumbers     ( MDM_CmdTypedef *at, MDM_RspTypedef *atrsp );
static AtCmdHandler_Status SIM_Handler_SendMessage              ( MDM_CmdTypedef *at, MDM_RspTypedef *atrsp );
static AtCmdHandler_Status SIM_Handler_SelectPhoneBookMemory    ( MDM_CmdTypedef *at, MDM_RspTypedef *atrsp );
static AtCmdHandler_Status SIM_Handler_SetPhoneBookEntry        ( MDM_CmdTypedef *at, MDM_RspTypedef *atrsp );
static AtCmdHandler_Status SIM_Handler_SetSIMOwnNumber          ( MDM_CmdTypedef *at, MDM_RspTypedef *atrsp );
static AtCmdHandler_Status SIM_Handler_RingIndicator ( MDM_CmdTypedef *at, MDM_RspTypedef *response );





                                      


/******************************************/
/*              内部变量[定义]            */
/******************************************/

extern MDM_DevTypedef *modem;

static MDM_CmdTypedef sim_commands[] 
= {
  /* Query pin status */
  { PIN_QUERY, "AT+CPIN?\r\n", 
  	800, 4,
    SIM_Handler_QueryPINstatus, SIM_UNKNOWN, FALSE },
      
  /* Query subscriber numbers */
  { QUERY_SUBSCRIBER_NUMBERS, "AT+CNUM\r\n", 
  	2000, 2,
    SIM_Handler_GetSubscriberNumbers, SIM_READY, FALSE },

  /* Query subscriber numbers */
  { QUERY_SUBSCRIBER_NUMBERS_FROM_PHONEBOOK, "AT+CPBR=1\r\n", 
  	1000, 1,
    SIM_Handler_GetSubscriberNumbers, SIM_READY, FALSE },

  /* Setup pin */
  { PIN_SETUP, "AT+CPIN=\"%s\"\r\n", 
  	2000, 2,
    SIM_Handler_PIN, SIM_UNKNOWN, FALSE },

  /* Change pin */
  { PIN_CHANGE, "AT+CPIN=%s\r\n", 
  	2000, 2,
    SIM_Handler_PIN, SIM_UNKNOWN, FALSE },

  /* Setup puk */
  { PUK_SETUP, "AT+CPIN=%s\r\n",  
  	2000, 2,
    SIM_Handler_PIN, SIM_UNKNOWN, FALSE },

  /* Get IMSI */
  { IMSI_QUERY, "AT+CIMI\r\n",              
  	800,  0,
    SIM_Handler_QueryIMSI, SIM_READY },
    
  /* Read sms */
  { SMS_READ, "AT+CMGR=%s\r\n",           
  	500,  0,
    SIM_Handler_Read, SIM_READY, FALSE },

  /* Delete all sms */
  { SMS_DELETE_ALL, "AT+CMGD=1,4\r\n",           
  	1800,  1,
    SIM_Handler_DeleteAllMessages, SIM_READY, FALSE },

  /* List SMS messages from preferred store */
  { SMS_LIST, "AT+CMGL=%s\r\n",           
  	1000,  0,
    SIM_Handler_List, SIM_READY, FALSE },

  /* Delete sms */
  { SMS_DELETE, "AT+CMGD=%s\r\n",           
  	500,  0,
    SIM_Handler_Delete, SIM_READY, FALSE },

  /* Select Phone Book Memory ON SIM own numbers (MSISDNs) list 
  (reading this storage is also available through +CNUM). */
  { PHONEBOOK_ENTRY_SET, "AT+CPBS=\"%s\"\r\n",           
  	500,  0,
    SIM_Handler_SelectPhoneBookMemory, SIM_READY, FALSE },

  /* Delete sms */
  { PHONEBOOK_SIM_OWN_NUMBERS_SET, "AT+CPBW=1,\"%s\",129\r\n",           
  	500,  0,
    SIM_Handler_SetPhoneBookEntry, SIM_READY, FALSE },

  /* Query sms service center */
  { SMS_CENTER_QUERY, "AT+CSCA?\r\n",             
  	3000, 2,
    SIM_Handler_QueryServiceCenter, SIM_READY, FALSE },

  /* Set sms service center */
  { SMS_SET_CENTER, "AT+CSCA=%s\r\n",           
  	8000, 2,
    SIM_Handler_SetServiceCenter, SIM_READY, FALSE },

  /* Send message from storage */
  { SIM_SEND, "AT+CMSS=%s\r\n",           
  	3000, 0, 
  	SIM_Handler_SendMessage, SIM_READY, FALSE },

  /*SMS Ring Indicator*/
  { SIM_RING_INDICATOR, "AT#E2SMSRI=%s\r\n",           
  	3000, 0, 
  	SIM_Handler_RingIndicator, SIM_READY, FALSE },
 

  { 0, NULL, 0, 0, NULL, SIM_UNKNOWN, FALSE }
}, *sim_command_p = sim_commands;





static ascii *imsi = NULL;
static ascii *cnum = NULL;
static SIM_InfoTypedef simInfo = { "", "" };




extern void SIM_Command_GetSIMCNUM ( void* ipc_data,ascii *pcnum )
{
	
	 sim_commands[ 2 ].handler = ( AtCmdHandler )&SIM_Handler_GetSIMinfo;
	 MDM_PostAtCmdByID ( sim_command_p, QUERY_SUBSCRIBER_NUMBERS_FROM_PHONEBOOK, 
                                NULL ); 
	 memcpy(pcnum,simInfo.cnum_code,sizeof(simInfo.cnum_code));

}



/**
 * @brief Changes auth (pin) status, sends auth_status signal if
 * status was changed.
 * @param status new pin status
 * @param send_reply should reply to get_auth_status ipc method be sent as well
 */
static void SIM_ChangeAuthStatus ( void* ipc_data, SIM_Status status, const ascii *message, bool send_reply )
{

#if 0
  if ( modem && status != modem->sim_status )
  {
    printf ( "Changing auth status from %d to %d.\r\n", ( s32 )modem->sim_status, ( s32 )status );
  }
#endif
 
  /* Send a reply to get auth_status ipc method if necessary */
  if ( send_reply && modem && modem->sim && modem->sim->get_auth_status_reply )
  {
			modem->sim->get_auth_status_reply ( NULL, ( const ascii* )message );  		
  }
  
			//printf ( " get_auth_status_reply Changing auth status from %d to %d.\r\n", ( s32 )modem->sim_status, ( s32 )status );

  /* If sim status has changed, send a signal */
  if ( modem && modem->sim && modem->sim->auth_status &&( status != modem->sim_status ) )
  {
			//printf ( " auth_status Changing auth status from %d to %d.\r\n", ( s32 )modem->sim_status, ( s32 )status );
			modem->sim->auth_status ( status );
  }

  
  /* finally change modem's sim status */
  MDM_ChangeSIMstatus ( status );
}






/**
 * Sends a reply to all pin related ipc method calls
 * @param modem modem pointer
 * @param cmd_id command/method call id
 */
extern void SIM_SendPINreply ( MDM_CmdTypedef *at )
{
  switch ( at->cmd_id )
  {
    case PIN_SETUP:
      if ( modem && modem->sim && modem->sim->send_auth_code_reply )
      {
        modem->sim->send_auth_code_reply ( NULL );
      }
      break;
        
    case PIN_CHANGE:
      if ( modem && modem->sim && modem->sim->change_auth_code_reply )
      {
        modem->sim->change_auth_code_reply ( NULL );
      }
      break;
        
    case PUK_SETUP:
      if ( modem && modem->sim && modem->sim->unlock_reply )
      {
        modem->sim->unlock_reply ( NULL );
      }
      break;
  }
}






/****************************************************
 *          C P I N toolbox           
 * The Set command sends the password to the G24 that is necessary before it can be 
 * operated (SIM PIN or SIM PUK). If there is no PIN request pending, no action is 
 * taken towards the G24, and an error message, +CME ERROR, is returned to the terminal. 
 * The Set command issued gives the code (SIM PIN or SIM PUK) corresponding to 
 * the error code required or returned as the result of the Read command. For example, 
 * if the SIM PIN is blocked, the error code 11 or "SIM PIN required" is returned. The 
 * user must then issue the Set command with the SIM PIN.
 ****************************************************/

/**
 * @brief AtCommand to setup pin value(s)
 * @param pin_code pin code(s) to set
 */
extern void SIM_Command_SetupPIN ( void* ipc_data, const ascii* pin_code )
{
  MDM_PostAtCmdByID ( sim_command_p, 
                      PIN_SETUP, 
                      pin_code );
}




/* 
 * 功能描述:
 * 引用参数:
 *          
 * 返回值  :
 * 
 */
extern void SIM_Command_Unlock( void* ipc_data, const ascii* puk, const ascii* new_pin )
{
  ascii params[ 36 ];

  sprintf ( ( char* )params, "\"%s\",\"%s\"", puk, new_pin );
  MDM_PostAtCmdByID ( sim_command_p, 
                      PUK_SETUP, 
                      params );
}






/* 
 * 功能描述:
 * 引用参数:
 *          
 * 返回值  :
 * 
 */
extern void SIM_Command_ChangeAuthCode ( void* ipc_data, const ascii* old_pin, const ascii* new_pin )
{
  ascii param[ 36 ];
  
  sprintf ( ( char* )param, "\"%s\",\"%s\"", old_pin, new_pin );
  MDM_PostAtCmdByID ( sim_command_p, 
                      PIN_CHANGE, 
                      param );
}






/**
 * @brief Handler to modems response from setupPin command
 * @param modem modem whose response to handle
 * @return true if responses were recognized
 */
static AtCmdHandler_Status SIM_Handler_PIN ( MDM_CmdTypedef *at, MDM_RspTypedef *response )
{
  AtCmdHandler_Status at_status = AT_HANDLER_DONT_UNDERSTAND;
  MDM_RSP_ID code = MDM_GetResultCode ( response );

  switch ( code )
  {
    case MDM_OK:
        SIM_SendPINreply ( at );
        SIM_ChangeAuthStatus ( NULL, SIM_READY, "SIM_READY", FALSE );
        at_status = AT_HANDLER_DONE;
        break;

    case MDM_ERROR:
        if ( modem->sim_status == SIM_NEED_PUK )
        {
          SIM_ChangeAuthStatus ( NULL, SIM_READY, "SIM_READY", FALSE );
        }
/* 
What should be done on each status
SIM_UNKNOWN=0 -> we're still at unknown state
SIM_MISSING_SIM -> we still don't have a sim card
SIM_NEED_PIN -> pin was wrong, we still need pin
SIM_NEED_PUK -> puk code was wrong, we still need puk
SIM_READY -> we never needed any code so nothing changed */
        at_status = AT_HANDLER_DONE_ERROR;
        break;

    default:
        break;
  }
  return at_status;
}






/****************************************************
 *          C P I N toolbox           
 * The Read command returns an alphanumeric string indicating the status 
 * of the SIM card, and whether a password is required or not. This is an independent 
 * SIM card lock status check only, and does not check the phone lock status.
 *  Table 3-171: SIM Card Errors
 * Error                    Description
 * 10 SIM not inserted      SIM Card is not inserted
 * 11 SIM PIN required      SIM Card waiting for SIM PIN to be entered
 * 12 SIM PUK required      SIM PIN is blocked
 * 13 SIM failure           SIM Card is permanently blocked
 * 17 SIM PIN2 required     SIM Card is waiting for SIM PIN2 to be entered
 * 18 SIM PUK2 required     SIM PIN2 is blocked
 ****************************************************/

/**
 * @brief AtCommand to query pin status
 * @param modem modem whose response to handle
 */
extern void SIM_Command_QueryPINstatus ( void* ipc_data )
{

  MDM_PostAtCmdByID ( sim_command_p, 
                      PIN_QUERY, 
                      NULL );
}







/**
 * @brief Handler to modems response from queryPinStatus command
 * @param modem modem whose response to handle
 * @return true if responses were recognized
 */
static AtCmdHandler_Status SIM_Handler_QueryPINstatus ( MDM_CmdTypedef *at, MDM_RspTypedef *response )
{
  MDM_RSP_ID code = MDM_GetResultCode ( response );

  /* Modem didn't reply or replied with an error */

  if ( code == MDM_ERROR )
  {
    s32 errorCode = 0;
    
    if ( TRUE == MDM_Utils_ParseCMEerror ( response, ( s32* )&errorCode ) )
    {
	  
      switch ( errorCode )
      {
        case SIM_ERR_NO_CARD:
		
          SIM_ChangeAuthStatus ( NULL, SIM_MISSING, "SIM_MISSING", TRUE );
          return AT_HANDLER_DONE_ERROR;

        default:
		
          SIM_ChangeAuthStatus ( NULL, SIM_INIT, "SIM_INIT", TRUE );
          return AT_HANDLER_RETRY;
      }
    }
  }
  else if ( code == MDM_TIME_OUT )
  {

    SIM_ChangeAuthStatus ( NULL, SIM_ERROR, "SIM_ERROR", TRUE );
    return AT_HANDLER_DONE_ERROR;
  }
  else
  {
    ascii *token = MDM_StrStr ( response, MDM_Sym2Str ( symbol_p, MDM_CPIN ) );

    if ( token )
    {
      if ( strstr ( ( const char* )token, "READY" ) )
      {
        
				lgsm_sim_get_imsi(NULL);

				SIM_ChangeAuthStatus ( NULL, SIM_READY, "SIM_READY", TRUE );

				
        
				return AT_HANDLER_DONE;
      }
      else 
      if ( strstr ( ( const char* )token, "SIM" ) )
      {
        if ( strstr ( ( const char* )token, "PIN" ) )
        {
          SIM_ChangeAuthStatus ( NULL, SIM_NEED_PIN, "SIM_NEED_PIN", TRUE );  
        }
        else 
        if ( strstr ( ( const char* )token, "PUK" ) )
        {
          SIM_ChangeAuthStatus ( NULL, SIM_NEED_PUK, "SIM_NEED_PUK", TRUE );  
        }
  
        return AT_HANDLER_DONE; 
      }
	   
    }
	
	  
  }

  return AT_HANDLER_DONT_UNDERSTAND;
}










/* 
 * 功能描述:
 * 引用参数:
 *          
 * 返回值  :
 * 
 */
static bool SIM_Command_GetSIMinfoReply ( void *ipc_data )
{
  if ( modem && modem->sim && modem->sim->get_sim_info_reply )
  {
    if ( imsi != NULL )
    {
      modem->sim->get_sim_info_reply ( NULL, imsi);
    }
    return TRUE;  
  }
  else
  {
    return FALSE;
  }
}






/* 
 * 功能描述:
 * 引用参数:
 *          
 * 返回值  :
 * 
 */
extern void SIM_Command_GetSIMinfo ( void* ipc_data )
{
  if ( imsi != NULL)
  {

  		printf("[SIM EVENT] IMSI query trigger IMSI ") ;
    	SIM_Command_GetSIMinfoReply ( ipc_data );
  }
  else
  {

    	printf ( "\r\n[SIM EVENT] IMSI query trigger\r\n" );
    	sim_commands[ 6 ].handler = ( AtCmdHandler )&SIM_Handler_GetSIMinfo;
    	MDM_PostAtCmdByID ( sim_command_p, 
                        IMSI_QUERY, 
                        NULL );
  }
}


extern void SIM_Command_GetImsi ( void* ipc_data )
{
  
    MDM_PostAtCmdByID ( sim_command_p, 
                        IMSI_QUERY, 
                        NULL );
}





/* 
 * 功能描述:
 * 引用参数:
 *          
 * 返回值  :
 * 
 */
 static AtCmdHandler_Status SIM_Handler_GetSIMinfo (  MDM_CmdTypedef *at, MDM_RspTypedef *response )
{
  AtCmdHandler_Status status = AT_HANDLER_DONT_UNDERSTAND;

  switch ( at->cmd_id )
  {
    case IMSI_QUERY:
      status = SIM_Handler_QueryIMSI ( at, response );
      switch ( status )
      {
        case AT_HANDLER_DONE_ERROR:
        case AT_HANDLER_ERROR:
            sim_commands[ 6 ].handler = ( AtCmdHandler )&SIM_Handler_QueryIMSI;
            return AT_HANDLER_DONE_ERROR;
            
        case AT_HANDLER_DONE:
            sim_commands[ 6 ].handler = ( AtCmdHandler )&SIM_Handler_QueryIMSI;
            sim_commands[ 2 ].handler = ( AtCmdHandler )&SIM_Handler_GetSIMinfo;
						//这里是	AT+CPBR	@goly
            MDM_PostAtCmdByID ( sim_command_p, 
                                QUERY_SUBSCRIBER_NUMBERS_FROM_PHONEBOOK, 
                                NULL );
            status = AT_HANDLER_DONE;
            break;
            
        case AT_HANDLER_DONT_UNDERSTAND:
        case AT_HANDLER_NEED_MORE:
        case AT_HANDLER_RETRY:
            return status;
      }
      break;

    case QUERY_SUBSCRIBER_NUMBERS_FROM_PHONEBOOK:
      status = SIM_Handler_GetSubscriberNumbers ( at, response );
      switch ( status )
      {
        case AT_HANDLER_DONE_ERROR:
        case AT_HANDLER_ERROR:
            sim_commands[ 2 ].handler = ( AtCmdHandler )&SIM_Handler_GetSubscriberNumbers;
            return AT_HANDLER_DONE_ERROR;
            
        case AT_HANDLER_DONE:
            sim_commands[ 2 ].handler = ( AtCmdHandler )&SIM_Handler_GetSubscriberNumbers;
            SIM_Command_GetSIMinfoReply ( NULL );
            break;
            
        case AT_HANDLER_DONT_UNDERSTAND:
        case AT_HANDLER_NEED_MORE:
        case AT_HANDLER_RETRY:
            return status;
      }
      break;
  }  

  return status;
}









/**
 * @brief Handler for replies sent by the gsm modem when a service center query
 * command is sent
 *
 * @param modem modem whose replies to interpret
 * @return true if replies were correct
 */
static AtCmdHandler_Status SIM_Handler_GetSubscriberNumbers ( MDM_CmdTypedef *at, MDM_RspTypedef *response )
{
  AtCmdHandler_Status status = AT_HANDLER_DONT_UNDERSTAND;
  MDM_RSP_ID code = MDM_GetResultCode ( response );

//  if ( code == MDM_OK )
  {
    ascii *token = NULL;

    token = MDM_StrStr ( response, MDM_Sym2Str ( symbol_p, MDM_CPBR ) );
//    token = MDM_StrStr ( response, MDM_Sym2Str ( symbol_p, MDM_CNUM ) );
     memcpy(simInfo.cnum_code,"13912999149",sizeof("13912999149"));
	           cnum = simInfo.cnum_code;
		  //printf("\r\n cnum =%s\r\n",cnum);
      return AT_HANDLER_DONE; 
//    if ( token ) 
//    {
//      token = ( ascii * )strtok ( ( char * )token, "," );
//      if ( token ) 
//      {
//        token = ( ascii * )strtok ( NULL, "," );
//        if ( token )
//        {
//          token++;
//          strncpy ( ( char * )simInfo.cnum_code, ( const char * )token, ( strlen ( ( const char * )token ) - 1 ) );
//          cnum = simInfo.cnum_code;
//		  printf("\r\n cnum =%s\r\n",cnum);
//          return AT_HANDLER_DONE; 
//        }
//      }
//    }   
  }
  
  if ( code == MDM_TIME_OUT )
  {
    if ( modem && modem->sim && modem->sim->get_sim_info_error )
    {
      modem->sim->get_sim_info_error ( NULL );
    }     
  } 

  return status;
}








/**
 * Handler for reply to query_imsi command.
 *
 * @param modem pointer to modem struct
 * @param response response sent by gsm modem
 * @return AT_HANDLER_DONE when response is recognized
 */
static AtCmdHandler_Status SIM_Handler_QueryIMSI ( MDM_CmdTypedef *at, MDM_RspTypedef *response )
{
  AtCmdHandler_Status status = AT_HANDLER_DONT_UNDERSTAND;
  MDM_RSP_ID code = MDM_GetResultCode ( response );

//  printf("response->indicator=%s\r\n",response->indicator);
  if ( code == MDM_TIME_OUT )
  {
    if ( modem && modem->sim && modem->sim->get_sim_info_error )
    {
      modem->sim->get_sim_info_error ( NULL );
    }    
    return AT_HANDLER_ERROR;
  }
  else
  if ( code == MDM_ERROR )
  {
    if ( modem && modem->sim && modem->sim->get_sim_info_error )
    {
      modem->sim->get_sim_info_error ( NULL );
    }    
    return AT_HANDLER_ERROR;
  }
  else
  if ( code == MDM_OK )
  {
    bool is_imsi = TRUE;
    ascii *token = NULL;

    {

#if defined __USE_G24__
      token = ( ascii * )MDM_StrStr ( response, ": " );
#else
      token = ( ascii * )MDM_StrStr ( response, "\r\n" );
#endif
      if ( token )
      {
        u8 i = 0;

        token += 2;
        for ( i = 0; i < 15; i++ ) 
        {
          if ( !ISVALIDDEC ( token[ i ] ) ) 
          {
            //is_imsi = FALSE;
            break;
          }
        }
//		printf("is_imsi=%d %c %c %c\r\n",is_imsi,token[0],token[1],token[3]);
        if ( is_imsi == TRUE )
        {
          strncpy ( ( char * )simInfo.imsi_code, ( const char * )token, 15 );
          imsi = simInfo.imsi_code;
					
					SIM_Command_GetSIMinfoReply(NULL);
          return AT_HANDLER_DONE;       
        }
      }
    }  
  }

  return status;
}







/****************************************************
 *          C S C A toolbox           
 * This command handles the selection of the SCA and the TOSCA. The SCA is the phone number 
 * of the SC (Service Center). The TOSCA can be 129 (local) or 145 (international), where 129 is 
 * the default value. The TOSCA parameter of the Set command is optional, and can be omitted. If 
 * the SCA parameter of the Set command is prefixed by the "+" character, 
 * it indicates that TOSCA is 145.
 ****************************************************/

/**
 * @brief Creates at command to set service center
 *
 * @param modem modem to send command to
 * @param centerNo center number
 */
extern void SIM_Command_SetServiceCenter ( void* ipc_data, const ascii* centerNo )
{
  ascii param[ 32 ];
  s32 format = 0;
  
  if ( strstr ( ( const char * )centerNo, "+" ) )
  {
    /* Internation format */
    format = 145;
  }
  else
  {
    /* national format */
    format = 129;
  }

  sprintf ( ( char* )param, "\"%s\",%d", centerNo, format );
  MDM_PostAtCmdByID ( sim_command_p, 
                      SMS_SET_CENTER, 
                      param );
}









/**
 * @brief Handler for replies sent by the gsm modem when a set service centre
 * command is sent
 *
 * @param modem modem whose replies to interpret
 * @return true if replies were correct
 */
static AtCmdHandler_Status SIM_Handler_SetServiceCenter ( MDM_CmdTypedef *at, MDM_RspTypedef *response )
{
  MDM_RSP_ID code = MDM_GetResultCode ( response );

  switch ( code )
  {
    case MDM_OK:
      if ( modem && modem->sim && modem->sim->set_service_center_number_reply )
      {
        modem->sim->set_service_center_number_reply ( NULL );
      }
      return AT_HANDLER_DONE;

    case MDM_ERROR:
    {
      ascii *token = MDM_StrStr ( response, MDM_Sym2Str ( symbol_p, MDM_CMS_ERROR ) );

      if ( token )
      {
        printf ( "\r\nERROR %s\r\n", token );      
      }

      if ( modem && modem->sim && modem->sim->set_service_center_number_error )
      {
        modem->sim->set_service_center_number_error ( NULL );
      }
      return AT_HANDLER_DONE_ERROR;    
    }

    default:
      break;
  }

  return AT_HANDLER_DONT_UNDERSTAND;
}






/**
 * @brief Creates at command to query service center
 *
 * @param modem modem to send command to
 */
extern void SIM_Command_QueryServiceCenter ( void* ipc_data )
{
  MDM_PostAtCmdByID ( sim_command_p, 
                      SMS_CENTER_QUERY, 
                      NULL );
}






/**
 * @brief Handler for replies sent by the gsm modem when a service center query
 * command is sent
 * @param modem modem whose replies to interpret
 * @return true if replies were correct
 */
static AtCmdHandler_Status SIM_Handler_QueryServiceCenter ( MDM_CmdTypedef *at, MDM_RspTypedef *response )
{
  MDM_RSP_ID code = MDM_GetResultCode ( response );

  if ( code == MDM_OK )
  {
    ascii *token = MDM_StrStr ( response, MDM_Sym2Str ( symbol_p, MDM_CSCA ) );

    if ( token )
    {
      token = ( ascii * ) strtok ( ( char * )token, "\"" );
      if ( token )
      {
        token = ( ascii * ) strtok ( NULL, "\"" );
        if ( token )
        {
          if ( modem && modem->sim && modem->sim->get_service_center_number_reply )
          {
            modem->sim->get_service_center_number_reply ( NULL, token );
          }        
        }  
      }
    }
    return AT_HANDLER_DONE;
  }
  else
  {
    return AT_HANDLER_RETRY;
  }
}









/****************************************************
 *          C M G D toolbox           
 * Deleting Messages.
 ****************************************************/

/**
 * @brief Creates the at command to delete a sms message.
 * @param modem modem whose sms to delete
 * @param pos sms message's index
 */
extern void SIM_Command_Delete ( void* ipc_data, s32 pos )
{
  ascii pos_str[ 8 ]; 
  
  if ( pos == 0 )
  {
    pos = 4;
    sprintf ( ( char* )pos_str, "%d", pos );  
  }
  else if ( pos > 0 )
  {
    sprintf ( ( char* )pos_str, "%d", pos );
  }

  MDM_PostAtCmdByID ( sim_command_p, 
                      SMS_DELETE, 
                      pos_str );
}







/**
 * @brief Handler for delete sms command's replies.
 *
 * @param modem whose replies to handle
 * @return true if response was recognized
 */
static AtCmdHandler_Status SIM_Handler_Delete ( MDM_CmdTypedef *at, MDM_RspTypedef *response )
{
  MDM_RSP_ID code = MDM_GetResultCode ( response );
   
  if ( code == MDM_OK )
  {
    if ( modem && modem->sim && modem->sim->delete_message_reply )
    {
      modem->sim->delete_message_reply ( NULL );
    }
    return AT_HANDLER_DONE;
  }
  else if ( code == MDM_ERROR )
  {
    return AT_HANDLER_DONE_ERROR;
  }

  return AT_HANDLER_DONT_UNDERSTAND;
}





/**
 * @brief Creates the at command to delete a sms message.
 * @param modem modem whose sms to delete
 * @param pos sms message's index
 */
extern void SIM_Command_DeleteAllMessages ( void* ipc_data )
{
  MDM_PostAtCmdByID ( sim_command_p, 
                      SMS_DELETE_ALL, 
                      NULL );
}






/**
 * @brief Handler for delete sms command's replies.
 *
 * @param modem whose replies to handle
 * @return true if response was recognized
 */
static AtCmdHandler_Status SIM_Handler_DeleteAllMessages ( MDM_CmdTypedef *at, MDM_RspTypedef *response )
{
  MDM_RSP_ID code = MDM_GetResultCode ( response );
   
  if ( code == MDM_OK )
  {
    if ( modem && modem->sim && modem->sim->delete_message_reply )
    {
      modem->sim->delete_message_reply ( NULL );
    }
    return AT_HANDLER_DONE;
  }
  else if ( code == MDM_ERROR )
  {
    return AT_HANDLER_RETRY;
  }

  return AT_HANDLER_DONT_UNDERSTAND;
}





/****************************************************
 *          C M S S toolbox           
 * Writing messages into the database, with or without destination address. In TEXT mode, the 
 * header parameters will be set according to CSMP settings.
 ****************************************************/


/**
 * @brief Sends a sms from database
 * Fetches specified sms from the database, prepares it for sending and
 * calls gsmd_sms_command_send to actually send the message.
 * @param modem modem whose message to send
 * @param msgid id of the mssage to send
 */
extern void SIM_Command_SendStoredMessage ( void* ipc_data, s32 index )
{
  /* want_report is currently ignored */
  ascii param[ 8 ];

  sprintf ( ( char* )param, "%d", index );
  MDM_PostAtCmdByID ( sim_command_p, 
                      SIM_SEND, 
                      param );
}


extern void SIM_Command_RingIndicator ( void* ipc_data, s32 time )
{
  /* want_report is currently ignored */
  ascii param[ 8 ];

  sprintf ( ( char* )param, "%d", time );
  MDM_PostAtCmdByID ( sim_command_p, 
                      SIM_RING_INDICATOR, 
                      param );
}


/* 
 * 功能描述:
 * 引用参数:
 *          
 * 返回值  :
 * 
 */
static AtCmdHandler_Status SIM_Handler_SendMessage ( MDM_CmdTypedef *at, MDM_RspTypedef *response )
{
  AtCmdHandler_Status status = AT_HANDLER_DONT_UNDERSTAND;
  return status;
}


static AtCmdHandler_Status SIM_Handler_RingIndicator ( MDM_CmdTypedef *at, MDM_RspTypedef *response )
{
  AtCmdHandler_Status status = AT_HANDLER_DONT_UNDERSTAND;
  return status;
}








/****************************************************
 *          C M G R toolbox           
 * These commands handle the reading of SMs. The command displays the message in location 
 * <index> of the preferred message storage <mem1> (selected using the +CPMS command). 
 * If the status of the message is "RECEIVED UNREAD", the +CMGR command changes the status to 
 * "RECEIVED READ". The +MMGR command does not change the message status.
 ****************************************************/

/**
 * @brief Creates the at command to read a sms message
 * @param pos index of the message to read
 */
extern void SIM_Command_Read ( void* ipc_data, s32 pos )
{
  ascii pos_str[ 8 ]; 

  sprintf ( ( char* )pos_str, "%d", pos );
  MDM_PostAtCmdByID ( sim_command_p, 
                      SMS_READ, 
                      pos_str );
}






/**
 * @brief Handler for replies sent by the gsm modem when a read command is sent.
 * @param at at command context
 * @param response modem's response
 * @return handler status
 */
static AtCmdHandler_Status SIM_Handler_Read ( MDM_CmdTypedef *at, MDM_RspTypedef *response )
{
  MDM_RSP_ID code = MDM_GetResultCode ( response );
  ascii *token = NULL;
  ascii telnumber[ 15 ], telinfo[ 160 ];
  
  if ( code == MDM_OK )
  {
    token = MDM_StrStr ( response, MDM_Sym2Str ( symbol_p, MDM_CMGR ) );
    if ( token )
    {
      token = ( ascii * ) strstr ( ( const char * )token, "REC UNREAD" );
      if ( token )
      {
        token = ( ascii * ) strstr ( ( char * )token, "," );
        if ( ++token )
        {
          token = ( ascii * ) strtok ( ( char * )token, "\"" );
          if ( token )
          {
            memset ( telnumber, 0, 15 );
            strncpy ( ( char * )telnumber, ( const char * )token, 15 );
            token = ( ascii * ) strtok ( ( char * )NULL, "\n" );
            if ( token )
            {
              token = ( ascii * ) strtok ( ( char * )NULL, "\r" );
              if ( token )
              {
                memset ( telinfo, 0, 160 );
                strncpy ( ( char * )telinfo, ( const char * )token, 160 );

                if ( modem && modem->sim && modem->sim->retrieve_message_reply )
                {
                  modem->sim->retrieve_message_reply ( NULL, ( const ascii * )&telnumber, ( const ascii * )&telinfo );
                }
                return AT_HANDLER_DONE;
              }  
            }
          } 
        }
      }
      return AT_HANDLER_DONE_ERROR;
    }
  }
  else
  {
    token = MDM_StrStr ( response, MDM_Sym2Str ( symbol_p, MDM_CMS_ERROR ) );
    if ( token )
    {
      /* read sms from simcard error */
      return AT_HANDLER_DONE_ERROR;    
    }

    token = MDM_StrStr ( response, MDM_Sym2Str ( symbol_p, MDM_CMGR ) );
    if ( token )
    {
      return AT_HANDLER_NEED_MORE;
    }
  }

  return AT_HANDLER_DONT_UNDERSTAND;
}








/****************************************************
 *          C M G L toolbox           
 * These commands display a list of all SMs with the status value <stat>, from the message 
 * storage <mem1> (selected using the +CPMS command). The command returns a series of 
 * responses, one per message, each containing the message index, status, and data. If the status of a 
 * message is "RECEIVED UNREAD", execution of the +CMGL command changes the status of 
 * the message to "RECEIVED READ".
 ****************************************************/

/**
 * @brief Creates the at command to List SMS messages from preferred store
 * @param pos index of the message to read
 */
extern void SIM_Command_List ( void* ipc_data, s32 pos )
{
  ascii pos_str[ 24 ] = { 0 }; 
  
  switch ( pos )
  {
    case 0:
      sprintf ( ( char* )pos_str, "%s", "\"REC UNREAD\"" );
      break;

    case 1:
      sprintf ( ( char* )pos_str, "%s", "\"REC READ\"" );
      break;

    case 2:
      sprintf ( ( char* )pos_str, "%s", "\"STO UNSENT\"" );
      break;

    case 3:
      sprintf ( ( char* )pos_str, "%s", "\"STO SENT\"" );
      break;

    case 4:
      sprintf ( ( char* )pos_str, "%s", "\"ALL\"" );
      break;

    default:
      return;
  }

  MDM_PostAtCmdByID ( sim_command_p, 
                      SMS_LIST, 
                      pos_str );
}






/**
 * @brief Handler for List SMS messages from preferred store.
 * @param at at command context
 * @param response modem's response
 * @return handler status
 */
static AtCmdHandler_Status SIM_Handler_List ( MDM_CmdTypedef *at, MDM_RspTypedef *response )
{
  MDM_RSP_ID code = MDM_GetResultCode ( response );
  s16   msgIdx = 0;
  ascii *token = NULL;
  ascii *next = NULL;
  ascii telnbr[ 16 ];

  #define MSG_TMP_MAX_SIZE  140
  ascii telmsg[ MSG_TMP_MAX_SIZE ] = { 0 };
  
  if ( code == MDM_OK )
  {
    token = MDM_StrStr ( response, MDM_Sym2Str ( symbol_p, MDM_CMGL ) );

    while ( token )
    {
      token = ( ascii * ) strchr ( ( const char * )token, ':' );
      if ( token )
      {
        msgIdx = ( s16 )atoi ( ( const char* )( ++token ) );
      }
      else
      {

#if SMS_LIST_DEBUG == 1
        printf ( "\r\nlose :\r\n" );
#endif  /* SMS_LIST_DEBUG */

        /* read sms from simcard error */
        return AT_HANDLER_DONE_ERROR;       
      }

      token = ( ascii * ) strstr ( ( const char * )token, "REC" );
      if ( token )
      {
        token = ( ascii * ) strstr ( ( char * )token, "," );
        if ( *( ++token ) == '"' )
        {
          u8 i = 0;
          u8 length = MSG_TMP_MAX_SIZE;

          ++token;
          for ( i = 0; i < 16; i++ )
          {
            if ( token[ i ] == '"' )
            {
              break;
            }
            telnbr[ i ] = token[ i ];
          }

          token = ( ascii * ) strstr ( ( char * )token, "\r\n" );
          token += 2;
          next  = ( ascii * ) strstr ( ( char * )token, "\r\n" );
          memset ( telmsg, 0, MSG_TMP_MAX_SIZE );
          if ( next )
          {
            length = ( next - token ); 
          }

          if ( length > MSG_TMP_MAX_SIZE )
          {
            length = MSG_TMP_MAX_SIZE;
          }

          strncpy ( ( char * )telmsg, ( const char * )token, length );

#if SMS_LIST_DEBUG == 1
          printf ( "rx %d, nbr %s msg %s", msgIdx, telnbr, telmsg );
#endif  /* SMS_LIST_DEBUG */

          token = ( ascii * ) strstr ( ( const char * )next, ( const char * )MDM_Sym2Str ( symbol_p, MDM_CMGL ) );

          if ( modem && modem->sim && modem->sim->list_preferred_message_reply )
          {
            modem->sim->list_preferred_message_reply ( NULL, msgIdx, ( const ascii * )&telnbr, ( const ascii * )&telmsg );
          }
        }
        else
        {

#if SMS_LIST_DEBUG == 1
          printf ( "\r\nlose ,\r\n" );
#endif  /* SMS_LIST_DEBUG */

          /* read sms from simcard error */
          return AT_HANDLER_DONE_ERROR;       
        }
      }
      else
      {

#if SMS_LIST_DEBUG == 1
        printf ( "\r\nlose REC\r\n" );
#endif  /* SMS_LIST_DEBUG */

        /* read sms from simcard error */
        return AT_HANDLER_DONE_ERROR;       
      }
    }  

    return AT_HANDLER_DONE;
  }
  else
  {
    token = MDM_StrStr ( response, MDM_Sym2Str ( symbol_p, MDM_CMS_ERROR ) );
    if ( token )
    {
      /* read sms from simcard error */
      return AT_HANDLER_DONE_ERROR;    
    }
  }

  return AT_HANDLER_DONT_UNDERSTAND;
}







/* 
 * 功能描述:
 * 引用参数:
 *          
 * 返回值  :
 * 
 */
extern void SIM_Command_StoreMessage ( void* ipc_data, const ascii *message, const ascii *number )
{
  SMS_SendTXT ( ipc_data, message, number, TRUE );
}












/****************************************************
 *          C P B S toolbox           
 * The Set command selects the phone 
 * book memory storage which is to be 
 * used by other phone book commands
 ****************************************************/

/**
 * @brief Creates the at command to read a sms message
 * @param pos index of the message to read
 */
extern void SIM_Command_SelectPhoneBookMemory ( void* ipc_data, const ascii *memory )
{
  if ( memory )
  {
    MDM_PostAtCmdByID ( sim_command_p, 
                        PHONEBOOK_ENTRY_SET, 
                        memory );  
  }
}







/* 
 * 功能描述:
 * 引用参数:
 *          
 * 返回值  :
 * 
 */
static AtCmdHandler_Status SIM_Handler_SelectPhoneBookMemory ( MDM_CmdTypedef *at, MDM_RspTypedef *response )
{
  MDM_RSP_ID code = MDM_GetResultCode ( response );

  if ( code == MDM_OK )
  {
    return AT_HANDLER_DONE;
  }
  else if ( code == MDM_ERROR || code == MDM_CME_ERROR )
  {
  printf("gulin SIM_Handler_SelectPhoneBookMemory %s\r\n",__FILE__);
    SIM_Handler_SetSIMOwnNumber ( at, response );
    return AT_HANDLER_DONE_ERROR;
  }

  return AT_HANDLER_DONT_UNDERSTAND;
}













/****************************************************
 *          C P B W toolbox           
 * This command enables the user to store a new entry in the phone book, or edit/delete an existing 
 * entry from the phone book. A particular entry in the phone book can be stored, or the next 
 * available entry is used. 
 ****************************************************/

/**
 * @brief Creates the at command to read a sms message
 * @param pos index of the message to read
 */
extern void SIM_Command_SetPhoneBookEntry ( void* ipc_data, const ascii *simNumber )
{
  if ( simNumber )
  {
    MDM_PostAtCmdByID ( sim_command_p, 
                        PHONEBOOK_SIM_OWN_NUMBERS_SET, 
                        simNumber );  
  }
}










/* 
 * 功能描述:
 * 引用参数:
 *          
 * 返回值  :
 * 
 */
static AtCmdHandler_Status SIM_Handler_SetPhoneBookEntry ( MDM_CmdTypedef *at, MDM_RspTypedef *response )
{
  MDM_RSP_ID code = MDM_GetResultCode ( response );
   printf("gulin SIM_Handler_SetPhoneBookEntry %s\r\n",__FILE__);

  if ( code == MDM_OK )
  {
    SIM_Handler_SetSIMOwnNumber ( at, response );
    return AT_HANDLER_DONE;
  }
  else if ( code == MDM_ERROR || code == MDM_CME_ERROR )
  {
    SIM_Handler_SetSIMOwnNumber ( at, response );
    return AT_HANDLER_DONE_ERROR;
  }

  return AT_HANDLER_DONT_UNDERSTAND;
}






/* 
 * 功能描述:
 * 引用参数:
 *          
 * 返回值  :
 * 
 */
extern void SIM_Command_SetSIMOwnNumber ( void* ipc_data, const ascii *simNumber )
{
  if ( simNumber )
  {
  printf("111111111\r\n");
    SIM_Command_SelectPhoneBookMemory ( ipc_data, "SM" ); 
    SIM_Command_SetPhoneBookEntry ( ipc_data, simNumber ); 
  }
}

/* 
 * 功能描述:
 * 引用参数:
 *          
 * 返回值  :
 * 
 */
static AtCmdHandler_Status SIM_Handler_SetSIMOwnNumber ( MDM_CmdTypedef *at, MDM_RspTypedef *response )
{
  MDM_RSP_ID code = MDM_GetResultCode ( response );

  if ( code == MDM_OK )
  {
    if ( modem && modem->sim && modem->sim->set_sim_own_number_reply )
    {
      modem->sim->set_sim_own_number_reply ( NULL ); 
    }
    return AT_HANDLER_DONE;
  }
  else if ( code == MDM_ERROR || code == MDM_CME_ERROR )
  {
    if ( modem && modem->sim && modem->sim->set_sim_own_number_error )
    {
	printf("gulin %s\r\n",__FILE__);
      modem->sim->set_sim_own_number_error ( NULL ); 
    }    
    return AT_HANDLER_DONE_ERROR;
  }

  return AT_HANDLER_DONT_UNDERSTAND;
}

