





#ifndef __STM32_UPGRADE_H__
#define __STM32_UPGRADE_H__

#include "stm32f4xx.h"
#include "stdbool.h"

/* �����������ѡ�� */
// <e>UPGRADE controller debug Definitions
// ===================
//   <i>Enable UPGRADE controller debug function
#define UPGRADE_DEBUG           0


/******************************************/
/*            UPGRADE����[����]           */
/******************************************/






/******************************************/
/*           �û��ӿں���[����]           */
/******************************************/

/* ������������ */
extern bool UPGRADE_Start ( void );

/* д���������� */
extern bool UPGRADE_WriteData ( u8 Data[], u32 DataLen );

/* ��װ�û����³��� */
extern void UPGRADE_InstallApplication ( void );

extern u32 FtpToFlash_DataCount;





extern  bool UPGRADE_GetUpgradeConfigure ( void );

extern bool UPGRADE_SetUpgradeConfigure(void);

































/******************************************/
/*           �Ͳ�ӿں���[����]           */
/******************************************/

/* ������Ԫ�ͼ���ʼ�� */
extern void UPGRADE_LowLevelIfInit ( void );





#endif	/* __STM32_UPGRADE_H__ */
