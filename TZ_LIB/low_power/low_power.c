#include "defines.h"



static ErrorStatus HSEStartUpStatus;

typedef enum
{
  LPWR_STAUTS_NORMAL,   /* ����״̬ */
  LPWR_STAUTS_STOP,     /* ֹͣ״̬ */
  LPWR_STAUTS_STANDBY,  /* ����״̬ */
} LPWR_Status;



typedef struct
{
  /* ״̬ */
  vu8 lpStatus;
  vu8 lpStatusBak;
  
  /* ����ʱ�� */
  TM_RTC_Int_t RTCAlarmValue;
  
  /* ���Ѵ����� */
  lp_processHdlr_f  lpHandler; 
} LPWR_IfTypedef;




/******************************************/
/*              �ڲ�����[����]            */
/******************************************/

static LPWR_IfTypedef Lpwr = { LPWR_STAUTS_NORMAL, LPWR_STAUTS_NORMAL, TM_RTC_Int_Disable, NULL };






/******************************************/
/*              �ڲ�����[����]            */
/******************************************/

static void LPWR_SetSTOPMode ( TM_RTC_Int_t AlarmValue );
static void LPWR_EnterSTOPMode ( void );
static void LPWR_EnterSTANDBYMode ( TM_RTC_Int_t AlarmValue );
static bool LPWR_IsNormalStatus ( void );

extern void MCU_PeriphDriver ( void );
extern void CLOCK_Stop ( void );
extern void ADC_Stop ( void );
extern void ADC_Restart ( void );

extern void RCC_SysClkConfigFromSTOPMode ( void );
extern bool RTC_SetAlarmTimeSlot ( TM_RTC_Int_t AlarmValue );


/* �ͷ�IO�ӿ���Դ */
#define DRV_RESOURCE_STOP() \
{ \
  CLOCK_Stop();  \
}







/* ����IO�ӿ���Դ */
#define DRV_RESOURCE_START() \
{\
  MCU_PeriphDriver(); \
}
 








/* 
 * ��������: Initializes Low Power application.
 * ���ò���:
 *          
 * ����ֵ  :
 * 
 */
extern void LPWR_LowLevelIfInit ( void )
{
  RCC_APB1PeriphClockCmd ( RCC_APB1Periph_PWR, ENABLE );
}













/* 
 * ��������: Initializes Low Power application.
 * ���ò���:
 *          
 * ����ֵ  :
 * 
 */
extern void LPWR_Start ( u8 LpType, lp_processHdlr_f lpHandler )
{
  static TM_RTC_Int_t AlarmValue = TM_RTC_Int_1s; 

  if ( lpHandler == NULL )
  {
    /* ���Ѵ�����Ϊ��ʱ��ֱ���˳� */
    return ;   
  }

#if LPWR_DEBUG == 1
{
  printf ( "\r\n[LPWR EVENT]  set LPWR_Mode type : %u \r\n", LpType  );
}
#endif  /* LPWR_DEBUG */

  Lpwr.lpHandler = ( lp_processHdlr_f )lpHandler;
  switch ( LpType )
  {
    case LPWR_RUN_Mode:   /* ����ģʽ */
      break;

    case LPWR_STOP_Mode:  /* ֹͣģʽ */
      LPWR_SetSTOPMode ( AlarmValue );
      break;

    case LPWR_STANDBY_Mode:  /* ����ģʽ */
      LPWR_EnterSTANDBYMode ( AlarmValue );
      break;
  }
}










/* 
 * ��������: Enters MCU in STOP mode. The wake-up from STOP mode is 
 *           performed by an RTC Alarm.
 * ���ò���:
 *          
 * ����ֵ  :
 * 
 */
static void LPWR_EnterSTOPMode ( void )
{ 
  if ( LPWR_IsNormalStatus () == TRUE )
  {

    /* ״̬���� */
    Lpwr.lpStatus = Lpwr.lpStatusBak;

    /* �ͷ�IO�ӿ���Դ */
    DRV_RESOURCE_STOP ();//��һ�����͹��ģ� ��Ҫ����IO��Ŀǰ��ʹ��@goly

    /* ֪ͨ׼����������״̬ */
    if ( Lpwr.lpHandler )
    {
      Lpwr.lpHandler ( LPWR_STOP_STATE_IS_READY );
    }
  }
}







