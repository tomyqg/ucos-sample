





#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "modem.h"






/******************************************/
/*              内部函数[声明]            */
/******************************************/

static AtCmdHandler_Status NETWORK_Handler_QueryNetworkRegistration         ( MDM_CmdTypedef *at, MDM_RspTypedef *atrsp );
static AtCmdHandler_Status NETWORK_Handler_QueryGprsNetworkRegistration     ( MDM_CmdTypedef *at, MDM_RspTypedef *atrsp );
static AtCmdHandler_Status NETWORK_Handler_QuerySignalStrength              ( MDM_CmdTypedef *at, MDM_RspTypedef *atrsp );
static AtCmdHandler_Status NETWORK_Handler_QueryCurrentOperator             ( MDM_CmdTypedef *at, MDM_RspTypedef *atrsp );
static AtCmdHandler_Status NETWORK_Handler_QueryAvailableOperators          ( MDM_CmdTypedef *at, MDM_RspTypedef *atrsp );
static AtCmdHandler_Status NETWORK_Handler_SetOperator                      ( MDM_CmdTypedef *at, MDM_RspTypedef *atrsp );
static AtCmdHandler_Status NETWORK_Handler_QuerySignalStrength              ( MDM_CmdTypedef *at, MDM_RspTypedef *atrsp );
static AtCmdHandler_Status NETWORK_Handler_NetworkRegister                  ( MDM_CmdTypedef *at, MDM_RspTypedef *atrsp );






/******************************************/
/*              内部变量[定义]            */
/******************************************/



extern MDM_DevTypedef *modem;

static MDM_CmdTypedef network_commands[] 
= {	
  /* Network registration */
  { NETWORK_REG_QUERY, "AT+CREG?\r\n", 
  	500, 3,
    NETWORK_Handler_QueryNetworkRegistration, SIM_UNKNOWN, FALSE },

  /* Network registration */
  { GPRS_NETWORK_REG_QUERY, "AT+CGREG?\r\n", 
  	500, 3,
    NETWORK_Handler_QueryGprsNetworkRegistration, SIM_READY, FALSE },

  /* Network registration */
  { NETWORK_REGISTER, "AT+CREG=1\r\n", 
  	1500, 1,
    NETWORK_Handler_NetworkRegister, SIM_UNKNOWN, FALSE },

  /* Set operator */
  { SET_OPERATOR, "AT+COPS=0,0\r\n", 
  	1800, 0,
    NETWORK_Handler_SetOperator, SIM_READY, FALSE },

  /* Set operator locked */
  { SET_OPERATOR_LOCKED, "AT+COPS=1,0,%s\r\n", 
  	1800, 0,
    NETWORK_Handler_SetOperator, SIM_READY, FALSE },

  /* Query current operator */
  { CUR_OP_QUERY, "AT+COPS?\r\n", 
  	1800, 0,
    NETWORK_Handler_QueryCurrentOperator, SIM_READY, FALSE },

  /* Query available operators */
  { OPS_QUERY, "AT+COPS=?\r\n", 
  	1800, 0,
    NETWORK_Handler_QueryAvailableOperators, SIM_READY, FALSE },

  /* Query signal strength */
  { SIGNAL_QUERY, "AT+CSQ\r\n", 
  	100, 0,
    NETWORK_Handler_QuerySignalStrength, SIM_READY, FALSE },


  { 0, NULL, 0, 0, NULL, SIM_UNKNOWN, FALSE }
}, *network_command_p = network_commands;






/****************************************************
 *          C O P S toolbox           
 * This command enables accessories to access the network registration information, 
 * and the selection and registration of the GSM network operator. 
 * The G24 is registered in the Home network. 
 ****************************************************/

/**
 * @brief AtCommand to set operator
 * @param modem modem whose response to handle
 * @param operator operator identifier
 * @param locked should operator be selected automatically (FALSE) or manually (TRUE)
 */
