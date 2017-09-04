#include <string.h>
#include <stdlib.h>
#include "modem.h"







/******************************************/
/*              内部函数[声明]            */
/******************************************/

static AtCmdHandler_Status CALL_Handler_Initiate    ( MDM_CmdTypedef *at, MDM_RspTypedef *atrsp );
static AtCmdHandler_Status CALL_Handler_AcceptCall  ( MDM_CmdTypedef *at, MDM_RspTypedef *atrsp );
static AtCmdHandler_Status CALL_Handler_ReleaseCall ( MDM_CmdTypedef *at, MDM_RspTypedef *atrsp );
static AtCmdHandler_Status CALL_Handler_SetCallRing ( MDM_CmdTypedef *at, MDM_RspTypedef *response);







/******************************************/
/*              内部变量[定义]            */
/******************************************/

extern MDM_DevTypedef *modem;

static MDM_CmdTypedef call_commands[] 
= {
  /* activates incoming call */
  { ACCEPT_CALL, "ATA\r\n", 
  	100, 0,
    CALL_Handler_AcceptCall, SIM_READY, FALSE },

  /* Hang up current active call */
  { HANGUP_CURRENT, "ATH\r\n",              
  	1000, 1,
    CALL_Handler_ReleaseCall, SIM_READY, FALSE },

  /* Initiate a call */
  { INITIATE_CALL, "ATD%s;\r\n",               
  	1500, 0,
    CALL_Handler_Initiate, SIM_READY, FALSE },

	{ CALL_RI_SET,"AT+CFGRI=0,1\r\n", 100, 1, 
	CALL_Handler_SetCallRing, SIM_READY, FALSE },
		
		
  { 0, NULL, 0, 0, NULL, SIM_UNKNOWN, FALSE }

}, *call_command_p = call_commands;



static ascii* call_status_names[] = 
{
  "CALL_IDLE",
  "CALL_CALLING",
  "CALL_CONNECTED",
  "CALL_WAITING",
  "CALL_INCOMING",
  "CALL_HOLD"
};



/* 当前呼叫控制 */
static Call cur_call = { CALL_IDLE, NULL, "129", 0 };








/**
* @brief Converts call status to a string and returns it
*
* @param status status to convert
* @return const gchar* containing status' name
*/
static ascii *MDM_Utils_CallStatus2String ( CallStatus status )
{
  return call_status_names[ status ];
}
                                                                  






/**
 * @brief Finds first call with incoming status and sends call status signal
 * Should be called when ring message is received from gsm modem.
 * @param modem modem who sent ring message
 */
extern void MDM_ConfigureCurrentCall ( Call *call )
{
  if ( call ) 
  {
    memcpy ( ( u8* )&cur_call, ( Call * )call, sizeof ( Call ) );  
  } 
}





/****************************************************
 *               A T H toolbox           
 * This command hangs up a call. The G24 terminates the call whether it is a data or voice call, and 
 * whether it is an incoming, originating, waiting, or connected call.
 * A NO CARRIER message is returned to the terminal before the regular OK approval.
 ****************************************************/

/**
 * @brief Function that creates hangup call command.
 * @param ipc_data pointer ipc data
 * @param message TODO what is this anyway?
 * @param id phone call's id
 */
extern void CALL_Command_Release ( void* ipc_data, const s32 id )
{
  if ( cur_call.id == 0 )
  {
    if ( modem && modem->call && modem->call->release_reply )
    {
      /* Call not found, cancelling all initiate call commands */
      modem->call->release_reply ( NULL );        
    } 
    return;
  }

  if ( cur_call.status == CALL_CONNECTED ||
       cur_call.status == CALL_INCOMING ||
       cur_call.status == CALL_CALLING ||
       cur_call.status == CALL_WAITING )
  {
    /* Hangup from connected state */
    MDM_PostAtCmdByID ( call_command_p, 
                        HANGUP_CURRENT, 
                        NULL );
  }
}


