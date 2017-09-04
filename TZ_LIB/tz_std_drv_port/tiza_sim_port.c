





#include <stdio.h>
#include <string.h>

#include "tiza_sim_port.h"

//#include "general.h"

//#include "tz_common.h"

#include "defines.h"

#include "pdp_interface.h"
#include "shell_usercmd.h"




/******************************************/
/*              外部变量[声明]            */
/******************************************/

/* 信号强度 */
static u8 csq = 0;

/* 网络状态 */
extern bool networkStatus = FALSE;

/* 定义SIM卡信息 */
extern ascii mySimCard[ 16 ] = { 0 };



/* 
 * 功能描述：
 * 引用参数：无
 *          
 * 返回值  ：无
 * 
 */
extern void sim_get_current_status ( SIM_Status status )
{
  switch ( status )
  {
    case SIM_UNKNOWN:
#if SIM_NETWORK_DEBUG == 1
{
      PRINTF_GPS_TIME ();
      printf ( "[SIM EVENT] SIM UNKNOWN\r\n" );
}
#endif  /* SIM_NETWORK_DEBUG */

    case SIM_MISSING:
#if SIM_NETWORK_DEBUG == 1
{
      PRINTF_GPS_TIME ();
      printf ( "[SIM EVENT] SIM MISSING\r\n" );
}
#endif  /* SIM_NETWORK_DEBUG */

    case SIM_ERROR:
#if SIM_NETWORK_DEBUG == 1
{
      PRINTF_GPS_TIME ();
      printf ( "[SIM EVENT] SIM ERROR\r\n" );
}
#endif  /* SIM_NETWORK_DEBUG */

      //STATUS_SetObjectStatus ( STATUS_SIM_CARD_OK_BIT, FALSE );


//      /* 延时复位 */
      TMR_Subscribe ( FALSE, 10, TMR_TYPE_100MS, ( tmr_procTriggerHdlr_t )System_ResetDly );
      break;

    case SIM_NEED_PIN:
#if SIM_NETWORK_DEBUG == 1
{
      PRINTF_GPS_TIME ();
      printf ( "[SIM EVENT] SIM NEED PIN\r\n" );
}
#endif  /* SIM_NETWORK_DEBUG */
      break;

    case SIM_NEED_PUK:
#if SIM_NETWORK_DEBUG == 1
{
      PRINTF_GPS_TIME ();
      printf ( "[SIM EVENT] SIM NEED PUK\r\n" );
}
#endif  /* SIM_NETWORK_DEBUG */
      break;

    case SIM_READY:
#if SIM_NETWORK_DEBUG == 1
{
      PRINTF_GPS_TIME ();
      printf ( "[SIM EVENT] SIM READY\r\n" );
}
#endif  /* SIM_NETWORK_DEBUG */

	
      networkStatus = FALSE;

      //lgsm_sim_get_sim_info ( NULL );
			SIM_Command_GetImsi(NULL);
			

#if SIM_NETWORK_DEBUG == 1
{
      PRINTF_GPS_TIME ();
      printf ( "[SIM EVENT] query csca\r\n" );
}
#endif  /* SIM_NETWORK_DEBUG */

      //lgsm_sim_get_service_center_number ( NULL );
      break;
  }
}






/* 
 * 功能描述：获取短信服务中心号码
 * 引用参数：无
 *          
 * 返回值  ：无
 * 
 */
extern void sim_get_service_center_number ( void* ipc_data, const ascii *number )
{
  if ( number )
  {
#if 0
    if ( strcmp ( ( const char * )vehicleParams.sc1, ( const char * )number ) != 0 )
    {

#if SIM_NETWORK_DEBUG == 1
{
      PRINTF_GPS_TIME ();
      printf ( "[SIM EVENT] set csca %s\r\n", vehicleParams.sc1 );
}
#endif  /* SIM_NETWORK_DEBUG */

      lgsm_sim_set_service_center_number ( NULL, ( const ascii * )vehicleParams.sc1 );  
    } 
#endif
  }
}









