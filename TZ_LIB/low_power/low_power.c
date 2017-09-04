#include "defines.h"



static ErrorStatus HSEStartUpStatus;

typedef enum
{
  LPWR_STAUTS_NORMAL,   /* 正常状态 */
  LPWR_STAUTS_STOP,     /* 停止状态 */
  LPWR_STAUTS_STANDBY,  /* 待机状态 */
} LPWR_Status;



typedef struct
{
  /* 状态 */
  vu8 lpStatus;
  vu8 lpStatusBak;
  
  /* 唤醒时间 */
  TM_RTC_Int_t RTCAlarmValue;
  
  /* 唤醒处理函数 */
  lp_processHdlr_f  lpHandler; 
} LPWR_IfTypedef;




/******************************************/
/*              内部变量[定义]            */
/******************************************/

static LPWR_IfTypedef Lpwr = { LPWR_STAUTS_NORMAL, LPWR_STAUTS_NORMAL, TM_RTC_Int_Disable, NULL };






/******************************************/
/*              内部函数[声明]            */
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


/* 释放IO接口资源 */
#define DRV_RESOURCE_STOP() \
{ \
  CLOCK_Stop();  \
}







/* 启动IO接口资源 */
#define DRV_RESOURCE_START() \
{\
  MCU_PeriphDriver(); \
}
 








/* 
 * 功能描述: Initializes Low Power application.
 * 引用参数:
 *          
 * 返回值  :
 * 
 */
extern void LPWR_LowLevelIfInit ( void )
{
  RCC_APB1PeriphClockCmd ( RCC_APB1Periph_PWR, ENABLE );
}













/* 
 * 功能描述: Initializes Low Power application.
 * 引用参数:
 *          
 * 返回值  :
 * 
 */
extern void LPWR_Start ( u8 LpType, lp_processHdlr_f lpHandler )
{
  static TM_RTC_Int_t AlarmValue = TM_RTC_Int_1s; 

  if ( lpHandler == NULL )
  {
    /* 唤醒处理函数为空时，直接退出 */
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
    case LPWR_RUN_Mode:   /* 运行模式 */
      break;

    case LPWR_STOP_Mode:  /* 停止模式 */
      LPWR_SetSTOPMode ( AlarmValue );
      break;

    case LPWR_STANDBY_Mode:  /* 待机模式 */
      LPWR_EnterSTANDBYMode ( AlarmValue );
      break;
  }
}










/* 
 * 功能描述: Enters MCU in STOP mode. The wake-up from STOP mode is 
 *           performed by an RTC Alarm.
 * 引用参数:
 *          
 * 返回值  :
 * 
 */
static void LPWR_EnterSTOPMode ( void )
{ 
  if ( LPWR_IsNormalStatus () == TRUE )
  {

    /* 状态拷贝 */
    Lpwr.lpStatus = Lpwr.lpStatusBak;

    /* 释放IO接口资源 */
    DRV_RESOURCE_STOP ();//进一步降低功耗， 需要考虑IO，目前待使用@goly

    /* 通知准备进入休眠状态 */
    if ( Lpwr.lpHandler )
    {
      Lpwr.lpHandler ( LPWR_STOP_STATE_IS_READY );
    }
  }
}







/* 
 * 功能描述: Enters MCU in STOP mode. The wake-up from STOP mode is 
 *           performed by an RTC Alarm.
 * 引用参数:
 *          
 * 返回值  :
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
 * 功能描述: Enters MCU in STANDBY mode. The wake-up from STANDBY mode is 
 *           performed by an RTC Alarm event.
 * 引用参数:
 *          
 * 返回值  :
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
 * 功能描述: 退出配置
 * 引用参数:
 *          
 * 返回值  :
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
 * 功能描述: Enters MCU in STANDBY mode. The wake-up from STANDBY mode is 
 *           performed by an RTC Alarm event.
 * 引用参数:
 *          
 * 返回值  :
 * 
 */
