#ifndef __TZ_ACC_H__
#define __TZ_ACC_H__
#include "tiza_defined.h"
#include "stm32f4xx.h"
/******************************************/
/*           TZ-ACC ����[����]            */
/******************************************/

typedef struct
{
  /* �ۼ�ʱ�� */
  u32 total_time;

  /* ���ϴ���� */
  u16 open_time;

  /* �ر��ϴ���� */ 
  u16 close_time;
  
  /* �ϴ�ʱ������ */
  u8 upload_time_count;
  
  struct UPLOAD_TIME_INFO
  {
    /* �ϴ�ʱ��㣺ʱ */
    u8 hh;

    /* �ϴ�ʱ��㣺�� */
    u8 mm;
  } upload_time[ TZ_MAX_UPLOAD_TIME_COUNT ];
	
}	acc_info_t;



extern acc_info_t accInfo;
extern s8 acc_io_handler;





/******************************************/
/*              �ⲿ����[����]            */
/******************************************/



extern void TZ_ACC_Init(void);
extern u8 TZ_Get_ACC_State(void);

#endif  /* __TZ_ACC_H__ */

