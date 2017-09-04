#include "defines.h"
#include "app_cfg.h"










/******************************************/
/*              �ڲ�����[����]            */
/******************************************/

static bool __OS_TASK_IS_STOP = FALSE;










/* 
 * ��������: LPWR ��ͣ����
 * ���ò���:
 *          
 * ����ֵ  :
 * 
 */
extern void LPWR_OSTaskSuspend ( void )
{
  INT8U err;

  if ( __OS_TASK_IS_STOP == FALSE )
  {

#if OS_CRITICAL_METHOD == 3                           /* Allocate storage for CPU status register      */
    OS_CPU_SR  cpu_sr = 0;
#endif

    OS_ENTER_CRITICAL ();
    __OS_TASK_IS_STOP = TRUE;
    OS_EXIT_CRITICAL ();


    OSSchedLock ();
    err = OSTaskSuspend ( APP_TASK_START_PRIO );
    if ( err != OS_ERR_NONE )
    {

#if LPWR_OS_PORT_DEBUG == 1
      printf ( "\r\n[LPWR PORT] suspend task APP_TASK_START_PRIO err**\r\n" );
#endif  /* LPWR_OS_PORT_DEBUG */
    
    }


#if __USE_USART__ == 1
    err = OSTaskSuspend ( APP_TASK_USART_RX_PRIO );
    if ( err != OS_ERR_NONE )
    {

#if LPWR_OS_PORT_DEBUG == 1
      printf ( "\r\n[LPWR PORT] suspend task APP_TASK_USART_RX_PRIO err**\r\n" );
#endif  /* LPWR_OS_PORT_DEBUG */
    
    }
#endif  /* __USE_USART__ */


#if __USE_CAN__ == 1  
    err = OSTaskSuspend ( APP_TASK_CANREC_PRIO );
    if ( err != OS_ERR_NONE )
    {

#if LPWR_OS_PORT_DEBUG == 1
      printf ( "\r\n[LPWR PORT] suspend task APP_TASK_CAN_RX_PRIO err**\r\n" );
#endif  /* LPWR_OS_PORT_DEBUG */
    
    }
#endif  /* __USE_CAN__ */



#if __USE_TMR__ == 1  
    err = OSTaskSuspend ( APP_TASK_TMR_10MS_PRIO );
    if ( err != OS_ERR_NONE )
    {

#if LPWR_OS_PORT_DEBUG == 1
      printf ( "\r\n[LPWR PORT] suspend task APP_TASK_TMR_10MS_PRIO err**\r\n" );
#endif  /* LPWR_OS_PORT_DEBUG */
    
    }

    err = OSTaskSuspend ( APP_TASK_TMR_100MS_PRIO );
    if ( err != OS_ERR_NONE )
    {

#if LPWR_OS_PORT_DEBUG == 1
      printf ( "\r\n[LPWR PORT] suspend task APP_TASK_TMR_100MS_PRIO err**\r\n" );
#endif  /* LPWR_OS_PORT_DEBUG */
    
    }
#endif  /* __USE_TMR__ */

    err = OSTaskSuspend ( APP_TASK_WORKER_PRIO );
    if ( err != OS_ERR_NONE )
    {

#if LPWR_OS_PORT_DEBUG == 1
      printf ( "\r\n[LPWR PORT] suspend task APP_TASK_WORKER_PRIO err**\r\n" );
#endif  /* LPWR_OS_PORT_DEBUG */
    
    }

    OSSchedUnlock ();
  }

}













/* 
 * ��������: LPWR �ָ�����
 * ���ò���:
 *          
 * ����ֵ  :
 * 
 */
extern void LPWR_OSTaskResume ( void )
{
  INT8U err;

  if ( __OS_TASK_IS_STOP == TRUE )
  {

#if OS_CRITICAL_METHOD == 3                           /* Allocate storage for CPU status register      */
    OS_CPU_SR  cpu_sr = 0;
#endif

    OS_ENTER_CRITICAL ();
    __OS_TASK_IS_STOP = FALSE;
    OS_EXIT_CRITICAL ();

    OSSchedLock ();
    err = OSTaskResume ( APP_TASK_START_PRIO );
    if ( err != OS_ERR_NONE )
    {

#if LPWR_OS_PORT_DEBUG == 1
      printf ( "\r\n[LPWR PORT] resume task APP_TASK_START_PRIO err**\r\n" );
#endif  /* LPWR_OS_PORT_DEBUG */
    
    }


#if __USE_USART__ == 1     
    err = OSTaskResume ( APP_TASK_USART_RX_PRIO );
    if ( err != OS_ERR_NONE )
    {

#if LPWR_OS_PORT_DEBUG == 1
      printf ( "\r\n[LPWR PORT] resume task APP_TASK_USART_RX_PRIO err**\r\n" );
#endif  /* LPWR_OS_PORT_DEBUG */
    
    }
#endif  /* __USE_USART__ */


#if __USE_CAN__ == 1      
    err = OSTaskResume ( APP_TASK_CANREC_PRIO );
    if ( err != OS_ERR_NONE )
    {

#if LPWR_OS_PORT_DEBUG == 1
      printf ( "\r\n[LPWR PORT] resume task APP_TASK_CAN_RX_PRIO err**\r\n" );
#endif  /* LPWR_OS_PORT_DEBUG */
    
    }
#endif  /* __USE_CAN__ */


#if __USE_TMR__ == 1      
    err = OSTaskResume ( APP_TASK_TMR_10MS_PRIO );
    if ( err != OS_ERR_NONE )
    {

#if LPWR_OS_PORT_DEBUG == 1
      printf ( "\r\n[LPWR PORT] resume task APP_TASK_TMR_10MS_PRIO err**\r\n" );
#endif  /* LPWR_OS_PORT_DEBUG */
    
    }

    err = OSTaskResume ( APP_TASK_TMR_100MS_PRIO );
    if ( err != OS_ERR_NONE )
    {

#if LPWR_OS_PORT_DEBUG == 1
      printf ( "\r\n[LPWR PORT] resume task APP_TASK_TMR_100MS_PRIO err**\r\n" );
#endif  /* LPWR_OS_PORT_DEBUG */
    
    }
#endif  /* __USE_TMR__ */

    err = OSTaskResume ( APP_TASK_WORKER_PRIO );
    if ( err != OS_ERR_NONE )
    {

#if LPWR_OS_PORT_DEBUG == 1
      printf ( "\r\n[LPWR PORT] resume task APP_TASK_WORKER_PRIO err**\r\n" );
#endif  /* LPWR_OS_PORT_DEBUG */
    
    }

    OSSchedUnlock ();
  }
}


