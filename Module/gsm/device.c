







#include <stdio.h>
#include <stdlib.h>
#include <string.h>


//#include "tz_common.h"
#include "defines.h"
#include "stdbool.h"
#include "modem.h"
#include "tiza_params.h"

/* Common routines */
#define IS_AF(c)  ((c >= 'A') && (c <= 'F'))
#define IS_af(c)  ((c >= 'a') && (c <= 'f'))
#define IS_09(c)  ((c >= '0') && (c <= '9'))
#define ISVALIDHEX(c)  IS_AF(c) || IS_af(c) || IS_09(c)
#define ISVALIDDEC(c)  IS_09(c)
#define CONVERTDEC(c)  (c - '0')




typedef struct
{
  /* information is valid? */
  bool is_valid;

  /* Product Serial Number Identification */
  ascii IMEI[ 16 ];

  /* Revision Identification */
  ascii SVN[ 64 ];

  /* Manufacturer Identification. */
  ascii ManufacturerID[ 64 ];

  /* Model Identification. */
  ascii ModelID[ 64 ];
} Device_InfoTypedef;


extern ascii E_IMEI[16] = {0};




/******************************************/
/*              内部函数[声明]            */
/******************************************/

static AtCmdHandler_Status Device_Handler_QueryDateTime     ( MDM_CmdTypedef *at, MDM_RspTypedef *atrsp );
static AtCmdHandler_Status Device_Handler_SetDateTime       ( MDM_CmdTypedef *at, MDM_RspTypedef *atrsp );
static AtCmdHandler_Status Device_Handler_QueryIMEI         ( MDM_CmdTypedef *at, MDM_RspTypedef *atrsp );
static AtCmdHandler_Status Device_Handler_QueryRevision     ( MDM_CmdTypedef *at, MDM_RspTypedef *atrsp );
static AtCmdHandler_Status Device_Handler_QueryCCID     	( MDM_CmdTypedef *at, MDM_RspTypedef *atrsp );
static AtCmdHandler_Status Device_Handler_QueryModel        ( MDM_CmdTypedef *at, MDM_RspTypedef *atrsp );
static AtCmdHandler_Status Device_Handler_QueryManufacturer ( MDM_CmdTypedef *at, MDM_RspTypedef *atrsp );
static AtCmdHandler_Status Device_Handler_SetAntennaPower   ( MDM_CmdTypedef *at, MDM_RspTypedef *atrsp );
static AtCmdHandler_Status Device_Handler_GetAntennaPower   ( MDM_CmdTypedef *at, MDM_RspTypedef *atrsp );
static AtCmdHandler_Status Device_Handler_GetInfo           ( MDM_CmdTypedef *at, MDM_RspTypedef *atrsp );
static AtCmdHandler_Status Device_Handler_ClearRING( MDM_CmdTypedef *at, MDM_RspTypedef* response );






/******************************************/
/*              内部变量[定义]            */
/******************************************/

extern MDM_DevTypedef *modem;


