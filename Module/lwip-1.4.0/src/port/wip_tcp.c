





#include "lwipopts.h"
#include "lwip/inet.h"
#include "lwip/tcp.h"

#include "wip_tcp.h"
#include "os_cpu.h"




/* TCP调试开关 */
#ifndef WIP_TCP_DEBUG
#define WIP_TCP_DEBUG     0
#endif  /* WIP_TCP_DEBUG */

	enum{
		FALSE = 0,
		TRUE = !FALSE
	};




typedef struct
{
  /* 是否有效? */
	bool isValid;

  /* 套接字引用 */
  struct tcp_pcb *socket;

  /* 端口 */
  u16 ipPort;

  /* 地址 */
	struct ip_addr ipAddr;

  /* 数据接收处理 */
  tcp_procDataHdlr_f   dataHandler;

  /* 创建新套接字处理 */
  tcp_newSocketHdlr_f  acceptNewSockHandler;

  /* 上层消息处理 */
  wip_eventHandler_f   eventHandler;

  /* 临时缓存 */
  struct pbuf *pbuf;

}	TCP_IfTypedef;





/******************************************/
/*              内部变量[定义]            */
/******************************************/

#define MEMP_TOTAL_NUM_TCP_PCB  ( MEMP_NUM_TCP_PCB + MEMP_NUM_TCP_PCB_LISTEN*2 )

/* 定义套接字连接资源 */
static TCP_IfTypedef tcpList[ MEMP_TOTAL_NUM_TCP_PCB ];

/*New_Add: 未做完*/
#define TOTAL_NUM_TCP_PCB_Ext  ( MEMP_NUM_TCP_PCB + MEMP_NUM_TCP_PCB_LISTEN*2 )

/* 定义套接字连接资源 */
static TCP_IfTypedef tcpListExt[ TOTAL_NUM_TCP_PCB_Ext ];
					 
/* 套接字使用数量 */
static vu8 tcpNumber = 0;
static struct tcp_pcb *wipTmpSocket = NULL;


/*
 0     No error, everything OK. 
-1     Out of memory error.     
-2     Buffer error.            
-3     Timeout.                 
-4     Routing problem.         
-5     Operation in progress    
-6     Illegal value.           
-7     Operation would block.   
-8     Connection aborted.      
-9     Connection reset.        
-10    Connection closed.       
-11    Not connected.           
*/




/******************************************/
/*              内部函数[声明]            */
/******************************************/

static err_t wip_TCPAccept ( void *arg, struct tcp_pcb *newpcb, err_t err );
static err_t wip_TCPRecvData ( void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err );
static err_t wip_TCPConnectedEvent ( void *arg, struct tcp_pcb *pcb, err_t err );
static err_t wip_TCPSent ( void *arg, struct tcp_pcb *tpcb, u16_t len );
extern u32 wip_UtilityInetAddr ( const char * str );






/* 
 * 功能描述：创建套接字事件处理器
 * 引用参数：(1)套接字句柄
 *           (2)事件处理器
 * 
 * 返回值  ：状态码
 * 
 */
extern bool wip_TCPCreateEventProcessor ( s8 sock_id, wip_eventHandler_f evHandler )
{
  if ( ( sock_id >= 0 )&&( sock_id < MEMP_TOTAL_NUM_TCP_PCB )&&( tcpList[ sock_id ].isValid == TRUE ) )
  {
    if ( evHandler )
    {

#if WIP_TCP_DEBUG == 1
      printf ( "\r\nTCP sockId %u create evHandler\r\n", sock_id );
#endif  /* WIP_TCP_DEBUG */

      tcpList[ sock_id ].eventHandler = ( wip_eventHandler_f )evHandler;
      return TRUE;    
    }
  }
  return FALSE;
}






/* 
 * 功能描述：创建服务器端
 * 引用参数：(1)端口
 *           (2)连接处理器
 *           (3)上下文
 * 
 * 返回值  ：服务器模式套接字句柄
 * 
 */
extern s8 wip_TCPServerCreate ( u16 Port, tcp_newSocketHdlr_f sockHandler, void *ctx )
{
	if ( Port && sockHandler )
  {
    if ( tcpNumber < MEMP_TOTAL_NUM_TCP_PCB )
    {
      u32 i = 0;

      for ( ; ( i < MEMP_TOTAL_NUM_TCP_PCB )&&( tcpList[ i ].isValid == TRUE ); i++ );
      if ( i < MEMP_TOTAL_NUM_TCP_PCB )
      {
        tcpList[ i ].socket = tcp_new ();
      	if ( tcpList[ i ].socket )
      	{
          s8 errorcode;

          tcpList[ i ].ipPort = Port;
          errorcode = ( s8 )tcp_bind ( tcpList[ i ].socket, IP_ADDR_ANY, tcpList[ i ].ipPort );
          if ( errorcode != ERR_OK )
          {

#if WIP_TCP_DEBUG == 1
            printf ( "\r\n***TCP CONN ERROR*** code %d\r\n", errorcode );
#endif  /* WIP_TCP_DEBUG */

            wip_TCPDestroy (i);
          }
          else
          {
            tcpList[ i ].socket = tcp_listen ( tcpList[ i ].socket );
            if ( tcpList[ i ].socket )
            {
              tcp_accept ( tcpList[ i ].socket, wip_TCPAccept );
              tcpList[ i ].isValid = TRUE;
              tcpList[ i ].acceptNewSockHandler = sockHandler;
              tcpNumber++;
              return i;       
            }
          }
      	}
      }
    }
  }

  return -1;
}








