



#include "rexmit_strategy.h"
#include <string.h>


//////////////////////////////////////////////////////

/* 定义重传调试开关 */
#define XMIT_USE_DEBUG      0

//////////////////////////////////////////////////////

enum{
		FALSE = 0,
		TRUE = !FALSE
	};
#define NULL 0                 


typedef struct 
{
  /* 重传生效标志 */
  u8 xmitIsValid;

  /* 重传类型 */
  u8 xmitType;

#if 0 /* 删除该字段 */ 
  /* 重传应答 */
  u8 xmitAck;
#endif

  /* 重传剩余计数器 */
  s8 xmitLeftRetries;

  /* 重传数据 */
  u8 *xmitPayload;

  /* 重传数据长度 */
  u16 xmitLength;

  /* 重传剩余时间 */
  s16 xmitTimeout;

  /* 重传重载计数器 */
  u16 xmitReloadTime;
} XMIT_IfTypedef;


/* 定义并发超时数量最大 */
#define XMIT_MAX_COUNT      6
#define XMIT_BASE_ADD(x)    ((x)+0x80)
#define XMIT_BASE_SUB(x)    ((x)-0x80)
#define XMIT_IS_VALID(x)    ((x)>=0x80)





/******************************************/
/*              内部变量[定义]            */
/******************************************/

/* 历史并发重传最大计数 */
static u8 xmitMaxTO;

/* 当前并发重传计数 */
static u8 xmitCurrentTO;

/* 定义超时重传控制块 */
static XMIT_IfTypedef rexmitTo[ XMIT_MAX_COUNT ];

/* 定义重传超时处理器 */
static xmit_TimeoutHdlr_f rexmitTimeoutHandler = NULL;

/* 定义数据输出端口 */
static xmit_OutPortHdlr_f rexmitDataPort = NULL;

#define XMIT_TO_INIT      0x00  /* 待初始化 */
#define XMIT_TO_RUN       0x01  /* 运行 */

/* 定义超时重传状态 */
static u8 xmitToState = XMIT_TO_INIT;
#define XMIT_IS_READY()   ( xmitToState == XMIT_TO_RUN )

/* 定义执行错误码 */
static s8 xmitLastErrCode = XMIT_ERR_OK;







/* 
 * 功能描述：获取上次错误码
 * 引用参数：无
 *          
 * 返回值  ：状态码
 * 
 */
extern s8 XMIT_GetLastErrCode ( void )
{
  return xmitLastErrCode;
}






/* 
 * 功能描述：清空错误代码
 * 引用参数：无
 *          
 * 返回值  ：状态码
 * 
 */
extern bool XMIT_ClearLastErrCode ( void )
{
  xmitLastErrCode = XMIT_ERR_OK;
  return TRUE;
}





/* 
 * 功能描述：初始化重传机制配置数据
 * 引用参数：无
 *          
 * 返回值  ：状态码
 * 
 */
extern void XMIT_InitTimeout ( xmit_OutPortHdlr_f xmitOP )
{

#if XMIT_USE_DEBUG == 1
  printf ( "\r\n[XMIT INFO] init timeout port\r\n" );
#endif  /* XMIT_USE_DEBUG */

  if ( xmitOP && XMIT_IS_READY() == FALSE )
  {
    xmitToState = XMIT_TO_RUN;
    rexmitDataPort = xmitOP;
    xmitMaxTO = 0;
    xmitCurrentTO = 0;
    rexmitTimeoutHandler = NULL;
    xmitLastErrCode = XMIT_ERR_OK;
    memset ( rexmitTo, 0, ( XMIT_MAX_COUNT * sizeof ( XMIT_IfTypedef ) ) );
  }
}


/* 
 * 功能描述：启动超时机制
 * 引用参数：(1)重传类型
 *           (2)重传次数
 *           (3)重传间隔
 *           (4)重传数据
 *           (5)重传大小
 *   
 * 返回值  ：状态码
 * 
 */
