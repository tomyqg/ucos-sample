

#ifndef __BUFFER_HELPER_H__
#define __BUFFER_HELPER_H__

#include "stm32f4xx.h"
#include "stdbool.h"



typedef struct FIFO_Type_RINGBUF  
{  
    u16   Depth;          // Fifo���  
    u16   Head;           // HeadΪ��ʼ���� 
    u16   Tail;           // Tail-1Ϊ���һ��Ԫ��  
    u16   Counter;        // ���ݼ����������������� 
    u16   DataBytes;      // ÿ�����ݵ��ֽ���  
    u8  *Buff;          // ������  
}FIFO_Type;  
 

extern FIFO_Type  RingBuf;
extern FIFO_Type *pRingBuf;














/************************************************************
 *V1.0 FIFO�ӿں���	@@@@goly								*
 *ʵ�ֹ��ܣ�												*
 *		  ���������������ӣ����ݶ�ȡ�����FIFO			*
 *												2017.1.10	*
 ************************************************************/


 /*���λ����ʼ��*/
void FIFO_Init(FIFO_Type *pFIFO,u8* databuf,u16 DataBytes, u16 depth); 
 /*���һ�����ݵ�����*/   
u8 FIFO_AddOne(FIFO_Type *pFIFO, void *pValue);  
 /*��Ӷ�������*/  
u16 FIFO_Adds(FIFO_Type *pFIFO, void *pValues, u16 bytesToAdd);  
 /*��ȡһ������*/ 
u8 FIFO_GetOne(FIFO_Type *pFIFO, void *pValue);  
 /*��ȡ��������*/  
u16 FIFO_Gets(FIFO_Type *pFIFO, void *pValues, u16 bytesToRead);  
 /*��ջ��λ���*/ 
void FIFO_Clear(FIFO_Type *pFIFO);  
 /*�����Ƿ�����*/ 
extern u8 FIFO_IsFull(FIFO_Type *pFIFO);











/******************************************/
/*              �������[����]            */
/******************************************/

/* ��������� */
#define SHABUF_ERR_NONE            0    /* ���� */
#define SHABUF_ERR_UNINIT         -1    /* δ��ʼ�� */
#define SHABUF_ERR_NO_RES         -2    /* ����Դ */
#define SHABUF_ERR_PTR_INVALID    -3    /* ��Чָ�� */





/******************************************/
/*              �ⲿ����[����]            */
/******************************************/

/* �������ʼ�� */
extern void SHABUF_InitBuffer ( void );

/* ��������� */
extern u8*  SHABUF_GetBuffer ( u16 length );

/* �������ͷ� */
//extern bool SHABUF_FreeBuffer ( u8* Data );//ADDgulin
extern bool SHABUF_FreeBuffer ( void* Data );

/* �������ȡ�ϴδ����� */
extern s8   SHABUF_GetLastErrCode ( void );

/* ����������ϴδ����� */
extern bool SHABUF_ClearLastErrCode ( void );

/* ���������ͼ��ʾ */
extern void SHABUF_ShowBufferMap ( void );







#endif  /* __BUFFER_HELPER_H__ */

