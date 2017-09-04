





#include "tiza_timer_port.h"
#include "os_timer.h"
#include "tiza_timer.h"

#define TMR_INFO_DEBUG      0










/* 定义定时器数量 */
#ifndef TMR_ID_COUNT
#define TMR_ID_COUNT    12
#endif  /* TMR_ID_COUNT */


typedef struct 
{
  /* 定时器数量 */
  vu8 tmr_nbr;

  /* 周期性标志 */
  bool tmr_cyclic[ TMR_ID_COUNT ];

  /* 定时器计数器 */
  vs32 tmr_count[ TMR_ID_COUNT ];

  /* 定时器资源 */
  tmr_t tmr_alloc[ TMR_ID_COUNT ];
} TMR_IfTypedef;







/******************************************/
/*              内部变量[定义]            */
/******************************************/

/* 定义定时器配置 */
static TMR_IfTypedef Timer = { 0, {FALSE}, {0}, {0} };

/* 定义1ms定时器数量 */
static vu8 Tmr1msCounter = 0;

/* 定义10ms定时器数量 */
static vu8 Tmr10msCounter = 0;

/* 定义100ms定时器数量 */
static vu8 Tmr100msCounter = 0;




/* 
 * 功能描述：定时器接口初始化
 * 引用参数：无
 *          
 * 返回值  ：状态码
 * 
 */
extern void TMR_LowLevelIfInit ( void )
{

  
	//硬件定时器初始化
	TMR_OSIfInit ();
	OpenTimer(10);//硬件定时器;基时间：10ms
  Tmr10msCounter = 0;
  Tmr100msCounter = 0;
  memset ( ( u8* )&Timer, 0, sizeof ( TMR_IfTypedef ) );
}








/* 
 * 功能描述：定时器接口初始化
 * 引用参数：(1)是否循环定时器
 *           (2)定时间隔
 *           (3)定时器类型
 *           (4)定时器响应处理机
 *          
 * 返回值  ：定时器句柄
 * 
 */
