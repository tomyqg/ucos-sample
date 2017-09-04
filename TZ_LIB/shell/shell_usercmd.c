#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "shell_usercmd.h"
#include "shell.h"
#include "tiza_params.h"
#include "gb_gprs_protocol.h"
#include "tiza_storage.h"
#include "tiza_init.h"
#include "tiza_upgrade.h"
#include "defines.h"
#include "GSM_xx.h"
#include "w25qxx_port.h"

//extern u8 Vid_CheckSum = 0;
extern ascii E_IMEI[16];


/******************************************/
/*              内部函数[声明]            */
/******************************************/

static void hdl_CmdRoutine_SYSSET       ( SHELL_CmdParserTypedef *paras );
static void hdl_CmdRoutine_SYSRESET     ( SHELL_CmdParserTypedef *paras );
static void hdl_CmdRoutine_SYSCONNECT   ( SHELL_CmdParserTypedef *paras );
static void hdl_CmdRoutine_SYSGPS       ( SHELL_CmdParserTypedef *paras );
static void hdl_CmdRoutine_SYSIO        ( SHELL_CmdParserTypedef *paras );
static void hdl_CmdRoutine_SYSADC       ( SHELL_CmdParserTypedef *paras );
static void hdl_CmdRoutine_SYSINFO      ( SHELL_CmdParserTypedef *paras );
static void hdl_CmdRoutine_SYSDEBUG     ( SHELL_CmdParserTypedef *paras );
static void hdl_CmdRoutine_SYSGSM       ( SHELL_CmdParserTypedef *paras );
static void hdl_CmdRoutine_SYSFTP       ( SHELL_CmdParserTypedef *paras );
static void hdl_CmdRoutine_SYSSMS       ( SHELL_CmdParserTypedef *paras );
static void hdl_CmdRoutine_SYSIP        ( SHELL_CmdParserTypedef *paras );
static void hdl_CmdRoutine_SYSCAN       ( SHELL_CmdParserTypedef *paras );
static void hdl_CmdRoutine_SYSRTC       ( SHELL_CmdParserTypedef *paras );
static void hdl_CmdRoutine_SYSVER       ( SHELL_CmdParserTypedef *paras );
static void hdl_CmdRoutine_SYSCELLPHONE ( SHELL_CmdParserTypedef *paras );
static void hdl_CmdRoutine_SYSSLEEP     ( SHELL_CmdParserTypedef *paras );
static void hdl_CmdRoutine_SYSUSART     ( SHELL_CmdParserTypedef *paras );
static void hdl_CmdRoutine_SYGBDEBUG ( SHELL_CmdParserTypedef *paras );


#if __USE_TCP_TEST__ == 1
static void hdl_CmdRoutine_SYSTCP       ( SHELL_CmdParserTypedef *paras );
#endif  /* __USE_TCP_TEST__ */

#if J1939_USART_DEBUG == 1
static void hdl_CmdRoutine_SYSJ1939     ( SHELL_CmdParserTypedef *paras );
#endif  /* J1939_USART_DEBUG */

#if ACC_WORK_STATS_DEBUG == 1
static void hdl_CmdRoutine_SYSACCSTATS  ( SHELL_CmdParserTypedef *paras );
#endif  /* ACC_WORK_STATS_DEBUG */ 













/* 
 * 功能描述：注册用户命令控制接口
 * 引用参数：无
 *          
 * 返回值  ：无
 * 
 */
extern void CMD_ApplicationIfInit ( void )
{
  SHELL_CmdSubscribe ( "TZ+SYSRESET",     ( shell_atCmdHandler_t )hdl_CmdRoutine_SYSRESET,      ( ADL_CMD_TYPE_TEST | ADL_CMD_TYPE_ACT  | ADL_CMD_TYPE_PARA ) );
  SHELL_CmdSubscribe ( "TZ+SYSSET",       ( shell_atCmdHandler_t )hdl_CmdRoutine_SYSSET,        ( ADL_CMD_TYPE_TEST | ADL_CMD_TYPE_READ | ADL_CMD_TYPE_PARA ) );
  SHELL_CmdSubscribe ( "TZ+SYSCONNECT",   ( shell_atCmdHandler_t )hdl_CmdRoutine_SYSCONNECT,    ( ADL_CMD_TYPE_TEST | ADL_CMD_TYPE_READ | ADL_CMD_TYPE_PARA ) );
  SHELL_CmdSubscribe ( "TZ+SYSGPS",       ( shell_atCmdHandler_t )hdl_CmdRoutine_SYSGPS,        ( ADL_CMD_TYPE_TEST | ADL_CMD_TYPE_READ | ADL_CMD_TYPE_PARA ) );
  SHELL_CmdSubscribe ( "TZ+SYSIO",        ( shell_atCmdHandler_t )hdl_CmdRoutine_SYSIO,         ( ADL_CMD_TYPE_TEST | ADL_CMD_TYPE_READ | ADL_CMD_TYPE_PARA ) );
  SHELL_CmdSubscribe ( "TZ+SYSADC",       ( shell_atCmdHandler_t )hdl_CmdRoutine_SYSADC,        ( ADL_CMD_TYPE_TEST | ADL_CMD_TYPE_READ | ADL_CMD_TYPE_PARA ) );
  SHELL_CmdSubscribe ( "TZ+SYSINFO",      ( shell_atCmdHandler_t )hdl_CmdRoutine_SYSINFO,       ( ADL_CMD_TYPE_TEST | ADL_CMD_TYPE_READ  | ADL_CMD_TYPE_PARA ) );
  SHELL_CmdSubscribe ( "TZ+SYSDEBUG",     ( shell_atCmdHandler_t )hdl_CmdRoutine_SYSDEBUG,      ( ADL_CMD_TYPE_TEST | ADL_CMD_TYPE_READ | ADL_CMD_TYPE_PARA ) );
  SHELL_CmdSubscribe ( "TZ+SYSGSM",       ( shell_atCmdHandler_t )hdl_CmdRoutine_SYSGSM,        ( ADL_CMD_TYPE_TEST | ADL_CMD_TYPE_READ | ADL_CMD_TYPE_PARA ) );
  SHELL_CmdSubscribe ( "TZ+SYSFTP",       ( shell_atCmdHandler_t )hdl_CmdRoutine_SYSFTP,        ( ADL_CMD_TYPE_TEST | ADL_CMD_TYPE_PARA ) );
  SHELL_CmdSubscribe ( "TZ+SYSSMS",       ( shell_atCmdHandler_t )hdl_CmdRoutine_SYSSMS,        ( ADL_CMD_TYPE_TEST | ADL_CMD_TYPE_PARA ) );
  SHELL_CmdSubscribe ( "TZ+SYSIP",        ( shell_atCmdHandler_t )hdl_CmdRoutine_SYSIP,         ( ADL_CMD_TYPE_TEST | ADL_CMD_TYPE_ACT  ) );
  SHELL_CmdSubscribe ( "TZ+SYSCAN",       ( shell_atCmdHandler_t )hdl_CmdRoutine_SYSCAN,        ( ADL_CMD_TYPE_TEST | ADL_CMD_TYPE_READ | ADL_CMD_TYPE_PARA ) );
  SHELL_CmdSubscribe ( "TZ+SYSRTC",       ( shell_atCmdHandler_t )hdl_CmdRoutine_SYSRTC,        ( ADL_CMD_TYPE_TEST | ADL_CMD_TYPE_READ | ADL_CMD_TYPE_PARA ) );
  SHELL_CmdSubscribe ( "TZ+SYSCELLPHONE", ( shell_atCmdHandler_t )hdl_CmdRoutine_SYSCELLPHONE,  ( ADL_CMD_TYPE_TEST | ADL_CMD_TYPE_READ | ADL_CMD_TYPE_PARA ) );
  SHELL_CmdSubscribe ( "TZ+SYSVER",       ( shell_atCmdHandler_t )hdl_CmdRoutine_SYSVER,        ( ADL_CMD_TYPE_TEST | ADL_CMD_TYPE_READ | ADL_CMD_TYPE_ACT ) );
  SHELL_CmdSubscribe ( "TZ+SYSSLEEP",     ( shell_atCmdHandler_t )hdl_CmdRoutine_SYSSLEEP,      ( ADL_CMD_TYPE_TEST | ADL_CMD_TYPE_READ | ADL_CMD_TYPE_PARA ) );
  SHELL_CmdSubscribe ( "TZ+SYSUSART",     ( shell_atCmdHandler_t )hdl_CmdRoutine_SYSUSART,      ( ADL_CMD_TYPE_TEST | ADL_CMD_TYPE_READ | ADL_CMD_TYPE_PARA | ADL_CMD_TYPE_ACT ) );
  SHELL_CmdSubscribe ( "TZ+GBDEBUG",      ( shell_atCmdHandler_t )hdl_CmdRoutine_SYGBDEBUG,       ( ADL_CMD_TYPE_TEST | ADL_CMD_TYPE_PARA ) );
#if __USE_TCP_TEST__ == 1  
  SHELL_CmdSubscribe ( "TZ+SYSTCP",       ( shell_atCmdHandler_t )hdl_CmdRoutine_SYSTCP,        ( ADL_CMD_TYPE_TEST | ADL_CMD_TYPE_ACT  | ADL_CMD_TYPE_PARA ) );
#endif  /* __USE_TCP_TEST__ */


#if J1939_USART_DEBUG == 1
  SHELL_CmdSubscribe ( "TZ+SYSJ1939",     ( shell_atCmdHandler_t )hdl_CmdRoutine_SYSJ1939,      ( ADL_CMD_TYPE_TEST | ADL_CMD_TYPE_PARA ) );
#endif  /* J1939_USART_DEBUG */


#if ACC_WORK_STATS_DEBUG == 1
  SHELL_CmdSubscribe ( "TZ+SYSACCSTATS",  ( shell_atCmdHandler_t )hdl_CmdRoutine_SYSACCSTATS,   ( ADL_CMD_TYPE_TEST | ADL_CMD_TYPE_READ | ADL_CMD_TYPE_PARA ) );
#endif  /* ACC_WORK_STATS_DEBUG */ 



 

}


/* 
 * 功能描述：国标调试
 * 引用参数：
 *          
 * 返回值  ：
 * 
 */