/* 
 * ��������: Enters MCU in STOP mode. The wake-up from STOP mode is 
 *           performed by an RTC Alarm.
 * ���ò���:
 *          
 * ����ֵ  :
 * 
 */
static void LPWR_SetSTOPMode (const TM_RTC_Int_t AlarmValue )
{
  Lpwr.RTCAlarmValue = AlarmValue;
  Lpwr.lpStatusBak = LPWR_STAUTS_STOP;

/////////////////////////////////////////

  LPWR_OSTaskSuspend ();

/////////////////////////////////////////
}


/* 
 * ��������: Enters MCU in STANDBY mode. The wake-up from STANDBY mode is 
 *           performed by an RTC Alarm event.
 * ���ò���:
 *          
 * ����ֵ  :
 * 
 */
static void LPWR_EnterSTANDBYMode ( const TM_RTC_Int_t AlarmValue )
{
  
   Lpwr.RTCAlarmValue = AlarmValue;
   Lpwr.lpStatusBak = LPWR_STAUTS_STANDBY;
  
   LPWR_OSTaskSuspend ();
  
}



extern  void EnterSTANDBYMode ( TM_RTC_Int_t AlarmValue )
{
  if ( AlarmValue > 0 && RTC_SetAlarmTimeSlot ( AlarmValue ) == FALSE )
  {
	    printf("\r\n  Enter STANDBYMode failed \r\n");
    return ;
  }

  Lpwr.lpStatus = LPWR_STAUTS_STANDBY;

  /* Enable WakeUp pin */
  PWR_WakeUpPinCmd ( ENABLE );
  WDG_KickCmd ();
  /* Request to enter STANDBY mode (Wake Up flag is cleared in PWR_EnterSTANDBYMode function) */
  //printf("\r\n Enter standby mode. %s,%d\r\n",__func__,__LINE__);
  PWR_EnterSTANDBYMode ();
}




  




/* 
 * ��������: �˳�����
 * ���ò���:
 *          
 * ����ֵ  :
 * 
 */
extern void LPWR_ExitProcess ( void )
{
		FlagStatus Status = RESET;
		 
		/* Check if the Wake-Up flag is set */
		Status = PWR_GetFlagStatus ( PWR_FLAG_WU );
		
		if(Status != RESET)
		{
			/* Clear Wake Up flag */
			PWR_ClearFlag ( PWR_FLAG_WU );
		}

		/* Check if the StandBy flag is set */
		Status = PWR_GetFlagStatus ( PWR_FLAG_SB );
		if(Status!= RESET)
		{
			/* System resumed from STANDBY mode */
			/* Clear StandBy flag */
			PWR_ClearFlag(PWR_FLAG_SB );
		}
}







/* 
 * ��������: Enters MCU in STANDBY mode. The wake-up from STANDBY mode is 
 *           performed by an RTC Alarm event.
 * ���ò���:
 *          
 * ����ֵ  :
 * 
 */
static bool LPWR_IsNormalStatus ( void )
{
  return ( Lpwr.lpStatus == LPWR_STAUTS_NORMAL ) ? TRUE : FALSE;
}








/* 
 * ��������: Enters MCU in STANDBY mode. The wake-up from STANDBY mode is 
 *           performed by an RTC Alarm event.
 * ���ò���:
 *          
 * ����ֵ  :
 * 
 */
extern u8 LPWR_GetPowerStatus ( void )
{
  /* ����ϵͳ������״̬ */
  return ( u8 )Lpwr.lpStatusBak; 
}









/* 
 * ��������: Enters MCU in STANDBY mode. The wake-up from STANDBY mode is 
 *           performed by an RTC Alarm event.
 * ���ò���:
 *          
 * ����ֵ  :
 * 
 */
