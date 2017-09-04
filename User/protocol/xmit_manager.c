#include <stdio.h>
#include <string.h>
#include "xmit_manager.h"
#include "wip_udp.h"
#include "wip_tcp.h"
#include "stdbool.h"
#include "rexmit_strategy.h"
#include "tiza_params.h"
#include "tiza_gprs_protocol.h"
#include "gb_gprs_protocol.h"
#include "tiza_utility.h"
#include "modem.h"

/******************************************/
/*              �ڲ�����[����]            */
/******************************************/

/* ����IP��ַ */
 ascii mylink_address[ 16 ] ="";

/* ����IP�˿� */
 u16   mylink_port;

/* ����TCP�׽��� */
 s8  serversocket = -1;
 
 /* �����Ӽ��� */
static u8 reConnectCnts = 0;


/* GPRS������ʱ�� */
 tmr_t *xmit_start_tmr = NULL;  

/* ����ԭʼ������ʾ�� */
#if TZ3000_PROTOCOL_DEBUG == 1
extern ascii rawDataSource[ 768 ] = {0};  
#endif  /* TIMER_1S_TRIGGER_DEBUG */

///*ȫ�ֱ�������*/
////1��������2����
//extern s8  TCP2_Server=-1;


/******************************************/
/*              �ڲ�����[����]            */
/******************************************/

static void XMIT_CfgSetting ( void );
extern void XMIT_Connect ( void );
static void XMIT_EventHandler ( wip_event_t ev, void *ctx );
bool Gprs_TimeoutHdlr ( s32 handle, u8 type, u8 *data, u16 size );


///*�ⲿ����*/
//extern status_param drv_param;



/* 
 * �����������������������
 * ���ò�������
 *          
 * ����ֵ  ����
 * 
 */
extern void XMIT_StartManager ( void )
{
  /* �������䳬ʱ������� */
//  XMIT_InitTimeout ( XMIT_DataOutput );
//  XMIT_SubscribeTimeoutHandler ( Gprs_TimeoutHdlr );
}

/* 
 * ����������TCP�������
 * ���ò�����(1)����
 *           (2)����
 *
 * ����ֵ  ��״̬��
 * 
 */
 

extern bool TCP_DataOutput ( u8 *data, u16 size )
{ 

  if ( serversocket >= 0 && data && size > 0 )
  {
#if  0
{
    ascii rawDataSource[ 768 ];
    MDM_Bytes2String ( ( u8* )rawDataSource, ( const u8* )data, size );
    printf ( "\r\n[TX LEGTH]:%d   [TX DATA] %s\r\n", size,rawDataSource );
}
#endif  

		return wip_TCPWrite( serversocket, data, size );
  }
  return FALSE;
}



/*
*******************************************************************
*��������wip_TCPWriteCheck
*��  ������
*��  �ܣ�TCP��д��״̬ 
*������: gl
* 
*******************************************************************
*/
extern bool wip_TCPWriteCheck (u16 size )
{ 
	if ( serversocket >= 0&& size > 0 )
	{
	return wip_TCPWriteState ( serversocket, size );
	}
	return FALSE;
}



/* 
 * �����������������
 * ���ò�����(1)����
 *           (2)����
 *
 * ����ֵ  ��״̬��
 * 
 */
extern bool XMIT_DataOutput ( u8 *data, u16 size )
{
  if ( serversocket >= 0 && data && size > 0 )
  {

  	return wip_TCPWrite( serversocket, data, size );
  }

  return FALSE;
}








/* 
 * ��������������ͨ������
 * ���ò�����ID
 *         
 * ����ֵ  ����
 * 
 */
extern void XMIT_Startup ( u8 ID )
{

  if (xmit_start_tmr )
  {
    TMR_UnSubscribe ( xmit_start_tmr, ( tmr_procTriggerHdlr_t )XMIT_Startup, TMR_TYPE_100MS );
    xmit_start_tmr = NULL;
  }

  /* ������������ */
  XMIT_Connect ();
}



/* 
 * ������������������
 * ���ò�������
 *         
 * ����ֵ  ����
 * 
 */
extern void XMIT_Connect ( void ) 
{

  if ( serversocket == -1 )
  {
  	/* ��ȡ�������� */
  	XMIT_CfgSetting();
	
		serversocket = wip_TCPClientCreate ( mylink_address, mylink_port, ( tcp_procDataHdlr_f )Gprs_ProtocolDataDecode );

		if(serversocket >=0)
		{
				printf("\r\nGSM_RI :%d \r\n",gsmring_io_handler);
				printf("\r\n[TCP EVENT]: Server %d  login  success ! IP: %s Port: %d\r\n",serversocket,mylink_address,mylink_port);
				login(); 	
				IsDeadZone_Network=-1;
		} 
		else
		{
				printf("\r\n[TCP EVENT] Server %d  login  failed ! IP: %s Port: %d\r\n",serversocket,mylink_address,mylink_port);
		}

#if TCP_CONNECT_DEBUG == 1
{
    if ( serversocket >= 0 )
    {
      PRINTF_GPS_TIME ();
      
    }
    else
    {
      PRINTF_GPS_TIME ();
      printf ( "\r\n[TCP EVENT] socket create err***\r\n" );
    }
}
#endif  /* TCP_CONNECT_DEBUG */



#if UDP_CONNECT_DEBUG == 1
{
    if ( serversocket >= 0 )
    {
      PRINTF_GPS_TIME ();
      printf ( "\r\n[UDP EVENT] socket okay\r\n" );
    }
    else
    {
      PRINTF_GPS_TIME ();
      printf ( "\r\n[UDP EVENT] socket create err***\r\n" );
    }
}
#endif  /* UDP_CONNECT_DEBUG */

  }
}







