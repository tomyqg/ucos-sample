

#ifndef __BUFFER_HELPER_H__
#define __BUFFER_HELPER_H__

#include "stm32f4xx.h"
#include "stdbool.h"



typedef struct FIFO_Type_RINGBUF  
{  
    u16   Depth;          // Fifo深度  
    u16   Head;           // Head为起始数据 
    u16   Tail;           // Tail-1为最后一个元素  
    u16   Counter;        // 数据计数器（数据条数） 
    u16   DataBytes;      // 每条数据的字节数  
    u8  *Buff;          // 缓存区  
}FIFO_Type;  
 

extern FIFO_Type  RingBuf;
extern FIFO_Type *pRingBuf;














/************************************************************
 *V1.0 FIFO接口函数	@@@@goly								*
 *实现功能：												*
 *		  单条或多条数据添加，数据读取，清空FIFO			*
 *												2017.1.10	*
 ************************************************************/


 /*环形缓存初始化*/
void FIFO_Init(FIFO_Type *pFIFO,u8* databuf,u16 DataBytes, u16 depth); 
 /*添加一条数据到缓存*/   
u8 FIFO_AddOne(FIFO_Type *pFIFO, void *pValue);  
 /*添加多条数据*/  
u16 FIFO_Adds(FIFO_Type *pFIFO, void *pValues, u16 bytesToAdd);  
 /*获取一条数据*/ 
u8 FIFO_GetOne(FIFO_Type *pFIFO, void *pValue);  
 /*获取多条数据*/  
u16 FIFO_Gets(FIFO_Type *pFIFO, void *pValues, u16 bytesToRead);  
 /*清空环形缓存*/ 
void FIFO_Clear(FIFO_Type *pFIFO);  
 /*缓存是否已满*/ 
extern u8 FIFO_IsFull(FIFO_Type *pFIFO);











/******************************************/
/*              缓存参数[配置]            */
/******************************************/

/* 定义错误码 */
#define SHABUF_ERR_NONE            0    /* 正常 */
#define SHABUF_ERR_UNINIT         -1    /* 未初始化 */
#define SHABUF_ERR_NO_RES         -2    /* 无资源 */
#define SHABUF_ERR_PTR_INVALID    -3    /* 无效指针 */





/******************************************/
/*              外部函数[声明]            */
/******************************************/

/* 共享缓存初始化 */
extern void SHABUF_InitBuffer ( void );

/* 共享缓存分配 */
extern u8*  SHABUF_GetBuffer ( u16 length );

/* 共享缓存释放 */
//extern bool SHABUF_FreeBuffer ( u8* Data );//ADDgulin
extern bool SHABUF_FreeBuffer ( void* Data );

/* 共享缓存获取上次错误码 */
extern s8   SHABUF_GetLastErrCode ( void );

/* 共享缓存清空上次错误码 */
extern bool SHABUF_ClearLastErrCode ( void );

/* 共享缓存分配图显示 */
extern void SHABUF_ShowBufferMap ( void );







#endif  /* __BUFFER_HELPER_H__ */

