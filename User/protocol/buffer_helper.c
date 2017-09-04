



#include "buffer_helper.h"

enum{
		FALSE = 0,
		TRUE = !FALSE
	};
#define NULL 0 


//////////////////////////////////////////////////////////

/* 定义共享缓存调试 */
#define SHABUF_USE_DEBUG      0

/* 定义统计功能 */
#define SHABUF_USE_STATISTIC  0

//////////////////////////////////////////////////////////




/*缓存临界开关*/
#define BUF_ENTER_CTL_SW   		1

/* 定义总缓存大小 */
#define BUFFER_TOTAL_SIZE       (1024*8)  /* 单位：byte */

/* 定义缓存片段大小 */
#define BUFFER_SEGMENT_SIZE     128   /* 单位：byte */

/* 定义缓存片段数量 */
#define BUFFER_SEGMENT_NUMBER   ( BUFFER_TOTAL_SIZE / BUFFER_SEGMENT_SIZE )



/******************************************/
/*              内部变量[定义]            */
/******************************************/
	

/* 共享数据存储 */
static u8 shareBuffer[ BUFFER_TOTAL_SIZE ] = { 0 };

/* 共享位图 */
static vu32 shareBitMap = 0;

/* 共享位图连续长度 */
static vu8  shareLenRecorder[ BUFFER_SEGMENT_NUMBER ];

/* 共享剩余长度 */
static vu32 shareLeftNbr;

/* 共享状态配置 */
static u8 shareState;
#define SHARE_BUFFER_IS_INVALID   0x00
#define SHARE_BUFFER_IS_VALID     0x01

/* 共享缓存是否可用?? */
#define SHARE_IS_READY_STATE()    ( shareState > 0 )

/* 定义执行错误码 */
static s8 shareLastErrCode = SHABUF_ERR_NONE;

#if SHABUF_USE_STATISTIC == 1
/* 共享失败次数 */
static vu32 shareGetOKCount;

/* 共享失败次数 */
static vu32 shareGetErrCount;

/* 共享失败次数 */
static vu32 shareFreeErrCount;
#endif  /* SHABUF_USE_STATISTIC */


/*
******************************************************
*													 *
*			FIFO配置 （数据上传报警事件处理）								 *
*													 *
******************************************************/
 

 FIFO_Type  RingBuf; //FIFO缓存
 FIFO_Type *pRingBuf=&RingBuf; //FIFO指针

/******************************************/
/*              内部函数[声明]            */
/******************************************/

static s32 SHABUF_HasVliadBuffer ( u8 bufNbr );





/* 
 ******************************************************************************
 * 功能描述：初始化环形缓存
 * 引用参数：
 *		   pFIFO     ：FIFI指针
 *		   DataBytes ：每条数据的字节数
 *         depth	 ：FIFO的深度（可存储的数据条数）
 * 返回值  ：无
 * 创建人  ：goly
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
 * 功能描述：判断FIFO是否为空
 * 引用参数：
 *		   pFIFO ：FIFI指针
 *
 * 返回值  ：1-TRUE ; 0-FALSE;
 * 创建人  ：goly
 *******************************************************************************
 */
u8 FIFO_IsEmpty(FIFO_Type *pFIFO)  
{  
    return (pFIFO->Counter == 0);  
} 


/* 
 ******************************************************************************
 * 功能描述：判断FIFO是否已满
 * 引用参数：
 *		   pFIFO ：FIFI指针
 *
 * 返回值  ：1-TRUE ; 0-FALSE;
 * 创建人  ：goly
 *******************************************************************************
 */
u8 FIFO_IsFull(FIFO_Type *pFIFO)  
{  
    return (pFIFO->Counter == pFIFO->Depth);  
}