/* 
 * ���������������¼�������
 * ���ò�����(1)�¼�
 *           (2)�û�������
 *
 * ����ֵ  ����
 * 
 */
static void XMIT_EventHandler ( wip_event_t event, void *ctx )
{
  switch (event)
  {
    case WIP_CEV_OPEN:  /* ���¼� */
    {

    }
      break;
    case WIP_CEV_READ:  /* ���¼� */
    {
//				#define COMMON_RX_BUFFER_LENGTH  500
//        u8 msgRxBuffer[COMMON_RX_BUFFER_LENGTH];
//				u32 length = wip_TCPReadData ( serversocket, msgRxBuffer, COMMON_RX_BUFFER_LENGTH );

//        Gprs_ProtocolDataDecode ( length ,msgRxBuffer);

      break;
    }
    case WIP_CEV_WRITE:   /* д�¼� */
      break;  
  }
}







/* 
 * �����������ر���������
 * ���ò�������
 *         
 * ����ֵ  ����
 * 
 */
extern void XMIT_Stop ( void )	
{

  if ( serversocket >= 0 )
  {
			if(wip_TCPDestroy ( serversocket ))
			{
				serversocket = -1;
			}
			printf("\r\nXMIT_Stop IS OK ......\r\n");
  }
}



/* 
 * ������������������
 * ���ò�������
 *         
 * ����ֵ  ����
 * 
 */
static void XMIT_CfgSetting ( void )
{
		XMIT_Stop ();

		/* �Ƿ���Ҫʹ����һ�ε�½�ķ���������??? */
		if ( ( centerSrvStatus & CENTER_SERVERX_IS_LOGIN_BIT ) == 0 )
		{
				if ( ( centerSrvStatus & CENTER_SERVER2_LOGIN_BIT ) == 0 )				
				{
					strncpy ( ( char * )mylink_address, ( const char * )gprsParams.IP1, 16 );
					mylink_port = gprsParams.Port1;
					centerSrvStatus |= CENTER_SERVER2_LOGIN_BIT;					         
				}
				else    
				{ 
					strncpy ( ( char * )mylink_address, ( const char * )gprsParams.IP2, 16 );		 
					mylink_port = gprsParams.Port2;
					//centerSrvStatus &= ( ~CENTER_SERVER2_LOGIN_BIT );
					centerSrvStatus=CENTER_SERVERX_IS_LOGIN_BIT;
				}
		}

  /* ���õ�¼״̬ */
  sys_status = PROC_SYSTEM_LOGIN;
}








/* 
 * ��������������������
 * ���ò�������
 *         
 * ����ֵ  ����
 * 
 */
extern void XMIT_ConnectOnceAgain ( void )
{
  reConnectCnts++;
	/*�л�������  ���Ե�½*/
  centerSrvStatus &= (~CENTER_SERVERX_IS_LOGIN_BIT);

#if TCP_CONNECT_DEBUG == 1
{
  PRINTF_GPS_TIME ();
  printf ( "\r\n[LOGIN EVENT]: reconnect count %u\r\n", reConnectCnts );
}
#endif  /* UDP_CONNECT_DEBUG */

  if ( reConnectCnts % 5 != 0 ) 
  {
    /* ������������ */
    XMIT_Connect ();                                                    
  }
  else
  {

////////////////////////////////////////////////////////////////////////////////
   
/*******************************/
/*     ��Ҫ���ǽ����Ĳ���      */
/*******************************/

	/* �Ľ����̲ο���
	 * (1)��ʱX���Ӻ����½�����������
	 * (2)����X���Ӿ��޷������Ľ������ӣ���λģ�飻
	 * (3)....
	 * */
			if ( reConnectCnts >= 2 )//10
			{
					sysParamsSave ();
		
					#if SYSTEM_RESTART_DEBUG == 1
					{
									PRINTF_GPS_TIME ();
									printf ( "\r\n[SYS RESET]: ConnectOnceAgain type\r\n" );
					}
					#endif  /* SYSTEM_RESTART_DEBUG */
					System_Reset ();		
			}
			else
			{
				/* ��ʱ5���� */
				TMR_Subscribe ( FALSE, 3000, TMR_TYPE_100MS, ( tmr_procTriggerHdlr_t )XMIT_Startup );  		
			}
  }	
}

/* 
 * ������������ʱ����
 * ���ò�������
 *         
 * ����ֵ  ����
 * 
 */
bool Gprs_TimeoutHdlr ( s32 handle, u8 type, u8 *data, u16 size )
{

  if ( handle > 0 && data && size > 0 )
  {
    switch ( type )
    {
      /* ��¼ */
      case PROC_SYSTEM_LOGIN:

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
    return TRUE;
  }

  return FALSE;
}

