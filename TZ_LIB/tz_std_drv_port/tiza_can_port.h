



#ifndef __TZ_CAN_H__
#define __TZ_CAN_H__



#include "stm32f4xx.h"
#include "tiza_include.h"


/* CAN���Ĵ��� */
typedef bool ( *can_procDataHdlr_f ) (u32 frameID, u16 DataSize, u8 *Data );



/* ���ܻ����� */
#define MAX_CAN_BUF_LENGTH            24


/******************************************/
/*            CAN����ģʽ[����]           */
/******************************************/
#define CAN_MODE_NORMAL           0x00    /* ����ģʽ */
#define CAN_MODE_LISTENER         0x01    /* ����ģʽ */       
#define CAN_MODE_TEST             0x02    /* ���Բ���ģʽ */





/******************************************/
/*            TZ-CAN����[����]            */
/******************************************/

#ifndef CAN_RECVED_BUFFER_LENGTH
#define CAN_RECVED_BUFFER_LENGTH      96    /* ��λ��bytes */
#endif  /* CAN_RECVED_BUFFER_LENGTH */


#ifndef CAN_TOSEND_BUFFER_LENGTH
#define CAN_TOSEND_BUFFER_LENGTH      32    /* ��λ��bytes */
#endif  /* CAN_TOSEND_BUFFER_LENGTH */


//#ifndef CAN_CONTROLLER_ID
//#define CAN_CONTROLLER_ID             ( 0x0000 )
//#endif  /* CAN_CONTROLLER_ID */


#ifndef CAN_LENGTH_BYTES
#define CAN_LENGTH_BYTES               1    /* ��λ��bytes */
#endif  /* CAN_LENGTH_BYTES */





enum CAN_STATUS
{
	CAN_STATUS_BEGIN = 0,
	CAN_STATUS_Unknow,
	CAN_STATUS_OK,
	CAN_STATUS_Bus_Off,
	CAN_STATUS_Error_Warning,
	CAN_STATUS_Error_Passive,
	CAN_STATUS_Error,
	CAN_STATUS_Wakeup,
	CAN_STATUS_Sleep,
	CAN_STATUS_Last_Error_Code,
	CAN_STATUS_END
};  /*  CANģ��״̬  */


enum  
{ 
    CAN_BUS_BDRT_10K  = 10,
	CAN_BUS_BDRT_20K  = 20,
	CAN_BUS_BDRT_40K  = 40,
	CAN_BUS_BDRT_50K  = 50,
	CAN_BUS_BDRT_80K  = 80,
	CAN_BUS_BDRT_100K = 100,
	CAN_BUS_BDRT_125K = 125,
	CAN_BUS_BDRT_200K = 200,
	CAN_BUS_BDRT_250K = 250,
	CAN_BUS_BDRT_400K = 400,
	CAN_BUS_BDRT_500K = 500,
	/*CAN_BUS_BDRT_666K = 666, ��ʱ������*/
	CAN_BUS_BDRT_800K = 800, 
	CAN_BUS_BDRT_1M   = 1000 
};	/*  CANģ�鲨����  */


/******************************************/
/*            CAN����֡[����]             */
/******************************************/
#define CAN_FRAME_EXT_AND_STD     0x00    /* ���֡ */   
#define CAN_FRAME_EXT             0x01    /* ��չ֡ */         
#define CAN_FRAME_STD             0x02    /* ��׼֡ */ 


typedef struct 
{
	/* �Ƿ���Ч״̬ */
	bool IsValid;  

	enum CAN_STATUS Status;

	/* ���ݴ����� */
	can_procDataHdlr_f  DataHandler;

	/* ����ģʽ */
	u8 CAN_RUN;

	/* ͬ����Ծ */
	u8 CAN_Sjw;

	/* ʱ���1 */
	u8 CAN_Bs1;

	/* ʱ���2 */
	u8 CAN_Bs2;

	/* CAN BRP */
	u16 CAN_BRP;

	/* ֡���� */
	u8 CAN_FrameType;
	
	u32 CAN_FrameID;

	u8 RxBuffer[ MAX_CAN_BUF_LENGTH ];
	u8 RxBufSiz;
} CAN_DevTypedef;  /* CANģ���ʼ������ */


/* CAN Param configure Structure definition */
typedef struct 
{
	/* CAN ������ */
	u32 u32Baudrate;

	can_procDataHdlr_f DataHandler;

} CAN_Paramdef;



typedef struct
{
  /* ���������ұ�� */
  u16 ControllerID;

  /* �����ֶγ��� */
  u8  LengthOpt;

  /* ���ݽ��ճ��� */
  u16 DataSize;

  /* ���ݴ洢��ַ */
  u8* Data;
} can_info_t;



typedef struct
{
  /* ���ݷ������� */
  u32 sendType;

  /* ���ݳ��� */
  u8 length;

  /* ���������� */
  u8 data[ CAN_TOSEND_BUFFER_LENGTH ];
} can_data_t;





/* CAN���ݱ��� */
extern can_data_t canBackupData;
extern  u8 * CanSendBuffer;


















/******************************************/
/*              �ⲿ����[����]            */
/******************************************/

/* CANӦ�ýӿڳ�ʼ�� */
extern void TZ_CAN_ApplicationIfInit ( void );

/* CAN���������� */
extern void CAN_ProcDataHdlr ( void );

/* ��������CAN�����·��Ĳ��� */
extern bool CAN_SetControllerParamsByServer ( u16 controllerID, u16 DataSize, u8 *IPData );

/* CAN���������ݸ�λ */
extern void CAN_BackupBufferReset ( void );

/* CAN���������ݶ�ȡ */
extern void CAN_BackupBufferRead ( void );

/* ��ȡCAN����������Ϣ */
extern can_info_t* CAN_GetDataInfo ( void );


uint8 NssInit(void);

#endif  /* __TZ_CAN_H__ */



