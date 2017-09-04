#include "includes.h"
#include "tiza_can_port.h"
#include "os_usart.h"
#include "tiza_gps_port.h"
#include "GSM_xx.h"
#include "buffer_helper.h"
#include "tiza_gprs_protocol.h"
#include "shell_usercmd.h"
#include "tiza_wdg.h"
#include "tiza_params.h"
#include "gb_gprs_protocol.h"
#include "xmit_manager.h"
#include "tiza_w25qxx.h"
#include "w25qxx_port.h"
#include "tiza_init.h"
#include "tiza_utility.h"
#include "sd_port.h"

/*
*=========================================================================
*调试开关量
*=========================================================================
*/


#if APP_STACK_CHECK_DEBUG  ==1

#define  APP_STACK_CHECK   												512
#define  APP_STACK_CHECK_PRIO   				 					21
static OS_STK App_Stack_Check[APP_STACK_CHECK];
static void AppStackCheck(void);
static void AppStackCheckCreate(void);
#endif

/*
**************************************************************************
*                              任务堆栈
**************************************************************************
*/


/* 定义每个任务的堆栈空间，app_cfg.h文件中宏定义栈大小 */
 static OS_STK AppTaskStartStk[APP_TASK_START_STK_SIZE];

/*
****************************************************************************
*                              函数声明
****************************************************************************
*/
static void AppTaskCreate(void);
static void AppTaskStart(void *p_arg);

void SPI_FLASH_TEST(void);
static void OS_Task_UseRate(void);
void SYS_CLOCK_PRINTF(void);

/*
***************************************************************************
*
*
***************************************************************************
*/







/*
***************************************************************************
*                               变量
****************************************************************************
*/

/* 定义一个邮箱，    这只是一个邮箱指针，  OSMboxCreate函数会创建邮箱必需的资源 */
OS_EVENT *AppUserIFMbox;
OS_EVENT *AppMsgMbox;

/* 定义一个信号量，  这只是一个信号量指针，OSSEMCreate函数会创建邮箱必需的资源 */
OS_EVENT *Semp;
OS_EVENT *AppTouchSemp;

/*创建一个内存块*/
/*创建一个内存分区 2个内存块 每个内存块2050个字节*/
//OS_MEM   *CommMem; 


/* ADD #001 任务超时监控机制 */
#if defined SYSTEM_RESTART__
#undef  RESTART_SYSTEM_COUNTER_MAX_NUMBER
#define RESTART_SYSTEM_COUNTER_MAX_NUMBER      			3   /* 总时长30秒 */
#endif  /* SYSTEM_RESTART__ */

vu32 __RestartSystemCounter = 0;
vu32 __RestartGprsCounter = 0;


/*
****************************************************************************
*	函 数 名: main
*	功能说明: 标准c程序入口。
*	形    参：无
*	返 回 值: 无
****************************************************************************
*/


int main(void)
{

	/* 初始化"uC/OS-II"内核 */
	OSInit();
  
	/* 创建一个启动任务（也就是主任务）。启动任务会创建所有的应用程序任务 */
	OSTaskCreateExt(AppTaskStart,	/* 启动任务函数指针 */
                    (void *)0,		/* 传递给任务的参数 */
                    (OS_STK *)&AppTaskStartStk[APP_TASK_START_STK_SIZE - 1], /* 指向任务栈栈顶的指针 */
                    APP_TASK_START_PRIO,	/* 任务的优先级，必须唯一，数字越低优先级越高 */
                    APP_TASK_START_PRIO,	/* 任务ID，一般和任务优先级相同 */
                    (OS_STK *)&AppTaskStartStk[0],/* 指向任务栈栈底的指针。OS_STK_GROWTH 决定堆栈增长方向 */
                    APP_TASK_START_STK_SIZE, /* 任务栈大小 */
                    (void *)0,	/* 一块用户内存区的指针，用于任务控制块TCB的扩展功能
                       （如任务切换时保存CPU浮点寄存器的数据）。一般不用，填0即可 */
                    OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR); /* 任务选项字 */
					
					/*  定义如下：
						OS_TASK_OPT_STK_CHK      使能检测任务栈，统计任务栈已用的和未用的
						OS_TASK_OPT_STK_CLR      在创建任务时，清零任务栈
						OS_TASK_OPT_SAVE_FP      如果CPU有浮点寄存器，则在任务切换时保存浮点寄存器的内容
					*/                  

	/* 指定任务的名称，用于调试。这个函数是可选的 */
	//OSTaskNameSet(APP_TASK_START_PRIO, APP_TASK_START_NAME, &err);
	
	/*ucosII的节拍计数器清0    节拍计数器是0-4294967295*/ 
	OSTimeSet(0);	
	
	/* 启动多任务系统，控制权交给uC/OS-II */
	OSStart();
}

