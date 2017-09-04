


#include "os_worker.h"
#include "worker.h"



/////////////////////////////////////////////////////////////////

/* 定义工作队列调试开关 */
#define USE_WORKER_DBG      0

/////////////////////////////////////////////////////////////////





/* 定义工作队列的最大数量 */
#define WORKER_MAX_NBR    2

typedef struct
{
  /* 工作状态 */
  u8 stat;

  /* 工作索引句柄 */
  worker_t *workHdlr;
} WQ_IfTypedef;

/* 定义工作队列起始地址 */
#define WQ_BASE_VALUE     0x30
#define WQ_ADD_BASE(x)    ((x)+WQ_BASE_VALUE)
#define WQ_SUB_BASE(x)    ((x)-WQ_BASE_VALUE)
#define WK_IS_VALID(x)    ((x)>=WQ_BASE_VALUE)







/******************************************/
/*              内部变量[定义]            */
/******************************************/

/* 定义工作队列 */
static WQ_IfTypedef workQueue[ WORKER_MAX_NBR ]; 

/* 定义工作队列使用数量 */
static u8 wqUsedNumber = 0;

/* 定义上层错误码 */
static s8 wqLastError = WORK_ERR_OK;

#define WQ_UNINIT       0x00
#define WQ_INITED       0x01

/* 定义工作状态  */
static u8 wqInitStat = WQ_UNINIT;
#define WQ_IS_VALID     ( wqInitStat != WQ_UNINIT )

/* 定义索引起始地址 */
static u8 wqAllocaIdx = 0;







/* 
 * 功能描述：低级任务接口初始化
 * 引用参数：无
 *          
 * 返回值  ：状态码
 * 
 */
extern void WORKER_LowLevelIfInit ( void )
{

#if USE_WORKER_DBG == 1
  printf ( "\r\n[WORK EVENT] low level if init..\r\n" );
#endif  /* USE_WORKER_DBG */

  wqUsedNumber = 0;
  wqLastError = WORK_ERR_OK;
  wqAllocaIdx = 0;
  wqInitStat  = WQ_INITED;
  memset ( workQueue, 0, ( sizeof ( WQ_IfTypedef ) * WORKER_MAX_NBR ) );
  WORKER_OSIfInit ();
}







/* 
 * 功能描述：任务执行
 * 引用参数：无
 *          
 * 返回值  ：状态码
 * 
 */
extern bool WORKER_DoWorks ( void )
{
  if ( wqUsedNumber > 0 )
  {
    u32 i = 0;
    WQ_IfTypedef *wq = workQueue;
    for ( i = 0; i < WORKER_MAX_NBR; i++ )
    {
      if ( ( wq + i )->workHdlr )
      {
#if OS_CRITICAL_METHOD == 3                           /* Allocate storage for CPU status register      */
        OS_CPU_SR  cpu_sr = 0;
#endif

#if USE_WORKER_DBG == 1
        printf ( "\r\n[WORK EVENT] exec work Id %u name 0x%X stat %u do-handler 0x%X, data 0x%X\r\n",
                 i, 
                 ( wq + i )->workHdlr, 
                 ( wq + i )->stat, 
                 ( wq + i )->workHdlr->func, 
                 ( wq + i )->workHdlr->data );
#endif  /* USE_WORKER_DBG */

        switch ( ( wq + i )->stat )
        {
          case WORK_BUSY_PENDING: /* 挂起 */
            {
              OS_ENTER_CRITICAL ();
              ( wq + i )->stat = WORK_BUSY_RUNNING;
              OS_EXIT_CRITICAL ();
            }
          case WORK_BUSY_RUNNING: /* 运行 */
            {
              /* 任务函数执行 */
              if ( ( wq + i )->workHdlr->func )
              {
                ( wq + i )->workHdlr->func ( ( wq + i )->workHdlr->data );
              }

              OS_ENTER_CRITICAL ();
              ( wq + i )->stat = WORK_NO_EXIST;

              /* BUG-FIX #000 修正清空的对象 */
              ( wq + i )->workHdlr = NULL;
              if ( wqUsedNumber > 0 )
              {
                wqUsedNumber--;
              }
              
              if ( wqUsedNumber == 0 )
              {
                wqAllocaIdx = 0;
              }
              OS_EXIT_CRITICAL ();            
            }
            break;

          case WORK_BUSY_DYING: /* 将死 */
          case WORK_NO_EXIST: /* 销毁 */
          default:
            {
              OS_ENTER_CRITICAL ();
              ( wq + i )->stat = WORK_NO_EXIST;

              /* BUG-FIX #001 修正清空的对象 */
              ( wq + i )->workHdlr = NULL;
              if ( wqUsedNumber > 0 )
              {
                wqUsedNumber--;
              }

              if ( wqUsedNumber == 0 )
              {
                wqAllocaIdx = 0;
              }
              OS_EXIT_CRITICAL ();
            }
            break;
        }
      }
    }
    return TRUE;
  }

  return FALSE;
}







