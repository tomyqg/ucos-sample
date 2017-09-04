/*

 * (1)文件创建；
 * (2)修改串口参数配置结构，增加二级缓冲机制；
 * (3)修改数据接收处理判断机制和数据中断接收处理；
 * (4)修改最大发送缓存长度为9000字节；
 * (5)更新缓冲器数据类型为易失类；
 *
 * >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
 * (6)增加串口收发条件编译机制；
 * (7)修改二级缓存计数器由u8变为u16类型;
 *
 *
 * 
 */








#include <stdarg.h>
#include "tiza_usart_port.h"
#include "defines.h"
#include "os_usart.h"
#include "gps.h"
#include "shell.h"



/* 中断类型：            奇偶错误中断   空闲总线中断   溢出-噪音-帧错误中断 */
#define USART_IRQ_TYPE	( USART_IT_PE | USART_IT_IDLE | USART_IT_ERR )  
#if USART1_DEBUG == 1 || USART2_DEBUG == 1 || USART3_DEBUG == 1 || USART4_DEBUG == 1 || USART5_DEBUG == 1
#define USART_ERR_DEBUG  1
#else
#define USART_ERR_DEBUG  0
#endif  /* USART_ERR_DEBUG */


#ifndef __USART_TX_RX_BUFF__
#define __USART_TX_RX_BUFF__

/* 发送超时时间 */
#define USART_TX_TIMEOUT_MAX      0x0FFF

/* 最大发送缓存长度 */
#ifndef MAX_TX_BUF_LENGTH
#define MAX_TX_BUF_LENGTH					9000  /* 单位：byte */
#endif	/* MAX_TX_BUF_LENGTH */

/* 最小接收缓存长度 */
#ifndef MIN_RX_BUF_LENGTH
#define MIN_RX_BUF_LENGTH					128		/* 单位：byte */
#endif	/* MIN_RX_BUF_LENGTH */

/* USART状态 */
enum USART_STATUS
{	
	USART_STATUS_BEGIN = 0,
  USART_STATUS_Unknow,
	USART_STATUS_Read_Data_Register_Not_Empty,
	USART_STATUS_Parity_Error,
	USART_STATUS_Transmission_Complete,
	USART_STATUS_IDLE_Line_Detected,
	USART_STATUS_CTS,
	USART_STATUS_LIN_Break_Detection,
	USART_STATUS_Overrun_Error,
	USART_STATUS_Noise_Error,
	USART_STATUS_Framing_Error,
	USART_STATUS_END
} ;






typedef struct 
{
	/* 是否有效？ */
	u8 DevStatus;
#define USART_INIT_PARAMS_BIT     0x01  /* 初始化参数 */
#define USART_START_OK_BIT        0x02  /* 启动成功 */
	
  /* 超时计数器 */
  vu32 UsartProcCount; 

  /* 超时重载计数器 */
  vu32 UsartRxTimeoutReload;

	/* 缓冲器类型 */
	vu8 bufferType;
#define USART_RX_DEFAULT_BUFFER		0x00  /* 一级缓存 */
#define USART_RX_FIFO_BUFFER			0x0F  /* 二级缓存 */

  /* USART接收缓冲 */
  vu16 RxCounter;
  vu8  *RxBuffer;

  /* 接收二级缓冲 */
  /* BUG-FIX #001 修改二级缓存计数器由u8变为u16类型 */
  vu16 SwCounter;
  vu8  *RxSWFIFO;

  /* USART数据处理句柄 */
  usart_procDataHdlr_f UsartDataHandler;

#if USART_ERR_DEBUG == 1
  /* 帧错误计数器 */
  vu32 FramErrCount;

  /* 噪音错误计数器 */
  vu32 NoiseErrCount;

  /* 溢出错误计数器 */
  vu32 OverErrCount;

  /* 缓存数据丢失错误计数器 */
  vu32 LostErrCount;
#endif  /* USART_ERR_DEBUG */

} USART_DevTypedef;


/* 是否二级缓冲 */
#define USART_IS_USING_SWFIFO(x)	( (x).bufferType == USART_RX_FIFO_BUFFER )

/* 使用后备二级缓冲 */
#define USART_SET_RX_SWFIFO(x)		( (x).bufferType = USART_RX_FIFO_BUFFER  )

/* 使用默认一级缓冲 */
#define USART_RESET_RX_BUFFER(x)	( (x).bufferType = USART_RX_DEFAULT_BUFFER )






#if __USE_USART__ == 1
#define UASRT_RX_EVENT_TRIGGER()   
#endif  /* __USE_USART__ */


/******************************************/
/*              内部变量[定义]            */
/******************************************/


#if __USE_USART_1__ == 1
#define USART1_DR_Base  			0x40013804

#if USART1_RxBufferSize < MIN_RX_BUF_LENGTH
#error ***ERROR MACRO USART1_RxBufferSize VALUE IS TO SMALL***
#else
#define USART1_RxHalfBufferSize   ( USART1_RxBufferSize >> 1 )
#endif	/* USART1_RxBufferSize */



