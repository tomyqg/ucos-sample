








#include <stdlib.h>
#include <stdio.h>
#include <string.h>

//#include "general.h"

#include "modem.h"
#include "modem_debug.h"
//#include "tz_common.h"
#include "defines.h"




/* 定义MODEM访问接口 */
MDM_DevTypedef *modem = NULL;





/******************************************/
/*                MODEM[配置]             */
/******************************************/

/* 短指令队列长度 */
#define MDM_SCMD_LIST_LENGTH        14

/* 短指令长度 */
#define MDM_SCMD_BLK_SIZE           64    /* 单位：byte */

/* 短指令缓存大小 */
#define MDM_TX_SBUFF_SIZE		        ( MDM_SCMD_LIST_LENGTH * MDM_SCMD_BLK_SIZE ) 

/* 长指令队列长度 */
#define MDM_LCMD_LIST_LENGTH        2

/* 长指令长度 */
#define MDM_LCMD_BLK_SIZE           200   /* 单位：byte */

/* 长指令缓存大小 */
#define MDM_TX_XBUFF_SIZE		        ( MDM_LCMD_LIST_LENGTH * MDM_LCMD_BLK_SIZE )

/* 临时指令收发缓冲区大小 */
#define MDM_RX_BUFF_SIZE_MAX		    512   /* 单位：byte */
#define MDM_TX_BUFF_SIZE_MAX		    200   /* 单位：byte */




typedef struct
{
  /* 计数器 */
  u8 shortCmdCounter;

  /* 计数器 */
  u8 longCmdCounter;

  /* 命令编号 */
  u32 shortCmdNumber;

  /* 命令编号 */
  u32 longCmdNumber;

  struct  /* 命令队列结构 */ 
  {
    /* 命令发送索引位置 */
    u32 CmdIndex;

    /* AT命令信息 */
    MDM_CmdTypedef At;
  } ShortCmd[ MDM_SCMD_LIST_LENGTH ], LongCmd[ MDM_LCMD_LIST_LENGTH ];
} MDM_CmdListCfgTypedef;





/******************************************/
/*              内部函数[声明]            */
/******************************************/

extern void MDM_ConfigureCurrentCall ( Call *call );

extern AtCmdHandler_Status MDM_Handler_UnsRsp               ( MDM_CmdTypedef *at, MDM_RspTypedef *atrsp );
extern AtCmdHandler_Status MDM_Utils_Handler                ( MDM_CmdTypedef *at, MDM_RspTypedef *atrsp );
static AtCmdHandler_Status MDM_Handler_Alive                ( MDM_CmdTypedef *at, MDM_RspTypedef *atrsp );
static AtCmdHandler_Status MDM_Handler_SetSerialPortRate    ( MDM_CmdTypedef *at, MDM_RspTypedef *atrsp );
static AtCmdHandler_Status MDM_Handler_ReadSerialPortRate   ( MDM_CmdTypedef *at, MDM_RspTypedef *atrsp );
extern void lgsm_network_query_signal_strength ( void* ipc_data );
extern void NETWORK_Command_QueryNetworkRegistration ( void* ipc_data );


extern bool CheckAppStackOverFlow(void);
extern bool CheckUsartStackOverFlow(void);
extern bool CheckTMRFastStackOverFlow(void);
extern bool CheckTMRSlowStackOverFlow(void);
extern bool CheckWorkStackOverFlow(void);
extern bool CheckRtcStackOverFlow(void);





/******************************************/
/*             MODEM定时器[配置]          */
/******************************************/

/* 超时监视定时器 */
static tmr_t *tmrMdmMonitor = NULL;

/* 循环发送定时器 */
static tmr_t *tmrMdmSendblk = NULL;









/******************************************/
/*                MODEM[配置]             */
/******************************************/

/* 定义MODEM工作对象 */
static worker_t mdmWorker;

/* 临时指令收发缓冲区定义 */
static ascii mdm_cmd_sbuffer[ MDM_TX_SBUFF_SIZE ];
static ascii mdm_cmd_xbuffer[ MDM_TX_XBUFF_SIZE ];

/* 待处理的AT响应 */
static ascii mdm_rx_buffer[ MDM_RX_BUFF_SIZE_MAX ];
static MDM_RspTypedef mdm_glb_rsp = { FALSE, MDM_NO_STRING, 0, mdm_rx_buffer };

/* 正在处理的AT指令 */
static ascii mdm_tx_buffer[ MDM_TX_BUFF_SIZE_MAX ];
static MDM_CmdTypedef mdm_glb_cmd = { 0, mdm_tx_buffer, 0, 0, NULL, SIM_UNKNOWN, FALSE };

/* 排队待发送AT命令 */
static MDM_CmdListCfgTypedef ready_cmd_list;


#define SHORT_CMD_COUNTER                 ( ready_cmd_list.shortCmdCounter )	//-待发送命令个数
#define SHORT_CMD_NUMBER                  ( ready_cmd_list.shortCmdNumber )		//-命令序列号,起标记作用
#define SHORT_CMD_INDEX( i )              ( ready_cmd_list.ShortCmd[ (i) ].CmdIndex )	//-当前成员序列号
#define SHORT_CMD( i )                    ( ready_cmd_list.ShortCmd[ (i) ].At )				//-当前命令的完整信息
#define SHORT_CMD_ID( i )                 ( ready_cmd_list.ShortCmd[ (i) ].At.cmd_id )	//-下面通过宏直接指向特定内容
#define SHORT_CMD_STATE( i )              ( ready_cmd_list.ShortCmd[ (i) ].At.required_state )
#define SHORT_CMD_AT_BUFFER( i )          ( ready_cmd_list.ShortCmd[ (i) ].At.AtBuffer )

#define LONG_CMD_COUNTER                  ( ready_cmd_list.longCmdCounter )
#define LONG_CMD_NUMBER                   ( ready_cmd_list.longCmdNumber )
#define LONG_CMD_INDEX( i )               ( ready_cmd_list.LongCmd[ (i) ].CmdIndex )
#define LONG_CMD( i )                     ( ready_cmd_list.LongCmd[ (i) ].At )
#define LONG_CMD_ID( i )                  ( ready_cmd_list.LongCmd[ (i) ].At.cmd_id )
#define LONG_CMD_STATE( i )               ( ready_cmd_list.LongCmd[ (i) ].At.required_state )
#define LONG_CMD_AT_BUFFER( i )           ( ready_cmd_list.LongCmd[ (i) ].At.AtBuffer )








