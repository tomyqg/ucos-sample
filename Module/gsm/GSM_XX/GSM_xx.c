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
/*              �ڲ�����[����]            */
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


/* ������� */
s32 ring_io_handler  = -1;

/* GSM��Դ���ƽ� */
s32 gsm_io_handler   = -1;

/* IGT�źſ��ƽ� */
s32 igt_io_handler   = -1;

/* �л���ʱ��־ */
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
/*              �ڲ�����[����]            */
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
 * ����������SIM6320C�ػ�
 * ���ò�������
 *          
 * ����ֵ  ����
 * 
 */
void SIM6320C_CLOSE(void)
{
		TM_GPIO_SetPinHigh(GPIOC,GPIO_Pin_0);  
}

/* 
 * ����������GSM_xx�ػ�
 * ���ò�������
 *          
 * ����ֵ  ����
 * 
 */
extern void GL868_SwitchOFF(void)
{
		/* GSM��Դ����͵�ƽ */
	TM_GPIO_SetPinHigh(GPIOC,GPIO_Pin_0);
	TMR_Subscribe( FALSE, 30, TMR_TYPE_100MS, ( tmr_procTriggerHdlr_t )SIM6320C_CLOSE );
}

/* 
 * ����������SIM6320C����
 * ���ò�������
 *          
 * ����ֵ  ����
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
 * ����������GSM_xx�ϵ翪��
 * ���ò�������
 *          
 * ����ֵ  ����
 * 
 */
