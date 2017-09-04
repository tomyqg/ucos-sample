

#include "stm32f4xx_can.h"

#include <stdio.h>
#include <string.h>
#include "project_configure.h"
#include "tiza_can_port.h"
#include "os_can.h"


#if CAN_CONTROLLER_ID < 0
#error ***ERROR can controller ID MUST >= 0 ***
#endif

#if CAN_LENGTH_BYTES <= 0
#error  ***ERROR can controller length field MUST >= 1 ***
#endif


/* 若用户未定义控制器 波特率，则自定义125kbps */
#ifndef CAN_BUS_BPS
#define CAN_BUS_BPS     ( 125 )
#endif







// u8 * CanSendBuffer;
/******************************************/
/*              内部函数[声明]            */
/******************************************/

static bool canDev_procDataHdlr (u32 id , u16 DataSize, u8 *Data );




/*********************/ 
/*    CAN控制器      */
/*********************/ 

static CAN_Paramdef   canDev_Param = { CAN_BUS_BPS, ( can_procDataHdlr_f )canDev_procDataHdlr };


/***********/
/* CAN缓冲 */
/***********/

/* CAN数据备份 */
 can_data_t canBackupData = { FALSE, CAN_TOSEND_BUFFER_LENGTH, {0} };

/* CAN上传数据 */
 u8 CanRecvedBuffer[ CAN_RECVED_BUFFER_LENGTH ] = {0};

/* CAN待发送数据 */
 u8 CanToSendBuffer[ CAN_TOSEND_BUFFER_LENGTH ] = {0};



/*********************/ 
/*   CAN协议信息     */
/*********************/ 

 can_info_t CanInfo = { 0, CAN_LENGTH_BYTES, 0, CanRecvedBuffer };



 
extern CAN_DevTypedef can ; 
/* 
 * 功能描述：注册CAN模块数据接收
 * 引用参数：
             ParamCAN参数
 *          
 * 返回值  ：状态码
 * 
 */
extern bool CAN_REV_CallBack_reg(CAN_Paramdef * Param )
{
	
	//if(Param->u32Baudrate)
	{
			can.DataHandler= Param->DataHandler;
	}
	return true;
} 
 
 
 
 
 

/* 
 * 功能描述：CAN controller
 * 引用参数：无
 *          
 * 返回值  ：状态码
 * 
 */
extern void TZ_CAN_ApplicationIfInit ( void )
{
  	
  
  bool result = CAN_REV_CallBack_reg (( CAN_Paramdef* )&canDev_Param  );
	
	OSCAN_InitOS();

#if defined DFZL_PROJECT
	DFZL_CAN_Application_Init();//初始化、、、数据发送
#endif  /* DFZL_PROJECT */
#if defined LDFZ_PROJECT
	LDFZ_CAN_Application_Init();//初始化、、、数据发送
#endif  /* LDFZ_PROJECT */


}



/* 
 * 功能描述：CAN controller
 * 引用参数：(1)长度
 *           (2)数据
 *          
 * 返回值  ：状态码
 * 
 */
static bool canDev_procDataHdlr (u32 id ,u16 DataSize, u8 *IPData )
{
#if defined DFZL_PROJECT
  DFZL_CAN_RecvDataHdlr ( id, DataSize, IPData );
#endif  /* DFZL_PROJECT */

#if defined LDFZ_PROJECT
  LDFZ_CAN_RecvDataHdlr ( id, DataSize, IPData );
#endif  /* LDFZ_PROJECT */
	
/************************CAN接口調試*********************************/
  return TRUE;  
}


uint8 NssInit(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOF | RCC_AHB1Periph_GPIOD, ENABLE);
	
    //串口4对应引脚复用映射
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource0, GPIO_AF_UART4);     //GPIOA0复用为UART4
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource1, GPIO_AF_UART4);    //GPIOA1复用为UART4
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; ///CT_GPS, GPS电源控制
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_NOPULL; 
	GPIO_Init(GPIOD, &GPIO_InitStructure); 
	//ON_GPS_PWR();

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6; ///GPS天线剪线报警
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_NOPULL; 
	GPIO_Init(GPIOF, &GPIO_InitStructure); 
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;    //复用功能
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //上拉
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =3;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);
			
	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(UART4, &USART_InitStructure);
	USART_ITConfig(UART4, USART_IT_RXNE, ENABLE);

	USART_Cmd(UART4, ENABLE);
	
	return 0;
}