/* USART1初始化参数 */
static USART_InitTypeDef usart1_init
= { USART1_BaudRate, 
    USART1_WordLength, 
    USART1_StopBits, 
    USART1_Parity, 
    ( USART_Mode_Rx | USART_Mode_Tx ), 
    USART1_HardwareFlowControl };

static u8 usart1_buffer[ USART1_RxBufferSize ];

/* 缓冲区1 */
static USART_DevTypedef	usart1 
#if USART_ERR_DEBUG == 1
= { 0, 0, USART1_RxTimeout, USART_RX_DEFAULT_BUFFER, 
    0, usart1_buffer, 
    0, ( u8* )&usart1_buffer[ USART1_RxHalfBufferSize ], 
    NULL, 
    0, 0, 0, 0  };
#else
= { 0, 0, USART1_RxTimeout, USART_RX_DEFAULT_BUFFER, 
    0, usart1_buffer, 
    0, ( u8* )&usart1_buffer[ USART1_RxHalfBufferSize ], 
    NULL };
#endif  /* USART_DATA_ERR_DEBUG */

#endif	/* __USE_USART_1__ */





#if __USE_USART_2__ == 1
#define USART2_DR_Base  			0x40004404

#if USART2_RxBufferSize < MIN_RX_BUF_LENGTH
#error ***ERROR MACRO USART2_RxBufferSize VALUE IS TO SMALL***
#else
#define USART2_RxHalfBufferSize   ( USART2_RxBufferSize >> 1 )
#endif	/* USART2_RxBufferSize */


/* USART2初始化参数 */
static USART_InitTypeDef usart2_init
= { USART2_BaudRate, 
    USART2_WordLength, 
    USART2_StopBits, 
    USART2_Parity, 
    ( USART_Mode_Rx | USART_Mode_Tx ), 
    USART2_HardwareFlowControl };


static u8 usart2_buffer[ USART2_RxBufferSize ];

/* 缓冲区2 */
static USART_DevTypedef	usart2 
#if USART_ERR_DEBUG == 1
= { 0, 0, USART2_RxTimeout, USART_RX_DEFAULT_BUFFER, 
    0, usart2_buffer, 0, 
    ( u8* )&usart2_buffer[ USART2_RxHalfBufferSize ], 
    NULL, 
    0, 0, 0, 0  };
#else
= { 0, 0, USART2_RxTimeout, USART_RX_DEFAULT_BUFFER, 
    0, usart2_buffer, 
    0, ( u8* )&usart2_buffer[ USART2_RxHalfBufferSize ], 
    NULL };
#endif  /* USART_DATA_ERR_DEBUG */

#endif	/* __USE_USART_2__ */



#if __USE_USART_3__ == 1
#define USART3_DR_Base  			0x40004804

#if USART3_RxBufferSize < MIN_RX_BUF_LENGTH
#error ***ERROR MACRO USART3_RxBufferSize VALUE IS TO SMALL***
#else
#define USART3_RxHalfBufferSize   ( USART3_RxBufferSize >> 1 )
#endif	/* USART3_RxBufferSize */


/* USART3初始化参数 */
static USART_InitTypeDef usart3_init
= { USART3_BaudRate, 
    USART3_WordLength, 
    USART3_StopBits, 
    USART3_Parity, 
    ( USART_Mode_Rx | USART_Mode_Tx ), 
    USART3_HardwareFlowControl };

static u8 usart3_buffer[ USART3_RxBufferSize ];

/* 缓冲区3 */
static USART_DevTypedef	usart3 
#if USART_ERR_DEBUG == 1
= { 0, 0, USART3_RxTimeout, USART_RX_DEFAULT_BUFFER, 
    0, usart3_buffer, 
    0, ( u8* )&usart3_buffer[ USART3_RxHalfBufferSize ], 
    NULL, 
    0, 0, 0, 0  };
#else
= { 0, 0, USART3_RxTimeout, USART_RX_DEFAULT_BUFFER, 
    0, usart3_buffer, 
    0, ( u8* )&usart3_buffer[ USART3_RxHalfBufferSize ], 
    NULL };
#endif  /* USART_DATA_ERR_DEBUG */

#endif	/* __USE_USART_3__ */



#if __USE_USART_4__ == 1

#if USART4_RxBufferSize < MIN_RX_BUF_LENGTH
#error ***ERROR MACRO USART4_RxBufferSize VALUE IS TO SMALL***
#else
#define USART4_RxHalfBufferSize   ( USART4_RxBufferSize >> 1 )
#endif	/* USART4_RxBufferSize */

/* USART4初始化参数 */
static USART_InitTypeDef usart4_init
= { USART4_BaudRate, 
    USART4_WordLength, 
    USART4_StopBits, 
    USART4_Parity, 
    ( USART_Mode_Rx | USART_Mode_Tx ), 
    USART4_HardwareFlowControl };

static u8 usart4_buffer[ USART4_RxBufferSize ];