extern void NETWORK_Command_SetOperator ( void* ipc_data, const ascii* operator, bool locked )
{
  if ( locked ) 
  {
    MDM_PostAtCmdByID ( network_command_p, 
                        SET_OPERATOR_LOCKED, 
                        operator );
  } 
  else 
  {
  	printf("start network query !!!");
    MDM_PostAtCmdByID ( network_command_p, 
                        SET_OPERATOR, 
                        NULL );
  }
}





/**
 * @brief Handler to modems response from SetOperator command
 * @param modem modem whose response to handle
 * @return true if responses were recognized
 */
static AtCmdHandler_Status NETWORK_Handler_SetOperator ( MDM_CmdTypedef *at, MDM_RspTypedef *response )
{
  MDM_RSP_ID code = MDM_GetResultCode ( response );

  /* OK or ERROR, if OK it will following with +CREG: status */
  if ( code == MDM_OK ) 
  {
    if ( modem && modem->network && modem->network->register_network_reply )
    {
      modem->network->register_network_reply ( NULL );
    }
    return AT_HANDLER_DONE;
  }
  else if ( ( code == MDM_TIME_OUT )||( code == MDM_ERROR ) )
  {
    if ( modem && modem->network && modem->network->register_network_reply )
    {
      modem->network->register_network_reply ( NULL );
    }
    return AT_HANDLER_DONE_ERROR;
  }

  return AT_HANDLER_DONT_UNDERSTAND;
}






/**
 * @brief AtCommand to query available operators
 * @param modem modem whose response to handle
 */
extern void NETWORK_Command_QueryAvailableOperators ( void* ipc_data )
{
  MDM_PostAtCmdByID ( network_command_p, 
                      OPS_QUERY, 
                      NULL );
}



/**
 * @brief Handler to modems response list_providers command
 * @param modem modem whose response to handle
 * @return true if responses were recognized
 */
static AtCmdHandler_Status NETWORK_Handler_QueryAvailableOperators ( MDM_CmdTypedef *at, MDM_RspTypedef *response)
{
  AtCmdHandler_Status status = AT_HANDLER_DONE;
  return status;
}






/**
 * @brief AtCommand to query current operator
 * @param modem modem whose response to handle
 */
extern void NETWORK_Command_QueryCurrentOperator ( void* ipc_data )
{  
  MDM_PostAtCmdByID ( network_command_p, 
                      CUR_OP_QUERY, 
                      NULL );
}




/**
 * @brief Handler to modems response from queryCurrentOperator command
 * @param modem modem whose response to handle
 * @return true if responses were recognized
 */
static AtCmdHandler_Status NETWORK_Handler_QueryCurrentOperator ( MDM_CmdTypedef *at, MDM_RspTypedef *response )
{
  MDM_RSP_ID code = MDM_GetResultCode ( response );

  if ( code == MDM_ERROR ) 
  {
    /* "UNABLE TO QUERY CURRENT OPERATOR" */
    return AT_HANDLER_DONE_ERROR;
  }
  else if ( code == MDM_OK ) 
  {
    ascii *token = MDM_StrStr ( response, MDM_Sym2Str ( symbol_p, MDM_COPS ) );

    if ( token ) 
    {
      token = ( ascii* ) strstr ( ( const char* )token, "," );
      if ( token ) 
      {
        /* get the operator name here; */
       //  modem->network->provider_name;
        
        /* We now got operator name, query for
         * signal strength */
        MDM_PostAtCmdByID ( network_command_p, 
                            SIGNAL_QUERY, 
                            NULL );

        /* this command ends with OK */
      }     
      return AT_HANDLER_DONE;
    }
  }

  return AT_HANDLER_DONT_UNDERSTAND;
}






/****************************************************
 *          C S Q toolbox           
 * This command displays the received signal strength indication <rssi> 
 * and channel bit error rate <ber>.
 ****************************************************/

/**
 * @brief AtCommand to query signal strength
 * @param modem modem whose response to handle
 */
extern void NETWORK_Command_QuerySignalStrength ( void* ipc_data )
{
  MDM_PostAtCmdByID ( network_command_p, 
                      SIGNAL_QUERY, 
                      NULL );
}




