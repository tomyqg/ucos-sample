#include "tiza_power_control.h"


/*
*******************************************************************
*	Fuction_Name£ºGPS_ConfigInit
* Parameter		£ºbaudrate
* Fuction			£ºGPS usart config
*	Author			: goly
* 
*******************************************************************
*/

void GPS_ConfigInit(u32 baudrate)
{
	//config GPS_USART
	TM_USART_Init(UART4,TM_USART_PinsPack_1,baudrate);
	//initation GPS control pin
	TM_GPIO_Init(GPIOD,GPIO_Pin_9,TM_GPIO_Mode_OUT,TM_GPIO_OType_PP,TM_GPIO_PuPd_NOPULL,TM_GPIO_Speed_Fast);
	//close gps control pin
	//TM_GPIO_SetPinLow(GPIOD,GPIO_Pin_9);
}

/*
*******************************************************************
*	Fuction_Name£ºGSM_ConfigInit
* Parameter		£ºbaudrate
* Fuction			£ºGSM usart config
*	Author			: goly
* 
*******************************************************************
*/
void GSM_ConfigInit(u32 baudrate)
{
	//config GSM USART
	TM_USART_Init(USART3,TM_USART_PinsPack_1,baudrate);
	//initation GSM control pin
	TM_GPIO_Init(GPIOD,GPIO_Pin_8,TM_GPIO_Mode_OUT,TM_GPIO_OType_PP,TM_GPIO_PuPd_UP,TM_GPIO_Speed_Fast);
	//close GSM control pin
	TM_GPIO_SetPinLow(GPIOD,GPIO_Pin_8);
}

/*
*******************************************************************
*	Fuction_Name£ºDebug_ConfigInit
* Parameter		£ºbaudrate
* Fuction			£ºDEBUG usart config
*	Author			: goly
* 
*******************************************************************
*/
void Debug_ConfigInit(u32 baudrate)
{
	//config DEBUG USART
	TM_USART_Init(USART1,TM_USART_PinsPack_1,baudrate);
}

/*
*******************************************************************
*	Fuction_Name£ºGPS_BackBAT_PowerControl_Init
* Parameter		£ºnone
* Fuction			£ºinitation GPS Backbattery power control pin
*	Author			: goly
* 
*******************************************************************
*/
void GPS_BackBAT_PowerControl_Init(void)
{
	//initation GPS control pin
	TM_GPIO_Init(GPIOD,GPIO_Pin_11,TM_GPIO_Mode_OUT,TM_GPIO_OType_PP,TM_GPIO_PuPd_NOPULL,TM_GPIO_Speed_Fast);
	//close gps control pin
	TM_GPIO_SetPinLow(GPIOD,GPIO_Pin_11);
	TM_GPIO_SetPinHigh(GPIOD,GPIO_Pin_11);
}



/*
*******************************************************************
*	Fuction_Name£ºGPS_BackBAT_PowerControl_Init
* Parameter		£ºnone
* Fuction			£ºinitation Battery power control pin
*	Author			: goly
* 
*******************************************************************
*/
void BAT_PowerControl_Init(void)
{
	TM_GPIO_Init(GPIOD,GPIO_Pin_10,TM_GPIO_Mode_OUT,TM_GPIO_OType_PP,TM_GPIO_PuPd_NOPULL,TM_GPIO_Speed_Fast);
	TM_GPIO_SetPinLow(GPIOD,GPIO_Pin_10);
}

/*
*******************************************************************
*	Fuction_Name£ºGPS_BackBAT_PowerControl_Init
* Parameter		£ºnone
* Fuction			£ºinitation CAN power control pin
*	Author			: goly
* 
*******************************************************************
*/
void CAN_PowerControl_Init(void)
{
		//initation SD control pin
	TM_GPIO_Init(GPIOC,GPIO_Pin_1,TM_GPIO_Mode_OUT,TM_GPIO_OType_PP,TM_GPIO_PuPd_NOPULL,TM_GPIO_Speed_Fast);
	//clsoe SD control pin
	TM_GPIO_SetPinLow(GPIOC,GPIO_Pin_1);
}
/*
*******************************************************************
*	Fuction_Name£ºGPS_BackBAT_PowerControl_Init
* Parameter		£ºnone
* Fuction			£ºinitation Common power control pin
*	Author			: goly
* 
*******************************************************************
*/
void COM_PowerComtrol_Init(void)
{
	//initation SD control pin
	TM_GPIO_Init(GPIOC,GPIO_Pin_2,TM_GPIO_Mode_OUT,TM_GPIO_OType_PP,TM_GPIO_PuPd_NOPULL,TM_GPIO_Speed_Fast);
	//clsoe SD control pin
	TM_GPIO_SetPinLow(GPIOC,GPIO_Pin_2);
}


void CAN_485_PowerComtrol_Init(void)
{
	//initation SD control pin
//	TM_GPIO_Init(GPIOG,GPIO_Pin_7,TM_GPIO_Mode_OUT,TM_GPIO_OType_PP,TM_GPIO_PuPd_NOPULL,TM_GPIO_Speed_Fast);
//	//clsoe SD control pin
//	TM_GPIO_SetPinLow(GPIOG,GPIO_Pin_7);
//	
//	TM_GPIO_Init(GPIOG,GPIO_Pin_8,TM_GPIO_Mode_OUT,TM_GPIO_OType_PP,TM_GPIO_PuPd_NOPULL,TM_GPIO_Speed_Fast);
//	//clsoe SD control pin
//	TM_GPIO_SetPinLow(GPIOG,GPIO_Pin_8);
}


