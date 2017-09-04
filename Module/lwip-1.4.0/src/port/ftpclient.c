





#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#include "wip_utility.h"
#include "wip_tcp.h"
#include "ftpclient.h"
#include "tiza_params.h"
#include "tiza_gprs_protocol.h"







/* 调试控制器 */
//extern sys_debug_t sysDebugCtroller;



typedef struct
{
  /* 服务器IP */
  ascii serverAddress[ FTP_SERV_ADDRESS_MAX ];

  /* 端口 */
  u16 serverPort;

  /* 用户名 */
  ascii *userName;

  /* 用户密码 */
  ascii *userPass;

  /* 文件名 */
  ascii *fileName;

  /* 文件大小 */
  s32 fileSize;

  /* 数据接收处理 */
  ftp_recvDataHdlr_f  rcvDataHdlr;

  /* 事件处理 */
  wip_eventHandler_f  procHandler;

  /* 事件信号 */
  wip_event_t ev;
} FTP_IfTypedef;






/******************************************/
/*              内部变量[定义]            */
/******************************************/

/* 数据套接字 */
static s8 ftpdatasock = -1; 

/* 命令套接字 */   
static s8 ftpctrlsock = -1;

/* FTP客户端 */
static FTP_IfTypedef ftpclient = { "xxxx", 21, NULL, NULL, NULL, 0, NULL, NULL, WIP_CEV_DONE };
   

/* FTP数据响应控制 */   
u32 ftp_restart_count = 0;





/******************************************/
/*         函数原型[声明]                 */
/******************************************/

static bool wip_FTPCtrlHandler ( u16 DataSize, u8 * Data );
static void wip_FTPSetStatus ( void );













/* 
 * 功能描述：
 * 引用参数：
 * 
 * 返回值  ：
 * 
 */
static void wip_FTPSetStatus ( void )
{
  if ( ftpclient.procHandler )
  {
    ftpclient.procHandler ( ftpclient.ev, NULL );
  }
}


extern bool wip_FTPDownload_no_response (void) 
{
	TMR_Subscribe ( FALSE, 100, TMR_TYPE_100MS, ( tmr_procTriggerHdlr_t )wip_FTP_no_response_Startup );
}


/* 
 * 功能描述：FTP操作入口
 * 引用参数：
 * 
 * 返回值  ：
 * 
 */
extern bool wip_FTPStartConnect ( ascii* ftpSevrAddress, u16 ftpSevrPort, 
                                  ascii* userName, 
                                  ascii* userPass,
                                  wip_eventHandler_f evHandler, void *ctx ) 
{
  if ( ftpctrlsock >= 0 )
  {
    wip_FTPDisconnected ();
  }
	else
	{
		wip_FTPDownload_no_response();
	}
  
  if ((ftpSevrAddress != NULL) && (userName != NULL) && (userPass != NULL) && (evHandler != NULL))
  {
			ftpclient.serverPort = ftpSevrPort; 
			ftpclient.procHandler = evHandler;    
			
			strncpy ( ( char * )ftpclient.serverAddress, ( const char* )ftpSevrAddress, FTP_SERV_ADDRESS_MAX );
			ftpclient.userName = userName;
			ftpclient.userPass = userPass;   
		
			if ( ftpctrlsock == -1 )
			{
		 
					ftpctrlsock = wip_TCPClientCreate ( ftpclient.serverAddress, ftpclient.serverPort, ( tcp_procDataHdlr_f )wip_FTPCtrlHandler );
					
					if ( ftpctrlsock >= 0 )
					{
							
							if ( ( vehicleParams.DebugCtroller.debugType & Debug_Type_FTP ) && vehicleParams.DebugCtroller.debugIsValid == TRUE )
							{
									printf("\r\n[FTP EVENT] TCPClientCreate OK ,debugIsValid TRUE \r\n");
									return wip_UtilityTcpCommandStart ( ftpctrlsock, TRUE );
							} 
							else
							{
									printf("\r\n[FTP EVENT] TCPClientCreate OK  \r\n");
									return wip_UtilityTcpCommandStart ( ftpctrlsock, FALSE );
							}
					}   
			}
  }
  printf("\r\n FTP连接失败  \r\n");
  return FALSE;
}







/* 
 * 功能描述：
 * 引用参数：
 * 
 * 返回值  ：
 * 
 */
extern bool wip_FTPDownloadFile ( ascii* file, ftp_recvDataHdlr_f ftpRcvDataHdlr )  
{
  if ( file && ftpRcvDataHdlr &&( ftpclient.ev == WIP_CEV_OPEN ) )
  {
    ftpclient.rcvDataHdlr = ftpRcvDataHdlr;
    ftpclient.fileName = file;
    ftpclient.fileSize = 0;
    
    wip_UtilityTcpCommandSend ( "SIZE", ftpclient.fileName );
    return TRUE;
  }

  return FALSE;
}








