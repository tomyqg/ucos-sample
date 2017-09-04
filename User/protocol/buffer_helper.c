



#include "buffer_helper.h"

enum{
		FALSE = 0,
		TRUE = !FALSE
	};
#define NULL 0 


//////////////////////////////////////////////////////////

/* ���干������� */
#define SHABUF_USE_DEBUG      0

/* ����ͳ�ƹ��� */
#define SHABUF_USE_STATISTIC  0

//////////////////////////////////////////////////////////




/*�����ٽ翪��*/
#define BUF_ENTER_CTL_SW   		1

/* �����ܻ����С */
#define BUFFER_TOTAL_SIZE       (1024*8)  /* ��λ��byte */

/* ���建��Ƭ�δ�С */
#define BUFFER_SEGMENT_SIZE     128   /* ��λ��byte */

/* ���建��Ƭ������ */
#define BUFFER_SEGMENT_NUMBER   ( BUFFER_TOTAL_SIZE / BUFFER_SEGMENT_SIZE )



/******************************************/
/*              �ڲ�����[����]            */
/******************************************/
	

/* �������ݴ洢 */
static u8 shareBuffer[ BUFFER_TOTAL_SIZE ] = { 0 };

/* ����λͼ */
static vu32 shareBitMap = 0;

/* ����λͼ�������� */
static vu8  shareLenRecorder[ BUFFER_SEGMENT_NUMBER ];

/* ����ʣ�೤�� */
static vu32 shareLeftNbr;

/* ����״̬���� */
static u8 shareState;
#define SHARE_BUFFER_IS_INVALID   0x00
#define SHARE_BUFFER_IS_VALID     0x01

/* �������Ƿ����?? */
#define SHARE_IS_READY_STATE()    ( shareState > 0 )

/* ����ִ�д����� */
static s8 shareLastErrCode = SHABUF_ERR_NONE;

#if SHABUF_USE_STATISTIC == 1
/* ����ʧ�ܴ��� */
static vu32 shareGetOKCount;

/* ����ʧ�ܴ��� */
static vu32 shareGetErrCount;

/* ����ʧ�ܴ��� */
static vu32 shareFreeErrCount;
#endif  /* SHABUF_USE_STATISTIC */


/*
******************************************************
*													 *
*			FIFO���� �������ϴ������¼�����								 *
*													 *
******************************************************/
 

 FIFO_Type  RingBuf; //FIFO����
 FIFO_Type *pRingBuf=&RingBuf; //FIFOָ��

/******************************************/
/*              �ڲ�����[����]            */
/******************************************/

static s32 SHABUF_HasVliadBuffer ( u8 bufNbr );





/* 
 ******************************************************************************
 * ������������ʼ�����λ���
 * ���ò�����
 *		   pFIFO     ��FIFIָ��
 *		   DataBytes ��ÿ�����ݵ��ֽ���
 *         depth	 ��FIFO����ȣ��ɴ洢������������
 * ����ֵ  ����
 * ������  ��goly
 * *****************************************************************************
 */
void FIFO_Init(FIFO_Type *pFIFO,u8* databuf,u16 DataBytes, u16 depth)  
{

    pFIFO->Buff = databuf;  
    pFIFO->DataBytes = DataBytes;  
    pFIFO->Depth = depth;  
    pFIFO->Head = 0;  
    pFIFO->Tail = 0;  
    pFIFO->Counter = 0;   
}

 
/* 
 ******************************************************************************
 * �����������ж�FIFO�Ƿ�Ϊ��
 * ���ò�����
 *		   pFIFO ��FIFIָ��
 *
 * ����ֵ  ��1-TRUE ; 0-FALSE;
 * ������  ��goly
 *******************************************************************************
 */
u8 FIFO_IsEmpty(FIFO_Type *pFIFO)  
{  
    return (pFIFO->Counter == 0);  
} 


/* 
 ******************************************************************************
 * �����������ж�FIFO�Ƿ�����
 * ���ò�����
 *		   pFIFO ��FIFIָ��
 *
 * ����ֵ  ��1-TRUE ; 0-FALSE;
 * ������  ��goly
 *******************************************************************************
 */
