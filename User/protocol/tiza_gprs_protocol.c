#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tiza_gprs_protocol.h"
#include "xmit_manager.h"
#include "rexmit_strategy.h"
#include "buffer_helper.h"
#include "tiza_gps_port.h"
#include "modem.h"
#include "tiza_storage.h"
#include "tiza_params.h"
#include "gb_gprs_protocol.h"
#include "ftpclient.h"
#include "tiza_upgrade.h"



/**********/
/* 标志组 */
/**********/

/* 获取SIM卡ICCID */
char *RECV_SIMICCID=NULL;

/* 中心服务器标志 */
u32 centerSrvStatus = CENTER_SERVER1_LOGIN_BIT;

/* 应用协议运行状态 */
sys_status_t sys_status = SYSTEM_PROC_BEGIN;





/****************/
/*   GPRS配置   */
/****************/

/* GPRS参数 */
extern gprs_Params_t gprsParams;

/*IMEI */
extern ascii E_IMEI[16];




/****************/
/*    FTP配置   */
/****************/

/* 端口号 */
static u16   DOTA_FtpPort;

/* 用户名 */
static ascii DOTA_FtpUsername[ FTP_LOGIN_MAX ];

/* 用户密码 */  
static ascii DOTA_FtpUserPwd[ FTP_PASSWORD_MAX ];

/* 服务器升级地址 */ 
static ascii DOTA_FtpAddress[ FTP_SERV_ADDRESS_MAX ]; 

/* 下载文件地址 */
static ascii DOTA_FtpFilename[ FTP_FILE_PATH_NAME_MAX ];

/* 文件数据计数器 */
static u32 DOTA_FtpDataCount = 0;

/**************************************************************/\

//flash中存入的BIN数据大小
u32 FtpToFlash_DataCount = 0;
 


/*************************************************************************************/



/**************************************************************/



/**************************************************************/


/******************************************/
/*              内部函数[声明]            */
/******************************************/

static u16 AccUploadWorkingParams ( void );
static u16 ControllerDataRefresh ( void );
static u16 MultControllerDataRefresh ( void );
/* FTP相关API */
static void ftp_eventHandler ( wip_event_t event, void *ctx );
static bool ftp_recvDataHdlr ( u16 DataSize, u8 *Data );

/* 协议处理 */
static void Gprs_upgradeResultMessage ( void );
static u8   Gprs_DataCheck ( u8 *data, u16 len );
static void SysResetType_Upgrade( void );









/******************************************/
/*              [GPRS协议部分]            */
/******************************************/


/* 
 * 功能描述：FTP数据接收处理
 * 引用参数：
 *         
 * 返回值  ：
 * 
 */
static bool ftp_recvDataHdlr ( u16 DataSize, u8 *Data )
{
  DOTA_FtpDataCount += DataSize;
  accInfo.close_time=0;//防止电话唤醒后，升级时进入休眠

  //FTP_PROFILE_GETDATA ( DataSize, DOTA_FtpDataCount );

///////////////////////////////////////////////////////////////////////////
	printf ( "[FTP] recv length %u, total %ubytes", DataSize, DOTA_FtpDataCount );//@goly
  if ( vehicleParams.DebugCtroller.debugType & Debug_Type_FTP )
  {
    PRINTF_GPS_TIME ();
    printf ( "[FTP] recv length %u, total %ubytes", DataSize, DOTA_FtpDataCount );
  }

///////////////////////////////////////////////////////////////////////////

  if(UPGRADE_WriteData ( Data, DataSize ) == FALSE )
  {
		/*注释 :下载文件时，会主动关闭连接  @goly 2017/07.11*/
    printf ( "[FTP EVENT]: flash compare error\r\n" );
    wip_FTPDisconnected ();
    Gprs_upgradeResultMessage ();
  }
  else
  {
		
    static s32 fileSize = 0;
    if ( fileSize <= 0 )
    {
      fileSize = wip_FTPGetFileSize ( DOTA_FtpFilename );
    }

#if TZ3000_PROTOCOL_DEBUG == 1 && __USE_DCM__ == 1
    DCM_FirmwareUpgradeStatus ( ( DOTA_FtpDataCount * 100 ) / fileSize );
#endif  /* __USE_DCM__ */
    
    
    if ( ( s32 )DOTA_FtpDataCount == fileSize )
    {

			FtpToFlash_DataCount= DOTA_FtpDataCount;


      printf ( "[FTP EVENT] finish download progress\r\n" );
			printf ( "[FTP EVENT] FtpToFlash_DataCount :%d \r\n", FtpToFlash_DataCount);

      DOTA_FtpDataCount = 0;
      vehicleParams.upgrade = UPGRADE_STATUS_IS_OK_BIT;

      wip_FTPDisconnected ();
			/*FTP升级信息无法使用TDP上报*/
			SysResetType_Upgrade();
				//Gprs_upgradeResultMessage ();//上传升级结果：预留@goly
      
			vehicleParams.DebugCtroller.debugIsValid = FALSE;
			vehicleParams.DebugCtroller.debugType &= ( ~Debug_Type_MDM );
			vehicleParams.DebugCtroller.debugType &= ( ~Debug_Type_GPS );
			vehicleParams.DebugCtroller.debugType &= ( ~Debug_Type_GSM );
			vehicleParams.DebugCtroller.debugType &= ( ~Debug_Type_SMS );
			vehicleParams.DebugCtroller.debugType &= ( ~Debug_Type_CAN );
			vehicleParams.DebugCtroller.debugType &= ( ~Debug_Type_FTP );
			STORAGE_WriteData ( VEHICLE_PARAMS, sizeof( vechile_Params_t ), ( u8 * )&vehicleParams );
      
    }  
  }
  return TRUE;
}




