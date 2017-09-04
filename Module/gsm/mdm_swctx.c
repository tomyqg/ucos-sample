






//#include "general.h"
#include "defines.h"
#include "modem.h"
#include "modem_debug.h"








extern MDM_DevTypedef *modem;

////////////////////////////////////////////////////////////////

/* �л�ͳ�Ƶ��� */
#define SWCTX_STATICS_DEBUG           1

////////////////////////////////////////////////////////////////


/* �л���� */
#define MDM_SWITCH_INETRVAL_MAX			  10		/* ��λ��100ms */

/* +++ָ��Դ��� */
#define ESCAPE_COMMAND_TRY_COUNTER    3

/* ATOָ��Դ��� */
#define ATO_COMMAND_TRY_COUNTER       3

/* modemģʽ */
typedef enum { __ONLINE__ = 0, __DATA__ } swMode_t;


typedef struct
{
  /* ģʽ */
  swMode_t dstMode;

  /* �л�״̬�Ƿ���Ч�� */
  u8 isVliad;
#define MDM_SW_STATE_IS_DEAD          0x00  /* ���κι��� */
#define MDM_SW_STATE_IS_STOPPED       0x01  /* ִ��ֹͣ�л� */
#define MDM_SW_STATE_IS_PROCESS       0x02  /* ִ���л����� */
#define MDM_SW_STATE_IS_INTERFERE     0x03  /* �ⲿ���ţ������л���ʱ */

  /* ��ʱ�� */
  tmr_t *switchTmr;

  /* ���Դ��� */
  u8 switchTryCount;

  /* �л�ԭ�� */
  u8 switchType;
  
  /* �л��Ƿ�ɹ��� */
  bool result;
} MDM_SwCtxTypedef;



#if SWCTX_PROCESS_DEBUG == 1
#include "tz_common.h"
#endif  /* SWCTX_PROCESS_DEBUG */






/******************************************/
/*              �ڲ�����[����]            */
/******************************************/

static AtCmdHandler_Status SWCTX_Handler_Escape           ( MDM_CmdTypedef *at, MDM_RspTypedef *response );
static AtCmdHandler_Status SWCTX_Handler_Return2DataState ( MDM_CmdTypedef *at, MDM_RspTypedef *response );
extern void Gl868_Command_Socket_Restore( void* ipc_data,const char connId );







/******************************************/
/*              �ڲ�����[����]            */
/******************************************/

static MDM_CmdTypedef swctx_commands[] 
= {
  /* Escape command */
  { ESCAPE_COMMAND, "+++", 
  	150, 
    ESCAPE_COMMAND_TRY_COUNTER,
  	SWCTX_Handler_Escape, SIM_UNKNOWN, FALSE },

  /* Return to Online Data State */
  { RETURN_TO_DATA_STATE, "ATO\r\n",              
  	300, 
    ATO_COMMAND_TRY_COUNTER,
    SWCTX_Handler_Return2DataState, SIM_READY, FALSE },

  { 0, NULL, 0, 0, NULL, SIM_UNKNOWN, FALSE }

}, *swctx_command_p = swctx_commands;

static MDM_SwCtxTypedef mdmSwContext = { __DATA__, FALSE, NULL, 0, MDM_Switch_Type_Unknow, FALSE };



#if SWCTX_STATICS_DEBUG == 1

/* AT�л��ɹ����� */
static s32 mdmSwAtOKCount     = 0;

/* AT�л�ʧ�ܴ��� */
static s32 mdmSwAtERRCount    = 0;

/* AT�л���ʱ���� */
static s32 mdmSwAtTOCount     = 0;

/* �����л��ɹ����� */
static s32 mdmSwDataOKCount   = 0;

/* �����л�ʧ�ܴ��� */
static s32 mdmSwDataERRCount  = 0;

/* �����л���ʱ���� */
static s32 mdmSwDataTOCount   = 0;
#endif  /* SWCTX_STATICS_DEBUG */





/******************************************/
/*              �ڲ�����[����]            */
/******************************************/

