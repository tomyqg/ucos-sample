#ifndef __DFZL_CONF_H__
#define __DFZL_CONF_H__






/* �ͻ����Ƽ�� */
#ifndef NT_FAC
#define NT_FAC             "DFZL"
#define CUSTOMER_NAME_STRING (NT_FAC) 
#endif




#include "DFZL_can_configure.h"


/* ����ACC��ʱ���� */
#ifndef NT_ACC_OPEN_TIME_INTERVAL
#define NT_ACC_OPEN_TIME_INTERVAL             60   /* ��λ��s */  
#define TZ_DEFAULT_ACC_OPEN_TIME_INTERVAL       NT_ACC_OPEN_TIME_INTERVAL
#endif


/* ����ACC��ʱ���� */
#ifndef NT_ACC_CLOSE_TIME_INTERVAL
#define NT_ACC_CLOSE_TIME_INTERVAL            600   /* ��λ��s */
#define TZ_DEFAULT_ACC_CLOSE_TIME_INTERVAL      NT_ACC_CLOSE_TIME_INTERVAL
#endif


/* ������������� */
#ifndef NT_CONTROLLER_TYPE
#define NT_CONTROLLER_TYPE                    0x01 /* CAN������  */
#define TZ_DEFAULT_CONTROLLER_TYPE              NT_CONTROLLER_TYPE
#endif


#endif  /* __NT_CONF_H__ */