extern void Call_handler_release(void)
{
   /* Hangup from connected state */
    MDM_PostAtCmdByID ( call_command_p, 
                        HANGUP_CURRENT, 
                        NULL );

}


/**
 * @brief Handler for ReleaseCall command
 * @param modem modem whose responses to handle
 * @return TRUE if handler understood the modem's reply
 */
static AtCmdHandler_Status CALL_Handler_ReleaseCall ( MDM_CmdTypedef *at, MDM_RspTypedef *response )
{
  MDM_RSP_ID code = MDM_GetResultCode ( response );

	  if ( code == MDM_ERROR || code == MDM_TIME_OUT )
		{
			if ( modem && modem->call && modem->call->release_error )
			{
				modem->call->release_error ( NULL );    
			}    
			return AT_HANDLER_DONE_ERROR;
		}
		else if ( code == MDM_OK )  /* ensures that call exists */
		{
			if ( modem && modem->call && modem->call->release_reply )
			{
				modem->call->release_reply ( NULL );    
			}

			return AT_HANDLER_DONE;
		}

  return AT_HANDLER_DONT_UNDERSTAND;
}

                                                         
/****************************************************
 *               A T D toolbox           
 * This command places a FAX/DATA/VOICE call on the current network.
 * The default call type is a data call (CSD). If the +FCLASS command was used to set the call type 
 * to be FAX, then the outgoing call is a fax call. 
 * There must be an explicit request in order to make a VOICE call. This request bypasses the 
 * +FCLASS setting.
 * If a DATA/FAX call was originated and answered by the remote side, a "OK" notification is sent 
 * to the terminal from the G24, and it moves to the online Data/Fax state (respectively).
 * For more information about call failure, use the AT+CEER command.
 ****************************************************/

/**
 * @brief Initiates a voice call
 * @param ipc_data pointer to ipc data
 * @param number phone number
 * @param type phone number type
 */
extern void CALL_Command_Initiate ( void* ipc_data, const ascii* number, const ascii* type )
{
  if ( number && type )
  {
    cur_call.status = CALL_CALLING;
    MDM_PostAtCmdByID ( call_command_p, 
                        INITIATE_CALL, 
                        ( const ascii* )number ); 
  }
}

/**
 * @brief set call RI pin
 * @param when cal,sms is comming ,ri interrupt from 1 to 0
 * @param 
 * @param goly@simcom6320c
 */
extern void GL868_Clear_RING(void* ipc_data);
static AtCmdHandler_Status CALL_Handler_SetCallRing ( MDM_CmdTypedef *at, MDM_RspTypedef *response )
{
		MDM_RSP_ID code = MDM_GetResultCode ( response );

		if ( code == MDM_ERROR || code == MDM_TIME_OUT )
		{
      //modem->call->set_ri_interrupt(NULL);
			return AT_HANDLER_DONE_ERROR;
		}
		else if ( code == MDM_OK )  /* ensures that call exists */
		{
			GL868_Clear_RING(NULL);
			return AT_HANDLER_DONE;
		}

		return AT_HANDLER_DONT_UNDERSTAND;



}

extern void GSM_Call_handler_SetCallRing(void* ipc_data)
{
   /* SET CALL RI Interrupt */
    MDM_PostAtCmdByID(call_command_p, 
                        CALL_RI_SET, 
                        NULL );

}


/****************************************************
 *               N O T E            
 * VOICE CALL:
 * 1st response - Voice call place begins OK
 * 2nd response - Voice call connected: OK
 * DATA/FAX:
 * 2nd response only - Data/Fax call connected 
 * CONNECT
 * When MO call fails:
 * 1. Connection Failure - NO CARRIER or BUSY or NO ANSWER
 * 2. General Failure - ERROR
 * 3. Security reason (such as SIM not present) - OPERATION NOT ALLOWED
 * 4. Unknown reason - UNKNOWN CALLING ERROR 
 ****************************************************/

/**
 * @brief Handler for dial command
 * @return TRUE if handler understood the modem's reply
 */
