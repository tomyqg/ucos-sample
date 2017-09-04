


#include "stm32f4xx.h"
#include "gb_data_type.h"
#include "tiza_timer_port.h"

#ifndef __TZ_TMR_H__
#define __TZ_TMR_H__


/*32960调试*/
/*下行数据 */
#define DEBUG_32960_DECODE     1

/*上行数据 */
#define DEBUG_32960            0
/*下行数据*/
#define DATA_DECODE_DEBUG      0

/*SD卡使能*/
#define SD_WRITE_ENABLE        1

/*告警时间补偿*/
#define TIME_BIAS_ENABLE       0

#if  TIME_BIAS_ENABLE==1 
//发送顺序    1:降序发送  0：升序发送
#define SEND_GRADEDOWN         0

#endif

/*内部缓存 30s报警*/
#define FIFO_RING_SW	  		   1
/*flash存储 3天内数据*/
#define FIFO_FLASH_SW	  		   0


/* 定义FIFO缓存大小 */
#define BUFFER_FIFO_SIZE        (1024*18)  /* 单位：byte */


/*SD文件系统 文件名*/
extern char SD_filename[30];



/* 用户自定义时钟类型 */
enum GprsTmrType
{
  TrackAccOpen = 0,
  TrackAccClose,
  Heart,
  Check,
  Noheart,
  Alarming,
  Overtime,
  AccOpen,
  AccClose,
  AccHistoryStats,
  Login,
  MassDataCompensator,
  CyclicPhoto,
  GprsTmrMax = 0x10
};


typedef struct
{
  /* 时间周期 */
  s32 time[ GprsTmrMax ];   
} gprs_tmr_conf_t;


/* GPRS时间设置 */
extern gprs_tmr_conf_t gprsTmr;



/*实时数据发送开关*/
extern u8 upload_switch;

extern u8 upload_switch_state;

extern vu32 __RestartGprsCounter;

extern s8 IsDeadZone_Network;

extern u16 alarm_alive;

extern s8 logout_flag;
/******************************************/
/*              外部函数[声明]            */
/******************************************/

extern void App_tmrStart ( void );
extern void App_tmrStop ( void );


extern void TmrOffsetDataHdlr(u8 times);
extern void TmrCorrectTimeHdlr(u8 times);

extern void TmrRealDataHdlr(u8 times);

extern void DeadData_FlashConfigInit(void);

/*^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
/* 登陆指令 */
extern void login ( void );

/* 数据包解析 */
extern void Gprs_ProtocolDataDecode (  u16 len,u8 *data );


/*^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
/* GPRS传输定时器句柄 */
extern tmr_t * tmrGprsProc ;
extern tmr_t *tmrRealData ;

extern tmr_t *tmrOffsetData;
extern tmr_t *tmrCrrectTime;




#endif  /* __TZ_TMR_H__ */