static MDM_CmdTypedef device_commands[] 
= {
  /* Query datetime */
  { DATE_TIME_QUERY, "AT+CCLK?\r\n",             
  	100, 0,
    Device_Handler_QueryDateTime, SIM_READY, FALSE },

  /* Set datetime */
  { SET_DATE_TIME, "AT+CCLK=\"%s\"\r\n",       
  	1000, 0,
    Device_Handler_SetDateTime, SIM_READY, FALSE },

  /* Get IMEI */
  { IMEI_QUERY, "AT+CGSN\r\n",              
  	100, 1,
    Device_Handler_QueryIMEI, SIM_UNKNOWN, FALSE },

  /* Get SW revision */
  { SW_REVISION_QUERY, "AT+CGMR\r\n",              
  	100, 0,
    Device_Handler_QueryRevision, SIM_UNKNOWN, FALSE },
	
	/* CLEAR phone_RING */
  { CALL_RING, "AT+CRIRS\r\n", 
  	100, 1,
    Device_Handler_ClearRING, SIM_READY, FALSE },

  /* Get model */
  { MODEL_QUERY, "AT+CGMM\r\n",              
  	100, 0,
    Device_Handler_QueryModel, SIM_UNKNOWN, FALSE },

  /* Get manufacturer */
  { MANUFACTURER_QUERY, "AT+CGMI\r\n",              
  	100, 0,
    Device_Handler_QueryManufacturer, SIM_UNKNOWN, FALSE },

  /* Set antenna power */
  { ANTENNA_POWER_SET, "AT+CFUN=1\r\n",            
  	100, 0,
    Device_Handler_SetAntennaPower, SIM_READY, FALSE },

  /* Get antenna power */
  { ANTENNA_POWER_GET, "AT+CFUN?\r\n",            
  	100, 0,
    Device_Handler_GetAntennaPower, SIM_UNKNOWN, FALSE },

	  /* Get CCID */
  { SW_CCID_QUERY, "AT+CICCID\r\n",              
  	100, 0,
    Device_Handler_QueryCCID, SIM_UNKNOWN, FALSE },
	
		
//	{ SW_CCID_QUERY, "AT+LCTSW\r\n",              
//  	100, 0,
//    Device_Handler_QueryCCID, SIM_UNKNOWN, FALSE },

  { 0, NULL, 0, 0, NULL, SIM_UNKNOWN, FALSE }
}, *device_command_p = device_commands;


static Device_InfoTypedef device = { FALSE, "", "", "", "" };











/* 
 * 功能描述: 
 * 引用参数:
 *          
 * 返回值  :
 * 
 */
static bool Device_Command_GetInfoReply (  void* ipc_data, ascii *info[] )
{
  if ( modem && modem->device && modem->device->get_info_reply )
  {
    modem->device->get_info_reply ( ipc_data, info );
  }

  return TRUE;
}







/* 
 * 功能描述: 
 * 引用参数:
 *          
 * 返回值  :
 * 
 */
