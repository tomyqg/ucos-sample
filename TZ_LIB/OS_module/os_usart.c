#include "os_usart.h"
#include "defines.h"
#include "ucos_ii.h"




/******************************************/
/*              内部函数[声明]            */
/******************************************/

static void App_TaskUSARTRx ( void *p_arg );




/******************************************/
/*              内部变量[定义]            */
/******************************************/

/* 是否初始化?? */
static bool usartIsInit = false;


static   __attribute__ ((aligned (8))) OS_STK   App_TaskUsartRxStk[ APP_TASK_USART_RX_STK_SIZE ];

/******************************************/
/*              内部变量[定义]            */
/******************************************/

OS_EVENT  *usart_os_sem0 = NULL;





#if __USE_USART_1__ == 1
static OS_EVENT  *usart_os_sem1 = NULL;
#endif  /* __USE_USART_1__ */


#if __USE_USART_2__ == 1
static OS_EVENT  *usart_os_sem2 = NULL;
#endif  /* __USE_USART_2__ */


#if __USE_USART_3__ == 1
static OS_EVENT  *usart_os_sem3 = NULL;
#endif  /* __USE_USART_3__ */


#if __USE_USART_4__ == 1
static OS_EVENT  *usart_os_sem4 = NULL;
#endif  /* __USE_USART_4__ */


#if __USE_USART_5__ == 1
static OS_EVENT  *usart_os_sem5 = NULL;
#endif  /* __USE_USART__ */


#if USART_TASK_DEBUG == 1

/* 定义样本采集次数 */
static vu32 systemMSCounter = 0;






/* 
 * 功能描述：
 * 引用参数：无
 *          
 * 返回值  ：状态码
 * 
 */
extern void USART_FreashCount ( void )
{
  systemMSCounter++;
}


#endif  /* USART_TASK_DEBUG */







/* 
 * 功能描述: USART 信号量创建
 * 引用参数:
 *          
 * 返回值  :
 * 
 */
extern void USART_OSIfInit ( void )
{

#if __USE_USART__ == 1
  CPU_INT08U  os_err;

  App_TaskUsartRxStk[0] = 0x5153;
  App_TaskUsartRxStk[1] = 0xAA55;
  
  if ( usartIsInit == TRUE )
  {
    return ;
  }
  usartIsInit = TRUE;

#if __USE_USART_1__ == 1
	usart_os_sem1 = ( OS_EVENT * )OSSemCreate ( 1 );
#endif  /* __USE_USART_1__ */


#if __USE_USART_2__ == 1
	usart_os_sem2 = ( OS_EVENT * )OSSemCreate ( 1 );
#endif  /* __USE_USART_2__ */


#if __USE_USART_3__ == 1
	usart_os_sem3 = ( OS_EVENT * )OSSemCreate ( 1 );
#endif  /* __USE_USART_3__ */


#if __USE_USART_4__ == 1
	usart_os_sem4 = ( OS_EVENT * )OSSemCreate ( 1 );
#endif  /* __USE_USART_4__ */


#if __USE_USART_5__ == 1
	usart_os_sem5 = ( OS_EVENT * )OSSemCreate ( 1 );
#endif  /* __USE_USART_5__ */	

  usart_os_sem0 = ( OS_EVENT * )OSSemCreate ( 1 );

  /* 创建USART接收处理任务 */
  os_err = OSTaskCreateExt((void (*)(void *)) App_TaskUSARTRx,
                           (void          * ) 0,
                           (OS_STK        * )&App_TaskUsartRxStk[ APP_TASK_USART_RX_STK_SIZE - 1 ],
                           (INT8U           ) APP_TASK_USART_RX_PRIO,
                           (INT16U          ) APP_TASK_USART_RX_PRIO,
                           (OS_STK        * )&App_TaskUsartRxStk[ 0 ],
                           (INT32U          ) APP_TASK_USART_RX_STK_SIZE,
                           (void          * ) 0,
                           (INT16U          )TZ_OS_TASK_OPT_CHECK);
#if OS_TASK_NAME_EN > 0u
  OSTaskNameSet(APP_TASK_USART_RX_PRIO, (INT8U *)(void *)"USART_Proc_RX", &os_err);
#endif

  os_err = os_err;
#endif  /* __USE_USART__ */


}
tmr_t * TMER_USART=NULL;

void TMR_TEST()
{
	static int tcount;
	printf("\r\n  TMR_Subscribe  is   ok   \r\n");
	tcount++;
	if(tcount==8)
	{
		TMR_UnSubscribe(TMER_USART,( tmr_procTriggerHdlr_t )TMR_TEST,TMR_TYPE_1MS);
	
	}

}

void TZ_UART_ApplicationIfInit(void)
{

	USART_OSIfInit();
	//软定时器测试
	//TMER_USART=TMR_Subscribe ( TRUE, 1000, TMR_TYPE_1MS, ( tmr_procTriggerHdlr_t )TMR_TEST );
}





