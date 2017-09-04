

#include "tiza_can.h"
//#include "tiza_selfdef.h"
#include "stm32f4xx_can.h"

#define CAN_DEBUG

#define ON_CAN_PWR()		(GPIO_SetBits(GPIOC, GPIO_Pin_1))
#define OFF_CAN_PWR()		(GPIO_ResetBits(GPIOC, GPIO_Pin_1))

static uint8 CanFilterClear(uint8 port);
static uint8 CanFilterSet(uint8 port,uint8 filter_index,uint32 can_id_1,uint32 can_id_2);

// wei chai offer
extern void CanRecMessage(uint8 port,uint32 CanId,uint8* Dat);

static bool can1_inited=FALSE, can2_inited=FALSE;

	
uint8 CanFilterClear(uint8 port)
{
	CAN_FilterInitTypeDef  CAN_FilterInitStructure;
	
	if((port != 1)&&(port != 2))
	{
		return 1;
	}
	CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdMask;
	CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit;
	CAN_FilterInitStructure.CAN_FilterIdHigh = 0x0000;
	CAN_FilterInitStructure.CAN_FilterIdLow = 0x0000;
	CAN_FilterInitStructure.CAN_FilterMaskIdHigh = 0x0000;
	CAN_FilterInitStructure.CAN_FilterMaskIdLow = 0x0000;
	CAN_FilterInitStructure.CAN_FilterActivation = ENABLE;
	
	if(port == 1)
	{
		CAN_FilterInitStructure.CAN_FilterNumber = 0;
		CAN_FilterInitStructure.CAN_FilterFIFOAssignment = CAN_FIFO0;   //关联FIFO0
	}
	else
	{
		CAN_FilterInitStructure.CAN_FilterNumber = 14;
		CAN_FilterInitStructure.CAN_FilterFIFOAssignment = CAN_FIFO1;   //关联FIFO1
	}
	
	CAN_FilterInit(&CAN_FilterInitStructure);
	
	return 0;
}
static uint8 CanFilterSet(uint8 port,uint8 filter_index,uint32 can_id_1,uint32 can_id_2)
{
	u32 tmp_u32_val;
	CAN_FilterInitTypeDef  CAN_FilterInitStructure;
	
	if((port != 1)&&(port != 2))
	{
		return 1;
	}
	if(port == 1)
	{
		if(filter_index > 13)
		{
			return 1;
		}
	}
	if(port == 2)
	{
		if((filter_index < 14)||(filter_index > 27))
		{
			return 1;
		}
	}
	CAN_FilterInitStructure.CAN_FilterNumber = filter_index;	
	CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdList;
	CAN_FilterInitStructure.CAN_FilterScale=CAN_FilterScale_32bit;	

	tmp_u32_val = can_id_1;
	tmp_u32_val = tmp_u32_val << 3;
	tmp_u32_val |= 0x04;
	CAN_FilterInitStructure.CAN_FilterIdHigh = tmp_u32_val >> 16;			
	CAN_FilterInitStructure.CAN_FilterIdLow = tmp_u32_val;
	
	tmp_u32_val = can_id_2;
	tmp_u32_val = tmp_u32_val << 3;
	tmp_u32_val |= 0x04;
	CAN_FilterInitStructure.CAN_FilterMaskIdHigh = tmp_u32_val >> 16;
	CAN_FilterInitStructure.CAN_FilterMaskIdLow = tmp_u32_val;
	
	if(port == 1)
	{
		CAN_FilterInitStructure.CAN_FilterFIFOAssignment=CAN_FIFO0;	
	}
	else
	{
		CAN_FilterInitStructure.CAN_FilterFIFOAssignment = CAN_FIFO1;
	}
	CAN_FilterInitStructure.CAN_FilterActivation=ENABLE;
	CAN_FilterInit(&CAN_FilterInitStructure);
	
	return 0;
}

