#include "defines.h"
#include "wip_init.h"
#include "tiza_acc.h"
#include "GSM_xx.h"
#include "project_configure.h"





#define TZ_USE_EXTI_TRIGGER     0

/*ACC�����͹�����Чʱ�䣺20*2s*/
/*��ʱ�䣺2s*/
#define LPWR_ACC_CLOSE_TIME          20

/*CAN ��Ĭʱ�� 2*2s*/
/*��ʱ�䣺2s*/
#define LPWR_CAN_CLOSE_TIME          2


extern bool RING_Handler ( u8 ID );

s8  gsmring_io_handler=-1;

/******************************************/
/*              �ڲ�����[����]            */
/******************************************/

static void SLEEP_Delay ( u8 ID );


/******************************************/
/*              �ⲿ����[����]            */
/******************************************/
/*CAN ���ݽ��ռ�����*/
u32 IsLPWR_Counter=0;

/******************************************/
/*              �ڲ�����[����]            */
/******************************************/

/* �˳�����״̬������ */
static s32 wakecount = 0; 

/* ������ʱ��ʱ�� */
static tmr_t *sleepTmr = NULL;


/*CAN�Ƿ����͹���*/
static bool IsCANEnter_LPWR=FALSE;

/*�����*/
static bool MP4560_IsDCDC_PWR=FALSE;
 



/* 
 * ��������: ������-->���ڼ�ذ弶�ĸ���״̬
 * ���ò�������
 * ����    ��2s    
 * ����ֵ  ��״̬��
 * @goly
 */
 
#define MONITOR_PRINT           0
uint16_t BatteryAdcValue=0;
uint32_t BatteryVoltage=0;
extern void TmrMonitorHdlr ( u8 ID )
{

	/*******************************
	 *      CAN ״̬���           *
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
	 *      ACC ״̬���           *
	 *******************************/
#if __USE_ACC__ == 1
		acc_io_handler=TZ_Get_ACC_State();
	 // printf("\r\n acc_io_handler : %d \r\n",acc_io_handler);
#endif
	

	 /*******************************
   *	 BAT_ADC﮵�ص�ѹ���		 *
   *******************************/
	 BatteryAdcValue=TM_ADC_Read(ADC1,TM_ADC_Channel_9);
	 
//	 printf("\r\n BatteryVoltage: %d  \r\n",BatteryAdcValue);

//	 BatteryVoltage=(BatteryAdcValue*3.2f*11);;
//	 
//	 printf("\r\n BatteryVoltage: %d  \r\n",BatteryVoltage);
	 
	 /*******************************
   * DCDC MP4560��������ѹ���  *
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
   *	 GPS����״̬���		 			 *
   *******************************/
	 //@ 1:GPS_Antenna ERROR 0: GPS_Antenna OK
	 uint16_t GPSAntennaValue=TM_GPIO_GetInputPinValue(GPIOF,GPIO_Pin_6);
	 
   /*******************************
   *	 SD�����״̬���		       *
   *******************************/	 
	 //@ 1:SD ERROR 0: SD OK
	 uint16_t SD_StateValue=TM_GPIO_GetInputPinValue(GPIOE,GPIO_Pin_14);
	 
	 /*******************************
   *	 GSM_RING������  		     *
   *******************************/
	  //@ 1: ok     0: SMS/phone is arrive
	 gsmring_io_handler=TM_GPIO_GetInputPinValue(GPIOF,GPIO_Pin_7);
	// printf("\r\nGSM_RING :%d \r\n",gsmring_io_handler);
	 /*******************************
	 *   ������                   *
	 *******************************/	 
	 //@ 1: ok     0: ����	 
	 uint16_t TamperState=TM_GPIO_GetInputPinValue(GPIOE,GPIO_Pin_15);
	 


#if MONITOR_PRINT ==1	
	 printf("\r\nMP4560Value :%d \r\n",MP4560Value);
	 printf("\r\nGPSAntennaValue :%d \r\n",GPSAntennaValue);
	 printf("\r\nSD_StateValue :%d \r\n",SD_StateValue);
	 printf("\r\nGSM_RING :%d \r\n",GSM_RINGValue);
	 printf("\r\nTamperState :%d \r\n",TamperState);
#endif
	/*******************************
   *	 �͹���״̬���		 			   *
   *******************************/
	 SLEEP_TYPEI_IsReady();

}


/* 
 * ����������
 * ���ò�������
 *          
 * ����ֵ  ��״̬��
 * 
 */
