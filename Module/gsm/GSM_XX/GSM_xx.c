#include <stdio.h>
#include <string.h>
#include "hardware_select_config.h"
#include "project_configure.h"
#include "defines.h"



#if  USE_GSM_MODEM_XXX ==1

#define GSM_MODEM_DEBUG    0


#include "GSM_xx.h"
#include "tiza_timer_port.h"
#include "tiza_sim_port.h"
#include "tiza_params.h  "
#include "lwip_sys_arch.h"
#include "wip_init.h"
#include  "modem_debug.h"

/******************************************/
/*              内部变量[定义]            */
/******************************************/

static Serial_IfTypedef   usart;
static SIM_IfTypedef      sim;
static Call_IfTypedef     call;
static PDP_IfTypedef      pdp;
static Device_IfTypedef		device;
static Network_IfTypedef  network;
static Vendor_IfTypedef   vendor;

static MDM_DevTypedef    
GSM_xx = { FALSE, 
          &usart, &call, NULL, &device, &network, &sim, &pdp, &vendor,
          NULL, 
          SIM_UNKNOWN,
          NULL, 
          MDM_UNINITIALIZED };


/* 振铃检测脚 */
s32 ring_io_handler  = -1;

/* GSM电源控制脚 */
s32 gsm_io_handler   = -1;

/* IGT信号控制脚 */
s32 igt_io_handler   = -1;

/* 切换超时标志 */
bool MDM_SWITCH_TO = FALSE;

s32 rts_io_handler  = -1;



extern void sim_read_all_message_reply ( void* ipc_data, s16 index, const ascii *sender_number, const ascii *contents );
extern void sim_incoming_message ( s32 index );
extern void Gl868_Command_Socket_Inactive_Timeout( void* ipc_data,u16 tout );
extern void Gl868_Command_Socket_Set( void* ipc_data, u8 type, u16 port, const char *IP);
extern void Gl868_Command_Socket_Open( void* ipc_data);
extern bool ftp_recvDataHdlr ( u16 DataSize, u8 *Data );
extern void Gl868_Select_Flow_control( void* ipc_data, u8 type);
extern void sim_get_imsi( void* ipc_data, ascii *imsi);
extern void GSM_Call_handler_SetCallRing(void* ipc_data);

/******************************************/
/*              内部函数[声明]            */
/******************************************/

static void Gl868_IfInit ( u8 ID );
extern void Gl868_Command_SocketConfigurationExtend ( void* ipc_data,const char connId, const char srMode,
                             const char recvDataMode, const char keepalive,const char ListenAutoRsp,const char endDataMode  );
extern void Gl868_Command_SocketDial ( void* ipc_data,const char connId, const char txProt,
                             const u16 rPort, const char *IP,const char closureType,const u16 lPort, const u8 connMode  );
extern void Gl868_Command_PppParamConfig ( void* ipc_data,const char *IP, const short LcpTimeOut,
                             const char PppMode);
extern void GL868_PDP_Command_AddContext ( void* ipc_data, int index, const char *APN );
extern void PDP_Command_GprsDetach ( void* ipc_data );
extern void Gl868_Command_GprsContextNoActivation ( void* ipc_data );
extern void Gl868_Command_Socket_Shutdown( void* ipc_data,const char connId );
extern void NETWORK_Command_QueryNetworkRegistration ( void* ipc_data );
extern void Gl868_Command_SocketConfiguration ( void* ipc_data,const char connId, const char cid,
							 const short pktSz, const short maxTo,const short connTo,const char txTo );
extern void Gl868_Cfg_Http_Params( void* ipc_data );
extern void GL868_PDP_M2mLocate_AddContext ( void* ipc_data, int index, const char *APN );
extern void Gl868_Pdp_Context_Activation( void* ipc_data, u8 cid, u8 stat);
extern void Gl868_Pdp_Context_Act_Config( void* ipc_data );
extern void Gl868_Dns_Response_Cach( void* ipc_data );
extern void Gl868_Request_Location( void* ipc_data );





void MDM_ResetPIN(void)
{
	/*GSM reset pin initation*/
	TM_GPIO_Init(GPIOF,GPIO_Pin_8,TM_GPIO_Mode_OUT,TM_GPIO_OType_PP,TM_GPIO_PuPd_NOPULL,TM_GPIO_Speed_Fast);
	/*GSM reset pin pulling up*/
	TM_GPIO_SetPinHigh(GPIOF,GPIO_Pin_8);
}