static bool LPWR_IsNormalStatus ( void )
{
  return ( Lpwr.lpStatus == LPWR_STAUTS_NORMAL ) ? TRUE : FALSE;
}








/* 
 * 功能描述: Enters MCU in STANDBY mode. The wake-up from STANDBY mode is 
 *           performed by an RTC Alarm event.
 * 引用参数:
 *          
 * 返回值  :
 * 
 */
extern u8 LPWR_GetPowerStatus ( void )
{
  /* 返回系统待运行状态 */
  return ( u8 )Lpwr.lpStatusBak; 
}









/* 
 * 功能描述: Enters MCU in STANDBY mode. The wake-up from STANDBY mode is 
 *           performed by an RTC Alarm event.
 * 引用参数:
 *          
 * 返回值  :
 * 
 */
extern void LPWR_CyclicWakeupHandler ( void )
{
  static u32 cyclicCount = ( 2 * OS_TICKS_PER_SEC );

  /* 是否假休眠状态？？？ */
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
        /* 假休眠状态下低功耗周期唤醒处理例程 */
        Lpwr.lpHandler ( LPWR_STOP_STATE_IS_SLEEPING );
      }
    }
    else
    {
      if ( cyclicCount % 500 == 0 )
      {

////////////////////////////////////////////////////////////////////////////////////

#if defined __USE_WDG__

  /* 看门狗喂食 */
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
 * 功能描述: Enters MCU in STOP mode. The wake-up from STOP mode is 
 *           performed by an RTC Alarm.
 * 引用参数:
 *          
 * 返回值  :
 * 
 */
extern bool LPWR_IsSTOPMode ( void )
{
	
  if ( Lpwr.lpStatusBak == LPWR_STAUTS_STOP ) 
  {
    if ( Lpwr.RTCAlarmValue > 0 && RTC_SetAlarmTimeSlot ( Lpwr.RTCAlarmValue ) == FALSE )
    {
      /* RTC配置失效，进入假休眠状态：
       * (1)关闭软定时功能
       * (2)切换为低功耗周期唤醒处理例程；
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
      /* 一旦配置RTC成功，则可以ENTER STOP模式 */
      LPWR_EnterSTOPMode();
    }
/*
*====================================================================================
*DCDC后端断电后，复位
*====================================================================================
*/		
//		uint16_t MP4560_handler=TM_GPIO_GetInputPinValue(GPIOD,GPIO_Pin_3);
//		if(!MP4560_handler)System_Reset();
		

	acc_io_handler=TZ_Get_ACC_State();

	

 //@ 1: ok     0: SMS/phone is arrive
	 gsmring_io_handler=TM_GPIO_GetInputPinValue(GPIOF,GPIO_Pin_7);	

		
	
////////////////////////////////////////////////////////////////////////////////////

#if defined __USE_WDG__

  /* 看门狗喂食 */
#if __USE_WDG__ == 1
      WDG_KickCmd ();
#endif
#endif  /* __USE_WDG__ */	
		
////////////////////////////////////////////////////////////////////////////////////
    PWR_EnterSTOPMode( PWR_Regulator_LowPower, PWR_STOPEntry_WFI ); 	
		LPWR_ExitProcess();
////////////////////////////////////////////////////////////////////////////////////

#if defined __USE_WDG__

  /* 看门狗喂食 */
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

  /* 看门狗喂食 */
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

  /* 看门狗喂食 */
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

  /* 看门狗喂食 */
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

  /* 看门狗喂食 */
#if __USE_WDG__ == 1
      WDG_KickCmd ();
#endif
#endif  /* __USE_WDG__ */


    return TRUE;
  }

  return FALSE;  
}










/* 
 * 功能描述: 恢复正常运行模式 
 *           
 * 引用参数:
 *          
 * 返回值  :
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

  /* 启动IO接口资源 */
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
	  SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk; /* 使能滴答定时器 */
		SystemInit();
}


/* 
 * 功能描述: 设置闹铃时间槽
 * 引用参数:
 *          
 * 返回值  :
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
