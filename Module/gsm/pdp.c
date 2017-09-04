











#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "modem.h"
//#include "tz_gpio.h"
//#include "timer.h"



/******************************************/
/*              内部函数[声明]            */
/******************************************/

static AtCmdHandler_Status PDP_Handler_GprsAttach               ( MDM_CmdTypedef *at, MDM_RspTypedef *atrsp );
static AtCmdHandler_Status PDP_Handler_ListGprsClasses          ( MDM_CmdTypedef *at, MDM_RspTypedef *atrsp );
static AtCmdHandler_Status PDP_Handler_SelectGprsClass          ( MDM_CmdTypedef *at, MDM_RspTypedef *atrsp );
static AtCmdHandler_Status PDP_Handler_Activate                 ( MDM_CmdTypedef *at, MDM_RspTypedef *atrsp );
static AtCmdHandler_Status PDP_Handler_Deactivate               ( MDM_CmdTypedef *at, MDM_RspTypedef *atrsp );
static AtCmdHandler_Status PDP_Handler_SelectContext            ( MDM_CmdTypedef *at, MDM_RspTypedef *atrsp );
static AtCmdHandler_Status PDP_Handler_AddContext               ( MDM_CmdTypedef *at, MDM_RspTypedef *atrsp );
static AtCmdHandler_Status PDP_Handler_DeleteContext            ( MDM_CmdTypedef *at, MDM_RspTypedef *atrsp );
static AtCmdHandler_Status PDP_Handler_listContext              ( MDM_CmdTypedef *at, MDM_RspTypedef *atrsp );
static AtCmdHandler_Status PDP_Handler_RequestGPRSservice       ( MDM_CmdTypedef *at, MDM_RspTypedef *atrsp );
static AtCmdHandler_Status GL868_Handler_GprsContextAct      ( MDM_CmdTypedef *at, MDM_RspTypedef *response );
static AtCmdHandler_Status GL868_Socket_ConfigurationExtend  ( MDM_CmdTypedef *at, MDM_RspTypedef *response );
static AtCmdHandler_Status GL868_Socket_Dial                       ( MDM_CmdTypedef *at, MDM_RspTypedef *response );
static AtCmdHandler_Status Gl868_Handler_SendData               ( MDM_CmdTypedef *at, MDM_RspTypedef *response );
static AtCmdHandler_Status PDP_Handler_GprsDetach              ( MDM_CmdTypedef *at, MDM_RspTypedef *response );
static AtCmdHandler_Status GL868_Handler_GprsContextNoAct ( MDM_CmdTypedef *at, MDM_RspTypedef *response );
static AtCmdHandler_Status PDP_Handler_SocketShutDown ( MDM_CmdTypedef *at, MDM_RspTypedef *response );
static AtCmdHandler_Status Gl868_Handler_Ftp_Time_Out( MDM_CmdTypedef *at, MDM_RspTypedef *response );
static AtCmdHandler_Status Gl868_Handler_Ftp_Close ( MDM_CmdTypedef *at, MDM_RspTypedef *response );
static AtCmdHandler_Status PDP_Handler_SocketInactiveTimeOut ( MDM_CmdTypedef *at, MDM_RspTypedef *response );
static AtCmdHandler_Status PDP_Handler_SocketSet( MDM_CmdTypedef *at, MDM_RspTypedef *response );
static AtCmdHandler_Status PDP_Handler_SocketOpen( MDM_CmdTypedef *at, MDM_RspTypedef *response );
static AtCmdHandler_Status PDP_Handler_SocketRestore ( MDM_CmdTypedef *at, MDM_RspTypedef *response );
static AtCmdHandler_Status GL868_Socket_Configuration( MDM_CmdTypedef *at, MDM_RspTypedef *response );
static AtCmdHandler_Status PDP_Select_flow_control( MDM_CmdTypedef *at, MDM_RspTypedef *response );
static AtCmdHandler_Status GL868_Ppp_ParamConfig( MDM_CmdTypedef *at, MDM_RspTypedef *response );
static AtCmdHandler_Status Cfg_Http_Params ( MDM_CmdTypedef *at, MDM_RspTypedef *response );
static AtCmdHandler_Status PDP_Context_Act_Config( MDM_CmdTypedef *at, MDM_RspTypedef *response );
static AtCmdHandler_Status PDP_Context_Activation( MDM_CmdTypedef *at, MDM_RspTypedef *response );
static AtCmdHandler_Status Dns_Response_Cach( MDM_CmdTypedef *at, MDM_RspTypedef *response );
static AtCmdHandler_Status Gl868_Request_Reference_location( MDM_CmdTypedef *at, MDM_RspTypedef *response );

extern bool ftp_recvDataHdlr ( u16 DataSize, u8 *Data );
extern void PowerOff_device(void);
extern u32 get_file_size(void);
extern bool CheckAppStackOverFlow(void);
extern bool CheckUsartStackOverFlow(void);
extern bool CheckTMRFastStackOverFlow(void);
extern bool CheckTMRSlowStackOverFlow(void);
extern bool CheckWorkStackOverFlow(void);
extern bool CheckRtcStackOverFlow(void);







/******************************************/
/*              内部变量[定义]            */
/******************************************/

extern MDM_DevTypedef *modem;