/* 
 * 功能描述：SIM6320C关机
 * 引用参数：无
 *          
 * 返回值  ：无
 * 
 */
void SIM6320C_CLOSE(void)
{
		TM_GPIO_SetPinHigh(GPIOC,GPIO_Pin_0);  
}

/* 
 * 功能描述：GSM_xx关机
 * 引用参数：无
 *          
 * 返回值  ：无
 * 
 */
extern void GL868_SwitchOFF(void)
{
		/* GSM电源输出低电平 */
	TM_GPIO_SetPinHigh(GPIOC,GPIO_Pin_0);
	TMR_Subscribe( FALSE, 30, TMR_TYPE_100MS, ( tmr_procTriggerHdlr_t )SIM6320C_CLOSE );
}

/* 
 * 功能描述：SIM6320C开机
 * 引用参数：无
 *          
 * 返回值  ：无
 * 
 */
void Modem_init(void)
{
	TM_GPIO_SetPinLow(GPIOC,GPIO_Pin_0);
	Gl868_IfInit(0);
}
void SIM6320C_Open(void)
{
	TM_GPIO_SetPinHigh(GPIOC,GPIO_Pin_0);

  TMR_Subscribe ( FALSE, 20, TMR_TYPE_100MS, ( tmr_procTriggerHdlr_t )Modem_init );

}
/* 
 * 功能描述：GSM_xx上电开机
 * 引用参数：无
 *          
 * 返回值  ：无
 * 
 */
extern void GL868_PowerSwitchON(void)
{
  /* 1.注册GSM电源控制脚 */
	//初始GMS开机引脚
	TM_GPIO_Init(GPIOC,GPIO_Pin_0,TM_GPIO_Mode_OUT,TM_GPIO_OType_PP,TM_GPIO_PuPd_NOPULL,TM_GPIO_Speed_Fast);
	/* GSM电源输出低电平 */
	TM_GPIO_SetPinLow(GPIOC,GPIO_Pin_0);

	TMR_Subscribe ( FALSE, 20, TMR_TYPE_100MS, ( tmr_procTriggerHdlr_t )SIM6320C_Open );
  	
}

tmr_t *tmrPpp = NULL;
extern void Gl868_Command_PppParamConfig ( void* ipc_data,const char *IP, const short LcpTimeOut,
                             const char PppMode);

extern void PppConfig ( void )
{
    u8 IP[16];

    sprintf ( ( char * )IP, "%s", "0.0.0.0" );
	
    Gl868_Select_Flow_control(NULL, 0);  //AT+FLO=0
    Gl868_Command_PppParamConfig(NULL, (const char *)IP, 25, 1);//AT#GPPPCFG="0.0.0.0",25,1
    GL868_PDP_Command_AddContext ( NULL, 1, ( const char* )"CMNET" );//AT+CGDCONT=1,"IP","CMNET"
    lgsm_pdp_request_gprs_service ( NULL, 1 ); //ATD*99***%s#
	
} 




extern void wip_NetExit ( void );
extern void Call_handler_release(void);
extern void XMIT_Stop ( void );
/* 
 * 功能描述：GSM_xx 基站定位
 * 引用参数：u8   act_flag     PDP deactivate 标志
 *          
 * 返回值  ：无
 * 
 */

extern void Gl868_M2mLocate_Request(u8 act_flag)
{
    XMIT_Stop ();
    wip_NetExit();
    Call_handler_release();//ATH
    NETWORK_Command_QueryNetworkRegistration( NULL );//AT+CREG?
    //NETWORK_Command_QueryCurrentOperator( NULL );    //AT+COPS?
    //Gl868_Cfg_Http_Params( NULL );                   //AT#HTTPCFG?
    if(act_flag != 0)
    {
      lgsm_pdp_deactivate(NULL, 1);// AT+CGACT=0,1
	    GL868_SwitchToATStatus ( MDM_Switch_Type_M2MLocate );
    }   
    GL868_PDP_M2mLocate_AddContext( NULL, 1, ( const char* )"ibox.tim.it" );//AT+CGDCONT=1,"IP","ibox.tim.it","0.0.0.0",0,0
    Gl868_Pdp_Context_Activation( NULL, 1, 1);   //AT#SGACT=1,1
    Gl868_Pdp_Context_Act_Config( NULL );        //AT#SGACTCFG?
    Gl868_Dns_Response_Cach( NULL );             //AT#CACHEDNS=1
    Gl868_Request_Location( NULL );              //AT#AGPSSND
}