extern s32 XMIT_StartupTimeout ( u8 type, u8 retry, s16 timeout, u8* data, u16 size )
{

#if XMIT_USE_DEBUG == 1
  printf ( "\r\n[XMIT INFO] start timeout type %4u retry %4u timeout %4ds msg 0x%X size %4u\r\n",
           type, retry, timeout, data, size );
#endif  /* XMIT_USE_DEBUG */


  if ( XMIT_IS_READY () == FALSE )
  {

#if XMIT_USE_DEBUG == 1
    printf ( "\r\n[XMIT INFO] error xmit isn't ready***\r\n" );
#endif  /* XMIT_USE_DEBUG */

    xmitLastErrCode = XMIT_ERR_INIT;
  }
  else
  if ( xmitCurrentTO >= XMIT_MAX_COUNT )
  {

#if XMIT_USE_DEBUG == 1
    printf ( "\r\n[XMIT INFO] error reach MAX xmit blks***\r\n" );
#endif  /* XMIT_USE_DEBUG */

    xmitLastErrCode = XMIT_ERR_NO_RES;
  }
  else
  if ( timeout > 0 && data && size > 0 )
  {
    u32 i = 0;
    XMIT_IfTypedef *xm = rexmitTo;

    for ( i = 0; i < XMIT_MAX_COUNT; i++ )
    {
      if ( xm->xmitIsValid == FALSE )
      {

#if XMIT_USE_DEBUG == 1
        printf ( "\r\n[XMIT INFO] get %u xmit blk\r\n", i );
#endif  /* XMIT_USE_DEBUG */

        /* 更新超时配置信息 */
        xm->xmitPayload = data;
        xm->xmitType = type;

#if 0 /* 删除该字段 */
        xm->xmitAck = 0;
#endif
        xm->xmitLeftRetries = retry;                   
        xm->xmitLength = size;
        xm->xmitTimeout = timeout;
        xm->xmitReloadTime = timeout;
        xmitCurrentTO++;
        xm->xmitIsValid = TRUE;
        return XMIT_BASE_ADD( i );
      }
      else
      {
        xm++;
      }
    }

    /* 配置参数异常! */
    if ( i == XMIT_MAX_COUNT )
    {
      xmitLastErrCode = XMIT_ERR_INTERNAL;
    }
  }
  else
  {
    xmitLastErrCode = XMIT_ERR_USER_PARAMS;
  }

  return xmitLastErrCode;
}









/* 
 * 功能描述：取消超时机制
 * 引用参数：无
 *          
 * 返回值  ：状态码
 * 
 */
extern bool XMIT_CancelTimeout ( s32 handle )
{
  if ( XMIT_IS_READY () == FALSE )
  {

#if XMIT_USE_DEBUG == 1
    printf ( "\r\n[XMIT INFO] error xmit isn't ready***\r\n" );
#endif  /* XMIT_USE_DEBUG */

    xmitLastErrCode = XMIT_ERR_INIT;
    return FALSE;
  }
  else 
  if ( XMIT_IS_VALID ( handle ) == TRUE )
  {
    u8 hdl = XMIT_BASE_SUB ( handle );
    XMIT_IfTypedef *xm = NULL;

    if ( hdl < XMIT_MAX_COUNT )
    {

#if XMIT_USE_DEBUG == 1
      printf ( "\r\n[XMIT INFO] cancel (0x%X) ok\r\n", handle );
#endif  /* XMIT_USE_DEBUG */

      xm = ( XMIT_IfTypedef * )&rexmitTo[ hdl ];
      if ( xm && xm->xmitIsValid == TRUE )
      {
        xmitCurrentTO--;
        memset ( xm, 0, sizeof ( XMIT_IfTypedef ) );
      }
      return TRUE;    
    }

#if XMIT_USE_DEBUG == 1
      printf ( "\r\n[XMIT INFO] error handler***\r\n" );
#endif  /* XMIT_USE_DEBUG */

  }

  xmitLastErrCode = XMIT_ERR_HANDLE;
  return FALSE;    
}








/* 
 * 功能描述：巡检超时机制
 * 引用参数：无
 *          
 * 返回值  ：无
 * 
 */