/* 
 * 功能描述：套接字接收处理
 * 引用参数：(1)用户数据
 *           (2)新协议控制块句柄
 *           (3)错误码
 * 
 * 返回值  ：错误码
 * 
 */
static err_t wip_TCPAccept ( void *arg, struct tcp_pcb *newpcb, err_t err )
{
  u32 i = 0;

  for ( ; ( i < MEMP_TOTAL_NUM_TCP_PCB )&&( tcpList[ i ].ipPort == newpcb->local_port ); i++ );
  switch ( err )
  {
    case ERR_OK:  /* No error, everything OK. */
      {
        if ( i < MEMP_TOTAL_NUM_TCP_PCB )
        {
          if ( wipTmpSocket == NULL && tcpList[ i ].acceptNewSockHandler )
          {
            wipTmpSocket = newpcb;
            tcpList[ i ].acceptNewSockHandler ( newpcb->remote_ip.addr, newpcb->remote_port );
            return err;
          }
        }

        if ( newpcb )
        {
          tcp_abort ( newpcb );
          newpcb = NULL;
        }
      }
    break;

    case ERR_MEM: /* Out of memory error. */
#if WIP_TCP_DEBUG == 1
      printf ( "\r\nOut of memory error." );
#endif  /* WIP_TCP_DEBUG */
      break;

    case ERR_BUF: /* Buffer error. */
#if WIP_TCP_DEBUG == 1
      printf ( "\r\nBuffer error." );
#endif  /* WIP_TCP_DEBUG */
      break;

    case ERR_TIMEOUT: /* Timeout. */
#if WIP_TCP_DEBUG == 1
      printf ( "\r\nTimeout." );
#endif  /* WIP_TCP_DEBUG */
      break;

    case ERR_RTE: /* Routing problem. */
#if WIP_TCP_DEBUG == 1
      printf ( "\r\nRouting problem." );
#endif  /* WIP_TCP_DEBUG */
      break;

    case ERR_ABRT:  /* Connection aborted. */
#if WIP_TCP_DEBUG == 1
      printf ( "\r\nConnection aborted." );
#endif  /* WIP_TCP_DEBUG */
      break;

    case ERR_RST: /* Connection reset. */
#if WIP_TCP_DEBUG == 1
      printf ( "\r\nConnection reset." );
#endif  /* WIP_TCP_DEBUG */
      break;

    case ERR_CLSD:  /* Connection closed. */
#if WIP_TCP_DEBUG == 1
      printf ( "\r\nConnection closed." );
#endif  /* WIP_TCP_DEBUG */
      break;

    case ERR_CONN:  /* Not connected. */
#if WIP_TCP_DEBUG == 1
      printf ( "\r\nNot connected." );
#endif  /* WIP_TCP_DEBUG */
      break;

    case ERR_VAL: /* Illegal value. */
#if WIP_TCP_DEBUG == 1
      printf ( "\r\nIllegal value." );
#endif  /* WIP_TCP_DEBUG */
      break;

    case ERR_ARG: /* Illegal argument. */
#if WIP_TCP_DEBUG == 1
      printf ( "\r\nIllegal argument." );
#endif  /* WIP_TCP_DEBUG */
      break;

    case ERR_USE: /* Address in use. */
#if WIP_TCP_DEBUG == 1
      printf ( "\r\nAddress in use." );
#endif  /* WIP_TCP_DEBUG */
      break;

    case ERR_IF:  /* Low-level netif error */
#if WIP_TCP_DEBUG == 1
      printf ( "\r\nLow-level netif error" );
#endif  /* WIP_TCP_DEBUG */
      break;

    case ERR_INPROGRESS:  /* Operation in progress */
#if WIP_TCP_DEBUG == 1
      printf ( "\r\nOperation in progress" );
#endif  /* WIP_TCP_DEBUG */
      break;

    default:
#if WIP_TCP_DEBUG == 1
      printf ( "\r\nOther error" );
#endif  /* WIP_TCP_DEBUG */
      break;
  }

  if ( err < ERR_TIMEOUT && tcpList[ i ].eventHandler )
  {
    tcpList[ i ].eventHandler ( WIP_CEV_ERROR, NULL );
  } 

  return err;
}







/* 
 * 功能描述：获取套接字选项
 * 引用参数：
 * 
 * 返回值  ：
 * 
 */
extern bool wip_TCPGetSockOption ( s8 sock_id, u32* address, u16* port )
{
  if ( ( sock_id >= 0 )&&( sock_id < MEMP_TOTAL_NUM_TCP_PCB )&&( tcpList[ sock_id ].isValid == TRUE ) )
  {
    *address = tcpList[ sock_id ].ipAddr.addr;
    *port = tcpList[ sock_id ].ipPort;
    return TRUE;
  }

  return FALSE;
}








/* 
 * 功能描述：接收新的客户套接字
 * 引用参数：
 * 
 * 返回值  ：
 * 
 */
extern s8 wip_TCPServerAcceptNewClient ( tcp_procDataHdlr_f tcpDataHandler )
{
  s32 i = 0;

  for ( ; ( i < MEMP_TOTAL_NUM_TCP_PCB )&&( tcpList[ i ].isValid == TRUE ); i++ );
  if ( i < MEMP_TOTAL_NUM_TCP_PCB && wipTmpSocket )
  {
    tcpList[ i ].socket = wipTmpSocket;
    tcpList[ i ].ipPort = wipTmpSocket->remote_port;
    tcpList[ i ].isValid = TRUE;
    tcpList[ i ].dataHandler = tcpDataHandler;
    tcpNumber++;
    tcp_recv ( tcpList[ i ].socket, wip_TCPRecvData );
    wipTmpSocket = NULL;
    return i;
  }

  if ( wipTmpSocket )
  {
    tcp_abort ( wipTmpSocket ); 
    wipTmpSocket = NULL;
  }
  
  return -1;
}



