







#ifndef __MODEM_H__
#define __MODEM_H__



#include "stm32f4xx.h"

#include "lgsm.h"
#include "serial.h"
#include "call_interface.h"
#include "device_interface.h"
#include "network_interface.h"
#include "sim_interface.h"
#include "sms_interface.h"
#include "pdp_interface.h"
#include "vendor_interface.h"
#include "tiza_include.h"














/******************************************/
/*              MDM����[����]             */
/******************************************/

typedef u8 MDM_RSP_ID;


/* Modem's status */
typedef enum {

	/* Modem is uninitialized
	 * Might also have once worked but timed out on AT command */
	MDM_UNINITIALIZED = 0,
	
	/* Modem's serial ports are initialized
	 * Waiting for initializing at commands to finish. */
	MDM_SERIAL_INIT,
	
	/* Modem is currently being initialized */
	MDM_INITIALIZING,
	
	/* Modem is ready to receive commands */
	MDM_READY,
	
	/* Modem is busy, and can't receive more commands */
	MDM_IS_BUSY,
	
	/* Modem should be resetted */
	MDM_RESET
} MDM_Status;




/**
 * @brief Symbol table struct, it is used by Scanner for token analysis
 */
typedef struct  
{
  ascii *symbol_name;
  u32   symbol_token;
} SymbolTable;

extern SymbolTable symbols[], *symbol_p;




typedef struct MDM_CMD_STRUCTER   MDM_CmdTypedef;
typedef struct MDM_RSP_STRUCTER   MDM_RspTypedef;


typedef enum 
{
	/* Completed successfully */
  AT_HANDLER_DONE = 0,   
  
  /* Completed with an error from modem */
  AT_HANDLER_DONE_ERROR,
  
  /* Need more input(response) from modem */
  AT_HANDLER_NEED_MORE,
  
  /* Handler doesn't understand the input */
  AT_HANDLER_DONT_UNDERSTAND,
  
  /* Retry needed */
  AT_HANDLER_RETRY,           
  
  /* Unexpected error */
  AT_HANDLER_ERROR            
} AtCmdHandler_Status;


/* AtCommand handler function definition */
typedef AtCmdHandler_Status ( *AtCmdHandler ) ( MDM_CmdTypedef *at, MDM_RspTypedef *atrsp );





struct MDM_CMD_STRUCTER
{
	/* AtCommand id */
	u32 cmd_id;
	
	/* AtCommand buffer */
	ascii *AtBuffer;
	
	/* AtCommand timeout in milliseconds */
	u32 timeout;

	/* default retry count for the command */
	s32 retry_counter;

	/* AT command response handle function */
	AtCmdHandler handler;
	
	/* Minimum sim state required for sending this command */
	SIM_Status required_state;

  /* Marked true when write is completed */
  bool write_complete; 
} ;	





struct MDM_RSP_STRUCTER
{
  /* AtCommand has been finished */
	bool finish;

  /* standard AtRsponse message */
	s32 Code;

  /* revice AtRsponse message length */
	u32 totalsize;

  /* AtRsponse buffer base pointer */
  ascii *indicator;
} ;





typedef struct {
	
  /* time out flag */
  bool is_timeout;
  
  /* SerialPlugin interface */
  Serial_IfTypedef *serial;
  
  /* voice call interface */
  Call_IfTypedef *call;

  /* sms interface */
  SMS_IfTypedef *sms;

  /* Device interface */
  Device_IfTypedef *device;

  /* Network interface */
  Network_IfTypedef *network;

  /* SIM interface */
  SIM_IfTypedef *sim;

  /* PDP Context interface */
  PDP_IfTypedef *pdp;

  /* NEW #001 ��������ר��ָ��ӿ� */
  /* Vendor interface */
  Vendor_IfTypedef *vendor;

  /* Pointer to inter-process communication data */
  void *ipc_data;

  /* Does sim card need pin code before being able to send commands */
  SIM_Status sim_status;

  /* Handle to sms database */
  void* sms_handle;

  /* Is modem alive (responses to AT command) */
  MDM_Status status;
} MDM_DevTypedef;







/******************************************/
/*            ����������[����]            */
/******************************************/

/* V24 State */
#define MDM_FCM_V24_STATE_AT    0   /* AT״̬ */
#define MDM_FCM_V24_STATE_DATA  1   /* ����״̬ */
#define MDM_FCM_V24_STATE_SW    2   /* �л�״̬ */