/* 
 * 功能描述：启动工作任务
 * 引用参数：工作对象
 *          
 * 返回值  ：状态码
 * 
 */
extern s16 WORKER_StartupWork ( worker_t *appWorker )
{

#if USE_WORKER_DBG == 1
  printf ( "\r\n[WORK EVENT] start worker 0x%X\r\n", appWorker );
#endif  /* USE_WORKER_DBG */

  if ( WQ_IS_VALID == FALSE )
  {

#if USE_WORKER_DBG == 1
    printf ( "\r\n[WORK EVENT] workqueue is err uninit***\r\n" );
#endif  /* USE_WORKER_DBG */

    wqLastError = WORK_ERR_UNINIT; 
  }
  else
  if ( appWorker && appWorker->func )
  {
    if ( wqUsedNumber < WORKER_MAX_NBR )
    {
      u32 i = 0;
      WQ_IfTypedef *wq = workQueue;

      if ( wqUsedNumber > 0 )
      {
        for ( i = 0; i < WORKER_MAX_NBR; i++ )
        {
          /* 不支持工作函数重复载入! */
          if ( ( wq + i )->stat > WORK_NO_EXIST && 
               ( wq + i )->workHdlr == appWorker )
          {

#if USE_WORKER_DBG == 1
            printf ( "\r\n[WORK EVENT] err work has started!!****\r\n" );
#endif  /* USE_WORKER_DBG */

            wqLastError = WORK_ERR_ALREADY;
            return wqLastError;
          }
        }
      }

__ALLOCA_WORKER__:
      {
        bool research = FALSE;

        for ( i = wqAllocaIdx; i < WORKER_MAX_NBR; i++ )
        {
          if ( ( wq + i )->stat == WORK_NO_EXIST 
            && ( wq + i )->workHdlr == NULL )
          {

#if OS_CRITICAL_METHOD == 3                           /* Allocate storage for CPU status register      */
            OS_CPU_SR  cpu_sr = 0;
#endif

#if USE_WORKER_DBG == 1
            printf ( "\r\n[WORK EVENT] alloca work Id %u name 0x%X do-handler 0x%X\r\n", 
                     i, appWorker, appWorker->func );
#endif  /* USE_WORKER_DBG */

            OS_ENTER_CRITICAL ();
            ( wq + i )->stat = WORK_BUSY_PENDING;
            ( wq + i )->workHdlr = appWorker;
            wqUsedNumber++;
            wqAllocaIdx = i;
            OS_EXIT_CRITICAL ();

            /* 激活工作队列守护任务 */
            WORKER_SemPost ();
            return WQ_ADD_BASE( i );
          }
        }

        if ( i == WORKER_MAX_NBR && research == FALSE )
        {
          i = 0;
          research = TRUE;
          goto __ALLOCA_WORKER__;
        }
      }

#if USE_WORKER_DBG == 1
      printf ( "\r\n[WORK EVENT] FATAL ERR workqueue internal***\r\n" );
#endif  /* USE_WORKER_DBG */
      wqLastError = WORK_ERR_INTERNAL;
    }
    else
    {

#if USE_WORKER_DBG == 1
      printf ( "\r\n[WORK EVENT] err no resource***\r\n" );
#endif  /* USE_WORKER_DBG */
      wqLastError = WORK_ERR_NO_RES;
    }
  }
  else
  {

#if USE_WORKER_DBG == 1
    printf ( "\r\n[WORK EVENT] err user params***\r\n" );
#endif  /* USE_WORKER_DBG */
    wqLastError = WORK_ERR_USR_PARAMS;
  }

  return wqLastError;
}









/* 
 * 功能描述：停止工作任务
 * 引用参数：工作句柄
 *          
 * 返回值  ：状态码
 * 
 */
