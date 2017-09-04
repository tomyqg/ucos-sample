
#include "tiza_init.h"
#include "core_cm4.h"
#include "tiza_upgrade.h"
#include "defines.h"
#include "project_configure.h"

/* 
 * 功能描述: 外设初始化
 * 引用参数:
 *          
 * 返回值  :
 * 
 */
extern void System_initProcess ( void )
{

   /* 系统时钟配置 */

	
	 /* Enable CRC clock */
	 RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_CRC, ENABLE);
	
	 NVIC_Configuration();  /* 中断优先级分组配置 */
	
   /* GPIO配置*/
	
   /*LED配置*/
		GPS_LED_Config();
	  WORK_LED_Config();

	 /* USART配置 */
		Debug_ConfigInit(115200);
		GPS_ConfigInit(115200);
		GSM_ConfigInit(115200);
	
#if __USE_RTC__==1
   /*RTC初始化*/
	 TM_RTC_Init(TM_RTC_ClockSource_External);
#endif
		

  /*看门狗配置 */
#if defined  __USE_WDG__
	#if __USE_WDG__ == 1 
		WDG_configuration(); 
	#endif  
#endif  /* __USE_WDG__ */


#if __USE_UPGRADE__ ==1
  /* 升级功能配置 */
  UPGRADE_LowLevelIfInit ();
#endif

  /* CAN配置 */
#if __USE_CAN__ == 1
	CanOpenPort(1,CAN1_BUS_BPS);
	CanOpenPort(2,CAN2_BUS_BPS);
#endif

#if __USE_SPI_1__==1
  /* SPI配置 */
	SPI_FLASH_Init();
#endif

	
#if __USE_FLASH__ ==1	
	/* FLASH配置 */
	/* flash低级接口初始化*/
	FLASH_LowlLevelIfInit();
#endif

#if __USE_TMR__==1
  /* 软定时器接口初始化 */
	TMR_LowLevelIfInit ();
#endif

  /* 工作队列配置 */
#if defined __USE_WORKQUEUE__
  /* 工作队列配置 */
	#if __USE_WORKQUEUE__ == 1
  WORKER_LowLevelIfInit();
	#endif
#endif  /* __USE_WORKQUEUE__ */

  /* 硬件定时器配置 */
	
	
#if __USE_LOW_PWR__ == 1
	
  /* 低功耗配置 */
	LPWR_LowLevelIfInit();
#endif	


  /* 电源电压配置 */
	COM_PowerComtrol_Init();
	BAT_PowerControl_Init();
	CAN_PowerControl_Init();
	GPS_BackBAT_PowerControl_Init();
  /* EXTI配置 */

  /*备份域配置 */
	/*监视器相关配置*/
	
	/*ACC检测*/

  /* ACC配置 */
	TZ_ACC_Init();
  //TM_GPIO_Init(GPIOE,GPIO_Pin_2,TM_GPIO_Mode_IN,TM_GPIO_OType_PP,TM_GPIO_PuPd_NOPULL,TM_GPIO_Speed_Fast);

  /*BAT_ADC检测脚*/
	//TM_GPIO_Init(GPIOB,GPIO_Pin_1,TM_GPIO_Mode_IN,TM_GPIO_OType_PP,TM_GPIO_PuPd_NOPULL,TM_GPIO_Speed_Fast);
	TM_ADC_InitADC(ADC1);
	TM_ADC_Init(ADC1,TM_ADC_Channel_9);
	
	/*DCDC MP4560 后端输出电压检测*/
	TM_GPIO_Init(GPIOD,GPIO_Pin_3,TM_GPIO_Mode_IN,TM_GPIO_OType_PP,TM_GPIO_PuPd_NOPULL,TM_GPIO_Speed_Fast);
	
	/*GPS天线检测*/
	TM_GPIO_Init(GPIOB,GPIO_Pin_0,TM_GPIO_Mode_IN,TM_GPIO_OType_PP,TM_GPIO_PuPd_NOPULL,TM_GPIO_Speed_Fast);
	
	/*SD卡检测*/
	TM_GPIO_Init(GPIOE,GPIO_Pin_14,TM_GPIO_Mode_IN,TM_GPIO_OType_PP,TM_GPIO_PuPd_NOPULL,TM_GPIO_Speed_Fast);
	
	/*GSM_RING脚检测*/
	TM_GPIO_Init(GPIOF,GPIO_Pin_7,TM_GPIO_Mode_IN,TM_GPIO_OType_PP,TM_GPIO_PuPd_NOPULL,TM_GPIO_Speed_Fast);
	
	/*防拆检测*/
  TM_GPIO_Init(GPIOE,GPIO_Pin_15,TM_GPIO_Mode_IN,TM_GPIO_OType_PP,TM_GPIO_PuPd_NOPULL,TM_GPIO_Speed_Fast);
}









/* 
 * 功能描述: 系统内核复位
 * 引用参数:
 *          
 * 返回值  :
 * 
 */
void System_Reset ( void )
{
		/*关闭电源*/
		
		//CAN_POWER_OFF;
		//GPS_POWER_OFF;
		GSM_POWER_OFF;
		//BAT_POWER_OFF;
	  //COM_POWER_OFF;
	
	  NVIC_SystemReset();


} 