u8 FIFO_IsFull(FIFO_Type *pFIFO)  
{  
    return (pFIFO->Counter == pFIFO->Depth);  
}

/* 
 ******************************************************************************
 * ������������FIFO���һ������
 * ���ò�����
 *		   pFIFO     ��FIFIָ��
 *		   pValue    ��Ҫ��ӵ�����ָ��
 *         
 * ����ֵ  ��1-TRUE or 0-FALSE
 * ������  ��goly
 *******************************************************************************
 */ 
u8 FIFO_AddOne(FIFO_Type *pFIFO, void *pValue)  
{  
    u8 *p;  
  
    if (FIFO_IsFull(pFIFO))  
    {  
        return 0;  
    }  
  
    p = (u8 *)pFIFO->Buff;  
    memcpy(p + pFIFO->Tail * pFIFO->DataBytes, (u8 *)pValue, pFIFO->DataBytes);  
      
    pFIFO->Tail ++;  
    if (pFIFO->Tail >= pFIFO->Depth)  
    {  
        pFIFO->Tail = 0;  
    }  
    pFIFO->Counter ++;  
    return 1;  
} 


 /* 
 ******************************************************************************
 * ������������FIFO��Ӷ�������
 * ���ò�����
 *		   pFIFO     ��FIFIָ��
 *		   pValue    ��Ҫ��ӵ�����ָ��
 *         bytesToAdd��Ҫ��ӵ���������
 * ����ֵ  ��cnt����ӵ���������
 * ������  ��goly
 *******************************************************************************
 */
u16 FIFO_Adds(FIFO_Type *pFIFO, void *pValues, u16 bytesToAdd)  
{  
    u8 *p;  
    u16 cnt = 0;  
  
    p = (u8 *)pValues;  
    while(bytesToAdd --)  
    {  
        if (FIFO_AddOne(pFIFO, p))  
        {  
            p += pFIFO->DataBytes;  
            cnt++;  
        }  
        else  
        {  
            break;  
        }  
    }  
  
    return cnt;  
}

  
 /* 
 ******************************************************************************
 * ������������FIFO��ȡһ������
 * ���ò�����
 *		   pFIFO     ��FIFIָ��
 *		   pValue    ������Ѷ����ݵ�ָ��
 *         
 * ����ֵ  ��1-TRUE or 0-FALSE
 * ������  ��goly
 *******************************************************************************
 */
u8 FIFO_GetOne(FIFO_Type *pFIFO, void *pValue)  
{  
    u8 *p;  
    if (FIFO_IsEmpty(pFIFO))  
    {  
        return 0;  
    }  
  
    p = (u8 *)pFIFO->Buff;  
    memcpy(pValue, p + pFIFO->Head * pFIFO->DataBytes, pFIFO->DataBytes);  
  
    pFIFO->Head ++;  
    if (pFIFO->Head >= pFIFO->Depth)  
    {  
        pFIFO->Head = 0;  
    }  
    pFIFO->Counter --;  
  
    return 1;  
} 

  
 /* 
 ******************************************************************************
 * ������������FIFO��ȡ��������
 * ���ò�����
 *		   pFIFO     ��FIFIָ��
 *		   pValue    ������Ѷ����ݵ�ָ��
 *         bytesToAdd��Ҫ��ȡ����������
 * ����ֵ  ��cnt��ʵ�ʶ�ȡ����������
 * ������  ��goly
 *******************************************************************************
 */
u16 FIFO_Gets(FIFO_Type *pFIFO, void *pValues, u16 bytesToRead)  
{  
    u16 cnt = 0;  
    u8 *p;  
  
    p =(u8 *) pValues;  
    while(bytesToRead--)  
    {  
        if (FIFO_GetOne(pFIFO, p))  
        {  
            p += pFIFO->DataBytes;  
            cnt++;  
        }  
        else  
        {  
            break;  
        }  
    }  
  
    return cnt;  
}
 
 /* 
 ******************************************************************************
 * �������������FIFO
 * ���ò�����
 *		   pFIFO     ��FIFIָ��
 * ����ֵ  ����
 * ������  ��goly
 *******************************************************************************
 */