static void MDM_SwitchCtxHdlr ( u8 ID );
static bool MDM_SwitchResourceConfig ( swMode_t mode );









/* 
 * ��������������+++ָ��
 * ���ò�������
 *          
 * ����ֵ  ��״̬��
 * 
 */
extern bool SWCTX_Command_Escape ( void* ipc_data )
{
  return MDM_PostAtCmdByID ( swctx_command_p, 
      	                     ESCAPE_COMMAND, 
      	                     NULL );
}







/* 
 * ����������+++ָ����Ӧ����
 * ���ò�������
 *          
 * ����ֵ  ��״̬��
 * 
 */
static AtCmdHandler_Status SWCTX_Handler_Escape ( MDM_CmdTypedef *at, MDM_RspTypedef *response )
{
  MDM_RSP_ID code = MDM_GetResultCode ( response );

  if ( code == MDM_OK || code == MDM_NO_CARRIER )  
  {

#if SWCTX_PROCESS_DEBUG == 1
{
    PRINTF_GPS_TIME ();
    printf ( "[SWC EVENT] at-switch is ok\r\n" );
}
#endif  /* SWCTX_PROCESS_DEBUG */

#if SWCTX_STATICS_DEBUG == 1
    mdmSwAtOKCount++;
#endif  /* SWCTX_STATICS_DEBUG */
    
    /* ת��ΪAT״̬ */
    MDM_FlowSwitchV24State ( MDM_FCM_V24_STATE_AT );		
    mdmSwContext.isVliad = MDM_SW_STATE_IS_STOPPED;
    mdmSwContext.result = TRUE;	
    if ( mdmSwContext.switchType & MDM_Switch_Type_SMS_Incoming )
    {
       mdmSwContext.switchType = MDM_Switch_Type_Unknow;
      if ( modem && modem->sim && modem->sim->incoming_message )
      {
        modem->sim->incoming_message (1);
      }
    }	
    return AT_HANDLER_DONE;
  }
  else
  if ( code == MDM_TIME_OUT )
  {

#if SWCTX_PROCESS_DEBUG == 1
{
    PRINTF_GPS_TIME ();
    printf ( "[SWC EVENT] at-switch is timeout***\r\n" );
}
#endif  /* SWCTX_PROCESS_DEBUG */

#if SWCTX_STATICS_DEBUG == 1
    mdmSwAtERRCount++;
#endif  /* SWCTX_STATICS_DEBUG */

    /* ת��ΪAT״̬ */
    MDM_FlowSwitchV24State ( MDM_FCM_V24_STATE_AT );
		mdmSwContext.isVliad = MDM_SW_STATE_IS_STOPPED;
		mdmSwContext.result = TRUE;
    return AT_HANDLER_DONE_ERROR;  	
  }

  return AT_HANDLER_DONT_UNDERSTAND;	
}








/* 
 * �������������ͷ�������״ָ̬��
 * ���ò�������
 *          
 * ����ֵ  ��״̬��
 * 
 */
extern bool SWCTX_Command_Return2DataState ( void* ipc_data )
{
  return MDM_PostAtCmdByID ( swctx_command_p, 
      	                     RETURN_TO_DATA_STATE, 
      	                     NULL );
}






/* 
 * ������������������״ָ̬����Ӧ����
 * ���ò�������
 *          
 * ����ֵ  ��״̬��
 * 
 */
