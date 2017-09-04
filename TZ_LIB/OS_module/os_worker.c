#include "os_worker.h"
#include "worker.h"




/******************************************/
/*              内部函数[声明]            */
/******************************************/

#if __USE_WORKQUEUE__ == 1
static void App_TaskWorkers ( void *p_arg );
#endif  /* __USE_WORKQUEUE__ */


/******************************************/
/*              内部变量[定义]            */
/******************************************/


#if __USE_WORKQUEUE__ == 1
static OS_EVENT  *work_os_sem = NULL;
static  __attribute__ ((aligned (8))) OS_STK   App_TaskWorker[ APP_TASK_WORKER_STK_SIZE ];
#endif  /* __USE_WORKQUEUE__ */




/* 
 * 功能描述: 信号量创建
 * 引用参数:
 *          
 * 返回值  :
 * 
 */
extern void WORKER_OSIfInit ( void )
{

#if __USE_WORKQUEUE__ == 1
  CPU_INT08U  os_err;
    App_TaskWorker[0] = 0x5153;
    App_TaskWorker[1] = 0xAA55;

	work_os_sem = ( OS_EVENT * )OSSemCreate ( 1 );

  /* 创建处理任务 */
  os_err = OSTaskCreateExt((void (*)(void *)) App_TaskWorkers,
                           (void          * ) 0,
                           (OS_STK        * )&App_TaskWorker[ APP_TASK_WORKER_STK_SIZE - 1 ],
                           (INT8U           ) APP_TASK_WORKER_PRIO,
                           (INT16U          ) APP_TASK_WORKER_PRIO,
                           (OS_STK        * )&App_TaskWorker[ 0 ],
                           (INT32U          ) APP_TASK_WORKER_STK_SIZE,
                           (void          * ) 0,
                           (INT16U          )TZ_OS_TASK_OPT_CHECK);
#if OS_TASK_NAME_EN > 0u
  OSTaskNameSet(APP_TASK_WORKER_PRIO, (INT8U *)(void *)"worker", &os_err);
#endif

  os_err = os_err;
#endif  /* __USE_WORKQUEUE__ */

}

extern bool CheckWorkStackOverFlow(void)
{
    if((App_TaskWorker[0] != 0x5153)||(App_TaskWorker[1] != 0xAA55))
    {
          printf("\r\n work stack over flow! \r\n");
          return TRUE;
    }
    else
    {
          return FALSE;
    }
}






#if __USE_WORKQUEUE__ == 1

/* 
 * 功能描述: 信号量发射
 * 引用参数:
 *          
 * 返回值  :
 * 
 */
static void App_TaskWorkers ( void *p_arg )
{
    CPU_INT08U   err;

    (void)p_arg;

    while (DEF_TRUE) {
    if ( WORKER_DoWorks() == FALSE )
    {
         OSSemPend ( work_os_sem, 0, ( u8* )&err );
    }
    else
    {
         OSSemPend ( work_os_sem, 100, ( u8* )&err );
    }
    }
}






/* 
 * 功能描述: WORKER 信号量发射
 * 引用参数:
 *          
 * 返回值  :
 * 
 */
extern void WORKER_SemPost ( void )
{
	
#if (OS_SEM_EN > 0)
  u8 err = OS_ERR_NONE;

  err = OSSemPost ( work_os_sem );
  err = err;
#endif
}

#endif  /* __USE_WORKQUEUE__ */



