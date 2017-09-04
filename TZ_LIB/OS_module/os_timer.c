/*

 * 
 * (1)�ļ�������
 * (2)�������ٶ�ʱ��������ƻ���;
 * (3)����TMR�źŴ������ƣ�
 * (4����1MS��ʱ��
 * 
 * 
 */
#include "tiza_timer_port.h"
#include "os_timer.h"
#include "hardware_select_config.h"





/******************************************/
/*              �ڲ�����[����]            */
/******************************************/

#if __USE_TMR__ == 1
static void App_TaskTMRFast ( void *p_arg );
static void App_TaskTMRSlow ( void *p_arg );
static void App_TaskTMR_1MS ( void *p_arg );
#endif  /* __USE_TMR__ */





/******************************************/
/*              �ڲ�����[����]            */
/******************************************/


#if __USE_TMR__ == 1
static OS_EVENT  *tmr_os_sem0 = NULL;
static  __attribute__ ((aligned (8))) OS_STK   App_TaskTMRFastStk[ APP_TASK_TMR_TICK_STK_SIZE ];
static OS_EVENT  *tmr_os_sem1 = NULL;
static  __attribute__ ((aligned (8))) OS_STK   App_TaskTMRSlowStk[ APP_TASK_TMR_SLOW_STK_SIZE ];
static OS_EVENT  *tmr_os_sem2 = NULL;
static  __attribute__ ((aligned (8))) OS_STK   App_TaskTMR1MSStk[ APP_TASK_TMR_SLOW_STK_SIZE ];
#endif  /* __USE_TMR__ */













/* 
 * ��������: TMR �ź�������
 * ���ò���:
 *          
 * ����ֵ  :
 * 
 */
extern void TMR_OSIfInit ( void )
{

#if __USE_TMR__ == 1
  CPU_INT08U  os_err;

    
	App_TaskTMRFastStk[0] =  0x5153;
	App_TaskTMRFastStk[1] =  0xAA55;
	 
	App_TaskTMRSlowStk[0] =  0x5153;
	App_TaskTMRSlowStk[1] =  0xAA55;

	App_TaskTMR1MSStk[0] =   0x5153;
	App_TaskTMR1MSStk[1] =   0xAA55;
	
	tmr_os_sem0  = ( OS_EVENT * )OSSemCreate ( 1 );


  /* ��������TMR��������(10ms)*/
  os_err = OSTaskCreateExt((void (*)(void *)) App_TaskTMRFast,
                           (void          * ) 0,
                           (OS_STK        * )&App_TaskTMRFastStk[ APP_TASK_TMR_TICK_STK_SIZE - 1 ],
                           (INT8U           ) APP_TASK_TMR_10MS_PRIO,
                           (INT16U          ) APP_TASK_TMR_10MS_PRIO,
                           (OS_STK        * )&App_TaskTMRFastStk[ 0 ],
                           (INT32U          ) APP_TASK_TMR_TICK_STK_SIZE,
                           (void          * ) 0,
                           (INT16U          )TZ_OS_TASK_OPT_CHECK);
#if OS_TASK_NAME_EN > 0u
  OSTaskNameSet(APP_TASK_TMR_10MS_PRIO, (INT8U *)(void *)"TaskTMR_10MS", &os_err);
#endif

	tmr_os_sem1  = ( OS_EVENT * )OSSemCreate ( 1 );

  /* ��������TMR��������100ms��*/
  os_err = OSTaskCreateExt((void (*)(void *)) App_TaskTMRSlow,
                           (void          * ) 0,
                           (OS_STK        * )&App_TaskTMRSlowStk[ APP_TASK_TMR_SLOW_STK_SIZE - 1 ],
                           (INT8U           ) APP_TASK_TMR_100MS_PRIO,
                           (INT16U          ) APP_TASK_TMR_100MS_PRIO,
                           (OS_STK        * )&App_TaskTMRSlowStk[ 0 ],
                           (INT32U          ) APP_TASK_TMR_SLOW_STK_SIZE,
                           (void          * ) 0,
													 (INT16U          )TZ_OS_TASK_OPT_CHECK);  
#if OS_TASK_NAME_EN > 0u
  OSTaskNameSet(APP_TASK_TMR_100MS_PRIO, (INT8U *)(void *)"TaskTMR_100MS", &os_err);
#endif

   tmr_os_sem2  = ( OS_EVENT * )OSSemCreate ( 1 );

  /* ����1MS_TMR�������� */
  os_err = OSTaskCreateExt((void (*)(void *)) App_TaskTMR_1MS,
                           (void          * ) 0,
                           (OS_STK        * )&App_TaskTMR1MSStk[ APP_TASK_TMR_1MS_STK_SIZE - 1 ],
                           (INT8U           ) APP_TASK_TMR_1MS_PRIO,
                           (INT16U          ) APP_TASK_TMR_1MS_PRIO,
                           (OS_STK        * )&App_TaskTMR1MSStk[ 0 ],
                           (INT32U          ) APP_TASK_TMR_1MS_STK_SIZE,
                           (void          * ) 0,
                           (INT16U          )TZ_OS_TASK_OPT_CHECK);  
#if OS_TASK_NAME_EN > 0u
  OSTaskNameSet(APP_TASK_TMR_1MS_PRIO, (INT8U *)(void *)"TaskTMR_1MS", &os_err);
#endif
  os_err = os_err;
#endif  /* __USE_TMR__ */

}