#if 0
uint8 CanSendBufferEmpty(uint8 Port)
{
	uint8_t transmit_mailbox = 0;

	if(Port != 1 && Port != 2)
	{
		return 0;
	}

	if(Port == 1)
	{
		/* Select one empty transmit mailbox */
		if ((CAN1->TSR&CAN_TSR_TME0) == CAN_TSR_TME0)
		{
			transmit_mailbox = 0;
		}
		else if ((CAN1->TSR&CAN_TSR_TME1) == CAN_TSR_TME1)
		{
			transmit_mailbox = 1;
		}
		else if ((CAN1->TSR&CAN_TSR_TME2) == CAN_TSR_TME2)
		{
			transmit_mailbox = 2;
		}
		else
		{
			transmit_mailbox = CAN_TxStatus_NoMailBox;
		}
	}

	if(Port == 2)
	{
		/* Select one empty transmit mailbox */
		if ((CAN2->TSR&CAN_TSR_TME0) == CAN_TSR_TME0)
		{
			transmit_mailbox = 0;
		}
		else if ((CAN2->TSR&CAN_TSR_TME1) == CAN_TSR_TME1)
		{
			transmit_mailbox = 1;
		}
		else if ((CAN2->TSR&CAN_TSR_TME2) == CAN_TSR_TME2)
		{
			transmit_mailbox = 2;
		}
		else
		{
			transmit_mailbox = CAN_TxStatus_NoMailBox;
		}
	}

	if(transmit_mailbox == CAN_TxStatus_NoMailBox)
	{
		return 0;
	}
	else
	{
		return 1;
	}
}
#endif
	
uint8 CanSendMessage(uint8 port,uint32 can_id,uint8 id_type,uint8* data, uint8 len)
{
	CanTxMsg TxMessage;
	
	if((port != 1)&&(port != 2))
	{
		return 1;
	}

	if(len > 8)
	{
		return 1;
	}

	if(id_type != 1 && id_type != 2)
	{
		return 1;
	}

	if(id_type == 1)
	{
		TxMessage.IDE = CAN_ID_STD;
		TxMessage.StdId = can_id;
	}
	else
	{
		TxMessage.IDE = CAN_ID_EXT;   
		TxMessage.ExtId = can_id;
	}
	
	TxMessage.RTR = CAN_RTR_DATA;
	TxMessage.DLC = len;
	memcpy(TxMessage.Data,data,len);
	
	if(port == 1)
	{
		CAN_Transmit(CAN1, &TxMessage);
	}
	else
	{
		CAN_Transmit(CAN2, &TxMessage);
	}
	
	return 0;
}
extern RING_T RingInBuffer;

CanRxMsg can_msg1;
void CAN1_RX0_IRQHandler(void)
{
	CAN_Receive(CAN1,CAN_FIFO0,&can_msg1);
  rng_enqueue (can_msg1, &RingInBuffer ); /* 入队列 */
	CAN_FIFORelease(CAN1,CAN_FIFO0);
	OSCAN_SemPost();
}
CanRxMsg can_msg2;
void CAN2_RX1_IRQHandler()
{
	CAN_Receive(CAN2, CAN_FIFO1, &can_msg2);
  rng_enqueue ( can_msg2, &RingInBuffer ); /* 入队列 */
	CAN_FIFORelease(CAN2,CAN_FIFO1);
	OSCAN_SemPost();
}

