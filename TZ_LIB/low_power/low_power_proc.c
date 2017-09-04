#include "defines.h"
#include "wip_init.h"
#include "tiza_acc.h"
#include "GSM_xx.h"
#include "project_configure.h"





#define TZ_USE_EXTI_TRIGGER     0

/*ACC——低功耗生效时间：20*2s*/
/*基时间：2s*/
#define LPWR_ACC_CLOSE_TIME          20

/*CAN 静默时间 2*2s*/
/*基时间：2s*/
#define LPWR_CAN_CLOSE_TIME          2


extern bool RING_Handler ( u8 ID );

s8  gsmring_io_handler=-1;

/******************************************/
/*              内部函数[声明]            */
/******************************************/

static void SLEEP_Delay ( u8 ID );


/******************************************/
/*              外部变量[定义]            */
/******************************************/
/*CAN 数据接收计数器*/
u32 IsLPWR_Counter=0;

/******************************************/
/*              内部变量[定义]            */
/******************************************/

/* 退出休眠状态计数器 */
static s32 wakecount = 0; 

/* 休眠延时定时器 */
static tmr_t *sleepTmr = NULL;


/*CAN是否进入低功耗*/
static bool IsCANEnter_LPWR=FALSE;

/*外电检测*/
static bool MP4560_IsDCDC_PWR=FALSE;
 



/* 
 * 功能描述: 监视器-->用于监控板级的各种状态
 * 引用参数：无
 * 周期    ：2s    
 * 返回值  ：状态码
 * @goly
 */
 
#define MONITOR_PRINT           0
uint16_t BatteryAdcValue=0;
uint32_t BatteryVoltage=0;
extern void TmrMonitorHdlr ( u8 ID )
{

	/*******************************
	 *      CAN 状态监测           *
	 *******************************/
	 IsLPWR_Counter++;
	 if(IsLPWR_Counter>=LPWR_CAN_CLOSE_TIME)
	 {
			IsCANEnter_LPWR=TRUE;
	 }
	 else
	 {
			IsCANEnter_LPWR=FALSE;
	 }

	 /*******************************
	 *      ACC 状态监测           *
	 *******************************/
#if __USE_ACC__ == 1
		acc_io_handler=TZ_Get_ACC_State();
	 // printf("\r\n acc_io_handler : %d \r\n",acc_io_handler);
#endif
	

	 /*******************************
   *	 BAT_ADC锂电池电压检测		 *
   *******************************/
	 BatteryAdcValue=TM_ADC_Read(ADC1,TM_ADC_Channel_9);
	 
//	 printf("\r\n BatteryVoltage: %d  \r\n",BatteryAdcValue);

//	 BatteryVoltage=(BatteryAdcValue*3.2f*11);;
//	 
//	 printf("\r\n BatteryVoltage: %d  \r\n",BatteryVoltage);
	 
	 /*******************************
   * DCDC MP4560后端输出电压检测  *
   *******************************/
	 //@ 1:output voltage 0: no output
	 uint16_t MP4560Value=TM_GPIO_GetInputPinValue(GPIOD,GPIO_Pin_3);
	 if(MP4560Value)
	 {
				MP4560_IsDCDC_PWR=TRUE;
	 }
	 else
	 {
				BAT_POWER_OFF;
				if(MP4560_IsDCDC_PWR)
				{
						printf ( "\r\n[SYS RESET]: DCDC_PWR type\r\n" );
						System_Reset();
				} 
	 }
	 
	 /*******************************
   *	 GPS天线状态监测		 			 *
   *******************************/
	 //@ 1:GPS_Antenna ERROR 0: GPS_Antenna OK
	 uint16_t GPSAntennaValue=TM_GPIO_GetInputPinValue(GPIOF,GPIO_Pin_6);
	 
   /*******************************
   *	 SD卡检测状态监测		       *
   *******************************/	 
	 //@ 1:SD ERROR 0: SD OK
	 uint16_t SD_StateValue=TM_GPIO_GetInputPinValue(GPIOE,GPIO_Pin_14);
	 
	 /*******************************
   *	 GSM_RING来电监测  		     *
   *******************************/
	  //@ 1: ok     0: SMS/phone is arrive
	 gsmring_io_handler=TM_GPIO_GetInputPinValue(GPIOF,GPIO_Pin_7);
	// printf("\r\nGSM_RING :%d \r\n",gsmring_io_handler);
	 /*******************************
	 *   防拆检测                   *
	 *******************************/	 
	 //@ 1: ok     0: 被拆	 
	 uint16_t TamperState=TM_GPIO_GetInputPinValue(GPIOE,GPIO_Pin_15);
	 


#if MONITOR_PRINT ==1	
	 printf("\r\nMP4560Value :%d \r\n",MP4560Value);
	 printf("\r\nGPSAntennaValue :%d \r\n",GPSAntennaValue);
	 printf("\r\nSD_StateValue :%d \r\n",SD_StateValue);
	 printf("\r\nGSM_RING :%d \r\n",GSM_RINGValue);
	 printf("\r\nTamperState :%d \r\n",TamperState);
#endif
	/*******************************
   *	 低功耗状态监测		 			   *
   *******************************/
	 SLEEP_TYPEI_IsReady();

}