static void hdl_CmdRoutine_SYGBDEBUG ( SHELL_CmdParserTypedef *paras )
{
      switch ( paras->type )
      {
        case ADL_CMD_TYPE_TEST:
					printf ( "\r\n+SYSCOM4:[<fun>]\r\n" );
          printf ( "\r\nOK\r\n" );
          break;
       
        case ADL_CMD_TYPE_PARA:
        {
					ascii *param0 = NULL, *param1 = NULL, *param2 = NULL;
					printf ( "\r\n+SYSSET:\r\n" );
					param0 = ADL_GET_PARAM( paras->rsp, 0 );
					if ( param0 )
          {
						
							if(
								0==strncmp((char*)param0,"login",6)||
								0==strncmp((char*)param0,"LOGOIN",6)	
							)
							{
									printf("\r\nTrigger Login\r\n");
									Modem_Stop(PROC_SYSTEM_INIT);
									TMR_Subscribe ( FALSE, 30, TMR_TYPE_100MS, ( tmr_procTriggerHdlr_t )System_Reset );

									PRINTF_GPS_TIME();
									return;
							}
							else
							if(
								0==strncmp((char*)param0,"logout",6)||
								0==strncmp((char*)param0,"LOGOUT",6)	
							)
							{
									printf("\r\nTrigger Loginout\r\n");
									sys_status=PROC_SYSTEM_LOGOUT;	
									logout_flag=1;
									PRINTF_GPS_TIME();
									return;
							}
							else
							if(
								0==strncmp((char*)param0,"alarm",5)||
								0==strncmp((char*)param0,"ALARM",5)	
							)
							{
									printf("\r\n Trigger 3 Level Alarm \r\n");
									upload_switch_state=1;	
									PRINTF_GPS_TIME();
									return;
							}
							else
							if(
								0==strncmp((char*)param0,"clearalarm",10)||
								0==strncmp((char*)param0,"CLEARALARM",10)	
							)
							{
									printf("\r\n clearalarm  ok \r\n");
									alarm_alive=0;
									PRINTF_GPS_TIME();
									return;
							
							}							
							else
							if(
								0==strncmp((char*)param0,"RESET",5)||
								0==strncmp((char*)param0,"reset",5)	
							)
							{
									printf ( "system will reset......\r\n" );
									printf ( "\r\nOK\r\n" );
									PRINTF_GPS_TIME();
									System_Reset ();	
									return;
							}
							else
							if(
								0==strncmp((char*)param0,"DISCONNECT",10)||
								0==strncmp((char*)param0,"disconnect",10)	
							)
							{
									Modem_Stop(PROC_SYSTEM_REISSUEDATA_STORAGE);						
									printf("\r\n 	disconnect tcp client......\r\n");
									PRINTF_GPS_TIME();
									return;
							}
							else
							if(
								0==strncmp((char*)param0,"RECONNECT",9)||
								0==strncmp((char*)param0,"reconnect",9)	
							)
							{
									Modem_Restart(PROC_SYSTEM_INIT);
									printf("\r\n 	reconnect service......\r\n");
									PRINTF_GPS_TIME();
									return;
							}
							printf ( "\r\nerror params\r\n\r\nERROR \r\n" );
					}
				}
			}
}

/* 
 * 功能描述：SIM卡号整数转字符串
 * 引用参数：
 *          
 * 返回值  ：
 * 
 */
static void SimCardToStr ( unsigned long long longInt, ascii *str )    
{
  unsigned long long remainVal = longInt;

  str[ 0 ] = (remainVal / 10000000000LL) + 0x30;

  remainVal = remainVal - (remainVal / 10000000000LL)*10000000000LL;
  str[ 1 ] = (remainVal / 1000000000LL) + 0x30;    

  remainVal = remainVal - (remainVal / 1000000000LL)*1000000000LL;
  str[ 2 ] = (remainVal / 100000000LL) + 0x30;
  remainVal = remainVal - (remainVal / 100000000LL)*100000000LL;
  str[ 3 ] = (remainVal / 10000000LL) + 0x30;
  remainVal = remainVal - (remainVal / 10000000LL)*10000000LL;
  str[ 4 ] = (remainVal / 1000000LL) + 0x30;
  remainVal = remainVal - (remainVal / 1000000LL)*1000000LL;
  str[ 5 ] = (remainVal / 100000LL) + 0x30;
  remainVal = remainVal - (remainVal / 100000LL)*100000LL;
  str[ 6 ] = (remainVal / 10000LL) + 0x30;
  remainVal = remainVal - (remainVal / 10000LL)*10000LL;
  str[ 7 ] = (remainVal / 1000LL) + 0x30;
  remainVal = remainVal - (remainVal / 1000LL)*1000;
  str[ 8 ] = (remainVal / 100LL) + 0x30;
  remainVal = remainVal - (remainVal / 100LL)*100LL;
  str[ 9 ] = (remainVal / 10LL) + 0x30;
  remainVal = remainVal - (remainVal / 10LL)*10LL;
  str[ 10] = (remainVal % 10LL) + 0x30;  
}


extern u8 Vid_DataCheck ( u8 *data, u16 len )
{
  if ( data && len > 0 )
  {
    u32 i = 0;
    u8 dataChk = 0;

    for ( i = 0; i < len; i++ )
    {
      dataChk += data[ i ]; 
    }

    return dataChk;
  }
  return 0;
}


/* 
 * 功能描述：系统参数设置
 * 引用参数：
 *          
 * 返回值  ：
 * 
 */
static void hdl_CmdRoutine_SYSSET ( SHELL_CmdParserTypedef *paras )
{
	int i=0;
  switch ( paras->type )
  {
    case ADL_CMD_TYPE_TEST:
      printf ( "\r\n+SYSSET:\r\n" );
      printf ( "IP1,\"XXX.XXX.XXX.XXX\",PORT\r\n" );
      printf ( "IP2,\"XXX.XXX.XXX.XXX\",PORT\r\n" );
      printf ( "APN,\"apn string\"\r\n" );
      printf ( "VID,\"vehicle-ID string\"\r\n" );
      printf ( "\r\nOK\r\n" );
      break;

    case ADL_CMD_TYPE_READ:
    {
			printf ( "\r\n+SYSSET:\r\n" );
      printf ( "TCP连接状态：\r\n");	
      /* 读取IP1 */
      printf ( "IP1,%s,%d\r\n", gprsParams.IP1, gprsParams.Port1 );

      /* 读取IP2 */
      printf ( "IP2,%s,%d\r\n", gprsParams.IP2, gprsParams.Port2 );
      
      /* 读取APN */
      printf ( "APN,%s\r\n", gprsParams.APN );
			
			/* 读取APN */
      printf ( "LOGIN_SN: %d\r\n", vehicleParams.LoginSerial);
			

      /* 读取VID */
			if ( gprsParams.devID[ 0 ] =='T'||gprsParams.devID[ 0 ] =='A' )
      {
					printf ("VID: %s \r\n", gprsParams.devID);
      }
      else  /*掉电后从flash 中读取vid*/
      {
            STORAGE_ReadData( GPRS_PARAMS, sizeof( gprs_Params_t ), ( u8* )&gprsParams );
						/* 读取VID */
						if ( gprsParams.devID[ 0 ] =='T'||gprsParams.devID[ 0 ] =='A')
						{
								printf ("VID: %s\r\n", gprsParams.devID);	
						}
						else
						{
								printf(" VID number is wrong. It should be set first\r\n");
						}      
			}
			/* 读取VIN */
			if ( vehicleParams.CarVIN[ 0 ] =='L'||vehicleParams.CarVIN[ 0 ] =='A' )
			{
					printf ("VIN: %s \r\n", vehicleParams.CarVIN);
					printf ( "\r\nOK\r\n" );      
			}
			else  /*掉电后从flash 中读取vid*/
			{
						STORAGE_ReadData( VEHICLE_PARAMS, sizeof( vehicleParams ), ( u8* )&vehicleParams );
						/* 读取VID */
						if ( vehicleParams.CarVIN[ 0 ] =='L'||vehicleParams.CarVIN[ 0 ] =='A' )
						{
								printf ("VIN: %s\r\n", vehicleParams.CarVIN);
								printf ( "\r\nOK\r\n" );      
						}
						else
						{
								printf(" VIN number is wrong. It should be set first\r\n");
						}      
			}
      break;
	}
		
    case ADL_CMD_TYPE_PARA:
    {
      
			ascii *param0 = NULL, *param1 = NULL, *param2 = NULL;
      printf ( "\r\n+SYSSET:\r\n" );
      param0 = ADL_GET_PARAM( paras->rsp, 0 );
      if ( param0 )
      {
          /* 设置IP1 */
          if ( 
               ( param0[ 0 ] == 'I' || param0[ 0 ] == 'i' ) && 
               ( param0[ 1 ] == 'P' || param0[ 1 ] == 'p' ) && param0[ 2 ] == '1' )
          {
            param1 = ADL_GET_PARAM( paras->rsp, 1 );
            if ( param1 )
            {
								snprintf ( ( char * )gprsParams.IP1, 16, "%s", param1 );
								param2 = ADL_GET_PARAM( paras->rsp, 2 );
								if ( param2 )
								{
									gprsParams.Port1 = ( u16 )DecToInt ( param2, strlen( ( const char * )param2 ) );
									
									/* 保存参数 */
									STORAGE_WriteData ( GPRS_PARAMS, sizeof( gprs_Params_t ), ( u8* )&gprsParams );
									STORAGE_FlashParams_WriteData(( u8* )&gprsParams ,sizeof( gprs_Params_t ),FLASH_GPRS_PARAMS);
									printf ( "IP1 %s:%d\r\nOK\r\n", gprsParams.IP1, gprsParams.Port1 );
									return ;	            	
								}
            }
          }
          
          /* 设置IP2 */
          if ( 
               ( param0[ 0 ] == 'I' || param0[ 0 ] == 'i' ) && 
               ( param0[ 1 ] == 'P' || param0[ 1 ] == 'p' ) && param0[ 2 ] == '2' )
          {
            param1 = ADL_GET_PARAM( paras->rsp, 1 );
            if ( param1 )
            {
								snprintf ( ( char * )gprsParams.IP2, 16, "%s", param1 );
								param2 = ADL_GET_PARAM( paras->rsp, 2 );
								if ( param2 )
								{
									gprsParams.Port2 = ( u16 )DecToInt ( param2, strlen( ( const char * )param2 ) );
									
									/* 保存参数 */
									STORAGE_WriteData ( GPRS_PARAMS, sizeof( gprs_Params_t ), ( u8* )&gprsParams );
									STORAGE_FlashParams_WriteData(( u8* )&gprsParams ,sizeof( gprs_Params_t ),FLASH_GPRS_PARAMS);
									printf ( "IP2 %s:%d\r\nOK\r\n", gprsParams.IP2, gprsParams.Port2 );
									return ;	            	
								}
            }
          }

          /* 设置APN */
          if (
               ( param0[ 0 ] == 'A' || param0[ 0 ] == 'a' ) && 
               ( param0[ 1 ] == 'P' || param0[ 1 ] == 'p' ) && 
               ( param0[ 2 ] == 'N' || param0[ 2 ] == 'n' )
              )
          {
            param1 = ADL_GET_PARAM( paras->rsp, 1 );
            if ( param1 )
            {
								snprintf ( ( char * )gprsParams.APN, 32, "%s", param1 );
		
								/* 保存参数 */
								STORAGE_WriteData ( GPRS_PARAMS, sizeof( gprs_Params_t ), ( u8* )&gprsParams );
								printf ( "APN %s\r\nOK\r\n", gprsParams.APN );
								return ;            	
            }
          }
					/* 设置登录流水号 */
          if (
								0==strncmp((char*)param0,"login_sn",8)||
								0==strncmp((char*)param0,"LOGIN_SN",8)	
             )
          {
            param1 = ADL_GET_PARAM( paras->rsp, 1 );
            if ( param1 )
            {
								vehicleParams.LoginSerial=stringToInt(param1,strlen(param1));
		
								/* 保存参数 */
								STORAGE_WriteData ( VEHICLE_PARAMS, sizeof( vechile_Params_t ), ( u8* )&vehicleParams );
								printf ( "LOGIN_SN :%d\r\nOK\r\n", vehicleParams.LoginSerial );
								return ;            	
            }
          }

          /* 设置VID */
          if (
               ( param0[ 0 ] == 'V' || param0[ 0 ] == 'v' ) && 
               ( param0[ 1 ] == 'I' || param0[ 1 ] == 'i' ) && 
               ( param0[ 2 ] == 'D' || param0[ 2 ] == 'd' )
              )
          {
		  	
						printf ( "%s\r\n", ADL_GET_PARAM( paras->rsp, 0 ) );
            param1 = ADL_GET_PARAM( paras->rsp, 1 );

            /* 与手提电话号码长度比较 */
            if ( param1 && strlen ( ( const char* )param1 ) == 17||param1 && strlen ( ( const char* )param1 ) == 11 )
            {
            	unsigned long long simCard = 0;
							printf ( "VID %s\r\n", param1 );
							printf ( "\r\nOK\r\n" );

#if 1
//      	 	for(i=0;i<17;i++)
//				 	{
//          	gprsParams.devID[ i ] = param1[i];
//						//printf ("%02x\t", param1[i] );
//				 	}
#endif							
							memcpy(gprsParams.devID,param1,17);
							gprsParams.DeviceIDSet = TRUE;

	            /* 保存参数 */
	            STORAGE_WriteData ( GPRS_PARAMS, sizeof( gprs_Params_t ), ( u8* )&gprsParams );    
	            return ;
            }
          }

					/* 设置VIN */
          if (
               ( param0[ 0 ] == 'V' || param0[ 0 ] == 'v' ) && 
               ( param0[ 1 ] == 'I' || param0[ 1 ] == 'i' ) && 
               ( param0[ 2 ] == 'N' || param0[ 2 ] == 'n' )
              )
          {
		  	
							printf ( "%s\r\n", ADL_GET_PARAM( paras->rsp, 0 ) );
							param1 = ADL_GET_PARAM( paras->rsp, 1 );

							/* VIN长度比较 */
							if ( param1 && strlen ( ( const char* )param1 ) == 17 )
							{
								
								unsigned long long simCard = 0;
								printf ( "VIN %s\r\n", param1 );
		
								memcpy(vehicleParams.CarVIN,param1,17);

								vehicleParams.CarVINSet = TRUE;
								
								printf ( "\r\nOK\r\n" );  

								/* 保存参数 CarVIN*/   
								STORAGE_WriteData ( VEHICLE_PARAMS, sizeof( vechile_Params_t ), ( u8* )&vehicleParams );
								STORAGE_FlashParams_WriteData(( u8* )&vehicleParams ,sizeof( vechile_Params_t ),FLASH_VEHICLE_PARAMS);
								return;
							}
							else
							{
								 printf ( "\r\n VIN 写入失败，请输入17位VIN  \r\n" );
							}
          }
      }
      printf ( "Error params\r\n" );
      printf ( "\r\nERROR\r\n" ); 
      break;
    }
  }
}