void FIFO_Clear(FIFO_Type *pFIFO)  
{
	if(pFIFO->Buff)
	{
		free(pFIFO->Buff); 
		pFIFO->Buff=NULL;
	} 
    
	pFIFO->Counter = 0;  
    pFIFO->Head = 0;  
    pFIFO->Tail = 0;  
}







/* 
 * ������������ȡ�ϴδ�����
 * ���ò�������
 *          
 * ����ֵ  ��״̬��
 * 
 */
extern s8 SHABUF_GetLastErrCode ( void )
{
#if SHABUF_USE_DEBUG == 1
  printf ( "\r\n[SHA BUF] last error code 0x%X\r\n", shareLastErrCode );
#endif  /* SHABUF_USE_DEBUG */

  return shareLastErrCode;
}






/* 
 * ������������մ������
 * ���ò�������
 *          
 * ����ֵ  ��״̬��
 * 
 */
extern bool SHABUF_ClearLastErrCode ( void )
{

#if SHABUF_USE_DEBUG == 1
  printf ( "\r\n[SHA BUF] clear last error code.\r\n" );
#endif  /* SHABUF_USE_DEBUG */

  shareLastErrCode = SHABUF_ERR_NONE;
  return TRUE;
}






/* 
 * ������������ʼ���������
 * ���ò�������
 *          
 * ����ֵ  ��״̬��
 * 
 */
extern void SHABUF_InitBuffer ( void )
{

#if SHABUF_USE_DEBUG == 1
  printf ( "\r\n[SHA BUF] init buffer start.\r\n" );
#endif  /* SHABUF_USE_DEBUG */

  shareLeftNbr = BUFFER_SEGMENT_NUMBER;
  shareBitMap = 0;

#if SHABUF_USE_STATISTIC == 1
  shareGetErrCount = 0;
  shareGetOKCount = 0;
  shareFreeErrCount = 0;
#endif  /* SHABUF_USE_STATISTIC */

  shareState = SHARE_BUFFER_IS_VALID;
  memset ( ( u8* )shareLenRecorder, 0, shareLeftNbr );
  memset ( shareBuffer, 0, BUFFER_TOTAL_SIZE );
}







/* 
 * ������������������Ч�Ļ���Ƭ��
 * ���ò�������
 *          
 * ����ֵ  ��״̬��
 * 
 */
static s32 SHABUF_HasVliadBuffer ( u8 bitNbr )
{
  s32 match = -1;

#if SHABUF_USE_DEBUG == 1
  printf ( "\r\n[SHA BUF] search valid buffer, need %u segs.\r\n", bitNbr );
#endif  /* SHABUF_USE_DEBUG */

  /* �Ƿ�ʣ���㹻�Ļ�������?? */
  if ( bitNbr <= shareLeftNbr )
  {
    u32 i = 0;
    u32 j = 0;
    u32 totoal = BUFFER_SEGMENT_NUMBER;
    u32 avail = 0;

#if SHABUF_USE_DEBUG == 1
    printf ( "\r\n[SHA BUF] left %u segs.\r\n", shareLeftNbr );
#endif  /* SHABUF_USE_DEBUG */

    /* �Ƿ���������ռ�?? */
    for ( i = 0; i < totoal; i++ )
    {
      /* �����״�����ƥ��Ľ�� */
      if ( j >= bitNbr && match == -1 )
      {
        match = i - j;         
      }

      if ( ( shareBitMap >> i ) & 0x01 )
      {
        /* �Ƿ�����������л�������Ȼ���?? */
        if ( j >= bitNbr && ( avail > j || avail == 0 ) )
        {
          avail = j;
          match = i - j;

          /* ������ȫƥ��ʱ������ֹ���� */
          if ( j == bitNbr )
          {
            return match;
          }
        }
        j = 0;
      }
      else
      {
        j++;
      }
    }

    /* BUG #001 j�������ﵽҪ�󣬵���i������δ����ʱ��
     * �����״�����ƥ��Ľ�� */
    if ( j >= bitNbr && match == -1 )
    {
      match = i - j;         
    }
  }

  return match;
}








/* 
 * ������������ȡ����
 * ���ò�������
 *          
 * ����ֵ  ��״̬��
 * 
 */
