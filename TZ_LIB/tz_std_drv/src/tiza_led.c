#include "defines.h"


/* 
 * 功能描述: GPS工作状态指示灯
 * 引用参数:
 *          
 * 返回值  :
 * 
 */
void GPS_LED_Config(void)
{	
		TM_GPIO_Init(GPIOB,GPIO_Pin_15,TM_GPIO_Mode_OUT,TM_GPIO_OType_PP,TM_GPIO_PuPd_NOPULL,TM_GPIO_Speed_Fast);		
		GPS_LED_OFF;
}

/* 
 * 功能描述:终端工作指示灯 
 * 引用参数:
 *          
 * 返回值  :
 * 
 */
void WORK_LED_Config(void)
{
		TM_GPIO_Init(GPIOB,GPIO_Pin_14,TM_GPIO_Mode_OUT,TM_GPIO_OType_PP,TM_GPIO_PuPd_NOPULL,TM_GPIO_Speed_Fast);		
		WORK_LED_OFF;

}