extern void LPWR_CyclicWakeupHandler ( void )
{
  static u32 cyclicCount = ( 2 * OS_TICKS_PER_SEC );

  /* �Ƿ������״̬������ */
  if ( Lpwr.lpStatusBak != Lpwr.lpStatus )
  {
     //printf("\r\n %s,%d Lpwr.lpStatusBak =%x,Lpwr.lpStatus =%x\r\n",__func__,__LINE__,Lpwr.lpStatusBak,Lpwr.lpStatus);
    if ( cyclicCount == 0 )
    {
      cyclicCount = ( 2 * OS_TICKS_PER_SEC );

#if LPWR_DEBUG == 1
      printf ( "\r\n[LPWR EVENT] fake sleep status handler\r\n" );
#endif  /* LPWR_DEBUG */
      if ( Lpwr.lpHandler )
      {
        /* ������״̬�µ͹������ڻ��Ѵ������� */
        Lpwr.lpHandler ( LPWR_STOP_STATE_IS_SLEEPING );
      }
    }
    else
    {
      if ( cyclicCount % 500 == 0 )
      {

////////////////////////////////////////////////////////////////////////////////////

#if defined __USE_WDG__

  /* ���Ź�ιʳ */
#if __USE_WDG__ == 1
        WDG_KickCmd ();
#endif
#endif  /* __USE_WDG__ */

////////////////////////////////////////////////////////////////////////////////////
      
      }
      cyclicCount--;  
    }  
  }
}










/* 
 * ��������: Enters MCU in STOP mode. The wake-up from STOP mode is 
 *           performed by an RTC Alarm.
 * ���ò���:
 *          
 * ����ֵ  :
 * 
 */
extern bool LPWR_IsSTOPMode ( void )
{
	
  if ( Lpwr.lpStatusBak == LPWR_STAUTS_STOP ) 
  {
    if ( Lpwr.RTCAlarmValue > 0 && RTC_SetAlarmTimeSlot ( Lpwr.RTCAlarmValue ) == FALSE )
    {
      /* RTC����ʧЧ�����������״̬��
       * (1)�ر���ʱ����
       * (2)�л�Ϊ�͹������ڻ��Ѵ������̣�
       * */
#if LPWR_DEBUG == 1
{
      printf ( "\r\n[LPWR EVENT] set alarm time error, rtc exception!\r\n" );
}
#endif  /* LPWR_DEBUG */

      return FALSE;
    }
    else
    {
      /* һ������RTC�ɹ��������ENTER STOPģʽ */
      LPWR_EnterSTOPMode();
    }
/*
*====================================================================================
*DCDC��˶ϵ�󣬸�λ
*====================================================================================
*/		
//		uint16_t MP4560_handler=TM_GPIO_GetInputPinValue(GPIOD,GPIO_Pin_3);
//		if(!MP4560_handler)System_Reset();
		

	acc_io_handler=TZ_Get_ACC_State();

	

 //@ 1: ok     0: SMS/phone is arrive
	 gsmring_io_handler=TM_GPIO_GetInputPinValue(GPIOF,GPIO_Pin_7);	

		
	
////////////////////////////////////////////////////////////////////////////////////

#if defined __USE_WDG__

  /* ���Ź�ιʳ */
#if __USE_WDG__ == 1
      WDG_KickCmd ();
#endif
#endif  /* __USE_WDG__ */	
		
////////////////////////////////////////////////////////////////////////////////////
    PWR_EnterSTOPMode( PWR_Regulator_LowPower, PWR_STOPEntry_WFI ); 	
		LPWR_ExitProcess();
////////////////////////////////////////////////////////////////////////////////////

#if defined __USE_WDG__

  /* ���Ź�ιʳ */
#if __USE_WDG__ == 1
    WDG_KickCmd ();
#endif
#endif  /* __USE_WDG__ */

////////////////////////////////////////////////////////////////////////////////////
    if ( Lpwr.lpHandler )
    {
      Lpwr.lpHandler ( LPWR_STOP_STATE_IS_SLEEPING );
    }

#if defined __USE_WDG__

  /* ���Ź�ιʳ */
#if __USE_WDG__ == 1
      WDG_KickCmd ();
#endif
#endif  /* __USE_WDG__ */

    return TRUE;
  }
  
   return FALSE;  
}