static MDM_CmdTypedef pdp_commands[] 
= {

  /* GPRS attach or detach */
  { ATTACH_GPRS_NETWORK, "AT+CGATT=1\r\n", 
  	3000, 2,
    PDP_Handler_GprsAttach, SIM_READY, FALSE },

  { DETACH_GPRS_NETWORK, "AT+CGATT=0\r\n", 
  	3000, 2,
    PDP_Handler_GprsDetach, SIM_READY, FALSE },

  /*GPRS Context Activation*/
  { GPRS_CONTEXT_ACT, "AT#GPRS=1\r\n", 
  	3000, 3,
    GL868_Handler_GprsContextAct, SIM_READY, FALSE },

   /*GPRS Context NO Activation*/
  { GPRS_CONTEXT_NO_ACT, "AT#GPRS=0\r\n", 
  	3000, 2,
    GL868_Handler_GprsContextNoAct, SIM_READY, FALSE },

   /*Socket Configuration Extended*/
  { SOCKET_CONFIGURATION, " AT#SCFG=%s\r\n", 
  	2000, 1,
    GL868_Socket_Configuration, SIM_READY, FALSE },

   /*Socket Configuration Extended*/
  { SOCKET_CONFIGURATION_EXTERND, "AT#SCFGEXT=%s\r\n", 
  	2000, 1,
    GL868_Socket_ConfigurationExtend, SIM_READY, FALSE },

  /*PPP-GPRS Parameters Configuration*/
 { PPP_PARAM_CONFIG, " AT#GPPPCFG=%s\r\n", 
  	1000, 1,
    GL868_Ppp_ParamConfig, SIM_READY, FALSE },
   
   /*Socket Dial*/
  { SOCKET_DIAL, "AT#SD=%s\r\n", 
  	2000, 1,
    GL868_Socket_Dial, SIM_READY, FALSE },

    /* Send Data In Command Mode */
  { GPRS_SSEND, "AT#SSEND=%s\r\n", 
  	1500, 2,
    Gl868_Handler_SendData, SIM_READY, FALSE },
    
   /*configure HTTP parameters*/
  { CFG_HTTP_PARAMS, "AT#HTTPCFG?\r\n", 
  	1500, 2,
    Cfg_Http_Params, SIM_READY, FALSE },
    
  /*Context activation and configuration*/
  { CONTEXT_ACT_CONFIG, "AT#SGACTCFG?\r\n", 
  	1500, 2,
    PDP_Context_Act_Config, SIM_READY, FALSE },
 
    
  /*Context Activation*/
  { CONTEXT_ACTIVATION, "AT#SGACT=%s\r\n", 
  	2000, 3,
    PDP_Context_Activation, SIM_READY, FALSE },

  /*DNS Response Caching*/
  { DNS_CACH, "AT#CACHEDNS=1\r\n", 
  	1000, 2,
    Dns_Response_Cach, SIM_READY, FALSE },

  /*send HTTP request to AGPS server*/
  { REQUEST_LOCATION, "AT#AGPSSND\r\n", 
  	9000, 0,
    Gl868_Request_Reference_location, SIM_READY, FALSE },
  
     /* ftp close*/
  { FTP_CLOSE, "AT#FTPCLOSE\r\n", 
  	900, 0,
    Gl868_Handler_Ftp_Close, SIM_READY, FALSE },

    /* FTP Time-Out*/
  { FTP_TIME_OUT, "AT#FTPTO=%s\r\n", 
  	900, 0,
    Gl868_Handler_Ftp_Time_Out, SIM_READY, FALSE },
    
   /* Socket Shutdown */
  { SOCKET_SHUTDOWN, "AT#SH=%s\r\n", 
  	3000, 2,
    PDP_Handler_SocketShutDown, SIM_READY, FALSE },
  
  /* Socket Restore */
   { SOCKET_RESTORE, "AT#SO=%s\r\n", 
	 3000, 2,
	 PDP_Handler_SocketRestore, SIM_READY, FALSE },

  /*Socket Inactivity Time-Out*/
  { SOCKET_INACTIVE_TIMEOUT, "AT#SKTTO=%s\r\n", 
  	3000, 2,
    PDP_Handler_SocketInactiveTimeOut, SIM_READY, FALSE },

  /*Socket Definition*/
   { SOCKET_SET, "AT#SKTSET=%s\r\n", 
	 3000, 2,
	 PDP_Handler_SocketSet, SIM_READY, FALSE },

  /*Socket Open*/
   { SOCKET_OPEN, "AT#SKTOP\r\n", 
	 3000, 2,
	 PDP_Handler_SocketOpen, SIM_READY, FALSE },
	 
  /*Select Flow Control*/
   { SELECT_FLOW_CONTROL, "AT+FLO=%s\r\n", 
	 3000, 2,
	 PDP_Select_flow_control, SIM_READY, FALSE },
	 
  /* activates incoming call */
  { LIST_GPRS_CLASSES, "AT+CGCLASS=?\r\n", 
  	100, 0,
    PDP_Handler_ListGprsClasses, SIM_READY, FALSE },

  /* activates incoming call */
  { SELECT_GPRS_CLASS, "AT+CGCLASS=%s\r\n", 
  	100, 0,
    PDP_Handler_SelectGprsClass, SIM_READY, FALSE },

  /* activates incoming call */
  { ACTIVE_PDP_CONTEXT, "AT+CGACT=1,%s\r\n", 
  	5500, 3,
    PDP_Handler_Activate, SIM_READY, FALSE },

  /* activates incoming call */
  { DEACTIVE_PDP_CONTEXT, "AT+CGACT=0,%s\r\n", 
  	2000, 1,
    PDP_Handler_Deactivate, SIM_READY, FALSE },

  /* Hang up current active call */
  { SELECT_PDP_CONTEXT, "AT+CGDCONT=%s\r\n",              
  	2000, 1,
    PDP_Handler_SelectContext, SIM_READY, FALSE },

  /* Initiate a call */
  { ADD_PDP_CONTEXT, "AT+CGDCONT=%s\r\n",               
  	1500, 3,
    PDP_Handler_AddContext, SIM_READY, FALSE },
	
	{ CTCC_ADD_PDP_CONTEXT, "AT+CSOCKAUTH=%s\r\n",               
  	1500, 3,
    PDP_Handler_AddContext, SIM_READY, FALSE },

  /* Hang up current active call */
  { DELETE_PDP_CONTEXT, "AT+CGDCONT=%s,\"IP\","",\"0.0.0.0\",0,0\r\n",              
  	1000, 1,
    PDP_Handler_DeleteContext, SIM_READY, FALSE },

  /* Initiate a call */
  { LIST_PDP_CONTEXT, "AT+CGDCONT?\r\n",               
  	1500, 0,
    PDP_Handler_listContext, SIM_READY, FALSE },

  /* Initiate a call */
//  { REQUEST_GPRS_SERVICE, "ATD*99***%s#\r\n",               
//  	2000, 1,
//    PDP_Handler_RequestGPRSservice, SIM_READY, FALSE },

   
  { REQUEST_GPRS_SERVICE, "ATD*99#\r\n",               
  	2000, 1,
    PDP_Handler_RequestGPRSservice, SIM_READY, FALSE },
//	{ REQUEST_GPRS_SERVICE, "ATD#777\r\n",               
//  	2000, 1,
//    PDP_Handler_RequestGPRSservice, SIM_READY, FALSE },

  { 0, NULL, 0, 0, NULL, SIM_UNKNOWN, FALSE }

}, *pdp_command_p = pdp_commands;





#define MAX_MSG_LEN  320
ascii gprs_str_msg[ MAX_MSG_LEN ];
s32   gprs_str_len = 0;




/**
* @brief Converts call status to a string and returns it
*
* @param status status to convert
* @return const gchar* containing status' name
*/
extern void PDP_Command_GprsAttach ( void* ipc_data )
{
  MDM_PostAtCmdByID ( pdp_command_p, 
                      ATTACH_GPRS_NETWORK,
                      NULL ); 
}

/**
* @brief Converts call status to a string and returns it
*
* @param status status to convert
* @return const gchar* containing status' name
*/
extern void PDP_Command_GprsDetach ( void* ipc_data )
{
  MDM_PostAtCmdByID ( pdp_command_p, 
                      DETACH_GPRS_NETWORK,
                      NULL ); 
}


/**
* @brief Converts call status to a string and returns it
*
* @param status status to convert
* @return const gchar* containing status' name
*/
extern void Gl868_Command_GprsContextActivation ( void* ipc_data )
{
  MDM_PostAtCmdByID ( pdp_command_p, 
                      GPRS_CONTEXT_ACT,
                      NULL ); 
}