extern u8* SHABUF_GetBuffer ( u16 length )
{

#if SHABUF_USE_DEBUG == 1
  printf ( "\r\n[SHA BUF] get user buffer size %u.\r\n", length );
#endif  /* SHABUF_USE_DEBUG */

  #if BUF_ENTER_CTL_SW
	  #ifdef OS_CRITICAL_METHOD
	  OS_CPU_SR     cpu_sr;
	  OS_ENTER_CRITICAL();
	  #endif
  #endif
  if ( SHARE_IS_READY_STATE() == FALSE )
  {

#if SHABUF_USE_DEBUG == 1
    printf ( "\r\n[SHA BUF] error uninit***\r\n" );
#endif  /* SHABUF_USE_DEBUG */

    shareLastErrCode = SHABUF_ERR_UNINIT;

	#if BUF_ENTER_CTL_SW
		#ifdef OS_CRITICAL_METHOD
  		OS_EXIT_CRITICAL();
		#endif
  	#endif
    return NULL;
  }
  else
  if ( length > 0 )
  {
    u32 bitNbr = 0;
    s32 match = 0;

    /* �����������������Ƭ������ */
    bitNbr = length / BUFFER_SEGMENT_SIZE;
    if ( length % BUFFER_SEGMENT_SIZE )
    {
      bitNbr++;
    }

    /* ��������ƥ�仺�� */
    match = SHABUF_HasVliadBuffer ( bitNbr );

    /* ��������ʱ */
    if ( match >= 0 && match < BUFFER_SEGMENT_NUMBER )
    {
      u32 i = 0;

      match += bitNbr;

      /* ��������Ļ�����Դ */
      for ( i = 0; i < bitNbr; i++ )
      {
        match--;
        shareBitMap |= ( 1 << match );
      }
      
      shareLeftNbr -= bitNbr;
      shareLenRecorder[ match ] = bitNbr;

#if SHABUF_USE_STATISTIC == 1
      shareGetOKCount++;
#endif  /* SHABUF_USE_STATISTIC */

#if SHABUF_USE_DEBUG == 3
      SHABUF_ShowBufferMap ();
#endif  /* SHABUF_USE_DEBUG */

      /* �����û��洢��ַ */
	  #if BUF_ENTER_CTL_SW 
	  	#ifdef OS_CRITICAL_METHOD
	  	OS_EXIT_CRITICAL();
		#endif
	  #endif
      return ( u8* )&shareBuffer[ match * BUFFER_SEGMENT_SIZE ];
    }

#if SHABUF_USE_DEBUG == 1
    printf ( "\r\n[SHA BUF] error no enough free buffer***\r\n" );
#endif  /* SHABUF_USE_DEBUG */

    shareLastErrCode = SHABUF_ERR_NO_RES;
  }

#if SHABUF_USE_STATISTIC == 1
  shareGetErrCount++;
#endif  /* SHABUF_USE_STATISTIC */

  #if BUF_ENTER_CTL_SW 
  	#ifdef OS_CRITICAL_METHOD
  	OS_EXIT_CRITICAL();
	#endif
  #endif
  return NULL;
}








/* 
 * �����������ͷŻ���
 * ���ò�������
 *          
 * ����ֵ  ��״̬��
 * 
 */