/* 缓冲区4 */
static USART_DevTypedef	usart4 
#if USART_ERR_DEBUG == 1
= { 0, 0, USART4_RxTimeout, USART_RX_DEFAULT_BUFFER, 
    0, usart4_buffer, 0, 
    ( u8* )&usart4_buffer[ USART4_RxHalfBufferSize ], 
    NULL, 
    0, 0, 0, 0  };
#else
= { 0, 0, USART4_RxTimeout, USART_RX_DEFAULT_BUFFER, 
    0, usart4_buffer, 
    0, ( u8* )&usart4_buffer[ USART4_RxHalfBufferSize ], 
    NULL };
#endif  /* USART_DATA_ERR_DEBUG */

#endif	/* __USE_USART_4__ */


#if __USE_USART_5__ == 1

#if USART5_RxBufferSize < MIN_RX_BUF_LENGTH
#error ***ERROR MACRO USART5_RxBufferSize VALUE IS TO SMALL***
#else
#define USART5_RxHalfBufferSize   ( USART5_RxBufferSize >> 1 )
#endif	/* USART5_RxBufferSize */


/* USART5初始化参数 */
static USART_InitTypeDef usart5_init
= { USART5_BaudRate, 
    USART5_WordLength, 
    USART5_StopBits, 
    USART5_Parity, 
    ( USART_Mode_Rx | USART_Mode_Tx ), 
    USART5_HardwareFlowControl };

static u8 usart5_buffer[ USART5_RxBufferSize ];

/* 缓冲区5 */
static USART_DevTypedef	usart5 
#if USART_ERR_DEBUG == 1
= { 0, 0, USART5_RxTimeout, USART_RX_DEFAULT_BUFFER, 
    0, usart5_buffer, 
    0, ( u8* )&usart5_buffer[ USART5_RxHalfBufferSize ], 
    NULL, 
    0, 0, 0, 0  };
#else
= { 0, 0, USART5_RxTimeout, USART_RX_DEFAULT_BUFFER, 
    0, usart5_buffer, 
    0, ( u8* )&usart5_buffer[ USART5_RxHalfBufferSize ], 
    NULL };
#endif  /* USART_DATA_ERR_DEBUG */

#endif	/* __USE_USART_5__ */

#endif	/* __USART_TX_RX_BUFF__ */








/******************************************/
/*              内部函数[声明]            */
/******************************************/

void SHELL_ReceiveHandler	(uint8_t data)
{
	
	  usart1.UsartProcCount = 0;
		if ( USART_IS_USING_SWFIFO ( usart1 ) == FALSE )
		{
			if ( usart1.RxCounter < USART1_RxHalfBufferSize )
			{
				usart1.RxBuffer[ usart1.RxCounter ] = data;
				usart1.RxCounter++;
			}
			else
			{
				USART_SET_RX_SWFIFO ( usart1 );
				usart1.SwCounter = 0;
				usart1.RxSWFIFO[ usart1.SwCounter ] = data;
				usart1.SwCounter++;
        UASRT_RX_EVENT_TRIGGER();
			}
		}
		else
		{
			if ( usart1.SwCounter < USART1_RxHalfBufferSize )
			{
				usart1.RxSWFIFO[ usart1.SwCounter ] = data;
				usart1.SwCounter++;
			}
			else
			{
				USART_RESET_RX_BUFFER ( usart1 );
				usart1.RxCounter = 0;
				usart1.RxBuffer[ usart1.RxCounter ] = data;
				usart1.RxCounter++;
        UASRT_RX_EVENT_TRIGGER();
			}
		}	
}			
		

		
void GSM_ReceiveHandler	(uint8_t data)
{
	
	  usart3.UsartProcCount = 0;
		if ( USART_IS_USING_SWFIFO ( usart3 ) == FALSE )
		{
			if ( usart3.RxCounter < USART3_RxHalfBufferSize )
			{
				usart3.RxBuffer[ usart3.RxCounter ] = data;
				usart3.RxCounter++;
			}
			else
			{
				USART_SET_RX_SWFIFO ( usart3 );
				usart3.SwCounter = 0;
				usart3.RxSWFIFO[ usart3.SwCounter ] = data;
				usart3.SwCounter++;
        UASRT_RX_EVENT_TRIGGER();
			}
		}
		else
		{
			if ( usart3.SwCounter < USART3_RxHalfBufferSize )
			{
				usart3.RxSWFIFO[ usart3.SwCounter ] = data;
				usart3.SwCounter++;
			}
			else
			{
				USART_RESET_RX_BUFFER ( usart3 );
				usart3.RxCounter = 0;
				usart3.RxBuffer[ usart3.RxCounter ] = data;
				usart3.RxCounter++;
        UASRT_RX_EVENT_TRIGGER();
			}
		}	
}			