/**
* @brief Converts call status to a string and returns it
*
* @param status status to convert
* @return const gchar* containing status' name
*/
extern void Gl868_Command_GprsContextNoActivation ( void* ipc_data )
{
  MDM_PostAtCmdByID ( pdp_command_p, 
                      GPRS_CONTEXT_NO_ACT,
                      NULL ); 
}



/**
* @brief Converts call status to a string and returns it
*
* @param status status to convert
* @return const gchar* containing status' name
*/
extern void Gl868_Command_SocketConfigurationExtend ( void* ipc_data,const char connId, const char srMode,
                             const char recvDataMode, const char keepalive,const char ListenAutoRsp,const char endDataMode  )
{
  ascii params[ 20 ];
  sprintf ( ( char * )params, "%d,%d,%d,%d,%d,%d", connId, srMode,recvDataMode,keepalive,ListenAutoRsp,endDataMode );
  MDM_PostAtCmdByID ( pdp_command_p, 
                      SOCKET_CONFIGURATION_EXTERND,
                      params ); 
}


extern void Gl868_Command_PppParamConfig ( void* ipc_data,const char *IP, const short LcpTimeOut,
                             const char PppMode)
{
  ascii params[ 20 ];
  sprintf ( ( char * )params, "\"%s\",%d,%d", IP, LcpTimeOut,PppMode );
  MDM_PostAtCmdByID ( pdp_command_p, 
                      PPP_PARAM_CONFIG,
                      params ); 
}

extern void Gl868_Command_SocketConfiguration ( void* ipc_data,const char connId, const char cid,
							 const short pktSz, const short maxTo,const short connTo,const char txTo )
{
  ascii params[ 20 ];
  sprintf ( ( char * )params, "%d,%d,%d,%d,%d,%d", connId, cid, pktSz, maxTo, connTo, txTo );
  MDM_PostAtCmdByID ( pdp_command_p, 
					  SOCKET_CONFIGURATION,
					  params ); 
}


/**
* @brief Converts call status to a string and returns it
*
* @param status status to convert
* @return const gchar* containing status' name
*/
extern void Gl868_Command_SocketDial ( void* ipc_data,const char connId, const char txProt,
                             const u16 rPort, const char *IP,const char closureType,const u16 lPort, const u8 connMode  )
{
  ascii params[ 78 ];
  sprintf ( ( char * )params, "%d,%d,%d,\"%s\",%d,%d,%d", connId, txProt,rPort,IP,closureType,lPort,connMode );
  MDM_PostAtCmdByID ( pdp_command_p, 
                      SOCKET_DIAL,
                      params ); 
}

/**
* @brief Converts call status to a string and returns it
*
* @param status status to convert
* @return const gchar* containing status' name
*/
extern void Gl868_Command_data_SocketDial ( void* ipc_data,const char connId, const char txProt,
                             const u16 rPort, const char *IP )
{
  ascii params[ 78 ];
  sprintf ( ( char * )params, "%d,%d,%d,\"%s\"", connId, txProt,rPort,IP );
  MDM_PostAtCmdByID ( pdp_command_p, 
                      SOCKET_DIAL,
                      params ); 
}


/**
* @brief Converts call status to a string and returns it
*
* @param status status to convert
* @return const gchar* containing status' name
*/
extern void Gl868_Command_Socket_Shutdown( void* ipc_data,const char connId )
{
  ascii params[ 5 ];
  sprintf ( ( char * )params, "%d", connId );
  MDM_PostAtCmdByID ( pdp_command_p, 
                      SOCKET_SHUTDOWN,
                      params ); 
}


extern void Gl868_Command_Socket_Restore( void* ipc_data,const char connId )
{
  ascii params[ 5 ];
  sprintf ( ( char * )params, "%d", connId );
  MDM_PostAtCmdByID ( pdp_command_p, 
					  SOCKET_RESTORE,
					  params ); 
}



extern void Gl868_Command_Socket_Inactive_Timeout( void* ipc_data,u16 tout )
{
  ascii params[ 5 ];
  sprintf ( ( char * )params, "%d", tout );
  MDM_PostAtCmdByID ( pdp_command_p, 
					  SOCKET_INACTIVE_TIMEOUT,
					  params ); 
}

extern void Gl868_Command_Socket_Set( void* ipc_data, u8 type, u16 port, const char *IP)
{
  ascii params[ 50 ];
  sprintf ( ( char * )params, "%d,%d,\"%s\"", type,port,IP );
  MDM_PostAtCmdByID ( pdp_command_p, 
					  SOCKET_SET,
					  params ); 
}

extern void Gl868_Command_Socket_Open( void* ipc_data)
{
  MDM_PostAtCmdByID ( pdp_command_p, 
					  SOCKET_OPEN,
					  NULL ); 
}


extern void Gl868_Select_Flow_control( void* ipc_data, u8 type)
{
  ascii params[ 5 ];
  sprintf ( ( char * )params, "%d", type );
  MDM_PostAtCmdByID ( pdp_command_p, 
					  SELECT_FLOW_CONTROL,
					  params ); 
}

extern void Gl868_Cfg_Http_Params( void* ipc_data )
{
  MDM_PostAtCmdByID ( pdp_command_p, 
					  CFG_HTTP_PARAMS,
					  NULL ); 
}

extern void Gl868_Pdp_Context_Act_Config( void* ipc_data )
{
	MDM_PostAtCmdByID ( pdp_command_p, 
						  CONTEXT_ACT_CONFIG,
						  NULL ); 
}


extern void Gl868_Pdp_Context_Activation( void* ipc_data, u8 cid, u8 stat)
{
	ascii params[ 20 ];
	sprintf ( ( char * )params, "%d,%d", cid, stat );
	MDM_PostAtCmdByID ( pdp_command_p, 
						  CONTEXT_ACTIVATION,
						  params ); 
}


extern void Gl868_Dns_Response_Cach( void* ipc_data )
{
	MDM_PostAtCmdByID ( pdp_command_p, 
						  DNS_CACH,
						  NULL ); 
}


extern void Gl868_Request_Location( void* ipc_data )
{
	MDM_PostAtCmdByID ( pdp_command_p, 
						  REQUEST_LOCATION,
						  NULL ); 
}


/**
 * @brief Sends pdu to gsm modem (used for storing and sending sms)
 * @param ipc_data ipc data pointer
 * @param message message's contents
 * @param number recipient's number
 * @param store are we storing sms (TRUE) or sending (FALSE)
 */