/* 
 * 功能描述：GSM_xx断电关机
 * 引用参数：无
 *          
 * 返回值  ：无
 * 
 */
extern void GL868_PowerSwitchOFF ( void )
{
  /* GSM电源输出低电平 */
  if ( gsm_io_handler > 0 )
  {

  }
}

static void GL868_NoRspErrorHandler ( void* ipc_data )
{
	  /************************************************************
	   *目前无响应处理的情况主要如下：
	   * (1)MODEM通信波特率不一致;					   [已处理]
	   * (2)MODEM启动失败，处于停止状态；			   [未处理]
	   * (3)MODEM处于数据通信状态，AT指令无响应；	   [已处理]
	   * (4)其他....
	   ************************************************************/
	
#if GSM_MODEM_DEBUG == 1
	{
	  PRINTF_GPS_TIME ();
	  printf ( "\r\n[GSM_xx ERR] modem isn't response\r\n" );
	}
#endif  /* GSM_MODEM_DEBUG */
	
	  /* 1.MODEM复位 */
	  MDM_Reset ();
	
	  /* 2.关闭通信端口 */
	  USART_Stop(USART_FCM_MODEM );
	
	
	  /* 3.更换通信波特率 */
	  if ( vehicleParams.gsmBaudrate == USART_BaudRate_115200bps )
	  {
	
#if GSM_MODEM_DEBUG == 1
	{
		PRINTF_GPS_TIME ();
		printf ( "\r\n[GSM_xx ERR] modem change baudrate 38400\r\n" );
	}
#endif  /* GSM_MODEM_DEBUG */
	
		vehicleParams.gsmBaudrate = USART_BaudRate_38400bps;
//		STORAGE_WriteData ( VEHICLE_PARAMS, sizeof( vechile_Params_t ), ( u8 * )&vehicleParams );
	  }
	  else
	  if ( vehicleParams.gsmBaudrate == USART_BaudRate_38400bps )
	  {
	
#if GSM_MODEM_DEBUG == 1
	{
		PRINTF_GPS_TIME ();
		printf ( "\r\n[GSM_xx ERR] modem change baudrate 9600\r\n" );
	}
#endif  /* GSM_MODEM_DEBUG */
	
		vehicleParams.gsmBaudrate = USART_BaudRate_9600bps;
//		STORAGE_WriteData ( VEHICLE_PARAMS, sizeof( vechile_Params_t ), ( u8 * )&vehicleParams );
	  }
	  else
	  if ( vehicleParams.gsmBaudrate == USART_BaudRate_9600bps )
	  {
	
#if GSM_MODEM_DEBUG == 1
	{
		PRINTF_GPS_TIME ();
		printf ( "\r\n[GSM_xx ERR] modem change baudrate 115200\r\n" );
	}
#endif  /* GSM_MODEM_DEBUG */
	
		vehicleParams.gsmBaudrate = USART_BaudRate_115200bps;
//		STORAGE_WriteData ( VEHICLE_PARAMS, sizeof( vechile_Params_t ), ( u8 * )&vehicleParams );
	  }
   
	  /* 4.重新启动通信端口，并尝试与MODEM交互 */
	  if ( TRUE == USART_IOCtl ( USART_FCM_MODEM,
								 USART_InitType_BaudRate, vehicleParams.gsmBaudrate,
								 USART_InitType_None ) )
	  {
		USARTx_Start ( USART_FCM_MODEM, MODEM_ReadData );
		GSM_xx.status = MDM_SERIAL_INIT;
		MDM_PostAliveTest ( FALSE );  
	  }
}






extern bool GL868_WriteData ( u8 FlowType, u8 Data[], u16 DataSize )
{	
	
  if ( ( vehicleParams.DebugCtroller.debugType & Debug_Type_MDM ) == 0 )
  {
    if ( Data && DataSize > 0 )
    {
    	switch ( FlowType )
    	{
    		case AT_Flow_Type:  /* AT指令流 */
  				if ( MDM_QueryFlowStatus () == MDM_FCM_V24_STATE_DATA ) 
  				{
            /* 数据通信模式下，收到上层应用传递的AT指令;
             * 丢弃当前的指令数据；
             * 警告：该情况会导致上层的部分功能丢失；
             **/
  					
            return FALSE;
  				}  			
    			break;
    		
    		case DATA_Flow_Type:  /* 数据流 */
  				if ( MDM_QueryFlowStatus () != MDM_FCM_V24_STATE_DATA )
  				{
            /* 非数据通信模式下，收到上层应用传递的数据信息;
             * 丢弃当前的数据信息；
             * 警告：该情况会导致上层发送的部分数据丢失；
             **/ 					
            return FALSE;	
  				}
    			break;
    		
    		default:
    			return FALSE;	
    	}
         
      return USART_WriteData ( USART_FCM_MODEM, Data, DataSize );
    }    
  }
  return FALSE;

}