extern tmr_t *TMR_Subscribe ( bool cyclic, u32 TimerValue, u8 TimerType, tmr_procTriggerHdlr_t Timerhdlr )
{


#if OS_CRITICAL_METHOD == 3                           /* Allocate storage for CPU status register      */
    OS_CPU_SR  cpu_sr = 0;
#endif

  switch ( TimerType )
  {
    case TMR_TYPE_1MS:
      if ( Timerhdlr && ( TimerValue > 0 ) )
      {
        if ( Timer.tmr_nbr < TMR_ID_COUNT )
        {
          u32 i;

          for ( i = 0; i < TMR_ID_COUNT; i++ )
          {
            if ( Timer.tmr_alloc[ i ].TimerId == 0 )
            {
              OS_ENTER_CRITICAL ();
              Timer.tmr_cyclic[ i ] = cyclic;
              Timer.tmr_alloc[ i ].TimerId    = ++Timer.tmr_nbr;
              Timer.tmr_alloc[ i ].TimerType  = TMR_TYPE_1MS;
              Timer.tmr_alloc[ i ].TimerValue = TimerValue;
              Timer.tmr_alloc[ i ].TimerHdlr  = ( tmr_procTriggerHdlr_t )Timerhdlr;
              Timer.tmr_count[ i ] = Timer.tmr_alloc[ i ].TimerValue;
              Tmr1msCounter++;
              OS_EXIT_CRITICAL ();

#if TMR_INFO_DEBUG == 1
              printf ( "\r\n[TMR EVENT] id %u cyclic %s, time %u (100ms), total tmr %u\r\n", 
                       Timer.tmr_alloc[ i ].TimerId, ( cyclic ? "TRUE" : "FALSE" ), Timer.tmr_alloc[ i ].TimerValue, Timer.tmr_nbr );
#endif  /* TMR_INFO_DEBUG */

              return ( tmr_t * )&Timer.tmr_alloc[ i ];            
            }
          }
        }

#if TMR_INFO_DEBUG == 1
        else
        {
          printf ( "\r\n[TMR EVENT] err no timer resource***\r\n" );
        }
#endif  /* TMR_INFO_DEBUG */
 
      }

#if TMR_INFO_DEBUG == 1
      else
      {
        printf ( "\r\n[TMR EVENT] err TimerValue or Timerhdlr***\r\n" );
      }
#endif  /* TMR_INFO_DEBUG */
      break;
    case TMR_TYPE_100MS:
      if ( Timerhdlr && ( TimerValue > 0 ) )
      {
        if ( Timer.tmr_nbr < TMR_ID_COUNT )
        {
          u32 i;

          for ( i = 0; i < TMR_ID_COUNT; i++ )
          {
            if ( Timer.tmr_alloc[ i ].TimerId == 0 )
            {
              OS_ENTER_CRITICAL ();
              Timer.tmr_cyclic[ i ] = cyclic;
              Timer.tmr_alloc[ i ].TimerId    = ++Timer.tmr_nbr;
              Timer.tmr_alloc[ i ].TimerType  = TMR_TYPE_100MS;
              Timer.tmr_alloc[ i ].TimerValue = TimerValue;
              Timer.tmr_alloc[ i ].TimerHdlr  = ( tmr_procTriggerHdlr_t )Timerhdlr;
              Timer.tmr_count[ i ] = Timer.tmr_alloc[ i ].TimerValue;
              Tmr100msCounter++;
              OS_EXIT_CRITICAL ();

#if TMR_INFO_DEBUG == 1
              printf ( "\r\n[TMR EVENT] id %u cyclic %s, time %u (100ms), total tmr %u\r\n", 
                       Timer.tmr_alloc[ i ].TimerId, ( cyclic ? "TRUE" : "FALSE" ), Timer.tmr_alloc[ i ].TimerValue, Timer.tmr_nbr );
#endif  /* TMR_INFO_DEBUG */

              return ( tmr_t * )&Timer.tmr_alloc[ i ];            
            }
          }
        }

#if TMR_INFO_DEBUG == 1
        else
        {
          printf ( "\r\n[TMR EVENT] err no timer resource***\r\n" );
        }
#endif  /* TMR_INFO_DEBUG */
 
      }

#if TMR_INFO_DEBUG == 1
      else
      {
        printf ( "\r\n[TMR EVENT] err TimerValue or Timerhdlr***\r\n" );
      }
#endif  /* TMR_INFO_DEBUG */
      break;

    case TMR_TYPE_10MS:
      if ( Timerhdlr && ( TimerValue > 0 ) )
      {
        if ( Timer.tmr_nbr < TMR_ID_COUNT )
        {
          u32 i;

          for ( i = 0; i < TMR_ID_COUNT; i++ )
          {
            if ( Timer.tmr_alloc[ i ].TimerId == 0 )
            {
              OS_ENTER_CRITICAL ();
              Timer.tmr_cyclic[ i ] = cyclic;
              Timer.tmr_alloc[ i ].TimerId    = ++Timer.tmr_nbr;
              Timer.tmr_alloc[ i ].TimerType  = TMR_TYPE_10MS;
              Timer.tmr_alloc[ i ].TimerValue = TimerValue;
              Timer.tmr_alloc[ i ].TimerHdlr  = ( tmr_procTriggerHdlr_t )Timerhdlr;
              Timer.tmr_count[ i ] = Timer.tmr_alloc[ i ].TimerValue;
              Tmr10msCounter++;
              OS_EXIT_CRITICAL ();

#if TMR_INFO_DEBUG == 1
              printf ( "\r\n[TMR EVENT] id %u cyclic %s, time %u (10ms), total tmr %u\r\n", 
                       Timer.tmr_alloc[ i ].TimerId, ( cyclic ? "TRUE" : "FALSE" ), Timer.tmr_alloc[ i ].TimerValue, Timer.tmr_nbr );
#endif  /* TMR_INFO_DEBUG */

              return ( tmr_t * )&Timer.tmr_alloc[ i ];            
            }
          }
        }

#if TMR_INFO_DEBUG == 1
        else
        {
          printf ( "\r\n[TMR EVENT] err no timer resource***\r\n" );
        }
#endif  /* TMR_INFO_DEBUG */
 
      }

#if TMR_INFO_DEBUG == 1
      else
      {
        printf ( "\r\n[TMR EVENT] err TimerValue or Timerhdlr***\r\n" );
      }
#endif  /* TMR_INFO_DEBUG */
      break;

    default:  break;
  }

  return NULL;
}







