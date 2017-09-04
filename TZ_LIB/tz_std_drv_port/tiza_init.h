



#ifndef __STM32_INITPORC_H__
#define __STM32_INITPORC_H__


#include "stm32f4xx.h"


/* Initialisation type */ 
typedef enum
{
  SYS_INIT_POWER_ON,              /* Normal power on */ 
  SYS_INIT_REBOOT_FROM_EXCEPTION, /* Reboot after an embedded application exception */ 
  SYS_INIT_DOWNLOAD_SUCCESS,      /* Reboot after a successfull download process */ 
  SYS_INIT_DOWNLOAD_ERROR         /* Reboot after an error in download process */ 
} sys_InitType_e;




/******************************************/
/*           ���ýӿں���[����]           */
/******************************************/

/* CPU��λ */
extern void System_Reset ( void );






extern sys_InitType_e sysInitType;



/******************************************/
/*           �Ͳ�ӿں���[����]           */
/******************************************/

/* ϵͳ��ʼ�� */
extern void System_initProcess ( void );




#endif	/* __STM32_INITPORC_H__ */