/* 
 * 功能描述：应用延时复位
 * 引用参数：
 *          
 * 返回值  ：
 * 
 */
extern void System_ResetDly ( u8 ID )
{
  System_Reset ();
}




/* 
 * 功能描述：系统复位
 * 引用参数：
 *          
 * 返回值  ：
 * 
 */
static void hdl_CmdRoutine_SYSRESET ( SHELL_CmdParserTypedef *paras )
{
  switch( paras->type )
  {
    case ADL_CMD_TYPE_TEST:
      printf ( "\r\n+SYSRESET:[<fun>[,<dly-time>]]\r\n" );
      printf ( "\r\nOK\r\n" );
      break;

    case ADL_CMD_TYPE_ACT:
      printf ( "\r\nOK\r\n" );

#if SYSTEM_RESTART_DEBUG == 1
      printf ( "\r\n[SYS RESET] usr-active type\r\n" );
#endif  /* SYSTEM_RESTART_DEBUG */
	  //vehicles_logout();
      //System_Reset ();
      break;

    case ADL_CMD_TYPE_PARA:
    {
      ascii *param0 = NULL, *param1 = NULL;
      printf ( "\r\n+SYSRESET:\r\n" );
      param0 = ADL_GET_PARAM( paras->rsp, 0 );



//	  if(param0[0]=='D') //触发3级报警
//	  {		  	
//			if(param0[1]=='D')
//			{
//				//日期不一致：流水号清零 ,更新登入时间
//				vehicleParams.LoginSerial=(param0[2]-0x30);
//				udpPositon.date_yy=RMC_d.date_yy;
//				udpPositon.date_mm=RMC_d.date_mm;
//				udpPositon.date_dd=RMC_d.date_dd;
//				
//			}
//			else if(param0[1]=='W')
//			{
//				vehicleParams.LoginSerial=0;
//				udpPositon.date_yy=1;
//				udpPositon.date_mm=1;
//				udpPositon.date_dd=1;			
//			}
//			/*更新车辆登录时间*/
//			
//			STORAGE_WriteData ( VEHICLE_PARAMS, sizeof( vechile_Params_t ), ( u8* )&vehicleParams );
//			/*更新车辆登录时间*/
//			STORAGE_WriteData ( LAST_POSITION,   sizeof( udp_Position_t ),   ( u8 * )&udpPositon );
//			printf ( "\r\n^^^^^^^^^车辆登入流水： %d        \r\n",vehicleParams.LoginSerial);
//			printf ( "\r\n^^^^^^^^^GPRS    时间： 20%u/%u/%u\r\n",RMC_d.date_yy, RMC_d.date_mm, RMC_d.date_dd);
//			printf ( "\r\n^^^^^^^^^flash   时间： 20%u/%u/%u\r\n",udpPositon.date_yy, udpPositon.date_mm, udpPositon.date_dd);		
//	  }
//	  else
//	  if(param0[0]=='R') //
//	  {
//	    printf ( "\r\n^^^^^^^^^车辆登入流水： %d        \r\n",vehicleParams.LoginSerial);
//			printf ( "\r\n^^^^^^^^^GPRS    时间： 20%u/%u/%u\r\n",RMC_d.date_yy, RMC_d.date_mm, RMC_d.date_dd);
//			printf ( "\r\n^^^^^^^^^flash   时间： 20%u/%u/%u\r\n",udpPositon.date_yy, udpPositon.date_mm, udpPositon.date_dd);
//	  }

      break;
    }
  }
}







/* 
 * 功能描述：系统应用连接
 * 引用参数：
 *          
 * 返回值  ：
 * 
 */
static void hdl_CmdRoutine_SYSCONNECT ( SHELL_CmdParserTypedef *paras )
{           
  switch ( paras->type )
  {
    case ADL_CMD_TYPE_TEST:
      printf ( "\r\n+SYSCONNECT:[<fun>[,<dly-time>]]\r\n" );
      printf ( "\r\nOK\r\n" );
      break;

    case ADL_CMD_TYPE_READ:
      printf ( "\r\n+SYSCONNECT:\r\n" );

      if ( sys_status < PROC_SYSTEM_LOGIN )
      {
        printf ( "system init...\r\n" );
      }
      else
      if ( sys_status == PROC_SYSTEM_LOGIN )
      {
        printf ( "system dial gprs, try to login %s %u\r\n", mylink_address, mylink_port );
      }
      else
      if ( sys_status > PROC_SYSTEM_LOGIN )
      {
        printf ( "system is connected with center %s %u\r\n", mylink_address, mylink_port );
      }
      printf ( "\r\nOK\r\n" );
      break;

    case ADL_CMD_TYPE_PARA:
    {
      u8 function = 0;
      ascii *param0 = NULL, *param1 = NULL;

      printf ( "\r\n+SYSCONNECT:\r\n" );
      param0 = ADL_GET_PARAM( paras->rsp, 0 );
			if ( param0 )
      {



//        function = DecToInt ( param0, strlen( ( const char * )param0 ) );
//        if ( function == 0 )
//        {
//					PRINTF_GPS_TIME();
//          return ;      
//        }
//        else
//        if ( function == 1 )
//        {
//          vehicleParams.DebugCtroller.debugType |= Debug_Type_MDM;
//          STORAGE_WriteData ( VEHICLE_PARAMS, sizeof( vechile_Params_t ), ( u8 * )&vehicleParams );
//        	printf ( "set modem debug on\r\n\r\nOK\r\n" );
//          return ;     
//        }
//        else
//        if ( function == 2 )
//        {
//          u32 dlyTime = 0;  
//          param1 = ADL_GET_PARAM( paras->rsp, 1 );
//          if ( param1 )
//          {
//  	        dlyTime = DecToInt ( param1, strlen( ( const char * )param1 ) );

//  	        if ( dlyTime > 0 && dlyTime <= 86400 )
//  	        {
//  	          xmit_start_tmr = TMR_Subscribe ( FALSE, ( 10 * dlyTime ), TMR_TYPE_100MS, ( tmr_procTriggerHdlr_t )XMIT_Startup );
//              if ( xmit_start_tmr > 0 )
//  	          {
//  	            printf ( "system will delay %us to reconnect\r\n\r\nOK\r\n", dlyTime );
//  	            return ;
//  	          }
//  	        }
//          }    
//        }
//        else
//        if ( function == 3 )
//        {
//          MDM_SwitchInfoStatistics ();
//          printf ( "\r\nOK\r\n" );
//          return ;
//        } 
      }
      printf ( "error params\r\n" );
      break;
    }
  }
}








/* 
 * 功能描述：GPS配置
 * 引用参数：
 *          
 * 返回值  ：
 * 
 */
static void hdl_CmdRoutine_SYSGPS ( SHELL_CmdParserTypedef *paras )
{      
  switch( paras->type )
  {
    case ADL_CMD_TYPE_TEST:
      printf ( "\r\n+SYSGPS:[<fun>]\r\n" );
      printf ( "\r\nOK\r\n" );
      break;

    case ADL_CMD_TYPE_READ:
    	printf ( "\r\n+SYSGPS:\r\n" );
    	Printf_TimeInfoDisplay ();
    	printf ( "\r\nOK\r\n" );
      break;

    case ADL_CMD_TYPE_PARA:
    {	
		  u8 status = 0;
      ascii *param0 = NULL;

      param0 = ADL_GET_PARAM( paras->rsp, 0 );
//      if ( param0 )
//      {
//        status = DecToInt ( param0, strlen( ( const char * )param0 ) );
//        if ( status == 0 )
//        {
//          vehicleParams.DebugCtroller.debugType &= ( ~Debug_Type_GPS );
//        	STORAGE_WriteData ( VEHICLE_PARAMS, sizeof( vechile_Params_t ), ( u8 * )&vehicleParams );
//        	printf ( "\r\n+SYSGPS: 0\r\nset gps debug off\r\n" );
//        	printf ( "\r\nOK\r\n" );
//        	return ;
//        }
//        else
//        if ( status == 1 )
//        {
//          vehicleParams.DebugCtroller.debugType |= Debug_Type_GPS;
//        	STORAGE_WriteData ( VEHICLE_PARAMS, sizeof( vechile_Params_t ), ( u8 * )&vehicleParams );
//        	printf ( "\r\n+SYSGPS: 1\r\nset gps debug on\r\n" );
//        	printf ( "\r\nOK\r\n" );
//        	return ;
//        }
//        else
//        if ( status == 2 )
//        {
//        	printf ( "\r\n+SYSGPS: 2\r\nview gps status\r\n" );
//        	GPS_StatusInfoDisplay ();
//        	printf ( "\r\nOK\r\n" );
//        	return ;
//        }
//      }
    	printf ( "\r\n+SYSGPS: error params\r\n" );
    	printf ( "\r\nERROR\r\n" );
      break;
    }
  }
}






/* 
 * 功能描述：IO配置
 * 引用参数：
 *          
 * 返回值  ：
 * 
 */