static MDM_CmdTypedef commands[] 
= {
  /* Disable command echos */
  { ENABLE_ECHO, "ATE1\r\n", 
  	100, 2, 
  	MDM_Utils_Handler, SIM_UNKNOWN, FALSE },

  /* Escape command */
  { ESCAPE_COMMAND, "+++", 
  	150, 1,
  	MDM_Utils_Handler, SIM_UNKNOWN, FALSE },

  /* query Local Terminal Serial Port Rate */
  { READ_SERIAL_PORT_RATE, "AT+IPR?\r\n", 
  	400, 2, 
  	MDM_Handler_ReadSerialPortRate, SIM_UNKNOWN, FALSE },

  /* set Local Terminal Serial Port Rate */
  { SET_SERIAL_PORT_RATE, "AT+IPR=%s\r\n", 
  	200, 1, 
  	MDM_Handler_SetSerialPortRate, SIM_UNKNOWN, FALSE },

  /* Enable everything */
  { POWER_ON, "AT+CFUN=1\r\n", 
  	1500, 0, 
  	MDM_Utils_Handler, SIM_UNKNOWN, FALSE },

  /* RTS/CTS Flow Control */
  { SOFTWARE_FOLW_CONTROL_ON, "AT&K4\r\n", 
  	1500, 2, 
  	MDM_Utils_Handler, SIM_UNKNOWN, FALSE },
      	
  /* Set response of AT+COPS? to respond with operator name */
  { SET_OP_QUERY_REPLY, "AT+COPS=3,0\r\n", 
  	100, 0,
    MDM_Utils_Handler, SIM_READY, FALSE },
    
  /* Enable Report Mobile Equipment Error */
  { ENABLE_CMEE, "AT+CMEE=1\r\n", 
  	100, 1,
  	MDM_Utils_Handler, SIM_UNKNOWN, FALSE },
  	
  /* Enable call waiting */
  { ENABLE_CCWA, "AT+CCWA=1,1\r\n", 
  	100, 0,
    MDM_Utils_Handler, SIM_READY, FALSE },
    
  /* Enable call query */
  { CUR_CALLS_QUERY, "AT+CLCC\r\n", 
  	100, 0,
    MDM_Utils_Handler, SIM_READY, FALSE },

  /* Query if we are muted */
  { MUTE_QUERY, "AT+CMUTE=?\r\n", 
  	500, 0,
    MDM_Utils_Handler, SIM_READY, FALSE },
    
  /* Enable caller identification */
  { ENABLE_CLIP, "AT+CLIP=1\r\n", 
  	1800, 3,
    MDM_Utils_Handler, SIM_READY, FALSE },
    
  /* Enable new message identification */
  { ENABLE_CNMI, "AT+CNMI=2,1\r\n", 
  	4500, 4,
    MDM_Utils_Handler, SIM_READY, FALSE },

#if 0
  /* Control a Status LED */
  { USING_SYNC_PIN, "AT^SSYNC=1\r\n", 
  	100, 2, 
  	MDM_Utils_Handler, SIM_UNKNOWN, FALSE },
#endif

  /* Circuit 108 Behavior */
  { DTR_SET, "AT&D2\r\n", 
  	400, 3,
  	MDM_Utils_Handler, SIM_UNKNOWN, FALSE },
  	
  /*Number Of Rings To Auto Answer*/
  { AUTO_ANSWER, "ATS0=%s\r\n", 
	 600, 3,
	 MDM_Utils_Handler, SIM_UNKNOWN, FALSE },

  /*Command Echo*/
  { COMMAND_ECHO, "ATE0V1\r\n", 
	 600, 3,
	 MDM_Utils_Handler, SIM_UNKNOWN, FALSE },
  	
  /* Just send at command */
  { ATT, "AT\r\n", 
  	500, 5,
  	MDM_Utils_Handler, SIM_UNKNOWN, FALSE },
  	
  /* Test modem is alive */
  { REPLY_COMMAND, "AT\r\n", 
  	500, 5,
    MDM_Handler_Alive, SIM_UNKNOWN, FALSE },

  /* Set TXT mode */
  { SET_TXT_MODE, "AT+CMGF=1\r\n", 
  	100, 0,
  	MDM_Utils_Handler, SIM_UNKNOWN, FALSE },
      	
  /* Test modem is alive */
  { SOFT_RESET, "ATZ\r\n", 
  	500, 3,
    MDM_Handler_Alive, SIM_UNKNOWN, FALSE },

  { 0, NULL, 0, 0, NULL, SIM_UNKNOWN, FALSE }
}, *command_p = commands;





extern SymbolTable symbols[] 
= {
  { "OK",         MDM_OK        },
  { "ERROR",      MDM_ERROR     },
  { "RING",       MDM_RING      },
  { "NO ANSWER",  MDM_NO_ANSWER },
  { "NO CARRIER", MDM_NO_CARRIER},
  { "BUSY",       MDM_BUSY      },
  { "READY",      MDM_READY     },
  { "CME ERROR",  MDM_CME_ERROR },
  { "CMS ERROR",  MDM_CMS_ERROR },
  { "IPR",        MDM_IPR       },
  { "CPIN",       MDM_CPIN      },
  { "CPAS",       MDM_CPAS      },
  { "CSQ",        MDM_CSQ       },
  { "CMUT",       MDM_CMUT      },
  { "CLIP",       MDM_CLIP      },
  { "CCWA",       MDM_CCWA      },
  { "CRING",      MDM_CRING     },
  { "CMTI",       MDM_CMTI      },
  { "CRSL",       MDM_CRSL      },
  { "CMGW",       MDM_CMGW      },
  { "PIN",        MDM_PIN       },
  { "PUK",        MDM_PUK       },
  { "COPS",       MDM_COPS      },
  { "CCLK",       MDM_CCLK      },
  { "CFUN",       MDM_CFUN      },
  { "CGSN",       MDM_CGSN      },
  { "CGMR",       MDM_CGMR      },
   // { "CCID",       MDM_CCID      },SCID
  //{ "SCID",       MDM_CCID      },
	{ "ICCID",       MDM_CCID      },
  { "CGMM",       MDM_CGMM      },
  { "CGMI",       MDM_CGMI      },
  { "CREG",       MDM_CREG      },
  { "CGREG",      MDM_CGREG     },
  { "SIM",        MDM_SIM       },
  { "CNUM",       MDM_CNUM      },
  { "CIMI",       MDM_CIMI      },
  { "CSCA",       MDM_CSCA      },
  { "CPBR",       MDM_CPBR      },
  { "CMGR",       MDM_CMGR      },
  { "CMGR",       MDM_CMGR      },
  { "CMGL",       MDM_CMGL      },
  { "CMGW",       MDM_CMGW      },
  { "CGCLASS",    MDM_CGCLASS   },
  { "CGACT",      MDM_CGACT     },
  { "CGDCONT",    MDM_CGDCONT   },           
  { "CGATT",      MDM_CGATT     },
  { NULL, 0                     }
}, *symbol_p = symbols;










/******************************************/
/*              辅助工具[声明]            */
/******************************************/

extern bool MDM_CmdCpy ( MDM_CmdTypedef *DstCmd, MDM_CmdTypedef *SrcCmd );
extern bool MDM_CmdSet ( MDM_CmdTypedef *SrcCmd );
extern bool MDM_StrIsTerminalResponse ( MDM_RSP_ID RspID );
extern MDM_RSP_ID MDM_StrGetID ( MDM_RspTypedef *response );
extern MDM_RSP_ID MDM_GetResultCode ( MDM_RspTypedef *response );








/******************************************/
/*              内部函数[声明]            */
/******************************************/

static void MDM_ClearAtCmdList ( void );
static void MDM_ClearGlobalAtCmd ( void );
static void MDM_ClearGlobalAtRsp ( void );
static void MDM_GeneralAtInit ( void );
static void MDM_GeneralAtInitWithSIMReady ( void );
static void MDM_RetryCurrentCommand ( void );
static void MDM_WaitForCmdRsp ( u32 Timeout );
static void MDM_HandleIncomingCall ( MDM_RspTypedef *response );
static void MDM_HandleNetworkStatus( MDM_RspTypedef *response );
static void MDM_HandleCallWaiting  ( MDM_RspTypedef *response );
static void MDM_HandleRing ( void );
static void MDM_PollingCmdList ( u8 ID );
static void MDM_WorkerExecTask ( void* data );








/**
 * @brief Initializes modem
 * @param modem pointer to modem to initialize
 */
extern void MDM_IfInit ( MDM_DevTypedef *MODEM )
{
  if ( MODEM )
  {

#if MDM_INFO_DEBUG == 1
    printf ( "\r\n[MDM STATE] uinitialized\r\n" );
#endif  /* MDM_INFO_DEBUG */

    modem = MODEM;
    mdmWorker.data = NULL;
    mdmWorker.func = MDM_WorkerExecTask;	//-被写入工作队列一次,就运行一次

    MDM_ClearGlobalAtCmd ();	//-当前AT指令
    MDM_ClearAtCmdList ();
    MDM_ClearGlobalAtRsp ();

    if ( tmrMdmSendblk == NULL )
    {
      /* 注册发送定时器 */
      tmrMdmSendblk = TMR_Subscribe ( TRUE, 2, TMR_TYPE_100MS, ( tmr_procTriggerHdlr_t )MDM_PollingCmdList );  
    }
  }  
}