void GPS_ReceiveHandler(uint8_t data)
{	
		usart4.UsartProcCount = 0;		
		if ( USART_IS_USING_SWFIFO ( usart4 ) == FALSE )
		{
			if ( usart4.RxCounter < USART4_RxHalfBufferSize )
			{
				usart4.RxBuffer[ usart4.RxCounter ] = data;
				usart4.RxCounter++;
			}
			else
			{
				USART_SET_RX_SWFIFO ( usart4 );
				usart4.SwCounter = 0;
				usart4.RxSWFIFO[ usart4.SwCounter ] = data;
				usart4.SwCounter++;
        UASRT_RX_EVENT_TRIGGER();
			}
		}
		else
		{
			if ( usart4.SwCounter < USART4_RxHalfBufferSize )
			{
				usart4.RxSWFIFO[ usart4.SwCounter ] = data;
				usart4.SwCounter++;
				
			}
			else
			{
				USART_RESET_RX_BUFFER ( usart4 );
				usart4.RxCounter = 0;
				usart4.RxBuffer[ usart4.RxCounter ] = data;
				usart4.RxCounter++;
        UASRT_RX_EVENT_TRIGGER();
				
			}
		}
}



/* 
 * 功能描述: USART数据消化
 * 引用参数: 用户数据
 *
 * 返回值  : 无
 * 
 */
static void USART_DataDigestHandler ( void* userData )
{
  USART_DevTypedef *usartx = ( USART_DevTypedef * )userData;

  if ( usartx )
  {
    bool timeout = FALSE;

#if OS_CRITICAL_METHOD == 3                           /* Allocate storage for CPU status register      */
    OS_CPU_SR  cpu_sr = 0;
#endif

    ++usartx->UsartProcCount;
    if ( usartx->UsartProcCount >= usartx->UsartRxTimeoutReload )
    {
      timeout = TRUE;
      usartx->UsartProcCount = 0; 
    }
    switch ( usartx->bufferType )
    {
      case USART_RX_DEFAULT_BUFFER: /* 一级缓冲 */
        {
          if ( usartx->SwCounter > 0 )
          {       
            /* 二级缓冲若存在数据则处理 */
            if ( usartx->UsartDataHandler )
            {
              usartx->UsartDataHandler ( usartx->SwCounter, ( u8* )usartx->RxSWFIFO );
            }
            OS_ENTER_CRITICAL ();
               usartx->SwCounter = 0;
            OS_EXIT_CRITICAL (); 
          }
          
          /* 一级缓冲超时 */
          if ( timeout == TRUE )
          {
            if ( usartx->RxCounter > 0 )
            {
              /* 数据处理时，切换使用另外的缓冲 */
              OS_ENTER_CRITICAL ();
              USART_SET_RX_SWFIFO ( (*usartx) );
              OS_EXIT_CRITICAL ();
            
              if ( usartx->UsartDataHandler )
              {
                usartx->UsartDataHandler ( usartx->RxCounter, ( u8* )usartx->RxBuffer );
              }
              OS_ENTER_CRITICAL ();
                  usartx->RxCounter = 0;
              OS_EXIT_CRITICAL (); 
            }
          }
        }
        break;

      case USART_RX_FIFO_BUFFER:   /* 二级缓冲 */
        {
          if ( usartx->RxCounter > 0 )
          {      
            /* 一级缓冲若存在数据则处理 */
            if ( usartx->UsartDataHandler )
            {
              usartx->UsartDataHandler ( usartx->RxCounter, ( u8* )usartx->RxBuffer );
            }
            OS_ENTER_CRITICAL ();
                usartx->RxCounter = 0;
            OS_EXIT_CRITICAL (); 
          } 
    
          /* 二级缓冲超时 */
          if ( timeout == TRUE )
          {
            if ( usartx->SwCounter > 0 )
            {
              /* 数据处理时，切换使用另外的缓冲 */
              OS_ENTER_CRITICAL ();
              USART_RESET_RX_BUFFER ( (*usartx) );
              OS_EXIT_CRITICAL ();
           
              if ( usartx->UsartDataHandler )
              {
                usartx->UsartDataHandler ( usartx->SwCounter, ( u8* )usartx->RxSWFIFO );
              }
              OS_ENTER_CRITICAL ();
                  usartx->SwCounter = 0;
              OS_EXIT_CRITICAL (); 
            }
          }      
        }
        break;
    }  
  }
}







/* 
 * 功能描述: USART数据接收处理
 * 引用参数: 无
 *          
 * 返回值  : 无
 * 
 */
extern void USART_LowLevelDataProcess ( void )
{

#if __USE_USART_1__ == 1 && USART1_Mode_Rx > 0
{
  if ( usart1.RxCounter || usart1.SwCounter )
  {
    USART_DataDigestHandler ( ( USART_DevTypedef * )&usart1 );  
  }
}
#endif  /* __USE_USART_1__ */	
	
	
	
	
	
#if __USE_USART_3__ == 1 && USART3_Mode_Rx > 0
{
  if ( usart3.RxCounter || usart3.SwCounter )
  {

		static worker_t usartWorker;
		usartWorker.func = USART_DataDigestHandler;
		usartWorker.data = ( void * )&usart3;
		WORKER_StartupWork ( ( worker_t * )&usartWorker );

  }
}
#endif  /* __USE_USART_3__ */
	


#if __USE_USART_4__ == 1 && USART4_Mode_Rx > 0

  if ( usart4.RxCounter || usart4.SwCounter )
  {
    USART_DataDigestHandler ( ( USART_DevTypedef * )&usart4 );
  }

#endif  /* __USE_USART_4__ */

}







