



#ifndef __TIZA_GPRS_PROTOCOL_H__
#define __TIZA_GPRS_PROTOCOL_H__





#include "stm32f4xx.h"
#include "tiza_defined.h"
#include "stdbool.h"




extern int Diag_can_flag;
extern int Diag_can_state;
extern int test_diag;

extern s8  serversocket;

/************/
/* GPRS���� */
/************/

/* �������й��� */
typedef enum
{
  SYSTEM_PROC_BEGIN = -1,

	/* ��ʼ�� */
  PROC_SYSTEM_INIT,

	/* ��¼ */
  PROC_SYSTEM_LOGIN,     

  /* ��½�ɹ� */
  PROC_SYSTEM_LOGIN_ACK,
	
	/* ʵʱ�����ϴ� */
  PROC_SYSTEM_REALDATA_UPLOAD,
	
	/* ʵʱ����Ӧ�� */
  PROC_SYSTEM_REALDATA_ACK,
	
	/*���������ϴ�(ä��)*/
	PROC_SYSTEM_REISSUEDATA_UPLOAD,

	/* ���� */
  PROC_SYSTEM_HEART,
	
	/* ���� */
  PROC_SYSTEM_ALARM,
	
//	/* �ǳ� */
//  PROC_SYSTEM_LOGOUT,

	/* ��Ӧ */
  PROC_SYSTEM_RESPONSE,

	/* ϵͳӦ�ò������û��ѯ */
  PROC_SYSTEM_SYSPARAMS,

	/* �ǳ� */
  PROC_SYSTEM_LOGOUT,
	
	/* �̼����� */
  PROC_SYSTEM_UPDATE,
	
	/* ���� */
  PROC_SYSTEM_SLEEP,

	/*ä�����ݴ洢*/
	PROC_SYSTEM_REISSUEDATA_STORAGE,
	
	/* �ǳ�Ӧ�� */
	PROC_SYSTEM_LOGOUT_ACK,
	
	/* ��������*/
	PROC_SYSTEM_DEBUG_CUT_NETWORK,

} sys_status_t;


/* ��������״̬ */
extern sys_status_t sys_status;





/* GPRS���� */
typedef struct
{
  /* �豸ID���ñ�־ */
  bool DeviceIDSet; 
  
  /* APN */
  ascii APN[ __APN_LENGTH__ ];

  /* IP1 */
  ascii IP1[ __SERVER_LENGTH__ ];						
  ascii ip1_domain[ __DOMAIN_LENGTH__ ];

  /* IP2 */
  ascii IP2[ __SERVER_LENGTH__ ];
  ascii ip2_domain[ __DOMAIN_LENGTH__ ];

  /* �ն�ID */
  ascii devID[ 17 ];

  /* Э��汾�� */
  u8 ver;

  /* �˿� */
  u16 Port1;
  u16 Port2;

  /* ������� */
  u16 check;
  u16 heart;
  u16 noheart;

  /* ����״̬(��) */
  u32 net_is_ok;

  /* ����״̬(��) */
  u32 net_is_bad;
	
/******************************************************************/	
//GPS�������
	  /* γ�� */
  ascii lat[ __LAT_LON_LENGTH__ ];
  
  /* ���� */
  ascii lon[ __LAT_LON_LENGTH__ ];
	/*γ��*/
	u32 lat_;
	/*����*/
	u32 lon_;
  
  /* �ٶ� */
  ascii speed;
  
  /* ���� */
  ascii dir;
  
  /* ״̬λ */
  ascii state[ __STATUS_BIT_LENGTH__ ];  

  u16 diy_can_baut;	//������

  u16 diy_can_type;	//֡����

  u16 diy_can_factry; //���ұ��

  u8  diy_can_nub;//һ���ж���֡

	u32 diy_can_data[ 20 ];
	
}gprs_Params_t;

/* GPRS���� */
extern gprs_Params_t gprsParams;



typedef __packed struct
{ 
  /* ���� �� */
  u32  date_yy;
  /* ���� �� */
  u32  date_mm;
  /* ���� �� */
  u32  date_dd;
	/* ʱ��_ʱ */ 
  u32		time_hh;		
  
  /* ʱ��_�� */
  u32		time_mm;		
  
  /* ʱ��_�� */
  u32		time_ss;
	
	/* ��λ״̬ */
	u8	status_;
	
	/* γ�� */
  ascii lat[ 4 ];
  
  /* ���� */
  ascii lon[ 4 ];	
		
}location_t;

extern location_t locations;




/* ����״̬ */
extern bool dial_OK;






/* ���ķ�������־ */
extern u32 centerSrvStatus;
#define CENTER_SERVER1_LOGIN_BIT            0x00    /* ��¼������1 */
#define CENTER_SERVER2_LOGIN_BIT            0x01    /* ��¼������2 */
#define CENTER_SERVERX_IS_LOGIN_BIT         0x80    /* �ѵ�¼������X */
#define CENTER_SERVERX_RSP_IS_TIMEOUT_BIT   0x10    /* ��������Ӧ��ʱ */

/* �������� */
#define TZ_ALARM_TYPE_OVERSPEED					0x01  /* ���ٱ��� */
#define TZ_ALARM_TYPE_GPS_ANTENNA				0x02  /* GPS����δ�ӻ���ϱ��� */
#define TZ_ALARM_TYPE_GPS_MOUDLE				0x03  /* GPSģ����ϱ��� */
#define TZ_ALARM_TYPE_LOW_VOLTAGE				0x04  /* �͵�ѹ���� */
#define TZ_ALARM_TYPE_DATA_COM_ABORT		0x05  /* ����ͨѶ�жϱ��� */
#define TZ_ALARM_TYPE_POWER_OFF					0x06  /* �ϵ籨�� */
#define TZ_ALARM_TYPE_TAIL							0x07  /* �ϳ����� */
#define TZ_ALARM_TYPE_POWER_NORMAL			0x08  /* ����ϵ籨�� */








/******************************************/
/*              �ⲿ����[����]            */
/******************************************/



/* FTP���ݽ��� */
extern bool ftp_parseDataHdlr ( u16 DataSize, u8 *Data );
extern void wip_FTP_no_response_Startup(void);

/* ��ȡ�ź�ǿ�� */
extern u8   getCsq ( void );


/* ����ָ�� */
extern void Gprs_Heart ( void );

/* ����ָ�� */
extern void Gprs_Sleep ( void );

/* �������� */
extern bool Gprs_ParamsConfigure ( u16 DataLen, u8 *Data );

/* ������ѯ */
extern u16  Gprs_ParamsQuery ( u8* response, u16 DataLen, u8 *Data );


/* ��ʱ������ */
extern bool Gprs_TimeoutHdlr ( s32 handle, u8 type, u8 *data, u16 size );
/*���ݷ���*/
extern u16 Offsetdata_upload_DeadTime(u8 *ringbuf);
extern u16 Offset_data_upload_tcp(u8 *ringbuf);
extern u16 OffsetData_Upload(u8 *ringbuf);
extern u16 OffsetData_UploadEx(u8 *ringbuf);
extern u16 Multi_realdata_upload(u16 Counter,u8 UploadTime,bool enable);
extern u16 Multi_offset_data_upload(void);




/****************************************************************
 32960ͨ��Э��ӿں���
*****************************************************************/

extern void vehicles_logout(void);

extern char *RECV_SIM_ICCID;
/*�̼�����*/
extern u8 UpGrade_datalen;
extern u8 UpGrade_data[];






#endif  /* __TZ_GPRS_PROTOCOL_H__ */