static void hdl_CmdRoutine_SYSIO ( SHELL_CmdParserTypedef *paras )
{ 
  switch( paras->type )
  {
    case ADL_CMD_TYPE_TEST:
      printf ( "\r\n+SYSIO:[<fun>,[<pin>[,<IO-level>]]]\r\n" );
      printf ( "\r\nOK\r\n" );
      break;

    case ADL_CMD_TYPE_READ:
//        printf ( "\r\n+SYSIO:\r\n" );
//        printf ( "GPS io %u\r\n",       ( GPIO_ReadPINlevel ( gps_io_handler ) > 0 ) ? 1 : 0 );
//        printf ( "GSM io %u\r\n",       ( GPIO_ReadPINlevel ( gsm_io_handler ) > 0 ) ? 1 : 0 );
//#if TZ_DEFAULT_CONTROLLER_TYPE == TZ_CONTROLLER_TYPE_CAN
////	    printf ( "CAN io %u\r\n", 		( GPIO_ReadPINlevel ( can_io_handler ) > 0 ) ? 1 : 0 );
//#endif  /* TZ_DEFAULT_CONTROLLER_TYPE */
//        printf ( "ACC io %u\r\n",       ( GPIO_ReadPINlevel ( acc_io_handler ) > 0 ) ? 1 : 0 );
//        printf ( "LOCK0 io %u\r\n",     ( GPIO_ReadPINlevel ( lk0_io_handler ) > 0 ) ? 1 : 0 );
//	printf ( "SHELL io %u\r\n", 	( GPIO_ReadPINlevel ( shell_io_handler ) > 0 ) ? 1 : 0 );
//	printf ( "ANTENNA io %u\r\n",   ( GPIO_ReadPINlevel ( antenna_io_handler ) > 0 ) ? 1 : 0 );
//        printf ( "LTC3824 io %u\r\n", ( GPIO_ReadPINlevel ( ltc5v_io_handler ) > 0 ) ? 1 : 0 );
//        //printf ( "Low Power Check io %u\r\n", 	( GPIO_ReadPINlevel ( lower_power_check_handler ) > 0 ) ? 1 : 0 );
//        printf ( "\r\nOK\r\n" );        
      break;

    case ADL_CMD_TYPE_PARA:
    {   
//        u8 function = 0;
//	    ascii *param0 = NULL, *param1 = NULL, *param2 = NULL;

//	    printf ( "\r\n+SYSIO:\r\n" );
//	    param0 = ADL_GET_PARAM( paras->rsp, 0 );
//      if ( param0 )
//      {
//        function = DecToInt ( param0, strlen( ( const char * )param0 ) );
//        if ( function == 0 )
//        {
//          param1 = ADL_GET_PARAM( paras->rsp, 1 );
//          if ( param1 )
//          {
//            if ( strcmp ( ( const char* )param1, "GPS" ) == 0 )
//                    {
//                        printf ( "GPS io %u\r\n", ( GPIO_ReadPINlevel ( gps_io_handler ) > 0 ) ? 1 : 0 );
//			printf ( "\r\nOK\r\n" );
//                        return ;
//                    }
//            else
//            if ( strcmp ( ( const char* )param1, "GSM" ) == 0 )
//                    {
//                        printf ( "GSM io %u\r\n",       ( GPIO_ReadPINlevel ( gsm_io_handler ) > 0 ) ? 1 : 0 );
//			printf ( "\r\nOK\r\n" );
//                        return ;
//                    }
//#if TZ_DEFAULT_CONTROLLER_TYPE == TZ_CONTROLLER_TYPE_CAN
//  	        else
//  	        if ( strcmp ( ( const char* )param1, "CAN" ) == 0 )
//  					{
//  						printf ( "CAN io %u\r\n", 		( GPIO_ReadPINlevel ( can_io_handler ) > 0 ) ? 1 : 0 );
//                                                printf ( "\r\nOK\r\n" );
//                                                return ;
//  					}
//#endif  /* TZ_DEFAULT_CONTROLLER_TYPE */
//            else
//            if ( strcmp ( ( const char* )param1, "ACC" ) == 0 )
//                    {
//                        printf ( "ACC io %u\r\n",       ( GPIO_ReadPINlevel ( acc_io_handler ) > 0 ) ? 1 : 0 );
//                        printf ( "\r\nOK\r\n" );
//                        return ;
//                    }
//            else
//            if ( strcmp ( ( const char* )param1, "LOCK0" ) == 0 )
//                    {
//                        printf ( "LOCK0 io %u\r\n",     ( GPIO_ReadPINlevel ( lk0_io_handler ) > 0 ) ? 1 : 0 );
//                        printf ( "\r\nOK\r\n" );
//                        return ;
//                    }
//			else
//  	        if ( strcmp ( ( const char* )param1, "SHELL" ) == 0 )
//  					{
//  						printf ( "SHELL io %u\r\n", 	( GPIO_ReadPINlevel ( shell_io_handler ) > 0 ) ? 1 : 0 );
//						printf ( "\r\nOK\r\n" );
//                                                return ;
//  					}
//			else
//  	        if ( strcmp ( ( const char* )param1, "ANTENNA" ) == 0 )
//  					{
//  						printf ( "ANTENNA io %u\r\n", ( GPIO_ReadPINlevel ( antenna_io_handler ) > 0 ) ? 1 : 0 );
//						printf ( "\r\nOK\r\n" );
//                                                return ;
//  					}
//            else
//	        if ( strcmp ( ( const char* )param1, "LTC3824" ) == 0 )
//	                    {
//	                        printf ( "LTC3824 io %u\r\n", ( GPIO_ReadPINlevel ( ltc5v_io_handler ) > 0 ) ? 1 : 0 );
//				printf ( "\r\nOK\r\n" );
//                                return ;
//	                    }
//			else
//	            {
//	                printf ( "error params\r\n" );
//	                printf ( "\r\nERROR\r\n" ); 
//	                return ;            
//	            }
//                    
//          }        
//        }
//        else
//        if ( function == 1 )
//        {
//          param1 = ADL_GET_PARAM( paras->rsp, 1 );
//          if ( param1 )
//          {
//            u8 type = 0;
//            
//            if ( strcmp ( ( const char* )param1, "GPS" ) == 0 )
//                    {
//                        type = 1;
//                    }
//            else
//            if ( strcmp ( ( const char* )param1, "GSM" ) == 0 )
//                    {
//                        type = 2;
//                    }
//#if TZ_DEFAULT_CONTROLLER_TYPE == TZ_CONTROLLER_TYPE_CAN
//            else
//            if ( strcmp ( ( const char* )param1, "CAN" ) == 0 )
//                    {
//                        type = 3;
//                    }
//#endif  /* TZ_DEFAULT_CONTROLLER_TYPE */
//            else
//            if ( strcmp ( ( const char* )param1, "LOCK0" ) == 0 )
//                    {
//                        type = 4;
//                    }
//            else
//            if ( strcmp ( ( const char* )param1, "LOCK1" ) == 0 )
//                    {
//                        type = 5;
//                    }
//            else
//            if ( strcmp ( ( const char* )param1, "LOCK2" ) == 0 )
//                    {
//                        type = 6;
//                    }

//                    param2 = ADL_GET_PARAM( paras->rsp, 2 );
//                    if ( param2 && type > 0 )
//                    {
//                        if ( *param2 == '0' || *param2 == '1' )
//                        {
//                            switch ( type )
//                            {
//                                case 1:
//                    if ( gps_io_handler > 0 )
//                    {
//                      GPIO_WritePINlevel ( gps_io_handler, ( *param2 == '0' ) ? IO_RESET : IO_SET );
//                      printf ( "GPS io %c\r\n", *param2 );
//                      printf ( "\r\nOK\r\n" );
//                      return ;
//                    }
//                                    break;  
//  
//                                case 2:
//                    if ( gsm_io_handler > 0 )
//                    {
//                      GPIO_WritePINlevel ( gsm_io_handler, ( *param2 == '0' ) ? IO_RESET : IO_SET );
//                      printf ( "GSM io %c\r\n", *param2 );
//                      printf ( "\r\nOK\r\n" );
//                      return ;
//                    }
//                                    break;
//#if TZ_DEFAULT_CONTROLLER_TYPE == TZ_CONTROLLER_TYPE_CAN
//                                case 3:
//                    if ( can_io_handler > 0 )
//                    {
//                      GPIO_WritePINlevel ( can_io_handler, ( *param2 == '0' ) ? IO_RESET : IO_SET );
//                      printf ( "CAN io %c\r\n", *param2 );
//                      printf ( "\r\nOK\r\n" );
//                      return ;
//                    }
//                                    break;
//#endif  /* TZ_DEFAULT_CONTROLLER_TYPE */    
//  
//                                case 4:
//                    if ( lk0_io_handler > 0 )
//                    {
//                      GPIO_WritePINlevel ( lk0_io_handler, ( *param2 == '0' ) ? IO_SET : IO_RESET );
//                      printf ( "LOCK0 io %c\r\n", *param2 );
//                      printf ( "\r\nOK\r\n" );
//                      return ;
//                    }
//                                    break;  
//                                                                                                                                                                                                                    
//                            }
//                        }                       
//                    }
//          }        
//        }
//      }
//        printf ( "error params\r\n" );
//        printf ( "\r\nERROR\r\n" );       
      break;
    }
  }
}







/* 
 * 功能描述：ADC配置
 * 引用参数：
 *          
 * 返回值  ：
 * 
 */
static void hdl_CmdRoutine_SYSADC ( SHELL_CmdParserTypedef *paras )
{    
  switch( paras->type )
  {
    case ADL_CMD_TYPE_TEST:
      printf ( "\r\n+SYSADC:[<low-power-volt mv>]\r\n" );
      printf ( "\r\nOK\r\n" );
      break;

    case ADL_CMD_TYPE_READ:
    	printf ( "\r\n+SYSADC:\r\n" );
//			ADC_StatusInfoDisplay ();
			printf ( "\r\nOK\r\n" );
      break;

    case ADL_CMD_TYPE_PARA:
    {
    	ascii *param0 = NULL;
    	u32 low_power_volt = 0;

      printf ( "\r\n+SYSADC:\r\n" );
      param0 = ADL_GET_PARAM( paras->rsp, 0 );
      if ( param0 )
      {
	      low_power_volt = DecToInt ( param0, strlen( ( const char * )param0 ) );
	      if ( low_power_volt > 0 )
	      {
//	        vehicleParams.VA = low_power_volt / 100.0;
//	        STORAGE_WriteData ( VEHICLE_PARAMS, sizeof( vechile_Params_t ), ( u8* )&vehicleParams ); 
//	        printf ( "%u mv\r\n", low_power_volt ); 
//	        printf ( "\r\nOK\r\n" );
	        return ;
	      }      	
      }
    	printf ( "error params\r\n" );
    	printf ( "\r\nERROR\r\n" );
      break;
    }
  }
}





////////////////////////////////////////////////////////////////////////////////////////////

extern bool usart_procDataHdlr1 ( u16 DataSize, u8 * Data );
extern bool usart_procDataHdlr2 ( u16 DataSize, u8 * Data );





/* 
 * 功能描述：串口波特率配置
 * 引用参数：
 *          
 * 返回值  ：
 * 
 */

