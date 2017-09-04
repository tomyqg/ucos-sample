






#ifndef __TZ_PARAMETERS_H__
#define __TZ_PARAMETERS_H__





#include "stm32f4xx.h"

//#include "tz_common.h"
#include "stdbool.h"
#include "tiza_defined.h"
#include "defines.h"






/******************************************/
/*              ȫ�ֱ���[����]            */
/******************************************/


/************************************************ 
 * ע  �⣺                                     *
 * (1)�����ֶ���Ĳ����洢��FRAM���棻          *
 * (2)�洢�����ID�������ó���8��(0~7);         *
 * (3)ÿ������Ĵ洢�ߴ���С��512bytes!!        *
 ************************************************/

enum   
{
  /* ϵͳ���� */
  VEHICLE_PARAMS = 0,

  /* GPRSͨ�Ų��� */
  GPRS_PARAMS,

  /* �����Чλ�� */
  LAST_POSITION,

  /* ACC��Ϣ */
  ACC_INFO,
  
  /* ACC����ʱ��ͳ�� */
  ACC_WORK_STATS,

  /* ���������� */
  CONTROLLER_DATA,

  /* ���������� */
  STORE_DATA_COMPENSATOR,
  /* 32960 ��������*/
  PARAMETER_DATA,

  /* ��С�Ѿ��޶������������޸ģ� */
  GPS_OBJ_NB = 8
};








/////////////////////////////////////////////////////////////////////////////////////

/* ����GPSѡ�� */
#define Debug_Type_GPS            0x0001 

/* ����GSMѡ�� */
#define Debug_Type_GSM            0x0002

/* ����FTPѡ�� */ 
#define Debug_Type_FTP            0x0004

/* ����CANѡ�� */
#define Debug_Type_CAN            0x0008

/* ����SMSѡ�� */
#define Debug_Type_SMS            0x0010

/* ����MODEMѡ�� */
#define Debug_Type_MDM            0x0020

/* ���Դ���ѡ�� */
#define Debug_Type_COM4           0x0100

typedef struct
{
  /* ���Զ˿� */
  u8 debugPort;	
	
  /* ����״̬ */
  bool debugIsValid;

  /* ����ѡ�� */
  u32 debugType;
} sys_debug_t;

/////////////////////////////////////////////////////////////////////////////////////




/* RMC�ṹ */
typedef struct  {

  /* ʱ��_ʱ */ 
  u32		time_hh;		
  
  /* ʱ��_�� */
  u32		time_mm;		
  
  /* ʱ��_�� */
  u32		time_ss;		
    
  /* ����_�� */
	u32		date_dd;	
    	
  /* ����_�� */
	u32		date_mm;	
    	
  /* ����_�� */
	u32		date_yy;	
       
} GPS_DATA;



/* ������Ϣ */
typedef struct
{

 /* ����������gl     */
	GPS_DATA GPS_d;
/***********************************************************/
  /*�ն˵�¼��ˮ��*/
  u16 LoginSerial;
	
  /* ����VIN */
  ascii CarVIN[ 17 ];
	
  /* ����VIN���ñ�־ */
  bool CarVINSet;
	
	/*DCDC ��������*/
	/* TRUE : ����� FALSE :����� */
//	bool IsDCDC_PWR;
/***********************************************************/
	
  /* ���Կڲ�����*/
  u32 debugBaudrate;

  /* GPS������ */
  u32 gpsBaudrate;

  /* GSM������ */
  u32 gsmBaudrate;
      
  /* ���Կ����� */
  sys_debug_t DebugCtroller;

  /* SIM���� */
  ascii sim_no[ __SIM_NUMBER_LENGTH__ ];

  /* IMSI�� */ 
  ascii imsi_code[ __IMSICODE_LENGTH__ ];

  /* ����ID */
  ascii factoryID[ __FACTORY_ID_SIZE__ ];

  /* ͨ���жϱ���ʱ�� */
  u16 controllerInterruptAlarmCount;

  /* ��������ʡ��ʱ�� */
  u16 sleepTime;

  /* �Ƿ�˯�ߣ� */
  u32 sleepMode;
#define SLEEP_STATUS_IS_SET_BIT             0x01      /* ʹ�����߹��� */


  /* �����Ƿ�ɹ��� */
  u8 upgrade;
#define UPGRADE_STATUS_IS_UPGRADING_BIT     0x01      /* ��������״̬ */
#define UPGRADE_STATUS_IS_OK_BIT            0x02      /* �����ɹ� */


#if TZ_DISTANCE_STATISTICS == 1
  /* ����ۻ� */
  u32 totalMoveDistance;
#endif  /* TZ_DISTANCE_STATISTICS */


u32 servercheck;


} vechile_Params_t;


/* �������� */
extern vechile_Params_t vehicleParams;


typedef struct {
    u8 bkp_no_ready;      /*BKP reg ��д��־      1: �޷�д�� 0:����д��  */
    u8 gps_position_flag; /*��λ��־     1: ��λ  0:����λ*/
    u8 ftp_no_reset;      /*ftp ��λ��־ 1: ��������FTP ��������λ 0: û�н���FTP */
    u8 gprs_ready;        /*gprs ʹ�ܱ�־ 1:gprs ����  0: gprs δ����*/
    u8 ack_flags;         /*��¼Ӧ���־1:�е�¼Ӧ�� 0:û�е�¼Ӧ��*/
    u8 wake_time_zero_flag; /*����ʱ����Ϊ0��־ 1: ����ʱ����Ϊ0     0: ����ʱ������Ϊ0*/
    u8 config_time;       /*����ʱ����Ϣ��־0:δ���� 1:�Ѿ�����*/
    u8 sms_flags;         /*SMS ���ñ�־ 1:������ 0:δ����(����Զ������)*/
		u8 gsm_ver[32];
    u8 ftp_buf_len;
    u8 ftp_buf[256];
	u8 ccid[20];
}status_param;

extern status_param drv_param;















/******************************************/
/*              �ⲿ����[����]            */
/******************************************/
extern void ParameterQuery_Init(void);
extern void sysParamsInit ( void );
extern void sysParamsSave ( void );
extern void sysParamsReset ( void );

extern void sysResetState ( void );
extern void sysResetGprs ( void );
extern void sysResetDebug ( void );
extern void sysResetVehicle ( void );
extern void sysResetAccWorkPara ( void );
extern void sysResetControllerPara ( void );
extern void sysResetAccWorkStatsInfo ( void );


#endif  /* __TZ_PARAMETERS_H__ */


