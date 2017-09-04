#ifndef _LED_H_
#define _LED_H_
#include "stm32f4xx.h"


#define  GPS_LED_ON	  		TM_GPIO_SetPinLow(GPIOB,GPIO_Pin_15)
#define	 GPS_LED_OFF	  	TM_GPIO_SetPinHigh(GPIOB,GPIO_Pin_15)
#define  GPS_LED_Toggle  	TM_GPIO_TogglePinValue(GPIOB,GPIO_Pin_15)


#define  WORK_LED_ON	  	TM_GPIO_SetPinLow(GPIOB,GPIO_Pin_14)
#define	 WORK_LED_OFF	  	TM_GPIO_SetPinHigh(GPIOB,GPIO_Pin_14)
#define  WORK_LED_Toggle  TM_GPIO_TogglePinValue(GPIOB,GPIO_Pin_14)
	

extern void GPS_LED_Config(void);
extern void WORK_LED_Config(void);

#endif