/* 
 * 功能描述：USART接口初始化
 * 引用参数：无
 *          
 * 返回值  ：状态码
 * 
 */
extern bool USARTx_Start ( usart_Flow_e Flow, usart_procDataHdlr_f DataHandler )
{
  bool result = FALSE;
  	
  switch ( Flow )
	{
#if __USE_USART_1__ == 1
		case DEV_USART1:
      if ( ( usart1.DevStatus & USART_INIT_PARAMS_BIT ) == 0 )
      {

        usart1.DevStatus |= USART_INIT_PARAMS_BIT;
			}

			if ( DataHandler && ( usart1.DevStatus & USART_INIT_PARAMS_BIT ) )
			{
				usart1.UsartDataHandler = DataHandler;
        usart1.UsartProcCount = 0;
        usart1.RxCounter = 0;
        usart1.SwCounter = 0;
        USART_RESET_RX_BUFFER ( usart1 );
        usart1.DevStatus |= USART_START_OK_BIT;
        result = TRUE;
      }
			break;
#endif	/* __USE_USART_1__ */

#if __USE_USART_2__ == 1
		case DEV_USART2:
      if ( ( usart2.DevStatus & USART_INIT_PARAMS_BIT ) == 0 )
      {        
        usart2.DevStatus |= USART_INIT_PARAMS_BIT;   
      }

			if ( DataHandler && ( usart2.DevStatus & USART_INIT_PARAMS_BIT ) )
			{
			  usart2.UsartDataHandler = DataHandler;
        usart2.UsartProcCount = 0;
        usart2.RxCounter = 0;
        usart2.SwCounter = 0;
        USART_RESET_RX_BUFFER ( usart2 );
        usart2.DevStatus |= USART_START_OK_BIT;

        result = TRUE;
			}
			break;
#endif	/* __USE_USART_2__ */

#if __USE_USART_3__ == 1
		case DEV_USART3:
      if ( ( usart3.DevStatus & USART_INIT_PARAMS_BIT ) == 0 )
      {
        usart3.DevStatus |= USART_INIT_PARAMS_BIT; 
       
      }

			if ( DataHandler && ( usart3.DevStatus & USART_INIT_PARAMS_BIT ) )
			{
			  usart3.UsartDataHandler = DataHandler;
        usart3.UsartProcCount = 0;
        usart3.RxCounter = 0;
        usart3.SwCounter = 0;
        USART_RESET_RX_BUFFER ( usart3 );
        usart3.DevStatus |= USART_START_OK_BIT;

        result = TRUE;
			}
			break;
#endif	/* __USE_USART_3__ */


#if __USE_USART_4__ == 1
		case DEV_USART4:
      if ( ( usart4.DevStatus & USART_INIT_PARAMS_BIT ) == 0 )
      {
        usart4.DevStatus |= USART_INIT_PARAMS_BIT; 
       
      }

			if ( DataHandler && ( usart4.DevStatus & USART_INIT_PARAMS_BIT ) )
			{
			  usart4.UsartDataHandler = DataHandler;
        usart4.UsartProcCount = 0;
        usart4.RxCounter = 0;
        usart4.SwCounter = 0;
        USART_RESET_RX_BUFFER ( usart4 );
        usart4.DevStatus |= USART_START_OK_BIT;
 
        result = TRUE;

			}
			break;
#endif	/* __USE_USART_4__ */


#if __USE_USART_5__ == 1
		case DEV_USART5:
      if ( ( usart5.DevStatus & USART_INIT_PARAMS_BIT ) == 0 )
      {
        usart5.DevStatus |= USART_INIT_PARAMS_BIT;
      }

			if ( DataHandler && ( usart5.DevStatus & USART_INIT_PARAMS_BIT ) )
			{
			  usart5.UsartDataHandler = DataHandler;
        usart5.UsartProcCount = 0;
        usart5.RxCounter = 0;
        usart5.SwCounter = 0;
        USART_RESET_RX_BUFFER ( usart5 );
        usart5.DevStatus |= USART_START_OK_BIT;

        result = TRUE;

			}
			break;
#endif	/* __USE_USART_5__ */


		default:	break;
	}

  return result;
}









/* 
 * 功能描述：USART接口初始化
 * 引用参数：无
 *          
 * 返回值  ：状态码
 * 
 */
