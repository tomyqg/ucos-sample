



#include "rexmit_strategy.h"
#include <string.h>


//////////////////////////////////////////////////////

/* �����ش����Կ��� */
#define XMIT_USE_DEBUG      0

//////////////////////////////////////////////////////

enum{
		FALSE = 0,
		TRUE = !FALSE
	};
#define NULL 0                 


typedef struct 
{
  /* �ش���Ч��־ */
  u8 xmitIsValid;

  /* �ش����� */
  u8 xmitType;

#if 0 /* ɾ�����ֶ� */ 
  /* �ش�Ӧ�� */
  u8 xmitAck;
#endif

  /* �ش�ʣ������� */
  s8 xmitLeftRetries;

  /* �ش����� */
  u8 *xmitPayload;

  /* �ش����ݳ��� */
  u16 xmitLength;

  /* �ش�ʣ��ʱ�� */
  s16 xmitTimeout;

  /* �ش����ؼ����� */
  u16 xmitReloadTime;
} XMIT_IfTypedef;


/* ���岢����ʱ������� */
#define XMIT_MAX_COUNT      6
#define XMIT_BASE_ADD(x)    ((x)+0x80)
#define XMIT_BASE_SUB(x)    ((x)-0x80)
#define XMIT_IS_VALID(x)    ((x)>=0x80)





/******************************************/
/*              �ڲ�����[����]            */
/******************************************/

/* ��ʷ�����ش������� */
static u8 xmitMaxTO;

/* ��ǰ�����ش����� */
static u8 xmitCurrentTO;

/* ���峬ʱ�ش����ƿ� */
static XMIT_IfTypedef rexmitTo[ XMIT_MAX_COUNT ];

/* �����ش���ʱ������ */
static xmit_TimeoutHdlr_f rexmitTimeoutHandler = NULL;

/* ������������˿� */
static xmit_OutPortHdlr_f rexmitDataPort = NULL;

#define XMIT_TO_INIT      0x00  /* ����ʼ�� */
#define XMIT_TO_RUN       0x01  /* ���� */

/* ���峬ʱ�ش�״̬ */
static u8 xmitToState = XMIT_TO_INIT;
#define XMIT_IS_READY()   ( xmitToState == XMIT_TO_RUN )

/* ����ִ�д����� */
static s8 xmitLastErrCode = XMIT_ERR_OK;







/* 
 * ������������ȡ�ϴδ�����
 * ���ò�������
 *          
 * ����ֵ  ��״̬��
 * 
 */
extern s8 XMIT_GetLastErrCode ( void )
{
  return xmitLastErrCode;
}






/* 
 * ������������մ������
 * ���ò�������
 *          
 * ����ֵ  ��״̬��
 * 
 */
extern bool XMIT_ClearLastErrCode ( void )
{
  xmitLastErrCode = XMIT_ERR_OK;
  return TRUE;
}





/* 
 * ������������ʼ���ش�������������
 * ���ò�������
 *          
 * ����ֵ  ��״̬��
 * 
 */
extern void XMIT_InitTimeout ( xmit_OutPortHdlr_f xmitOP )
{

#if XMIT_USE_DEBUG == 1
  printf ( "\r\n[XMIT INFO] init timeout port\r\n" );
#endif  /* XMIT_USE_DEBUG */

  if ( xmitOP && XMIT_IS_READY() == FALSE )
  {
    xmitToState = XMIT_TO_RUN;
    rexmitDataPort = xmitOP;
    xmitMaxTO = 0;
    xmitCurrentTO = 0;
    rexmitTimeoutHandler = NULL;
    xmitLastErrCode = XMIT_ERR_OK;
    memset ( rexmitTo, 0, ( XMIT_MAX_COUNT * sizeof ( XMIT_IfTypedef ) ) );
  }
}


/* 
 * ����������������ʱ����
 * ���ò�����(1)�ش�����
 *           (2)�ش�����
 *           (3)�ش����
 *           (4)�ش�����
 *           (5)�ش���С
 *   
 * ����ֵ  ��״̬��
 * 
 */
