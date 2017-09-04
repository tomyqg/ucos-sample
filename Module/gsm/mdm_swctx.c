






//#include "general.h"
#include "defines.h"
#include "modem.h"
#include "modem_debug.h"








extern MDM_DevTypedef *modem;

////////////////////////////////////////////////////////////////

/* 切换统计调试 */
#define SWCTX_STATICS_DEBUG           1

////////////////////////////////////////////////////////////////


/* 切换间隔 */
#define MDM_SWITCH_INETRVAL_MAX			  10		/* 单位：100ms */

/* +++指令尝试次数 */
#define ESCAPE_COMMAND_TRY_COUNTER    3

/* ATO指令尝试次数 */
#define ATO_COMMAND_TRY_COUNTER       3

/* modem模式 */
typedef enum { __ONLINE__ = 0, __DATA__ } swMode_t;


typedef struct
{
  /* 模式 */
  swMode_t dstMode;

  /* 切换状态是否有效？ */
  u8 isVliad;
#define MDM_SW_STATE_IS_DEAD          0x00  /* 无任何工作 */
#define MDM_SW_STATE_IS_STOPPED       0x01  /* 执行停止切换 */
#define MDM_SW_STATE_IS_PROCESS       0x02  /* 执行切换处理 */
#define MDM_SW_STATE_IS_INTERFERE     0x03  /* 外部干扰，导致切换延时 */

  /* 定时器 */
  tmr_t *switchTmr;

  /* 尝试次数 */
  u8 switchTryCount;

  /* 切换原因 */
  u8 switchType;
  
  /* 切换是否成功？ */
  bool result;
} MDM_SwCtxTypedef;



#if SWCTX_PROCESS_DEBUG == 1
#include "tz_common.h"
#endif  /* SWCTX_PROCESS_DEBUG */






/******************************************/
/*              内部函数[声明]            */
/******************************************/

static AtCmdHandler_Status SWCTX_Handler_Escape           ( MDM_CmdTypedef *at, MDM_RspTypedef *response );
static AtCmdHandler_Status SWCTX_Handler_Return2DataState ( MDM_CmdTypedef *at, MDM_RspTypedef *response );
extern void Gl868_Command_Socket_Restore( void* ipc_data,const char connId );







/******************************************/
/*              内部变量[定义]            */
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

/* AT切换成功次数 */
static s32 mdmSwAtOKCount     = 0;

/* AT切换失败次数 */
static s32 mdmSwAtERRCount    = 0;

/* AT切换超时次数 */
static s32 mdmSwAtTOCount     = 0;

/* 数据切换成功次数 */
static s32 mdmSwDataOKCount   = 0;

/* 数据切换失败次数 */
static s32 mdmSwDataERRCount  = 0;

/* 数据切换超时次数 */
static s32 mdmSwDataTOCount   = 0;
#endif  /* SWCTX_STATICS_DEBUG */





/******************************************/
/*              内部函数[声明]            */
/******************************************/

static void MDM_SwitchCtxHdlr ( u8 ID );
static bool MDM_SwitchResourceConfig ( swMode_t mode );









/* 
 * 功能描述：发送+++指令
 * 引用参数：无
 *          
 * 返回值  ：状态码
 * 
 */
extern bool SWCTX_Command_Escape ( void* ipc_data )
{
  return MDM_PostAtCmdByID ( swctx_command_p, 
      	                     ESCAPE_COMMAND, 
      	                     NULL );
}







/* 
 * 功能描述：+++指令响应处理
 * 引用参数：无
 *          
 * 返回值  ：状态码
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
    
    /* 转换为AT状态 */
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

    /* 转换为AT状态 */
    MDM_FlowSwitchV24State ( MDM_FCM_V24_STATE_AT );
		mdmSwContext.isVliad = MDM_SW_STATE_IS_STOPPED;
		mdmSwContext.result = TRUE;
    return AT_HANDLER_DONE_ERROR;  	
  }

  return AT_HANDLER_DONT_UNDERSTAND;	
}