/* 
 * 功能描述: 清空当前AT指令
 * 引用参数: 无
 *          
 * 返回值  : 无
 * 
 */
static void MDM_ClearAtCmdList ( void )	//-AT指令列表
{
  u32 i = 0;

  ready_cmd_list.shortCmdCounter = 0;
  ready_cmd_list.shortCmdNumber = 0;
  for ( i = 0; i < MDM_SCMD_LIST_LENGTH; i++ )
  {
    ready_cmd_list.ShortCmd[ i ].CmdIndex = 0;
    ready_cmd_list.ShortCmd[ i ].At.cmd_id = 0;        
    ready_cmd_list.ShortCmd[ i ].At.timeout = 0;
    ready_cmd_list.ShortCmd[ i ].At.retry_counter = 0;
    ready_cmd_list.ShortCmd[ i ].At.handler = NULL;
    ready_cmd_list.ShortCmd[ i ].At.required_state = SIM_UNKNOWN;
    ready_cmd_list.ShortCmd[ i ].At.write_complete = FALSE;
    ready_cmd_list.ShortCmd[ i ].At.AtBuffer = ( ascii* )&mdm_cmd_sbuffer[ i * MDM_SCMD_BLK_SIZE ]; 

#if MDM_INFO_DEBUG == 1 && MDM_INIT_DEBUG == 1
{
    printf ( "\r\n[MDM EVENT] At cmd list address = %X\r\n", 
             ready_cmd_list.ShortCmd[ i ].At.AtBuffer );
}
#endif  /* MDM_INFO_DEBUG */
  }

  ready_cmd_list.longCmdCounter = 0;
  ready_cmd_list.longCmdNumber = 0;
  for ( i = 0; i < MDM_LCMD_LIST_LENGTH; i++ )
  {
    ready_cmd_list.LongCmd[ i ].CmdIndex = 0;
    ready_cmd_list.LongCmd[ i ].At.cmd_id = 0;        
    ready_cmd_list.LongCmd[ i ].At.timeout = 0;
    ready_cmd_list.LongCmd[ i ].At.retry_counter = 0;
    ready_cmd_list.LongCmd[ i ].At.handler = NULL;
    ready_cmd_list.LongCmd[ i ].At.required_state = SIM_UNKNOWN;
    ready_cmd_list.LongCmd[ i ].At.write_complete = FALSE;
    ready_cmd_list.LongCmd[ i ].At.AtBuffer = ( ascii* )&mdm_cmd_xbuffer[ i * MDM_LCMD_BLK_SIZE ]; 

#if MDM_INFO_DEBUG == 1 && MDM_INIT_DEBUG == 1
{
    printf ( "\r\n[MDM EVENT] At cmd list address = %X\r\n", 
             ready_cmd_list.LongCmd[ i ].At.AtBuffer );
}
#endif  /* MDM_INFO_DEBUG */
  }
}











/* 
 * 功能描述: 清空当前AT指令
 * 引用参数: 无
 *          
 * 返回值  : 无
 * 
 */
static void MDM_ClearGlobalAtCmd ( void )
{

#if MDM_INFO_DEBUG == 1 && MDM_INIT_DEBUG == 1
{
  printf ( "\r\n[MDM EVENT] MDM_ClearGlobalAtCmd trigger\r\n" );
}
#endif  /* MDM_INFO_DEBUG */

  mdm_glb_cmd.cmd_id  = 0;
  mdm_glb_cmd.timeout = 0;
  mdm_glb_cmd.retry_counter = 0;
  mdm_glb_cmd.handler = NULL;
  mdm_glb_cmd.required_state = SIM_UNKNOWN;
  mdm_glb_cmd.write_complete = FALSE;

  if ( mdm_glb_cmd.AtBuffer )
  {
    memset ( ( u8* )&mdm_glb_cmd.AtBuffer[ 0 ], 0, MDM_TX_BUFF_SIZE_MAX );
  }
}









/* 
 * 功能描述: 清空当前AT响应
 * 引用参数: 无
 *          
 * 返回值  : 无
 * 
 */
static void MDM_ClearGlobalAtRsp ( void )	//-清空当前AT响应
{

#if MDM_INFO_DEBUG == 1 && MDM_INIT_DEBUG == 1
{
  printf ( "\r\n[MDM EVENT] MDM_ClearGlobalAtRsp trigger\r\n" );
}
#endif  /* MDM_INFO_DEBUG */

  mdm_glb_rsp.finish = FALSE;
	mdm_glb_rsp.Code = MDM_NO_STRING;
  mdm_glb_rsp.totalsize = 0;

  if ( mdm_glb_rsp.indicator )
  {
    memset ( ( u8* )&mdm_glb_rsp.indicator[ 0 ], 0, MDM_RX_BUFF_SIZE_MAX );
  }
}

extern void MDM_At_Command_Echo( void );
extern void GL868_PDP_Command_AddContext ( void* ipc_data, int index, const char *APN );
extern void SIM_Command_RingIndicator ( void* ipc_data, s32 time );
extern void GL868_Command_UseSyncLED ( void* ipc_data, u8 mode, u8 on_duration, u8 off_duration );
extern void GL868_Get_Version(void* ipc_data);
extern void GL868_GET_ICCID(void* ipc_data);
/**
 * @brief Sends initializing at commands to the modem
 * @param modem modem to initialize
 */

 static void MDM_GeneralAtInit ( void ) //gulin
{
	
  MDM_PostAtCmdByID ( command_p, READ_SERIAL_PORT_RATE,       NULL ); 
  //GL868_Command_UseSyncLED(NULL, 3, 10, 10);/*默认1s 闪烁*/
  MDM_PostAtCmdByID ( command_p, ENABLE_ECHO,                NULL );
  MDM_PostAtCmdByID ( command_p, ENABLE_CMEE,                 NULL );
	GL868_GET_ICCID(NULL);
	lgsm_query_imei( NULL );
	GSM_Call_handler_SetCallRing(NULL);
	GL868_Get_Version(NULL);
	lgsm_sim_get_auth_status(NULL);		//CPIN查询
	
}

 
extern void SIM_Command_GetSIMCNUM ( void* ipc_data,ascii *pcnum );
extern void Gl868_Select_Flow_control( void* ipc_data, u8 type);
extern void GL868_PDP_Command_AddContext ( void* ipc_data, int index, const char *APN );


/**
 * @brief Sends initializing at commands that need sim_status >=
 * SIM_READY to the modem
 * @param modem modem to initialize
 */
static void MDM_GeneralAtInitWithSIMReady ( void )
{
  lgsm_network_register_network ( NULL );
  //lgsm_sim_select_phone_book_memory ( NULL, "ON" ); //

  //MDM_PostAtCmdByID ( command_p, ENABLE_CNMI,        NULL );
  //MDM_PostAtCmdByID ( command_p, ENABLE_CLIP,        NULL );
  //MDM_PostAtCmdByID ( command_p, SET_TXT_MODE,       NULL );                                  
  //MDM_PostAtCmdByID ( command_p, SET_OP_QUERY_REPLY, NULL );
  //MDM_PostAtCmdByID ( command_p, ENABLE_CCWA,        NULL );
   //lgsm_sim_delete_all_messages ( NULL );
	
	 //lgsm_pdp_attach_gprs_network ( NULL );
   lgsm_network_query_signal_strength(NULL);
   NETWORK_Command_QueryNetworkRegistration ( NULL );


}








/**
 * @brief Re-excute current AT command at the tail of command queue
 * @param modem Modem device struct pointer
 * @param interface interface whose current command to retry
 */