/* 
 * 功能描述：
 * 引用参数：无
 *          
 * 返回值  ：无
 * 
 */
extern void sim_get_imsi( void* ipc_data, ascii *imsi)
{
 
  if ( imsi != NULL)
  {
    //STATUS_SetObjectStatus ( STATUS_SIM_CARD_OK_BIT, TRUE );

#if SIM_NETWORK_DEBUG == 1
{
    PRINTF_GPS_TIME ();
    printf ( "[SIM EVENT] IMSI code %s \r\n", imsi );
}
#endif  /* SIM_NETWORK_DEBUG */

//    if ( strcmp ( ( const char * )vehicleParams.imsi_code, TZ_DEFAULT_SIM_IMSICODE ) == 0 )
//    {
//      STATUS_SetObjectStatus ( STATUS_IMSI_CODE_FAULT_BIT, TRUE );

//      sprintf ( ( char * )vehicleParams.imsi_code, "%s", imsi );
//      STORAGE_WriteData ( VEHICLE_PARAMS, sizeof( vechile_Params_t ), ( u8 * )&vehicleParams );
//    }
//    else
//    if ( strcmp ( ( const char * )vehicleParams.imsi_code, ( const char * )imsi ) == 0 )
//    {
//      STATUS_SetObjectStatus ( STATUS_IMSI_CODE_FAULT_BIT, TRUE );
//    }

    //STORAGE_ReadData ( GPRS_PARAMS, sizeof( gprs_Params_t ), ( u8* )&gprsParams );
   
    //lgsm_sim_list_preferred_message ( NULL, SMS_STAT_REC_UNREAD );//读短信功能
    lgsm_network_query_signal_strength ( NULL );
    lgsm_network_get_status ( NULL );
  }
}






/* 						
 * 功能描述：获取imei码
 * 引用参数：无
 *          
 * 返回值  ：无
 * 
 */
extern void sim_get_imei_code ( void* ipc_data, ascii *imei )
{

}





/* 
 * 功能描述：
 * 引用参数：无
 *          
 * 返回值  ：无
 * 
 */
extern void sim_get_info_error ( void* ipc_data )
{

#if SIM_NETWORK_DEBUG == 1
{
  PRINTF_GPS_TIME ();
  printf ( "[SIM EVENT] sim get info error\r\n" );
}
#endif  /* SIM_NETWORK_DEBUG */

  lgsm_sim_get_sim_info ( NULL );
}







/* 
 * 功能描述：
 * 引用参数：无
 *          
 * 返回值  ：无
 * 
 */
extern void sim_set_sim_own_number_error ( void* ipc_data )
{
//  lgsm_sim_set_sim_own_number ( NULL, ( const ascii * )vehicleParams.sim_no );
}











/* 
 * 功能描述：每120秒查询一次CSQ状态
 * 引用参数：无
 *          
 * 返回值  ：无
 * 
 */
extern void network_query_signal_strength ( void* ipc_data, u8 strength )
{  
  if ( strength == 99 )
  {
    csq = 0;  
  }
  else
  {
    csq = strength;
  }

  if ( strength < 10 || strength == 99 )
  {
    //STATUS_SetObjectStatus ( STATUS_CSQ_SIGNAL_OK_BIT, FALSE ); 
  }
  else
  {
    //STATUS_SetObjectStatus ( STATUS_CSQ_SIGNAL_OK_BIT, TRUE ); 
  }  
}








/* 
 * 功能描述：获取信号强度
 * 引用参数：
 *          
 * 返回值  ：模块信号强度
 * 
 */
extern u8 getCsq ( void )
{
	return csq;
}








/* 
 * 功能描述：
 * 引用参数：
 *          
 * 返回值  ：
 * 
 */
static void network_attach_delay ( u8 ID )
{

	
  lgsm_network_query_signal_strength( NULL );
  /* 查询网络状态 */
  lgsm_network_get_status ( NULL );
  /* 执行gprs附着 */
  //lgsm_pdp_attach_gprs_network ( NULL ); 
}







