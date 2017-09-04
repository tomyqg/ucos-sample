#include "stm32f4xx.h"



#ifndef __STM32_WDG_H__
#define __STM32_WDG_H__






/******************************************/
/*               WDG����[����]            */
/******************************************/

/* ��ѡ���Ź����� */
#define WDG_TYPE_EXTERNAL         0x01			/* �ⲿ���Ź� */
#define WDG_TYPE_INDEPENDENCY     0x02			/* �������Ź� */
#define WDG_TYPE_WINDOWS          0x03			/* ���ڿ��Ź� */










/******************************************/
/*           �Ͳ�ӿں���[����]           */
/******************************************/

/* ���Ź����� */
extern void WDG_configuration ( void );


/* ι�� */
extern void WDG_KickCmd ( void );




#endif	/* __STM32_WDG_H__ */