static void hdl_CmdRoutine_SYSUSART ( SHELL_CmdParserTypedef *paras )
{
  switch ( paras->type )
  {
     case ADL_CMD_TYPE_TEST:
       printf ( "\r\n+SYSUSART:[<port>,[baudrate]]\r\n" );
       printf ( "\r\nOK\r\n" );
       break; 

     case ADL_CMD_TYPE_ACT:
//      USART_DebugInfo ();
      break;

     case ADL_CMD_TYPE_READ:
       {
          printf ( "\r\n+SYSUSART:\r\n\r\n" );
#if __USE_USART_1__ == 1
    	    printf ( "USART1 %u HZ, 8N1\r\n", vehicleParams.debugBaudrate );
#endif 
#if __USE_USART_2__ == 1
    	    printf ( "USART2 %u HZ, 8N1\r\n", vehicleParams.gpsBaudrate );
#endif 
#if __USE_USART_3__ == 1
    	    printf ( "USART3 %u HZ, 8N1\r\n", vehicleParams.gsmBaudrate );
#endif 
          printf ( "\r\nOK\r\n" );
        }       
        break; 

     case ADL_CMD_TYPE_PARA:
      {
//        ascii *param0 = NULL, *param1 = NULL;
//        u32 baudrate = 0;
//  
//        printf ( "\r\n+SYSUSART:\r\n" );
//        param0 = ADL_GET_PARAM( paras->rsp, 0 );
//        param1 = ADL_GET_PARAM( paras->rsp, 1 );
//        if ( param1 )
//        {
//          baudrate = ( u32 )atoi ( ( const char * )param1 );
//        }

//        if ( *param0 == '0' && baudrate >= 9600 )
//        {
//          printf ( "\r\nuart 1 baudrate %ubps\r\n", baudrate );
//          printf ( "\r\nOK\r\n" );
//          USART_Stop ( DEV_USART1 );
//          vehicleParams.debugBaudrate = baudrate;
//          STORAGE_WriteData ( VEHICLE_PARAMS, sizeof( vechile_Params_t ), ( u8 * )&vehicleParams );
//          if ( TRUE == USART_IOCtl ( DEV_USART1,
//                                     USART_InitType_BaudRate, vehicleParams.debugBaudrate,
//                                     USART_InitType_None ) )
//          {
//            USART_Start ( DEV_USART1, usart_procDataHdlr1 );
//          }
//          return ;
//        }
//        else
//        if ( *param0 == '1' && baudrate >= 4800 )
//        {
//          printf ( "\r\nuart 2 baudrate %ubps\r\n", baudrate );
//          printf ( "\r\nOK\r\n" );
//          vehicleParams.gpsBaudrate = baudrate;
//          USART_Stop ( DEV_USART2 );
//          STORAGE_WriteData ( VEHICLE_PARAMS, sizeof( vechile_Params_t ), ( u8 * )&vehicleParams );
//          if ( TRUE == USART_IOCtl ( DEV_USART2,
//                                     USART_InitType_BaudRate, vehicleParams.gpsBaudrate,
//                                     USART_InitType_None ) )
//          {
//            USART_Start ( DEV_USART2, usart_procDataHdlr2 );
//          }
//          return ;
//        }
//        else
//        if ( *param0 == '2' && baudrate >= 9600 )
//        {
//          vehicleParams.gsmBaudrate = baudrate;
//          printf ( "\r\nuart 3 baudrate %ubps\r\n", baudrate );
//          printf ( "\r\nOK\r\n" );
//          USART_Stop ( USART_FCM_MODEM );
//          STORAGE_WriteData ( VEHICLE_PARAMS, sizeof( vechile_Params_t ), ( u8 * )&vehicleParams );
//          if ( TRUE == USART_IOCtl ( USART_FCM_MODEM,
//                                     USART_InitType_BaudRate, vehicleParams.gsmBaudrate,
//                                     USART_InitType_None ) )
//          {
//            USART_Start ( USART_FCM_MODEM, MODEM_ReadData );
//          }
//          return ;
//        }
//      	printf ( "error params\r\n" );
//      	printf ( "\r\nERROR\r\n" );
        break;
      }
  } 
}







/* 
 * 功能描述：
 * 引用参数：
 *          
 * 返回值  ：
 * 
 */
static void hdl_CmdRoutine_SYSINFO ( SHELL_CmdParserTypedef *paras )
{   
  switch( paras->type )
  {
    case ADL_CMD_TYPE_TEST:
      printf ( "\r\n+SYSINFO:[<reset-params>]\r\n" );
      printf ( "\r\nOK\r\n" );
      break;

    case ADL_CMD_TYPE_READ:
    {
					unsigned long long simCard = 0;

					/* 读取IP1 */
					printf ( "IP1,%s,%d\r\n", gprsParams.IP1, gprsParams.Port1 );

					/* 读取IP2 */
					printf ( "IP2,%s,%d\r\n", gprsParams.IP2, gprsParams.Port2 );
						
					printf ("VIN: %s \r\n", vehicleParams.CarVIN);
						
					printf ( "ACC open %us close %us sleep time %us\r\n", 
								accInfo.open_time, 
								accInfo.close_time,
								vehicleParams.sleepTime );
					printf ( "sleep mode %d\r\n", vehicleParams.sleepMode );

					printf ( "\r\nOK\r\n" );
     }
      break;

    case ADL_CMD_TYPE_PARA:
    {
    	u8 status = 0;
      ascii *param0 = NULL;

      printf ( "\r\n+SYSINFO:\r\n" );
      param0 = ADL_GET_PARAM( paras->rsp, 0 );
      if ( param0 )
      {
        status = DecToInt ( param0, strlen( ( const char * )param0 ) );
        if ( status == 0 )
        {
  				sysParamsReset ();
  				printf ( "reset system params\r\n" );
  				printf ( "\r\nOK\r\n" );
  				return ;
        }
      }
    	printf ( "error params\r\n" );
    	printf ( "\r\nERROR\r\n" );
      break;
    }
  }
}








/* 
 * 功能描述：
 * 引用参数：
 *          
 * 返回值  ：
 * 
 */
static void hdl_CmdRoutine_SYSDEBUG ( SHELL_CmdParserTypedef *paras )
{
  switch( paras->type )
  {
    case ADL_CMD_TYPE_TEST:
      printf ( "\r\n+SYSDEBUG:[<debug-port>[,<status>]]\r\n" );
      printf ( "\r\nOK\r\n" );
      break;

    case ADL_CMD_TYPE_READ:
      printf ( "\r\n+SYSDEBUG: debug-port %d status %d\r\n", 
	              vehicleParams.DebugCtroller.debugPort, vehicleParams.DebugCtroller.debugIsValid );
      printf ( "GSM: debug %s\r\n", ( vehicleParams.DebugCtroller.debugType & Debug_Type_GSM ) ? "TRUE" : "FALSE" );
      printf ( "GPS: debug %s\r\n", ( vehicleParams.DebugCtroller.debugType & Debug_Type_GPS ) ? "TRUE" : "FALSE" );
      printf ( "FTP: debug %s\r\n", ( vehicleParams.DebugCtroller.debugType & Debug_Type_FTP ) ? "TRUE" : "FALSE" );
      printf ( "CAN: debug %s\r\n", ( vehicleParams.DebugCtroller.debugType & Debug_Type_CAN ) ? "TRUE" : "FALSE" );
      printf ( "SMS: debug %s\r\n", ( vehicleParams.DebugCtroller.debugType & Debug_Type_SMS ) ? "TRUE" : "FALSE" );
      printf ( "MDM: debug %s\r\n", ( vehicleParams.DebugCtroller.debugType & Debug_Type_MDM ) ? "TRUE" : "FALSE" );
      printf ( "COM4:debug %s\r\n", ( vehicleParams.DebugCtroller.debugType & Debug_Type_COM4) ? "TRUE" : "FALSE" );
      printf ( "\r\nOK\r\n" );
      break;

    case ADL_CMD_TYPE_PARA:
    {
      u8 debugPort = 0;
      ascii *param0 = NULL, *param1 = NULL;

      printf ( "\r\n+SYSDEBUG:\r\n" );
      param0 = ADL_GET_PARAM( paras->rsp, 0 );
      if ( param0 )
      {
      	debugPort = DecToInt ( param0, strlen( ( const char * )param0 ) );

	      /* 设置DEBUG */
	    	if ( debugPort < 5 )
	      {
		      param1 = ADL_GET_PARAM( paras->rsp, 1 );
		      if ( *param1 == '0' )
		      {
		        vehicleParams.DebugCtroller.debugIsValid = FALSE;
		      }
		      else
		      if ( *param1 == '1' )
		      {
		        vehicleParams.DebugCtroller.debugIsValid = TRUE;
		      }
					else
					{
		        printf ( "error params\r\n" );
		        printf ( "\r\nERROR\r\n" );
		        return ;
					}

          vehicleParams.DebugCtroller.debugPort = debugPort;
		      STORAGE_WriteData ( VEHICLE_PARAMS, sizeof( vechile_Params_t ), ( u8 * )&vehicleParams );
		      printf ( "debug-port %d status %d\r\n",
		                vehicleParams.DebugCtroller.debugPort,
		                vehicleParams.DebugCtroller.debugIsValid );
		      printf ( "\r\nOK\r\n" );
		      return ;
	      }
      }
      printf ( "error params\r\n" );
      printf ( "\r\nERROR\r\n" );
      break;
    }
  }
}






/* 
 * 功能描述：
 * 引用参数：
 *          
 * 返回值  ：
 * 
 */
static void hdl_CmdRoutine_SYSGSM ( SHELL_CmdParserTypedef *paras )
{           
  switch( paras->type )
  {
    case ADL_CMD_TYPE_TEST:
      printf ( "\r\n+SYSGSM:[<fun>]\r\n" );
      printf ( "\r\nOK\r\n" );
      break;

    case ADL_CMD_TYPE_READ:
	  	printf ( "\r\n+SYSGSM:\r\n" );
	  	GSM_StatusInfoDisplay ();
	  	printf ( "\r\nOK\r\n" );
      break;

    case ADL_CMD_TYPE_PARA:
    {	
      u8 status = 0;
		  ascii *param0 = NULL;

      printf ( "\r\n+SYSGSM:\r\n" );
      param0 = ADL_GET_PARAM( paras->rsp, 0 );
//      if ( param0 )
//      {
//        status = DecToInt ( param0, strlen( ( const char * )param0 ) );
//        if ( status == 0 )
//        {
//        	vehicleParams.DebugCtroller.debugType &= ( ~Debug_Type_GSM );
//        	STORAGE_WriteData ( VEHICLE_PARAMS, sizeof( vechile_Params_t ), ( u8 * )&vehicleParams );
//        	printf ( "0\r\nset gsm debug off\r\n" );
//        	printf ( "\r\nOK\r\n" );
//        	return ;
//        }
//        else
//        if ( status == 1 )
//        {
//        	vehicleParams.DebugCtroller.debugType |= Debug_Type_GSM;
//        	STORAGE_WriteData ( VEHICLE_PARAMS, sizeof( vechile_Params_t ), ( u8 * )&vehicleParams );
//        	printf ( "1\r\nset gsm debug on\r\n" );
//        	printf ( "\r\nOK\r\n" );
//        	return ;
//        }
//        else
//        if ( status == 2 )
//        {
//        	printf ( "2\r\nquery gsm status\r\n" );
//        	GSM_StatusInfoDisplay ();
//        	printf ( "\r\nOK\r\n" ); 
//        	return ;     	
//        }
//        else
//        if ( status == 3 )
//        {
//          if ( MDM_QueryFlowStatus () == MDM_FCM_V24_STATE_AT )
//          {
//            printf ( "\r\ngsm flow status MDM_FCM_V24_STATE_AT\r\n" );
//          }
//          else
//          if ( MDM_QueryFlowStatus () == MDM_FCM_V24_STATE_DATA )
//          {
//            printf ( "\r\ngsm flow status MDM_FCM_V24_STATE_DATA\r\n" );
//          }
//          else
//          if ( MDM_QueryFlowStatus () == MDM_FCM_V24_STATE_SW )
//          {
//            printf ( "\r\ngsm flow status MDM_FCM_V24_STATE_SW\r\n" );
//          }
//          printf ( "\r\nOK\r\n" );
//          return ;
//        }
//      }
    	printf ( "error params\r\n" );
    	printf ( "\r\nERROR\r\n" );
      break;
    }
  }
}