extern void wip_NetInit ( ascii* );





/* 
 * 功能描述：每120秒查询一次GPRS网络状态
 * 引用参数：无
 *          
 * 返回值  ：无
 * 
 */
extern void network_get_current_status ( u8 type, NetworkStatus status )
{ 
  if ( GPRS_NETWORK_TYPE == type )
  {
    switch ( status )
    {
      case NETWORK_UNKNOWN:
#if SIM_NETWORK_DEBUG == 1
{
        PRINTF_GPS_TIME ();
        printf ( "[SIM EVENT] NETWORK_UNKNOWN\r\n" );
}
#endif  /* SIM_NETWORK_DEBUG */

      case NETWORK_UNREGISTERED:
      {
        tmr_t* myTmr = NULL;

#if SIM_NETWORK_DEBUG == 1
{
        PRINTF_GPS_TIME ();
        printf ( "[SIM EVENT] NETWORK_UNREGISTERED\r\n" );
}
#endif  /* SIM_NETWORK_DEBUG */

        networkStatus = FALSE;
//        gprsParams.net_is_bad++;
//        gprsParams.net_is_ok = 0;
        myTmr = TMR_Subscribe ( FALSE, 50, TMR_TYPE_100MS, ( tmr_procTriggerHdlr_t )network_attach_delay );
//				if ( myTmr == NULL )
//        {
//          lgsm_network_get_status ( NULL );
//        }	
      }
      break;

      case NETWORK_ROAMING:
#if SIM_NETWORK_DEBUG == 1
{
        PRINTF_GPS_TIME ();
        printf ( "[SIM EVENT] NETWORK_ROAMING\r\n" );
}
#endif  /* SIM_NETWORK_DEBUG */

      case NETWORK_REGISTERED:

#if SIM_NETWORK_DEBUG == 1
{
        PRINTF_GPS_TIME ();
        printf ( "[SIM EVENT] NETWORK_REGISTERED networkStatus=%d\r\n",networkStatus);
}
#endif  /* SIM_NETWORK_DEBUG */
        if ( networkStatus == FALSE )
        {  	   
//          wip_NetInit ( ( ascii* )gprsParams.APN );
					char lwip_apn[]="unused";
					wip_NetInit ( ( ascii* )lwip_apn );	
        }
        networkStatus = TRUE;
//        gprsParams.net_is_ok++;
        break;
  
      case NETWORK_BUSY:
      {
        tmr_t* myTmr = NULL;

#if SIM_NETWORK_DEBUG == 1
{
        PRINTF_GPS_TIME ();
        printf ( "[SIM EVENT] NETWORK_BUSY\r\n" );
}
#endif  /* SIM_NETWORK_DEBUG */

        //lgsm_network_get_status ( NULL );
        myTmr = TMR_Subscribe ( FALSE, 40, TMR_TYPE_100MS, ( tmr_procTriggerHdlr_t )network_attach_delay );
//        if ( myTmr == NULL )
//        {
//          lgsm_network_get_status ( NULL );
//        }	
      }
      break;

      case NETWORK_DENIED:

#if SIM_NETWORK_DEBUG == 1
{
        PRINTF_GPS_TIME ();
        printf ( "[SIM EVENT] NETWORK_DENIED\r\n" );
}
#endif  /* SIM_NETWORK_DEBUG */

        networkStatus = FALSE;
//        gprsParams.net_is_bad++;
//        gprsParams.net_is_ok = 0;
        break;
    }
  }
}




/* 
 * 功能描述：
 * 引用参数：无
 *          
 * 返回值  ：无
 * 
 */
extern void GSM_StatusInfoDisplay ( void )
{
//  printf ( "\r\nGSM sim %s, csq %u, netwrok %s\r\n", 
//	       ( STATUS_QueryObjectStatus ( STATUS_SIM_CARD_OK_BIT ) == TRUE ) ? "OK" : "KO", csq, ( networkStatus == TRUE ) ? "OK" : "KO" );
}