/* 
 * 功能描述：定时器注销接口
 * 引用参数：(1)定时器句柄
 *           (2)定时器响应处理机
 *           (3)定时器类型
 *          
 * 返回值  ：状态码
 * 
 */
extern bool TMR_UnSubscribe ( tmr_t *tmr, tmr_procTriggerHdlr_t Timerhdlr, u8 TimerType )
{

#if OS_CRITICAL_METHOD == 3                           /* Allocate storage for CPU status register      */
    OS_CPU_SR  cpu_sr = 0;
#endif

  switch ( TimerType )
  {
  	case TMR_TYPE_1MS:
      if ( tmr && Timerhdlr )
      {
        u32 i;

        for ( i = 0; ( i < TMR_ID_COUNT ) && ( Timer.tmr_nbr > 0 ) && ( Tmr1msCounter > 0 ); i++ )
        {
          if ( ( Timer.tmr_alloc[ i ].TimerHdlr == ( tmr_procTriggerHdlr_t )Timerhdlr ) 
            && ( Timer.tmr_alloc[ i ].TimerType == TMR_TYPE_1MS )
            && ( ( tmr_t * )&Timer.tmr_alloc[ i ] == tmr )
            && ( Timer.tmr_alloc[ i ].TimerId > 0 ) )
          {
            OS_ENTER_CRITICAL ();
            Timer.tmr_nbr--;
            Timer.tmr_cyclic[ i ] = FALSE;
            Timer.tmr_alloc[ i ].TimerId    = 0;
            Timer.tmr_alloc[ i ].TimerType  = TMR_TYPE_1MS;
            Timer.tmr_alloc[ i ].TimerValue = 0;
            Timer.tmr_alloc[ i ].TimerHdlr  = ( tmr_procTriggerHdlr_t )NULL;
            Timer.tmr_count[ i ] = 0;
            Tmr1msCounter--;
            OS_EXIT_CRITICAL ();

#if TMR_INFO_DEBUG == 1
            printf ( "\r\n[TMR EVENT] tmr left %u, released %u (100ms)\r\n", Timer.tmr_nbr, i );
#endif  /* TMR_INFO_DEBUG */

            return TRUE;            
          }
        }

#if TMR_INFO_DEBUG == 1
        {
          printf ( "\r\n[TMR EVENT] tmr left %u, no tmr released\r\n", Timer.tmr_nbr );        
        }
#endif  /* TMR_INFO_DEBUG */

      } 

#if TMR_INFO_DEBUG == 1
      else
      {
        printf ( "\r\n[TMR EVENT] err tmr released***\r\n" );      
      }
#endif  /* TMR_INFO_DEBUG */
      break;
    case TMR_TYPE_100MS:
      if ( tmr && Timerhdlr )
      {
        u32 i;

        for ( i = 0; ( i < TMR_ID_COUNT ) && ( Timer.tmr_nbr > 0 ) && ( Tmr100msCounter > 0 ); i++ )
        {
          if ( ( Timer.tmr_alloc[ i ].TimerHdlr == ( tmr_procTriggerHdlr_t )Timerhdlr ) 
            && ( Timer.tmr_alloc[ i ].TimerType == TMR_TYPE_100MS )
            && ( ( tmr_t * )&Timer.tmr_alloc[ i ] == tmr )
            && ( Timer.tmr_alloc[ i ].TimerId > 0 ) )
          {
            OS_ENTER_CRITICAL ();
            Timer.tmr_nbr--;
            Timer.tmr_cyclic[ i ] = FALSE;
            Timer.tmr_alloc[ i ].TimerId    = 0;
            Timer.tmr_alloc[ i ].TimerType  = TMR_TYPE_100MS;
            Timer.tmr_alloc[ i ].TimerValue = 0;
            Timer.tmr_alloc[ i ].TimerHdlr  = ( tmr_procTriggerHdlr_t )NULL;
            Timer.tmr_count[ i ] = 0;
            Tmr100msCounter--;
            OS_EXIT_CRITICAL ();

#if TMR_INFO_DEBUG == 1
            printf ( "\r\n[TMR EVENT] tmr left %u, released %u (100ms)\r\n", Timer.tmr_nbr, i );
#endif  /* TMR_INFO_DEBUG */

            return TRUE;            
          }
        }

#if TMR_INFO_DEBUG == 1
        {
          printf ( "\r\n[TMR EVENT] tmr left %u, no tmr released\r\n", Timer.tmr_nbr );        
        }
#endif  /* TMR_INFO_DEBUG */

      } 

#if TMR_INFO_DEBUG == 1
      else
      {
        printf ( "\r\n[TMR EVENT] err tmr released***\r\n" );      
      }
#endif  /* TMR_INFO_DEBUG */
      break;

    case TMR_TYPE_10MS:
      if ( tmr && Timerhdlr )
      {
        u32 i;

        for ( i = 0; ( i < TMR_ID_COUNT ) && ( Timer.tmr_nbr > 0 ) && ( Tmr10msCounter > 0 ); i++ )
        {
          if ( ( Timer.tmr_alloc[ i ].TimerHdlr == ( tmr_procTriggerHdlr_t )Timerhdlr )
            && ( Timer.tmr_alloc[ i ].TimerType == TMR_TYPE_10MS )
            && ( ( tmr_t * )&Timer.tmr_alloc[ i ] == tmr )
            && ( Timer.tmr_alloc[ i ].TimerId > 0 ) )
          {
            OS_ENTER_CRITICAL ();
            Timer.tmr_nbr--;
            Timer.tmr_cyclic[ i ] = FALSE;
            Timer.tmr_alloc[ i ].TimerId    = 0;
            Timer.tmr_alloc[ i ].TimerType  = TMR_TYPE_10MS;
            Timer.tmr_alloc[ i ].TimerValue = 0;
            Timer.tmr_alloc[ i ].TimerHdlr  = ( tmr_procTriggerHdlr_t )NULL;
            Timer.tmr_count[ i ] = 0;
            Tmr10msCounter--;
            OS_EXIT_CRITICAL ();

#if TMR_INFO_DEBUG == 1
            printf ( "\r\n[TMR EVENT] tmr left %u, released %u (10ms)\r\n", Timer.tmr_nbr, i );
#endif  /* TMR_INFO_DEBUG */

            return TRUE;            
          }
        }

#if TMR_INFO_DEBUG == 1
        {
          printf ( "\r\n[TMR EVENT] tmr left %u, no tmr released\r\n", Timer.tmr_nbr );        
        }
#endif  /* TMR_INFO_DEBUG */

      } 

#if TMR_INFO_DEBUG == 1
      else
      {
        printf ( "\r\n[TMR EVENT] err tmr released***\r\n" );      
      }
#endif  /* TMR_INFO_DEBUG */
      break;

    default:  break;
  }

  return FALSE;
}