extern s32 XMIT_StartupTimeout ( u8 type, u8 retry, s16 timeout, u8* data, u16 size )
{

#if XMIT_USE_DEBUG == 1
  printf ( "\r\n[XMIT INFO] start timeout type %4u retry %4u timeout %4ds msg 0x%X size %4u\r\n",
           type, retry, timeout, data, size );
#endif  /* XMIT_USE_DEBUG */


  if ( XMIT_IS_READY () == FALSE )
  {

#if XMIT_USE_DEBUG == 1
    printf ( "\r\n[XMIT INFO] error xmit isn't ready***\r\n" );
#endif  /* XMIT_USE_DEBUG */

    xmitLastErrCode = XMIT_ERR_INIT;
  }
  else
  if ( xmitCurrentTO >= XMIT_MAX_COUNT )
  {

#if XMIT_USE_DEBUG == 1
    printf ( "\r\n[XMIT INFO] error reach MAX xmit blks***\r\n" );
#endif  /* XMIT_USE_DEBUG */

    xmitLastErrCode = XMIT_ERR_NO_RES;
  }
  else
  if ( timeout > 0 && data && size > 0 )
  {
    u32 i = 0;
    XMIT_IfTypedef *xm = rexmitTo;

    for ( i = 0; i < XMIT_MAX_COUNT; i++ )
    {
      if ( xm->xmitIsValid == FALSE )
      {

#if XMIT_USE_DEBUG == 1
        printf ( "\r\n[XMIT INFO] get %u xmit blk\r\n", i );
#endif  /* XMIT_USE_DEBUG */

        /* ���³�ʱ������Ϣ */
        xm->xmitPayload = data;
        xm->xmitType = type;

#if 0 /* ɾ�����ֶ� */
        xm->xmitAck = 0;
#endif
        xm->xmitLeftRetries = retry;                   
        xm->xmitLength = size;
        xm->xmitTimeout = timeout;
        xm->xmitReloadTime = timeout;
        xmitCurrentTO++;
        xm->xmitIsValid = TRUE;
        return XMIT_BASE_ADD( i );
      }
      else
      {
        xm++;
      }
    }

    /* ���ò����쳣! */
    if ( i == XMIT_MAX_COUNT )
    {
      xmitLastErrCode = XMIT_ERR_INTERNAL;
    }
  }
  else
  {
    xmitLastErrCode = XMIT_ERR_USER_PARAMS;
  }

  return xmitLastErrCode;
}









/* 
 * ����������ȡ����ʱ����
 * ���ò�������
 *          
 * ����ֵ  ��״̬��
 * 
 */
extern bool XMIT_CancelTimeout ( s32 handle )
{
  if ( XMIT_IS_READY () == FALSE )
  {

#if XMIT_USE_DEBUG == 1
    printf ( "\r\n[XMIT INFO] error xmit isn't ready***\r\n" );
#endif  /* XMIT_USE_DEBUG */

    xmitLastErrCode = XMIT_ERR_INIT;
    return FALSE;
  }
  else 
  if ( XMIT_IS_VALID ( handle ) == TRUE )
  {
    u8 hdl = XMIT_BASE_SUB ( handle );
    XMIT_IfTypedef *xm = NULL;

    if ( hdl < XMIT_MAX_COUNT )
    {

#if XMIT_USE_DEBUG == 1
      printf ( "\r\n[XMIT INFO] cancel (0x%X) ok\r\n", handle );
#endif  /* XMIT_USE_DEBUG */

      xm = ( XMIT_IfTypedef * )&rexmitTo[ hdl ];
      if ( xm && xm->xmitIsValid == TRUE )
      {
        xmitCurrentTO--;
        memset ( xm, 0, sizeof ( XMIT_IfTypedef ) );
      }
      return TRUE;    
    }

#if XMIT_USE_DEBUG == 1
      printf ( "\r\n[XMIT INFO] error handler***\r\n" );
#endif  /* XMIT_USE_DEBUG */

  }

  xmitLastErrCode = XMIT_ERR_HANDLE;
  return FALSE;    
}








/* 
 * ����������Ѳ�쳬ʱ����
 * ���ò�������
 *          
 * ����ֵ  ����
 * 
 */
