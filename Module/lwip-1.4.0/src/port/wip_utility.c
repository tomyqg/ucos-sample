



#include <stdio.h>
#include <string.h>

#include "ppp.h"
#include "wip_tcp.h"
#include "wip_utility.h"



	enum{
		FALSE = 0,
		TRUE = !FALSE
	};





typedef struct 
{
/* 定义消息缓存大小 */
#define MSG_BUFF_LENGTH     256   /* 单位：byte */
  ascii msgBuffer[ MSG_BUFF_LENGTH ];

  /* 套接字句柄 */
  s32   sessionSock;

  /* 是否调试 */
  bool  debugIsValid;  
} CMD_IfTypedef;


/******************************************/
/*              内部变量[定义]            */
/******************************************/

static CMD_IfTypedef cmd = { {0}, -1, FALSE };






/******************************************/
/*         函数原型[声明]                 */
/******************************************/

static bool send_cmd ( void );
static bool cmd_build ( ascii *buffer, u16 bufsize, ascii *Cmd, ascii *Arg );











/* 
 * 功能描述：生成命令
 * 引用参数：
 * 
 * 返回值  ：
 * 
 */
static bool cmd_build ( ascii *buffer, u16 bufsize, ascii *Cmd, ascii *Arg ) 
{
  s32 counter = 0;

  if ( buffer && bufsize > 0 && Cmd )
  {
    if ( Arg )
    {
      counter = snprintf ( ( char * )buffer, bufsize, "%s %s\r\n", Cmd, Arg );
    }
    else    
    {
      counter = snprintf ( ( char * )buffer, bufsize, "%s\r\n", Cmd );
    }  
  }
  
  return ( counter < bufsize ) ? TRUE : FALSE;
}








/* 
 * 功能描述：发送命令给服务器
 * 引用参数：
 * 
 * 返回值  ：
 * 
 */
static bool send_cmd ( void )   
{
  s32 slen = strlen ( ( const char * )cmd.msgBuffer );

  if ( cmd.sessionSock >= 0 && slen > 0 )
  {
    return wip_TCPWriteData ( cmd.sessionSock, cmd.msgBuffer, slen );
  }
  
  return FALSE;
}








/* 
 * 功能描述：命令发送
 * 引用参数：
 * 
 * 返回值  ：
 * 
 */
extern bool wip_UtilityTcpCommandSend ( ascii *Cmd, ascii *Arg )    
{
  if ( cmd_build ( cmd.msgBuffer, MSG_BUFF_LENGTH, Cmd, Arg ) == FALSE ) 
  {
    if ( cmd.debugIsValid == TRUE )
    {
    	printf ( "\r\n%s command is too long\r\n", Cmd );	
    }
  }
  else
  {
    if ( cmd.debugIsValid == TRUE )
    {
    	printf ( "\r\nSending %s command\r\n", cmd.msgBuffer );	
    }
    return send_cmd ();
  }
  return FALSE;
}






/* 
 * 功能描述：命令行工具启动
 * 引用参数：
 * 
 * 返回值  ：
 * 
 */
extern bool wip_UtilityTcpCommandStart ( s32 socket, bool debugOpt ) 
{
  if ( socket >= 0 && cmd.sessionSock == -1 )
  {
  	cmd.sessionSock = socket;
  	cmd.debugIsValid = debugOpt;
    memset ( cmd.msgBuffer, 0, MSG_BUFF_LENGTH );
  	return TRUE;
  }
  return FALSE;
}






/* 
 * 功能描述：命令行工具停止
 * 引用参数：
 * 
 * 返回值  ：
 * 
 */
extern bool wip_UtilityTcpCommandStop ( void ) 
{
  if ( cmd.sessionSock >= 0 )
  {
  	cmd.sessionSock = -1;
    cmd.debugIsValid = FALSE;
    memset ( cmd.msgBuffer, 0, MSG_BUFF_LENGTH );
  }

  return TRUE;
}








/* 
 * 功能描述：地址查询
 * 引用参数：
 * 
 * 返回值  ：
 * 
 */
extern bool wip_UtilityQueryAddress ( u8 addrType, u32 *addr )
{
  bool result = FALSE;

  if ( addr )
  {
    ip_addr_t queryaddr = {0};

    switch ( addrType )
    {
      case WIP_ADDR_TYPE_OUR_IP:
        pppIOCtl ( 0, PPPCTLG_IPADDR, &queryaddr );
        *addr = queryaddr.addr;
        result = TRUE; 
        break;

      case WIP_ADDR_TYPE_DNS1_IP:
        pppIOCtl ( 0, PPPCTLG_DNS1ADDR, &queryaddr );
        *addr = queryaddr.addr;
        result = TRUE; 
        break;

      case WIP_ADDR_TYPE_DNS2_IP:
        pppIOCtl ( 0, PPPCTLG_DNS2ADDR, &queryaddr );
        *addr = queryaddr.addr;
        result = TRUE; 
        break;

      default:
        break;
    }
  }

  return result;
}





/* 
 * 功能描述：IP地址转换接口
 * 引用参数：
 * 
 * 返回值  ：
 * 
 */
extern u32 wip_UtilityInetAddr ( const char * str )
{
  u16 num;
  u32 addr = 0;
  u8 parts = 0;
  u8 *ap;

  ap = (u8 *) & addr;
  while (parts < 4) 
  {
      if (*str < '0' || *str > '9')
          break;
      for (num = 0; num <= 255;) 
      {
        num = (num * 10) + (*str - '0');
        if (*++str < '0' || *str > '9')
            break;
      }
      if (num > 255)
          break;
      parts++;
      *ap++ = (u8) num;
      if (*str != '.') 
      {
        if (parts == 4)
            return addr;
        break;
      }
      str++;
  }
  return 0;
}