extern void USART_Stop ( usart_Flow_e Flow )
{
	switch ( Flow )
	{
#if __USE_USART_1__ == 1
		case DEV_USART1:
#if USART1_DEBUG == 1
      printf ( "\r\n[USART1] stop\r\n" );
#endif  /* USART1_DEBUG */ 

      USART_ITConfig ( USART1, USART_IT_RXNE | USART_IT_PE | USART_IT_ERR, DISABLE );	    
	    USART_Cmd ( USART1, DISABLE );
      USART_DeInit ( USART1 );
		  usart1.UsartDataHandler = NULL;
      usart1.UsartProcCount = 0;
      usart1.RxCounter = 0;
      usart1.SwCounter = 0;
      usart1.DevStatus = 0;

#if USART_ERR_DEBUG == 1
#if USART1_DEBUG == 1
      usart1.FramErrCount = 0;
      usart1.OverErrCount = 0;
      usart1.NoiseErrCount = 0;
      usart1.LostErrCount = 0;
#endif  /* USART1_DEBUG */
#endif  /* USART_ERR_DEBUG */
			break;
#endif	/* __USE_USART_1__ */

#if __USE_USART_2__ == 1
		case DEV_USART2:
#if USART2_DEBUG == 1
      printf ( "\r\n[USART2] stop\r\n" );
#endif  /* USART2_DEBUG */

	    USART_ITConfig ( USART2, USART_IT_RXNE | USART_IT_PE | USART_IT_ERR, DISABLE );
	    USART_Cmd ( USART2, DISABLE );
		  usart2.UsartDataHandler = NULL;
      usart2.UsartProcCount = 0;
      usart2.RxCounter = 0;
      usart2.SwCounter = 0;
      usart2.DevStatus = 0;

#if USART_ERR_DEBUG == 1
#if usart2_DEBUG == 1
      usart2.FramErrCount = 0;
      usart2.OverErrCount = 0;
      usart2.NoiseErrCount = 0;
      usart2.LostErrCount = 0;
#endif  /* usart2_DEBUG */
#endif  /* USART_ERR_DEBUG */
			break;
#endif	/* __USE_USART_2__ */

#if __USE_USART_3__ == 1
		case DEV_USART3:
#if USART3_DEBUG == 1
      printf ( "\r\n[USART3] stop\r\n" );
#endif  /* USART3_DEBUG */

	    USART_ITConfig ( USART3, USART_IT_RXNE | USART_IT_PE | USART_IT_ERR, DISABLE );
	    USART_Cmd ( USART3, DISABLE );
		  usart3.UsartDataHandler = NULL;
      usart3.UsartProcCount = 0;
      usart3.RxCounter = 0;
      usart3.SwCounter = 0;
      usart3.DevStatus = 0;

#if USART_ERR_DEBUG == 1
#if usart3_DEBUG == 1
      usart3.FramErrCount = 0;
      usart3.OverErrCount = 0;
      usart3.NoiseErrCount = 0;
      usart3.LostErrCount = 0;
#endif  /* usart3_DEBUG */
#endif  /* USART_ERR_DEBUG */
			break;
#endif	/* __USE_USART_3__ */

#if __USE_USART_4__ == 1
		case DEV_USART4:
#if USART4_DEBUG == 1
      printf ( "\r\n[USART4] stop\r\n" );
#endif  /* USART4_DEBUG */

	    USART_ITConfig ( UART4, USART_IT_RXNE | USART_IT_PE | USART_IT_ERR, DISABLE );
	    USART_Cmd ( UART4, DISABLE );
		  usart4.UsartDataHandler = NULL;
      usart4.UsartProcCount = 0;
      usart4.RxCounter = 0;
      usart4.SwCounter = 0;
      usart4.DevStatus = 0;

#if USART_ERR_DEBUG == 1
#if usart4_DEBUG == 1
      usart4.FramErrCount = 0;
      usart4.OverErrCount = 0;
      usart4.NoiseErrCount = 0;
      usart4.LostErrCount = 0;
#endif  /* usart4_DEBUG */
#endif  /* USART_ERR_DEBUG */
			break;
#endif	/* __USE_USART_4__ */

#if __USE_USART_5__ == 1
		case DEV_USART5:
#if USART5_DEBUG == 1
      printf ( "\r\n[USART5] stop\r\n" );
#endif  /* USART5_DEBUG */

	    USART_ITConfig ( UART5, USART_IT_RXNE | USART_IT_PE | USART_IT_ERR, DISABLE );
	    USART_Cmd ( UART5, DISABLE );
		  usart5.UsartDataHandler = NULL;
      usart5.UsartProcCount = 0;
      usart5.RxCounter = 0;
      usart5.SwCounter = 0;
      usart5.DevStatus = 0;

#if USART_ERR_DEBUG == 1
#if usart5_DEBUG == 1
      usart5.FramErrCount = 0;
      usart5.OverErrCount = 0;
      usart5.NoiseErrCount = 0;
      usart5.LostErrCount = 0;
#endif  /* usart5_DEBUG */
#endif  /* USART_ERR_DEBUG */
			break;
#endif	/* __USE_USART_5__ */
			
		default:	break;
	}
}




     