/* 
 * 功能描述：发送返回数据状态指令
 * 引用参数：无
 *          
 * 返回值  ：状态码
 * 
 */
extern bool SWCTX_Command_Return2DataState ( void* ipc_data )
{
  return MDM_PostAtCmdByID ( swctx_command_p, 
      	                     RETURN_TO_DATA_STATE, 
      	                     NULL );
}






/* 
 * 功能描述：返回数据状态指令响应处理
 * 引用参数：无
 *          
 * 返回值  ：状态码
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

    /* 转换为数据状态 */
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
    
    /* 切换失败，此时为AT通信状态 */
    mdmSwContext.isVliad = MDM_SW_STATE_IS_STOPPED;
    mdmSwContext.result  = FALSE;
    return AT_HANDLER_DONE_ERROR;			
  }
	
  return AT_HANDLER_DONT_UNDERSTAND;
}








extern void wip_NetDeInit ( void );




/* 
 * 功能描述：切换监视器
 * 引用参数：无
 *          
 * 返回值  ：状态码
 * 
 */
static void MDM_SwitchCtxHdlr ( u8 ID )
{


/*********************************************** 
 *                 切换过程结束                *
 ***********************************************/

  /* 切换停止状态下 */
  if ( mdmSwContext.isVliad == MDM_SW_STATE_IS_STOPPED )
	{

#if SWCTX_PROCESS_DEBUG == 1
{
    PRINTF_GPS_TIME ();
    printf ( "[SWC EVENT] modem switch is stop\r\n" );
}
#endif  /* SWCTX_PROCESS_DEBUG */

    /* 注销切换定时器资源 */
    mdmSwContext.isVliad = MDM_SW_STATE_IS_DEAD;
    mdmSwContext.switchTryCount = 0;


    /* 状态切换失败 */    
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

        /* 通知应用程序异常 */
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

        /* 需要考虑重新拨号 */
	
        //MDM_FlowSwitchV24State ( MDM_FCM_V24_STATE_AT );
        //wip_NetDeInit ();  
 
////////////////////////////////////////////////////

        /* 通知应用程序异常 */
        MDM_FlowServiceException ( ( u8 )MDM_FCM_EVENT_SWITCH_TO );
////////////////////////////////////////////////////
  	  }
    }
    else
    {

/****************************** 
 *   查询是否外部呼叫？       *
 *      Y ==> 直接退出        *
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
      /* 返回数据通信状态 */
      MDM_SwitchToDataStatus ();
    }
	}


/*********************************************** 
 *                 切换过程进行                *
 ***********************************************/

  /* 监控切换 */
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

      /* 数据模式下判断切换是否超时？ */
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

        /* 假设为数据模式的可能性较大 */
        MDM_FlowSwitchV24State ( MDM_FCM_V24_STATE_DATA );

        /* 注销切换资源 */
        MDM_SwitchResourceRelease ();

////////////////////////////////////////////////////

        /* 通知应用程序异常 */
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

      /* 命令模式下判断切换是否超时？ */
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
        
        /* 注销切换资源 */
        MDM_SwitchResourceRelease ();

        /* 需要考虑重新拨号 */
        MDM_FlowSwitchV24State ( MDM_FCM_V24_STATE_AT );

        wip_NetDeInit ();

////////////////////////////////////////////////////

        /* 通知应用程序异常 */
        MDM_FlowServiceException ( ( u8 )MDM_FCM_EVENT_SWITCH_TO );
