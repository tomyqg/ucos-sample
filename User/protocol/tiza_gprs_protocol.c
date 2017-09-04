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
/* ��־�� */
/**********/

/* ��ȡSIM��ICCID */
char *RECV_SIMICCID=NULL;

/* ���ķ�������־ */
u32 centerSrvStatus = CENTER_SERVER1_LOGIN_BIT;

/* Ӧ��Э������״̬ */
sys_status_t sys_status = SYSTEM_PROC_BEGIN;





/****************/
/*   GPRS����   */
/****************/

/* GPRS���� */
extern gprs_Params_t gprsParams;

/*IMEI */
extern ascii E_IMEI[16];




/****************/
/*    FTP����   */
/****************/

/* �˿ں� */
static u16   DOTA_FtpPort;

/* �û��� */
static ascii DOTA_FtpUsername[ FTP_LOGIN_MAX ];

/* �û����� */  
static ascii DOTA_FtpUserPwd[ FTP_PASSWORD_MAX ];

/* ������������ַ */ 
static ascii DOTA_FtpAddress[ FTP_SERV_ADDRESS_MAX ]; 

/* �����ļ���ַ */
static ascii DOTA_FtpFilename[ FTP_FILE_PATH_NAME_MAX ];

/* �ļ����ݼ����� */
static u32 DOTA_FtpDataCount = 0;

/**************************************************************/\

//flash�д����BIN���ݴ�С
u32 FtpToFlash_DataCount = 0;
 


/*************************************************************************************/



/**************************************************************/



/**************************************************************/


/******************************************/
/*              �ڲ�����[����]            */
/******************************************/

static u16 AccUploadWorkingParams ( void );
static u16 ControllerDataRefresh ( void );
static u16 MultControllerDataRefresh ( void );
/* FTP���API */
static void ftp_eventHandler ( wip_event_t event, void *ctx );
static bool ftp_recvDataHdlr ( u16 DataSize, u8 *Data );

/* Э�鴦�� */
static void Gprs_upgradeResultMessage ( void );
static u8   Gprs_DataCheck ( u8 *data, u16 len );
static void SysResetType_Upgrade( void );









/******************************************/
/*              [GPRSЭ�鲿��]            */
/******************************************/


/* 
 * ����������FTP���ݽ��մ���
 * ���ò�����
 *         
 * ����ֵ  ��
 * 
 */
static bool ftp_recvDataHdlr ( u16 DataSize, u8 *Data )
{
  DOTA_FtpDataCount += DataSize;
  accInfo.close_time=0;//��ֹ�绰���Ѻ�����ʱ��������

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
		/*ע�� :�����ļ�ʱ���������ر�����  @goly 2017/07.11*/
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
			/*FTP������Ϣ�޷�ʹ��TDP�ϱ�*/
			SysResetType_Upgrade();
				//Gprs_upgradeResultMessage ();//�ϴ����������Ԥ��@goly
      
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
 * ����������FTPӦ��Э�����
 * ���ò�����
 *         
 * ����ֵ  ��
 * 
 */
extern bool ftp_parseDataHdlr ( u16 DataSize, u8 *Data )
{
  u8 *tmp = NULL;
  /*ϵͳ״̬*/
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
				 
								printf("\r\n ׼������  \r\n");
								
								/* ���Լ�ʱ��ʼ */
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
 * ����������FTP�¼�������
 * ���ò�����
 *         
 * ����ֵ  ��
 * 
 */
static void ftp_eventHandler ( wip_event_t event, void *ctx )
{
  switch (event )
  {
  	printf ( "\r\n [FTP] event: %d   \r\n",event);
    case WIP_CEV_OPEN:  /* ���Ӵ� */

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
				printf ( "\r\n [FTP] ����:  �����ļ� \r\n");
        wip_FTPDownloadFile ( DOTA_FtpFilename, ( ftp_recvDataHdlr_f )ftp_recvDataHdlr );
      }
      else
      {
				printf ( "\r\n [FTP] �����ж�\r\n");
        wip_FTPDisconnected ();
        Gprs_upgradeResultMessage ();
        //STATUS_SetObjectStatus ( STATUS_FTP_IS_USING_BIT, FALSE );
        //FTP_PROFILE_STOP ();    
      }
      break;

    case WIP_CEV_ERROR:   /* ͨ�Ŵ��� */
			printf ( "\r\n [FTP EVENT] WIP_CEV_ERROR......\r\n");
      Gprs_upgradeResultMessage (); 
     // STATUS_SetObjectStatus ( STATUS_FTP_IS_USING_BIT, FALSE );
      //FTP_PROFILE_STOP ();
      break;  

    case WIP_CEV_PEER_CLOSE:  /* Զ�˹ر����� */
			printf ( "\r\n [FTP EVENT] WIP_CEV_PEER_CLOSE......\r\n");
      break; 
  }
}








/* 
 * ��������������У��
 * ���ò�����(1)����
 *           (2)����
 *         
 * ����ֵ  ��У���
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
 * ������������λ��ʱ�뻺��
 * ���ò���������
 *     
 * ����ֵ  ����
 * 
 */
static void Gprs_ResetTimeoutByType ( u8 type )
{
  switch ( type )
  {
    /* ��¼ */
    case PROC_SYSTEM_LOGIN:
      break;
	 /* �ǳ� */
	case PROC_SYSTEM_LOGOUT:
      break;
    /* ���� */
    case PROC_SYSTEM_SLEEP:
      break;
    /* ���� */
    case PROC_SYSTEM_ALARM:
      break;
    /* �̼����� */
    case PROC_SYSTEM_UPDATE:
      break;
    default:
      break;
  }
}








/*
******************************************************************************
* ��������
* ���ܣ�
* �����ˣ�gl
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
		printf("\r\n����_hTmrCAN_Diag2");
		CAN_SendMessage ( ( u8* )Diagnose_SendBuffer, 10 );
		
	}
	#endif
}














/*����Ƚ�������(�Ӵ�С)*/
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
 * ����������ftp��������ϴ�
 * ���ò�������
 *         
 * ����ֵ  ����
 * 
 */
static void Gprs_upgradeResultMessage ( void )   
{

}


/* 
 * ����������������ϵͳ��λ
 * ���ò�������
 *         
 * ����ֵ  ����
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




















