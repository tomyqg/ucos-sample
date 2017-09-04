#include "includes.h"
#include "os_can.h"
#include "tiza_can_port.h"
#include "hardware_select_config.h"



OS_STK App_TaskCanRecStk[APP_TASK_CANREC_STK_SIZE];
OS_EVENT * OS_EventCanRec  = NULL;

CanRxMsg InBoundBuffer[IN_BUFFER_SIZE];	   //接收队列

 RING_T   RingInBuffer  = { &InBoundBuffer[0], 0, 0, IN_BUFFER_SIZE - 1 };  //接收队列
 
 
 /******************************************/
/*              内部变量[定义]            */
/******************************************/
/* CAN 模块默认配置 */
CAN_DevTypedef can = 
{ FALSE, CAN_STATUS_BEGIN, NULL, CAN_MODE_NORMAL, 0, 0, 0, 0, CAN_FRAME_STD, 0,{0}, 0 };


void CAN_RECEIVE_DATA_Handle(void);




/* CAN 接收任务 激活标志 */
#if ( Sofe_Cache == 0 )
static bool m_bCanDealActive = false; 
extern bool OSCAN_GetTaskActiveFalg( void )
{
    return m_bCanDealActive; 
} 
#endif

/* 
 * 功能描述: 
 * 引用参数:
 *          
 * 返回值  :
 * 
 */
extern void  OSCAN_InitOS (void)
{
  INT8U  os_err = OS_ERR_NONE;
  
  os_err = OSTaskCreateExt((void (*)(void *)) App_TaskCanRec,
                        (void          * ) 0,
                        (OS_STK        * )&App_TaskCanRecStk[APP_TASK_CANREC_STK_SIZE - 1],
                        (INT8U           ) APP_TASK_CANREC_PRIO,
                        (INT16U          ) APP_TASK_CANREC_PRIO,
                        (OS_STK        * )&App_TaskCanRecStk,
                        (INT32U          ) APP_TASK_CANREC_STK_SIZE,
                        (void          * ) 0,
                        (INT16U          )TZ_OS_TASK_OPT_CHECK);
												
#if OS_TASK_NAME_EN > 0u
  OSTaskNameSet(APP_TASK_CANREC_PRIO, (INT8U *)(void *)"Task_CanRec", &os_err);
#endif
   
  os_err=os_err;
}






/* 
 * 功能描述: 
 * 引用参数:
 *          
 * 返回值  :
 * 
 */
static void  OSCAN_SemCreate (INT16U cnt)
{
   OS_EventCanRec = OSSemCreate(cnt);

   if( OS_EventCanRec  == NULL)
   {
#if __USE_DEBUG_EN__  > 0
      printf("\r\n[CAN MODULE] ---> [CAN OS]   OSCAN_SemCreate Failed! \r\n"); 
#endif
   }
}






/* 
 * 功能描述: 
 * 引用参数:
 *          
 * 返回值  :
 * 
 */
static INT8U  OSCAN_SemPend (INT16U timeout)
{
  INT8U err = OS_ERR_NONE;

 	OSSemPend(OS_EventCanRec, timeout, &err);
	return err;
}




#if 0
static void  OSCAN_SemAccept (void)
{
 	OSSemAccept(OS_EventCanRec);
 
}
#endif





/* 
 * 功能描述: 
 * 引用参数:
 *          
 * 返回值  :
 * 
 */
extern INT8U  OSCAN_SemDel (INT8U opt)/*Tyope: OS_DEL_ALWAYS; OS_DEL_NO_PEND*/
{
  INT8U err = OS_ERR_NONE;
  
  OSSemDel(OS_EventCanRec, opt, &err);
  return err; 
}








/* 
 * 功能描述: 
 * 引用参数:
 *          
 * 返回值  :
 * 
 */
extern void  OSCAN_SemPost (void)
{
   OSSemPost(OS_EventCanRec);
 
}





/* 
 * 功能描述: 
 * 引用参数:
 *          
 * 返回值  :
 * 
 */
extern void  OSCAN_SemQuery (void)
{	
  OS_SEM_DATA* pData =  NULL;

 	OSSemQuery(OS_EventCanRec, pData);
}