//extern bool SHABUF_FreeBuffer ( u8* Data )//ADDgulin
extern bool SHABUF_FreeBuffer ( void* Data )
{

#if SHABUF_USE_DEBUG == 1
  printf ( "\r\n[SHA BUF] free user buffer addr 0x%X.\r\n", Data );
#endif  /* SHABUF_USE_DEBUG */
  #if BUF_ENTER_CTL_SW
  	#ifdef OS_CRITICAL_METHOD
	OS_CPU_SR     cpu_sr;
	OS_ENTER_CRITICAL();
	#endif 
  #endif
  if ( SHARE_IS_READY_STATE() == FALSE )
  {

#if SHABUF_USE_DEBUG == 1
    printf ( "\r\n[SHA BUF] error uninit***\r\n" );
#endif  /* SHABUF_USE_DEBUG */

    shareLastErrCode = SHABUF_ERR_UNINIT;
	#if BUF_ENTER_CTL_SW
		#ifdef OS_CRITICAL_METHOD
  		OS_EXIT_CRITICAL();
		#endif
  	#endif
    return FALSE;
  }
  else
  //if ( Data && Data >= shareBuffer )//ADDgulin
	if ( (u8*)Data && (u8*)Data >= shareBuffer )
  {
    u32 offset = ( (u8*)Data - shareBuffer );
    u32 hit = 0;

    /* �Ƿ��û��Ƿ�Ļ����ַ��Ч?? */
    if ( offset < BUFFER_TOTAL_SIZE && ( offset % BUFFER_SEGMENT_SIZE ) == 0 )
    {
      hit = offset / BUFFER_SEGMENT_SIZE;

      /* ��ѯ����λͼ״̬�Ƿ���ȷ?? */
      if ( ( shareBitMap >> hit ) & 0x01 )
      {
        /* �Ƿ�洢�˸��û�������?? */
        if ( shareLenRecorder[ hit ] > 0 )
        {
          u32 i = 0;
          u32 length = shareLenRecorder[ hit ] * BUFFER_SEGMENT_SIZE;

          /* ɾ���û��������� */
          memset ( ( u8* )&shareBuffer[ hit * BUFFER_SEGMENT_SIZE ], 0, length );

          /* �ͷŻ������ָʾ��Ϣ */
          shareLeftNbr += shareLenRecorder[ hit ];
          for ( i = 0; i < shareLenRecorder[ hit ]; i++ )
          {
            shareBitMap &= ( ~( 1 << ( hit + i ) ) );
          }
          shareLenRecorder[ hit ] = 0;

#if SHABUF_USE_DEBUG == 3
          SHABUF_ShowBufferMap ();
#endif  /* SHABUF_USE_DEBUG */
		  #if BUF_ENTER_CTL_SW
		  	#ifdef OS_CRITICAL_METHOD
		  	OS_EXIT_CRITICAL();
			#endif
		  #endif
          return TRUE;
        }
      }
    }

#if SHABUF_USE_DEBUG == 1
    printf ( "\r\n[SHA BUF] user-ptr is invalid***\r\n" );
#endif  /* SHABUF_USE_DEBUG */

#if SHABUF_USE_STATISTIC == 1
    shareFreeErrCount++;
#endif  /* SHABUF_USE_STATISTIC */

    shareLastErrCode = SHABUF_ERR_PTR_INVALID;
  }
  #if BUF_ENTER_CTL_SW
  	#ifdef OS_CRITICAL_METHOD
  	OS_EXIT_CRITICAL();
	#endif
  #endif
  return FALSE;
}







/* 
 * �������������������ݷֲ���ʾ
 * ���ò�������
 *          
 * ����ֵ  ��״̬��
 * 
 */
extern void SHABUF_ShowBufferMap ( void )
{
  u32 i = 0;

  printf ( "\r\n=============================\r\n" );
  printf ( "\r\ntotal blks %4u used %4u left %4u", 
           BUFFER_SEGMENT_NUMBER, 
           ( BUFFER_SEGMENT_NUMBER - shareLeftNbr ), 
           shareLeftNbr );

#if SHABUF_USE_STATISTIC == 1
  printf ( "\r\nget success %4u err %4u free err %4u\r\n",
           shareGetOKCount, 
           shareGetErrCount, 
           shareFreeErrCount );
#endif  /* SHABUF_USE_STATISTIC */

  printf ( "\r\nBUFFER BIT MAP\r\n" );
  printf ( "\r\n=============================\r\n" );
  for ( i = 0; i < BUFFER_SEGMENT_NUMBER; i++ )
  {
    if ( ( shareBitMap >> i ) & 0x01 )
    {
      printf ( " %c ", 'X' );
    }
    else
    {
      printf ( " %c ", 'O' );
    }
  }
  printf ( "\r\n=============================\r\n" );
  for ( i = 0; i < BUFFER_SEGMENT_NUMBER; i++ )
  {
    if ( shareLenRecorder[ i ] > 0 )
    {
      printf ( "[ %u ~~ %u ] ", i, ( i + shareLenRecorder[ i ] ) );
    }
  }
  printf ( "\r\n=============================\r\n" );
}