#if  APP_STACK_CHECK_DEBUG ==1
static void AppStackCheckCreate(void)
{
  CPU_INT08U  os_err;
  os_err = OSTaskCreateExt((void (*)(void *)) AppStackCheck,
                           (void          * ) 0,
                           (OS_STK        * )&App_Stack_Check[ APP_STACK_CHECK - 1 ],
                           (INT8U           ) APP_STACK_CHECK_PRIO,
                           (INT16U          ) APP_STACK_CHECK_PRIO,
                           (OS_STK        * )&App_Stack_Check[ 0 ],
                           (INT32U          ) App_Stack_Check,
                           (void          * ) 0,
                           (INT16U          )(OS_TASK_OPT_STK_CHK)); 

#if OS_TASK_NAME_EN > 0u
  OSTaskNameSet(APP_STACK_CHECK_PRIO, (INT8U *)(void *)"StackCheck", &os_err);
#endif

}
#endif

/*
*********************************************************************************************************
*	函 数 名: AppTaskStart
*	功能说明: 这是一个启动任务，在多任务系统启动后，必须初始化滴答计数器(在BSP_Init中实现)
*	形    参：p_arg 是在创建该任务时传递的形参
*	返 回 值: 无
	优 先 级：1
*********************************************************************************************************
*/
static void AppTaskStart(void *p_arg)
{	
    /* 仅用于避免编译器告警，编译器不会产生任何目标代码 */	
    (void)p_arg;   
	
		/* BSP 初始化。 BSP = Board Support Package 板级支持包，可以理解为底层驱动。*/
		bsp_Init();
    CPU_Init();           /* Init CPU name & int. dis. time measuring fncts.          */
		BSP_Tick_Init();
		//Mem_Init();           /* Init Memory Management Module.                           */

	/*检测CPU能力，统计模块初始化。该函数将检测最低CPU占有率 */
	#if (OS_TASK_STAT_EN > 0)
			OSStatInit();
	#endif
	/*堆栈检测任务*/
	#if  APP_STACK_CHECK_DEBUG ==1
	AppStackCheckCreate();
	#endif
		
	/* 创建应用程序的任务*/
	AppTaskCreate();
#if  0
	SYS_CLOCK_PRINTF();
#endif
	while (1)     
	{   
			 WDG_KickCmd();
		
#if APP_STACK_CHECK_DEBUG	== 1		
			OS_Task_UseRate();
#endif		
			//SD_WriteDataSeek(&file,"gulin.txt",str,12);
	 
			//Printf_TimeInfoDisplay();

			 __RestartSystemCounter++;

			 __RestartGprsCounter++;
			
			if(( __RestartGprsCounter>= RESTART_SYSTEM_COUNTER_MAX_NUMBER) ||( __RestartSystemCounter >= RESTART_SYSTEM_COUNTER_MAX_NUMBER) )
			{    
#if SYSTEM_RESTART_DEBUG == 1
						printf ( "\r\n[SYS RESET] task type\r\n" );
#endif  /* SYSTEM_RESTART_DEBUG */
						System_Reset();
			}  
			OSTimeDlyHMSM(0,0,10,0);		
	}
	
}

/*
*********************************************************************************************************
*	函 数 名: AppTaskCreate
*	功能说明: 创建应用任务
*	形    参：p_arg 是在创建该任务时传递的形参
*	返 回 值: 无
*********************************************************************************************************
*/
static void AppTaskCreate (void)
{
	/* 共享缓存初始化 */
  SHABUF_InitBuffer ();
	
	/* 应用参数获取 */
	ParameterQuery_Init();
  sysParamsInit();
	
	/*保存应用参数*/
	sysParamsSave();
	
	/* 启动传输管理器 */
  XMIT_StartManager ();	
	
	/*初始化模块*/
	CMD_ApplicationIfInit();
	GPS_ApplicationIfInit();
	TZ_CAN_ApplicationIfInit();
	TZ_UART_ApplicationIfInit();
	
	/*注册模块*/
	USARTx_Start(SHELL_USART, Shell_DataHandler);
	USARTx_Start(GPS_USART,GPS_DataHandler);
	MDM_ResetPIN();
  MODEM_Start();
  USARTx_Start ( ( usart_Flow_e )GSM_USART, MODEM_ReadData );
 
 /*启动应用*/
  App_tmrStart();
}




#if  APP_STACK_CHECK_DEBUG ==1
void AppStackCheck(void)
{
	while(1)
	{
			OS_Task_UseRate();
			OSTimeDlyHMSM(0, 0, 30, 0);
	}
}
#endif