/**
 * @brief Handler to modems response from querySignalStrength command
 * @param modem modem whose response to handle
 * @return true if responses were recognized
 */
static AtCmdHandler_Status NETWORK_Handler_QuerySignalStrength ( MDM_CmdTypedef *at, MDM_RspTypedef *response )
{
  MDM_RSP_ID code = MDM_GetResultCode ( response );

  if ( code == MDM_ERROR ) 
  {
    if ( modem && modem->network && modem->network->get_status_error )
    {
      modem->network->get_status_error ( NULL ); 
    }
    return AT_HANDLER_DONE_ERROR;
  }
  else if ( code == MDM_OK ) 
  {
    ascii *token = MDM_StrStr ( response, MDM_Sym2Str ( symbol_p, MDM_CSQ ) );

    if ( token ) 
    {
      token = ( ascii * )strstr ( ( char * )token, ": " );
      token += 2;
      if ( token )
      {
        /* Network status should always exist */
        if ( modem && modem->network && modem->network && modem->network->query_signal_strength_reply ) 
        {
          modem->network->query_signal_strength_reply ( NULL, ( u8 )atoi ( ( const char* )token ) );
        }        
      }
      return AT_HANDLER_DONE; 
    }
  }

  return AT_HANDLER_DONT_UNDERSTAND;
}






/****************************************************
 *         C R E G toolbox           
 * The Read command returns the status of the result code presentation and 
 * shows whether the network has currently indicated the registration of 
 * the G24. Location information elements <lac> and <ci> are returned 
 * only when <n>=2 and the G24 is registered in the network..
 ****************************************************/


/**
 * @brief AtCommand to query network registration
 * @param modem modem whose response to handle
 */
extern void NETWORK_Command_QueryNetworkRegistration ( void* ipc_data )
{
  MDM_PostAtCmdByID ( network_command_p, 
                      NETWORK_REG_QUERY, 
                      NULL );
}






/**
 * @brief Handler to modems response from queryNetworkRegistration command
 * @param modem modem whose response to handle
 * @return true if responses were recognized
 */