/* 
 * 功能描述：
 * 引用参数：
 *          
 * 返回值  ：
 * 
 */
static void hdl_CmdRoutine_SYSFTP ( SHELL_CmdParserTypedef *paras )
{           
  switch( paras->type )
  {
    case ADL_CMD_TYPE_TEST:
      printf ( "\r\n+SYSFTP:[<fun>]\r\n" );
      printf ( "\r\nOK\r\n" );
      break;

    case ADL_CMD_TYPE_PARA:
    {	
		  u8 status = 0;
      ascii *param0 = NULL, *param1 = NULL;

      param0 = ADL_GET_PARAM( paras->rsp, 0 );
      if ( param0 )
      {
        status = DecToInt ( param0, strlen( ( const char * )param0 ) );
        if ( status == 0 )
        {
        	vehicleParams.DebugCtroller.debugType &= ( ~Debug_Type_FTP );
        	STORAGE_WriteData ( VEHICLE_PARAMS, sizeof( vechile_Params_t ), ( u8 * )&vehicleParams );
        	printf ( "\r\n+SYSFTP: 0\r\nset ftp debug off\r\n\r\nOK\r\n" );
        	return ;
        }
        else
        if ( status == 1 )
        {
        	vehicleParams.DebugCtroller.debugType |= Debug_Type_FTP;
        	STORAGE_WriteData ( VEHICLE_PARAMS, sizeof( vechile_Params_t ), ( u8 * )&vehicleParams );
        	printf ( "\r\n+SYSFTP: 1\r\nset ftp debug on\r\n\r\nOK\r\n" );
        	return ;
        }
        else
        if ( status == 2 )
        {
        	param1 = ADL_GET_PARAM( paras->rsp, 1 );
					printf("\r\n param1 =%s\r\n",param1);
          if ( param1 )
          {
						
            ftp_parseDataHdlr( strlen ( ( const char* )param1 ), ( u8 * )param1 );
            printf ( "\r\n+SYSFTP: 2\r\nstart ftp\r\n\r\nOK\r\n" );            
          }
        	return ;
        }
      }
    	printf ( "\r\n+SYSFTP: error params\r\n\r\nERROR\r\n" );
      break;
    }
  }
}




#if __USE_TCP_TEST__ == 1


/* 
 * 功能描述：
 * 引用参数：
 *          
 * 返回值  ：
 * 
 */
static void hdl_CmdRoutine_SYSTCP ( SHELL_CmdParserTypedef *paras )
{           
  switch( paras->type )
  {
    case ADL_CMD_TYPE_TEST:
      printf ( "\r\n+SYSTCP:[<time>],[<size>],[<total-count>,[<port>]]\r\n" );
      printf ( "\r\nOK\r\n" );
      break;

    case ADL_CMD_TYPE_PARA:
    {	
      ascii *param0 = NULL, *param1 = NULL, *param2 = NULL, *param3 = NULL;
      u32 time = 0, size = 0, total = 0;
      u16 port = 0;

      param0 = ADL_GET_PARAM( paras->rsp, 0 );
      if ( param0 )
      {
        time = DecToInt ( param0, strlen( ( const char * )param0 ) );
      }

      param1 = ADL_GET_PARAM( paras->rsp, 1 );
      if ( param1 )
      {
        size = DecToInt ( param1, strlen( ( const char * )param1 ) );
      }

      param2 = ADL_GET_PARAM( paras->rsp, 2 );
      if ( param2 )
      {
        total = DecToInt ( param2, strlen( ( const char * )param2 ) );
      }

      param3 = ADL_GET_PARAM( paras->rsp, 3 );
      if ( param3 )
      {
        port = DecToInt ( param3, strlen( ( const char * )param3 ) );
      }

      if ( time == 0 && size == 0 && total == 0 && port == 0 )
    	{
        printf ( "\r\n+SYSTCP: error params\r\n\r\nERROR\r\n" );
      }
      else
      {
        TCP_TestParamsSet ( time, size, total, port );
        printf ( "\r\n+SYSTCP:\r\n\r\nOK\r\n" );
      }
      break;
    }

    case ADL_CMD_TYPE_ACT:
      {
        printf ( "\r\n+SYSTCP:\r\n" );
        TCP_TestParamsGet ();
        printf ( "\r\nOK\r\n" );  
      }
      break;
  }
}


#endif  /* __USE_TCP_TEST__ == 1 */







/* 
 * 功能描述：
 * 引用参数：
 *          
 * 返回值  ：
 * 
 */
static void hdl_CmdRoutine_SYSSMS ( SHELL_CmdParserTypedef *paras )
{           
  switch( paras->type )
  {
    case ADL_CMD_TYPE_TEST:
      printf ( "\r\n+SYSSMS:[<fun>]\r\n" );
      printf ( "\r\nOK\r\n" );
      break;

    case ADL_CMD_TYPE_PARA:
    {	
		  u8 status = 0;
      ascii *param0 = NULL;

      param0 = ADL_GET_PARAM( paras->rsp, 0 );
//      if ( param0 )
//      {
//        status = DecToInt ( param0, strlen( ( const char * )param0 ) );
//        if ( status == 0 )
//        {
//        	vehicleParams.DebugCtroller.debugType &= ( ~Debug_Type_SMS );
//        	STORAGE_WriteData ( VEHICLE_PARAMS, sizeof( vechile_Params_t ), ( u8 * )&vehicleParams );
//        	printf ( "\r\n+SYSSMS: 0\r\nset sms debug off\r\n\r\nOK\r\n" );
//        	return ;
//        }
//        else
//        if ( status == 1 )
//        {
//        	vehicleParams.DebugCtroller.debugType |= Debug_Type_SMS;
//        	STORAGE_WriteData ( VEHICLE_PARAMS, sizeof( vechile_Params_t ), ( u8 * )&vehicleParams );
//        	printf ( "\r\n+SYSSMS: 1\r\nset sms debug on\r\n\r\nOK\r\n" );
//        	return ;
//        }      
//      }
    	printf ( "\r\n+SYSSMS: error params\r\n\r\nERROR\r\n" );
      break;
    }
  }
}






void stats_display ( void );


/* 
 * 功能描述：
 * 引用参数：
 *          
 * 返回值  ：
 * 
 */
static void hdl_CmdRoutine_SYSIP ( SHELL_CmdParserTypedef *paras )
{
  switch ( paras->type )
  {
    case ADL_CMD_TYPE_TEST:
      printf ( "\r\n+SYSIP:\r\n" );
      printf ( "\r\nOK\r\n" );
      break;

    case ADL_CMD_TYPE_ACT:
      printf ( "\r\n+SYSIP:\r\n" );
      stats_display ();
      printf ( "\r\nOK\r\n" );
      break;
  }
}






/* 
 * 功能描述：
 * 引用参数：
 *          
 * 返回值  ：
 * 
 */