/*
*********************************************************************************************************
*	函 数 名: OS_Task_UseRate
*	功能说明: 将uCOS-II任务信息打印到串口
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void OS_Task_UseRate(void)
{
	OS_TCB      *ptcb;	        /* 定义一个任务控制块指针, TCB = TASK CONTROL BLOCK */
	OS_STK_DATA stk;	        /* 用于读取任务栈使用情况 */

	ptcb    = &OSTCBTbl[0];		/* 指向第1个任务控制块(TCB) */

	/* 打印标题 */
	printf("  Prio   Used  Free  Per    Taskname\r\n");
	OSTimeDly(10);
	/* 遍历任务控制块列表(TCB list)，打印所有的任务的优先级和名称 */
	while (ptcb != NULL)
	{
		/* 
			ptcb->OSTCBPrio : 任务控制块中保存的任务的优先级
			ptcb->OSTCBTaskName : 任务控制块中保存的任务名称，需要在创建任务的时候
			调用OSTaskNameSet()指定任务名称，比如：
			OSTaskNameSet(APP_TASK_USER_IF_PRIO, "User I/F", &err);
		*/
		OSTaskStkChk(ptcb->OSTCBPrio, &stk);	/* 获得任务栈已用空间 */
		printf("   %2d  %5d  %5d  %02d%%   %s\r\n", ptcb->OSTCBPrio, 
		stk.OSUsed, stk.OSFree, (stk.OSUsed * 100) / (stk.OSUsed + stk.OSFree),
		ptcb->OSTCBTaskName);		
		ptcb = ptcb->OSTCBPrev;		            /* 指向上一个任务控制块 */
	}
}



void SYS_CLOCK_PRINTF(void)
{
	
	RCC_ClocksTypeDef Clock_Info;	 

	RCC_GetClocksFreq(&Clock_Info);
	
	printf("\r\nHCLK_Frequency:   %d	\r\n ",Clock_Info.HCLK_Frequency);
	printf("\r\nPCLK1_Frequency:	%d 	\r\n ",Clock_Info.PCLK1_Frequency);
	printf("\r\nPCLK2_Frequency:	%d 	\r\n ",Clock_Info.PCLK2_Frequency);
	printf("\r\nSYSCLK_Frequency: %d	\r\n ",Clock_Info.SYSCLK_Frequency);



}




void SPI_FLASH_TEST(void)
{

	  u32 temp=SPI_FLASH_ReadID();
		printf("\r\n SPI_FLASH_ReadID: %x \r\n",temp);
		//temp=SPI_FLASH_ReadDeviceID();
		printf("\r\n SPI_FLASH_ReadDeviceID: %x \r\n",temp);		
		
		SPI_FLASH_BufWrite(W25Q64_Start_Address,(u8*)"guchujie 1018@tiza.EEE",strlen("guchujie 1018@tiza.com"));
		
		SPI_FLASH_BufWrite  (W25Q64_Start_Address+26,(u8*)"guchujie 1018@tiza.VVV",strlen("guchujie 1018@tiza.UUU"));
		char buffer[50];
		SPI_FLASH_BufRead((u8*)buffer,W25Q64_Start_Address,22);
		printf("\r\nSPI_FLASH_BufferRead:%s \r\n ",buffer);
		printf("\r\nSPI_FLASH_BufferRead--------> \r\n ");
		
		char buf[50];
		SPI_FLASH_BufRead((u8*)buf,W25Q64_Start_Address+26,22);
		for(u8 i=0;i<23;i++)
		{
				printf("%c",buf[i]);
		}
		printf("\r\n\r\n");	



}

#define USE_GSM_RING   1
#if OS_VERSION >= 251

static u32 SemTimer_count=0;
void  App_TaskIdleHook (void)
{		
	
#if __USE_RTC__==1
		SemTimer_count++;
		if(SemTimer_count>=0x50)
		{
				SemTimer_count=0;


				TM_RTC_GetDateTime(&RTC_time,TM_RTC_Format_BIN);

		}		
#endif
	
	

		
#if defined SYSTEM_RESTART__
	
	#if SYSTEM_RESTART__ == 1
		__RestartSystemCounter = 0;
	#endif
	
#endif  /* SYSTEM_RESTART__ */

#if __USE_ACC__ == 1
	acc_io_handler=TZ_Get_ACC_State();
#endif
	
#if USE_GSM_RING	==1
 //@ 1: ok     0: SMS/phone is arrive
	 gsmring_io_handler=TM_GPIO_GetInputPinValue(GPIOF,GPIO_Pin_7);	
#endif	
	
#if defined __USE_LOW_PWR__
  
	/* 低功耗配置 */
	#if __USE_LOW_PWR__ == 1
		LPWR_IsSTOPMode();
	#endif 
	
#endif  /* __USE_LOW_PWR__ */
	

}
#endif



