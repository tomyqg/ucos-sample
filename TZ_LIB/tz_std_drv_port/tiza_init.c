
#include "tiza_init.h"
#include "core_cm4.h"
#include "tiza_upgrade.h"
#include "defines.h"
#include "project_configure.h"

/* 
 * ��������: �����ʼ��
 * ���ò���:
 *          
 * ����ֵ  :
 * 
 */
extern void System_initProcess ( void )
{

   /* ϵͳʱ������ */

	
	 /* Enable CRC clock */
	 RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_CRC, ENABLE);
	
	 NVIC_Configuration();  /* �ж����ȼ��������� */
	
   /* GPIO����*/
	
   /*LED����*/
		GPS_LED_Config();
	  WORK_LED_Config();

	 /* USART���� */
		Debug_ConfigInit(115200);
		GPS_ConfigInit(115200);
		GSM_ConfigInit(115200);
	
#if __USE_RTC__==1
   /*RTC��ʼ��*/
	 TM_RTC_Init(TM_RTC_ClockSource_External);
#endif
		

  /*���Ź����� */
#if defined  __USE_WDG__
	#if __USE_WDG__ == 1 
		WDG_configuration(); 
	#endif  
#endif  /* __USE_WDG__ */


#if __USE_UPGRADE__ ==1
  /* ������������ */
  UPGRADE_LowLevelIfInit ();
#endif

  /* CAN���� */
#if __USE_CAN__ == 1
	CanOpenPort(1,CAN1_BUS_BPS);
	CanOpenPort(2,CAN2_BUS_BPS);
#endif

#if __USE_SPI_1__==1
  /* SPI���� */
	SPI_FLASH_Init();
#endif

	
#if __USE_FLASH__ ==1	
	/* FLASH���� */
	/* flash�ͼ��ӿڳ�ʼ��*/
	FLASH_LowlLevelIfInit();
#endif

#if __USE_TMR__==1
  /* ��ʱ���ӿڳ�ʼ�� */
	TMR_LowLevelIfInit ();
#endif

  /* ������������ */
#if defined __USE_WORKQUEUE__
  /* ������������ */
	#if __USE_WORKQUEUE__ == 1
  WORKER_LowLevelIfInit();
	#endif
#endif  /* __USE_WORKQUEUE__ */

  /* Ӳ����ʱ������ */
	
	
#if __USE_LOW_PWR__ == 1
	
  /* �͹������� */
	LPWR_LowLevelIfInit();
#endif	


  /* ��Դ��ѹ���� */
	COM_PowerComtrol_Init();
	BAT_PowerControl_Init();
	CAN_PowerControl_Init();
	GPS_BackBAT_PowerControl_Init();
  /* EXTI���� */

  /*���������� */
	/*�������������*/
	
	/*ACC���*/

  /* ACC���� */
	TZ_ACC_Init();
  //TM_GPIO_Init(GPIOE,GPIO_Pin_2,TM_GPIO_Mode_IN,TM_GPIO_OType_PP,TM_GPIO_PuPd_NOPULL,TM_GPIO_Speed_Fast);

  /*BAT_ADC����*/
	//TM_GPIO_Init(GPIOB,GPIO_Pin_1,TM_GPIO_Mode_IN,TM_GPIO_OType_PP,TM_GPIO_PuPd_NOPULL,TM_GPIO_Speed_Fast);
	TM_ADC_InitADC(ADC1);
	TM_ADC_Init(ADC1,TM_ADC_Channel_9);
	
	/*DCDC MP4560 ��������ѹ���*/
	TM_GPIO_Init(GPIOD,GPIO_Pin_3,TM_GPIO_Mode_IN,TM_GPIO_OType_PP,TM_GPIO_PuPd_NOPULL,TM_GPIO_Speed_Fast);
	
	/*GPS���߼��*/
	TM_GPIO_Init(GPIOB,GPIO_Pin_0,TM_GPIO_Mode_IN,TM_GPIO_OType_PP,TM_GPIO_PuPd_NOPULL,TM_GPIO_Speed_Fast);
	
	/*SD�����*/
	TM_GPIO_Init(GPIOE,GPIO_Pin_14,TM_GPIO_Mode_IN,TM_GPIO_OType_PP,TM_GPIO_PuPd_NOPULL,TM_GPIO_Speed_Fast);
	
	/*GSM_RING�ż��*/
	TM_GPIO_Init(GPIOF,GPIO_Pin_7,TM_GPIO_Mode_IN,TM_GPIO_OType_PP,TM_GPIO_PuPd_NOPULL,TM_GPIO_Speed_Fast);
	
	/*������*/
  TM_GPIO_Init(GPIOE,GPIO_Pin_15,TM_GPIO_Mode_IN,TM_GPIO_OType_PP,TM_GPIO_PuPd_NOPULL,TM_GPIO_Speed_Fast);
}









/* 
 * ��������: ϵͳ�ں˸�λ
 * ���ò���:
 *          
 * ����ֵ  :
 * 
 */
void System_Reset ( void )
{
		/*�رյ�Դ*/
		
		//CAN_POWER_OFF;
		//GPS_POWER_OFF;
		GSM_POWER_OFF;
		//BAT_POWER_OFF;
	  //COM_POWER_OFF;
	
	  NVIC_SystemReset();


} 