static void MDM_RetryCurrentCommand ( void )
{
  if ( modem )
  {
    modem->is_timeout = FALSE;
    
    if ( mdm_glb_cmd.retry_counter > 0 )
    {
      mdm_glb_cmd.retry_counter--;

#if MDM_INFO_DEBUG == 1 && MDM_MONITOR_DEBUG == 1
{
      printf ( "\r\n[MDM EVENT] Current cmd %s, retry counter %d\r\n", 
               mdm_glb_cmd.AtBuffer, 
               mdm_glb_cmd.retry_counter );
}
#endif  /* MDM_INFO_DEBUG */

    	if ( modem->serial && modem->serial->write_data && mdm_glb_cmd.AtBuffer )
    	{
        s32 length = strlen ( ( const char* )mdm_glb_cmd.AtBuffer );
        if ( length > 0 && modem->serial->write_data ( AT_Flow_Type, mdm_glb_cmd.AtBuffer, length ) == TRUE )
        {
      		mdm_glb_cmd.write_complete = TRUE;
          mdm_glb_rsp.finish = FALSE;
        }
    	}   
    }  
  }
}






/* 
 * 功能描述: 执行发送任务
 * 引用参数: 任务数据
 *          
 * 返回值  : 无
 * 
 */
static void MDM_WorkerExecTask ( void* data )	//-实现AT指令的发送
{
  if ( modem->status < MDM_IS_BUSY && modem->status >= MDM_SERIAL_INIT )
  {
    bool found = FALSE;
    u32 sendCmdIndex = 0;
    u16 i = 0;
    u16 tempCmdLoc = 0;

    /* BUG #003 修正0号索引不能发射的错误!! */
    if ( SHORT_CMD_COUNTER > 0 )	//-大于0说明有数据待发送
    {
      for ( ( i = 0, sendCmdIndex = 0 ); i < MDM_SCMD_LIST_LENGTH; i++ )
      {
        if ( SHORT_CMD_ID( i ) && ( SHORT_CMD_STATE( i ) <= modem->sim_status ) )
        {   

#if MDM_INFO_DEBUG == 1 && MDM_AT_SEND_DEBUG == 9
{
        printf ( "\r\n[MDM EVENT] get idx %u, ser %u\r\n", 
                 i, SHORT_CMD_INDEX( i ) );
}
#endif  /* MDM_INFO_DEBUG */ 

          if ( sendCmdIndex > SHORT_CMD_INDEX( i ) ) 
          {
            /* MOD #002 去除sendCmdIndex与0的条件比较 */
            sendCmdIndex = SHORT_CMD_INDEX( i );
            tempCmdLoc = i;
          }
          else
          if ( found == FALSE )
          {
            /* MOD #003 初始化赋值操作 */
            sendCmdIndex = SHORT_CMD_INDEX( i );
            tempCmdLoc = i;  
          }
#if 0 
          else
          if ( sendCmdIndex == 0 && i == 0 )
          {
            sendCmdIndex = SHORT_CMD_INDEX( i );
            tempCmdLoc = i;
          }
#endif
          found = TRUE;    
        }
      }
      
      if ( found == TRUE )
      {
       
#if MDM_INFO_DEBUG == 1 && MDM_AT_SEND_DEBUG == 1
{
        printf ( "\r\n[MDM EVENT] read short cmds left %u, send idx %u, ser %u\r\n", 
                 SHORT_CMD_COUNTER, 
                 tempCmdLoc, 
                 SHORT_CMD_INDEX( tempCmdLoc ) );
}
#endif  /* MDM_INFO_DEBUG */            

        MDM_ClearGlobalAtCmd ();
        MDM_ClearGlobalAtRsp ();     
        MDM_CmdCpy ( ( MDM_CmdTypedef * )&mdm_glb_cmd, ( MDM_CmdTypedef * )&SHORT_CMD ( tempCmdLoc ) );
  
      	if ( modem->serial && modem->serial->write_data && mdm_glb_cmd.AtBuffer )
        {
          u32 length = strlen ( ( const char* )mdm_glb_cmd.AtBuffer );

          if ( length > 0 && modem->serial->write_data ( AT_Flow_Type, mdm_glb_cmd.AtBuffer, length ) == TRUE )
          {//-上面完成了向串口写数据
            /* NEW #001 增加切换监视 */
            if ( SHORT_CMD_ID( tempCmdLoc ) == ESCAPE_COMMAND 
              || SHORT_CMD_ID( tempCmdLoc ) == RETURN_TO_DATA_STATE )
            {
              /* 通知监视单元启动切换过程监控 */
              MDM_SwitchStartWatch ();                  
            }


            SHORT_CMD_INDEX ( tempCmdLoc ) = 0;	//-发送完成了清0
            MDM_CmdSet ( ( MDM_CmdTypedef * )&SHORT_CMD ( tempCmdLoc ) );
            if ( SHORT_CMD_COUNTER > 0 )
            {
              SHORT_CMD_COUNTER--;
            }

#if MDM_INFO_DEBUG == 1
{
            printf ( "\r\n[MDM STATE] is busy\r\n" );
}
#endif  /* MDM_INFO_DEBUG */

            modem->status = MDM_IS_BUSY;	//-说明处于等待应答状态
            mdm_glb_cmd.write_complete = TRUE;	//-说明发送完成了
            MDM_WaitForCmdRsp ( mdm_glb_cmd.timeout ); 
          }

#if MDM_INFO_DEBUG == 8
          else
          {
            printf ( "\r\n[MDM EVENT] fail to write %ubytes\r\n", length );
          }
#endif  /* MDM_INFO_DEBUG */
          return ;
        }
      }          
    }


    /* BUG #004 修正0号索引不能发射的错误!! */
    if ( LONG_CMD_COUNTER > 0 )
    {
      for ( ( i = 0, sendCmdIndex = 0 ); i < MDM_LCMD_LIST_LENGTH; i++ )
      {
        if ( LONG_CMD_ID( i ) && ( LONG_CMD_STATE( i ) <= modem->sim_status ) )
        {   
          if ( sendCmdIndex > LONG_CMD_INDEX( i ) ) 
          {
            sendCmdIndex = LONG_CMD_INDEX( i );
            tempCmdLoc = i;
          }
          else
          if ( found == FALSE )
          {
            /* MOD #004 初始化赋值操作 */
            sendCmdIndex = LONG_CMD_INDEX( i );
            tempCmdLoc = i;  
          }
#if 0
          else
          if ( sendCmdIndex == 0 )
          {
            sendCmdIndex = LONG_CMD_INDEX( i ); 
            tempCmdLoc = i;
          }
#endif
          found = TRUE;    
        }
      }
      
      if ( found == TRUE )
      {

#if MDM_INFO_DEBUG == 1 && MDM_AT_SEND_DEBUG == 1
{
        printf ( "\r\n[MDM EVENT] read long cmds left %u, send index %u, ser %u\r\n", 
                 LONG_CMD_COUNTER, 
                 tempCmdLoc, 
                 LONG_CMD_INDEX( tempCmdLoc ) );
}
#endif  /* MDM_INFO_DEBUG */

        MDM_ClearGlobalAtCmd ();
        MDM_ClearGlobalAtRsp ();       
        MDM_CmdCpy ( ( MDM_CmdTypedef * )&mdm_glb_cmd, ( MDM_CmdTypedef * )&LONG_CMD ( tempCmdLoc ) );
  
      	if ( modem->serial && modem->serial->write_data && mdm_glb_cmd.AtBuffer )
        {
          u32 length = strlen ( ( const char* )mdm_glb_cmd.AtBuffer );

          if ( length > 0 && modem->serial->write_data ( AT_Flow_Type, mdm_glb_cmd.AtBuffer, length ) == TRUE )
          {
            /* NEW #002 增加切换监视 */
            if ( LONG_CMD_ID( tempCmdLoc ) == ESCAPE_COMMAND 
              || LONG_CMD_ID( tempCmdLoc ) == RETURN_TO_DATA_STATE )
            {
              /* 通知监视单元启动切换过程监控 */
              MDM_SwitchStartWatch ();                  
            }

            LONG_CMD_INDEX ( tempCmdLoc ) = 0;
            MDM_CmdSet ( ( MDM_CmdTypedef * )&LONG_CMD ( tempCmdLoc ) );
            if ( LONG_CMD_COUNTER > 0 )
            {
              LONG_CMD_COUNTER--;
            }

#if MDM_INFO_DEBUG == 1
{
            printf ( "\r\n[MDM STATE] is busy\r\n" );
}
#endif  /* MDM_INFO_DEBUG */

            modem->status = MDM_IS_BUSY;
            mdm_glb_cmd.write_complete = TRUE;
            MDM_WaitForCmdRsp ( mdm_glb_cmd.timeout );
          }
        }
      }          
    }
  }
}