/* 
 * 功能描述：
 * 引用参数：
 * 
 * 返回值  ：
 * 
 */
extern s32 wip_FTPGetFileSize  ( ascii* file )
{
  if ( file && strcmp ( ( const char *)ftpclient.fileName, ( const char * )file ) == 0 )
  {
    return ftpclient.fileSize;
  }

  return -1;
}



void Tmr_FTP_226_CloseHandler(void)
{

		ftpclient.ev = WIP_CEV_PEER_CLOSE;
		printf ( "\r\n[FTP EVENT]: ***226" );

}

/* 
 * 功能描述：FTP命令通道
 * 引用参数：
 * 
 * 返回值  ：
 * 
 */
static bool wip_FTPCtrlHandler ( u16 DataSize, u8 *Data )    
{
  char* token = NULL;
  OS_CPU_SR     cpu_sr;
	
////	USART_WriteData ( SHELL_USART, Data, DataSize );//@goly
//////////////////////////////////////////////////
//  if ( vehicleParams.DebugCtroller.debugIsValid == TRUE && ( vehicleParams.DebugCtroller.debugType & Debug_Type_FTP )  )
//  {
//    USART_WriteData ( vehicleParams.DebugCtroller.debugPort, Data, DataSize );
//  }
//////////////////////////////////////////////////
	ftp_restart_count++;
  if ( DataSize > 1 )
	{	
    /* BUG-FIX #002 增加空格搜索，增加可靠性 */
		if ( ( token = strstr ( ( const char* )Data, "220 " ) ) != NULL && token == ( char* )Data )
		{
      wip_UtilityTcpCommandSend ( "USER", ftpclient.userName );  
    }
    else
		if ( ( token = strstr ( ( const char* )Data, "332 " ) ) != NULL && token == ( char* )Data )
		{
      wip_UtilityTcpCommandSend ( "USER", ftpclient.userName ); 
    }
    else
		if ( ( token = strstr ( ( const char* )Data, "331 " ) ) != NULL && token == ( char* )Data )
		{
      wip_UtilityTcpCommandSend ( "PASS", ftpclient.userPass ); 
    }
    else
		if ( ( token = strstr ( ( const char* )Data, "230 " ) ) != NULL && token == ( char* )Data )
		{
      wip_UtilityTcpCommandSend ( "TYPE", "I" );  
    }
    else
		if ( ( token = strstr ( ( const char* )Data, "200 " ) ) != NULL && token == ( char* )Data )
		{
      if ( ftpclient.ev < WIP_CEV_OPEN )
      {
        
				OS_ENTER_CRITICAL();
				ftpclient.ev = WIP_CEV_OPEN;
        wip_FTPSetStatus ();  
				OS_EXIT_CRITICAL();
      }
    }

    /* BUG-FIX #001 出现相同的213代码时重复执行流程错误 */
    else
		if ( ( token = strstr ( ( const char* )Data, "213 " ) ) != NULL && token == ( char* )Data )
		{
      ascii *pfileSize = ( ascii* )token + 4;
       
      if ( pfileSize > 0 )
      {
        s32 length = atoi ( ( const char * )pfileSize );

        if ( length > 0 )
        {
          ftpclient.fileSize = ( u32 )length;

          printf ( "\r\nfile name %s size %u bytes\r\n", ftpclient.fileName, ftpclient.fileSize );
          wip_UtilityTcpCommandSend ( "PASV", NULL );
        }
      }
      else
      {
        ftpclient.ev = WIP_CEV_ERROR;
        wip_FTPDisconnected ();        
      }
    }
    else
		if ( ( token = strstr ( ( const char* )Data, "227 " ) ) != NULL && token == ( char* )Data )
		{
      ascii ipAddress[ 16 ] = "\0";
      u16 ipPort = 0;
      ascii * token = ( ascii * )strtok ( ( char * )Data, "(" );

      /* MOD #001 增加数据套接字句柄检查 */
      if ( token && ftpdatasock == -1 )
      {
        token = ( ascii * )strtok ( NULL, "," );
        if ( token )
        {
          strcat ( ( char * )ipAddress, ( const char * )token );
          strcat ( ( char * )ipAddress, "." );
          token = ( ascii * )strtok ( NULL, "," );
          if ( token )
          {
            strcat ( ( char * )ipAddress, ( const char * )token );
            strcat ( ( char * )ipAddress, "." );
            token = ( ascii * )strtok ( NULL, "," );
            if ( token )
            {
              strcat ( ( char * )ipAddress, ( const char * )token );
              strcat ( ( char * )ipAddress, "." );
              token = ( ascii * )strtok ( NULL, "," );
              if ( token )
              {
                strcat ( ( char * )ipAddress, ( const char * )token );
                token = ( ascii * )strtok ( NULL, "," );
                if ( token )
                {
                  ipPort = 256 * atoi ( ( const char * )token );
                  token = ( ascii * )strtok ( NULL, ")" );
                  if ( token )
                  {
                    ipPort += atoi ( ( const char * )token );
										
										printf("\r\n ipAddress:%s ipPort: %d \r\n",ipAddress ,ipPort);
                    ftpdatasock = wip_TCPClientCreate ( ipAddress, ipPort, 
                                                        ( tcp_procDataHdlr_f )ftpclient.rcvDataHdlr );
										printf("\r\n ftpdatasock: %d \r\n",ftpdatasock);
                    if ( ftpdatasock >= 0 )
                    {
                      wip_UtilityTcpCommandSend ( "RETR", ftpclient.fileName );
                      return TRUE; 
                    }
                  } 
                }  
              } 
            }
          }
        }  
      }
    }
    else
		if ( ( token = strstr ( ( const char* )Data, "226 " ) ) != NULL && token == ( char* )Data )
		{
			
			//TMR_Subscribe ( FALSE, 10, TMR_TYPE_100MS, ( tmr_procTriggerHdlr_t )Tmr_FTP_226_CloseHandler );
      ftpclient.ev = WIP_CEV_PEER_CLOSE;
      printf ( "\r\n***226\r\n" );
    }
//    else
//		if ( strstr ( ( const char* )Data, "530" ) )
//		{
//      ftpclient.ev = WIP_CEV_ERROR;
//
//      printf ( "\r\n***530\r\n" );
//      wip_FTPDisconnected ();
//    }
    else
		if ( ( token = strstr ( ( const char* )Data, "221 " ) ) != NULL && token == ( char* )Data )
		{
      ftpclient.ev = WIP_CEV_ERROR;

      printf ( "\r\n***221" );
      wip_FTPDisconnected ();
    }
    else
		if ( ( token = strstr ( ( const char* )Data, "421 " ) ) != NULL && token == ( char* )Data )
		{
      ftpclient.ev = WIP_CEV_ERROR;

      printf ( "\r\n***421" );
      wip_FTPDisconnected ();
    }
    else
		if ( ( token = strstr ( ( const char* )Data, "425 " ) ) != NULL && token == ( char* )Data )
		{
      ftpclient.ev = WIP_CEV_ERROR;
      printf ( "\r\n***425" );
      wip_FTPDisconnected ();
    }
    else
		if ( ( token = strstr ( ( const char* )Data, "426 " ) ) != NULL && token == ( char* )Data )
		{
      ftpclient.ev = WIP_CEV_ERROR;

      printf ( "\r\n***426" );
      wip_FTPDisconnected ();
    }
    else
		if ( ( token = strstr ( ( const char* )Data, "450 " ) ) != NULL && token == ( char* )Data )
		{
      ftpclient.ev = WIP_CEV_ERROR;

      printf ( "\r\n***450" );
      wip_FTPDisconnected ();
    }
    else
		if ( ( token = strstr ( ( const char* )Data, "451 " ) ) != NULL && token == ( char* )Data )
		{
      ftpclient.ev = WIP_CEV_ERROR;

      printf ( "\r\n***451" );
      wip_FTPDisconnected ();
    }
    else
		if ( ( token = strstr ( ( const char* )Data, "503 " ) ) != NULL && token == ( char* )Data )
		{
      ftpclient.ev = WIP_CEV_ERROR;

      printf ( "\r\n***503" );
      wip_FTPDisconnected ();
    }
    else
		if ( ( token = strstr ( ( const char* )Data, "550 " ) ) != NULL && token == ( char* )Data )
		{
      ftpclient.ev = WIP_CEV_ERROR;

      printf ( "\r\n***550" );
      wip_FTPDisconnected ();
    }
	}  
	
  return TRUE;
}









/* 
 * 功能描述：完成任务后退出
 * 引用参数：
 * 
 * 返回值  ：
 * 
 */
extern void wip_FTPDisconnected ( void ) 
{
  if ( ftpdatasock >= 0 )
  {
    wip_TCPDestroy ( ftpdatasock );
    ftpdatasock = -1;
  }

  wip_UtilityTcpCommandSend ( "QUIT", NULL );
  wip_UtilityTcpCommandStop ();

  if ( ftpclient.ev > WIP_CEV_OPEN )
  {
    wip_FTPSetStatus ();
  }

  if ( ftpctrlsock >= 0 )
  {
    wip_TCPDestroy ( ftpctrlsock );
    ftpctrlsock = -1;
  }

  ftpclient.ev = WIP_CEV_DONE;
  ftpclient.fileName = NULL;
  ftpclient.userName = NULL;
  ftpclient.userPass = NULL;
  ftpclient.fileSize = 0;
  ftpclient.rcvDataHdlr = NULL;
  memset ( ( char * )ftpclient.serverAddress, 0, FTP_SERV_ADDRESS_MAX );
}