/* 
 * 功能描述：
 * 引用参数：无
 *          
 * 返回值  ：状态码
 * 
 */
extern bool SLEEP_TYPEI_IsReady ( void )
{  

  /**********************************
	 *  休眠判断条件:
	 *
   *  是否打开休眠功能；
   *  ACC关闭已满X分钟；
   *  
   *  .............
	 *  其他休眠条件待添加 
	 *  @goly
   **********************************/
	if(acc_io_handler&&IsCANEnter_LPWR)
		accInfo.close_time++;
	else
		accInfo.close_time=0;
	//printf("\r\naccInfo.close_time : %d vehicleParams.sleepMode: %d  \r\n",accInfo.close_time,vehicleParams.sleepMode);
  /* 是否允许休眠? */
#if 0
  if ( ( vehicleParams.sleepMode& SLEEP_STATUS_IS_SET_BIT )  /* 休眠功能打开 */ 
     &&( accInfo.close_time >= LPWR_ACC_CLOSE_TIME )  /* ACC关闭时间超过60s */ 
		 )
#else
	if (  
     ( accInfo.close_time >= LPWR_ACC_CLOSE_TIME )  /* ACC关闭时间超过60s */ 
		 )
#endif
  {		
			/* 启动延时休眠控制定时器 */
			if (sleepTmr == NULL )
			{
				vehicles_logout();

				printf ( "\r\n[LPW EVENT] sleepMode: %d  ,close_time:%d  \r\n", vehicleParams.sleepMode,accInfo.close_time );

				printf("\r\n gsmring_io_handler :%d \r\n",gsmring_io_handler);
				/*符合休眠条件时，检测GSM模块RING脚，振铃模式是否正常 [1：正常]  */
				/* 1：正常   0： 不正常*/
				if(gsmring_io_handler!=1)
				{

						printf ( "\r\n[SYS RESET]: GSM_RI type( clear not ok)\r\n" );
						System_Reset();
				}
				
				sleepTmr = TMR_Subscribe ( FALSE, 50, TMR_TYPE_100MS, ( tmr_procTriggerHdlr_t )SLEEP_Delay );
				return TRUE; 
			} 
  }
  return FALSE;
}







extern void GPIO_ApplicationReset ( bool set );
/* 
 * 功能描述：休眠唤醒后处理函数 (~2秒)
 * 引用参数：
 *          
 * 返回值  ：
 * 
 */
static void LPWR_Wakeup_Handler ( u8 state )
{
  if ( ( state == LPWR_STOP_STATE_IS_SLEEPING )  
    && ( SLEEP_TYPEI_IsNeedExit () == TRUE))  /* ACC关闭传输间隔唤醒 */  
  {

/////////////////////////////////////////////////////////////      	
    LPWR_EnterRUNMode ();
/////////////////////////////////////////////////////////////

  }
	else
  if ( state == LPWR_STOP_STATE_IS_RETURN_NORMAL )//开始启动应用
  {

		//GSM_POWER_ON;  //CT_GSMCM
    /*设置应用运行状态 */
    sys_status = PROC_SYSTEM_INIT;
		/*设置服务器登陆状态*/
		centerSrvStatus = CENTER_SERVER1_LOGIN_BIT;

		/*应用参数获取*/
		DeadData_FlashConfigInit();
		//sysParamsInit();
		
		result=f_mount(0, &fats);
    printf ( "[LPW EVENT] DEIVICE is wake-up\r\n" );
#if GSM_FUNCTION_IS_OFF == 1
		MODEM_Start();
#endif  /* GSM_FUNCTION_IS_OFF */		
	
  }

}