extern void GL868_PowerSwitchON(void)
{
  /* 1.ע��GSM��Դ���ƽ� */
	//��ʼGMS��������
	TM_GPIO_Init(GPIOC,GPIO_Pin_0,TM_GPIO_Mode_OUT,TM_GPIO_OType_PP,TM_GPIO_PuPd_NOPULL,TM_GPIO_Speed_Fast);
	/* GSM��Դ����͵�ƽ */
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
 * ����������GSM_xx ��վ��λ
 * ���ò�����u8   act_flag     PDP deactivate ��־
 *          
 * ����ֵ  ����
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
 * ����������GSM_xx�ϵ�ػ�
 * ���ò�������
 *          
 * ����ֵ  ����
 * 
 */
extern void GL868_PowerSwitchOFF ( void )
{
  /* GSM��Դ����͵�ƽ */
  if ( gsm_io_handler > 0 )
  {

  }
}

static void GL868_NoRspErrorHandler ( void* ipc_data )
{
	  /************************************************************
	   *Ŀǰ����Ӧ����������Ҫ���£�
	   * (1)MODEMͨ�Ų����ʲ�һ��;					   [�Ѵ���]
	   * (2)MODEM����ʧ�ܣ�����ֹͣ״̬��			   [δ����]
	   * (3)MODEM��������ͨ��״̬��ATָ������Ӧ��	   [�Ѵ���]
	   * (4)����....
	   ************************************************************/
	
#if GSM_MODEM_DEBUG == 1
	{
	  PRINTF_GPS_TIME ();
	  printf ( "\r\n[GSM_xx ERR] modem isn't response\r\n" );
	}
#endif  /* GSM_MODEM_DEBUG */
	
	  /* 1.MODEM��λ */
	  MDM_Reset ();
	
	  /* 2.�ر�ͨ�Ŷ˿� */
	  USART_Stop(USART_FCM_MODEM );
	
	
	  /* 3.����ͨ�Ų����� */
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
   
	  /* 4.��������ͨ�Ŷ˿ڣ���������MODEM���� */
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
    		case AT_Flow_Type:  /* ATָ���� */
  				if ( MDM_QueryFlowStatus () == MDM_FCM_V24_STATE_DATA ) 
  				{
            /* ����ͨ��ģʽ�£��յ��ϲ�Ӧ�ô��ݵ�ATָ��;
             * ������ǰ��ָ�����ݣ�
             * ���棺������ᵼ���ϲ�Ĳ��ֹ��ܶ�ʧ��
             **/
  					
            return FALSE;
  				}  			
    			break;
    		
    		case DATA_Flow_Type:  /* ������ */
  				if ( MDM_QueryFlowStatus () != MDM_FCM_V24_STATE_DATA )
  				{
            /* ������ͨ��ģʽ�£��յ��ϲ�Ӧ�ô��ݵ�������Ϣ;
             * ������ǰ��������Ϣ��
             * ���棺������ᵼ���ϲ㷢�͵Ĳ������ݶ�ʧ��
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
 * ��������: GSM_xx��ȡ����
 * ���ò���: (1)���ݳ���
 *           (2)��������          
 *
 * ����ֵ  : ״̬��
 * 
 */


extern bool GL868_ReadData ( u16 DataSize, u8 *Data )
{  
    u8 flowType = MDM_QueryFlowStatus ();
	  if ( flowType == MDM_FCM_V24_STATE_DATA )
	  {
				if(sys_status<PROC_SYSTEM_SLEEP)
				{
						/* ����ͨ��״̬���Ƿ�ʧ����?? */
						if ( strstr ( ( const char * )Data, "NO CARRIER" ) )
						{
								if(sys_status==PROC_SYSTEM_UPDATE)//�̼�����ʱ������ֱ������
								{	
										printf ( "\r\n[SYS RESET]: NO CARRIER --> PROC_SYSTEM_UPDATE TYPE  \r\n" );
										System_Reset();
								}
								else
								{
										#if SYSTEM_RESTART_DEBUG == 1
												printf ( "\r\n[SYS RESET]:MDM_FCM_V24_STATE_DATA --> NO CARRIER Type\r\n" );
										#endif  /* SYSTEM_RESTART_DEBUG */
										/* 1.�ر�modem*/
										Modem_Stop(PROC_SYSTEM_REISSUEDATA_STORAGE);
										/* 2.����modem*/
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
								/* 1.�ر�modem*/
								Modem_Stop(PROC_SYSTEM_REISSUEDATA_STORAGE);
								/* 2.����modem*/
								TMR_Subscribe ( FALSE, 20, TMR_TYPE_100MS, ( tmr_procTriggerHdlr_t )Modem_Restart );	
						}
				}
				/* ADD #001 �����л�״̬�µ��û����ݶ�ȡ����
				 * �л�״̬������MODEM���ݶ�ȡ:
				 * (1)��MODEM���ݽ������Ϊ�϶�ȷ��ʱ�������û����ݵĿ����Խ�С;
				 * (2)��֮����������û�����. */
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
		/* �����¼� */
		case MDM_FCM_EVENT_FLOW_OPENNED:  
#if GSM_MODEM_DEBUG == 1
	{
		  PRINTF_GPS_TIME ();
		  printf ( "\r\n[GSM_xx FLW] modem flow is open\r\n" );
	}
#endif  /* GSM_MODEM_DEBUG */
	
		  /* Ĭ��ģ��״̬ΪATָ��ͨ�� */
		  MDM_FlowSwitchV24State ( MDM_FCM_V24_STATE_AT );
		  break;
	
		/* ��ATģʽ�л� */
		case MDM_FCM_EVENT_V24_AT_MODE_FROM_CALL: 
		  /* ���ó�ʱ���: 10x */
		  USART_SetRxTimeoutByNms ( USART_FCM_MODEM, 10 );
		  break;
	
		/* ������ATģʽ */
		case MDM_FCM_EVENT_V24_AT_MODE:
#if GSM_MODEM_DEBUG == 1
	{
		  PRINTF_GPS_TIME ();
		  printf ( "\r\n[GSM_xx FLW] modem is at mode\r\n" );
	}
#endif  /* GSM_MODEM_DEBUG */
		  break;
	
		/* ������ģʽ�л� */
		case MDM_FCM_EVENT_V24_DATA_MODE_FROM_CALL:
		  break;
	
		/* ����������ģʽ */
		case MDM_FCM_EVENT_V24_DATA_MODE:
#if 1	
		
#if GSM_MODEM_DEBUG == 1
		{
		  PRINTF_GPS_TIME ();
		  printf ( "\r\n[MC52I FLW] modem is data mode\r\n" );
		}
#endif  /* GSM_MODEM_DEBUG */
	
		  /* ���ó�ʱ���: 4x */
		  USART_SetRxTimeoutByNms ( USART_FCM_MODEM, 4 );
	
		  /* ��SIO�ӿ� */
			sio_open();
#endif	  
		  break;
	
		/* ���ر��¼� */
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
	
		/* MODEM�л���ʱ */
		case MDM_FCM_EVENT_SWITCH_TO:
		  MDM_SWITCH_TO = TRUE;
		  break;
	
		/* MODEM�л�ʧ�ܻ򲦺�ʧ��******* */
		case MDM_FCM_EVENT_SWITCH_ERR:
		case MDM_FCM_EVENT_DIAL_ERR:
		  //sysParamsSave ();
		  break;
		}
		return TRUE;
}






/* 
 * ����������GSM_xx�ӿڳ�ʼ��
 * ���ò�������
 *          
 * ����ֵ  ����
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
		
	  GSM_xx.sim_status = SIM_UNKNOWN;	  /* ״̬�л� */
	  GSM_xx.is_timeout = FALSE;
	  MDM_IfInit ( ( MDM_DevTypedef * )&GSM_xx );
	  MDM_PostAliveTest ( FALSE );
	 
}



/* 
 * ����������ʱ����Ӧ
 * ���ò�������
 *          
 * ����ֵ  ��״̬��
 * 
 */
extern bool RING_Handler ( u8 ID )
{
  /* ��������ŵ͵�ƽ�ź�ʱ�� */
  return FALSE;
}



#endif  /* __USE_MODEM_GL868__ */



