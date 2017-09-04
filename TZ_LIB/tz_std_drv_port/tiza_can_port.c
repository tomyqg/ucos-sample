

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


/* ���û�δ��������� �����ʣ����Զ���125kbps */
#ifndef CAN_BUS_BPS
#define CAN_BUS_BPS     ( 125 )
#endif







// u8 * CanSendBuffer;
/******************************************/
/*              �ڲ�����[����]            */
/******************************************/

static bool canDev_procDataHdlr (u32 id , u16 DataSize, u8 *Data );




/*********************/ 
/*    CAN������      */
/*********************/ 

static CAN_Paramdef   canDev_Param = { CAN_BUS_BPS, ( can_procDataHdlr_f )canDev_procDataHdlr };


/***********/
/* CAN���� */
/***********/

/* CAN���ݱ��� */
 can_data_t canBackupData = { FALSE, CAN_TOSEND_BUFFER_LENGTH, {0} };

/* CAN�ϴ����� */
 u8 CanRecvedBuffer[ CAN_RECVED_BUFFER_LENGTH ] = {0};

/* CAN���������� */
 u8 CanToSendBuffer[ CAN_TOSEND_BUFFER_LENGTH ] = {0};



/*********************/ 
/*   CANЭ����Ϣ     */
/*********************/ 

 can_info_t CanInfo = { 0, CAN_LENGTH_BYTES, 0, CanRecvedBuffer };



 
extern CAN_DevTypedef can ; 
/* 
 * ����������ע��CANģ�����ݽ���
 * ���ò�����
             ParamCAN����
 *          
 * ����ֵ  ��״̬��
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
 * ����������CAN controller
 * ���ò�������
 *          
 * ����ֵ  ��״̬��
 * 
 */
extern void TZ_CAN_ApplicationIfInit ( void )
{
  	
  
  bool result = CAN_REV_CallBack_reg (( CAN_Paramdef* )&canDev_Param  );
	
	OSCAN_InitOS();

#if defined DFZL_PROJECT
	DFZL_CAN_Application_Init();//��ʼ�����������ݷ���
#endif  /* DFZL_PROJECT */
#if defined LDFZ_PROJECT
	LDFZ_CAN_Application_Init();//��ʼ�����������ݷ���
#endif  /* LDFZ_PROJECT */


}



/* 
 * ����������CAN controller
 * ���ò�����(1)����
 *           (2)����
 *          
 * ����ֵ  ��״̬��
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
	
/************************CAN�ӿ��{ԇ*********************************/
  return TRUE;  
}


uint8 NssInit(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOF | RCC_AHB1Periph_GPIOD, ENABLE);
	
    //����4��Ӧ���Ÿ���ӳ��
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource0, GPIO_AF_UART4);     //GPIOA0����ΪUART4
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource1, GPIO_AF_UART4);    //GPIOA1����ΪUART4
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; ///CT_GPS, GPS��Դ����
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_NOPULL; 
	GPIO_Init(GPIOD, &GPIO_InitStructure); 
	//ON_GPS_PWR();

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6; ///GPS���߼��߱���
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_NOPULL; 
	GPIO_Init(GPIOF, &GPIO_InitStructure); 
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;    //���ù���
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //����
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3;//��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =3;		//�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
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