extern bool WORKER_StopWork ( s16 workhandler )
{

#if USE_WORKER_DBG == 1
  printf ( "\r\n[WORK EVENT] stop worker handler %d\r\n", workhandler );
#endif  /* USE_WORKER_DBG */

  if ( WQ_IS_VALID == FALSE )
  {

#if USE_WORKER_DBG == 1
    printf ( "\r\n[WORK EVENT] workqueue is err uninit***\r\n" );
#endif  /* USE_WORKER_DBG */

    wqLastError = WORK_ERR_UNINIT;
  }
  else
  if ( wqUsedNumber > 0 )
  {
    if ( WK_IS_VALID ( workhandler ) == TRUE )
    {
      u32 i = WQ_SUB_BASE( workhandler );
      WQ_IfTypedef *wq = workQueue;

      if ( i >= WORKER_MAX_NBR )
      {
        wqLastError = WORK_ERR_USR_PARAMS;
        return FALSE;
      }
      else
      if ( ( wq + i )->workHdlr && ( wq + i )->stat != WORK_NO_EXIST )
      {
#if OS_CRITICAL_METHOD == 3                           /* Allocate storage for CPU status register      */
        OS_CPU_SR  cpu_sr = 0;
#endif

        OS_ENTER_CRITICAL ();
        if ( ( wq + i )->stat == WORK_BUSY_RUNNING )
        {

#if USE_WORKER_DBG == 3
          printf ( "\r\n[WORK EVENT] set work Id %u stat WORK_BUSY_DYING\r\n", i );
#endif  /* USE_WORKER_DBG */

          ( wq + i )->stat = WORK_BUSY_DYING;
        }
        else
        {

#if USE_WORKER_DBG == 1
          printf ( "\r\n[WORK EVENT] set work Id %u stat WORK_NO_EXIST\r\n", i );
#endif  /* USE_WORKER_DBG */

          ( wq + i )->workHdlr = NULL;
          ( wq + i )->stat = WORK_NO_EXIST;
          if ( wqUsedNumber > 0 )
          {
            wqUsedNumber--;
          }

          if ( wqUsedNumber == 0 )
          {
            wqAllocaIdx = 0;
          }
        }
        OS_EXIT_CRITICAL ();
        return TRUE;
      }
      else
      {

#if USE_WORKER_DBG == 1
        printf ( "\r\n[WORK EVENT] FATAL ERR workqueue internal***\r\n" );
#endif  /* USE_WORKER_DBG */

        wqLastError = WORK_ERR_INTERNAL;
      }
    }
    else
    {

#if USE_WORKER_DBG == 1
      printf ( "\r\n[WORK EVENT] err user handler***\r\n" );
#endif  /* USE_WORKER_DBG */

      wqLastError = WORK_ERR_HANDLE;
    }
  }
  else
  {

#if USE_WORKER_DBG == 1
    printf ( "\r\n[WORK EVENT] err user params***\r\n" );
#endif  /* USE_WORKER_DBG */

    wqLastError = WORK_ERR_USR_PARAMS;
  }

  return FALSE;
}









/* 
 * 功能描述：获取工作状态
 * 引用参数：工作句柄
 *          
 * 返回值  ：状态码
 * 
 */
extern u32 WORKER_GetWorkStat ( s16 workhandler )
{

#if USE_WORKER_DBG == 1
  printf ( "\r\n[WORK EVENT] get worker handler %d\r\n", workhandler );
#endif  /* USE_WORKER_DBG */

  if ( WQ_IS_VALID == FALSE )
  {

#if USE_WORKER_DBG == 1
    printf ( "\r\n[WORK EVENT] workqueue is err uninit***\r\n" );
#endif  /* USE_WORKER_DBG */
    wqLastError = WORK_ERR_UNINIT; 
  }
  else
  if ( wqUsedNumber > 0 )
  {
    if ( WK_IS_VALID ( workhandler ) == TRUE  )
    {
      u32 i = WQ_SUB_BASE( workhandler );
      WQ_IfTypedef *wq = workQueue;

      if ( i < WORKER_MAX_NBR && ( wq + i )->workHdlr )
      {
        return ( wq + i )->stat;
      }
    }
  }

  return WORK_NO_EXIST;
}







/* 
 * 功能描述：获取上次错误码
 * 引用参数：无
 *          
 * 返回值  ：状态码
 * 
 */
extern void WORKER_ClearLastErrorCode ( void )
{
  wqLastError = WORK_ERR_OK;
}





/* 
 * 功能描述：清空错误代码
 * 引用参数：无
 *          
 * 返回值  ：状态码
 * 
 */
extern s8 WORKER_GetLastErrorCode ( void )
{
  return wqLastError;
}