////////////////////////////////////////////////////

        /* 处理情况描述：切换已成功，但未能接收完整的相应信息 */
        //MDM_FlowSwitchV24State ( MDM_FCM_V24_STATE_DATA );      
      }
	  }    
  }
  
  /* 干扰状态下 */
  else
  if ( mdmSwContext.isVliad == MDM_SW_STATE_IS_INTERFERE )
  {
    static u32 interfereCount = 0;

    interfereCount++;
    if ( interfereCount > 6 )
    {
      /* 受外部来电、传真等信号干扰的情况
       * 切换控制功能将延时6秒，
       * 等待系统恢复到正常数据通信状态 */
      interfereCount = 0;
  		mdmSwContext.isVliad = MDM_SW_STATE_IS_STOPPED;
  		mdmSwContext.result = TRUE;
    }
  }

  /* 无任何动作状态下 */
  else
  if ( mdmSwContext.isVliad == MDM_SW_STATE_IS_DEAD )
  {
    static u32 swExceptionCount = 0;
#define MDM_SW_EXCEPTION_MAX_INETERVAL    120   /* 单位：s */    

    /* 是否切换状态?? */
    if ( MDM_QueryFlowStatus () == MDM_FCM_V24_STATE_SW )
    {
      /* 切换控制功能已经停止，但模块仍处于切换阶段，
       * 在此情况下为切换异常。
       * 通知上层应用进行决策处理。 */
      swExceptionCount++;
      if ( swExceptionCount >= MDM_SW_EXCEPTION_MAX_INETERVAL )
      {
        swExceptionCount = 0;

////////////////////////////////////////////////////

        /* 通知应用程序异常 */
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
 * 功能描述：切换监视器启动
 * 引用参数：无
 *          
 * 返回值  ：状态码
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
 * 功能描述：切换资源配置
 * 引用参数：无
 *          
 * 返回值  ：状态码
 * 
 */
static bool MDM_SwitchResourceConfig ( swMode_t mode )
{
  MDM_FlowSwitchV24State ( MDM_FCM_V24_STATE_SW );
  mdmSwContext.result = FALSE;
  mdmSwContext.switchTryCount = 0;

  if ( mode == __ONLINE__ )
  {
    /* 发送切换指令 */
    if ( SWCTX_Command_Escape ( NULL ) == TRUE )
	  {
      mdmSwContext.dstMode = __ONLINE__;
//      mdmSwContext.isVliad = MDM_SW_STATE_IS_PROCESS;
    }
    else
    {
      /* 指令发送失败，退回数据状态 */
      MDM_FlowSwitchV24State ( MDM_FCM_V24_STATE_DATA );
      return FALSE;  
    }
  }
  else
  if ( mode == __DATA__ )
  {
    /* 发送切换指令 */	
    if ( SWCTX_Command_Return2DataState ( NULL ) == TRUE )
    {
      mdmSwContext.dstMode = __DATA__;
//      mdmSwContext.isVliad = MDM_SW_STATE_IS_PROCESS;
    }
    else
    {
      /* 指令发送失败，退回AT状态 */
      MDM_FlowSwitchV24State ( MDM_FCM_V24_STATE_AT );
      return FALSE;  
    }
	
  }
  else
  {
    return FALSE;
  }

  /* 注册切换定时器资源 */
  if ( mdmSwContext.switchTmr == NULL )
  {
    mdmSwContext.switchTmr = TMR_Subscribe ( TRUE, MDM_SWITCH_INETRVAL_MAX, TMR_TYPE_100MS, 
                                              ( tmr_procTriggerHdlr_t )MDM_SwitchCtxHdlr );
  }
  return TRUE;
}










/* 
 * 功能描述：切换到AT模式
 * 引用参数：无
 *          
 * 返回值  ：状态码
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
	
  /* 查询是否数据状态？？ */
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
 * 功能描述：切换到数据模式
 * 引用参数：无
 *          
 * 返回值  ：状态码
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
 * 功能描述：切换资源释放
 * 引用参数：无
 *          
 * 返回值  ：状态码
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

  /* 注销切换定时器资源 */
  mdmSwContext.isVliad = MDM_SW_STATE_IS_DEAD;
  mdmSwContext.result  = FALSE;
  mdmSwContext.switchTryCount = 0;
  mdmSwContext.dstMode = __DATA__;
  mdmSwContext.switchType = MDM_Switch_Type_Unknow;

}









/* 
 * 功能描述：切换资源统计
 * 引用参数：无
 *          
 * 返回值  ：状态码
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



