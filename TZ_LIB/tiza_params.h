






#ifndef __TZ_PARAMETERS_H__
#define __TZ_PARAMETERS_H__





#include "stm32f4xx.h"

//#include "tz_common.h"
#include "stdbool.h"
#include "tiza_defined.h"
#include "defines.h"






/******************************************/
/*              全局变量[定义]            */
/******************************************/


/************************************************ 
 * 注  意：                                     *
 * (1)本部分定义的参数存储在FRAM里面；          *
 * (2)存储对象的ID数量不得超过8个(0~7);         *
 * (3)每个对象的存储尺寸须小于512bytes!!        *
 ************************************************/

enum   
{
  /* 系统参数 */
  VEHICLE_PARAMS = 0,

  /* GPRS通信参数 */
  GPRS_PARAMS,

  /* 最后有效位置 */
  LAST_POSITION,

  /* ACC信息 */
  ACC_INFO,
  
  /* ACC工作时段统计 */
  ACC_WORK_STATS,

  /* 控制器数据 */
  CONTROLLER_DATA,

  /* 控制器数据 */
  STORE_DATA_COMPENSATOR,
  /* 32960 参数设置*/
  PARAMETER_DATA,

  /* 大小已经限定，请勿任意修改！ */
  GPS_OBJ_NB = 8
};








/////////////////////////////////////////////////////////////////////////////////////

/* 调试GPS选项 */
#define Debug_Type_GPS            0x0001 

/* 调试GSM选项 */
#define Debug_Type_GSM            0x0002

/* 调试FTP选项 */ 
#define Debug_Type_FTP            0x0004

/* 调试CAN选项 */
#define Debug_Type_CAN            0x0008

/* 调试SMS选项 */
#define Debug_Type_SMS            0x0010

/* 调试MODEM选项 */
#define Debug_Type_MDM            0x0020

/* 调试串口选项 */
#define Debug_Type_COM4           0x0100

typedef struct
{
  /* 调试端口 */
  u8 debugPort;	
	
  /* 调试状态 */
  bool debugIsValid;

  /* 调试选项 */
  u32 debugType;
} sys_debug_t;

/////////////////////////////////////////////////////////////////////////////////////




/* RMC结构 */
typedef struct  {

  /* 时间_时 */ 
  u32		time_hh;		
  
  /* 时间_分 */
  u32		time_mm;		
  
  /* 时间_秒 */
  u32		time_ss;		
    
  /* 日期_日 */
	u32		date_dd;	
    	
  /* 日期_月 */
	u32		date_mm;	
    	
  /* 日期_年 */
	u32		date_yy;	
       
} GPS_DATA;



/* 参数信息 */
typedef struct
{

 /* 新增参数：gl     */
	GPS_DATA GPS_d;
/***********************************************************/
  /*终端登录流水号*/
  u16 LoginSerial;
	
  /* 车辆VIN */
  ascii CarVIN[ 17 ];
	
  /* 车辆VIN设置标志 */
  bool CarVINSet;
	
	/*DCDC 后端外电检测*/
	/* TRUE : 有外电 FALSE :无外电 */
//	bool IsDCDC_PWR;
/***********************************************************/
	
  /* 调试口波特率*/
  u32 debugBaudrate;

  /* GPS波特率 */
  u32 gpsBaudrate;

  /* GSM波特率 */
  u32 gsmBaudrate;
      
  /* 调试控制器 */
  sys_debug_t DebugCtroller;

  /* SIM卡号 */
  ascii sim_no[ __SIM_NUMBER_LENGTH__ ];

  /* IMSI码 */ 
  ascii imsi_code[ __IMSICODE_LENGTH__ ];

  /* 厂家ID */
  ascii factoryID[ __FACTORY_ID_SIZE__ ];

  /* 通信中断报警时间 */
  u16 controllerInterruptAlarmCount;

  /* 车机进入省电时间 */
  u16 sleepTime;

  /* 是否睡眠？ */
  u32 sleepMode;
#define SLEEP_STATUS_IS_SET_BIT             0x01      /* 使能休眠功能 */


  /* 升级是否成功？ */
  u8 upgrade;
#define UPGRADE_STATUS_IS_UPGRADING_BIT     0x01      /* 正在升级状态 */
#define UPGRADE_STATUS_IS_OK_BIT            0x02      /* 升级成功 */


#if TZ_DISTANCE_STATISTICS == 1
  /* 里程累积 */
  u32 totalMoveDistance;
#endif  /* TZ_DISTANCE_STATISTICS */


u32 servercheck;


} vechile_Params_t;


/* 车机参数 */
extern vechile_Params_t vehicleParams;


typedef struct {
    u8 bkp_no_ready;      /*BKP reg 读写标志      1: 无法写入 0:可以写入  */
    u8 gps_position_flag; /*定位标志     1: 定位  0:不定位*/
    u8 ftp_no_reset;      /*ftp 复位标志 1: 正在运行FTP 不立即复位 0: 没有进行FTP */
    u8 gprs_ready;        /*gprs 使能标志 1:gprs 开启  0: gprs 未开启*/
    u8 ack_flags;         /*登录应答标志1:有登录应答 0:没有登录应答*/
    u8 wake_time_zero_flag; /*唤醒时间间隔为0标志 1: 唤醒时间间隔为0     0: 唤醒时间间隔不为0*/
    u8 config_time;       /*配置时间信息标志0:未配置 1:已经配置*/
    u8 sms_flags;         /*SMS 配置标志 1:已配置 0:未配置(用于远程升级)*/
		u8 gsm_ver[32];
    u8 ftp_buf_len;
    u8 ftp_buf[256];
	u8 ccid[20];
}status_param;

extern status_param drv_param;















/******************************************/
/*              外部函数[声明]            */
/******************************************/
extern void ParameterQuery_Init(void);
extern void sysParamsInit ( void );
extern void sysParamsSave ( void );
extern void sysParamsReset ( void );

extern void sysResetState ( void );
extern void sysResetGprs ( void );
extern void sysResetDebug ( void );
extern void sysResetVehicle ( void );
extern void sysResetAccWorkPara ( void );
extern void sysResetControllerPara ( void );
extern void sysResetAccWorkStatsInfo ( void );


#endif  /* __TZ_PARAMETERS_H__ */