/* 
 * 功能描述: GSM_xx读取数据
 * 引用参数: (1)数据长度
 *           (2)数据内容          
 *
 * 返回值  : 状态码
 * 
 */


extern bool GL868_ReadData ( u16 DataSize, u8 *Data )
{  
    u8 flowType = MDM_QueryFlowStatus ();
	  if ( flowType == MDM_FCM_V24_STATE_DATA )
	  {
				if(sys_status<PROC_SYSTEM_SLEEP)
				{
						/* 数据通信状态下是否丢失连接?? */
						if ( strstr ( ( const char * )Data, "NO CARRIER" ) )
						{
								if(sys_status==PROC_SYSTEM_UPDATE)//固件升级时，断网直接重启
								{	
										printf ( "\r\n[SYS RESET]: NO CARRIER --> PROC_SYSTEM_UPDATE TYPE  \r\n" );
										System_Reset();
								}
								else
								{
										#if SYSTEM_RESTART_DEBUG == 1
												printf ( "\r\n[SYS RESET]:MDM_FCM_V24_STATE_DATA --> NO CARRIER Type\r\n" );
										#endif  /* SYSTEM_RESTART_DEBUG */
										/* 1.关闭modem*/
										Modem_Stop(PROC_SYSTEM_REISSUEDATA_STORAGE);
										/* 2.重启modem*/
									  TMR_Subscribe ( FALSE, 20, TMR_TYPE_100MS, ( tmr_procTriggerHdlr_t )Modem_Restart );	
								}														
						}
						else
						{
								SIO_Read ( DataSize, Data );
						}
				}
	  }
    else
    {
//////////////////////////////////////////////////		
#if  AT_DISPLAY_DEBUG ==1 
				for(u32 i=0;i<DataSize;i++)
				{
					printf("%c",Data[i]);
				}
#endif
//////////////////////////////////////////////////
				if(sys_status<PROC_SYSTEM_SLEEP)
				{
						if ( strstr ( ( const char * )Data, "NO CARRIER" ))
						{
								#if SYSTEM_RESTART_DEBUG == 1
										printf ( "\r\n[SYS RESET]:MDM_FCM_V24_STATE_SW --> NO CARRIER Type\r\n" );
								#endif  /* SYSTEM_RESTART_DEBUG */
								/* 1.关闭modem*/
								Modem_Stop(PROC_SYSTEM_REISSUEDATA_STORAGE);
								/* 2.重启modem*/
								TMR_Subscribe ( FALSE, 20, TMR_TYPE_100MS, ( tmr_procTriggerHdlr_t )Modem_Restart );	
						}
				}
				/* ADD #001 增加切换状态下的用户数据读取机制
				 * 切换状态下优先MODEM数据读取:
				 * (1)若MODEM数据解析结果为肯定确认时，则是用户数据的可能性较小;
				 * (2)反之，则可能是用户数据. */
				if ( MDM_ReadData ( DataSize, Data ) == FALSE && flowType == MDM_FCM_V24_STATE_SW )
				{
						SIO_Read ( DataSize, Data ); 
				}
    }
		return TRUE;
}