extern bool LPWR_IsSTANDBYMode ( void )
{
  if ( Lpwr.lpStatusBak == LPWR_STAUTS_STANDBY ) 
  {
    
////////////////////////////////////////////////////////////////////////////////////

#if defined __USE_WDG__

  /* ���Ź�ιʳ */
#if __USE_WDG__ == 1
      WDG_KickCmd ();
#endif
#endif  /* __USE_WDG__ */


     /* Check if the Wake-Up flag is set */
    PWR_ClearFlag ( PWR_FLAG_WU );
    EnterSTANDBYMode(Lpwr.RTCAlarmValue);


////////////////////////////////////////////////////////////////////////////////////

    LPWR_ExitProcess ();
////////////////////////////////////////////////////////////////////////////////////

#if defined __USE_WDG__

  /* ���Ź�ιʳ */
#if __USE_WDG__ == 1
    WDG_KickCmd ();
#endif
#endif  /* __USE_WDG__ */

////////////////////////////////////////////////////////////////////////////////////


    /* Configures system clock after wake-up from STOP: enable HSE, PLL and select PLL
       as system clock source (HSE and PLL are disabled in STOP mode) */
    if ( Lpwr.lpHandler )
    {
      Lpwr.lpHandler ( LPWR_STOP_STATE_IS_SLEEPING );
    }

#if defined __USE_WDG__

  /* ���Ź�ιʳ */
#if __USE_WDG__ == 1
      WDG_KickCmd ();
#endif
#endif  /* __USE_WDG__ */


    return TRUE;
  }

  return FALSE;  
}










/* 
 * ��������: �ָ���������ģʽ 
 *           
 * ���ò���:
 *          
 * ����ֵ  :
 * 
 */
extern void LPWR_EnterRUNMode ( void )
{
  Lpwr.lpStatusBak = Lpwr.lpStatus = LPWR_STAUTS_NORMAL;
  Lpwr.RTCAlarmValue = TM_RTC_Int_Disable;
	TM_RTC_Interrupts(TM_RTC_Int_Disable);
  accInfo.close_time=0;
	
  if ( Lpwr.lpHandler )
  {
    Lpwr.lpHandler( LPWR_STOP_STATE_IS_RETURN_NORMAL );
    Lpwr.lpHandler = NULL;
  }

  /* ����IO�ӿ���Դ */
  DRV_RESOURCE_START();

/////////////////////////////////////////

  LPWR_OSTaskResume();

/////////////////////////////////////////

}

/*******************************************************************************
* Function Name  : SYSCLKConfig_STOP
* Description    : Configures system clock after wake-up from STOP: enable HSE, PLL
*                  and select PLL as system clock source.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/



void RCC_SysClkConfigFromSTOPMode ( void )
{
	  SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk; /* ʹ�ܵδ�ʱ�� */
		SystemInit();
}


/* 
 * ��������: ��������ʱ���
 * ���ò���:
 *          
 * ����ֵ  :
 * 
 */


bool RTC_SetAlarmTimeSlot ( TM_RTC_Int_t AlarmValue )
{

		if ( RtcTimeStart == TRUE )
		{
			if(Lpwr.lpStatus==LPWR_STAUTS_NORMAL)
			{
					TM_RTC_Interrupts(AlarmValue);
			}
			return TRUE;   
		}

	#if RTC_DEBUG == 1
		else
		{
			printf ( "\r\n[RTC EVENT] rtc isn't ready for use\r\n" );    
		}    
	#endif  /* RTC_DEBUG */

		return FALSE;
}


void CLOCK_Stop(void)
{
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR,ENABLE);
		SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;  

}

void MCU_PeriphDriver(void)
{
	
		SPI_FLASH_Init();

}