/* 
 * 功能描述：定时休眠函数
 * 引用参数：无
 *          
 * 返回值  ：状态码
 * 
 */
static void SLEEP_Delay ( u8 ID )
{

  SLEEP_TYPEI_IsSleeping();  
}








/* 
 * 功能描述：
 * 引用参数：无
 *          
 * 返回值  ：状态码
 * 
 */
extern void SLEEP_TYPEI_IsSleeping ( void )
{
  /* 注销休眠延时定时器 */
  if ( sleepTmr )
  {
    TMR_UnSubscribe ( sleepTmr, ( tmr_procTriggerHdlr_t )SLEEP_Delay, TMR_TYPE_100MS );
    sleepTmr = NULL;
		
		if(tmrRealData)
		TMR_UnSubscribe(tmrRealData,( tmr_procTriggerHdlr_t )TmrRealDataHdlr,TMR_TYPE_10MS);
		tmrRealData = NULL;
		
		if(tmrOffsetData)
		TMR_UnSubscribe(tmrOffsetData,( tmr_procTriggerHdlr_t )TmrOffsetDataHdlr,TMR_TYPE_10MS);
		tmrOffsetData = NULL;

  }
  /* 是否允许休眠? */
#if 0
  if ( ( vehicleParams.sleepMode & SLEEP_STATUS_IS_SET_BIT )  /* 休眠功能打开 */ 
     &&( accInfo.close_time >= LPWR_ACC_CLOSE_TIME )  /* ACC关闭时间超过60s */ 
     )  
#else
	if ( /* 休眠功能打开 */ 
     ( accInfo.close_time >= LPWR_ACC_CLOSE_TIME )  /* ACC关闭时间超过60s */ 
     ) 
#endif
  {
	
		//GPS_BAT_POWER_OFF; //GPS备份电池
	
		GPS_POWER_OFF;  //CT_GPS
		CAN_POWER_OFF;  //CHK
		BAT_POWER_OFF;  //CT5V
		//GSM_POWER_OFF;  //CT_GSMCM
		COM_POWER_OFF;  //CHK1
		
		
		
#if GPS_FUNCTION_IS_OFF == 1
		GPS_LED_OFF;//关闭GPS灯
#endif  /* GPS_FUNCTION_IS_OFF */

		printf("\r\n SLEEP_TYPEI_IsSleeping   \r\n");
#if GSM_FUNCTION_IS_OFF == 1
		XMIT_Stop();
    wip_NetExit();
		MODEM_Reset();		
#endif  /* GSM_FUNCTION_IS_OFF */

#if CAN_FUNCTION_IS_OFF ==1	
    CAN_Sleep(CAN1);
		CAN_Sleep(CAN2);	
#endif

    //串口缺省 
		USART_DeInit(USART2);
		//USART_DeInit(USART3);
		USART_DeInit(UART4);
		//GPS
		TM_GPIO_Init(GPIOA,GPIO_Pin_0,TM_GPIO_Mode_OUT,TM_GPIO_OType_PP,TM_GPIO_PuPd_NOPULL,TM_GPIO_Speed_Fast);
		TM_GPIO_Init(GPIOA,GPIO_Pin_1,TM_GPIO_Mode_OUT,TM_GPIO_OType_PP,TM_GPIO_PuPd_NOPULL,TM_GPIO_Speed_Fast);
		TM_GPIO_SetPinLow(GPIOA,GPIO_Pin_0);
		TM_GPIO_SetPinLow(GPIOA,GPIO_Pin_1);
		//CAN1
		TM_GPIO_Init(GPIOA,GPIO_Pin_11,TM_GPIO_Mode_OUT,TM_GPIO_OType_PP,TM_GPIO_PuPd_NOPULL,TM_GPIO_Speed_Fast);
		TM_GPIO_Init(GPIOA,GPIO_Pin_12,TM_GPIO_Mode_OUT,TM_GPIO_OType_PP,TM_GPIO_PuPd_NOPULL,TM_GPIO_Speed_Fast);
		TM_GPIO_SetPinLow(GPIOA,GPIO_Pin_11);
		TM_GPIO_SetPinLow(GPIOA,GPIO_Pin_12);
		//CAN2
		TM_GPIO_Init(GPIOB,GPIO_Pin_12,TM_GPIO_Mode_OUT,TM_GPIO_OType_PP,TM_GPIO_PuPd_NOPULL,TM_GPIO_Speed_Fast);
		TM_GPIO_Init(GPIOB,GPIO_Pin_13,TM_GPIO_Mode_OUT,TM_GPIO_OType_PP,TM_GPIO_PuPd_NOPULL,TM_GPIO_Speed_Fast);
		TM_GPIO_SetPinLow(GPIOB,GPIO_Pin_12);
		TM_GPIO_SetPinLow(GPIOB,GPIO_Pin_13);
		
		
		//485
		TM_GPIO_Init(GPIOA,GPIO_Pin_2,TM_GPIO_Mode_OUT,TM_GPIO_OType_PP,TM_GPIO_PuPd_NOPULL,TM_GPIO_Speed_Fast);
		TM_GPIO_Init(GPIOA,GPIO_Pin_3,TM_GPIO_Mode_OUT,TM_GPIO_OType_PP,TM_GPIO_PuPd_NOPULL,TM_GPIO_Speed_Fast);
		TM_GPIO_Init(GPIOG,GPIO_Pin_5,TM_GPIO_Mode_OUT,TM_GPIO_OType_PP,TM_GPIO_PuPd_NOPULL,TM_GPIO_Speed_Fast);
		TM_GPIO_SetPinLow(GPIOG,GPIO_Pin_5);
		TM_GPIO_SetPinLow(GPIOA,GPIO_Pin_2);
		TM_GPIO_SetPinLow(GPIOA,GPIO_Pin_3);


		
    LPWR_Start( LPWR_STOP_Mode, ( lp_processHdlr_f )LPWR_Wakeup_Handler );
  }
}