/* 
 * 功能描述：创建客户端程序
 * 引用参数：
 * 
 * 返回值  ：
 * 
 */
extern s8 wip_TCPClientCreate ( ascii* serverIP, u16 serverPort, tcp_procDataHdlr_f tcpDataHandler )
{
	OS_CPU_SR     cpu_sr;
  if ( serverIP && serverPort && tcpDataHandler )
  {
    if ( tcpNumber <MEMP_TOTAL_NUM_TCP_PCB    )
    {
      s32 i = 0;

      for ( ; ( i < MEMP_TOTAL_NUM_TCP_PCB )&&( tcpList[ i ].isValid == TRUE ); i++ );
      if ( i < MEMP_TOTAL_NUM_TCP_PCB )
      {
        tcpList[ i ].socket = tcp_new ();
      	if ( tcpList[ i ].socket )
      	{
      		s8 errorcode;
          u32 address = wip_UtilityInetAddr ( ( const char* )serverIP );

#if WIP_TCP_DEBUG == 2
          IP4_ADDR ( &tcpList[ i ].ipAddr, 202, 102, 90, 166 );
#endif  /* WIP_TCP_DEBUG */
          
          ip4_addr_set_u32 ( &tcpList[ i ].ipAddr, address );
          tcpList[ i ].ipPort = serverPort;
          tcpList[ i ].dataHandler = tcpDataHandler;
          tcpList[ i ].isValid = TRUE;
          errorcode = ( s8 )tcp_connect ( tcpList[ i ].socket, &tcpList[ i ].ipAddr, tcpList[ i ].ipPort,
                                          wip_TCPConnectedEvent );
					
          if ( errorcode != ERR_OK )
          {

#if WIP_TCP_DEBUG == 1
            printf ( "\r\n***TCP CONN ERROR*** code %d\r\n", errorcode );
#endif  /* WIP_TCP_DEBUG */
            wip_TCPDestroy (i); 
          }
          else
          {

#if WIP_TCP_DEBUG == 1
            printf ( "\r\n***TCP CONN OK***\r\n" );
#endif  /* WIP_TCP_DEBUG */
            tcpNumber++;
            return i;             
          }
      	}
      }
    }
  }

  return -1;
}







/* 
 * 功能描述：发送成功回调
 * 引用参数：
 * 
 * 返回值  ：
 * 
 */
static err_t wip_TCPSent ( void *arg, struct tcp_pcb *pcb, u16_t len )
{
  u32 i = 0;

  for ( ; ( i < MEMP_TOTAL_NUM_TCP_PCB )&&( tcpList[ i ].socket != pcb ); i++ );
  if ( i < MEMP_TOTAL_NUM_TCP_PCB && tcpList[ i ].isValid == TRUE )
  {

#if WIP_TCP_DEBUG == 1
    printf ( "\r\ntcp sockid %u send event\r\n", i );
#endif  /* WIP_TCP_DEBUG */

    if ( tcpList[ i ].eventHandler )
    {
      tcpList[ i ].eventHandler ( WIP_CEV_WRITE, NULL );
    }           
  }
  else
  {
#if WIP_TCP_DEBUG == 1
    printf ( "\r\nsocket id %u, status valid %d\r\n", i, tcpList[ i ].isValid );    
#endif  /* WIP_TCP_DEBUG */
  }

	
#if WIP_TCP_DEBUG == 1
   printf ( "\r\nNo error, everything OK.\r\n" );
#endif  /* WIP_TCP_DEBUG */

  return ERR_OK;
}






/* 
 * 功能描述：连接事件处理
 * 引用参数：(1)用户数据
 *           (2)套接字
 *           (3)错误码
 * 
 * 返回值  ：错误码
 * 
 */
