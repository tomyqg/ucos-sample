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
*���Կ�����
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
*                              �����ջ
**************************************************************************
*/


/* ����ÿ������Ķ�ջ�ռ䣬app_cfg.h�ļ��к궨��ջ��С */
 static OS_STK AppTaskStartStk[APP_TASK_START_STK_SIZE];

/*
****************************************************************************
*                              ��������
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
*                               ����
****************************************************************************
*/

/* ����һ�����䣬    ��ֻ��һ������ָ�룬  OSMboxCreate�����ᴴ������������Դ */
OS_EVENT *AppUserIFMbox;
OS_EVENT *AppMsgMbox;

/* ����һ���ź�����  ��ֻ��һ���ź���ָ�룬OSSEMCreate�����ᴴ������������Դ */
OS_EVENT *Semp;
OS_EVENT *AppTouchSemp;

/*����һ���ڴ��*/
/*����һ���ڴ���� 2���ڴ�� ÿ���ڴ��2050���ֽ�*/
//OS_MEM   *CommMem; 


/* ADD #001 ����ʱ��ػ��� */
#if defined SYSTEM_RESTART__
#undef  RESTART_SYSTEM_COUNTER_MAX_NUMBER
#define RESTART_SYSTEM_COUNTER_MAX_NUMBER      			3   /* ��ʱ��30�� */
#endif  /* SYSTEM_RESTART__ */

vu32 __RestartSystemCounter = 0;
vu32 __RestartGprsCounter = 0;


/*
****************************************************************************
*	�� �� ��: main
*	����˵��: ��׼c������ڡ�
*	��    �Σ���
*	�� �� ֵ: ��
****************************************************************************
*/