static AtCmdHandler_Status NETWORK_Handler_QueryNetworkRegistration ( MDM_CmdTypedef *at, MDM_RspTypedef *response )
{
  MDM_RSP_ID code = MDM_GetResultCode ( response );

  /* for queryNetworkRegistration */
  if ( code == MDM_ERROR ) 
  {
    if ( modem && modem->network && modem->network->get_status_error )
    {
      modem->network->get_status_error ( NULL ); 
    }
    return AT_HANDLER_DONE_ERROR;
  }
  else if ( code == MDM_OK ) 
  {
    ascii *token = MDM_StrStr ( response, MDM_Sym2Str ( symbol_p, MDM_CREG ) );

    if ( token ) 
    {
      token = ( ascii * )strstr ( ( const char * )token, "," );
      if ( token ) 
      {
        u32 code = atoi ( ( const char* )++token );
        
        //printf ( "\r\nCREG code %u", code );
               
        //if ( modem && modem->network && modem->network->network_status )
        {
          modem->network->network_status = ( NetworkStatus )code;
          MDM_ChangeNetworkStatus ( modem->network->network_status );
        }
      }
      return AT_HANDLER_DONE;
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
extern void NETWORK_Command_GetStatus ( void* ipc_data )
{
  if ( modem && modem->network )
  {
    switch ( modem->network->network_status ) 
    {
      /* We aren't registered, then we have no provider or signal
       * TODO are these assumption correct? */
      case NETWORK_UNREGISTERED:
      case NETWORK_BUSY:
      case NETWORK_DENIED:     
        /* Not registered to network, sending status signal */
        if ( modem && modem->network && modem->network->status )
        {
          modem->network->status ( GSM_NETWORK_TYPE, modem->network->network_status );
        }
      break;
      
      case NETWORK_REGISTERED:
      case NETWORK_ROAMING:
        /* if we have current operator name, 
         * then just query signal strength */
        if ( modem && modem->network && modem->network->provider_name ) 
        {
          /* We have provider name, query signal */
          MDM_PostAtCmdByID ( network_command_p, 
                              SIGNAL_QUERY, 
                              NULL );
        } 
        else 
        {
          /* If we don't have current operator name then we'll query it
           * and that handler will also query for the signal */
          MDM_PostAtCmdByID ( network_command_p, 
                              CUR_OP_QUERY, 
                              NULL );
        }
        break;
    
      case NETWORK_UNKNOWN:
        /* Unknown status, query status */
        MDM_PostAtCmdByID ( network_command_p, 
                            NETWORK_REG_QUERY, 
                            NULL );
        break;
    }  
  }
}






/* 
 * 功能描述:
 * 引用参数:
 *          
 * 返回值  :
 * 
 */
extern void NETWORK_Command_RegisterWithProvider ( void* ipc_data, int index )
{
  if ( modem && modem->network && modem->network->register_with_provider_reply )
  {
    modem->network->register_with_provider_reply ( ipc_data );
  }
}













/* 
 * 功能描述:
 * 引用参数:
 *          
 * 返回值  :
 * 
 */
extern void NETWORK_Command_NetworkRegister ( void* ipc_data )
{
  MDM_PostAtCmdByID ( network_command_p, 
                      NETWORK_REGISTER, 
                      NULL );
}







/* 
 * 功能描述:
 * 引用参数:
 *          
 * 返回值  :
 * 
 */
static AtCmdHandler_Status NETWORK_Handler_NetworkRegister ( MDM_CmdTypedef *at, MDM_RspTypedef *response )
{
  MDM_RSP_ID code = MDM_GetResultCode ( response );

  /* for queryNetworkRegistration */
  if ( code == MDM_ERROR ) 
  {
    if ( modem && modem->network && modem->network->get_status_error )
    {
      modem->network->get_status_error ( NULL ); 
    }
    return AT_HANDLER_DONE_ERROR;
  }
  else if ( code == MDM_OK ) 
  {
    ascii *token = MDM_StrStr ( response, MDM_Sym2Str ( symbol_p, MDM_CREG ) );

    if ( token ) 
    {      
      return AT_HANDLER_DONE;
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
extern void NETWORK_Command_QueryGprsNetworkRegistration ( void* ipc_data )
{
  MDM_PostAtCmdByID ( network_command_p, 
                      GPRS_NETWORK_REG_QUERY, 
                      NULL );
}






/* 
 * 功能描述:
 * 引用参数:
 *          
 * 返回值  :
 * 
 */
static u8 cgreg = 0;
static AtCmdHandler_Status NETWORK_Handler_QueryGprsNetworkRegistration ( MDM_CmdTypedef *at, MDM_RspTypedef *response )
{
  MDM_RSP_ID code = MDM_GetResultCode ( response );

  /* for queryNetworkRegistration */
  if ( code == MDM_ERROR ) 
  {
    if ( modem && modem->network && modem->network->get_status_error )
    {
      modem->network->get_status_error ( NULL ); 
    }
    return AT_HANDLER_DONE_ERROR;
  }
  else if ( code == MDM_OK ) 
  {
    ascii *token = MDM_StrStr ( response, MDM_Sym2Str ( symbol_p, MDM_CGREG ) );

    if ( token ) 
    {
      token = ( ascii * )strstr ( ( const char * )token, "," );
      if ( token ) 
      {
        u32 code = 0;
        
        ++token;
        code = atoi ( ( const char* )token );
        cgreg = code;
        if ( modem && modem->network && modem->network->status )
        {
          modem->network->status ( GPRS_NETWORK_TYPE, ( NetworkStatus )code );
        }
      }
      return AT_HANDLER_DONE;
    }
  }

  return AT_HANDLER_DONT_UNDERSTAND;
}

extern u8 getCgreg ( void )
{
	return cgreg;
}





