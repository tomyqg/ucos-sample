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
/*              内部变量[定义]            */
/******************************************/

/* 连接IP地址 */
 ascii mylink_address[ 16 ] ="";

/* 连接IP端口 */
 u16   mylink_port;

/* 连接TCP套接字 */
 s8  serversocket = -1;
 
 /* 重连接计数 */
static u8 reConnectCnts = 0;


/* GPRS启动定时器 */
 tmr_t *xmit_start_tmr = NULL;  

/* 调试原始数据显示区 */
#if TZ3000_PROTOCOL_DEBUG == 1
extern ascii rawDataSource[ 768 ] = {0};  
#endif  /* TIMER_1S_TRIGGER_DEBUG */

///*全局变量声明*/
////1：服务器2启用
//extern s8  TCP2_Server=-1;


/******************************************/
/*              内部函数[声明]            */
/******************************************/

static void XMIT_CfgSetting ( void );
extern void XMIT_Connect ( void );
static void XMIT_EventHandler ( wip_event_t ev, void *ctx );
bool Gprs_TimeoutHdlr ( s32 handle, u8 type, u8 *data, u16 size );


///*外部变量*/
//extern status_param drv_param;



/* 
 * 功能描述：启动传输管理器
 * 引用参数：无
 *          
 * 返回值  ：无
 * 
 */
extern void XMIT_StartManager ( void )
{
  /* 启动传输超时管理机制 */
//  XMIT_InitTimeout ( XMIT_DataOutput );
//  XMIT_SubscribeTimeoutHandler ( Gprs_TimeoutHdlr );
}

/* 
 * 功能描述：TCP数据输出
 * 引用参数：(1)数据
 *           (2)长度
 *
 * 返回值  ：状态码
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
*函数名：wip_TCPWriteCheck
*参  数：无
*功  能：TCP可写入状态 
*创建人: gl
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
 * 功能描述：数据输出
 * 引用参数：(1)数据
 *           (2)长度
 *
 * 返回值  ：状态码
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
 * 功能描述：传输通道建立
 * 引用参数：ID
 *         
 * 返回值  ：无
 * 
 */
extern void XMIT_Startup ( u8 ID )
{

  if (xmit_start_tmr )
  {
    TMR_UnSubscribe ( xmit_start_tmr, ( tmr_procTriggerHdlr_t )XMIT_Startup, TMR_TYPE_100MS );
    xmit_start_tmr = NULL;
  }

  /* 创建数据连接 */
  XMIT_Connect ();
}



/* 
 * 功能描述：数据连接
 * 引用参数：无
 *         
 * 返回值  ：无
 * 
 */
extern void XMIT_Connect ( void ) 
{

  if ( serversocket == -1 )
  {
  	/* 获取连接配置 */
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
 * 功能描述：传输事件管理器
 * 引用参数：(1)事件
 *           (2)用户上下文
 *
 * 返回值  ：无
 * 
 */
static void XMIT_EventHandler ( wip_event_t event, void *ctx )
{
  switch (event)
  {
    case WIP_CEV_OPEN:  /* 打开事件 */
    {

    }
      break;
    case WIP_CEV_READ:  /* 读事件 */
    {
//				#define COMMON_RX_BUFFER_LENGTH  500
//        u8 msgRxBuffer[COMMON_RX_BUFFER_LENGTH];
//				u32 length = wip_TCPReadData ( serversocket, msgRxBuffer, COMMON_RX_BUFFER_LENGTH );

//        Gprs_ProtocolDataDecode ( length ,msgRxBuffer);

      break;
    }
    case WIP_CEV_WRITE:   /* 写事件 */
      break;  
  }
}







/* 
 * 功能描述：关闭数据连接
 * 引用参数：无
 *         
 * 返回值  ：无
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
 * 功能描述：连接配置
 * 引用参数：无
 *         
 * 返回值  ：无
 * 
 */
static void XMIT_CfgSetting ( void )
{
		XMIT_Stop ();

		/* 是否需要使用上一次登陆的服务器配置??? */
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

  /* 设置登录状态 */
  sys_status = PROC_SYSTEM_LOGIN;
}








/* 
 * 功能描述：传输重连接
 * 引用参数：无
 *         
 * 返回值  ：无
 * 
 */
extern void XMIT_ConnectOnceAgain ( void )
{
  reConnectCnts++;
	/*切换服务器  尝试登陆*/
  centerSrvStatus &= (~CENTER_SERVERX_IS_LOGIN_BIT);

#if TCP_CONNECT_DEBUG == 1
{
  PRINTF_GPS_TIME ();
  printf ( "\r\n[LOGIN EVENT]: reconnect count %u\r\n", reConnectCnts );
}
#endif  /* UDP_CONNECT_DEBUG */

  if ( reConnectCnts % 5 != 0 ) 
  {
    /* 创建数据连接 */
    XMIT_Connect ();                                                    
  }
  else
  {

////////////////////////////////////////////////////////////////////////////////
   
/*******************************/
/*     需要考虑进步的操作      */
/*******************************/

	/* 改进流程参考：
	 * (1)延时X分钟后重新建立数据连接
	 * (2)连续X分钟均无法与中心建立连接，则复位模块；
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
				/* 延时5分钟 */
				TMR_Subscribe ( FALSE, 3000, TMR_TYPE_100MS, ( tmr_procTriggerHdlr_t )XMIT_Startup );  		
			}
  }	
}

/* 
 * 功能描述：超时处理
 * 引用参数：无
 *         
 * 返回值  ：无
 * 
 */
bool Gprs_TimeoutHdlr ( s32 handle, u8 type, u8 *data, u16 size )
{

  if ( handle > 0 && data && size > 0 )
  {
    switch ( type )
    {
      /* 登录 */
      case PROC_SYSTEM_LOGIN:

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
    return TRUE;
  }

  return FALSE;
}