/* 
 * 功能描述: 循环发送AT指令
 * 引用参数: 定时器ID
 *          
 * 返回值  : 无
 * 
 */
static void MDM_PollingCmdList ( u8 ID )
{
  if ( modem )
  {
    switch ( MDM_QueryFlowStatus () )
    {
      case MDM_FCM_V24_STATE_AT:  /* AT模式 */ 
      case MDM_FCM_V24_STATE_SW:  /* 切换模式 */
        if ( SHORT_CMD_COUNTER || LONG_CMD_COUNTER )
        {
          /* 待发送AT队列非空，故执行任务 */
          WORKER_StartupWork ( ( worker_t * )&mdmWorker );
        }
        break;
  
      case MDM_FCM_V24_STATE_DATA:
        break;
    }
  }
}











/**
 * @brief Sends initializing at commands to the modem
 * @param modem modem to initialize
 */
extern bool MDM_PostAtCmdByID ( MDM_CmdTypedef *command, AtCommandID id, const ascii* params )
{
  if ( command &&( id >= PIN_QUERY && id <= VENDOR_COMMAND )&& modem )
  {
    while ( command->cmd_id > 0 )
    {
      if ( command->cmd_id != id )
      {//-通过比较ID号找到对应的指令
        command++;
      }
      else
      {
        u32 i = 0;


/****************************** 
 *   查询是否数据模式？       *
 *      Y ==> 切换状态        *
 ******************************/
#if 1
        if ( MDM_QueryFlowStatus () == MDM_FCM_V24_STATE_DATA )
        {
          MDM_SwitchToATStatus ( MDM_Switch_Type_Active );
        }
#endif

#if 1   /* MOD #001 不支持相同指令覆盖机制 */
//		if((id != SIGNAL_QUERY)&&(id != SET_TXT_MODE)&&(id != NETWORK_REG_QUERY))
		{

	        for ( i = 0; ( SHORT_CMD_ID( i ) != id )&&( i < MDM_SCMD_LIST_LENGTH ); i++ );
	        if ( i < MDM_SCMD_LIST_LENGTH )
	        {
	          MDM_CmdCpy ( ( MDM_CmdTypedef * )&SHORT_CMD( i ), command );
	          SHORT_CMD_INDEX( i ) = ++SHORT_CMD_NUMBER; 

	          if ( params )
	          {
	            snprintf ( ( char* )SHORT_CMD_AT_BUFFER( i ), MDM_SCMD_BLK_SIZE, 
	                       ( const char* )command->AtBuffer, params );
	          }

#if MDM_INFO_DEBUG == 1 && MDM_AT_SEND_DEBUG == 1
	{
	          printf ( "\r\n[MDM EVENT] rewrite short cmd [%u] %s, counter %u\r\n", 
	                   i, 
	                   SHORT_CMD_AT_BUFFER( i ), 
	                   SHORT_CMD_COUNTER );
	}
#endif  /* MDM_INFO_DEBUG */

	          return TRUE;        
	        }
		}
#endif

        for ( i = 0; SHORT_CMD_ID( i ) &&( i < MDM_SCMD_LIST_LENGTH ); i++ );
        if ( i < MDM_SCMD_LIST_LENGTH )
        {
          u8 len = strlen ( ( const char* )command->AtBuffer );

          if ( params )
          {
            len += strlen ( ( const char* )params );
          }

          if ( len < MDM_SCMD_BLK_SIZE )
          {
            ++SHORT_CMD_COUNTER;	//-增加了一个待处理命令

            /* BUG #000 修正参数拷贝顺序错误!! */
            MDM_CmdCpy ( ( MDM_CmdTypedef * )&SHORT_CMD( i ), command );
            if ( MDM_QueryFlowStatus () == MDM_FCM_V24_STATE_SW && SHORT_CMD_ID( i ) == ESCAPE_COMMAND )
            {
              SHORT_CMD_INDEX( i ) = 0;
//              MDM_ClearGlobalAtCmd ();
//              MDM_ClearGlobalAtRsp ();
            }
            else
            if ( SHORT_CMD_ID( i ) == RETURN_TO_DATA_STATE ) //MDM_QueryFlowStatus () == MDM_FCM_V24_STATE_SW && 
            {
              SHORT_CMD_INDEX( i ) = 0xFFFFFFFF;
            }
            else
            {
              SHORT_CMD_INDEX( i ) = ++SHORT_CMD_NUMBER;  
            }

            if ( params )
            {
              snprintf ( ( char* )SHORT_CMD_AT_BUFFER( i ), MDM_SCMD_BLK_SIZE, 
                         ( const char* )command->AtBuffer, params );
            }

#if MDM_INFO_DEBUG == 1 && MDM_AT_SEND_DEBUG == 1
{
            printf ( "\r\n[MDM EVENT] write short cmd [%u] %s, counter %u, ser %u\r\n", 
                     i, 
                     SHORT_CMD_AT_BUFFER( i ), 
                     SHORT_CMD_COUNTER, 
                     SHORT_CMD_INDEX( i ) );
}
#endif  /* MDM_INFO_DEBUG */

            return TRUE;          
          }
        }

        for ( i = 0; LONG_CMD_ID( i ) &&( i < MDM_LCMD_LIST_LENGTH ); i++ );
        if ( i < MDM_LCMD_LIST_LENGTH )
        {
          ++LONG_CMD_COUNTER;

          /* BUG #001 修正参数拷贝顺序错误!! */
          MDM_CmdCpy ( ( MDM_CmdTypedef * )&LONG_CMD( i ), command );
          if ( MDM_QueryFlowStatus () == MDM_FCM_V24_STATE_SW && LONG_CMD_ID( i ) == ESCAPE_COMMAND )
          {
            LONG_CMD_INDEX( i ) = 0;
//            MDM_ClearGlobalAtCmd ();
//            MDM_ClearGlobalAtRsp ();
          }
          else
          if ( LONG_CMD_ID( i ) == RETURN_TO_DATA_STATE ) //MDM_QueryFlowStatus () == MDM_FCM_V24_STATE_SW &&
          {
            LONG_CMD_INDEX( i ) = 0xFFFFFFFF;
          }
          else
          {
            LONG_CMD_INDEX( i ) = ++LONG_CMD_NUMBER;
          }

          if ( params )
          {
            snprintf ( ( char* )LONG_CMD_AT_BUFFER( i ), MDM_LCMD_BLK_SIZE, 
                       ( const char* )command->AtBuffer, params );
          }

#if MDM_INFO_DEBUG == 1 && MDM_AT_SEND_DEBUG == 1
{
          printf ( "\r\n[MDM EVENT] write long cmd [%u] %s, counter %u, ser %u\r\n", 
                   i, 
                   LONG_CMD_AT_BUFFER( i ), 
                   LONG_CMD_COUNTER, 
                   LONG_CMD_INDEX( i ) );
}
#endif  /* MDM_INFO_DEBUG */

          return TRUE;
        }
        break;       
      }
    }  
  }

  return FALSE;
}








/* 
 * 功能描述: 通用工具处理器
 * 引用参数: (1)命令
 *           (2)响应 
 *
 * 返回值  : 处理状态
 * 
 */
