#ifndef __TZ_ACC_H__
#define __TZ_ACC_H__
#include "tiza_defined.h"
#include "stm32f4xx.h"
/******************************************/
/*           TZ-ACC 参数[配置]            */
/******************************************/

typedef struct
{
  /* 累计时间 */
  u32 total_time;

  /* 打开上传间隔 */
  u16 open_time;

  /* 关闭上传间隔 */ 
  u16 close_time;
  
  /* 上传时间点个数 */
  u8 upload_time_count;
  
  struct UPLOAD_TIME_INFO
  {
    /* 上传时间点：时 */
    u8 hh;

    /* 上传时间点：分 */
    u8 mm;
  } upload_time[ TZ_MAX_UPLOAD_TIME_COUNT ];
	
}	acc_info_t;



extern acc_info_t accInfo;
extern s8 acc_io_handler;





/******************************************/
/*              外部函数[声明]            */
/******************************************/



extern void TZ_ACC_Init(void);
extern u8 TZ_Get_ACC_State(void);

#endif  /* __TZ_ACC_H__ */

