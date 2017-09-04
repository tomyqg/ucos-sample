









#ifndef __FLASH_H__
#define __FLASH_H__



#include "stm32f4xx_flash.h"

#include "stdbool.h"



/******************************************/
/*             FLASH����[����]            */
/******************************************/




/******************************************/
/*              �ⲿ����[����]            */
/******************************************/

/* FLASH�洢�����Ƿ���� */
extern u16  FLASH_ObjectExist ( u16 ID );

/* FLASH�洢�������ݶ�ȡ */
extern bool FLASH_ReadData ( u16 ID, u16 Len, u8 *ReadData );

/* FLASH�洢��������д�� */
extern bool FLASH_WriteData ( u16 ID, u16 Len, u8 *WriteData );

/* ����ȫ��FALSH�洢���� */
extern bool FLASH_EraseAll ( void );

/* �����ض�FLASH�洢���� */
extern bool FLASH_EraseObject ( u16 ID );











































































#ifndef __USE_FLASH__
#define __USE_FLASH__		  1
#endif  /* __USE_FLASH__ */


/******************************************/
/*            �ͼ��ӿں���[����]          */
/******************************************/

/*  */
extern void FLASH_LowlLevelIfInit ( void );


#endif  /* __FLASH_H__ */