extern AtCmdHandler_Status MDM_Utils_Handler ( MDM_CmdTypedef *at, MDM_RspTypedef *response )
{
  MDM_RSP_ID code = MDM_GetResultCode ( response );

  switch ( code )
  {
    case MDM_OK:
    	return AT_HANDLER_DONE;
            
    case MDM_ERROR:
     	//return AT_HANDLER_DONE_ERROR;
            
    case MDM_TIME_OUT:
      return AT_HANDLER_RETRY;

    default:
      return AT_HANDLER_DONT_UNDERSTAND;
  }
}







/* 
 * 功能描述: 响应超时处理
 * 引用参数:
 *          
 * 返回值  :
 * 
 */
static void MDM_RspTimeoutHandler ( u8 ID )
{

#if MDM_INFO_DEBUG == 1 && MDM_MONITOR_DEBUG == 1
  printf ( "\r\n[MDM EVENT] response timeout trigger!\r\n" );
#endif  /* MDM_INFO_DEBUG */    
  
  if ( mdm_glb_cmd.retry_counter <= 0 )
  {
    mdm_glb_rsp.finish = TRUE;
    mdm_glb_rsp.Code = MDM_TIME_OUT;
    if ( mdm_glb_cmd.handler )
    {
      mdm_glb_cmd.handler ( ( MDM_CmdTypedef * )&mdm_glb_cmd, ( MDM_RspTypedef * )&mdm_glb_rsp );
    }
    
    if ( modem )
    {
      modem->is_timeout = FALSE;
      modem->status = MDM_READY;    
    }
  }
  else
  {
    MDM_RetryCurrentCommand ();
  }

  if ( mdm_glb_rsp.finish == TRUE )
  {

#if MDM_INFO_DEBUG == 1 && MDM_MONITOR_DEBUG == 1
{
    printf ( "\r\n[MDM EVENT] monitor stop\r\n" );
}
#endif  /* MDM_INFO_DEBUG */ 

    if ( tmrMdmMonitor )
    {
      TMR_UnSubscribe ( tmrMdmMonitor, ( tmr_procTriggerHdlr_t )MDM_RspTimeoutHandler, TMR_TYPE_10MS );
      tmrMdmMonitor = NULL;
    }
  }
}










/* 
 * 功能描述: 等待用户指令响应
 * 引用参数:
 *          
 * 返回值  :
 * 
 */
static void MDM_WaitForCmdRsp ( u32 Timeout )
{
  /* Wait for a modem operation to complete 
     or a TIMEOUT to occur */
  if ( ( mdm_glb_rsp.finish != TRUE )&&( Timeout != 0x00 ) )
  {
    if ( tmrMdmMonitor )
    {
      TMR_UnSubscribe ( tmrMdmMonitor, ( tmr_procTriggerHdlr_t )MDM_RspTimeoutHandler, TMR_TYPE_10MS );
      tmrMdmMonitor = NULL;
    }

    tmrMdmMonitor = TMR_Subscribe ( TRUE, Timeout, TMR_TYPE_10MS, ( tmr_procTriggerHdlr_t )MDM_RspTimeoutHandler );
  }
}







/**
 * @brief Re-excute current AT command at the tail of command queue
 * @param modem Modem device struct pointer
 * @param interface interface whose current command to retry
 */
extern s32 MDM_QueryCurrentRetryCounter ( void )
{
  return mdm_glb_cmd.retry_counter;
}








/* 
 * 功能描述: 读取MODEM响应
 * 引用参数:
 *          
 * 返回值  :
 * 
 */
extern bool MDM_ReadData ( u16 DataSize, u8 *Data )
{
  AtCmdHandler_Status status = AT_HANDLER_DONT_UNDERSTAND;
  AtCmdHandler handler = NULL;
  u16 length = 0;
  u16 index  = 0;
  bool result = FALSE;


  if ( mdm_glb_cmd.handler )	//-指向正在处理的AT指令
  {
   	handler = mdm_glb_cmd.handler;
  }

  /* 清空接收缓冲区 */
  //mdm_glb_rsp.totalsize = 0;
  
  /* BUG #004 修正比较对象 */
  while ( index < DataSize )     
  {
    /* 1.更新数据接收处理长度 */
    if ( ( mdm_glb_rsp.totalsize + DataSize - index ) < MDM_RX_BUFF_SIZE_MAX )
    {
      length = ( DataSize - index );    
    }
    else
    {
      length = ( MDM_RX_BUFF_SIZE_MAX - mdm_glb_rsp.totalsize );
    }
    memcpy ( ( u8* )&mdm_glb_rsp.indicator[ mdm_glb_rsp.totalsize ], Data + index, length );
  	mdm_glb_rsp.totalsize += length;
  	
		mdm_glb_rsp.Code = MDM_StrGetID ( ( MDM_RspTypedef * )&mdm_glb_rsp );	
    
		mdm_glb_rsp.finish = MDM_StrIsTerminalResponse ( mdm_glb_rsp.Code );

    /* 2.数据分析处理 */
    if ( handler )
    {
      status = ( *handler )( ( MDM_CmdTypedef * )&mdm_glb_cmd, ( MDM_RspTypedef * )&mdm_glb_rsp );
			
			
      switch ( status )
      {
        case AT_HANDLER_DONE:
        case AT_HANDLER_DONE_ERROR:
#if MDM_INFO_DEBUG == 1 && MDM_AT_HANDLER_DEBUG == 1
{
            printf ( "\r\n[MDM EVENT] status AT_HANDLER_DONE\r\n" );
}
#endif  /* MDM_AT_HANDLER_DEBUG */
            result = TRUE;
            if ( tmrMdmMonitor )
            {
              TMR_UnSubscribe ( tmrMdmMonitor, ( tmr_procTriggerHdlr_t )MDM_RspTimeoutHandler, TMR_TYPE_10MS );
              tmrMdmMonitor = NULL;
            }
            MDM_Handler_UnsRsp ( ( MDM_CmdTypedef * )&mdm_glb_cmd, ( MDM_RspTypedef * )&mdm_glb_rsp );
            MDM_ClearGlobalAtCmd ();
            MDM_ClearGlobalAtRsp ();

            if ( modem )
            {
              /* if handled then process next command if there is */
              modem->status = MDM_READY;            
            }
            break;
            
        case AT_HANDLER_ERROR:
#if MDM_INFO_DEBUG == 1 && MDM_AT_HANDLER_DEBUG == 1
{
            printf ( "\r\n[MDM EVENT] status AT_HANDLER_ERROR\r\n" );
}
#endif  /* MDM_AT_HANDLER_DEBUG */

            if ( modem )
            {
              modem->is_timeout = FALSE;
              modem->status = MDM_READY;            
            }

            /* run unsolicite because we don't know if the message
             * belonged to handler or not */ 
            MDM_Handler_UnsRsp ( ( MDM_CmdTypedef * )&mdm_glb_cmd, ( MDM_RspTypedef * )&mdm_glb_rsp );
            break;
            
        case AT_HANDLER_NEED_MORE:
#if MDM_INFO_DEBUG == 1 && MDM_AT_HANDLER_DEBUG == 1
{
            printf ( "\r\n[MDM EVENT] status AT_HANDLER_NEED_MORE\r\n" );
}
#endif  /* MDM_AT_HANDLER_DEBUG */
            break;
            
        case AT_HANDLER_DONT_UNDERSTAND:
#if MDM_INFO_DEBUG == 1 && MDM_AT_HANDLER_DEBUG == 1
{
            printf ( "\r\n[MDM EVENT] status AT_HANDLER_DONT_UNDERSTAND\r\n" );
}
#endif  /* MDM_AT_HANDLER_DEBUG */

            if ( MDM_Handler_UnsRsp ( ( MDM_CmdTypedef * )&mdm_glb_cmd, ( MDM_RspTypedef * )&mdm_glb_rsp ) == AT_HANDLER_DONE )
            {
            	if ( modem )
              {
                modem->status = MDM_READY;
              }        
            }
            break;
  
        case AT_HANDLER_RETRY:
#if MDM_INFO_DEBUG == 1 && MDM_AT_HANDLER_DEBUG == 1
{
            printf ( "\r\n[MDM EVENT] status AT_HANDLER_RETRY\r\n" );
}
#endif  /* MDM_AT_HANDLER_DEBUG */

            //MDM_RetryCurrentCommand ();
            result = TRUE;
            break;
      }


/****************************** 
 *   查询是否切换模式？       *
 *      Y ==> 清除响应        *
 ******************************/

      if ( MDM_QueryFlowStatus () == MDM_FCM_V24_STATE_SW )
      {
        MDM_ClearGlobalAtRsp ();  
      }

    }
    else
    {
      if ( MDM_Handler_UnsRsp ( ( MDM_CmdTypedef * )&mdm_glb_cmd, ( MDM_RspTypedef * )&mdm_glb_rsp ) == AT_HANDLER_DONE )
      {
      	if ( modem )
        {
          modem->status = MDM_READY;
        }

#if MDM_INFO_DEBUG == 1 && MDM_AT_HANDLER_DEBUG == 1
{
        printf ( "\r\n[MDM EVENT] status MDM_Handler_UnsRsp\r\n" );
}
#endif  /* MDM_AT_HANDLER_DEBUG */

        result = TRUE;
      }
    }

    index += length;
    if ( mdm_glb_rsp.totalsize >= MDM_RX_BUFF_SIZE_MAX )
    {
      mdm_glb_rsp.totalsize = 0;
      memset ( mdm_glb_rsp.indicator, 0, MDM_RX_BUFF_SIZE_MAX );
    }
  }

  return result;
}









