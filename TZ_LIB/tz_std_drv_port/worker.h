


#ifndef __STM32_WORKER_H__
#define __STM32_WORKER_H__

#include "stm32f4xx.h"
#include "defines.h"



/******************************************/
/*              �������[����]            */
/******************************************/

enum {
  WORK_NO_EXIST	    = 0,      /* ���� */
	WORK_BUSY_PENDING	= 1 << 0, /* ���� */
	WORK_BUSY_RUNNING	= 1 << 1, /* ���� */
  WORK_BUSY_DYING	  = 1 << 2  /* ���� */
};


/* ��������� */
#define WORK_ERR_OK             (0)   /* ���� */
#define WORK_ERR_INTERNAL       (-1)  /* �ڲ����� */
#define WORK_ERR_NO_RES         (-2)  /* ����Դ */
#define WORK_ERR_ALREADY        (-3)  /* �����ѽ��� */
#define WORK_ERR_UNINIT         (-4)  /* δ��ʼ�� */
#define WORK_ERR_HANDLE         (-5)  /* ������� */
#define WORK_ERR_USR_PARAMS     (-6)  /* ����������� */


/* ���幤�������� */
typedef void (*work_func_t) ( void* data );



/* ���幤������ */
typedef struct work_struct 
{
	void* data;
	work_func_t func;
} worker_t;





/******************************************/
/*              �ⲿ����[����]            */
/******************************************/

/* �������������� */
extern s16  WORKER_StartupWork ( worker_t *appWorker );

/* ֹͣ���������� */
extern bool WORKER_StopWork ( s16 workhandler );

/* ��ȡ����������ִ��״̬ */
extern u32  WORKER_GetWorkStat ( s16 workhandler );

/* ��չ����������ϴδ����� */
extern void WORKER_ClearLastErrorCode ( void );

/* ��ȡ�����������ϴδ����� */
extern s8   WORKER_GetLastErrorCode ( void );



























































/******************************************/
/*           �Ͳ�ӿں���[����]           */
/******************************************/

/* �����ߵ�Ԫ�ͼ���ʼ�� */
extern void WORKER_LowLevelIfInit ( void );

/* ������������ִ��ģʽ */
extern bool WORKER_DoWorks ( void );

#endif  /* __STM32_WORKER_H__ */