/* 
 * 功能描述：定时器更新1ms处理
 * 引用参数：无
 *          
 * 返回值  ：无
 * 
 */
extern void TMR_1MS_UpdateRequest ( void )
{
  u32 i;

  if ( Timer.tmr_nbr > 0 && Tmr1msCounter > 0 )
  {
    for ( i = 0; i < TMR_ID_COUNT; i++ )
    {
      if ( Timer.tmr_alloc[ i ].TimerId > 0 && Timer.tmr_alloc[ i ].TimerType == TMR_TYPE_1MS )
      {
        if ( Timer.tmr_count[ i ] <= 1 )
        {
          u8 TimerId = Timer.tmr_alloc[ i ].TimerId;

          if ( Timer.tmr_cyclic[ i ] == FALSE )
          {

#if OS_CRITICAL_METHOD == 3                           /* Allocate storage for CPU status register      */
    OS_CPU_SR  cpu_sr = 0;
#endif
            OS_ENTER_CRITICAL ();
            Timer.tmr_alloc[ i ].TimerId = 0;
            Timer.tmr_nbr--;
            Tmr10msCounter--;
            OS_EXIT_CRITICAL ();

          }
          else
          {
            Timer.tmr_count[ i ] = Timer.tmr_alloc[ i ].TimerValue;
          }

          if ( Timer.tmr_alloc[ i ].TimerHdlr )
          {
            Timer.tmr_alloc[ i ].TimerHdlr ( TimerId );
          }
        } 
        else  
        {
          Timer.tmr_count[ i ]--;
        }          
      }
    } 
  }
}