/**
 * @brief Parses cms error and stoes it to given variable
 * @param response string to parse cms error from
 * @param code code to store cms code
 * @return true if cms error was found and parsed from response
 */
extern bool MDM_Utils_ParseCMSerror ( MDM_RspTypedef *response, s32 *code )
{
  ascii *token = MDM_StrStr ( response, MDM_Sym2Str ( symbol_p, MDM_CMS_ERROR ) );

  if ( token )
  {
    token = ( ascii* ) strchr ( ( const char* )token, ' ' );
    if ( token )
    {
      *code = atoi ( ( const char* )token );
      return TRUE;     
    } 
  }
  return FALSE;
}










/**
 * @brief Parses cme error and stoes it to given variable
 * @param response string to parse cms error from
 * @param code code to store cms code
 * @return true if cms error was found and parsed from response
 */
extern bool MDM_Utils_ParseCMEerror ( MDM_RspTypedef *response, s32 *code )
{
  ascii *token = MDM_StrStr ( response, MDM_Sym2Str ( symbol_p, MDM_CME_ERROR ) );

  if ( token )
  {
    token = ( ascii* ) strchr ( ( const char* )token, ' ' );
    if ( token )
    {
      *code = atoi ( ( const char* )token );
      return TRUE;     
    } 
  }
  return FALSE;
}









/**
 * @brief Sets modem's state to RESET
 * @param modem modem to set
 */
extern void MDM_Reset ( void )
{
  if ( modem )
  {

#if MDM_INFO_DEBUG == 1
{
    printf ( "\r\n[MDM STATE] is reset\r\n" );
}
#endif  /* MDM_INFO_DEBUG */

    modem->status = MDM_RESET;
    modem->sim_status = SIM_UNKNOWN;
    modem->is_timeout = FALSE;

    MDM_ClearGlobalAtCmd ();
    MDM_ClearAtCmdList ();
    MDM_ClearGlobalAtRsp ();
  }
}








/* Common routines */
#define IS_AF(c)  ((c >= 'A') && (c <= 'F'))
#define IS_af(c)  ((c >= 'a') && (c <= 'f'))
#define IS_09(c)  ((c >= '0') && (c <= '9'))
#define ISVALIDHEX(c)  IS_AF(c) || IS_af(c) || IS_09(c)
#define ISVALIDDEC(c)  IS_09(c)
#define CONVERTDEC(c)  (c - '0')







/**
 * @brief Handles unsolicited messages such as +CREG.
 * Unsolicite mesaage is an unexpected message or reply or a reply the
 * command's handler didn't understand.
 * @param at current at command's context
 * @param response message to handle
 * @return handler status
 */
static AtCmdHandler_Status MDM_Handler_UnsRsp ( MDM_CmdTypedef *at, MDM_RspTypedef *response )
{
  MDM_RSP_ID code = MDM_GetResultCode ( response );
  AtCmdHandler_Status at_status = AT_HANDLER_DONT_UNDERSTAND;

  /* Running our default handler */
  switch ( code )
  {
    case MDM_BUSY:	/* remote hangup the call */
      at_status = AT_HANDLER_DONE;
      break;

#if 0        
    case MDM_RING:	/* handled in CLIP */
		  printf("\r\n   \r\n");
      MDM_HandleRing ();
      break;
#endif

    case MDM_NO_ANSWER:
      at_status = AT_HANDLER_DONE;
      break;

    case MDM_NO_CARRIER:
      at_status = AT_HANDLER_DONE;
      break;

    case MDM_CONNECT:
      at_status = AT_HANDLER_DONE;
      break;
                    
    default:
    {  
      ascii *token = NULL;      

      if ( MDM_StrStr ( response, "+" ) )
      {
        token = MDM_StrStr ( response, MDM_Sym2Str ( symbol_p, MDM_CREG ) );
        if ( token )
        {
          MDM_HandleNetworkStatus ( response );
          at_status = AT_HANDLER_DONE;
        } 

        token = MDM_StrStr ( response, MDM_Sym2Str ( symbol_p, MDM_CMTI ) );
        if ( token )
        {
          token = ( ascii* ) strchr ( ( const char * )token, ',' );
          if ( token )
          {
            ++token;
            if ( ISVALIDDEC ( *token ) )
            {
              s32 sms_pos = atoi ( ( const char * )token ); 

              if ( sms_pos > 0 && modem && modem->sim && modem->sim->incoming_message )
              {
                modem->sim->incoming_message ( sms_pos );
                at_status = AT_HANDLER_DONE;
              }
            }
          }
        }
    
        token = MDM_StrStr ( response, MDM_Sym2Str ( symbol_p, MDM_CCWA ) );
        if ( token )
        {
          MDM_HandleCallWaiting ( response );
          at_status = AT_HANDLER_DONE;
        }  
    
        token = MDM_StrStr ( response, MDM_Sym2Str ( symbol_p, MDM_CLIP ) );
        if ( token )
        {

#if MDM_INFO_DEBUG == 1
{
          printf ( "\r\n[MDM EVENT] clip event trigger\r\n" );
}
#endif  /* MDM_AT_HANDLER_DEBUG */

          /* display the incoming call phoneNo */
          MDM_HandleIncomingCall ( response );
          at_status = AT_HANDLER_DONE;
        }  

        token = MDM_StrStr ( response, MDM_Sym2Str ( symbol_p, MDM_CRING ) );
        if ( token )
        {
          /* don't handle RING, we use CLIP */ 
          MDM_HandleRing ();
          at_status = AT_HANDLER_DONE;
        }
      }
      break;
    } 
  }

  return at_status;
}







/**
 * @brief Handles clip messages
 * Creates a new call with unknown call id
 * @param response string with CLIP
 */
static void MDM_HandleIncomingCall ( MDM_RspTypedef *response )
{
  /* +CLIP: "",128,,128,"" */
  MDM_RSP_ID code = MDM_GetResultCode ( response );

#if 0  
  if ( code == MDM_RING )
#endif

  {
    Call call = { CALL_INCOMING, NULL, "129", 1 };

#if MDM_INFO_DEBUG == 67
{
    printf ( "\r\n[MDM EVENT] incomming call\r\n" );
}
#endif  /* MDM_AT_HANDLER_DEBUG */

    MDM_ConfigureCurrentCall ( ( Call * )&call );
    if ( modem && modem->call && modem->call->call_status )
    {
      modem->call->call_status ( call.id, CALL_INCOMING ); 
    }
  }  
}