//#define __USE_USART1_DMA__
//#define __USE_USART2_DMA__
//#define __USE_USART3_DMA__



/* 
 * 功能描述：USART数据传输
 * 引用参数：无
 *          
 * 返回值  ：状态码
 * 
 */
extern bool USART_WriteData ( usart_Flow_e Flow, u8 Data[], u16 DataLen )
{
	bool result = false;
	
	
	
  if ( ( DataLen > 0 )&&( DataLen < MAX_TX_BUF_LENGTH )&&( Data != NULL ) ) 
  {
		u16 timeout = 0;
		switch ( Flow )
		{	
#if __USE_USART_1__ == 1 && USART1_Mode_Tx > 0
			case DEV_USART1:
        if ( ( usart1.DevStatus & USART_START_OK_BIT ) == 0 ) return FALSE;

        USART_SemPend ( DEV_USART1 );
        
        while ( DataLen > 0 )
        {
          USART_SendData ( USART1, ( u8 )*Data );
  
        	/* Loop until the end of transmission */
          timeout = 0;
        	while ( USART_GetFlagStatus ( USART1, USART_FLAG_TXE ) == RESET && timeout < USART_TX_TIMEOUT_MAX )
          {
            timeout++;  
          }

          if ( timeout >= USART_TX_TIMEOUT_MAX )
          {
            result = FALSE;
            USART_SemPost ( DEV_USART1 );
            return result; 
          }
          else
          {
            Data++;
            DataLen--;          
          }
        }
	      result = TRUE;
        USART_SemPost ( DEV_USART1 );    
				break;
#endif	/* __USE_USART_1__ */
			
			
			
#if __USE_USART_3__ == 1 && USART3_Mode_Tx > 0
			case DEV_USART3:
        if ( ( usart3.DevStatus & USART_START_OK_BIT ) == 0 )
        {  
          return FALSE;
        }

        USART_SemPend ( DEV_USART3 );

#if USE_GSM_UART_DMA == 1				
				if(TM_USART_DMA_Sending(USART3)==0)
				{
						TM_USART_DMA_Send(USART3,(uint8_t*)Data,DataLen);
				}
#else
        while ( DataLen > 0 )
        {
          USART_SendData ( USART3, ( u8 )*Data );
        	/* Loop until the end of transmission */
          timeout = 0;
					//printf("%c",Data[timeout]);
        	while ( USART_GetFlagStatus ( USART3, USART_FLAG_TXE ) == RESET && timeout < USART_TX_TIMEOUT_MAX )
          {
            timeout++;  
          }
          if ( timeout >= USART_TX_TIMEOUT_MAX )
          {
            result = FALSE;
            USART_SemPost ( DEV_USART3 );
            return result; 
          }
          else
          {
            Data++;
            DataLen--;          
          }
        }
#endif
	      result = TRUE; 
        USART_SemPost ( DEV_USART3 );  
				break;
#endif	/* __USE_USART_3__ */
			
			default:  break;
		}	
  } 
  	
	return result;
}









/* 
 * 功能描述：USART数据传输
 * 引用参数：无
 *          
 * 返回值  ：状态码
 * 
 */
extern bool USART_SetRxTimeoutByNms ( usart_Flow_e Flow, u32 RxTimeoutValue )
{
	switch ( Flow )
	{
#if ( __USE_USART_1__ == 1 )
		case DEV_USART1:   
      if ( RxTimeoutValue > 0 )
      {
        usart1.UsartRxTimeoutReload = RxTimeoutValue;
        return TRUE;
      }
			break;
#endif	/* __USE_USART_1__ */

#if ( __USE_USART_2__ == 1 )
		case DEV_USART2: 
      if ( RxTimeoutValue > 0 )
      {
        usart2.UsartRxTimeoutReload = RxTimeoutValue;
        return TRUE;
      }
		  break;
#endif	/* __USE_USART_2__ */

#if ( __USE_USART_3__ == 1 )
		case DEV_USART3:  
      if ( RxTimeoutValue > 0 )
      {
        usart3.UsartRxTimeoutReload = RxTimeoutValue;
        return TRUE;
      }
			break;
#endif	/* __USE_USART_3__ */


#if ( __USE_USART_4__ == 1 )
		case DEV_USART4:  
      if ( RxTimeoutValue > 0 )
      {
        usart4.UsartRxTimeoutReload = RxTimeoutValue;
        return TRUE;
      }
			break;
#endif	/* __USE_USART_4__ */


#if ( __USE_USART_5__ == 1 )
		case DEV_USART5:  
      if ( RxTimeoutValue > 0 )
      {
        usart5.UsartRxTimeoutReload = RxTimeoutValue;
        return TRUE;
      }
			break;
#endif	/* __USE_USART_5__ */
			
		default:  break;
	}
  
  return FALSE;	
}











/* 
 * 功能描述: USART数据接收处理
 * 引用参数:
 *          
 * 返回值  :
 * 
 */