/* 
 * 功能描述：定时器更新10ms处理
 * 引用参数：无
 *          
 * 返回值  ：无
 * 
 */
extern void TMR_TickUpdateRequest ( void )
{
  u32 i;

  if ( Timer.tmr_nbr > 0 && Tmr10msCounter > 0 )
  {
    for ( i = 0; i < TMR_ID_COUNT; i++ )
    {
      if ( Timer.tmr_alloc[ i ].TimerId > 0 && Timer.tmr_alloc[ i ].TimerType == TMR_TYPE_10MS )
      {
        if ( Timer.tmr_count[ i ] <= 1 )
        {
          u8 TimerId = Timer.tmr_alloc[ i ].TimerId;

          if ( Timer.tmr_cyclic[ i ] == FALSE )
          {

#if OS_CRITICAL_METHOD == 3                           /* Allocate storage for CPU status register      */
    OS_CPU_SR  cpu_sr = 0;
#endif
            OS_ENTER_CRITICAL ();
            Timer.tmr_alloc[ i ].TimerId = 0;
            Timer.tmr_nbr--;
            Tmr10msCounter--;
            OS_EXIT_CRITICAL ();

          }
          else
          {
            Timer.tmr_count[ i ] = Timer.tmr_alloc[ i ].TimerValue;
          }

          if ( Timer.tmr_alloc[ i ].TimerHdlr )
          {
            Timer.tmr_alloc[ i ].TimerHdlr ( TimerId );
          }
        } 
        else  
        {
          Timer.tmr_count[ i ]--;
        }          
      }
    } 
  }
}











/* 
 * 功能描述：定时器更新100ms处理
 * 引用参数：无
 *          
 * 返回值  ：无
 * 
 */
extern void TMR_100msUpdateRequest ( void )
{
  u32 i;

  if ( Timer.tmr_nbr > 0 && Tmr100msCounter > 0 )
  {
    for ( i = 0; i < TMR_ID_COUNT; i++ )
    {
      if ( Timer.tmr_alloc[ i ].TimerId > 0 && Timer.tmr_alloc[ i ].TimerType == TMR_TYPE_100MS )
      {
        if ( Timer.tmr_count[ i ] <= 1 )
        {
          u8 TimerId = Timer.tmr_alloc[ i ].TimerId;

          if ( Timer.tmr_cyclic[ i ] == FALSE )
          {

#if OS_CRITICAL_METHOD == 3                           /* Allocate storage for CPU status register      */
    OS_CPU_SR  cpu_sr = 0;
#endif
            OS_ENTER_CRITICAL ();
            Timer.tmr_alloc[ i ].TimerId = 0;
            Timer.tmr_nbr--;
            Tmr100msCounter--;
            OS_EXIT_CRITICAL ();

          }
          else
          {
            Timer.tmr_count[ i ] = Timer.tmr_alloc[ i ].TimerValue;
          }

          if ( Timer.tmr_alloc[ i ].TimerHdlr )
          {
            Timer.tmr_alloc[ i ].TimerHdlr ( TimerId );
          }
        } 
        else  
        {
          Timer.tmr_count[ i ]--;
        }          
      }
    } 
  }
}