static AtCmdHandler_Status SWCTX_Handler_Return2DataState ( MDM_CmdTypedef *at, MDM_RspTypedef *response )
{
  MDM_RSP_ID code = MDM_GetResultCode ( response );

  if ( code == MDM_CONNECT || code == MDM_TIME_OUT )  
  {

#if SWCTX_PROCESS_DEBUG == 1
{
    PRINTF_GPS_TIME ();
    printf ( "[SWC EVENT] switch is ok\r\n" );
}
#endif  /* SWCTX_PROCESS_DEBUG */

#if SWCTX_STATICS_DEBUG == 1
    mdmSwDataOKCount++;
#endif  /* SWCTX_STATICS_DEBUG */

    /* ת��Ϊ����״̬ */
    MDM_FlowSwitchV24State ( MDM_FCM_V24_STATE_DATA );
		mdmSwContext.isVliad = MDM_SW_STATE_IS_STOPPED;
		mdmSwContext.result = TRUE;
    return AT_HANDLER_DONE;
  }
  else
  if ( code == MDM_ERROR || code == MDM_NO_CARRIER )
  {

#if SWCTX_PROCESS_DEBUG == 1
{
    PRINTF_GPS_TIME ();
    printf ( "[SWC EVENT] data-switch is err***\r\n" );
}
#endif  /* SWCTX_PROCESS_DEBUG */

#if SWCTX_STATICS_DEBUG == 1
    mdmSwDataERRCount++;
#endif  /* SWCTX_STATICS_DEBUG */
    
    /* �л�ʧ�ܣ���ʱΪATͨ��״̬ */
    mdmSwContext.isVliad = MDM_SW_STATE_IS_STOPPED;
    mdmSwContext.result  = FALSE;
    return AT_HANDLER_DONE_ERROR;			
  }
	
  return AT_HANDLER_DONT_UNDERSTAND;
}








extern void wip_NetDeInit ( void );




/* 
 * �����������л�������
 * ���ò�������
 *          
 * ����ֵ  ��״̬��
 * 
 */