extern bool CheckUsartStackOverFlow(void)
{
    if((App_TaskUsartRxStk[0] != 0x5153)||(App_TaskUsartRxStk[1] != 0xAA55))
    {
          printf("\r\n Usart stack over flow! \r\n");
          return TRUE;
    }
    else
    {
          return FALSE;
    }
}







/* 
 * 功能描述: USART 信号量挂起
 * 引用参数:
 *          
 * 返回值  :
 * 
 */
extern void USART_SemPend ( usart_Flow_e Flow )
{
#if (OS_SEM_EN > 0)
  u8 err;

	switch ( Flow )
	{

#if __USE_USART_1__ == 1
		case DEV_USART1:
			OSSemPend ( usart_os_sem1, 0, ( u8* )&err );
			break;
#endif  /* __USE_USART_1__ */

#if __USE_USART_2__ == 1
		case DEV_USART2:
			OSSemPend ( usart_os_sem2, 0, ( u8* )&err );
			break;
#endif  /* __USE_USART_2__ */

#if __USE_USART_3__ == 1
		case DEV_USART3:
			OSSemPend ( usart_os_sem3, 0, ( u8* )&err );
			break;
#endif  /* __USE_USART_3__ */	

#if __USE_USART_4__ == 1
		case DEV_USART4:
			OSSemPend ( usart_os_sem4, 0, ( u8* )&err );
			break;
#endif  /* __USE_USART_4__ */

#if __USE_USART_5__ == 1
		case DEV_USART5:
			OSSemPend ( usart_os_sem5, 0, ( u8* )&err );
			break;
#endif  /* __USE_USART_5__ */	
	}
  
#endif	
}









/* 
 * 功能描述: USART 信号量挂起
 * 引用参数:
 *          
 * 返回值  :
 * 
 */
extern bool USART_SemPendExt ( usart_Flow_e Flow, u32 timeout )
{
#if (OS_SEM_EN > 0)
  u8 err;

	switch ( Flow )
	{

#if __USE_USART_1__ == 1
		case DEV_USART1:
			OSSemPend ( usart_os_sem1, timeout, ( u8* )&err );
			break;
#endif  /* __USE_USART_1__ */

#if __USE_USART_2__ == 1
		case DEV_USART2:
			OSSemPend ( usart_os_sem2, timeout, ( u8* )&err );
			break;
#endif  /* __USE_USART_2__ */

#if __USE_USART_3__ == 1
		case DEV_USART3:
			OSSemPend ( usart_os_sem3, timeout, ( u8* )&err );
			break;
#endif  /* __USE_USART_3__ */	

#if __USE_USART_4__ == 1
		case DEV_USART4:
			OSSemPend ( usart_os_sem4, timeout, ( u8* )&err );
			break;
#endif  /* __USE_USART_4__ */

#if __USE_USART_5__ == 1
		case DEV_USART5:
			OSSemPend ( usart_os_sem5, timeout, ( u8* )&err );
			break;
#endif  /* __USE_USART_5__ */	
	}
  
  if ( err != OS_ERR_NONE )
  {
    return FALSE;
  }

  return TRUE;
#endif	
}




     



/* 
 * 功能描述: USART 信号量发射
 * 引用参数:
 *          
 * 返回值  :
 * 
 */
extern void USART_SemPost ( usart_Flow_e Flow )
{
#if (OS_SEM_EN > 0)

	switch ( Flow )
	{
		
#if __USE_USART_1__ == 1
		case DEV_USART1:
			OSSemPost ( usart_os_sem1 );
			break;
#endif  /* __USE_USART_1__ */
		
#if __USE_USART_2__ == 1
		case DEV_USART2:
			OSSemPost ( usart_os_sem2 );
			break;
#endif  /* __USE_USART_2__ */
		
#if __USE_USART_3__ == 1
		case DEV_USART3:
			OSSemPost ( usart_os_sem3 );
			break;
#endif  /* __USE_USART_3__ */
		
#if __USE_USART_4__ == 1
		case DEV_USART4:
			OSSemPost ( usart_os_sem4 );
			break;
#endif  /* __USE_USART_4__ */
		
#if __USE_USART_5__ == 1
		case DEV_USART5:
			OSSemPost ( usart_os_sem5 );
			break;
#endif  /* __USE_USART_5__ */	
	}
	
#endif
}






/* 
 * 功能描述: USART 接收处理
 * 引用参数:
 *          
 * 返回值  :
 * 
 */
static void App_TaskUSARTRx ( void *p_arg )
{
    CPU_INT08U   err;


    (void)p_arg;

    while (DEF_TRUE) {
      OSSemPend ( usart_os_sem0, 0, ( u8* )&err );
      USART_LowLevelDataProcess ();

    }
}





/* 
 * 功能描述: USART 信号量发射
 * 引用参数: 无
 *          
 * 返回值  : 无
 * 
 */
extern void USART_SemPost2 ( void )
{
  OSSemPost ( usart_os_sem0 );
}