/* FCM Events */
typedef enum
{
  MDM_FCM_EVENT_FLOW_OPENNED,   /* �����¼� */
  MDM_FCM_EVENT_FLOW_CLOSED,    /* ���ر��¼� */
  MDM_FCM_EVENT_V24_DATA_MODE,  /* ����������ģʽ */
  MDM_FCM_EVENT_V24_AT_MODE,    /* ������ATģʽ */
  MDM_FCM_EVENT_RESUME,         /* �������¼� */
  MDM_FCM_EVENT_V24_DATA_MODE_FROM_CALL,  /* ������ģʽ�л� */
  MDM_FCM_EVENT_V24_AT_MODE_FROM_CALL,    /* ��ATģʽ�л� */

  /* !!!�����¼�Ϊ�쳣��ʹ�÷���ʱ��Ҫע�⴦��!!! */
  MDM_FCM_EVENT_SWITCH_TO,      /* ���л���ʱ�¼� */
  MDM_FCM_EVENT_SWITCH_ERR,     /* ���л�ʧ���¼� */
  MDM_FCM_EVENT_DIAL_ERR        /* ����ʧ���¼� */
} mdm_fcmEvent_e;


/* FCM control events handler */
typedef bool ( *mdm_fcmCtrlHdlr_f ) ( mdm_fcmEvent_e Event );


/******************************************/
/*              �ⲿ����[����]            */
/******************************************/

extern void MDM_FlowServiceStart ( mdm_fcmCtrlHdlr_f CtrlHandler );
extern void MDM_FlowServiceStop ( void );
extern void MDM_FlowSwitchV24State ( u8 State );
extern u8   MDM_QueryFlowStatus ( void );
extern void MDM_FlowServiceException ( u8 State );








/******************************************/
/*          ״̬�л�����[����]            */
/******************************************/

/* �����л����� */
#define MDM_Switch_Type_Unknow									0x00    /* Ĭ���� */
#define MDM_Switch_Type_Calling         				0x01    /* �������� */
#define MDM_Switch_Type_SMS_Incoming    				0x02    /* SMS���� */
#define MDM_Switch_Type_Active									0x04    /* Ӧ���������� */
#define MDM_Switch_Type_M2MLocate               0x05    /*��վ��λ����*/





/******************************************/
/*              �ⲿ����[����]            */
/******************************************/

extern void MDM_SwitchResourceRelease ( void );
extern bool MDM_SwitchToDataStatus ( void );
extern bool MDM_SwitchToATStatus ( u8 switchType );
extern void MDM_SwitchInfoStatistics ( void );
extern void MDM_SwitchStartWatch ( void );








/******************************************/
/*          �����ⲿ����[����]            */
/******************************************/

extern void MDM_IfInit ( MDM_DevTypedef *MDM );
extern void MDM_Reset ( void );
extern void MDM_ChangeNetworkStatus ( NetworkStatus status );   
extern void MDM_ChangeSIMstatus ( SIM_Status status );
extern void MDM_PostAliveTest ( bool reset ); 
extern s32  MDM_QueryCurrentRetryCounter ( void );                 
extern bool MDM_PostAtCmdByID ( MDM_CmdTypedef *command, AtCommandID id, const ascii* params );
extern bool MDM_Utils_ParseCMSerror ( MDM_RspTypedef *response, s32 *code );
extern bool MDM_Utils_ParseCMEerror ( MDM_RspTypedef *response, s32 *code );
extern MDM_RSP_ID MDM_GetResultCode ( MDM_RspTypedef *response );




/******************************************/
/*              ���ߺ���[����]            */
/******************************************/

extern ascii* MDM_StrTok ( MDM_RspTypedef *response, ascii *key );
extern ascii* MDM_StrStr ( MDM_RspTypedef *response, ascii *key );
extern ascii* MDM_Sym2Str( const SymbolTable *symbol, s32 token );
extern u32 MDM_String2Bytes ( const u8* Src, u8* Dst, u16 SrcLength );
extern u32 MDM_Bytes2String ( u8* Dst, const u8* Src, u16 SrcLength );




/******************************************/
/*              �ⲿ����[����]            */
/******************************************/

extern bool MDM_ReadData ( u16 DataSize, u8 *Data );






#endif	/* __MODEM_H__ */

