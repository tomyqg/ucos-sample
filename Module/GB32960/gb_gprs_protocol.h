


#include "stm32f4xx.h"
#include "gb_data_type.h"
#include "tiza_timer_port.h"

#ifndef __TZ_TMR_H__
#define __TZ_TMR_H__


/*32960����*/
/*�������� */
#define DEBUG_32960_DECODE     1

/*�������� */
#define DEBUG_32960            0
/*��������*/
#define DATA_DECODE_DEBUG      0

/*SD��ʹ��*/
#define SD_WRITE_ENABLE        1

/*�澯ʱ�䲹��*/
#define TIME_BIAS_ENABLE       0

#if  TIME_BIAS_ENABLE==1 
//����˳��    1:������  0��������
#define SEND_GRADEDOWN         0

#endif

/*�ڲ����� 30s����*/
#define FIFO_RING_SW	  		   1
/*flash�洢 3��������*/
#define FIFO_FLASH_SW	  		   0


/* ����FIFO�����С */
#define BUFFER_FIFO_SIZE        (1024*18)  /* ��λ��byte */


/*SD�ļ�ϵͳ �ļ���*/
extern char SD_filename[30];



/* �û��Զ���ʱ������ */
enum GprsTmrType
{
  TrackAccOpen = 0,
  TrackAccClose,
  Heart,
  Check,
  Noheart,
  Alarming,
  Overtime,
  AccOpen,
  AccClose,
  AccHistoryStats,
  Login,
  MassDataCompensator,
  CyclicPhoto,
  GprsTmrMax = 0x10
};


typedef struct
{
  /* ʱ������ */
  s32 time[ GprsTmrMax ];   
} gprs_tmr_conf_t;


/* GPRSʱ������ */
extern gprs_tmr_conf_t gprsTmr;



/*ʵʱ���ݷ��Ϳ���*/
extern u8 upload_switch;

extern u8 upload_switch_state;

extern vu32 __RestartGprsCounter;

extern s8 IsDeadZone_Network;

extern u16 alarm_alive;

extern s8 logout_flag;
/******************************************/
/*              �ⲿ����[����]            */
/******************************************/

extern void App_tmrStart ( void );
extern void App_tmrStop ( void );


extern void TmrOffsetDataHdlr(u8 times);
extern void TmrCorrectTimeHdlr(u8 times);

extern void TmrRealDataHdlr(u8 times);

extern void DeadData_FlashConfigInit(void);

/*^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
/* ��½ָ�� */
extern void login ( void );

/* ���ݰ����� */
extern void Gprs_ProtocolDataDecode (  u16 len,u8 *data );


/*^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
/* GPRS���䶨ʱ����� */
extern tmr_t * tmrGprsProc ;
extern tmr_t *tmrRealData ;

extern tmr_t *tmrOffsetData;
extern tmr_t *tmrCrrectTime;




#endif  /* __TZ_TMR_H__ */




