



#include "modem.h"






typedef struct 
{
  /* 当前模式 */
  u8 cur_mode;

  /* 流控制处理句柄 */
  mdm_fcmCtrlHdlr_f handler;
} MDM_FlowCfgTypedef;






/******************************************/
/*              内部变量[定义]            */
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
    /* 配置状态与控制处理器 */
    mdm_fcm.cur_mode = MDM_FCM_V24_STATE_AT;
    mdm_fcm.handler = CtrlHandler;

    /* 流控制打开 */
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
  /* 状态转换 */
  switch ( State )
  {
    /* AT状态 */
    case MDM_FCM_V24_STATE_AT:
      mdm_fcm.cur_mode = MDM_FCM_V24_STATE_AT;
      if ( mdm_fcm.handler )
      {
        mdm_fcm.handler ( MDM_FCM_EVENT_V24_AT_MODE );
      }
      break;

    /* 数据状态 */
    case MDM_FCM_V24_STATE_DATA:
      mdm_fcm.cur_mode = MDM_FCM_V24_STATE_DATA;
      if ( mdm_fcm.handler )
      {
        mdm_fcm.handler ( MDM_FCM_EVENT_V24_DATA_MODE );
      }
      break;
    
    /* 切换状态 */
    case MDM_FCM_V24_STATE_SW:
      if ( mdm_fcm.cur_mode == MDM_FCM_V24_STATE_DATA )
      {
        mdm_fcm.cur_mode = MDM_FCM_V24_STATE_SW;
        if ( mdm_fcm.handler )
        {
          /* ->指令状态 */
          mdm_fcm.handler ( MDM_FCM_EVENT_V24_AT_MODE_FROM_CALL );
        }      
      }
      else 
      if ( mdm_fcm.cur_mode == MDM_FCM_V24_STATE_AT )
      {
        mdm_fcm.cur_mode = MDM_FCM_V24_STATE_SW;
        if ( mdm_fcm.handler )
        {
          /* ->数据状态 */
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
      case MDM_FCM_EVENT_SWITCH_TO:   /* 流切换超时事件 */
      case MDM_FCM_EVENT_SWITCH_ERR:  /* 流切换失败事件 */
      case MDM_FCM_EVENT_DIAL_ERR:    /* 拨号失败事件 */
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
  /* 停止使用切换资源 */
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