static AtCmdHandler_Status CALL_Handler_Initiate ( MDM_CmdTypedef *at, MDM_RspTypedef *response )
{
  AtCmdHandler_Status status = AT_HANDLER_DONT_UNDERSTAND;
  MDM_RSP_ID code = MDM_GetResultCode ( response );

  switch ( code )
  {
    case MDM_OK:
      if ( cur_call.status == CALL_IDLE )
      {
        ++cur_call.id;
        cur_call.status = CALL_CALLING;
      }
      else if ( cur_call.status == CALL_CALLING )
      {
        cur_call.status = CALL_CONNECTED;
      }

      if ( modem && modem->call && modem->call->initiate_reply )
      {
        modem->call->initiate_reply ( NULL, cur_call.id );      
      }
      status = AT_HANDLER_DONE;
      break;
              
    case MDM_NO_CARRIER:
    case MDM_NO_ANSWER:
    case MDM_BUSY:
      if ( modem && modem->call && modem->call->initiate_error )
      {
        modem->call->activate_error ( NULL );
      }
      CALL_Command_Release ( NULL, cur_call.id );
      status = AT_HANDLER_DONE_ERROR;
      break;

    default:
      break;
  }
  
  return status;
}









/****************************************************
 *               A T A toolbox           
 * This command answers an incoming VOICE/DATA/FAX call after a RING/+CRING indication 
 * is sent to the terminal. 
 * If the incoming call is answered (connected), the G24 sends a CONNECT notification to the terminal.
 * If the MT call fails, the possible notifications are:
 * - NO CARRIER - Connection Failure
 * - ERROR - General Failure 
 ****************************************************/


/**
 * @brief Function that creates activate call command.
 * @param modem modem to send the command to
 * @param ipc_data pointer to ipc data
 */
extern void CALL_Command_Activate ( void* ipc_data, s32 id )
{
  if ( !cur_call.id )
  {
    if ( modem && modem->call && modem->call->activate_error )
    {
      modem->call->activate_error ( NULL );
    } 
    return;
  }

  /* Make sure the call we are trying to activate isn't already activated */
  if ( cur_call.status == CALL_CONNECTED )
  {
    if ( modem && modem->call && modem->call->activate_reply )
    {
      /* Trying to activate a call that is already active. */
      modem->call->activate_reply ( MDM_Utils_CallStatus2String ( CALL_CONNECTED ) );        
    }
  }
  else
  {
    /* If we have no current calls, send ATA */
    MDM_PostAtCmdByID ( call_command_p, 
                        ACCEPT_CALL, 
                        NULL );  
  }
}




/**
 * @brief Handler for accept command
 * @param modem modem whose responses to handle
 * @return TRUE if handler understood the modem's reply
 */
static AtCmdHandler_Status CALL_Handler_AcceptCall (  MDM_CmdTypedef *at, MDM_RspTypedef *response )
{
  AtCmdHandler_Status status = AT_HANDLER_DONT_UNDERSTAND;
  MDM_RSP_ID code = MDM_GetResultCode ( response );
    
  /* ensures that call exists */
  if ( !cur_call.id )
  {
    if ( modem && modem->call && modem->call->activate_error )
    {
      modem->call->activate_error ( NULL );
    }
    return AT_HANDLER_ERROR;
  }
  
  if ( code == MDM_OK )
  {
    /* Finally set our current call's status to connected */
    cur_call.status = CALL_CONNECTED;
    if ( modem && modem->call && modem->call->activate_reply )
    {
      modem->call->activate_reply ( MDM_Utils_CallStatus2String ( CALL_CONNECTED ) );
    }   
    status = AT_HANDLER_DONE;
  }
  else if ( code == MDM_NO_CARRIER )
  {
    /* remove call */
    CALL_Command_Release ( NULL, cur_call.id );
    status = AT_HANDLER_DONE;
  }
  else if ( code == MDM_ERROR || code == MDM_TIME_OUT )
  {
    /* remove call */
    CALL_Command_Release ( NULL, cur_call.id );
    status = AT_HANDLER_DONE_ERROR;
  }
  
  return status;
}