static void MDM_SwitchCtxHdlr ( u8 ID )
{


/*********************************************** 
 *                 �л����̽���                *
 ***********************************************/

  /* �л�ֹͣ״̬�� */
  if ( mdmSwContext.isVliad == MDM_SW_STATE_IS_STOPPED )
	{

#if SWCTX_PROCESS_DEBUG == 1
{
    PRINTF_GPS_TIME ();
    printf ( "[SWC EVENT] modem switch is stop\r\n" );
}
#endif  /* SWCTX_PROCESS_DEBUG */

    /* ע���л���ʱ����Դ */
    mdmSwContext.isVliad = MDM_SW_STATE_IS_DEAD;
    mdmSwContext.switchTryCount = 0;


    /* ״̬�л�ʧ�� */    
    if ( mdmSwContext.result == FALSE )
    {
      if ( mdmSwContext.dstMode == __ONLINE__ )
  	  {

#if SWCTX_PROCESS_DEBUG == 1
{
        PRINTF_GPS_TIME ();
        printf ( "[SWC EVENT] at-switch is err******\r\n" );
}
#endif  /* SWCTX_PROCESS_DEBUG */

        MDM_FlowSwitchV24State ( MDM_FCM_V24_STATE_DATA );  

////////////////////////////////////////////////////

        /* ֪ͨӦ�ó����쳣 */
        MDM_FlowServiceException ( ( u8 )MDM_FCM_EVENT_SWITCH_TO );
////////////////////////////////////////////////////
  	  }
  	  else 
      if ( mdmSwContext.dstMode == __DATA__ )
  	  {

#if SWCTX_PROCESS_DEBUG == 1
{
        PRINTF_GPS_TIME ();
        printf ( "[SWC EVENT] data-switch is err******\r\n" );
}
#endif  /* SWCTX_PROCESS_DEBUG */

        /* ��Ҫ�������²��� */
	
        //MDM_FlowSwitchV24State ( MDM_FCM_V24_STATE_AT );
        //wip_NetDeInit ();  
 
////////////////////////////////////////////////////

        /* ֪ͨӦ�ó����쳣 */
        MDM_FlowServiceException ( ( u8 )MDM_FCM_EVENT_SWITCH_TO );
////////////////////////////////////////////////////
  	  }
    }
    else
    {

/****************************** 
 *   ��ѯ�Ƿ��ⲿ���У�       *
 *      Y ==> ֱ���˳�        *
 ******************************/
  
      if ( mdmSwContext.switchType & MDM_Switch_Type_Calling )
      {
        mdmSwContext.switchType = MDM_Switch_Type_Unknow;
        mdmSwContext.isVliad = MDM_SW_STATE_IS_INTERFERE;
        return ;
      }
#if 0
      if ( mdmSwContext.switchType & MDM_Switch_Type_SMS_Incoming )
      {
        mdmSwContext.switchType = MDM_Switch_Type_Unknow;
        mdmSwContext.isVliad = MDM_SW_STATE_IS_INTERFERE;
        return ;
      }

      if(mdmSwContext.switchType & MDM_Switch_Type_M2MLocate)
      {
        mdmSwContext.switchType = MDM_Switch_Type_Unknow;
        mdmSwContext.isVliad = MDM_SW_STATE_IS_INTERFERE;
        return ;
      }
#endif
      /* ��������ͨ��״̬ */
      MDM_SwitchToDataStatus ();
    }
	}


/*********************************************** 
 *                 �л����̽���                *
 ***********************************************/

  /* ����л� */
  else
  if ( mdmSwContext.isVliad == MDM_SW_STATE_IS_PROCESS )
  {

#if SWCTX_PROCESS_DEBUG == 1
{
    PRINTF_GPS_TIME ();
    printf ( "[SWC EVENT] monitor is procedding\r\n" );
}
#endif  /* SWCTX_PROCESS_DEBUG */

    ++mdmSwContext.switchTryCount;
    if ( mdmSwContext.dstMode == __ONLINE__ )
	  {

#if SWCTX_PROCESS_DEBUG == 1
{
      PRINTF_GPS_TIME ();
      printf ( "[SWC EVENT] at-switch is monitoring\r\n" );
}
#endif  /* SWCTX_PROCESS_DEBUG */

      /* ����ģʽ���ж��л��Ƿ�ʱ�� */
      if ( mdmSwContext.switchTryCount > ( ESCAPE_COMMAND_TRY_COUNTER * 6 ) )
      {

#if SWCTX_PROCESS_DEBUG == 1
{
        PRINTF_GPS_TIME ();
        printf ( "[SWC EVENT] at-switch monitor timeout***\r\n" );
}
#endif  /* SWCTX_PROCESS_DEBUG */

#if SWCTX_STATICS_DEBUG == 1
        mdmSwAtTOCount++;
#endif  /* SWCTX_STATICS_DEBUG */

        /* ����Ϊ����ģʽ�Ŀ����Խϴ� */
        MDM_FlowSwitchV24State ( MDM_FCM_V24_STATE_DATA );

        /* ע���л���Դ */
        MDM_SwitchResourceRelease ();

////////////////////////////////////////////////////

        /* ֪ͨӦ�ó����쳣 */
        MDM_FlowServiceException ( ( u8 )MDM_FCM_EVENT_SWITCH_TO );
////////////////////////////////////////////////////
      }      
	  }
	  else 
    if ( mdmSwContext.dstMode == __DATA__ )
	  {

#if SWCTX_PROCESS_DEBUG == 1
{
      PRINTF_GPS_TIME ();
      printf ( "[SWC EVENT] data-switch is monitoring\r\n" );
}
#endif  /* SWCTX_PROCESS_DEBUG */

      /* ����ģʽ���ж��л��Ƿ�ʱ�� */
      if ( mdmSwContext.switchTryCount > ( ATO_COMMAND_TRY_COUNTER * 6 ) )
      {

#if SWCTX_PROCESS_DEBUG == 1
{
        PRINTF_GPS_TIME ();
        printf ( "[SWC EVENT] data-switch monitor timeout***\r\n" );
}
#endif  /* SWCTX_PROCESS_DEBUG */

#if SWCTX_STATICS_DEBUG == 1
        mdmSwDataTOCount++;
#endif  /* SWCTX_STATICS_DEBUG */
        
        /* ע���л���Դ */
        MDM_SwitchResourceRelease ();

        /* ��Ҫ�������²��� */
        MDM_FlowSwitchV24State ( MDM_FCM_V24_STATE_AT );

        wip_NetDeInit ();

////////////////////////////////////////////////////

        /* ֪ͨӦ�ó����쳣 */
        MDM_FlowServiceException ( ( u8 )MDM_FCM_EVENT_SWITCH_TO );
////////////////////////////////////////////////////

        /* ��������������л��ѳɹ�����δ�ܽ�����������Ӧ��Ϣ */
        //MDM_FlowSwitchV24State ( MDM_FCM_V24_STATE_DATA );      
      }
	  }    
  }
  
  /* ����״̬�� */
  else
  if ( mdmSwContext.isVliad == MDM_SW_STATE_IS_INTERFERE )
  {
    static u32 interfereCount = 0;

    interfereCount++;
    if ( interfereCount > 6 )
    {
      /* ���ⲿ���硢������źŸ��ŵ����
       * �л����ƹ��ܽ���ʱ6�룬
       * �ȴ�ϵͳ�ָ�����������ͨ��״̬ */
      interfereCount = 0;
  		mdmSwContext.isVliad = MDM_SW_STATE_IS_STOPPED;
  		mdmSwContext.result = TRUE;
    }
  }

  /* ���κζ���״̬�� */
  else
  if ( mdmSwContext.isVliad == MDM_SW_STATE_IS_DEAD )
  {
    static u32 swExceptionCount = 0;
#define MDM_SW_EXCEPTION_MAX_INETERVAL    120   /* ��λ��s */    

    /* �Ƿ��л�״̬?? */
    if ( MDM_QueryFlowStatus () == MDM_FCM_V24_STATE_SW )
    {
      /* �л����ƹ����Ѿ�ֹͣ����ģ���Դ����л��׶Σ�
       * �ڴ������Ϊ�л��쳣��
       * ֪ͨ�ϲ�Ӧ�ý��о��ߴ��� */
      swExceptionCount++;
      if ( swExceptionCount >= MDM_SW_EXCEPTION_MAX_INETERVAL )
      {
        swExceptionCount = 0;

////////////////////////////////////////////////////

        /* ֪ͨӦ�ó����쳣 */
        MDM_FlowServiceException ( ( u8 )MDM_FCM_EVENT_SWITCH_ERR );
////////////////////////////////////////////////////

      }
    }
    else
    {
      swExceptionCount = 0;
    }
  }
}