/* 
 ******************************************************************************
 * 功能描述：向FIFO添加一条数据
 * 引用参数：
 *		   pFIFO     ：FIFI指针
 *		   pValue    ：要添加的数据指针
 *         
 * 返回值  ：1-TRUE or 0-FALSE
 * 创建人  ：goly
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
 * 功能描述：向FIFO添加多条数据
 * 引用参数：
 *		   pFIFO     ：FIFI指针
 *		   pValue    ：要添加的数据指针
 *         bytesToAdd：要添加的数据条数
 * 返回值  ：cnt：添加的数据条数
 * 创建人  ：goly
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
 * 功能描述：从FIFO读取一条数据
 * 引用参数：
 *		   pFIFO     ：FIFI指针
 *		   pValue    ：存放已读数据的指针
 *         
 * 返回值  ：1-TRUE or 0-FALSE
 * 创建人  ：goly
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
 * 功能描述：从FIFO读取多条数据
 * 引用参数：
 *		   pFIFO     ：FIFI指针
 *		   pValue    ：存放已读数据的指针
 *         bytesToAdd：要读取的数据条数
 * 返回值  ：cnt：实际读取的数据条数
 * 创建人  ：goly
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
 * 功能描述：清空FIFO
 * 引用参数：
 *		   pFIFO     ：FIFI指针
 * 返回值  ：无
 * 创建人  ：goly
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
 * 功能描述：获取上次错误码
 * 引用参数：无
 *          
 * 返回值  ：状态码
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
 * 功能描述：清空错误代码
 * 引用参数：无
 *          
 * 返回值  ：状态码
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
 * 功能描述：初始化缓存控制
 * 引用参数：无
 *          
 * 返回值  ：状态码
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
 * 功能描述：搜索最有效的缓存片段
 * 引用参数：无
 *          
 * 返回值  ：状态码
 * 
 */
static s32 SHABUF_HasVliadBuffer ( u8 bitNbr )
{
  s32 match = -1;

#if SHABUF_USE_DEBUG == 1
  printf ( "\r\n[SHA BUF] search valid buffer, need %u segs.\r\n", bitNbr );
#endif  /* SHABUF_USE_DEBUG */

  /* 是否剩余足够的缓存数量?? */
  if ( bitNbr <= shareLeftNbr )
  {
    u32 i = 0;
    u32 j = 0;
    u32 totoal = BUFFER_SEGMENT_NUMBER;
    u32 avail = 0;

#if SHABUF_USE_DEBUG == 1
    printf ( "\r\n[SHA BUF] left %u segs.\r\n", shareLeftNbr );
#endif  /* SHABUF_USE_DEBUG */

    /* 是否存在连续空间?? */
    for ( i = 0; i < totoal; i++ )
    {
      /* 保存首次搜索匹配的结果 */
      if ( j >= bitNbr && match == -1 )
      {
        match = i - j;         
      }

      if ( ( shareBitMap >> i ) & 0x01 )
      {
        /* 是否出现连续空闲缓存或优先缓存?? */
        if ( j >= bitNbr && ( avail > j || avail == 0 ) )
        {
          avail = j;
          match = i - j;

          /* 出现完全匹配时，则终止搜索 */
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

    /* BUG #001 j计数器达到要求，但是i计数器未符合时，
     * 保存首次搜索匹配的结果 */
    if ( j >= bitNbr && match == -1 )
    {
      match = i - j;         
    }
  }

  return match;
}








/* 
 * 功能描述：获取缓存
 * 引用参数：无
 *          
 * 返回值  ：状态码
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

    /* 计算所需的连续缓存片段数量 */
    bitNbr = length / BUFFER_SEGMENT_SIZE;
    if ( length % BUFFER_SEGMENT_SIZE )
    {
      bitNbr++;
    }

    /* 搜索优先匹配缓存 */
    match = SHABUF_HasVliadBuffer ( bitNbr );

    /* 条件满足时 */
    if ( match >= 0 && match < BUFFER_SEGMENT_NUMBER )
    {
      u32 i = 0;

      match += bitNbr;

      /* 分配所需的缓存资源 */
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

      /* 返回用户存储地址 */
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
 * 功能描述：释放缓存
 * 引用参数：无
 *          
 * 返回值  ：状态码
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

    /* 是否用户是否的缓存地址有效?? */
    if ( offset < BUFFER_TOTAL_SIZE && ( offset % BUFFER_SEGMENT_SIZE ) == 0 )
    {
      hit = offset / BUFFER_SEGMENT_SIZE;

      /* 查询缓存位图状态是否正确?? */
      if ( ( shareBitMap >> hit ) & 0x01 )
      {
        /* 是否存储了该用户的数据?? */
        if ( shareLenRecorder[ hit ] > 0 )
        {
          u32 i = 0;
          u32 length = shareLenRecorder[ hit ] * BUFFER_SEGMENT_SIZE;

          /* 删除用户缓存数据 */
          memset ( ( u8* )&shareBuffer[ hit * BUFFER_SEGMENT_SIZE ], 0, length );

          /* 释放缓存控制指示信息 */
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
 * 功能描述：共享缓存数据分布显示
 * 引用参数：无
 *          
 * 返回值  ：状态码
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