/* 
 * 功能描述：
 * 引用参数：无
 *          
 * 返回值  ：状态码
 * 
 */
extern bool SLEEP_TYPEI_IsNeedExit ( void )
{

  /**********************************
   * 唤醒判断条件：
   *  
   *  ACC打开；
   *  接收到中心激活指令；
   *  达到休眠条件下的发送数据条件
   *   
   *  ...............
	 *  其他唤醒条件
	 *  @goly
   **********************************/

  /* 唤醒倒数计数器控制 */


#if GPS_FUNCTION_IS_OFF == 1

#endif  /* GPS_FUNCTION_IS_OFF */

  /*唤醒条件*/

  if((!acc_io_handler)||(!gsmring_io_handler)) //ACC唤醒条件
	{	
		RCC_SysClkConfigFromSTOPMode();

    
    if(gsmring_io_handler==0)
		{
				System_Reset();
		}

		COM_POWER_ON;  //CHK1
		CAN_POWER_ON;  //CHK
		//BAT_POWER_ON;  //CT5V
		GPS_BAT_POWER_ON;
    GPS_POWER_ON;  //CT_GPS
		
#if GSM_FUNCTION_IS_OFF == 1

#endif  /* GSM_FUNCTION_IS_OFF */		

		
#if GPS_FUNCTION_IS_OFF == 1	
		GPS_ConfigInit(115200);	
#endif  /* GPS_FUNCTION_IS_OFF */
	
	
#if  CAN_FUNCTION_IS_OFF==1
		CanOpenPort(1,CAN1_BUS_BPS);
	  CanOpenPort(2,CAN2_BUS_BPS);
#endif
		
    return TRUE;
  }

    
  return FALSE;
}




/* 
 * 功能描述：
 * 引用参数：无
 *          
 * 返回值  ：状态码
 * 
 */

void Modem_Stop(sys_status_t states)
{	
		sys_status=states;
	  IsDeadZone_Network=states;
		GSM_POWER_OFF;
		XMIT_Stop();
		wip_NetExit();
		MODEM_Reset();
}

/* 
 * 功能描述：
 * 引用参数：无
 *          
 * 返回值  ：状态码
 * 
 */
void Modem_Restart(sys_status_t states)
{

		GSM_POWER_ON;	
		/*设置应用运行状态 */
		sys_status=states;
		/*设置服务器登陆状态*/
		centerSrvStatus = CENTER_SERVER1_LOGIN_BIT;

		/*应用参数获取*/
		sysParamsInit();
	
		MODEM_Start();
}