static err_t wip_TCPConnectedEvent ( void *arg, struct tcp_pcb *pcb, err_t err )
{
  u32 i = 0;

  for ( ; ( i < MEMP_TOTAL_NUM_TCP_PCB )&&( tcpList[ i ].socket != pcb ); i++ );
  switch ( err )
  {
    case ERR_OK:  /* No error, everything OK. */
      {
        if ( i < MEMP_TOTAL_NUM_TCP_PCB && tcpList[ i ].isValid == TRUE )
        {

#if WIP_TCP_DEBUG == 1
          printf ( "\r\ntcp sockid %u connected event", i );
#endif  /* WIP_TCP_DEBUG */
          //tcp_nagle_disable ( tcpList[ i ].socket );
          tcp_recv ( tcpList[ i ].socket, wip_TCPRecvData );
          tcp_sent ( tcpList[ i ].socket, wip_TCPSent );
        }
        else
        {
#if WIP_TCP_DEBUG == 1
          printf ( "\r\nsocket id %u, status valid %d", i, tcpList[ i ].isValid );    
#endif  /* WIP_TCP_DEBUG */
        }

#if WIP_TCP_DEBUG == 1
         printf ( "\r\nNo error, everything OK." );
#endif  /* WIP_TCP_DEBUG */

      }
    break;

    case ERR_MEM: /* Out of memory error. */
#if WIP_TCP_DEBUG == 1
      printf ( "\r\nOut of memory error." );
#endif  /* WIP_TCP_DEBUG */
      break;

    case ERR_BUF: /* Buffer error. */
#if WIP_TCP_DEBUG == 1
      printf ( "\r\nBuffer error." );
#endif  /* WIP_TCP_DEBUG */
      break;

    case ERR_TIMEOUT: /* Timeout. */
#if WIP_TCP_DEBUG == 1
      printf ( "\r\nTimeout." );
#endif  /* WIP_TCP_DEBUG */
      break;

    case ERR_RTE: /* Routing problem. */
#if WIP_TCP_DEBUG == 1
      printf ( "\r\nRouting problem." );
#endif  /* WIP_TCP_DEBUG */
      break;

    case ERR_ABRT:  /* Connection aborted. */
#if WIP_TCP_DEBUG == 1
      printf ( "\r\nConnection aborted." );
#endif  /* WIP_TCP_DEBUG */
      break;

    case ERR_RST: /* Connection reset. */
#if WIP_TCP_DEBUG == 1
      printf ( "\r\nConnection reset." );
#endif  /* WIP_TCP_DEBUG */
      break;

    case ERR_CLSD:  /* Connection closed. */
#if WIP_TCP_DEBUG == 1
      printf ( "\r\nConnection closed." );
#endif  /* WIP_TCP_DEBUG */
      break;

    case ERR_CONN:  /* Not connected. */
#if WIP_TCP_DEBUG == 1
      printf ( "\r\nNot connected." );
#endif  /* WIP_TCP_DEBUG */
      break;

    case ERR_VAL: /* Illegal value. */
#if WIP_TCP_DEBUG == 1
      printf ( "\r\nIllegal value." );
#endif  /* WIP_TCP_DEBUG */
      break;

    case ERR_ARG: /* Illegal argument. */
#if WIP_TCP_DEBUG == 1
      printf ( "\r\nIllegal argument." );
#endif  /* WIP_TCP_DEBUG */
      break;

    case ERR_USE: /* Address in use. */
#if WIP_TCP_DEBUG == 1
      printf ( "\r\nAddress in use." );
#endif  /* WIP_TCP_DEBUG */
      break;

    case ERR_IF:  /* Low-level netif error */
#if WIP_TCP_DEBUG == 1
      printf ( "\r\nLow-level netif error" );
#endif  /* WIP_TCP_DEBUG */
      break;

    case ERR_INPROGRESS:  /* Operation in progress */
#if WIP_TCP_DEBUG == 1
      printf ( "\r\nOperation in progress" );
#endif  /* WIP_TCP_DEBUG */
      break;

    default:
#if WIP_TCP_DEBUG == 1
      printf ( "\r\nOther error" );
#endif  /* WIP_TCP_DEBUG */
      break;
  }

  if ( err < ERR_TIMEOUT && tcpList[ i ].eventHandler )
  {
    tcpList[ i ].eventHandler ( WIP_CEV_ERROR, NULL );
  } 

  return err;
}






/* 
 * 功能描述：数据接收
 * 引用参数：(1)用户数据
 *           (2)套接字
 *           (3)接收缓存
 *           (4)错误码
 * 
 * 返回值  ：错误码
 * 
 */
