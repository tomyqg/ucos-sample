



#ifndef __TIZA_GPRS_PROTOCOL_H__
#define __TIZA_GPRS_PROTOCOL_H__





#include "stm32f4xx.h"
#include "tiza_defined.h"
#include "stdbool.h"




extern int Diag_can_flag;
extern int Diag_can_state;
extern int test_diag;

extern s8  serversocket;

/************/
/* GPRS配置 */
/************/

/* 车机运行过程 */
typedef enum
{
  SYSTEM_PROC_BEGIN = -1,

	/* 初始化 */
  PROC_SYSTEM_INIT,

	/* 登录 */
  PROC_SYSTEM_LOGIN,     

  /* 登陆成功 */
  PROC_SYSTEM_LOGIN_ACK,
	
	/* 实时数据上传 */
  PROC_SYSTEM_REALDATA_UPLOAD,
	
	/* 实时数据应答 */
  PROC_SYSTEM_REALDATA_ACK,
	
	/*补发数据上传(盲区)*/
	PROC_SYSTEM_REISSUEDATA_UPLOAD,

	/* 心跳 */
  PROC_SYSTEM_HEART,
	
	/* 报警 */
  PROC_SYSTEM_ALARM,
	
//	/* 登出 */
//  PROC_SYSTEM_LOGOUT,

	/* 响应 */
  PROC_SYSTEM_RESPONSE,

	/* 系统应用参数设置或查询 */
  PROC_SYSTEM_SYSPARAMS,

	/* 登出 */
  PROC_SYSTEM_LOGOUT,
	
	/* 固件更新 */
  PROC_SYSTEM_UPDATE,
	
	/* 休眠 */
  PROC_SYSTEM_SLEEP,

	/*盲区数据存储*/
	PROC_SYSTEM_REISSUEDATA_STORAGE,
	
	/* 登出应答 */
	PROC_SYSTEM_LOGOUT_ACK,
	
	/* 断网调试*/
	PROC_SYSTEM_DEBUG_CUT_NETWORK,

} sys_status_t;


/* 车机运行状态 */
extern sys_status_t sys_status;





/* GPRS参数 */
typedef struct
{
  /* 设备ID设置标志 */
  bool DeviceIDSet; 
  
  /* APN */
  ascii APN[ __APN_LENGTH__ ];

  /* IP1 */
  ascii IP1[ __SERVER_LENGTH__ ];						
  ascii ip1_domain[ __DOMAIN_LENGTH__ ];

  /* IP2 */
  ascii IP2[ __SERVER_LENGTH__ ];
  ascii ip2_domain[ __DOMAIN_LENGTH__ ];

  /* 终端ID */
  ascii devID[ 17 ];

  /* 协议版本号 */
  u8 ver;

  /* 端口 */
  u16 Port1;
  u16 Port2;

  /* 心跳间隔 */
  u16 check;
  u16 heart;
  u16 noheart;

  /* 网络状态(好) */
  u32 net_is_ok;

  /* 网络状态(差) */
  u32 net_is_bad;
	
/******************************************************************/	
//GPS相关数据
	  /* 纬度 */
  ascii lat[ __LAT_LON_LENGTH__ ];
  
  /* 经度 */
  ascii lon[ __LAT_LON_LENGTH__ ];
	/*纬度*/
	u32 lat_;
	/*经度*/
	u32 lon_;
  
  /* 速度 */
  ascii speed;
  
  /* 方向 */
  ascii dir;
  
  /* 状态位 */
  ascii state[ __STATUS_BIT_LENGTH__ ];  

  u16 diy_can_baut;	//波特率

  u16 diy_can_type;	//帧类型

  u16 diy_can_factry; //厂家编号

  u8  diy_can_nub;//一共有多少帧

	u32 diy_can_data[ 20 ];
	
}gprs_Params_t;

/* GPRS参数 */
extern gprs_Params_t gprsParams;



typedef __packed struct
{ 
  /* 日期 年 */
  u32  date_yy;
  /* 日期 月 */
  u32  date_mm;
  /* 日期 日 */
  u32  date_dd;
	/* 时间_时 */ 
  u32		time_hh;		
  
  /* 时间_分 */
  u32		time_mm;		
  
  /* 时间_秒 */
  u32		time_ss;
	
	/* 定位状态 */
	u8	status_;
	
	/* 纬度 */
  ascii lat[ 4 ];
  
  /* 经度 */
  ascii lon[ 4 ];	
		
}location_t;

extern location_t locations;




/* 拨号状态 */
extern bool dial_OK;






/* 中心服务器标志 */
extern u32 centerSrvStatus;
#define CENTER_SERVER1_LOGIN_BIT            0x00    /* 登录服务器1 */
#define CENTER_SERVER2_LOGIN_BIT            0x01    /* 登录服务器2 */
#define CENTER_SERVERX_IS_LOGIN_BIT         0x80    /* 已登录服务器X */
#define CENTER_SERVERX_RSP_IS_TIMEOUT_BIT   0x10    /* 服务器响应超时 */

/* 报警类型 */
#define TZ_ALARM_TYPE_OVERSPEED					0x01  /* 超速报警 */
#define TZ_ALARM_TYPE_GPS_ANTENNA				0x02  /* GPS天线未接或剪断报警 */
#define TZ_ALARM_TYPE_GPS_MOUDLE				0x03  /* GPS模块故障报警 */
#define TZ_ALARM_TYPE_LOW_VOLTAGE				0x04  /* 低电压报警 */
#define TZ_ALARM_TYPE_DATA_COM_ABORT		0x05  /* 数据通讯中断报警 */
#define TZ_ALARM_TYPE_POWER_OFF					0x06  /* 断电报警 */
#define TZ_ALARM_TYPE_TAIL							0x07  /* 拖车报警 */
#define TZ_ALARM_TYPE_POWER_NORMAL			0x08  /* 解除断电报警 */








/******************************************/
/*              外部函数[声明]            */
/******************************************/



/* FTP数据解析 */
extern bool ftp_parseDataHdlr ( u16 DataSize, u8 *Data );
extern void wip_FTP_no_response_Startup(void);

/* 获取信号强度 */
extern u8   getCsq ( void );


/* 心跳指令 */
extern void Gprs_Heart ( void );

/* 休眠指令 */
extern void Gprs_Sleep ( void );

/* 参数配置 */
extern bool Gprs_ParamsConfigure ( u16 DataLen, u8 *Data );

/* 参数查询 */
extern u16  Gprs_ParamsQuery ( u8* response, u16 DataLen, u8 *Data );


/* 超时处理器 */
extern bool Gprs_TimeoutHdlr ( s32 handle, u8 type, u8 *data, u16 size );
/*数据发送*/
extern u16 Offsetdata_upload_DeadTime(u8 *ringbuf);
extern u16 Offset_data_upload_tcp(u8 *ringbuf);
extern u16 OffsetData_Upload(u8 *ringbuf);
extern u16 OffsetData_UploadEx(u8 *ringbuf);
extern u16 Multi_realdata_upload(u16 Counter,u8 UploadTime,bool enable);
extern u16 Multi_offset_data_upload(void);




/****************************************************************
 32960通信协议接口函数
*****************************************************************/

extern void vehicles_logout(void);

extern char *RECV_SIM_ICCID;
/*固件升级*/
extern u8 UpGrade_datalen;
extern u8 UpGrade_data[];






#endif  /* __TZ_GPRS_PROTOCOL_H__ */