static void hdl_CmdRoutine_SYSCAN ( SHELL_CmdParserTypedef *paras )
{
u8 i =0;
  switch( paras->type )
  {
    case ADL_CMD_TYPE_TEST:
      printf ( "\r\n+SYSCAN:[<fun>[,<frame-type>,<can-frame>]]\r\n" );
      printf ( "\r\nOK\r\n" );
      break;

    case ADL_CMD_TYPE_READ:
//      printf ( "\r\n+SYSCAN:\r\n" );
//      printf ( "CAN %u kHZ, FRAME-TYPE %u FACTORY %u  CAN_ID_number\r\n", gprsParams.diy_can_baut, gprsParams.diy_can_type,gprsParams.diy_can_factry,gprsParams.diy_can_nub );
// 	  for(i=0;i<gprsParams.diy_can_nub;i++)
//	  {
//	   	  printf("CANID%d %X\r\n",i,gprsParams.diy_can_data[ i ]);

//	  }
//		printf ( "\r\nOK\r\n" );
      break;

    case ADL_CMD_TYPE_PARA:
//    {	
//		  u8 status = 0;
//      ascii *param0 = NULL;
//	  ascii *param1 = NULL;
//      param0 = ADL_GET_PARAM( paras->rsp, 0 );
//      if ( param0 )
//      {

//		if(((param0[ 0 ]=='B')||(param0[ 0 ]=='b'))&&
//		   ((param0[ 1 ]=='u')||(param0[ 1 ]=='U'))&&
//		   ((param0[ 2 ]=='S')||(param0[ 2 ]=='s')))
//		   {	
//		   		param1 = ADL_GET_PARAM( paras->rsp, 1 );
//				gprsParams.diy_can_baut = DecToInt ( param1, strlen( ( const char * )param1 ) );
//		   		printf("CAN bus %d\r\n",gprsParams.diy_can_baut);
//		   }

//		if(((param0[ 0 ]=='T')||(param0[ 0 ]=='t'))&&
//		   ((param0[ 1 ]=='Y')||(param0[ 1 ]=='y'))&&
//		   ((param0[ 2 ]=='P')||(param0[ 2 ]=='p')))
//		   {	
//		   		param1 = ADL_GET_PARAM( paras->rsp, 1 );
//				gprsParams.diy_can_type = DecToInt ( param1, strlen( ( const char * )param1 ) );
//				printf("CAN type %d\r\n",gprsParams.diy_can_type);
//		   
//		   }
//		if(((param0[ 0 ]=='C')||(param0[ 0 ]=='c'))&&
//		   ((param0[ 1 ]=='N')||(param0[ 1 ]=='n'))&&
//		   ((param0[ 2 ]=='B')||(param0[ 2 ]=='b')))
//		   {	
//		   		param1 = ADL_GET_PARAM( paras->rsp, 1 );
//				gprsParams.diy_can_nub = DecToInt ( param1, strlen( ( const char * )param1 ) );
//				printf("CAN ID number %d\r\n",gprsParams.diy_can_nub);
//		   
//		   }

//		if(((param0[ 0 ]=='F')||(param0[ 0 ]=='f'))&&
//		   ((param0[ 1 ]=='C')||(param0[ 1 ]=='c'))&&
//		   ((param0[ 2 ]=='T')||(param0[ 2 ]=='t')))
//		   {	
//		   		param1 = ADL_GET_PARAM( paras->rsp, 1 );
//				gprsParams.diy_can_factry = DecToInt ( param1, strlen( ( const char * )param1 ) );		   
//				printf("CAN factry NB %d\r\n",gprsParams.diy_can_factry);
//		   
//		   }


//		if(((param0[ 0 ]=='C')||(param0[ 0 ]=='c'))&&
//		   ((param0[ 1 ]=='I')||(param0[ 1 ]=='i'))&&
//		   ((param0[ 2 ]=='D')||(param0[ 2 ]=='d'))&&
//		   ((param0[ 3 ]=='1')))
//		   {	
//		   		param1 = ADL_GET_PARAM( paras->rsp, 1 );
//				gprsParams.diy_can_data[ 0 ] = HexToInt ( param1, strlen( ( const char * )param1 ) );
//				printf("CAN diy_can_data_0  %x\r\n",gprsParams.diy_can_data[ 0 ]);
//		   
//		   }

//		if(((param0[ 0 ]=='C')||(param0[ 0 ]=='c'))&&
//		   ((param0[ 1 ]=='I')||(param0[ 1 ]=='i'))&&
//		   ((param0[ 2 ]=='D')||(param0[ 2 ]=='d'))&&
//		   ((param0[ 3 ]=='2')))
//		   {	
//		   		param1 = ADL_GET_PARAM( paras->rsp, 1 );
//				gprsParams.diy_can_data[ 1 ] = HexToInt ( param1, strlen( ( const char * )param1 ) );		   
//		   				printf("CAN diy_can_data_1  %x\r\n",gprsParams.diy_can_data[ 1 ]);

//		   }

//		if(((param0[ 0 ]=='C')||(param0[ 0 ]=='c'))&&
//		   ((param0[ 1 ]=='I')||(param0[ 1 ]=='i'))&&
//		   ((param0[ 2 ]=='D')||(param0[ 2 ]=='d'))&&
//		   ((param0[ 3 ]=='3')))
//		   {	
//		   		param1 = ADL_GET_PARAM( paras->rsp, 1 );
//				gprsParams.diy_can_data[ 2 ] = HexToInt ( param1, strlen( ( const char * )param1 ) );		   
//				   				printf("CAN diy_can_data_2  %x\r\n",gprsParams.diy_can_data[ 2 ]);

//		   }
//		if(((param0[ 0 ]=='C')||(param0[ 0 ]=='c'))&&
//		   ((param0[ 1 ]=='I')||(param0[ 1 ]=='i'))&&
//		   ((param0[ 2 ]=='D')||(param0[ 2 ]=='d'))&&
//		   ((param0[ 3 ]=='4')))
//		   {	
//		   		param1 = ADL_GET_PARAM( paras->rsp, 1 );
//				gprsParams.diy_can_data[ 3 ] = HexToInt ( param1, strlen( ( const char * )param1 ) );		   
//		   				printf("CAN diy_can_data_3  %x\r\n",gprsParams.diy_can_data[ 3 ]);

//		   }

//		if(((param0[ 0 ]=='C')||(param0[ 0 ]=='c'))&&
//		   ((param0[ 1 ]=='I')||(param0[ 1 ]=='i'))&&
//		   ((param0[ 2 ]=='D')||(param0[ 2 ]=='d'))&&
//		   ((param0[ 3 ]=='5')))
//		   {	
//		   		param1 = ADL_GET_PARAM( paras->rsp, 1 );
//				gprsParams.diy_can_data[ 4 ] = HexToInt ( param1, strlen( ( const char * )param1 ) );		   
//			   				printf("CAN diy_can_data_4  %x\r\n",gprsParams.diy_can_data[ 4 ]);

//		   }

//		if(((param0[ 0 ]=='C')||(param0[ 0 ]=='c'))&&
//		   ((param0[ 1 ]=='I')||(param0[ 1 ]=='i'))&&
//		   ((param0[ 2 ]=='D')||(param0[ 2 ]=='d'))&&
//		   ((param0[ 3 ]=='6')))
//		   {	
//		   		param1 = ADL_GET_PARAM( paras->rsp, 1 );
//				gprsParams.diy_can_data[ 5 ] = HexToInt ( param1, strlen( ( const char * )param1 ) );		   
//		  		   				printf("CAN diy_can_data_5  %x\r\n",gprsParams.diy_can_data[ 5 ]);

//		   }

//		if(((param0[ 0 ]=='C')||(param0[ 0 ]=='c'))&&
//		   ((param0[ 1 ]=='I')||(param0[ 1 ]=='i'))&&
//		   ((param0[ 2 ]=='D')||(param0[ 2 ]=='d'))&&
//		   ((param0[ 3 ]=='7')))
//		   {	
//		   		param1 = ADL_GET_PARAM( paras->rsp, 1 );
//				gprsParams.diy_can_data[ 6 ] = HexToInt ( param1, strlen( ( const char * )param1 ) );		   
//		 		   				printf("CAN diy_can_data_6  %x\r\n",gprsParams.diy_can_data[ 6 ]);

//		   }

//			if(((param0[ 0 ]=='C')||(param0[ 0 ]=='c'))&&
//		   ((param0[ 1 ]=='I')||(param0[ 1 ]=='i'))&&
//		   ((param0[ 2 ]=='D')||(param0[ 2 ]=='d'))&&
//		   ((param0[ 3 ]=='8')))
//		   {	
//		   		param1 = ADL_GET_PARAM( paras->rsp, 1 );
//				gprsParams.diy_can_data[ 7 ] = HexToInt ( param1, strlen( ( const char * )param1 ) );		   
//		  		   				printf("CAN diy_can_data_7  %x\r\n",gprsParams.diy_can_data[ 7 ]);

//		   }

//		if(((param0[ 0 ]=='C')||(param0[ 0 ]=='c'))&&
//		   ((param0[ 1 ]=='I')||(param0[ 1 ]=='i'))&&
//		   ((param0[ 2 ]=='D')||(param0[ 2 ]=='d'))&&
//		   ((param0[ 3 ]==9)))
//		   {	
//		   		param1 = ADL_GET_PARAM( paras->rsp, 1 );
//				gprsParams.diy_can_data[ 8 ] = HexToInt ( param1, strlen( ( const char * )param1 ) );		   
//		  		   				printf("CAN diy_can_data_8  %x\r\n",gprsParams.diy_can_data[ 8 ]);

//		   }

//		if(((param0[ 0 ]=='C')||(param0[ 0 ]=='c'))&&
//		   ((param0[ 1 ]=='I')||(param0[ 1 ]=='i'))&&
//		   ((param0[ 2 ]=='D')||(param0[ 2 ]=='d'))&&
//		   ((param0[ 3 ]=='A')||(param0[ 3 ]=='a')))
//		   {	
//		   		param1 = ADL_GET_PARAM( paras->rsp, 1 );
//				gprsParams.diy_can_data[ 9 ] = HexToInt ( param0, strlen( ( const char * )param1 ) );		   
//		  		   				printf("CAN diy_can_data_9  %x\r\n",gprsParams.diy_can_data[ 9 ]);

//		   }

//		if(((param0[ 0 ]=='C')||(param0[ 0 ]=='c'))&&
//		   ((param0[ 1 ]=='I')||(param0[ 1 ]=='i'))&&
//		   ((param0[ 2 ]=='D')||(param0[ 2 ]=='d'))&&
//		   ((param0[ 3 ]=='B')||(param0[ 3 ]=='b')))
//		   {	
//		   		param1 = ADL_GET_PARAM( paras->rsp, 1 );
//				gprsParams.diy_can_data[ 10 ] = HexToInt ( param1, strlen( ( const char * )param1 ) );		   
//		  		   				printf("CAN diy_can_data_10  %x\r\n",gprsParams.diy_can_data[ 10 ]);

//		   }

//		if(((param0[ 0 ]=='C')||(param0[ 0 ]=='c'))&&
//		   ((param0[ 1 ]=='I')||(param0[ 1 ]=='i'))&&
//		   ((param0[ 2 ]=='D')||(param0[ 2 ]=='d'))&&
//		   ((param0[ 3 ]=='C')||(param0[ 3 ]=='c')))
//		   {	
//		   		param1 = ADL_GET_PARAM( paras->rsp, 1 );
//				gprsParams.diy_can_data[11 ] = HexToInt ( param1, strlen( ( const char * )param1 ) );		   
//		   		   				printf("CAN diy_can_data_11  %x\r\n",gprsParams.diy_can_data[ 11 ]);

//		   }


//		if(((param0[ 0 ]=='C')||(param0[ 0 ]=='c'))&&
//		   ((param0[ 1 ]=='I')||(param0[ 1 ]=='i'))&&
//		   ((param0[ 2 ]=='D')||(param0[ 2 ]=='d'))&&
//		   ((param0[ 3 ]=='D')||(param0[ 3 ]=='d')))
//		   {	
//		   		param1 = ADL_GET_PARAM( paras->rsp, 1 );
//				gprsParams.diy_can_data[ 12 ] = HexToInt ( param1, strlen( ( const char * )param1 ) );		   
//		  		   				printf("CAN diy_can_data_12  %x\r\n",gprsParams.diy_can_data[ 12 ]);

//		   }

//		if(((param0[ 0 ]=='C')||(param0[ 0 ]=='c'))&&
//		   ((param0[ 1 ]=='I')||(param0[ 1 ]=='i'))&&
//		   ((param0[ 2 ]=='D')||(param0[ 2 ]=='d'))&&
//		   ((param0[ 3 ]=='E')||(param0[ 3 ]=='e')))
//		   {	
//		   		param1 = ADL_GET_PARAM( paras->rsp, 1 );
//				gprsParams.diy_can_data[ 13 ] = HexToInt ( param0, strlen( ( const char * )param0 ) );		   
//		   		   				printf("CAN diy_can_data_13  %x\r\n",gprsParams.diy_can_data[ 13 ]);

//		   }
//			if(((param0[ 0 ]=='C')||(param0[ 0 ]=='c'))&&
//		   ((param0[ 1 ]=='I')||(param0[ 1 ]=='i'))&&
//		   ((param0[ 2 ]=='D')||(param0[ 2 ]=='d'))&&
//		   ((param0[ 3 ]=='F')||(param0[ 3 ]=='f')))
//		   {	
//		   		param1 = ADL_GET_PARAM( paras->rsp, 1 );
//				gprsParams.diy_can_data[ 14 ] = HexToInt ( param1, strlen( ( const char * )param1 ) );		   
//		   		   				printf("CAN diy_can_data_14  %x\r\n",gprsParams.diy_can_data[ 14 ]);

//		   }
// STORAGE_WriteData ( GPRS_PARAMS, sizeof( gprs_Params_t ), ( u8* )&gprsParams );

//   #if 0
//        status = DecToInt ( param0, strlen( ( const char * )param0 ) );
//        if ( status == 0 )
//        {
//        	vehicleParams.DebugCtroller.debugType &= ( ~Debug_Type_CAN );
//        	STORAGE_WriteData ( VEHICLE_PARAMS, sizeof( vechile_Params_t ), ( u8 * )&vehicleParams );
//        	printf ( "\r\n+SYSCAN: 0\r\nset can-controller debug off\r\n\r\nOK\r\n" );
//        	return ;
//        }
//        else
//        if ( status == 1 )
//        {
//        	vehicleParams.DebugCtroller.debugType |= Debug_Type_CAN;
//        	STORAGE_WriteData ( VEHICLE_PARAMS, sizeof( vechile_Params_t ), ( u8 * )&vehicleParams );
//        	printf ( "\r\n+SYSCAN: 1\r\nset can-controller debug on\r\n\r\nOK\r\n" );
//        	return ;
//        }
//        else
//        if ( status == 2 && ( vehicleParams.DebugCtroller.debugType & Debug_Type_CAN ) )
//        {
//          ascii *param1 = ADL_GET_PARAM( paras->rsp, 1 );	

//          if ( param1 && ( strlen ( ( const char* )param1 ) == 1 ) )
//          {
//            ascii *param2 = ADL_GET_PARAM( paras->rsp, 2 ); 
//            u8 can_senddata[ 12 ] = {0};
//            u8 length = strlen ( ( const char* )param2 );
//                              
//            switch ( *param1 )
//            {

//#if ( CAN_FRAME_STD == CAN_IDE_TYPE )||( CAN_FRAME_EXT_AND_STD == CAN_IDE_TYPE ) 
//              case '0':
//                {
//                  if ( length > 4 && length < 20 )
//                  {
//                    MDM_String2Bytes ( ( const u8* )param2, can_senddata, length );
//                  }
//                  else
//                  if ( length >= 20 )
//                  {
//                    length = 20;
//                    MDM_String2Bytes ( ( const u8* )param2, can_senddata, length );
//                  }
//                  else
//                  {
//                    length = 0; 
//                  }

//                  length /= 2;
//                  if ( length > 0 && TRUE == CAN_SendMessage ( can_senddata, length ) )
//                  {
//                    printf ( "\r\n+SYSCAN:\r\nsend std-frame\r\n\r\nOK\r\n" );
//                    return ;
//                  }
//                }                
//                break;
//#endif  /* CAN_FRAME_STD */ 

//#if ( CAN_FRAME_EXT == CAN_IDE_TYPE )||( CAN_FRAME_EXT_AND_STD == CAN_IDE_TYPE )
//              case '1':
//                if ( param2 )
//                {
//                  if ( length > 8 && length < 24 )
//                  {
//                    MDM_String2Bytes ( ( const u8* )param2, can_senddata, length );
//                  }
//                  else
//                  if ( length >= 24 )
//                  {
//                    length = 24;
//                    MDM_String2Bytes ( ( const u8* )param2, can_senddata, length );
//                  }
//                  else
//                  {
//                    length = 0; 
//                  }

//                  length /= 2;
//                  if ( length > 0 && TRUE == CAN_SendMessage ( can_senddata, length ) )
//                  {
//                    printf ( "\r\n+SYSCAN:\r\nsend ext-frame\r\n\r\nOK\r\n" );
//                    return ;
//                  }
//                }              
//                break;
//#endif  /* CAN_FRAME_EXT */ 

//              default:
//                break;
//            }
//          }
//        }
//#endif      
//      }
//    	printf ( "\r\n+SYSCAN: error params\r\n\r\nERROR\r\n" );
      break;
//    }
  }
}