extern u32 XMIT_PollingTimeout ( void )
{
  u32 leftTime = 0xFFFFFF;

#if XMIT_USE_DEBUG == 6
  printf ( "\r\n[XMIT INFO] polling timeout list\r\n" );
#endif  /* XMIT_USE_DEBUG */

  if ( XMIT_IS_READY () == FALSE )
  {

#if XMIT_USE_DEBUG == 1
    printf ( "\r\n[XMIT INFO] error xmit isn't ready***\r\n" );
#endif  /* XMIT_USE_DEBUG */

    xmitLastErrCode = XMIT_ERR_INIT;
  }
  else
  if ( xmitCurrentTO > 0 )
  {
    u32 i = 0;
    XMIT_IfTypedef *xm = rexmitTo;

    if ( xmitCurrentTO > xmitMaxTO )
    {
      xmitMaxTO = xmitCurrentTO;
    }
  
    /* 执行超时巡检操作 */
    for ( i = 0; i < XMIT_MAX_COUNT; ( i++, xm++ ) )
    {
      if ( xm->xmitIsValid == FALSE )
      {
        continue;
      }
      else 
      if ( xm->xmitTimeout > 0 )
      {
        xm->xmitTimeout--;
      }

      if ( leftTime > xm->xmitTimeout )
      {
        leftTime = xm->xmitTimeout;
      }

      /* 是否达到超时间隔?? */
      if ( xm->xmitTimeout <= 0 )
      {
        if ( xm->xmitLeftRetries <= 0 )
        {

#if XMIT_USE_DEBUG == 1
          printf ( "\r\n[XMIT INFO] %u time out type %4u msg 0x%X size %4u\r\n", 
                   i, xm->xmitType, xm->xmitPayload, xm->xmitLength );
#endif  /* XMIT_USE_DEBUG */

          if ( rexmitTimeoutHandler )
          {
            rexmitTimeoutHandler ( XMIT_BASE_ADD( i ), xm->xmitType, xm->xmitPayload, xm->xmitLength );
          }
  
          /* BUG #001 若资源重新被利用，则无需主动进行资源释放操作 */
          if ( xm->xmitLeftRetries <= 0 && xm->xmitTimeout <= 0 )
          {

#if XMIT_USE_DEBUG == 1
            printf ( "\r\n[XMIT INFO] cancel (0x%X) timeout\r\n", XMIT_BASE_ADD( i ) );
#endif  /* XMIT_USE_DEBUG */

            /* 释放超时控制块 */
            XMIT_CancelTimeout ( XMIT_BASE_ADD( i ) );        
          }       
        }
        else
        {
          xm->xmitLeftRetries--;
          xm->xmitTimeout = xm->xmitReloadTime;

#if XMIT_USE_DEBUG == 1
          printf ( "\r\n[XMIT INFO] timeout blk %u data type %u out\r\n", i, xm->xmitType );
#endif  /* XMIT_USE_DEBUG */

          /* 数据输出 */
          if ( rexmitDataPort )
          {
            rexmitDataPort ( xm->xmitPayload, xm->xmitLength );
          }
        }
      } 
    }
  }

  return leftTime;
}







/* 
 * 功能描述：查询超时状态
 * 引用参数：无
 *          
 * 返回值  ：状态码
 * 
 */
extern void XMIT_QueryTimeoutState ( s32 handle )
{

#if XMIT_USE_DEBUG == 1
  printf ( "\r\n[XMIT INFO] query (0x%X) timeout state\r\n", handle );
#endif  /* XMIT_USE_DEBUG */

  if ( XMIT_IS_READY () == FALSE )
  {

#if XMIT_USE_DEBUG == 1
    printf ( "\r\n[XMIT INFO] error xmit isn't ready***\r\n" );
#endif  /* XMIT_USE_DEBUG */

    xmitLastErrCode = XMIT_ERR_INIT;
    return ;
  }
  else
  if ( XMIT_IS_VALID ( handle ) == TRUE )
  {
    u8 hdl = XMIT_BASE_SUB ( handle );

    if ( hdl < XMIT_MAX_COUNT )
    {
      XMIT_IfTypedef *xm = ( XMIT_IfTypedef * )&rexmitTo[ hdl ];
      if ( xm && xm->xmitIsValid == TRUE )
      {
        
      }    
    }
  }

  xmitLastErrCode = XMIT_ERR_HANDLE;
}








/* 
 * 功能描述：注册超时处理器
 * 引用参数：无
 *          
 * 返回值  ：状态码
 * 
 */
extern bool XMIT_SubscribeTimeoutHandler ( xmit_TimeoutHdlr_f xmitToHdlr )
{

#if XMIT_USE_DEBUG == 1
  printf ( "\r\n[XMIT INFO] set timeout handler 0x%X\r\n", xmitToHdlr );
#endif  /* XMIT_USE_DEBUG */

  if ( XMIT_IS_READY () == FALSE )
  {

#if XMIT_USE_DEBUG == 1
    printf ( "\r\n[XMIT INFO] error xmit isn't ready***\r\n" );
#endif  /* XMIT_USE_DEBUG */

    xmitLastErrCode = XMIT_ERR_INIT;
    return FALSE;
  }
  else
  if ( xmitToHdlr )
  {
    rexmitTimeoutHandler = xmitToHdlr;
    return TRUE;
  }

  return FALSE;
}