/* 
 * �����������л�����������
 * ���ò�������
 *          
 * ����ֵ  ��״̬��
 * 
 */
extern void MDM_SwitchStartWatch ( void )
{
  if ( MDM_QueryFlowStatus () == MDM_FCM_V24_STATE_SW )
  {
    mdmSwContext.isVliad = MDM_SW_STATE_IS_PROCESS;  
  }
}











/* 
 * �����������л���Դ����
 * ���ò�������
 *          
 * ����ֵ  ��״̬��
 * 
 */
static bool MDM_SwitchResourceConfig ( swMode_t mode )
{
  MDM_FlowSwitchV24State ( MDM_FCM_V24_STATE_SW );
  mdmSwContext.result = FALSE;
  mdmSwContext.switchTryCount = 0;

  if ( mode == __ONLINE__ )
  {
    /* �����л�ָ�� */
    if ( SWCTX_Command_Escape ( NULL ) == TRUE )
	  {
      mdmSwContext.dstMode = __ONLINE__;
//      mdmSwContext.isVliad = MDM_SW_STATE_IS_PROCESS;
    }
    else
    {
      /* ָ���ʧ�ܣ��˻�����״̬ */
      MDM_FlowSwitchV24State ( MDM_FCM_V24_STATE_DATA );
      return FALSE;  
    }
  }
  else
  if ( mode == __DATA__ )
  {
    /* �����л�ָ�� */	
    if ( SWCTX_Command_Return2DataState ( NULL ) == TRUE )
    {
      mdmSwContext.dstMode = __DATA__;
//      mdmSwContext.isVliad = MDM_SW_STATE_IS_PROCESS;
    }
    else
    {
      /* ָ���ʧ�ܣ��˻�AT״̬ */
      MDM_FlowSwitchV24State ( MDM_FCM_V24_STATE_AT );
      return FALSE;  
    }
	
  }
  else
  {
    return FALSE;
  }

  /* ע���л���ʱ����Դ */
  if ( mdmSwContext.switchTmr == NULL )
  {
    mdmSwContext.switchTmr = TMR_Subscribe ( TRUE, MDM_SWITCH_INETRVAL_MAX, TMR_TYPE_100MS, 
                                              ( tmr_procTriggerHdlr_t )MDM_SwitchCtxHdlr );
  }
  return TRUE;
}