extern bool SLEEP_TYPEI_IsReady ( void )
{  

  /**********************************
	 *  �����ж�����:
	 *
   *  �Ƿ�����߹��ܣ�
   *  ACC�ر�����X���ӣ�
   *  
   *  .............
	 *  ����������������� 
	 *  @goly
   **********************************/
	if(acc_io_handler&&IsCANEnter_LPWR)
		accInfo.close_time++;
	else
		accInfo.close_time=0;
	//printf("\r\naccInfo.close_time : %d vehicleParams.sleepMode: %d  \r\n",accInfo.close_time,vehicleParams.sleepMode);
  /* �Ƿ���������? */
#if 0
  if ( ( vehicleParams.sleepMode& SLEEP_STATUS_IS_SET_BIT )  /* ���߹��ܴ� */ 
     &&( accInfo.close_time >= LPWR_ACC_CLOSE_TIME )  /* ACC�ر�ʱ�䳬��60s */ 
		 )
#else
	if (  
     ( accInfo.close_time >= LPWR_ACC_CLOSE_TIME )  /* ACC�ر�ʱ�䳬��60s */ 
		 )
#endif
  {		
			/* ������ʱ���߿��ƶ�ʱ�� */
			if (sleepTmr == NULL )
			{
				vehicles_logout();

				printf ( "\r\n[LPW EVENT] sleepMode: %d  ,close_time:%d  \r\n", vehicleParams.sleepMode,accInfo.close_time );

				printf("\r\n gsmring_io_handler :%d \r\n",gsmring_io_handler);
				/*������������ʱ�����GSMģ��RING�ţ�����ģʽ�Ƿ����� [1������]  */
				/* 1������   0�� ������*/
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
 * �������������߻��Ѻ����� (~2��)
 * ���ò�����
 *          
 * ����ֵ  ��
 * 
 */
static void LPWR_Wakeup_Handler ( u8 state )
{
  if ( ( state == LPWR_STOP_STATE_IS_SLEEPING )  
    && ( SLEEP_TYPEI_IsNeedExit () == TRUE))  /* ACC�رմ��������� */  
  {

/////////////////////////////////////////////////////////////      	
    LPWR_EnterRUNMode ();
/////////////////////////////////////////////////////////////

  }
	else
  if ( state == LPWR_STOP_STATE_IS_RETURN_NORMAL )//��ʼ����Ӧ��
  {

		//GSM_POWER_ON;  //CT_GSMCM
    /*����Ӧ������״̬ */
    sys_status = PROC_SYSTEM_INIT;
		/*���÷�������½״̬*/
		centerSrvStatus = CENTER_SERVER1_LOGIN_BIT;

		/*Ӧ�ò�����ȡ*/
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
 * ������������ʱ���ߺ���
 * ���ò�������
 *          
 * ����ֵ  ��״̬��
 * 
 */
static void SLEEP_Delay ( u8 ID )
{

  SLEEP_TYPEI_IsSleeping();  
}








/* 
 * ����������
 * ���ò�������
 *          
 * ����ֵ  ��״̬��
 * 
 */
extern void SLEEP_TYPEI_IsSleeping ( void )
{
  /* ע��������ʱ��ʱ�� */
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
  /* �Ƿ���������? */
#if 0
  if ( ( vehicleParams.sleepMode & SLEEP_STATUS_IS_SET_BIT )  /* ���߹��ܴ� */ 
     &&( accInfo.close_time >= LPWR_ACC_CLOSE_TIME )  /* ACC�ر�ʱ�䳬��60s */ 
     )  
#else
	if ( /* ���߹��ܴ� */ 
     ( accInfo.close_time >= LPWR_ACC_CLOSE_TIME )  /* ACC�ر�ʱ�䳬��60s */ 
     ) 
#endif
  {
	
		//GPS_BAT_POWER_OFF; //GPS���ݵ��
	
		GPS_POWER_OFF;  //CT_GPS
		CAN_POWER_OFF;  //CHK
		BAT_POWER_OFF;  //CT5V
		//GSM_POWER_OFF;  //CT_GSMCM
		COM_POWER_OFF;  //CHK1
		
		
		
#if GPS_FUNCTION_IS_OFF == 1
		GPS_LED_OFF;//�ر�GPS��
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

    //����ȱʡ 
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
 * ����������
 * ���ò�������
 *          
 * ����ֵ  ��״̬��
 * 
 */
extern bool SLEEP_TYPEI_IsNeedExit ( void )
{

  /**********************************
   * �����ж�������
   *  
   *  ACC�򿪣�
   *  ���յ����ļ���ָ�
   *  �ﵽ���������µķ�����������
   *   
   *  ...............
	 *  ������������
	 *  @goly
   **********************************/

  /* ���ѵ������������� */


#if GPS_FUNCTION_IS_OFF == 1

#endif  /* GPS_FUNCTION_IS_OFF */

  /*��������*/

  if((!acc_io_handler)||(!gsmring_io_handler)) //ACC��������
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
 * ����������
 * ���ò�������
 *          
 * ����ֵ  ��״̬��
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
 * ����������
 * ���ò�������
 *          
 * ����ֵ  ��״̬��
 * 
 */
void Modem_Restart(sys_status_t states)
{

		GSM_POWER_ON;	
		/*����Ӧ������״̬ */
		sys_status=states;
		/*���÷�������½״̬*/
		centerSrvStatus = CENTER_SERVER1_LOGIN_BIT;

		/*Ӧ�ò�����ȡ*/
		sysParamsInit();
	
		MODEM_Start();
}