static bool GL868_FlowCtrlHandler ( mdm_fcmEvent_e Event )
{
		switch ( Event )
		{
		/* 流打开事件 */
		case MDM_FCM_EVENT_FLOW_OPENNED:  
#if GSM_MODEM_DEBUG == 1
	{
		  PRINTF_GPS_TIME ();
		  printf ( "\r\n[GSM_xx FLW] modem flow is open\r\n" );
	}
#endif  /* GSM_MODEM_DEBUG */
	
		  /* 默认模块状态为AT指令通信 */
		  MDM_FlowSwitchV24State ( MDM_FCM_V24_STATE_AT );
		  break;
	
		/* 流AT模式切换 */
		case MDM_FCM_EVENT_V24_AT_MODE_FROM_CALL: 
		  /* 设置超时间隔: 10x */
		  USART_SetRxTimeoutByNms ( USART_FCM_MODEM, 10 );
		  break;
	
		/* 流进入AT模式 */
		case MDM_FCM_EVENT_V24_AT_MODE:
#if GSM_MODEM_DEBUG == 1
	{
		  PRINTF_GPS_TIME ();
		  printf ( "\r\n[GSM_xx FLW] modem is at mode\r\n" );
	}
#endif  /* GSM_MODEM_DEBUG */
		  break;
	
		/* 流数据模式切换 */
		case MDM_FCM_EVENT_V24_DATA_MODE_FROM_CALL:
		  break;
	
		/* 流进入数据模式 */
		case MDM_FCM_EVENT_V24_DATA_MODE:
#if 1	
		
#if GSM_MODEM_DEBUG == 1
		{
		  PRINTF_GPS_TIME ();
		  printf ( "\r\n[MC52I FLW] modem is data mode\r\n" );
		}
#endif  /* GSM_MODEM_DEBUG */
	
		  /* 设置超时间隔: 4x */
		  USART_SetRxTimeoutByNms ( USART_FCM_MODEM, 4 );
	
		  /* 打开SIO接口 */
			sio_open();
#endif	  
		  break;
	
		/* 流关闭事件 */
		case MDM_FCM_EVENT_FLOW_CLOSED:
#if 1		
#if GSM_MODEM_DEBUG == 1
	{
		  PRINTF_GPS_TIME ();
		  printf ( "\r\n[MC52I FLW] modem flow is close\r\n" );
	}
#endif  /* GSM_MODEM_DEBUG */
	
		  SIO_Close ();
#endif
		  break;
	
		/* MODEM切换超时 */
		case MDM_FCM_EVENT_SWITCH_TO:
		  MDM_SWITCH_TO = TRUE;
		  break;
	
		/* MODEM切换失败或拨号失败******* */
		case MDM_FCM_EVENT_SWITCH_ERR:
		case MDM_FCM_EVENT_DIAL_ERR:
		  //sysParamsSave ();
		  break;
		}
		return TRUE;
}






/* 
 * 功能描述：GSM_xx接口初始化
 * 引用参数：无
 *          
 * 返回值  ：无
 * 
 */
 u8 gl868_init_flag = 0;
static void Gl868_IfInit ( u8 ID )
{
		
	  if ( GSM_xx.serial )
	  {
				GSM_xx.serial->write_data = MODEM_WriteData;
				GSM_xx.serial->read_data  = MODEM_ReadData;
				GSM_xx.serial->no_response_error = GL868_NoRspErrorHandler;
				GSM_xx.status = MDM_SERIAL_INIT;
				MDM_FlowServiceStart ( GL868_FlowCtrlHandler );
	  }
		if(GSM_xx.call)
		{
			  GSM_xx.call->set_ri_interrupt=GSM_Call_handler_SetCallRing;
		
		}
		if ( GSM_xx.pdp )
		{
				GSM_xx.pdp->request_gprs_service_error =pdp_request_gprs_service_error;
		}
	
	  if ( GSM_xx.network )
	  {
				GSM_xx.network->query_signal_strength_reply = network_query_signal_strength;
				GSM_xx.network->status = network_get_current_status;
	  }  
	
	  if ( GSM_xx.sim )
	  {
				GSM_xx.sim->get_sim_info_reply = sim_get_imsi;
				GSM_xx.sim->list_preferred_message_reply = sim_read_all_message_reply;
				GSM_xx.sim->set_sim_own_number_error = sim_set_sim_own_number_error;
				GSM_xx.sim->auth_status = sim_get_current_status;
				GSM_xx.sim->incoming_message = sim_incoming_message;
	  }
		
	  GSM_xx.sim_status = SIM_UNKNOWN;	  /* 状态切换 */
	  GSM_xx.is_timeout = FALSE;
	  MDM_IfInit ( ( MDM_DevTypedef * )&GSM_xx );
	  MDM_PostAliveTest ( FALSE );
	 
}



/* 
 * 功能描述：时钟响应
 * 引用参数：无
 *          
 * 返回值  ：状态码
 * 
 */
extern bool RING_Handler ( u8 ID )
{
  /* 定义振铃脚低电平信号时间 */
  return FALSE;
}



#endif  /* __USE_MODEM_GL868__ */