uint8 CanClosePort(uint8 port)
{
	if((port != 1)&&(port != 2))
	{
		return 1;
	}
	if(port ==1)
	{
		CAN_ITConfig(CAN1, CAN_IT_FMP0, DISABLE);
		CAN_DeInit(CAN1);

		can1_inited = FALSE;
	}
	else
	{
		CAN_ITConfig(CAN2, CAN_IT_FMP1, DISABLE);
		CAN_DeInit(CAN2);

		can2_inited = FALSE;
	}

	if(can1_inited == FALSE && can2_inited == FALSE)
	{
		OFF_CAN_PWR();
	}
	
	return 0;
}    
uint8 CanOpenPort(uint8 port,uint32 bps)
{
	uint32 temp_bps;
	
	GPIO_InitTypeDef GPIO_InitStructure;
	CAN_InitTypeDef CAN_InitStructure;
	RCC_ClocksTypeDef RCC_ClocksStatus;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	if((port != 1)&&(port != 2))
	{
		return 1;
	}

	// wei chai only this 3 speeds
	if(bps != 250 && bps != 500 && bps != 1000)
	{
		return 1;
	}

	temp_bps = bps * 1000;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);

	///can_pwr
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType=GPIO_OType_PP;  //推挽输出
	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_NOPULL; 
	GPIO_Init(GPIOC, &GPIO_InitStructure); 

	ON_CAN_PWR();
	
	if(port == 1)
	{
	    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);

		//CAN1对应引脚复用映射
		GPIO_PinAFConfig(GPIOA, GPIO_PinSource11, GPIO_AF_CAN1); //GPIOA11复用为CAN1
		GPIO_PinAFConfig(GPIOA, GPIO_PinSource12, GPIO_AF_CAN1); //GPIOA12复用为CAN1
		
		///Configure CAN1 pin: TX RX
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_11;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //上拉
		GPIO_Init(GPIOA, &GPIO_InitStructure);

		RCC_GetClocksFreq(&RCC_ClocksStatus);
		CAN_DeInit(CAN1);
		CAN_StructInit(&CAN_InitStructure);
		/// CAN1 cell init
		CAN_InitStructure.CAN_TTCM=DISABLE;
		CAN_InitStructure.CAN_ABOM=ENABLE;
		CAN_InitStructure.CAN_AWUM=DISABLE;
		CAN_InitStructure.CAN_NART=DISABLE;
		CAN_InitStructure.CAN_RFLM=DISABLE;
		CAN_InitStructure.CAN_TXFP=DISABLE;
		CAN_InitStructure.CAN_Mode=CAN_Mode_Normal;
		CAN_InitStructure.CAN_SJW=CAN_SJW_1tq;
		CAN_InitStructure.CAN_BS1=CAN_BS1_7tq;
		CAN_InitStructure.CAN_BS2=CAN_BS2_6tq;

		CAN_InitStructure.CAN_Prescaler=RCC_ClocksStatus.PCLK1_Frequency/(14*temp_bps);      ///波特率计算，42M/（1+7+6）/12=250k
		if(!CAN_Init(CAN1, &CAN_InitStructure))
		{
			///CAN1初始化失败
			return 1;
		}
		
		CAN_ITConfig(CAN1, CAN_IT_FMP0, ENABLE);
		
		NVIC_InitStructure.NVIC_IRQChannel = CAN1_RX0_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3;//抢占优先级3
		NVIC_InitStructure.NVIC_IRQChannelSubPriority =3;		//子优先级3
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&NVIC_InitStructure);

		CanFilterClear(1);

		can1_inited = TRUE;
	}
	else
	{
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
		// if using can2 only, should enable can1 clock
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN2 | RCC_APB1Periph_CAN1, ENABLE);

		//CAN2对应引脚复用映射
		GPIO_PinAFConfig(GPIOB, GPIO_PinSource13, GPIO_AF_CAN2); //GPIOA13复用为CAN2
		GPIO_PinAFConfig(GPIOB, GPIO_PinSource12, GPIO_AF_CAN2); //GPIOA12复用为CAN2
		
		///Configure can2 pin: TX RX
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_12;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //上拉
		GPIO_Init(GPIOB, &GPIO_InitStructure);

	    RCC_GetClocksFreq(&RCC_ClocksStatus);
		CAN_DeInit(CAN2);
		CAN_StructInit(&CAN_InitStructure);
		/// CAN2 cell init
		CAN_InitStructure.CAN_TTCM=DISABLE;
		CAN_InitStructure.CAN_ABOM=ENABLE;
		CAN_InitStructure.CAN_AWUM=DISABLE;
		CAN_InitStructure.CAN_NART=DISABLE;
		CAN_InitStructure.CAN_RFLM=DISABLE;
		CAN_InitStructure.CAN_TXFP=DISABLE;
		CAN_InitStructure.CAN_Mode=CAN_Mode_Normal;
		CAN_InitStructure.CAN_SJW=CAN_SJW_1tq;
		CAN_InitStructure.CAN_BS1=CAN_BS1_7tq;
		CAN_InitStructure.CAN_BS2=CAN_BS2_6tq;

		CAN_InitStructure.CAN_Prescaler=RCC_ClocksStatus.PCLK1_Frequency/(14*temp_bps);      ///波特率计算，42M/（1+7+6）/12=250k
		if(!CAN_Init(CAN2, &CAN_InitStructure))
		{
			///CAN2初始化失败
			return 1;
		}
		
		CAN_ITConfig(CAN2, CAN_IT_FMP1, ENABLE);
		
		NVIC_InitStructure.NVIC_IRQChannel = CAN2_RX1_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3;//抢占优先级3
		NVIC_InitStructure.NVIC_IRQChannelSubPriority =4;		//子优先级3
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&NVIC_InitStructure);

		CanFilterClear(2);

		can2_inited = TRUE;
	}
	
	return 0;
}