extern u8 ack_flags;
extern void Gl868_GprsSendData ( void* ipc_data, const char connId, const ascii *message )
{
   ascii params[ 2 ];
   
  /* read message from sms_db */
  if ( !message ) 
  {
		/* Message needs contents and a number */
    return;
  }
   sprintf ( ( char * )params, "%d", connId);
  /* if it is not asccii 7bits */
 
    gprs_str_len = strlen ( ( const char* )message ) + 1;
    sprintf ( ( char* )gprs_str_msg, "%s\x1A", message );
    //printf("\r\n gprs_str_len =%d,gprs_str_msg =%s\r\n",gprs_str_len,gprs_str_msg);
	
    MDM_PostAtCmdByID ( pdp_command_p, GPRS_SSEND, ( const ascii* )params );
  
}


extern void Gl868_SendData(u8 *data, u16 size)
{
      if ( modem && modem->serial && modem->serial->write_data )
    {
      /* Modem is ready to receive txt */
      /* Directly write txt to modem */
      modem->serial->write_data ( AT_Flow_Type, ( u8 * )data, size );	  
    }

}

extern void Gl868_FtpOpen ( void* ipc_data, const char *IP, u16 Port, const char *username,  const char *passwd, const char mode)
{
   ascii params[ 60 ];

   sprintf ( ( char * )params, "\"%s:%d\",\"%s\",\"%s\",%d", IP,Port,username,passwd,mode);
  
    MDM_PostAtCmdByID ( pdp_command_p, FTP_OPEN, ( const ascii* )params );
  
}

extern void Gl868_FtpClose ( void* ipc_data )
{
    MDM_PostAtCmdByID ( pdp_command_p, FTP_CLOSE, NULL );
}


extern void Gl868_FtpTimeOut ( void* ipc_data, const short time_out)
{
   ascii params[ 10 ];
	
   sprintf ( ( char * )params, "%d", time_out);
	  
   MDM_PostAtCmdByID ( pdp_command_p, FTP_TIME_OUT, ( const ascii* )params );
	  
}

extern void Gl868_FtpGetFilesize ( void* ipc_data, const char *filename)
{
   ascii params[ 50 ];
	
   sprintf ( ( char * )params, "\"%s\"", filename);
	  
   MDM_PostAtCmdByID ( pdp_command_p, FTP_GET_FILESIZE, ( const ascii* )params );
	  
}

extern void Gl868_FtpGetFile ( void* ipc_data, const char *filename)
{
   ascii params[ 50 ];
	
   sprintf ( ( char * )params, "\"%s\"", filename);
	  
   MDM_PostAtCmdByID ( pdp_command_p, FTP_GET_FILE, ( const ascii* )params );
	  
}





/**
 * @brief Handler for sending sms at command's replies.
 * @return TRUE if replies were recognized
 */