static err_t wip_TCPRecvData ( void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err )
{


  u32 i = 0;

  for ( ; ( i < MEMP_TOTAL_NUM_TCP_PCB )&&( pcb != tcpList[ i ].socket ); i++ );
  switch ( err )
  {
    case ERR_OK:  /* No error, everything OK. */
    {
      if ( i < MEMP_TOTAL_NUM_TCP_PCB && p != NULL ) 
      {
        struct pbuf *tmpbuf = p;
         
        tcp_recved ( pcb, p->tot_len ); 

        while ( tmpbuf != NULL )
        {
          if ( tcpList[ i ].dataHandler )
          {
						tcpList[ i ].dataHandler ( tmpbuf->len, (u8 *)tmpbuf->payload );
          }
          tmpbuf = tmpbuf->next;
        }
      }    
    }    
    break;

    case ERR_MEM: /* Out of memory error. */
#if WIP_TCP_DEBUG == 1
      printf ( "\r\nOut of memory error." );
#endif  /* WIP_TCP_DEBUG */
      break;

    case ERR_BUF: /* Buffer error. */
#if WIP_TCP_DEBUG == 1
      printf ( "\r\nBuffer error." );
#endif  /* WIP_TCP_DEBUG */
      break;

    case ERR_TIMEOUT: /* Timeout. */
#if WIP_TCP_DEBUG == 1
      printf ( "\r\nTimeout." );
#endif  /* WIP_TCP_DEBUG */
      break;

    case ERR_RTE: /* Routing problem. */
#if WIP_TCP_DEBUG == 1
      printf ( "\r\nRouting problem." );
#endif  /* WIP_TCP_DEBUG */
      break;

    case ERR_ABRT:  /* Connection aborted. */
#if WIP_TCP_DEBUG == 1
      printf ( "\r\nConnection aborted." );
#endif  /* WIP_TCP_DEBUG */
      break;

    case ERR_RST: /* Connection reset. */
#if WIP_TCP_DEBUG == 1
      printf ( "\r\nConnection reset." );
#endif  /* WIP_TCP_DEBUG */
      break;

    case ERR_CLSD:  /* Connection closed. */
#if WIP_TCP_DEBUG == 1
      printf ( "\r\nConnection closed." );
#endif  /* WIP_TCP_DEBUG */
      break;

    case ERR_CONN:  /* Not connected. */
#if WIP_TCP_DEBUG == 1
      printf ( "\r\nNot connected." );
#endif  /* WIP_TCP_DEBUG */
      break;

    case ERR_VAL: /* Illegal value. */
#if WIP_TCP_DEBUG == 1
      printf ( "\r\nIllegal value." );
#endif  /* WIP_TCP_DEBUG */
      break;

    case ERR_ARG: /* Illegal argument. */
#if WIP_TCP_DEBUG == 1
      printf ( "\r\nIllegal argument." );
#endif  /* WIP_TCP_DEBUG */
      break;

    case ERR_USE: /* Address in use. */
#if WIP_TCP_DEBUG == 1
      printf ( "\r\nAddress in use." );
#endif  /* WIP_TCP_DEBUG */
      break;

    case ERR_IF:  /* Low-level netif error */
#if WIP_TCP_DEBUG == 1
      printf ( "\r\nLow-level netif error" );
#endif  /* WIP_TCP_DEBUG */
      break;

    case ERR_INPROGRESS:  /* Operation in progress */
#if WIP_TCP_DEBUG == 1
      printf ( "\r\nOperation in progress" );
#endif  /* WIP_TCP_DEBUG */
      break;

    default:
#if WIP_TCP_DEBUG == 1
      printf ( "\r\nOther error" );
#endif  /* WIP_TCP_DEBUG */
      break;
  }

  if ( p )
  {
    pbuf_free ( p );
  }

  if ( err < ERR_TIMEOUT && tcpList[ i ].eventHandler )
  {
    tcpList[ i ].eventHandler ( WIP_CEV_ERROR, NULL );
  } 

  return err;
}
/*
*******************************************************************
*函数名：wip_TCPWriteState
*参  数：无
*功  能：查询TCP缓存是否可以写入
*返回值：true:可写入  false:不可写入
*创建人: gl
* 
*******************************************************************
*/
extern bool wip_TCPWriteState( s8 sock_id, u16 bufsize )
{
  	bool result = FALSE;
	err_t err;
	u16_t len;

  	if ( ( sock_id >= 0 )&&( sock_id < MEMP_TOTAL_NUM_TCP_PCB )&&( tcpList[ sock_id ].isValid == TRUE ) )
  	{

		//检查TCP是否可以写入
		err = tcp_write_checks(tcpList[ sock_id ].socket, bufsize);

		switch ( err )
	  {
	      case ERR_OK:  /* No error, everything OK. */
	        result = TRUE;  
	      break;
	      case ERR_MEM: /* Out of memory error. */	
	      		printf ( "\r\nOut of memory error." );
	      break;
	      default: /* Buffer error. */
	      		printf ( "\r\n other error \r\n" );
	      break;
		}
	}
	return result;
}



/* 
 * 功能描述：数据发送
 * 引用参数：(1)套接字句柄
 *           (2)发送缓存
 *           (3)发送长度
 * 
 * 返回值  ：状态码
 * 
 */
extern bool wip_TCPWriteData ( s8 sock_id, void *buffer, u16 bufsize )
{
  bool result = FALSE;
	err_t err;
	u16_t len;

  if ( ( sock_id >= 0 )&&( sock_id < MEMP_TOTAL_NUM_TCP_PCB )&&( tcpList[ sock_id ].isValid == TRUE ) )
  {
  	/* We cannot send more data than space available in the send
  	   buffer. */
  	if ( tcp_sndbuf ( tcpList[ sock_id ].socket ) < bufsize )
    {
  		len = tcp_sndbuf ( tcpList[ sock_id ].socket );
  	} else {
  		len = bufsize;
  	}
  
  	err = tcp_write ( tcpList[ sock_id ].socket, ( const void * )buffer, len, 
                      TCP_WRITE_FLAG_COPY );
  	if ( err != ERR_OK )  
    {
#if WIP_TCP_DEBUG == 1
      printf ( "\r\nsend_msgdata: error writing!\r\n" );
#endif  /* WIP_TCP_DEBUG */
  	}

    switch ( err )
    {
      case ERR_OK:  /* No error, everything OK. */
        result = TRUE;
#if WIP_TCP_DEBUG == 1
        //printf ( "\r\nNo error, everything OK." );   
#endif  /* WIP_TCP_DEBUG */
      break;
 
    case ERR_MEM: /* Out of memory error. */
#if WIP_TCP_DEBUG == 1
      printf ( "\r\nOut of memory error." );
#endif  /* WIP_TCP_DEBUG */
      break;

    case ERR_BUF: /* Buffer error. */
#if WIP_TCP_DEBUG == 1
      printf ( "\r\nBuffer error." );
#endif  /* WIP_TCP_DEBUG */
      break;

    case ERR_TIMEOUT: /* Timeout. */
#if WIP_TCP_DEBUG == 1
      printf ( "\r\nTimeout." );
#endif  /* WIP_TCP_DEBUG */
      break;

    case ERR_RTE: /* Routing problem. */
#if WIP_TCP_DEBUG == 1
      printf ( "\r\nRouting problem." );
#endif  /* WIP_TCP_DEBUG */
      break;

    case ERR_ABRT:  /* Connection aborted. */
#if WIP_TCP_DEBUG == 1
      printf ( "\r\nConnection aborted." );
#endif  /* WIP_TCP_DEBUG */
      break;

    case ERR_RST: /* Connection reset. */
#if WIP_TCP_DEBUG == 1
      printf ( "\r\nConnection reset." );
#endif  /* WIP_TCP_DEBUG */
      break;

    case ERR_CLSD:  /* Connection closed. */
#if WIP_TCP_DEBUG == 1
      printf ( "\r\nConnection closed." );
#endif  /* WIP_TCP_DEBUG */
      break;

    case ERR_CONN:  /* Not connected. */
#if WIP_TCP_DEBUG == 1
      printf ( "\r\nNot connected." );
#endif  /* WIP_TCP_DEBUG */
      break;

    case ERR_VAL: /* Illegal value. */
#if WIP_TCP_DEBUG == 1
      printf ( "\r\nIllegal value." );
#endif  /* WIP_TCP_DEBUG */
      break;

    case ERR_ARG: /* Illegal argument. */
#if WIP_TCP_DEBUG == 1
      printf ( "\r\nIllegal argument." );
#endif  /* WIP_TCP_DEBUG */
      break;

    case ERR_USE: /* Address in use. */
#if WIP_TCP_DEBUG == 1
      printf ( "\r\nAddress in use." );
#endif  /* WIP_TCP_DEBUG */
      break;

    case ERR_IF:  /* Low-level netif error */
#if WIP_TCP_DEBUG == 1
      printf ( "\r\nLow-level netif error" );
#endif  /* WIP_TCP_DEBUG */
      break;

    case ERR_INPROGRESS:  /* Operation in progress */
#if WIP_TCP_DEBUG == 1
      printf ( "\r\nOperation in progress" );
#endif  /* WIP_TCP_DEBUG */
      break;

    default:
#if WIP_TCP_DEBUG == 1
      printf ( "\r\nOther error" );
#endif  /* WIP_TCP_DEBUG */
      break;
    }

    if ( err < ERR_TIMEOUT && tcpList[ sock_id ].eventHandler )
    {
      tcpList[ sock_id ].eventHandler ( WIP_CEV_ERROR, NULL );
    }
  }

  return result;
}