extern u32 XMIT_PollingTimeout ( void )
{
  u32 leftTime = 0xFFFFFF;

#if XMIT_USE_DEBUG == 6
  printf ( "\r\n[XMIT INFO] polling timeout list\r\n" );
#endif  /* XMIT_USE_DEBUG */

  if ( XMIT_IS_READY () == FALSE )
  {

#if XMIT_USE_DEBUG == 1
    printf ( "\r\n[XMIT INFO] error xmit isn't ready***\r\n" );
#endif  /* XMIT_USE_DEBUG */

    xmitLastErrCode = XMIT_ERR_INIT;
  }
  else
  if ( xmitCurrentTO > 0 )
  {
    u32 i = 0;
    XMIT_IfTypedef *xm = rexmitTo;

    if ( xmitCurrentTO > xmitMaxTO )
    {
      xmitMaxTO = xmitCurrentTO;
    }
  
    /* ִ�г�ʱѲ����� */
    for ( i = 0; i < XMIT_MAX_COUNT; ( i++, xm++ ) )
    {
      if ( xm->xmitIsValid == FALSE )
      {
        continue;
      }
      else 
      if ( xm->xmitTimeout > 0 )
      {
        xm->xmitTimeout--;
      }

      if ( leftTime > xm->xmitTimeout )
      {
        leftTime = xm->xmitTimeout;
      }

      /* �Ƿ�ﵽ��ʱ���?? */
      if ( xm->xmitTimeout <= 0 )
      {
        if ( xm->xmitLeftRetries <= 0 )
        {

#if XMIT_USE_DEBUG == 1
          printf ( "\r\n[XMIT INFO] %u time out type %4u msg 0x%X size %4u\r\n", 
                   i, xm->xmitType, xm->xmitPayload, xm->xmitLength );
#endif  /* XMIT_USE_DEBUG */

          if ( rexmitTimeoutHandler )
          {
            rexmitTimeoutHandler ( XMIT_BASE_ADD( i ), xm->xmitType, xm->xmitPayload, xm->xmitLength );
          }
  
          /* BUG #001 ����Դ���±����ã�����������������Դ�ͷŲ��� */
          if ( xm->xmitLeftRetries <= 0 && xm->xmitTimeout <= 0 )
          {

#if XMIT_USE_DEBUG == 1
            printf ( "\r\n[XMIT INFO] cancel (0x%X) timeout\r\n", XMIT_BASE_ADD( i ) );
#endif  /* XMIT_USE_DEBUG */

            /* �ͷų�ʱ���ƿ� */
            XMIT_CancelTimeout ( XMIT_BASE_ADD( i ) );        
          }       
        }
        else
        {
          xm->xmitLeftRetries--;
          xm->xmitTimeout = xm->xmitReloadTime;

#if XMIT_USE_DEBUG == 1
          printf ( "\r\n[XMIT INFO] timeout blk %u data type %u out\r\n", i, xm->xmitType );
#endif  /* XMIT_USE_DEBUG */

          /* ������� */
          if ( rexmitDataPort )
          {
            rexmitDataPort ( xm->xmitPayload, xm->xmitLength );
          }
        }
      } 
    }
  }

  return leftTime;
}







/* 
 * ������������ѯ��ʱ״̬
 * ���ò�������
 *          
 * ����ֵ  ��״̬��
 * 
 */
extern void XMIT_QueryTimeoutState ( s32 handle )
{

#if XMIT_USE_DEBUG == 1
  printf ( "\r\n[XMIT INFO] query (0x%X) timeout state\r\n", handle );
#endif  /* XMIT_USE_DEBUG */

  if ( XMIT_IS_READY () == FALSE )
  {

#if XMIT_USE_DEBUG == 1
    printf ( "\r\n[XMIT INFO] error xmit isn't ready***\r\n" );
#endif  /* XMIT_USE_DEBUG */

    xmitLastErrCode = XMIT_ERR_INIT;
    return ;
  }
  else
  if ( XMIT_IS_VALID ( handle ) == TRUE )
  {
    u8 hdl = XMIT_BASE_SUB ( handle );

    if ( hdl < XMIT_MAX_COUNT )
    {
      XMIT_IfTypedef *xm = ( XMIT_IfTypedef * )&rexmitTo[ hdl ];
      if ( xm && xm->xmitIsValid == TRUE )
      {
        
      }    
    }
  }

  xmitLastErrCode = XMIT_ERR_HANDLE;
}








/* 
 * ����������ע�ᳬʱ������
 * ���ò�������
 *          
 * ����ֵ  ��״̬��
 * 
 */
extern bool XMIT_SubscribeTimeoutHandler ( xmit_TimeoutHdlr_f xmitToHdlr )
{

#if XMIT_USE_DEBUG == 1
  printf ( "\r\n[XMIT INFO] set timeout handler 0x%X\r\n", xmitToHdlr );
#endif  /* XMIT_USE_DEBUG */

  if ( XMIT_IS_READY () == FALSE )
  {

#if XMIT_USE_DEBUG == 1
    printf ( "\r\n[XMIT INFO] error xmit isn't ready***\r\n" );
#endif  /* XMIT_USE_DEBUG */

    xmitLastErrCode = XMIT_ERR_INIT;
    return FALSE;
  }
  else
  if ( xmitToHdlr )
  {
    rexmitTimeoutHandler = xmitToHdlr;
    return TRUE;
  }

  return FALSE;
}