void wip_FTP_no_response_Startup(void)
{

	if(ftp_restart_count<2)
	{
			wip_FTPStartConnect ( 
														DOTA_FtpAddress, 
														DOTA_FtpPort, 
														DOTA_FtpUsername, 
														DOTA_FtpUserPwd, 
														( wip_eventHandler_f )ftp_eventHandler, 
														NULL );
			DOTA_FtpDataCount = 0;

	}

}

/* 
 * 功能描述：FTP应用协议解析
 * 引用参数：
 *         
 * 返回值  ：
 * 
 */
extern bool ftp_parseDataHdlr ( u16 DataSize, u8 *Data )
{
  u8 *tmp = NULL;
  /*系统状态*/
	sys_status=PROC_SYSTEM_UPDATE;
  if ( Data == NULL || DataSize == 0 )
  {
    return FALSE;
  }

  tmp = ( u8 * )strstr ( ( const char * )Data, "ftp://" );
  if ( tmp != NULL )
  {
    u8 *infoStr = NULL;
    u8 *token;

    infoStr = SHABUF_GetBuffer ( 256 );
    if ( infoStr )
    {
      memcpy ( infoStr, ( u8* )&Data[ 6 ], ( DataSize - 6 ) );
      token = ( u8 * )strtok ( ( char * )infoStr, ":" );

      if ( token != NULL )
      {
        memset ( DOTA_FtpUsername, 0, FTP_LOGIN_MAX );                
        memcpy ( DOTA_FtpUsername, token, strlen ( ( const char * )token ) );
      }
      token = NULL;
      token = ( u8 * )strtok ( NULL, "@"); 

      if ( token != NULL )
      {
        memset ( DOTA_FtpUserPwd, 0, FTP_PASSWORD_MAX );              
        memcpy ( DOTA_FtpUserPwd, token, strlen ( ( const char * )token ) );
      }
      token = NULL;
      token = ( u8 * )strtok ( NULL, ":" );
      if ( token != NULL )
      {
        memset ( DOTA_FtpAddress, 0, FTP_SERV_ADDRESS_MAX );             
        memcpy ( DOTA_FtpAddress, token, strlen ( ( const char * )token ) );
        token = NULL;
        token = ( u8 * )strtok ( NULL,":" );
        if ( token != NULL )
        {
          DOTA_FtpPort = ( u32 )atoi ( ( const char * )token );
          token = NULL;
          token = ( u8 * )strchr ( ( const char * )&Data[ 6 ], '/' );
					token = ( u8 * )strtok ((char*) token,";" );
          if ( token != NULL )
          {
            memset ( DOTA_FtpFilename, 0, FTP_FILE_PATH_NAME_MAX );                       
            memcpy ( DOTA_FtpFilename, token, ( ( u8* )&Data[ DataSize ] - token));
            token  = NULL;
//						printf("\r\n DOTA_FtpAddress:%s  \r\n",DOTA_FtpAddress);
//						printf("\r\n DOTA_FtpPort:%d     \r\n",DOTA_FtpPort);
//						printf("\r\n DOTA_FtpUsername:%s  \r\n",DOTA_FtpUsername);
//						printf("\r\n DOTA_FtpUserPwd:%s  \r\n",DOTA_FtpUserPwd);
//						printf("\r\n DOTA_FtpFilename:%s  \r\n",DOTA_FtpFilename);
            if ( TRUE == wip_FTPStartConnect ( 
                                  DOTA_FtpAddress, 
                                  DOTA_FtpPort, 
                                  DOTA_FtpUsername, 
                                  DOTA_FtpUserPwd, 
                                  ( wip_eventHandler_f )ftp_eventHandler, 
                                  NULL ) )
            {
								//STATUS_SetObjectStatus ( STATUS_FTP_IS_USING_BIT, TRUE );
								DOTA_FtpDataCount = 0;
								//vehicleParams.upgrade = UPGRADE_STATUS_IS_UPGRADING_BIT;
								//STORAGE_WriteData( VEHICLE_PARAMS, sizeof( vechile_Params_t ), ( u8 * )&vehicleParams );
				 
								printf("\r\n 准备升级  \r\n");
								
								/* 测试计时开始 */
								//FTP_PROFILE_START();
								if ( infoStr )
								{
									SHABUF_FreeBuffer ( infoStr );
									infoStr = NULL;
								}
								return TRUE;
            }
          }
        }
      }      
    }

    if ( infoStr )
    {
      SHABUF_FreeBuffer ( infoStr );
      infoStr = NULL;
    }
  }

  return FALSE;
}