/* 
 * 功能描述: 
 * 引用参数:
 *          
 * 返回值  :
 * 
 */
//extern void  CAN_RECEIVE_DATA_Handle(void);

void App_TaskCanRec(void *parg)
{

	OSCAN_SemCreate	(0);
//	#if  OSCAN_DEBUG == 1
//	printf("\r\n[CAN MODULE] ---> [CAN OS]  is   run ........\r\n"); 
//	#endif

	while(1)
	{ 
	   OSCAN_SemPend(0);
	   CAN_RECEIVE_DATA_Handle();
	}


}




/****************************************************************************************
@函数名称           : void rng_enqueue(CAN_PACKET_T msg, RING_T *ring)           
@参数               : CAN_PACKET_T msg
                      RING_T *ring
@返回值             : 
@描述               : 函数把CAN_PACKET_T msg加到RING_T *ring指向的结构的尾部
@最后编辑时间       : 2011-11-4
@版本               : V1.0.0
*****************************************************************************************/
extern void rng_enqueue(CanRxMsg msg, RING_T *ring)
{
    if(((ring->tail+1)==ring->head)||((ring->tail==ring->buffer_size)&&(ring->head==0)))//环形队列满
    {
        return;
    }
    else
    {
        ring->buffer[ring->tail] = msg;                         //enqueue CAN message
        ring->tail++;
        if(ring->tail > ring->buffer_size)                      //wrap,CAN_BUF_MAX ->0
            ring->tail = 0;                             
        return;
    }
}


/****************************************************************************************
@函数名称           : CAN_PACKET_T *rng_dequeue(RING_T *ring)            
@参数               : RING_T *ring
@返回值             : 
@描述               : 从ring中的头部取出一个CAN_PACKET_T，如果是空就返回NULL
@最后编辑时间       : 2011-11-4
@版本               : V1.0.0
*****************************************************************************************/
extern CanRxMsg *rng_dequeue(RING_T *ring)
{
    CanRxMsg *temp;

    if(ring->head == ring->tail)
    {
        return NULL;                                        //ring buffer is empty
    }
    else
    {
        temp = &ring->buffer[ring->head];
        ring->head++;
        if(ring->head > ring->buffer_size)
            ring->head = 0;                                 //wrap,CAN_BUF_MAX ->0
        return temp;
    }   
} 



/* 
 * 功能描述：CAN FIFO0接收数据处理
 * 引用参数：无
 *          
 * 返回值  ：无
 * 
 */