/* 
 * 功能描述：TCP数据立即发送
 * 引用参数：(1)套接字句柄
 *           (2)发送缓存
 *           (3)发送长度
 * 
 * 返回值  ：状态码
 * 
 */

extern bool wip_TCPWrite( s8 sock_id, void *buffer, u16 bufsize )
{
  bool result = FALSE;
	err_t err;
	u16_t len;

  if ( ( sock_id >= 0 )&&( sock_id < MEMP_TOTAL_NUM_TCP_PCB )&&( tcpList[ sock_id ].isValid == TRUE ) )
  {
  	/* We cannot send more data than space available in the send
  	   buffer. */
  	if ( tcp_sndbuf ( tcpList[ sock_id ].socket ) < bufsize )
    {
  		return FALSE;
  	} 
    else 
    {
  		len = bufsize;
  	}
  
  	err = tcp_write ( tcpList[ sock_id ].socket, ( const void * )buffer, len, 
                      TCP_WRITE_FLAG_COPY );
  	if ( err != ERR_OK )  
    {
#if WIP_TCP_DEBUG == 1
      printf ( "\r\nsend_msgdata: error writing!\r\n" );
#endif  /* WIP_TCP_DEBUG */
  	}
    else
    if ( err == ERR_OK )
    {
      err = tcp_output ( tcpList[ sock_id ].socket );
    }

    switch ( err )
    {
      case ERR_OK:  /* No error, everything OK. */
        result = TRUE;
#if WIP_TCP_DEBUG == 1
        //printf ( "\r\nNo error, everything OK." );   
#endif  /* WIP_TCP_DEBUG */ 
      break;
 
    case ERR_MEM: /* Out of memory error. */
#if WIP_TCP_DEBUG == 1
      printf ( "\r\nOut of memory error." );
#endif  /* WIP_TCP_DEBUG */
      break;

    case ERR_BUF: /* Buffer error. */
#if WIP_TCP_DEBUG == 1
      printf ( "\r\nBuffer error." );
#endif  /* WIP_TCP_DEBUG */
      break;

    case ERR_TIMEOUT: /* Timeout. */
#if WIP_TCP_DEBUG == 1
      printf ( "\r\nTimeout." );
#endif  /* WIP_TCP_DEBUG */
      break;

    case ERR_RTE: /* Routing problem. */
#if WIP_TCP_DEBUG == 1
      printf ( "\r\nRouting problem." );
#endif  /* WIP_TCP_DEBUG */
      break;

    case ERR_ABRT:  /* Connection aborted. */
#if WIP_TCP_DEBUG == 1
      printf ( "\r\nConnection aborted." );
#endif  /* WIP_TCP_DEBUG */
      break;

    case ERR_RST: /* Connection reset. */
#if WIP_TCP_DEBUG == 1
      printf ( "\r\nConnection reset." );
#endif  /* WIP_TCP_DEBUG */
      break;

    case ERR_CLSD:  /* Connection closed. */
#if WIP_TCP_DEBUG == 1
      printf ( "\r\nConnection closed." );
#endif  /* WIP_TCP_DEBUG */
      break;

    case ERR_CONN:  /* Not connected. */
#if WIP_TCP_DEBUG == 1
      printf ( "\r\nNot connected." );
#endif  /* WIP_TCP_DEBUG */
      break;

    case ERR_VAL: /* Illegal value. */
#if WIP_TCP_DEBUG == 1
      printf ( "\r\nIllegal value." );
#endif  /* WIP_TCP_DEBUG */
      break;

    case ERR_ARG: /* Illegal argument. */
#if WIP_TCP_DEBUG == 1
      printf ( "\r\nIllegal argument." );
#endif  /* WIP_TCP_DEBUG */
      break;

    case ERR_USE: /* Address in use. */
#if WIP_TCP_DEBUG == 1
      printf ( "\r\nAddress in use." );
#endif  /* WIP_TCP_DEBUG */
      break;

    case ERR_IF:  /* Low-level netif error */
#if WIP_TCP_DEBUG == 1
      printf ( "\r\nLow-level netif error" );
#endif  /* WIP_TCP_DEBUG */
      break;

    case ERR_INPROGRESS:  /* Operation in progress */
#if WIP_TCP_DEBUG == 1
      printf ( "\r\nOperation in progress" );
#endif  /* WIP_TCP_DEBUG */
      break;

    default:
#if WIP_TCP_DEBUG == 1
      printf ( "\r\nOther error" );
#endif  /* WIP_TCP_DEBUG */
      break;
    }

    if ( err < ERR_TIMEOUT && tcpList[ sock_id ].eventHandler )
    {
      tcpList[ sock_id ].eventHandler ( WIP_CEV_ERROR, NULL );
    }
  }

  return result;
}