/* 
 * 功能描述：FTP事件处理器
 * 引用参数：
 *         
 * 返回值  ：
 * 
 */
static void ftp_eventHandler ( wip_event_t event, void *ctx )
{
  switch (event )
  {
  	printf ( "\r\n [FTP] event: %d   \r\n",event);
    case WIP_CEV_OPEN:  /* 连接打开 */

      if ( UPGRADE_Start () == TRUE )
      {

#if TZ3000_PROTOCOL_DEBUG == 1 && __USE_DCM__ == 1
        DCM_ControlCmd ( 1, 2, 1, "[FTP] start file download progress" );
#endif  /* __USE_DCM__ */

#if TZ_USE_STORAGE_MASS_DATA == 1
{
        sysResetMassDataCompensator ();
}
#endif  /* TZ_USE_STORAGE_MASS_DATA */ 
				printf ( "\r\n [FTP] 升级:  下载文件 \r\n");
        wip_FTPDownloadFile ( DOTA_FtpFilename, ( ftp_recvDataHdlr_f )ftp_recvDataHdlr );
      }
      else
      {
				printf ( "\r\n [FTP] 升级中断\r\n");
        wip_FTPDisconnected ();
        Gprs_upgradeResultMessage ();
        //STATUS_SetObjectStatus ( STATUS_FTP_IS_USING_BIT, FALSE );
        //FTP_PROFILE_STOP ();    
      }
      break;

    case WIP_CEV_ERROR:   /* 通信错误 */
			printf ( "\r\n [FTP EVENT] WIP_CEV_ERROR......\r\n");
      Gprs_upgradeResultMessage (); 
     // STATUS_SetObjectStatus ( STATUS_FTP_IS_USING_BIT, FALSE );
      //FTP_PROFILE_STOP ();
      break;  

    case WIP_CEV_PEER_CLOSE:  /* 远端关闭连接 */
			printf ( "\r\n [FTP EVENT] WIP_CEV_PEER_CLOSE......\r\n");
      break; 
  }
}








/* 
 * 功能描述：数据校验
 * 引用参数：(1)数据
 *           (2)长度
 *         
 * 返回值  ：校验和
 * 
 */
static u8 Gprs_DataCheck ( u8 *data, u16 len )
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
 * 功能描述：复位超时与缓存
 * 引用参数：类型
 *     
 * 返回值  ：无
 * 
 */
static void Gprs_ResetTimeoutByType ( u8 type )
{
  switch ( type )
  {
    /* 登录 */
    case PROC_SYSTEM_LOGIN:
      break;
	 /* 登出 */
	case PROC_SYSTEM_LOGOUT:
      break;
    /* 休眠 */
    case PROC_SYSTEM_SLEEP:
      break;
    /* 报警 */
    case PROC_SYSTEM_ALARM:
      break;
    /* 固件更新 */
    case PROC_SYSTEM_UPDATE:
      break;
    default:
      break;
  }
}








/*
******************************************************************************
* 函数名：
* 功能：
* 创建人：gl
*
******************************************************************************
*/

static void _hTmrCAN_DiagHdlr ( u8 ID )
{
	#if 0
	static int i=0;
	if(Diag_can_flag==1)
	{
		u8 Diagnose_SendBuffer[12] = {0};
		printf("\r\n进入_hTmrCAN_Diag2");
		CAN_SendMessage ( ( u8* )Diagnose_SendBuffer, 10 );
		
	}
	#endif
}














/*信噪比进行排序(从大到小)*/
void check_snr(u8 num,u8 *src, u8 *dst)
{
    u8 i = 0,j = 0,temp = 0;

	for(j = 0; j <= 12; j++)
	{
	   //printf("s_%d ",src[j]);
       for(i = 0; i< 13 - j; i++)
       {
          if(src[i] < src[i + 1])
          {
             temp = src[i];
			 src[i] =  src[i + 1];
			 src[i + 1] = temp;
          }
       }
	   //printf("d_%d ",src[i]);
	}
	memcpy(dst, src, 13);
}




/* 
 * 功能描述：ftp升级结果上传
 * 引用参数：无
 *         
 * 返回值  ：无
 * 
 */
static void Gprs_upgradeResultMessage ( void )   
{

}


/* 
 * 功能描述：升级后系统复位
 * 引用参数：无
 *         
 * 返回值  ：无
 * 
 */
static void SysResetType_Upgrade( void )   
{
  if ( vehicleParams.upgrade & UPGRADE_STATUS_IS_OK_BIT )
  {
     vehicleParams.upgrade = 0;
     sysParamsSave ();
     UPGRADE_InstallApplication ();  
  }  

}




