extern bool USART_IOCtl ( usart_Flow_e Flow, ... )
{
  #define USART_INIT_MAX_ARGS_NUMBER   11  
  
  USART_InitTypeDef *usartx = NULL;
  va_list va_args;
  s32 initType = 0, initCount = USART_INIT_MAX_ARGS_NUMBER;

	switch ( Flow )
	{
#if ( __USE_USART_1__ == 1 )
		case DEV_USART1:
      usartx = ( USART_InitTypeDef * )&usart1_init;   
			break;
#endif	/* __USE_USART_1__ */

#if ( __USE_USART_2__ == 1 )
		case DEV_USART2:
      usartx = ( USART_InitTypeDef * )&usart2_init;   
		  break;
#endif	/* __USE_USART_2__ */

#if ( __USE_USART_3__ == 1 )
		case DEV_USART3:
      usartx = ( USART_InitTypeDef * )&usart3_init;    
			break;
#endif	/* __USE_USART_3__ */

#if ( __USE_USART_4__ == 1 )
		case DEV_USART4:
      usartx = ( USART_InitTypeDef * )&usart4_init;    
			break;
#endif	/* __USE_USART_4__ */

#if ( __USE_USART_5__ == 1 )
		case DEV_USART5:
      usartx = ( USART_InitTypeDef * )&usart5_init;    
			break;
#endif	/* __USE_USART_5__ */
			
		default:  
      return FALSE;
	}

  va_start ( va_args, Flow );
  while ( initCount > 0 && usartx )
  {
    initType = va_arg ( va_args, u32 );
    initCount--;
    switch ( initType )
    {
      case USART_InitType_BaudRate:
        usartx->USART_BaudRate = va_arg ( va_args, u32 );
        initCount--;

#if USART_ERR_DEBUG == 1
        printf ( "\r\n[USART] BaudRate %u\r\n", usartx->USART_BaudRate );
#endif  /* USART_ERR_DEBUG */
        break;

      case USART_InitType_WordLength:
        usartx->USART_WordLength = va_arg ( va_args, u32 );
        initCount--;

#if USART_ERR_DEBUG == 1
        printf ( "\r\n[USART] WordLength %u\r\n", usartx->USART_WordLength );
#endif  /* USART_ERR_DEBUG */ 
        break;

      case USART_InitType_StopBits:
        usartx->USART_StopBits = va_arg ( va_args, u32 );
        initCount--;

#if USART_ERR_DEBUG == 1
        printf ( "\r\n[USART] StopBits %u\r\n", usartx->USART_StopBits );
#endif  /* USART_ERR_DEBUG */ 
        break;

      case USART_InitType_Parity:
        usartx->USART_Parity = va_arg ( va_args, u32 );
        initCount--;

#if USART_ERR_DEBUG == 1
        printf ( "\r\n[USART] Parity %u\r\n", usartx->USART_Parity );
#endif  /* USART_ERR_DEBUG */ 
        break;

      case USART_InitType_HardwareFlowControl:
        usartx->USART_HardwareFlowControl = va_arg ( va_args, u32 );
        initCount--;

#if USART_ERR_DEBUG == 1
        printf ( "\r\n[USART] HardwareFlowControl %u\r\n", usartx->USART_HardwareFlowControl );
#endif  /* USART_ERR_DEBUG */ 
        break;

      case USART_InitType_None:
        initCount--;
        usartx = NULL;

#if USART_ERR_DEBUG == 1
        printf ( "\r\n[USART] end\r\n" );
#endif  /* USART_ERR_DEBUG */ 
        break;
    } 
  }

  va_end ( va_args );   
  return TRUE;
}




extern bool GPS_DataHandler ( u16 DataSize, u8 * Data )
{
  
#if OS_CRITICAL_METHOD == 3                           /* Allocate storage for CPU status register      */
    OS_CPU_SR  cpu_sr = 0;
#endif
		OS_ENTER_CRITICAL ();
	
		GPS_Polling ( ( ascii * )Data, ( s32 )DataSize );
		
	
#if GPS_DEBUG ==  1
	u32 i;
	printf("\r\n");
	for(i=0;i<DataSize;i++)
	{
		printf("%c",Data[i]);
	}
	printf("\r\n");
#endif 
	
	OS_EXIT_CRITICAL ();
	return true;
}


extern bool Shell_DataHandler( u16 DataSize, u8 * Data )
{

#if OS_CRITICAL_METHOD == 3                           /* Allocate storage for CPU status register      */
    OS_CPU_SR  cpu_sr = 0;
#endif
	OS_ENTER_CRITICAL ();
		/* 用户命令解释器 */
		if ( FALSE == SHELL_CmdParser ( DataSize, Data ) )
		{	  
	//    if ( vehicleParams.DebugCtroller.debugIsValid == TRUE && ( vehicleParams.DebugCtroller.debugType & Debug_Type_MDM ) )
	//    {
	//      USART_WriteData ( ( usart_Flow_e )DEV_USART3, Data, DataSize ); 
	//    }	
		}
	OS_EXIT_CRITICAL ();
  return TRUE;

}