extern bool CheckTMRFastStackOverFlow(void)
{
    if((App_TaskTMRFastStk[0] != 0x5153)||(App_TaskTMRFastStk[1] != 0xAA55))
    {
          printf("\r\n TMR Fast stack over flow! \r\n");
          return TRUE;
    }     
    else
    {
          return FALSE;
    }
}

extern bool CheckTMRSlowStackOverFlow(void)
{
    if((App_TaskTMRSlowStk[0] != 0x5153)||(App_TaskTMRSlowStk[1] != 0xAA55))
    {
          printf("\r\n TMR Slow stack over flow! \r\n");
          return TRUE;
    }     
    else
    {
          return FALSE;
    }
}






#if __USE_TMR__ == 1

/* 
 * ��������: TMR�ź�������
 * ���ò���:
 *          
 * ����ֵ  :
 * 
 */
static void App_TaskTMRFast ( void *p_arg )
{
    CPU_INT08U   err;


    (void)p_arg;

    while (DEF_TRUE) {
        OSSemPend ( tmr_os_sem0, 0, ( u8* )&err );
        //if (err == OS_TIMEOUT) 
        {
            TMR_TickUpdateRequest ();
        }
    }
}










/* 
 * ��������: TMR�ź�������
 * ���ò���:
 *          
 * ����ֵ  :
 * 
 */
static void App_TaskTMRSlow ( void *p_arg )
{
    CPU_INT08U   err;


    (void)p_arg;

    while (DEF_TRUE) {
        OSSemPend ( tmr_os_sem1, 0, ( u8* )&err );
        //if (err == OS_TIMEOUT) 
        {
            TMR_100msUpdateRequest ();
        }
    }
}


/* 
 * ��������: TMR�ź�������
 * ���ò���:
 *          
 * ����ֵ  :
 * 
 */
static void App_TaskTMR_1MS ( void *p_arg )
{
    CPU_INT08U   err;

    (void)p_arg;

    while (DEF_TRUE) {
        OSSemPend ( tmr_os_sem2, 0, ( u8* )&err );
        //if (err == OS_TIMEOUT) 
        {
            TMR_1MS_UpdateRequest();
        }
    }
}

/* 
 * ��������: TMR�ź�������
 * ���ò���:
 *          
 * ����ֵ  :
 * 
 */
extern void TMR_SemPost ( void )
{
  static vu32 singalCount = 0;

  singalCount++;

  /* ����=10ms */
  OSSemPost ( tmr_os_sem0 );
  if ( singalCount >= 10 )
  {
    /* ����==100ms */
    singalCount = 0;
    OSSemPost ( tmr_os_sem1 );     
  }   
}


/* 
**********************************************************************************
 * ������:TMR_SemPost_1MS
 * ��  ��: ��
 * ��  �ܣ������ź���        
 * ����ֵ: ��
 ********************************************************************************** 
 */
extern void TMR_SemPost_1MS( void )
{
  /* ����=1ms */
  OSSemPost ( tmr_os_sem2 );   
}

#endif  /* __USE_TMR__ */