extern void Device_Command_GetInfo (  void* ipc_data )
{
  if ( device.is_valid )
  {
    Device_Command_GetInfoReply ( NULL, NULL );
  }
  else
  {
    device_commands[ 2 ].handler = Device_Handler_GetInfo;
    MDM_PostAtCmdByID ( device_command_p, 
                        IMEI_QUERY, 
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
static AtCmdHandler_Status Device_Handler_GetInfo ( MDM_CmdTypedef *at, MDM_RspTypedef* response )
{
  AtCmdHandler_Status status = AT_HANDLER_DONT_UNDERSTAND;

  switch ( at->cmd_id ) 
  {
    case IMEI_QUERY:
      status = Device_Handler_QueryIMEI ( at, response );
      switch ( status ) 
      {
        case AT_HANDLER_DONE_ERROR:
        case AT_HANDLER_ERROR:
                device_commands[ 2 ].handler = Device_Handler_QueryIMEI;
                return AT_HANDLER_DONE_ERROR;
                
        case AT_HANDLER_DONE:
                device_commands[ 2 ].handler = Device_Handler_QueryIMEI;
                device_commands[ 5 ].handler = Device_Handler_GetInfo;
                MDM_PostAtCmdByID ( device_command_p, MANUFACTURER_QUERY, NULL );
                status = AT_HANDLER_DONE;
                break;

        case AT_HANDLER_DONT_UNDERSTAND:
        case AT_HANDLER_NEED_MORE:
        case AT_HANDLER_RETRY:
                return status;
      }
      break;
            
    case MANUFACTURER_QUERY:
      status = Device_Handler_QueryManufacturer ( at, response );
      switch ( status )
      {
        case AT_HANDLER_DONE_ERROR:
        case AT_HANDLER_ERROR:
                return AT_HANDLER_DONE_ERROR;
                
        case AT_HANDLER_DONE:
                device_commands[ 5 ].handler = Device_Handler_QueryManufacturer;
                device_commands[ 3 ].handler = Device_Handler_GetInfo;
                MDM_PostAtCmdByID ( device_command_p, SW_REVISION_QUERY, NULL );
                status = AT_HANDLER_DONE;
                break;
                
        case AT_HANDLER_DONT_UNDERSTAND:
        case AT_HANDLER_NEED_MORE:
        case AT_HANDLER_RETRY:
                return status;
      }
      break;
            
    case SW_REVISION_QUERY:
      status = Device_Handler_QueryRevision ( at, response );
      switch ( status )
      {
        case AT_HANDLER_DONE_ERROR:
        case AT_HANDLER_ERROR:
                return AT_HANDLER_DONE_ERROR;
                
        case AT_HANDLER_DONE:
                device_commands[ 3 ].handler = Device_Handler_QueryRevision;
                device_commands[ 4 ].handler = Device_Handler_GetInfo;
                MDM_PostAtCmdByID ( device_command_p, MODEL_QUERY, NULL );
                status = AT_HANDLER_DONE;
                break;
                
        case AT_HANDLER_DONT_UNDERSTAND:
        case AT_HANDLER_NEED_MORE:
        case AT_HANDLER_RETRY:
                return status;
      }
      break;
            
    case MODEL_QUERY:
      status = Device_Handler_QueryModel ( at, response );
      switch ( status )
      {
        case AT_HANDLER_DONE_ERROR:
        case AT_HANDLER_ERROR:
                return AT_HANDLER_DONE_ERROR;
                
        case AT_HANDLER_DONE:
                device.is_valid = TRUE;
                device_commands[ 4 ].handler = Device_Handler_QueryModel;
                Device_Command_GetInfoReply ( NULL, NULL );
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







/****************************************************
 *               C C L K toolbox           
 * The Read command returns the current date, time and time zone setting.
 * By default, <time> will represent the network updated time.
 * If the user has used the Set command once, then <time> will represent the Set command setting.
 * Note: If network operator does not support System Clock Update Message, the initial date, time and time zone, 
 * displayed by CCLK Read Command could be invalid (user's responsibility to set date, 
 * time and time zone by CCLK Set Command).
 * Note: See Execute Command for how-to enable back network update time.
 ****************************************************/


/**
 * @brief AtCommand to query datetime
 * @param modem modem whose response to handle
 */
void Device_Command_QueryDateTime ( void* ipc_data )
{
  MDM_PostAtCmdByID ( device_command_p, 
                      DATE_TIME_QUERY, 
                      NULL );
}
extern void GL868_Get_Version(void* ipc_data)
{
    MDM_PostAtCmdByID ( device_command_p, SW_REVISION_QUERY, NULL );
}

extern void GL868_Clear_RING(void* ipc_data)
{
    MDM_PostAtCmdByID ( device_command_p, CALL_RING, NULL );
}

extern void GL868_GET_ICCID(void* ipc_data)
{
    MDM_PostAtCmdByID ( device_command_p, SW_CCID_QUERY, NULL );
}


/**
 * @brief Handler to modems response from queryDateTime command
 * @param modem modem whose response to handle
 * @return true if responses were recognized
 */
static AtCmdHandler_Status Device_Handler_QueryDateTime ( MDM_CmdTypedef *at, MDM_RspTypedef* response )
{
  MDM_RSP_ID code = MDM_GetResultCode ( response );

  if ( code == MDM_TIME_OUT ) 
  {
    return AT_HANDLER_DONE;
  }
  else if ( !MDM_StrStr ( response, "+" ) )
  {
    return AT_HANDLER_DONT_UNDERSTAND;
  }    
  else
  {
    ascii *token = MDM_StrStr ( response, MDM_Sym2Str ( symbol_p, MDM_CCLK ) );

    return AT_HANDLER_DONE;
  }
}






/****************************************************
 *               C C L K toolbox           
 * The Set command sets the date, time and time zone of the system clock.
 * Note: Set Command sets user defined system clock values and saves 
 * them in the NVM memory. These saved values are kept after power-cycle as well.
 ****************************************************/

/**
 * @brief AtCommand to set datetime
 * @param modem modem whose response to handle
 * @param date_time datetime to set. Format "yy/MM/dd,hh:mm:ss:zz"
 */
extern void Device_Command_SetDateTime ( void* ipc_data, const char* date_time )
{
  MDM_PostAtCmdByID ( device_command_p, 
                      SET_DATE_TIME, 
                      ( const ascii* )date_time );
}



/**
 * @brief Handler to modems response from setDateTime command
 * @param modem modem whose response to handle
 * @return true if responses were recognized
 */
static AtCmdHandler_Status Device_Handler_SetDateTime (  MDM_CmdTypedef *at, MDM_RspTypedef* response )
{
  MDM_RSP_ID code = MDM_GetResultCode ( response );

  if ( code == MDM_OK ) 
  {
    return AT_HANDLER_DONE;
  } 
  else if ( ( code == MDM_ERROR )||( code == MDM_TIME_OUT ) ) 
  { 
  	/* if format uncorrect, */
    return AT_HANDLER_DONE;
  }

  return AT_HANDLER_DONT_UNDERSTAND;
}








/****************************************************
 *               C G S N toolbox           
 * This command displays the product serial number identification IMEI (International Mobile 
 * Equipment Identification). It can be used even when the SIM card is not inserted.
 ****************************************************/


/* 
 * 功能描述: 
 * 引用参数:
 *          
 * 返回值  :
 * 
 */
extern void Device_Command_QueryIMEI ( void* ipc_data )
{
  MDM_PostAtCmdByID ( device_command_p, 
                      IMEI_QUERY, 
                      NULL );  
}





/* 
 * 功能描述: 
 * 引用参数:
 *          
 * 返回值  :
 * 
 */
static AtCmdHandler_Status Device_Handler_QueryIMEI ( MDM_CmdTypedef *at, MDM_RspTypedef* response )
{
  bool is_imei = TRUE;
  ascii *token = MDM_StrStr ( response, MDM_Sym2Str( symbol_p, MDM_CGSN ) );

  if ( token ) 
  {
		
//		
//		printf("\r\n token: %s\r\n",token);

//#if defined __USE_G24__
//    token = MDM_StrStr ( response, "\"" );
//    ++token;
//    token = ( ascii * )strtok ( ( char * )token, "\"" );
//#else
//    token = ( ascii * )strstr ( ( char * )token, "\r\n" );
//    token += 2;
//#endif

    if ( token ) 
    {
			token=MDM_StrStr ( response, (ascii * )"CGSN" );
			
			
      if ( strlen ( ( const char * )token ) >= 15 )
      {
				
				token=token+9;
        strncpy ( ( char * )device.IMEI, ( const char * )token, 16 );
//        if ( strlen ( ( const char * )device.IMEI ) == 16 ) 
//        {
//          s32 i = 0;
//					
//					printf("\r\n*****************************\r\n");
//          for ( i = 0; i < 15; i++ ) 
//          {
//						printf("%c",device.IMEI[i]);
//            if ( !ISVALIDHEX ( device.IMEI[ i ] ) ) 
//            {
//              is_imei = FALSE;
//							//printf("\r\n device.IMEI: %s\r\n",device.IMEI);
//              break;
//            }
//          }
//        } 
//        else 
//        {
//        	is_imei = FALSE;
//        }
      }
    }
  }
   
  if ( is_imei == FALSE ) 
  {
			return AT_HANDLER_RETRY;
  }
  else
  {
			strcpy ( ( char * )E_IMEI, ( const char * )device.IMEI);
			printf("\r\n E_IMEI: %s\r\n",E_IMEI);
			if ( modem && modem->device && modem->device->get_imei_reply )
			{
					modem->device->get_imei_reply ( NULL, ( ascii * )device.IMEI );
			}
			return AT_HANDLER_DONE;
  }
}







/****************************************************
 *               C G M R toolbox           
 * These commands request the revision identification. The G24 outputs a string containing the 
 * revision identification information of the software version contained within the device. 
 ****************************************************/

/* 
 * 功能描述: 
 * 引用参数:
 *          
 * 返回值  :
 * 
 */
 
/*外部变量*/
extern status_param drv_param;
extern u32 DecToInt ( ascii *str, u16 len );


static AtCmdHandler_Status Device_Handler_ClearRING( MDM_CmdTypedef *at, MDM_RspTypedef* response )
{
	MDM_RSP_ID code = MDM_GetResultCode ( response );
	if ( code == MDM_OK ) 
  {
    return AT_HANDLER_DONE;
  } 
  else if ( ( code == MDM_ERROR )||( code == MDM_TIME_OUT ) ) 
  { 
  	/* if format uncorrect, */
    return AT_HANDLER_DONE;
  }
}


static AtCmdHandler_Status Device_Handler_QueryRevision ( MDM_CmdTypedef *at, MDM_RspTypedef* response )
{
  ascii * token = MDM_StrStr ( response, MDM_Sym2Str ( symbol_p, MDM_CGMR ) );
  if ( token ) 
  {
    token = MDM_StrStr ( response, "Revision" );
	  memset(drv_param.gsm_ver,0,32);
	  token=token+10;
	  //printf("\r\n token=%s \r\n",token);
	  strncpy ( ( char * )drv_param.gsm_ver, ( const char * )token, 11 );
	  //drv_param.gsm_ver = DecToInt(device.SVN,strlen(( const char * )device.SVN));
	  //printf("Revision=%s \r\n",drv_param.gsm_ver);
	  return AT_HANDLER_DONE;
  }
  else 
  {
    return AT_HANDLER_NEED_MORE;
  }

  return AT_HANDLER_DONT_UNDERSTAND;
}

static AtCmdHandler_Status Device_Handler_QueryCCID ( MDM_CmdTypedef *at, MDM_RspTypedef* response )
{
	
	
	MDM_RSP_ID code = MDM_GetResultCode ( response );
  if ( code == MDM_TIME_OUT )
  {

    MDM_PostAtCmdByID ( device_command_p, SW_CCID_QUERY, NULL );  
    return AT_HANDLER_ERROR;
  }
  else
  if ( code == MDM_ERROR )
  {
    MDM_PostAtCmdByID ( device_command_p, SW_CCID_QUERY, NULL );   
    return AT_HANDLER_ERROR;
  }
  else
  if ( code == MDM_OK )
	{	
		ascii * token = MDM_StrStr ( response, MDM_Sym2Str ( symbol_p, MDM_CCID ) );
		if ( token ) 
		{
			//token = MDM_StrStr ( response, "^SCID:" );
			token = MDM_StrStr ( response, "+ICCID:" );
			
			memset(drv_param.ccid,0,20);
			token=token+8;
			strncpy ( ( char * )drv_param.ccid, ( const char * )token, 20 );
			//printf("ICCIC: %s",drv_param.ccid);
			return AT_HANDLER_DONE;
		}
	}
	return AT_HANDLER_DONT_UNDERSTAND;
}







/****************************************************
 *               C G M I toolbox           
 * These commands display manufacturer identification. The G24 outputs a string containing 
 * manufacturer identification information, indicating that this is a Motorola device. 
 ****************************************************/

/* 
 * 功能描述: 
 * 引用参数:
 *          
 * 返回值  :
 * 
 */
static AtCmdHandler_Status Device_Handler_QueryManufacturer ( MDM_CmdTypedef *at, MDM_RspTypedef* response )
{
  ascii * token = MDM_StrStr ( response, MDM_Sym2Str ( symbol_p, MDM_CGMI ) );

  if ( token ) 
  {
    token = MDM_StrStr ( response, "\"" );
    token = ( ascii * )strtok ( ( char * )++token, "\"" );
    if ( token ) 
    {
      if ( strlen ( ( const char * )token ) < 64 )
      {
        strncpy ( ( char * )device.ManufacturerID, ( const char * )token, strlen ( ( const char * )token ) );
        return AT_HANDLER_DONE;
      }
    }
  }
  else 
  {
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
static AtCmdHandler_Status Device_Handler_QueryModel ( MDM_CmdTypedef *at, MDM_RspTypedef* response )
{
  ascii *token = MDM_StrStr ( response, MDM_Sym2Str ( symbol_p, MDM_CGMM ) );

  if ( token ) 
  {
    token = MDM_StrStr ( response, "\"" );
    if ( token ) 
    {
      if ( strlen ( ( const char * )token ) < 64 )
      {
        strncpy ( ( char * )device.ModelID, ( const char * )token, strlen ( ( const char * )token ) );
        return AT_HANDLER_DONE;
      }
    }
  }
  else
  {
    return AT_HANDLER_NEED_MORE;
  }  

  return AT_HANDLER_DONT_UNDERSTAND;
}









/****************************************************
 *               C F U N toolbox           
 * This command shuts down the phone functionality of smart phones and PDAs with phone 
 * capabilities in order to prevent interference from a nearby environment. 
 * This enables other functionality to continue to be used in environments where phone use is either impractical or not permitted. 
 * For example, on airplanes the use of cellular phones is forbidden during the entire 
 * flight, but the use of computers is allowed during much of the flight. This command enables other 
 * functionality to continue while preventing use of phone functionality. 
 ****************************************************/

/* 
 * 功能描述: 
 * 引用参数:
 *          
 * 返回值  :
 * 
 */
void Device_Command_SetAntennaPower ( void* ipc_data, bool antenna_power )
{
  MDM_PostAtCmdByID ( device_command_p, 
                      ANTENNA_POWER_SET, 
                      NULL );
}




/* 
 * 功能描述: 
 * 引用参数:
 *          
 * 返回值  :
 * 
 */
static AtCmdHandler_Status Device_Handler_SetAntennaPower ( MDM_CmdTypedef *at, MDM_RspTypedef* response )
{
  MDM_RSP_ID code = MDM_GetResultCode ( response );

  if ( code == MDM_OK ) 
  {
    ascii *token = MDM_StrStr ( response, MDM_Sym2Str ( symbol_p, MDM_CFUN ) );

    if ( token ) 
    {
      token = MDM_StrStr ( response, " " );

      if ( token )
      {
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
void Device_Command_GetAntennaPower ( void* ipc_data )
{
  MDM_PostAtCmdByID ( device_command_p, 
                      ANTENNA_POWER_GET, 
                      NULL );
}





/* 
 * 功能描述: 
 * 引用参数:
 *          
 * 返回值  :
 * 
 */
static AtCmdHandler_Status Device_Handler_GetAntennaPower ( MDM_CmdTypedef *at, MDM_RspTypedef* response )
{
  MDM_RSP_ID code = MDM_GetResultCode ( response );

  if ( code == MDM_OK ) 
  {
    ascii *token = MDM_StrStr ( response, MDM_Sym2Str ( symbol_p, MDM_CFUN ) );

    if ( token ) 
    {
      token = MDM_StrStr ( response, " " );

      if ( token )
      {
        if ( !ISVALIDDEC ( ( int )token ) )
        {
          return AT_HANDLER_DONE_ERROR;
        }
        else
        {
          if ( modem && modem->device && modem->device->get_antenna_power_reply )
          {
            /* displays the current level of functionality.
               0 Full functionality (default 0)
               1 Full functionality
               4 Disables phone transmit & receive RF circuits */
            modem->device->get_antenna_power_reply ( NULL, ( atoi ( ( const char * )token ) - 0x30 ) ? TRUE : FALSE );          
          }
          return AT_HANDLER_DONE;
        }
      } 
    } 
  }

  return AT_HANDLER_DONT_UNDERSTAND;
}