int main(void)
{

	/* ��ʼ��"uC/OS-II"�ں� */
	OSInit();
  
	/* ����һ����������Ҳ���������񣩡���������ᴴ�����е�Ӧ�ó������� */
	OSTaskCreateExt(AppTaskStart,	/* ����������ָ�� */
                    (void *)0,		/* ���ݸ�����Ĳ��� */
                    (OS_STK *)&AppTaskStartStk[APP_TASK_START_STK_SIZE - 1], /* ָ������ջջ����ָ�� */
                    APP_TASK_START_PRIO,	/* ��������ȼ�������Ψһ������Խ�����ȼ�Խ�� */
                    APP_TASK_START_PRIO,	/* ����ID��һ����������ȼ���ͬ */
                    (OS_STK *)&AppTaskStartStk[0],/* ָ������ջջ�׵�ָ�롣OS_STK_GROWTH ������ջ�������� */
                    APP_TASK_START_STK_SIZE, /* ����ջ��С */
                    (void *)0,	/* һ���û��ڴ�����ָ�룬����������ƿ�TCB����չ����
                       ���������л�ʱ����CPU����Ĵ��������ݣ���һ�㲻�ã���0���� */
                    OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR); /* ����ѡ���� */
					
					/*  �������£�
						OS_TASK_OPT_STK_CHK      ʹ�ܼ������ջ��ͳ������ջ���õĺ�δ�õ�
						OS_TASK_OPT_STK_CLR      �ڴ�������ʱ����������ջ
						OS_TASK_OPT_SAVE_FP      ���CPU�и���Ĵ��������������л�ʱ���渡��Ĵ���������
					*/                  

	/* ָ����������ƣ����ڵ��ԡ���������ǿ�ѡ�� */
	//OSTaskNameSet(APP_TASK_START_PRIO, APP_TASK_START_NAME, &err);
	
	/*ucosII�Ľ��ļ�������0    ���ļ�������0-4294967295*/ 
	OSTimeSet(0);	
	
	/* ����������ϵͳ������Ȩ����uC/OS-II */
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
*	�� �� ��: AppTaskStart
*	����˵��: ����һ�����������ڶ�����ϵͳ�����󣬱����ʼ���δ������(��BSP_Init��ʵ��)
*	��    �Σ�p_arg ���ڴ���������ʱ���ݵ��β�
*	�� �� ֵ: ��
	�� �� ����1
*********************************************************************************************************
*/
static void AppTaskStart(void *p_arg)
{	
    /* �����ڱ���������澯����������������κ�Ŀ����� */	
    (void)p_arg;   
	
		/* BSP ��ʼ���� BSP = Board Support Package �弶֧�ְ����������Ϊ�ײ�������*/
		bsp_Init();
    CPU_Init();           /* Init CPU name & int. dis. time measuring fncts.          */
		BSP_Tick_Init();
		//Mem_Init();           /* Init Memory Management Module.                           */

	/*���CPU������ͳ��ģ���ʼ�����ú�����������CPUռ���� */
	#if (OS_TASK_STAT_EN > 0)
			OSStatInit();
	#endif
	/*��ջ�������*/
	#if  APP_STACK_CHECK_DEBUG ==1
	AppStackCheckCreate();
	#endif
		
	/* ����Ӧ�ó��������*/
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
*	�� �� ��: AppTaskCreate
*	����˵��: ����Ӧ������
*	��    �Σ�p_arg ���ڴ���������ʱ���ݵ��β�
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void AppTaskCreate (void)
{
	/* �������ʼ�� */
  SHABUF_InitBuffer ();
	
	/* Ӧ�ò�����ȡ */
	ParameterQuery_Init();
  sysParamsInit();
	
	/*����Ӧ�ò���*/
	sysParamsSave();
	
	/* ������������� */
  XMIT_StartManager ();	
	
	/*��ʼ��ģ��*/
	CMD_ApplicationIfInit();
	GPS_ApplicationIfInit();
	TZ_CAN_ApplicationIfInit();
	TZ_UART_ApplicationIfInit();
	
	/*ע��ģ��*/
	USARTx_Start(SHELL_USART, Shell_DataHandler);
	USARTx_Start(GPS_USART,GPS_DataHandler);
	MDM_ResetPIN();
  MODEM_Start();
  USARTx_Start ( ( usart_Flow_e )GSM_USART, MODEM_ReadData );
 
 /*����Ӧ��*/
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
*	�� �� ��: OS_Task_UseRate
*	����˵��: ��uCOS-II������Ϣ��ӡ������
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void OS_Task_UseRate(void)
{
	OS_TCB      *ptcb;	        /* ����һ��������ƿ�ָ��, TCB = TASK CONTROL BLOCK */
	OS_STK_DATA stk;	        /* ���ڶ�ȡ����ջʹ����� */

	ptcb    = &OSTCBTbl[0];		/* ָ���1��������ƿ�(TCB) */

	/* ��ӡ���� */
	printf("  Prio   Used  Free  Per    Taskname\r\n");
	OSTimeDly(10);
	/* ����������ƿ��б�(TCB list)����ӡ���е���������ȼ������� */
	while (ptcb != NULL)
	{
		/* 
			ptcb->OSTCBPrio : ������ƿ��б������������ȼ�
			ptcb->OSTCBTaskName : ������ƿ��б�����������ƣ���Ҫ�ڴ��������ʱ��
			����OSTaskNameSet()ָ���������ƣ����磺
			OSTaskNameSet(APP_TASK_USER_IF_PRIO, "User I/F", &err);
		*/
		OSTaskStkChk(ptcb->OSTCBPrio, &stk);	/* �������ջ���ÿռ� */
		printf("   %2d  %5d  %5d  %02d%%   %s\r\n", ptcb->OSTCBPrio, 
		stk.OSUsed, stk.OSFree, (stk.OSUsed * 100) / (stk.OSUsed + stk.OSFree),
		ptcb->OSTCBTaskName);		
		ptcb = ptcb->OSTCBPrev;		            /* ָ����һ��������ƿ� */
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
  
	/* �͹������� */
	#if __USE_LOW_PWR__ == 1
		LPWR_IsSTOPMode();
	#endif 
	
#endif  /* __USE_LOW_PWR__ */
	

}
#endif