/* 
 * 功能描述：
 * 引用参数：
 *          
 * 返回值  ：
 * 
 */
static void hdl_CmdRoutine_SYSRTC ( SHELL_CmdParserTypedef *paras )
{         
  switch( paras->type )
  {
    case ADL_CMD_TYPE_TEST:
      printf ( "\r\n+SYSRTC:\r\n" );
      printf ( "\r\nOK\r\n" );
      break;

    case ADL_CMD_TYPE_READ:
    	printf ( "\r\nOK\r\n" );
      break;

    case ADL_CMD_TYPE_PARA:
    	printf ( "\r\nOK\r\n" );
      break;
  }
}







/* 
 * 功能描述：
 * 引用参数：
 *          
 * 返回值  ：
 * 
 */
static void hdl_CmdRoutine_SYSCELLPHONE ( SHELL_CmdParserTypedef *paras )
{         
  switch( paras->type )
  {
    case ADL_CMD_TYPE_TEST:
      printf ( "\r\n+SYSCELLPHONE:[<id>,<phone-number>]\r\n" );
      printf ( "\r\nOK\r\n" );
      break;

    case ADL_CMD_TYPE_READ:
//      printf ( "\r\n+SYSCELLPHONE:\r\n" );
//      printf ( "0) %s\r\n", vehicleParams.sc1 );
//      printf ( "1) %s\r\n", vehicleParams.sc2 );
//      printf ( "2) %s\r\n", vehicleParams.c1 );
//      printf ( "3) %s\r\n", vehicleParams.c2 );
//      printf ( "4) %s\r\n", vehicleParams.c3 );
//    	printf ( "\r\nOK\r\n" );
      break;

    case ADL_CMD_TYPE_PARA:
//    {
//    	ascii *param0 = NULL;

//      printf ( "\r\n+SYSCELLPHONE:\r\n" );
//      param0 = ADL_GET_PARAM( paras->rsp, 0 );
//      if ( param0 && strlen ( ( const char * )param0 ) == 1 )
//      {
//        ascii *param1 = ADL_GET_PARAM( paras->rsp, 1 ); 
//        u8 length = strlen ( ( const char* )param1 );

//        if ( param1 && length < 15 )
//        {
//          switch ( *param0 )
//          {
//            case '0':
//              sprintf ( ( char * )vehicleParams.sc1, "%s", param1 );
//              STORAGE_WriteData ( VEHICLE_PARAMS, sizeof( vechile_Params_t ), ( u8 * )&vehicleParams );
//              printf ( "%s\r\n\r\nOK\r\n", param1 );
//              return;
//            
//            case '1':
//              sprintf ( ( char * )vehicleParams.sc2, "%s", param1 );
//              STORAGE_WriteData ( VEHICLE_PARAMS, sizeof( vechile_Params_t ), ( u8 * )&vehicleParams );
//              printf ( "%s\r\n\r\nOK\r\n", param1 );
//              return;
//            
//            case '2':
//              sprintf ( ( char * )vehicleParams.c1, "%s", param1 );
//              STORAGE_WriteData ( VEHICLE_PARAMS, sizeof( vechile_Params_t ), ( u8 * )&vehicleParams );
//              printf ( "%s\r\n\r\nOK\r\n", param1 );
//              return;
//  
//            case '3':
//              sprintf ( ( char * )vehicleParams.c2, "%s", param1 );
//              STORAGE_WriteData ( VEHICLE_PARAMS, sizeof( vechile_Params_t ), ( u8 * )&vehicleParams );
//              printf ( "%s\r\n\r\nOK\r\n", param1 );
//              return;
//  
//            case '4':
//              sprintf ( ( char * )vehicleParams.c3, "%s", param1 );
//              STORAGE_WriteData ( VEHICLE_PARAMS, sizeof( vechile_Params_t ), ( u8 * )&vehicleParams );
//              printf ( "%s\r\n\r\nOK\r\n", param1 );
//              return;
//  
//            default:
//              break;                                                        
//          }     
//        }
//      }
//      printf ( "error params\r\n\r\nERROR\r\n" );
      break;
//    }
  }
}









/* 
 * 功能描述：
 * 引用参数：
 *          
 * 返回值  ：
 * 
 */
static void hdl_CmdRoutine_SYSVER ( SHELL_CmdParserTypedef *paras )
{         
  switch( paras->type )
  {
    case ADL_CMD_TYPE_READ:
    case ADL_CMD_TYPE_ACT:
      printf ( "\r\n+SYSVER:\r\n" );

      /* 读取程序版本号 */
      printf ( "software %s released @ %s %s\r\nhardware %s\r\n", 
               SOFTWARE_VERSION, __DATE__, __TIME__,
               HARDWARE_EDITION );
    	printf ( "\r\nOK\r\n" );
      break;
  }
}







#if ACC_WORK_STATS_DEBUG == 1
extern u8 acc_upload_time_hour, acc_upload_time_minute;



/* 
 * 功能描述：
 * 引用参数：
 *          
 * 返回值  ：
 * 
 */
static void hdl_CmdRoutine_SYSACCSTATS ( SHELL_CmdParserTypedef *paras )
{         

}
#endif  /* ACC_WORK_STATS_DEBUG */









/* 
 * 功能描述：
 * 引用参数：
 *          
 * 返回值  ：
 * 
 */
static void hdl_CmdRoutine_SYSSLEEP ( SHELL_CmdParserTypedef *paras )
{         
  switch( paras->type )
  {
    case ADL_CMD_TYPE_TEST:
      printf ( "\r\n+SYSSLEEP:[<sleep_type>,<alarm-time>]\r\n" );
      printf ( "\r\nOK\r\n" );
      break;

    case ADL_CMD_TYPE_READ:
			printf ( "\r\n+SYSINFO:\r\n" ); 
    	printf ( "sleep mode %d\r\n", vehicleParams.sleepMode );
    	printf ( "\r\nOK\r\n" );
      break;

    case ADL_CMD_TYPE_PARA:
    {
        ascii *param0 = NULL, *param1 = NULL;

        printf ( "\r\n+SYSSLEEP:\r\n" );
        param0 = ADL_GET_PARAM( paras->rsp, 0 );
        if ( param0 )
        {
          param1 = ADL_GET_PARAM( paras->rsp, 1 );
          if ( param1 )
          {
            s32 time = atoi ( ( const char* )param1 ); 
          }

          if ( *param0 == '0' )
          {
            printf ( "\r\nsleep mode off\r\n" );
						//SleepAcc_Enable=0;
						//acc_io_handler=-1;
						vehicleParams.sleepMode=FALSE;
						STORAGE_WriteData ( VEHICLE_PARAMS, sizeof( vechile_Params_t ), ( u8* )&vehicleParams );
						STORAGE_FlashParams_WriteData(( u8* )&vehicleParams ,sizeof( vechile_Params_t ),FLASH_VEHICLE_PARAMS);
          }
          else
          if ( *param0 == '1' )
          {
						printf ( "\r\nsleep mode on\r\n" );
						//SleepAcc_Enable=1;
						vehicleParams.sleepMode=TRUE;
						STORAGE_WriteData ( VEHICLE_PARAMS, sizeof( vechile_Params_t ), ( u8* )&vehicleParams );
						STORAGE_FlashParams_WriteData(( u8* )&vehicleParams ,sizeof( vechile_Params_t ),FLASH_VEHICLE_PARAMS);
          } 
					else
					if ( *param0 == '2' )
					{
						printf ( "\r\nEnter sleep mode\r\n" );
						SLEEP_TYPEI_IsSleeping();	
					}
        }
				printf ( "\r\nOK\r\n" );
        break;      
      }
  }
}







#if J1939_USART_DEBUG == 1
/* 
 * 功能描述：
 * 引用参数：
 *          
 * 返回值  ：
 * 
 */
static void hdl_CmdRoutine_SYSJ1939 ( SHELL_CmdParserTypedef *paras )
{           
  switch( paras->type )
  {
    case ADL_CMD_TYPE_TEST:
      printf ( "\r\n+SYSJ1939:[<message>]\r\n" );
      printf ( "\r\nOK\r\n" );
      break;

    case ADL_CMD_TYPE_PARA:
    {	
      ascii *param0 = NULL;

      param0 = ADL_GET_PARAM( paras->rsp, 0 );
      if ( param0 )
      {
        u8 data[ 64 ] = { 0 };  
        s8 length = strlen ( ( const char* )param0 );

        /* 格式转换 */
        length = MDM_String2Bytes ( ( const u8* )param0, data, length );
        printf ( "\r\n+SYSJ1939:\r\nrx-size %u\r\n", length );
        printf ( "\r\nOK\r\n" );
        if ( length <= 8 && length > 0 )
        {
          J1939_L21_CommandMessage ( 0x00FF4F, 0x4F, data, length );
        }
        else
        {
          J1939_L21_CommandMessage ( 0x004F00, 0x54, data, length ); 
        }
        return ;              
      }
    	printf ( "\r\n+SYSJ1939: error params\r\n\r\nERROR\r\n" );
      break;
    }
  }
}
#endif  /* J1939_USART_DEBUG */