#define CME_ERROR_OP_NO_SUPPORT   4
static AtCmdHandler_Status Gl868_Handler_SendData ( MDM_CmdTypedef *at, MDM_RspTypedef *response )
{
  MDM_RSP_ID code = MDM_GetResultCode ( response );
 
  
  if ( code == MDM_ERROR )
  {
      s32 errorCode = 0;
    
    if ( TRUE == MDM_Utils_ParseCMEerror ( response, ( s32* )&errorCode ) )
    {
      switch ( errorCode )
      {
        case CME_ERROR_OP_NO_SUPPORT:

          return AT_HANDLER_DONE_ERROR;

        default:

          return AT_HANDLER_DONE_ERROR;
      }
    }
  }
  else if ( MDM_StrStr ( response, ">" ) )
  {
    if ( modem && modem->serial && modem->serial->write_data )
    {
      /* Modem is ready to receive txt */
      /* Directly write txt to modem */
      modem->serial->write_data ( AT_Flow_Type, ( u8 * )gprs_str_msg, gprs_str_len );	  
    }

    /* Wait for ok to the txt */
    return AT_HANDLER_NEED_MORE;  
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
static AtCmdHandler_Status PDP_Handler_GprsAttach ( MDM_CmdTypedef *at, MDM_RspTypedef *response )
{
  MDM_RSP_ID code = MDM_GetResultCode ( response );

  /* Modem didn't reply or replied with an error */
  if ( code == MDM_ERROR )
  {
    return AT_HANDLER_RETRY;
  }
  else if ( code == MDM_OK )
  {
    if ( modem && modem->pdp && modem->pdp->attach_gprs_network_reply )
    {
      modem->pdp->attach_gprs_network_reply ( NULL );
    }
    return AT_HANDLER_DONE;
  }

  return AT_HANDLER_DONT_UNDERSTAND;
}

static AtCmdHandler_Status PDP_Handler_GprsDetach ( MDM_CmdTypedef *at, MDM_RspTypedef *response )
{
  MDM_RSP_ID code = MDM_GetResultCode ( response );

  /* Modem didn't reply or replied with an error */
  if ( code == MDM_ERROR )
  {
    return AT_HANDLER_RETRY;
  }
  else if ( code == MDM_OK )
  {
    if ( modem && modem->pdp && modem->pdp->detach_gprs_network_reply )
    {
      modem->pdp->detach_gprs_network_reply ( NULL );
    }
    return AT_HANDLER_DONE;
  }

  return AT_HANDLER_DONT_UNDERSTAND;
}


static AtCmdHandler_Status GL868_Handler_GprsContextAct ( MDM_CmdTypedef *at, MDM_RspTypedef *response )
{
  MDM_RSP_ID code = MDM_GetResultCode ( response );

  /* Modem didn't reply or replied with an error */
  if ( code == MDM_ERROR )
  {
    return AT_HANDLER_RETRY;
  }
  else if ( code == MDM_OK )
  {
    if ( modem && modem->pdp && modem->pdp->gprs_context_activation )
    {
      modem->pdp->gprs_context_activation ( NULL );
    }
    return AT_HANDLER_DONE;
  }

  return AT_HANDLER_DONT_UNDERSTAND;
}

static AtCmdHandler_Status GL868_Handler_GprsContextNoAct ( MDM_CmdTypedef *at, MDM_RspTypedef *response )
{
  MDM_RSP_ID code = MDM_GetResultCode ( response );

  /* Modem didn't reply or replied with an error */
  if ( code == MDM_ERROR )
  {
    return AT_HANDLER_RETRY;
  }
  else if ( code == MDM_OK )
  {
    if ( modem && modem->pdp && modem->pdp->gprs_context_no_activation )
    {
      modem->pdp->gprs_context_no_activation ( NULL );
    }
    return AT_HANDLER_DONE;
  }

  return AT_HANDLER_DONT_UNDERSTAND;
}


static AtCmdHandler_Status GL868_Socket_ConfigurationExtend ( MDM_CmdTypedef *at, MDM_RspTypedef *response )
{
  MDM_RSP_ID code = MDM_GetResultCode ( response );

  /* Modem didn't reply or replied with an error */
  if ( code == MDM_ERROR )
  {
    return AT_HANDLER_RETRY;
  }
  else if ( code == MDM_OK )
  {
    if ( modem && modem->pdp && modem->pdp->socket_configuration_extend )
    {
      modem->pdp->socket_configuration_extend ( NULL );
    }
    return AT_HANDLER_DONE;
  }

  return AT_HANDLER_DONT_UNDERSTAND;
}

static AtCmdHandler_Status GL868_Ppp_ParamConfig( MDM_CmdTypedef *at, MDM_RspTypedef *response )
{
	  MDM_RSP_ID code = MDM_GetResultCode ( response );
	
	  /* Modem didn't reply or replied with an error */
	  if ( code == MDM_ERROR )
	  {
		return AT_HANDLER_RETRY;
	  }
	  else if ( code == MDM_OK )
	  {
		if ( modem && modem->pdp && modem->pdp->ppp_param_config )
		{
		  modem->pdp->ppp_param_config ( NULL );
		}
		return AT_HANDLER_DONE;
	  }
	
	  return AT_HANDLER_DONT_UNDERSTAND;
}


static AtCmdHandler_Status GL868_Socket_Configuration( MDM_CmdTypedef *at, MDM_RspTypedef *response )
{
  MDM_RSP_ID code = MDM_GetResultCode ( response );

  /* Modem didn't reply or replied with an error */
  if ( code == MDM_ERROR )
  {
    return AT_HANDLER_RETRY;
  }
  else if ( code == MDM_OK )
  {
    if ( modem && modem->pdp && modem->pdp->socket_configuration )
    {
      modem->pdp->socket_configuration ( NULL );
    }
    return AT_HANDLER_DONE;
  }

  return AT_HANDLER_DONT_UNDERSTAND;
}

static AtCmdHandler_Status GL868_Socket_Dial ( MDM_CmdTypedef *at, MDM_RspTypedef *response )
{
  MDM_RSP_ID code = MDM_GetResultCode ( response );

  /* Modem didn't reply or replied with an error */
  if ( code == MDM_ERROR )
  {
    return AT_HANDLER_RETRY;
  }
  else if ( code == MDM_CONNECT )
  {
    if ( modem && modem->pdp && modem->pdp->socket_dial )
    {
      modem->pdp->socket_dial ( NULL );
    }
    return AT_HANDLER_DONE;
  }
  else if( code == MDM_OK )
  {
	 if ( modem && modem->pdp && modem->pdp->socket_dial )
	{
	  modem->pdp->socket_dial ( NULL );
	}
	return AT_HANDLER_DONE;
  }
  	

  return AT_HANDLER_DONT_UNDERSTAND;
}

static AtCmdHandler_Status Gl868_Request_Reference_location( MDM_CmdTypedef *at, MDM_RspTypedef *response )
{
	  MDM_RSP_ID code = MDM_GetResultCode ( response );
	  
	  if ( code == MDM_ERROR )
	  {
			/* "UNABLE TO QUERY CURRENT OPERATOR" */
		    return AT_HANDLER_DONE_ERROR;

	  }
	   else if ( code == MDM_OK ) 
	  {
            ;
	  }
	  else 
	  {
	    return AT_HANDLER_DONE;
	    //printf("\r\n response->indicator =%s\r\n",response->indicator);
#if 0
		token = MDM_StrStr ( response, "#AGPSRING" );
	   
		if ( token ) 
		{
		  memcpy(infstr, response->indicator + 9, response->totalsize - 9);
		  token = ( ascii * )strtok ( ( char * )infstr, "," );
		  if ( token != NULL ) 
		  { 
			;
		  }
		  token = NULL;
		  token = ( ascii * )strtok (NULL, "," );
		  if ( token != NULL )
		  {
              memcpy ( lat, token, strlen ( ( const char * )token ) );
			  //printf("\r\n lat =%s\r\n",lat);
			  //printf("\r\n latitude =%lf\r\n",strtod((const char *)lat,NULL));
		  }
		  token = NULL;
		  token = ( ascii * )strtok (NULL, "," );
		  if ( token != NULL )
		  {
              memcpy ( lon, token, strlen ( ( const char * )token ) );
			  //printf("\r\n lon =%s\r\n",lon);
			  //printf("\r\n longitude =%lf\r\n",strtod((const char *)lon,NULL));
		  }
		  //printf("RMC_d.long_ = %u,RMC_d.lat_ = %u",RMC_d.long_,RMC_d.lat_);
		  longitude = (u32)(strtod(lon,NULL)*1000000);
		  latitude = (u32)(strtod(lat,NULL)*1000000);
		  printf("base position: longitude = %u,latitude =%u",longitude,latitude);
		  WDG_KickCmd ();
		  
		  BKP_WriteBackupRegister(BKP_DR3,longitude&0xffff);
		  //printf("\r\n %s,%d\r\n",__func__,__LINE__);
		  BKP_WriteBackupRegister(BKP_DR4,(longitude>>16)&0xffff);
		   //printf("\r\n %s,%d\r\n",__func__,__LINE__);
		  BKP_WriteBackupRegister(BKP_DR5,latitude&0xffff);
		   //printf("\r\n %s,%d\r\n",__func__,__LINE__);
		  BKP_WriteBackupRegister(BKP_DR6,(latitude>>16)&0xffff);
		   //printf("\r\n %s,%d\r\n",__func__,__LINE__);
	   
		  return AT_HANDLER_DONE;
		}
#endif	
	  }
	 
		
		return AT_HANDLER_DONT_UNDERSTAND;
}



static AtCmdHandler_Status PDP_Handler_SocketShutDown ( MDM_CmdTypedef *at, MDM_RspTypedef *response )
{
  MDM_RSP_ID code = MDM_GetResultCode ( response );

  /* Modem didn't reply or replied with an error */
  if ( code == MDM_ERROR )
  {
    return AT_HANDLER_RETRY;
  }
  else if ( code == MDM_OK )
  {
    if ( modem && modem->pdp && modem->pdp->socket_shutdown )
    {
      modem->pdp->socket_shutdown ( NULL );
    }
    return AT_HANDLER_DONE;
  }

  return AT_HANDLER_DONT_UNDERSTAND;
}

static AtCmdHandler_Status PDP_Handler_SocketRestore ( MDM_CmdTypedef *at, MDM_RspTypedef *response )
{
	  MDM_RSP_ID code = MDM_GetResultCode ( response );
	
	  /* Modem didn't reply or replied with an error */
	  if ( code == MDM_ERROR )
	  {
		return AT_HANDLER_RETRY;
	  }
	  else if ( code == MDM_OK )
	  {
		if ( modem && modem->pdp && modem->pdp->socket_restore )
		{
		  modem->pdp->socket_restore ( NULL );
		}
		return AT_HANDLER_DONE;
	  }
	
	  return AT_HANDLER_DONT_UNDERSTAND;
}


static AtCmdHandler_Status PDP_Handler_SocketInactiveTimeOut ( MDM_CmdTypedef *at, MDM_RspTypedef *response )
{
  MDM_RSP_ID code = MDM_GetResultCode ( response );

  /* Modem didn't reply or replied with an error */
  if ( code == MDM_ERROR )
  {
    return AT_HANDLER_RETRY;
  }
  else if ( code == MDM_OK )
  {
    if ( modem && modem->pdp && modem->pdp->socket_inactive_timeout )
    {
      modem->pdp->socket_inactive_timeout ( NULL );
    }
    return AT_HANDLER_DONE;
  }

  return AT_HANDLER_DONT_UNDERSTAND;
}

static AtCmdHandler_Status PDP_Handler_SocketSet( MDM_CmdTypedef *at, MDM_RspTypedef *response )
{
  MDM_RSP_ID code = MDM_GetResultCode ( response );

  /* Modem didn't reply or replied with an error */
  if ( code == MDM_ERROR )
  {
	return AT_HANDLER_RETRY;
  }
  else if ( code == MDM_OK )
  {
	if ( modem && modem->pdp && modem->pdp->socket_set )
	{
	  modem->pdp->socket_set ( NULL );
	}
	return AT_HANDLER_DONE;
  }

  return AT_HANDLER_DONT_UNDERSTAND;
}

static AtCmdHandler_Status PDP_Handler_SocketOpen( MDM_CmdTypedef *at, MDM_RspTypedef *response )
{
  MDM_RSP_ID code = MDM_GetResultCode ( response );

  /* Modem didn't reply or replied with an error */
  if ( code == MDM_ERROR )
  {
	return AT_HANDLER_RETRY;
  }
  else if ( code == MDM_OK )
  {
	if ( modem && modem->pdp && modem->pdp->socket_open )
	{
	  modem->pdp->socket_open ( NULL );
	}
	return AT_HANDLER_DONE;
  }

  return AT_HANDLER_DONT_UNDERSTAND;
}


static AtCmdHandler_Status PDP_Select_flow_control( MDM_CmdTypedef *at, MDM_RspTypedef *response )
{
	  MDM_RSP_ID code = MDM_GetResultCode ( response );
	
	  /* Modem didn't reply or replied with an error */
	  if ( code == MDM_ERROR )
	  {
		return AT_HANDLER_RETRY;
	  }
	  else if ( code == MDM_OK )
	  {
		if ( modem && modem->pdp && modem->pdp->selet_flow_control)
		{
		  modem->pdp->selet_flow_control ( NULL );
		}
		return AT_HANDLER_DONE;
	  }
	
	  return AT_HANDLER_DONT_UNDERSTAND;
}



static AtCmdHandler_Status Gl868_Handler_Ftp_Close ( MDM_CmdTypedef *at, MDM_RspTypedef *response )
{
	  MDM_RSP_ID code = MDM_GetResultCode ( response );
	
	  /* Modem didn't reply or replied with an error */
	  if ( code == MDM_ERROR )
	  {
		return AT_HANDLER_RETRY;
	  }
	  else if ( code == MDM_OK )
	  {
		if ( modem && modem->pdp && modem->pdp->ftp_close )
		{
		  modem->pdp->ftp_close ( NULL );
		}
		return AT_HANDLER_DONE;
	  }
	
	  return AT_HANDLER_DONT_UNDERSTAND;
	}

static AtCmdHandler_Status Gl868_Handler_Ftp_Time_Out( MDM_CmdTypedef *at, MDM_RspTypedef *response )
{
	  MDM_RSP_ID code = MDM_GetResultCode ( response );

	  /* Modem didn't reply or replied with an error */
	  if ( code == MDM_ERROR )
	  {
		return AT_HANDLER_RETRY;
	  }
	  else if ( code == MDM_OK )
	  {
		if ( modem && modem->pdp && modem->pdp->ftp_time_out )
		{
		  modem->pdp->ftp_time_out ( NULL );
		}
		return AT_HANDLER_DONE;
	  }
	
	  return AT_HANDLER_DONT_UNDERSTAND;
}




/**
* @brief Converts call status to a string and returns it
*
* @param status status to convert
* @return const gchar* containing status' name
*/
extern void PDP_Command_ListGprsClasses ( void* ipc_data )
{
  MDM_PostAtCmdByID ( pdp_command_p, 
                      LIST_GPRS_CLASSES,
                      NULL ); 
}






/* 
 * 功能描述: 
 * 引用参数:
 *          
 * 返回值  :
 * 
 */
static AtCmdHandler_Status PDP_Handler_ListGprsClasses ( MDM_CmdTypedef *at, MDM_RspTypedef *response )
{
  MDM_RSP_ID code = MDM_GetResultCode ( response );

  /* Modem didn't reply or replied with an error */
  if ( code == MDM_ERROR )
  {
    return AT_HANDLER_DONE_ERROR;
  }
  else if ( code == MDM_OK )
  {
    if ( modem && modem->pdp && modem->pdp->list_gprs_classes_reply )
    {
      modem->pdp->list_gprs_classes_reply ( NULL, NULL);
    }
    return AT_HANDLER_DONE;
  }

  return AT_HANDLER_DONT_UNDERSTAND;
}





/**
* @brief Converts call status to a string and returns it
*
* @param status status to convert
* @return const gchar* containing status' name
*/
extern void PDP_Command_SelectGprsClass ( void* ipc_data, const char *class )
{
  if ( class )
  {
    MDM_PostAtCmdByID ( pdp_command_p, 
                        SELECT_GPRS_CLASS,
                        ( const ascii* )class );  
  }
}






/* 
 * 功能描述: 
 * 引用参数:
 *          
 * 返回值  :
 * 
 */
static AtCmdHandler_Status PDP_Handler_SelectGprsClass ( MDM_CmdTypedef *at, MDM_RspTypedef *response )
{
  MDM_RSP_ID code = MDM_GetResultCode ( response );

  /* Modem didn't reply or replied with an error */
  if ( code == MDM_ERROR )
  {
    return AT_HANDLER_DONE_ERROR;
  }
  else if ( code == MDM_OK )
  {
    if ( modem && modem->pdp && modem->pdp->select_gprs_class_reply )
    {
      modem->pdp->select_gprs_class_reply( NULL);
    }    
    return AT_HANDLER_DONE;
  }

  return AT_HANDLER_DONT_UNDERSTAND;
}







/**
* @brief Converts call status to a string and returns it
*
* @param status status to convert
* @return const gchar* containing status' name
*/
extern void PDP_Command_Activate ( void* ipc_data, int index )
{
  if ( index >= 1 && index <= 4 )
  {
    ascii Params[ 12 ];

    sprintf ( ( char * )Params, "%1d", index );
    MDM_PostAtCmdByID ( pdp_command_p, 
                        ACTIVE_PDP_CONTEXT,
                        ( const ascii * )Params );   
  }
  
}









/* 
 * 功能描述: 
 * 引用参数:
 *          
 * 返回值  :
 * 
 */
static AtCmdHandler_Status PDP_Handler_Activate ( MDM_CmdTypedef *at, MDM_RspTypedef *response )
{
  MDM_RSP_ID code = MDM_GetResultCode ( response );

  /* Modem didn't reply or replied with an error */
  if ( code == MDM_ERROR )
  {
    ascii *token = MDM_StrStr ( response, MDM_Sym2Str ( symbol_p, MDM_CME_ERROR ) );
    
    if ( token )
    {
      if ( modem && modem->pdp && modem->pdp->activate_error )
      {
        modem->pdp->activate_error ( NULL, token );
      }    
    }
    return AT_HANDLER_DONE_ERROR;
  }
  else if ( code == MDM_OK )
  {
    if ( modem && modem->pdp && modem->pdp->activate_reply )
    {
      modem->pdp->activate_reply ( NULL );
    }
    return AT_HANDLER_DONE;
  }

  return AT_HANDLER_DONT_UNDERSTAND;
}







/**
* @brief Converts call status to a string and returns it
*
* @param status status to convert
* @return const gchar* containing status' name
*/
extern void PDP_Command_Deactivate ( void* ipc_data, int index )
{
  if ( index >= 1 && index <= 4 )
  {
    ascii Params[ 12 ];

    sprintf ( ( char * )Params, "%1d", index );    
    MDM_PostAtCmdByID ( pdp_command_p, 
                        DEACTIVE_PDP_CONTEXT,
                        ( const ascii * )Params );   
  }
}







/* 
 * 功能描述: 
 * 引用参数:
 *          
 * 返回值  :
 * 
 */
static AtCmdHandler_Status PDP_Handler_Deactivate ( MDM_CmdTypedef *at, MDM_RspTypedef *response )
{
  MDM_RSP_ID code = MDM_GetResultCode ( response );

  /* Modem didn't reply or replied with an error */
  if ( code == MDM_ERROR )
  {
    return AT_HANDLER_RETRY;
  }
  else 
  if ( code == MDM_OK )
  {
    if ( modem && modem->pdp && modem->pdp->deactivate_reply )
    {
      modem->pdp->deactivate_reply( NULL);
    }    
    return AT_HANDLER_DONE;
  }

  return AT_HANDLER_DONT_UNDERSTAND;
}








/**
* @brief Converts call status to a string and returns it
*
* @param status status to convert
* @return const gchar* containing status' name
*/
extern void PDP_Command_SelectContext ( void* ipc_data, const char *context )
{
  if ( context )
  {
    MDM_PostAtCmdByID ( pdp_command_p, 
                        SELECT_PDP_CONTEXT,
                        NULL );   
  }
}




/* 
 * 功能描述: 
 * 引用参数:
 *          
 * 返回值  :
 * 
 */
static AtCmdHandler_Status PDP_Handler_SelectContext ( MDM_CmdTypedef *at, MDM_RspTypedef *response )
{
  MDM_RSP_ID code = MDM_GetResultCode ( response );

  /* Modem didn't reply or replied with an error */
  if ( code == MDM_ERROR )
  {
    return AT_HANDLER_DONE_ERROR;
  }
  else if ( code == MDM_OK )
  {
    if ( modem && modem->pdp && modem->pdp->select_context_reply )
    {
       modem->pdp->select_context_reply( NULL);
    }   
    return AT_HANDLER_DONE;
  }

  return AT_HANDLER_DONT_UNDERSTAND;
}





/**
* @brief Converts call status to a string and returns it
*
* @param status status to convert
* @return const gchar* containing status' name
*/

extern void CTCC_PDP_Command_AddContext(void* ipc_data ,const char *UserName, const char *Password)
{
	ascii params[ 78 ];
	sprintf ( ( char * )params, ",,\"%s\",\"%s\"", UserName, Password );
  MDM_PostAtCmdByID ( pdp_command_p, 
                        CTCC_ADD_PDP_CONTEXT,
                        params );
	


}

extern void PDP_Command_AddContext ( void* ipc_data, int index, const char *APN )
{
  ascii params[ 78 ];

  if ( APN )
  {
    sprintf ( ( char * )params, "%d,\"IP\",\"%s\",\"\",0,0", index, APN );
    MDM_PostAtCmdByID ( pdp_command_p, 
                        ADD_PDP_CONTEXT,
                        params );  
  } 
}


extern void GL868_PDP_Command_AddContext ( void* ipc_data, int index, const char *APN )
{
  ascii params[ 78 ];

  if ( APN )
  {
    sprintf ( ( char * )params, "%d,\"IP\",\"%s\"", index, APN );
    MDM_PostAtCmdByID ( pdp_command_p, 
                        ADD_PDP_CONTEXT,
                        params );  
  } 
}


extern void GL868_PDP_M2mLocate_AddContext ( void* ipc_data, int index, const char *APN )
{
  ascii params[ 78 ];

  if ( APN )
  {
    sprintf ( ( char * )params, "%d,\"IP\",\"%s\",\"0.0.0.0\",0,0", index, APN );
    MDM_PostAtCmdByID ( pdp_command_p, 
                        ADD_PDP_CONTEXT,
                        params );  
  } 
}




/* 
 * 功能描述: 
 * 引用参数:
 *          
 * 返回值  :
 * 
 */
static AtCmdHandler_Status PDP_Handler_AddContext ( MDM_CmdTypedef *at, MDM_RspTypedef *response )
{
  MDM_RSP_ID code = MDM_GetResultCode ( response );

  /* Modem didn't reply or replied with an error */
  if ( code == MDM_ERROR )
  {
    return AT_HANDLER_DONE_ERROR;
  }
  else if ( code == MDM_TIME_OUT )
  {
    return AT_HANDLER_RETRY;
  }
  else if ( code == MDM_OK )
  {
    if ( modem && modem->pdp && modem->pdp->add_context_reply )
    {
      modem->pdp->add_context_reply( NULL, TRUE );
    }
    return AT_HANDLER_DONE;
  }

  return AT_HANDLER_DONT_UNDERSTAND;
}


static AtCmdHandler_Status Cfg_Http_Params ( MDM_CmdTypedef *at, MDM_RspTypedef *response )
{
  MDM_RSP_ID code = MDM_GetResultCode ( response );

  /* Modem didn't reply or replied with an error */
  if ( code == MDM_ERROR )
  {
    return AT_HANDLER_DONE_ERROR;
  }
  else if ( code == MDM_TIME_OUT )
  {
    return AT_HANDLER_RETRY;
  }
  else if ( code == MDM_OK )
  {
    if ( modem && modem->pdp && modem->pdp->cfg_http_params )
    {
      modem->pdp->cfg_http_params( NULL );
    }
    return AT_HANDLER_DONE;
  }

  return AT_HANDLER_DONT_UNDERSTAND;
}


static AtCmdHandler_Status PDP_Context_Act_Config( MDM_CmdTypedef *at, MDM_RspTypedef *response )
{
	  MDM_RSP_ID code = MDM_GetResultCode ( response );
	
	  /* Modem didn't reply or replied with an error */
	  if ( code == MDM_ERROR )
	  {
		return AT_HANDLER_DONE_ERROR;
	  }
	  else if ( code == MDM_TIME_OUT )
	  {
		return AT_HANDLER_RETRY;
	  }
	  else if ( code == MDM_OK )
	  {
		if ( modem && modem->pdp && modem->pdp->pdp_context_act_config )
		{
		  modem->pdp->pdp_context_act_config( NULL, TRUE );
		}
		return AT_HANDLER_DONE;
	  }
	
	  return AT_HANDLER_DONT_UNDERSTAND;
}



static AtCmdHandler_Status PDP_Context_Activation( MDM_CmdTypedef *at, MDM_RspTypedef *response )
{
	  MDM_RSP_ID code = MDM_GetResultCode ( response );
	
	  /* Modem didn't reply or replied with an error */
	  if ( code == MDM_ERROR )
	  {
		return AT_HANDLER_DONE_ERROR;
	  }
	  else if ( code == MDM_TIME_OUT )
	  {
		return AT_HANDLER_RETRY;
	  }
	  else if ( code == MDM_OK )
	  {
		if ( modem && modem->pdp && modem->pdp->pdp_context_activation )
		{
		  modem->pdp->pdp_context_activation( NULL, TRUE );
		}
		return AT_HANDLER_DONE;
	  }
	
	  return AT_HANDLER_DONT_UNDERSTAND;
}


static AtCmdHandler_Status Dns_Response_Cach( MDM_CmdTypedef *at, MDM_RspTypedef *response )
{
	  MDM_RSP_ID code = MDM_GetResultCode ( response );
	
	  /* Modem didn't reply or replied with an error */
	  if ( code == MDM_ERROR )
	  {
		return AT_HANDLER_DONE_ERROR;
	  }
	  else if ( code == MDM_TIME_OUT )
	  {
		return AT_HANDLER_RETRY;
	  }
	  else if ( code == MDM_OK )
	  {
		if ( modem && modem->pdp && modem->pdp->dns_response_cach )
		{
		  modem->pdp->dns_response_cach( NULL, TRUE );
		}
		return AT_HANDLER_DONE;
	  }
	
	  return AT_HANDLER_DONT_UNDERSTAND;
}

/**
* @brief Converts call status to a string and returns it
*
* @param status status to convert
* @return const gchar* containing status' name
*/
extern void PDP_Command_DeleteContext ( void* ipc_data, int index )
{
  ascii Params[ 12 ];

  sprintf ( ( char * )Params, "%1d", index );   
  MDM_PostAtCmdByID ( pdp_command_p, 
                      DELETE_PDP_CONTEXT,
                      ( const ascii * )Params );   
}









/* 
 * 功能描述: 
 * 引用参数:
 *          
 * 返回值  :
 * 
 */
static AtCmdHandler_Status PDP_Handler_DeleteContext ( MDM_CmdTypedef *at, MDM_RspTypedef *response )
{
  MDM_RSP_ID code = MDM_GetResultCode ( response );

  /* Modem didn't reply or replied with an error */
  if ( code == MDM_ERROR )
  {
    return AT_HANDLER_DONE_ERROR;
  }
  else if ( code == MDM_OK )
  {
    if ( modem && modem->pdp && modem->pdp->delete_context_reply )
    {
      modem->pdp->delete_context_reply ( NULL );
    }
    return AT_HANDLER_DONE;
  }

  return AT_HANDLER_DONT_UNDERSTAND;
}





/**
* @brief Converts call status to a string and returns it
*
* @param status status to convert
* @return const gchar* containing status' name
*/
extern void PDP_Command_ListContexts ( void* ipc_data )
{
  MDM_PostAtCmdByID ( pdp_command_p, 
                      LIST_PDP_CONTEXT,
                      NULL );  
}









/* 
 * 功能描述: 
 * 引用参数:
 *          
 * 返回值  :
 * 
 */
static AtCmdHandler_Status PDP_Handler_listContext ( MDM_CmdTypedef *at, MDM_RspTypedef *response )
{
  MDM_RSP_ID code = MDM_GetResultCode ( response );

  /* Modem didn't reply or replied with an error */
  if ( code == MDM_ERROR )
  {
    return AT_HANDLER_DONE_ERROR;
  }
  else if ( code == MDM_TIME_OUT )
  {
    return AT_HANDLER_RETRY;
  }
  else if ( code == MDM_OK )
  {
    if ( modem && modem->pdp && modem->pdp->list_contexts_reply )
    {
      const char *contexts[] = { "First", "Second", "Third" };

      modem->pdp->list_contexts_reply ( NULL, contexts );
    }
    return AT_HANDLER_DONE;
  }

  return AT_HANDLER_DONT_UNDERSTAND;
}









/**
* @brief Converts call status to a string and returns it
*
* @param status status to convert
* @return const gchar* containing status' name
*/
extern void PDP_Command_RequestGPRSservice ( void* ipc_data, int index )
{
  ascii Params[ 12 ];

  sprintf ( ( char * )Params, "%1d", index );  
  MDM_PostAtCmdByID ( pdp_command_p, 
                      REQUEST_GPRS_SERVICE,
                      ( const ascii * )Params );   
}










/* 
 * 功能描述: 
 * 引用参数:
 *          
 * 返回值  :
 * 
 */
static AtCmdHandler_Status PDP_Handler_RequestGPRSservice ( MDM_CmdTypedef *at, MDM_RspTypedef *response )
{
  MDM_RSP_ID code = MDM_GetResultCode ( response );
  static s8 pdp_err_count = 0;

  /* GPRS连续拨号失败的最大次数 */
  #define PDP_MAX_ERR_COUNT   6

  /* Modem didn't reply or replied with an error */
  if ( code == MDM_ERROR || code == MDM_NO_CARRIER )
  {
    ascii *token = MDM_StrStr ( response, MDM_Sym2Str ( symbol_p, MDM_CME_ERROR ) ); 

    if ( token )
    {
      /* NEW #001 增加连续拨号失败最大次数的异常通知机制 */
      pdp_err_count++;
      if ( pdp_err_count > PDP_MAX_ERR_COUNT )
      {
        pdp_err_count = 0;
        MDM_FlowServiceException ( MDM_FCM_EVENT_DIAL_ERR );
				printf("\r\n---->拨号失败.......\r\n");
      }
      else
      if ( modem && modem->pdp && modem->pdp->request_gprs_service_error )
      {
        modem->pdp->request_gprs_service_error ( NULL, token );
      }
    }
    return AT_HANDLER_RETRY;
  }
  else if ( code == MDM_CONNECT )
  {
	
    pdp_err_count = 0;
    MDM_FlowSwitchV24State ( MDM_FCM_V24_STATE_DATA );
    return AT_HANDLER_DONE;
  }

  return AT_HANDLER_DONT_UNDERSTAND;
}





