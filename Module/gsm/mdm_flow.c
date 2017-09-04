



#include "modem.h"






typedef struct 
{
  /* ��ǰģʽ */
  u8 cur_mode;

  /* �����ƴ����� */
  mdm_fcmCtrlHdlr_f handler;
} MDM_FlowCfgTypedef;






/******************************************/
/*              �ڲ�����[����]            */
/******************************************/

static MDM_FlowCfgTypedef mdm_fcm = { MDM_FCM_V24_STATE_AT, NULL };







/**
 * @brief Initializes modem
 * @param modem pointer to modem to initialize
 */
extern void MDM_FlowServiceStart ( mdm_fcmCtrlHdlr_f CtrlHandler )
{
  if ( CtrlHandler )
  {
    /* ����״̬����ƴ����� */
    mdm_fcm.cur_mode = MDM_FCM_V24_STATE_AT;
    mdm_fcm.handler = CtrlHandler;

    /* �����ƴ� */
    if ( mdm_fcm.handler )
    {
      mdm_fcm.handler ( MDM_FCM_EVENT_FLOW_OPENNED );
    }
  } 
}







/**
 * @brief Initializes modem
 * @param modem pointer to modem to initialize
 */
extern void MDM_FlowSwitchV24State ( u8 State )
{
  /* ״̬ת�� */
  switch ( State )
  {
    /* AT״̬ */
    case MDM_FCM_V24_STATE_AT:
      mdm_fcm.cur_mode = MDM_FCM_V24_STATE_AT;
      if ( mdm_fcm.handler )
      {
        mdm_fcm.handler ( MDM_FCM_EVENT_V24_AT_MODE );
      }
      break;

    /* ����״̬ */
    case MDM_FCM_V24_STATE_DATA:
      mdm_fcm.cur_mode = MDM_FCM_V24_STATE_DATA;
      if ( mdm_fcm.handler )
      {
        mdm_fcm.handler ( MDM_FCM_EVENT_V24_DATA_MODE );
      }
      break;
    
    /* �л�״̬ */
    case MDM_FCM_V24_STATE_SW:
      if ( mdm_fcm.cur_mode == MDM_FCM_V24_STATE_DATA )
      {
        mdm_fcm.cur_mode = MDM_FCM_V24_STATE_SW;
        if ( mdm_fcm.handler )
        {
          /* ->ָ��״̬ */
          mdm_fcm.handler ( MDM_FCM_EVENT_V24_AT_MODE_FROM_CALL );
        }      
      }
      else 
      if ( mdm_fcm.cur_mode == MDM_FCM_V24_STATE_AT )
      {
        mdm_fcm.cur_mode = MDM_FCM_V24_STATE_SW;
        if ( mdm_fcm.handler )
        {
          /* ->����״̬ */
          mdm_fcm.handler ( MDM_FCM_EVENT_V24_DATA_MODE_FROM_CALL );
        }      
      }
      break;
  }
}









/**
 * @brief Initializes modem
 * @param modem pointer to modem to initialize
 */
extern void MDM_FlowServiceException ( u8 State )
{
  if ( mdm_fcm.handler )
  {
    switch ( State )
    {
      case MDM_FCM_EVENT_SWITCH_TO:   /* ���л���ʱ�¼� */
      case MDM_FCM_EVENT_SWITCH_ERR:  /* ���л�ʧ���¼� */
      case MDM_FCM_EVENT_DIAL_ERR:    /* ����ʧ���¼� */
        mdm_fcm.handler ( ( mdm_fcmEvent_e )State );
        break;

      default:
        break;
    }
  }  
}







/**
 * @brief Initializes modem
 * @param modem pointer to modem to initialize
 */
extern void MDM_FlowServiceStop ( void )
{
  /* ֹͣʹ���л���Դ */
  MDM_SwitchResourceRelease ();
  if ( mdm_fcm.handler )
  {
    mdm_fcm.handler ( MDM_FCM_EVENT_FLOW_CLOSED );
  }
  mdm_fcm.cur_mode = MDM_FCM_V24_STATE_AT;   
}






/**
 * @brief Initializes modem
 * @param modem pointer to modem to initialize
 */
extern u8 MDM_QueryFlowStatus ( void )
{
  return mdm_fcm.cur_mode;
}