/**
 * @brief Parses clcc line and applies information to modem's calls
 * Can also create new calls if necessary.
 * @param modem pointer to modem whose calls to modify
 * @param clcc line returned by clcc
 * @return handled call or NULL
 */
void MDM_ApplyCurrentCall ( const ascii *clcc, Call *call )
{
  s32 state = 0;

#if 0
  ascii *number;
  s32 index = 0;
#endif

  switch ( state )
  {
    case 0:
      call->status = CALL_CONNECTED;
      break;
        
    case 1:
      call->status = CALL_HOLD;
      break;
        
    case 2:
    case 3:
      call->status = CALL_CALLING;
      break;
        
    case 4:
    case 5:
      call->status = CALL_INCOMING;
      break;
  }
}







/**
 * @brief Finds first call with incoming status and sends call status signal
 * Should be called when ring message is received from gsm modem.
 * @param modem modem who sent ring message
 */
static void MDM_HandleRing ( void )
{
}








/**
 * @brief Handles CCWA (call waiting) messages from modem
 * @param modem modem whose call waiting message to handle
 * @param response string with CCWA
 */
static void MDM_HandleCallWaiting ( MDM_RspTypedef *response )
{
}






/**
 * @brief Handles network status (+CREG) messages
 * @param response string to handle
 */
static void MDM_HandleNetworkStatus ( MDM_RspTypedef *response )
{
  NetworkStatus status = NETWORK_UNKNOWN;
  ascii *token = MDM_StrStr ( response, MDM_Sym2Str ( symbol_p, MDM_CREG ) );
  
  if ( token ) 
  {
    token = ( ascii * )strchr ( ( const char * )token, ' ' );
    if ( token ) 
    {
      status = ( NetworkStatus )atoi ( ( const char * )token );   
      MDM_ChangeNetworkStatus ( status );
    }
  } 
}







/**
 * @brief Adds an modem alive test command (AT) or soft reset (ATZ) to the front
 * of the queue ignoring uncancellable commands and processes it
 * @param modem_if pointer to modem device struct
 * @param reset if TRUE, send ATZ instead of AT
 */
extern void MDM_PostAliveTest ( bool reset )
{
  
  if ( reset == TRUE )
  {
    MDM_PostAtCmdByID ( command_p, 
                        SOFT_RESET, 
                        NULL ); 
  }
  else
  {
    MDM_PostAtCmdByID ( command_p, 
                        REPLY_COMMAND, 
                        NULL ); 
  }
}


extern void MDM_Atd2_Command( void )
{
    MDM_PostAtCmdByID ( command_p, 
                        DTR_SET, 
                        NULL );
}

extern void MDM_At_Auto_Answer(const char num)
{
	ascii params[ 10 ];
	sprintf ( ( char * )params, "%d", num );
    MDM_PostAtCmdByID ( command_p, 
                        AUTO_ANSWER, 
                        params );
}

extern void MDM_At_Command_Echo( void )
{
    MDM_PostAtCmdByID ( command_p, 
                        COMMAND_ECHO, 
                        NULL );
}




/**
 * @brief Handler for command to test if modem is alive
 * @param at current at command
 * @param response message to handle
 * @return handler status
 */
static AtCmdHandler_Status MDM_Handler_ReadSerialPortRate ( MDM_CmdTypedef *at, MDM_RspTypedef *response )
{
  MDM_RSP_ID code = MDM_GetResultCode ( response );
  ascii *token = NULL;

  if ( code == MDM_OK )
  {
    token = MDM_StrStr ( response, MDM_Sym2Str ( symbol_p, MDM_IPR ) );
    if ( token )
    {
      token = ( ascii* )strchr ( ( const char * )token, ' ' );
      if ( token )
      {
        s32 baudrate = atoi ( ( const char* )token );

#if 0        
        if ( baudrate != 115200 )
        {
          MDM_PostAtCmdByID ( command_p, 
                              SET_SERIAL_PORT_RATE, 
                              "115200" );                     
        }
#endif

        return AT_HANDLER_DONE; 
      }  
    }
  }

  return AT_HANDLER_DONT_UNDERSTAND; 
}







/**
 * @brief Handler for command to test if modem is alive
 * @param at current at command
 * @param response message to handle
 * @return handler status
 */
static AtCmdHandler_Status MDM_Handler_SetSerialPortRate ( MDM_CmdTypedef *at, MDM_RspTypedef *response )
{
  MDM_RSP_ID code = MDM_GetResultCode ( response );

  if ( code == MDM_OK )
  {
    MDM_Reset ();
    if ( modem )
    {
      modem->status = MDM_SERIAL_INIT;
    }
    MDM_PostAliveTest ( FALSE );
    return AT_HANDLER_DONE;
  }

  return AT_HANDLER_DONT_UNDERSTAND;
}










/**
 * @brief Handler for command to test if modem is alive
 * @param at current at command
 * @param response message to handle
 * @return handler status
 */
static AtCmdHandler_Status MDM_Handler_Alive ( MDM_CmdTypedef *at, MDM_RspTypedef *response )
{
  MDM_RSP_ID code = MDM_GetResultCode ( response );

  /* Modem didn't reply or replied with an error */
  if ( code == MDM_TIME_OUT )
  {
    if ( modem && modem->serial && modem->serial->no_response_error )
    {
      /* Ask vendor if it can deal with an unresponsive modem */
      modem->serial->no_response_error ( NULL );  
    }      

    return AT_HANDLER_ERROR;
  }
  else if ( code == MDM_OK )
  {
    /* we've got a response, modem is alive. Start initializations */
    if ( modem && modem->status != MDM_INITIALIZING )
    {
      modem->status = MDM_INITIALIZING;
    }
		
    MDM_GeneralAtInit ();
    return AT_HANDLER_DONE;
  }

  return AT_HANDLER_DONT_UNDERSTAND;
}











/**
 * @brief Changes sim's status
 * @param modem modem whose sim status to change
 * @param status new status to set
 */
extern void MDM_ChangeSIMstatus ( SIM_Status status )
{
  if ( modem )
  {
    if ( ( modem->sim_status < SIM_READY )&&( status >= SIM_READY ) )
    {
      MDM_GeneralAtInitWithSIMReady ();
    }
  	
    modem->sim_status = status;    
  }
}







/**
 * @brief Change network status
 * @param modem modem interface pointer
 * @param new status
 */
extern void MDM_ChangeNetworkStatus ( NetworkStatus status )
{
  if ( modem && modem->network && modem->network->network_status )
  {
    modem->network->network_status = status;
  }
//  printf("MDM_ChangeNetworkStatus status=%d\r\n",status);
  switch ( status )
  {
    case NETWORK_UNREGISTERED:  /* NETWORK_UNREGISTERED -> clear provider name, send empty status signal */
    case NETWORK_BUSY:          /* NETWORK_UNKNOWN -> send empty status signal */
    case NETWORK_DENIED:        /* NETWORK_UNKNOWN -> send empty status signal */
    case NETWORK_UNKNOWN:       /* NETWORK_UNKNOWN -> send empty status signal */
      if ( modem && modem->network && modem->network->status && modem->network->network_status )
      {
        modem->network->status ( GSM_NETWORK_TYPE, modem->network->network_status );
      }
      break;

    case NETWORK_ROAMING:       /* NETWORK_ROAMING -> ask provider name and signal strength */
    case NETWORK_REGISTERED:    /* NETWORK_REGISTERED -> ask provider name and signal strength */
      if ( modem && modem->network && modem->network->status && modem->network->network_status )
      {
        modem->network->status ( GSM_NETWORK_TYPE, modem->network->network_status );
      }
      break;
  }
}