#if 1
void  CAN_RECEIVE_DATA_Handle(void)
{
		 CanRxMsg* CanMessageRX = NULL;
		 if ( can.DataHandler )
		 {
				 /* 出队列 */
				 while( NULL != ( CanMessageRX = rng_dequeue ( &RingInBuffer ) ) ) 
				 {
						 switch ( CanMessageRX->IDE )
						 {
									#if ( CAN_FRAME_STD == CAN_IDE_TYPE )||( CAN_FRAME_EXT_AND_STD == CAN_IDE_TYPE )
									 case CAN_ID_STD:
				//	                can.RxBuffer[ 0 ] = ( u8 )( CanMessageRX->StdId >> 8 );
				//	                can.RxBuffer[ 1 ] = ( u8 )( CanMessageRX->StdId );
				//	  	            memcpy ( ( u8* )&can.RxBuffer[ 2 ], CanMessageRX->Data, CanMessageRX->DLC );
				//	                can.RxBufSiz = CanMessageRX->DLC + 2;
				//									can.CAN_FrameType = CAN_FRAME_STD;
													can.CAN_FrameID=CanMessageRX->StdId;
													can.RxBufSiz=CanMessageRX->DLC;
													can.CAN_FrameType = CAN_FRAME_STD;
													memcpy (can.RxBuffer, CanMessageRX->Data, can.RxBufSiz );
													if ( can.RxBufSiz > 0 )
													{
														 can.DataHandler(can.CAN_FrameID,can.RxBufSiz,  can.RxBuffer );
														 can.RxBufSiz = 0; 
														 can.CAN_FrameID=0;
													}
									 break;
									#endif  /* CAN_FRAME_STD */
					
									#if ( CAN_FRAME_EXT == CAN_IDE_TYPE )||( CAN_FRAME_EXT_AND_STD == CAN_IDE_TYPE )  		
									 case CAN_ID_EXT:
				//	                can.RxBuffer[ 0 ] = ( u8 )( CanMessageRX->ExtId >> 24 );
				//	                can.RxBuffer[ 1 ] = ( u8 )( CanMessageRX->ExtId >> 16 );
				//	                can.RxBuffer[ 2 ] = ( u8 )( CanMessageRX->ExtId >> 8 );
				//	                can.RxBuffer[ 3 ] = ( u8 )( CanMessageRX->ExtId );
				//									memcpy ( ( u8* )&can.RxBuffer[ 4 ], CanMessageRX->Data, CanMessageRX->DLC );
				//	                can.RxBufSiz = CanMessageRX->DLC + 4;
				//	                can.CAN_FrameType = CAN_FRAME_EXT;
													can.CAN_FrameID=(u8)CanMessageRX->ExtId;
													can.RxBufSiz=CanMessageRX->DLC;
													can.CAN_FrameType = CAN_FRAME_EXT;
													memcpy (can.RxBuffer, CanMessageRX->Data, can.RxBufSiz );
													if ( can.RxBufSiz > 0 )
													{
														 can.DataHandler (CanMessageRX->ExtId,can.RxBufSiz,  can.RxBuffer );
														 can.RxBufSiz = 0;
														 can.CAN_FrameID=0;
													}
					
									break;
									#endif  /* CAN_FRAME_EXT */
						
									default:
									break;
						 }
						 
						 
				}
		}

}

#else
void  CAN_RECEIVE_DATA_Handle(void)
{
   //ITStatus Status = RESET;
   CanRxMsg* RxMessage = NULL;
   if ( can.DataHandler )
   {
	   /* 出队列 */
	   while( NULL != ( RxMessage = rng_dequeue ( &RingInBuffer ) ) ) 
	   {


		
	  	   switch ( RxMessage->IDE )
	  	   {
							#if ( CAN_FRAME_STD == CAN_IDE_TYPE )||( CAN_FRAME_EXT_AND_STD == CAN_IDE_TYPE )
							 case CAN_ID_STD:
											can.RxBuffer[ 0 ] = ( u8 )( RxMessage->StdId >> 8 );
											can.RxBuffer[ 1 ] = ( u8 )( RxMessage->StdId );
											memcpy ( ( u8* )&can.RxBuffer[ 2 ], RxMessage->Data, RxMessage->DLC );
											can.RxBufSiz = RxMessage->DLC + 2;
											can.CAN_FrameType = CAN_FRAME_STD;
											break;
							#endif  /* CAN_FRAME_STD */
			
							#if ( CAN_FRAME_EXT == CAN_IDE_TYPE )||( CAN_FRAME_EXT_AND_STD == CAN_IDE_TYPE )  		
							 case CAN_ID_EXT:
											can.RxBuffer[ 0 ] = ( u8 )( RxMessage->ExtId >> 24 );
											can.RxBuffer[ 1 ] = ( u8 )( RxMessage->ExtId >> 16 );
											can.RxBuffer[ 2 ] = ( u8 )( RxMessage->ExtId >> 8 );
											can.RxBuffer[ 3 ] = ( u8 )( RxMessage->ExtId );
											memcpy ( ( u8* )&can.RxBuffer[ 4 ], RxMessage->Data, RxMessage->DLC );
											can.RxBufSiz = RxMessage->DLC + 4;
											can.CAN_FrameType = CAN_FRAME_EXT;
											break;
							#endif  /* CAN_FRAME_EXT */
				
	            default:
											break;
	  	   }

				if ( can.RxBufSiz > 0 )
	      {
					
	         //can.DataHandler ( can.RxBufSiz, can.RxBuffer );
	         can.RxBufSiz = 0; 
	      }
	  }
  }

}


#endif