extern bool wip_TCPWriteDataExt( s8 sock_id, void *buffer, u16 bufsize )
{
  bool result = FALSE;
	err_t err;
	u16_t len;

  if ( ( sock_id >= 0 )&&( sock_id < MEMP_TOTAL_NUM_TCP_PCB )&&( tcpListExt[ sock_id ].isValid == TRUE ) )
  {
  	/* We cannot send more data than space available in the send
  	   buffer. */
  	if ( tcp_sndbuf ( tcpListExt[ sock_id ].socket ) < bufsize )
    {
  		return FALSE;
  	} 
    else 
    {
  		len = bufsize;
  	}
  	err = tcp_write ( tcpListExt[ sock_id ].socket, ( const void * )buffer, len, 
                      TCP_WRITE_FLAG_COPY );
  	if ( err != ERR_OK )  
    {
      printf ( "\r\nsend_msgdata**: error writing!\r\n" );

  	}
    else
    if ( err == ERR_OK )
    {
      err = tcp_output ( tcpListExt[ sock_id ].socket );
    }

    switch ( err )
    {
      case ERR_OK:  /* No error, everything OK. */
        result = TRUE;
      break;
 
    case ERR_MEM: /* Out of memory error. */
#if WIP_TCP_DEBUG == 1
      printf ( "\r\nOut of memory error." );
#endif  /* WIP_TCP_DEBUG */
      break;

    default:
#if WIP_TCP_DEBUG == 1
      printf ( "\r\nOther error" );
#endif  /* WIP_TCP_DEBUG */
      break;
    }

    if ( err < ERR_TIMEOUT && tcpListExt[ sock_id ].eventHandler )
    {
      tcpListExt[ sock_id ].eventHandler( WIP_CEV_ERROR, NULL );
    }
  }

  return result;
}





/* 
 * 功能描述：TCP静态数据发送
 * 引用参数：(1)套接字句柄
 *           (2)静态发送缓存
 *           (3)发送长度
 * 
 * 返回值  ：状态码
 * 
 */
extern bool wip_TCPWriteStaticData ( s8 sock_id, void *buffer, u16 bufsize )
{
  bool result = FALSE;
	err_t err;

  if ( ( sock_id >= 0 )&&( sock_id < MEMP_TOTAL_NUM_TCP_PCB )&&( tcpList[ sock_id ].isValid == TRUE ) )
  {
  	err = tcp_write ( tcpList[ sock_id ].socket, ( const void * )buffer, bufsize, 
                      TCP_WRITE_FLAG_MORE );
  	if ( err != ERR_OK )  
    {
#if WIP_TCP_DEBUG == 1
      printf ( "\r\nsend_msgdata: error writing!\r\n" );
#endif  /* WIP_TCP_DEBUG */
  	}

    switch ( err )
    {
      case ERR_OK:  /* No error, everything OK. */
        result = TRUE;
#if WIP_TCP_DEBUG == 1
        //printf ( "\r\nNo error, everything OK." );   
#endif  /* WIP_TCP_DEBUG */ 
      break;
 
    case ERR_MEM: /* Out of memory error. */
#if WIP_TCP_DEBUG == 1
      printf ( "\r\nOut of memory error." );
#endif  /* WIP_TCP_DEBUG */
      break;

    case ERR_BUF: /* Buffer error. */
#if WIP_TCP_DEBUG == 1
      printf ( "\r\nBuffer error." );
#endif  /* WIP_TCP_DEBUG */
      break;

    case ERR_TIMEOUT: /* Timeout. */
#if WIP_TCP_DEBUG == 1
      printf ( "\r\nTimeout." );
#endif  /* WIP_TCP_DEBUG */
      break;

    case ERR_RTE: /* Routing problem. */
#if WIP_TCP_DEBUG == 1
      printf ( "\r\nRouting problem." );
#endif  /* WIP_TCP_DEBUG */
      break;

    case ERR_ABRT:  /* Connection aborted. */
#if WIP_TCP_DEBUG == 1
      printf ( "\r\nConnection aborted." );
#endif  /* WIP_TCP_DEBUG */
      break;

    case ERR_RST: /* Connection reset. */
#if WIP_TCP_DEBUG == 1
      printf ( "\r\nConnection reset." );
#endif  /* WIP_TCP_DEBUG */
      break;

    case ERR_CLSD:  /* Connection closed. */
#if WIP_TCP_DEBUG == 1
      printf ( "\r\nConnection closed." );
#endif  /* WIP_TCP_DEBUG */
      break;

    case ERR_CONN:  /* Not connected. */
#if WIP_TCP_DEBUG == 1
      printf ( "\r\nNot connected." );
#endif  /* WIP_TCP_DEBUG */
      break;

    case ERR_VAL: /* Illegal value. */
#if WIP_TCP_DEBUG == 1
      printf ( "\r\nIllegal value." );
#endif  /* WIP_TCP_DEBUG */
      break;

    case ERR_ARG: /* Illegal argument. */
#if WIP_TCP_DEBUG == 1
      printf ( "\r\nIllegal argument." );
#endif  /* WIP_TCP_DEBUG */
      break;

    case ERR_USE: /* Address in use. */
#if WIP_TCP_DEBUG == 1
      printf ( "\r\nAddress in use." );
#endif  /* WIP_TCP_DEBUG */
      break;

    case ERR_IF:  /* Low-level netif error */
#if WIP_TCP_DEBUG == 1
      printf ( "\r\nLow-level netif error" );
#endif  /* WIP_TCP_DEBUG */
      break;

    case ERR_INPROGRESS:  /* Operation in progress */
#if WIP_TCP_DEBUG == 1
      printf ( "\r\nOperation in progress" );
#endif  /* WIP_TCP_DEBUG */
      break;

    default:
#if WIP_TCP_DEBUG == 1
      printf ( "\r\nOther error" );
#endif  /* WIP_TCP_DEBUG */
      break;
    }

    if ( err < ERR_TIMEOUT && tcpList[ sock_id ].eventHandler )
    {
      tcpList[ sock_id ].eventHandler ( WIP_CEV_ERROR, NULL );
    }
  }

  return result;
}








