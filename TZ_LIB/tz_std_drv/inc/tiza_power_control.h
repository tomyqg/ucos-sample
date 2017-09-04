#ifndef _POWER_CONTROL_H
#define _POWER_CONTROL_H
#include "defines.h"


/*���õ�Դ�������� ��SD����Դ���ƽ� ��232��Դ���ƽš�485��Դ���ƽ� */
#define  	COM_POWER_ON     TM_GPIO_SetPinHigh(GPIOC,GPIO_Pin_2)	
#define  	COM_POWER_OFF    TM_GPIO_SetPinLow(GPIOC,GPIO_Pin_2)

/*CAN��Դ��*/
#define  	CAN_POWER_ON     TM_GPIO_SetPinHigh(GPIOC,GPIO_Pin_1)	
#define  	CAN_POWER_OFF    TM_GPIO_SetPinLow(GPIOC,GPIO_Pin_1)

/*GPS��Դ��*/
#define  	GPS_POWER_ON     TM_GPIO_SetPinHigh(GPIOD,GPIO_Pin_9)	
#define  	GPS_POWER_OFF    TM_GPIO_SetPinLow(GPIOD,GPIO_Pin_9)

/*GSM��Դ��*/
#define  	GSM_POWER_ON     TM_GPIO_SetPinHigh(GPIOD,GPIO_Pin_8)	
#define  	GSM_POWER_OFF    TM_GPIO_SetPinLow(GPIOD,GPIO_Pin_8)
/*﮵�س��IC��Դ���ƽ�*/
#define  	BAT_POWER_ON     TM_GPIO_SetPinHigh(GPIOD,GPIO_Pin_10)
#define  	BAT_POWER_OFF    TM_GPIO_SetPinLow(GPIOD,GPIO_Pin_10)

#define  	GPS_BAT_POWER_ON     TM_GPIO_SetPinHigh(GPIOD,GPIO_Pin_11)
#define  	GPS_BAT_POWER_OFF    TM_GPIO_SetPinLow(GPIOD,GPIO_Pin_11)
	

void Debug_ConfigInit(u32 baudrate);
void GPS_ConfigInit(u32 baudrate);
void GSM_ConfigInit(u32 baudrate);

void CAN_485_PowerComtrol_Init(void);
void COM_PowerComtrol_Init(void);
void BAT_PowerControl_Init(void);
void CAN_PowerControl_Init(void);
void GPS_BackBAT_PowerControl_Init(void);

#endif


