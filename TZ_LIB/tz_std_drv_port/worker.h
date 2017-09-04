


#ifndef __STM32_WORKER_H__
#define __STM32_WORKER_H__

#include "stm32f4xx.h"
#include "defines.h"



/******************************************/
/*              传输参数[配置]            */
/******************************************/

enum {
  WORK_NO_EXIST	    = 0,      /* 销毁 */
	WORK_BUSY_PENDING	= 1 << 0, /* 挂起 */
	WORK_BUSY_RUNNING	= 1 << 1, /* 运行 */
  WORK_BUSY_DYING	  = 1 << 2  /* 将死 */
};


/* 定义错误码 */
#define WORK_ERR_OK             (0)   /* 正常 */
#define WORK_ERR_INTERNAL       (-1)  /* 内部错误 */
#define WORK_ERR_NO_RES         (-2)  /* 无资源 */
#define WORK_ERR_ALREADY        (-3)  /* 任务已建立 */
#define WORK_ERR_UNINIT         (-4)  /* 未初始化 */
#define WORK_ERR_HANDLE         (-5)  /* 句柄错误 */
#define WORK_ERR_USR_PARAMS     (-6)  /* 输入参数错误 */


/* 定义工作处理函数 */
typedef void (*work_func_t) ( void* data );



/* 定义工作对象 */
typedef struct work_struct 
{
	void* data;
	work_func_t func;
} worker_t;





/******************************************/
/*              外部函数[声明]            */
/******************************************/

/* 启动工作者事务 */
extern s16  WORKER_StartupWork ( worker_t *appWorker );

/* 停止工作者事务 */
extern bool WORKER_StopWork ( s16 workhandler );

/* 获取工作者事务执行状态 */
extern u32  WORKER_GetWorkStat ( s16 workhandler );

/* 清空工作者事务上次错误码 */
extern void WORKER_ClearLastErrorCode ( void );

/* 获取工作者事务上次错误码 */
extern s8   WORKER_GetLastErrorCode ( void );



























































/******************************************/
/*           低层接口函数[声明]           */
/******************************************/

/* 工作者单元低级初始化 */
extern void WORKER_LowLevelIfInit ( void );

/* 工作者事务链执行模式 */
extern bool WORKER_DoWorks ( void );

#endif  /* __STM32_WORKER_H__ */