/* 
 * 功能描述：套接字销毁
 * 引用参数：套接字句柄
 * 
 * 返回值  ：状态码
 * 
 */
extern bool wip_TCPDestroy ( s8 sock_id )
{
  if ( ( sock_id >= 0 )&&( sock_id < MEMP_TOTAL_NUM_TCP_PCB )&&( tcpList[ sock_id ].isValid == TRUE ) )
  {
    tcp_recv  ( tcpList[ sock_id ].socket, NULL );
    tcp_arg   ( tcpList[ sock_id ].socket, NULL );
    tcp_sent  ( tcpList[ sock_id ].socket, NULL );
    tcp_poll  ( tcpList[ sock_id ].socket, NULL, 0 );
    tcp_err   ( tcpList[ sock_id ].socket, NULL );
    tcp_accept( tcpList[ sock_id ].socket, NULL ); 

    tcp_close ( tcpList[ sock_id ].socket );
    tcpList[ sock_id ].isValid = FALSE;
    tcpList[ sock_id ].socket = NULL;
    tcpList[ sock_id ].ipPort = 0;
    IP4_ADDR ( &tcpList[ sock_id ].ipAddr, 0, 0, 0, 0 );
    tcpList[ sock_id ].dataHandler = NULL;
    tcpList[ sock_id ].acceptNewSockHandler = NULL;
    tcpList[ sock_id ].eventHandler = NULL;
    tcpNumber--;
    return TRUE;
  }

  return FALSE;  
}

/* 
 * 功能描述：套接字销毁
 * 引用参数：套接字句柄
 * 
 * 返回值  ：状态码
 * 
 */
extern bool wip_TCPDestroyExt ( s8 sock_id )
{
  if ( ( sock_id >= 0 )&&( sock_id < MEMP_TOTAL_NUM_TCP_PCB )&&( tcpListExt[ sock_id ].isValid == TRUE ) )
  {
    tcp_recv  ( tcpListExt[ sock_id ].socket, NULL );
    tcp_arg   ( tcpListExt[ sock_id ].socket, NULL );
    tcp_sent  ( tcpListExt[ sock_id ].socket, NULL );
    tcp_poll  ( tcpListExt[ sock_id ].socket, NULL, 0 );
    tcp_err   ( tcpListExt[ sock_id ].socket, NULL );
    tcp_accept( tcpListExt[ sock_id ].socket, NULL ); 

    tcp_close ( tcpListExt[ sock_id ].socket );
    tcpListExt[ sock_id ].isValid = FALSE;
    tcpListExt[ sock_id ].socket = NULL;
    tcpListExt[ sock_id ].ipPort = 0;
    IP4_ADDR ( &tcpListExt[ sock_id ].ipAddr, 0, 0, 0, 0 );
    tcpListExt[ sock_id ].dataHandler = NULL;
    tcpListExt[ sock_id ].acceptNewSockHandler = NULL;
    tcpListExt[ sock_id ].eventHandler = NULL;
    tcpNumber--;
    return TRUE;
  }

  return FALSE;  
}

/*
*********************************************************************************** 
 * 函数名  : wip_TCPReadData
 * 功能描述：读取TCP数据
 * 引用参数：         
 * 返回值  ：数据长度
 * 创建人  ：gl
 **********************************************************************************
 */
extern u16 wip_TCPReadData ( s8 sock_id, void *buffer, u16 bufsize )
{
  if ( ( sock_id >= 0 )&&( sock_id < MEMP_NUM_TCP_PCB )&&( tcpList[ sock_id ].isValid == TRUE ) )
  {
  	printf("\r\n/* 读---sock_id：%d\r\n",sock_id);
    if( buffer && bufsize && tcpList[ sock_id ].pbuf )
    {
      u16 length = pbuf_copy_partial ( tcpList[ sock_id ].pbuf, buffer, bufsize, 0 );
       
      pbuf_free( tcpList[ sock_id ].pbuf );
      tcpList[ sock_id ].pbuf = NULL;
      return length; 
    }  
  }

  return 0;
}


