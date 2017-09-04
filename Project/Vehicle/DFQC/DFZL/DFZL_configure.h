#ifndef __DFZL_CONF_H__
#define __DFZL_CONF_H__






/* 客户名称简称 */
#ifndef NT_FAC
#define NT_FAC             "DFZL"
#define CUSTOMER_NAME_STRING (NT_FAC) 
#endif




#include "DFZL_can_configure.h"


/* 定义ACC开时间间隔 */
#ifndef NT_ACC_OPEN_TIME_INTERVAL
#define NT_ACC_OPEN_TIME_INTERVAL             60   /* 单位：s */  
#define TZ_DEFAULT_ACC_OPEN_TIME_INTERVAL       NT_ACC_OPEN_TIME_INTERVAL
#endif


/* 定义ACC关时间间隔 */
#ifndef NT_ACC_CLOSE_TIME_INTERVAL
#define NT_ACC_CLOSE_TIME_INTERVAL            600   /* 单位：s */
#define TZ_DEFAULT_ACC_CLOSE_TIME_INTERVAL      NT_ACC_CLOSE_TIME_INTERVAL
#endif


/* 定义控制器类型 */
#ifndef NT_CONTROLLER_TYPE
#define NT_CONTROLLER_TYPE                    0x01 /* CAN控制器  */
#define TZ_DEFAULT_CONTROLLER_TYPE              NT_CONTROLLER_TYPE
#endif


#endif  /* __NT_CONF_H__ */