/* 
 * �����������л���ATģʽ
 * ���ò�������
 *          
 * ����ֵ  ��״̬��
 * 
 */
extern bool MDM_SwitchToATStatus ( u8 switchType )
{
	mdmSwContext.switchType = switchType;

#if SWCTX_PROCESS_DEBUG == 1
{
  PRINTF_GPS_TIME ();
  printf ( "[SWC EVENT] -->at mode\r\n" );
}
#endif  /* SWCTX_PROCESS_DEBUG */
	
  /* ��ѯ�Ƿ�����״̬���� */
	if ( MDM_QueryFlowStatus () == MDM_FCM_V24_STATE_DATA )
	{

#if SWCTX_PROCESS_DEBUG == 1
{
    PRINTF_GPS_TIME ();
    printf ( "[SWC EVENT] start at-switch\r\n" );
}
#endif  /* SWCTX_PROCESS_DEBUG */

    return MDM_SwitchResourceConfig ( __ONLINE__ );
	}

  return FALSE;
}










/* 
 * �����������л�������ģʽ
 * ���ò�������
 *          
 * ����ֵ  ��״̬��
 * 
 */
extern bool MDM_SwitchToDataStatus ( void )
{

#if SWCTX_PROCESS_DEBUG == 1
{
  PRINTF_GPS_TIME ();
  printf ( "[SWC EVENT] -->data mode\r\n" );
}
#endif  /* SWCTX_PROCESS_DEBUG */

  if ( MDM_QueryFlowStatus () == MDM_FCM_V24_STATE_AT ) // && mdmSwContext.dstMode == __ONLINE__
	{

#if SWCTX_PROCESS_DEBUG == 1
{
    PRINTF_GPS_TIME ();
    printf ( "[SWC EVENT] start data-switch\r\n" );
}
#endif  /* SWCTX_PROCESS_DEBUG */

    return MDM_SwitchResourceConfig ( __DATA__ );
	} 

  return FALSE;
}









/* 
 * �����������л���Դ�ͷ�
 * ���ò�������
 *          
 * ����ֵ  ��״̬��
 * 
 */
extern void MDM_SwitchResourceRelease ( void )
{

#if SWCTX_PROCESS_DEBUG == 1
{
  PRINTF_GPS_TIME ();
  printf ( "[SWC EVENT] switch resource is released\r\n" );
}
#endif  /* SWCTX_PROCESS_DEBUG */

  /* ע���л���ʱ����Դ */
  mdmSwContext.isVliad = MDM_SW_STATE_IS_DEAD;
  mdmSwContext.result  = FALSE;
  mdmSwContext.switchTryCount = 0;
  mdmSwContext.dstMode = __DATA__;
  mdmSwContext.switchType = MDM_Switch_Type_Unknow;

}









/* 
 * �����������л���Դͳ��
 * ���ò�������
 *          
 * ����ֵ  ��״̬��
 * 
 */
extern void MDM_SwitchInfoStatistics ( void )
{
#if SWCTX_STATICS_DEBUG == 1
{
  printf ( "[SWC EVENT] at-switch %d ok, %d ko, %d to\r\n", mdmSwAtOKCount, mdmSwAtERRCount, mdmSwAtTOCount );
  printf ( "[SWC EVENT] data-switch %d ok, %d ko, %d to\r\n", mdmSwDataOKCount, mdmSwDataERRCount, mdmSwDataTOCount );
}
#endif  /* SWCTX_STATICS_DEBUG */
}



