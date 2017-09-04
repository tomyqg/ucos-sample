




#ifndef __TZ_DEFINED_H__
#define __TZ_DEFINED_H__



/* 程序版本号 */
#ifndef NT_SVN
#define NT_SVN             								"ZT3000-DF-V1.00"
#define SOFTWARE_VERSION     									(NT_SVN) 
#endif


/* 硬件版本号 */
#ifndef NT_HVN
#define NT_HVN             										"ZT3000-v1.2"
#define HARDWARE_EDITION     										(NT_HVN) 
#endif


/* 客户名称 */
#ifndef CUSTOMER_NAME_STRING
#define CUSTOMER_NAME_STRING                    "XXXXXX"
#endif  /* CUSTOMER_NAME_STRING */




/* 默认短信服务中心号码 */
#ifndef TZ_DEFAULT_SMS_SERVICE_CENTER
#define TZ_DEFAULT_SMS_SERVICE_CENTER	          "+8615951766741"
#endif

/* 默认短信中心号码 */
#ifndef TZ_DEFAULT_SMS_CENTER
#define TZ_DEFAULT_SMS_CENTER	                  "15951766741"
#endif

/* 默认SIM卡IMSI号 */
#ifndef TZ_DEFAULT_SIM_IMSICODE
#define TZ_DEFAULT_SIM_IMSICODE                 "XXXXXXXXXXXXXXX"
#endif

/* 默认GPRS-APN */
#ifndef TZ_DEFAULT_GPRS_APN
#define TZ_DEFAULT_GPRS_APN	                    "CMNET"
#endif

/* 默认GPRS用户名 */
#ifndef TZ_DEFAULT_GPRS_USR
#define TZ_DEFAULT_GPRS_USR	                    ""
#endif

/* 默认GPRS密码 */
#ifndef TZ_DEFAULT_GPRS_PWD
#define TZ_DEFAULT_GPRS_PWD	                    ""
#endif

/* 默认服务器地址1 */
#ifndef TZ_DEFAULT_PEER_ADDR1
#define TZ_DEFAULT_PEER_ADDR1	                  "218.94.153.146"
#endif

/* 默认服务器地址2 */
#ifndef TZ_DEFAULT_PEER_ADDR2
#define TZ_DEFAULT_PEER_ADDR2	                  "218.94.153.146"
#endif

/* 默认服务器端口号 */
#ifndef TZ_DEFAULT_PEER_PORT
#define TZ_DEFAULT_PEER_PORT                    21000
#endif

/* 默认服务器端口号 */
#ifndef TZ_DEFAULT_PEER_VIN
#define TZ_DEFAULT_PEER_VIN                    "A0000018061716611"
#endif


/* 默认测试服务器地址 */
#ifndef TZ_DEFAULT_TEST_ADDR
#define TZ_DEFAULT_TEST_ADDR                    "218.94.153.146"
#endif

/* 默认测试服务器端口 */
#ifndef TZ_DEFAULT_TEST_PORT
#define TZ_DEFAULT_TEST_PORT                    4096
#endif

/* 默认主域名 */
#ifndef TZ_DEFAULT_MS_DOMIAN
#define TZ_DEFAULT_MS_DOMIAN                    "www.baidu.cn"
#endif

/* 默认从域名 */
#ifndef TZ_DEFAULT_SS_DOMIAN
#define TZ_DEFAULT_SS_DOMIAN                    "www.baidu.cn"
#endif

/* 默认中心心跳检测间隔 */
#ifndef TZ_DEFAULT_SERVER_HEART_CHECK_INTERVAL
#define TZ_DEFAULT_SERVER_HEART_CHECK_INTERVAL  180		/* 单位：s */
#endif

/* 默认终端上传心跳间隔 */
#ifndef TZ_DEFAULT_VEHICLE_HEART_INTERVAL
#define TZ_DEFAULT_VEHICLE_HEART_INTERVAL       30	   /* 单位：s */
#endif  

/* 默认追踪间隔 */
#ifndef TZ_DEFAULT_GPRS_TRACK_INETRVAL
#define TZ_DEFAULT_GPRS_TRACK_INETRVAL          30			/* 单位：s */
#endif

/* 默认盲区补偿间隔 */
#ifndef TZ_DEFAULT_GPRS_COMPENSATOR_INETRVAL
#define TZ_DEFAULT_GPRS_COMPENSATOR_INETRVAL    90    /* 单位：s */
#endif

/* 默认历史工作时段统计间隔 */
#ifndef TZ_DEFAULT_GPRS_ACC_STATS_INETRVAL
#define TZ_DEFAULT_GPRS_ACC_STATS_INETRVAL      45    /* 单位：s */  
#endif

/* 默认ACC开上传工作参数时间间隔 */
#ifndef TZ_DEFAULT_ACC_OPEN_TIME_INTERVAL
#define TZ_DEFAULT_ACC_OPEN_TIME_INTERVAL       30		/* 单位：s */
#endif


/* 默认ACC关上传工作参数时间间隔 */
#ifndef TZ_DEFAULT_ACC_CLOSE_TIME_INTERVAL
#define TZ_DEFAULT_ACC_CLOSE_TIME_INTERVAL      600		/* 单位：s */
#endif


/* 默认进入休眠状态的时间间隔 */
#ifndef TZ_DEFAULT_SLEEP_TIME_INTERVAL
#define TZ_DEFAULT_SLEEP_TIME_INTERVAL          600		/* 单位：s */
#endif


/* 默认仪表通信中断报警时间间隔 */
#ifndef TZ_DEFAULT_CTL_INTERRUPT_ALARM_INTERVAL
#define TZ_DEFAULT_CTL_INTERRUPT_ALARM_INTERVAL 120		/* 单位：s */
#endif


/* 默认低电压报警值 */
#ifndef TZ_DEFAULT_LOW_POWER_VOLTAGE
#define TZ_DEFAULT_LOW_POWER_VOLTAGE            20	/* 单位：v */
#endif


/* 默认协议版本号 */
#ifndef TZ_DEFAULT_PROTOCOL_VERSION              
#define TZ_DEFAULT_PROTOCOL_VERSION             4
#endif


/* 默认厂家标志 */
#ifndef TZ_DEFAULT_FACTORY_ID  
#define TZ_DEFAULT_FACTORY_ID                   0x26
#endif


/* 控制器类型 */
#ifndef TZ_DEFAULT_CONTROLLER_TYPE 
#define TZ_DEFAULT_CONTROLLER_TYPE              ( TZ_CONTROLLER_TYPE_NONE )
#endif  


/* 中心超时重传次数 */
#ifndef TZ_MAX_TIME_OUT_COUNT
#define TZ_MAX_TIME_OUT_COUNT                   3
#endif


/* 上传时间点的个数 */
#ifndef TZ_MAX_UPLOAD_TIME_COUNT
#define TZ_MAX_UPLOAD_TIME_COUNT                24
#endif


/* 使能工作时段统计 */
#ifndef TZ_USE_ACC_WORK_STATS
#define TZ_USE_ACC_WORK_STATS                   1
#endif  


/* 工作时段统计历史个数 */
#ifndef TZ_MAX_WORK_LOG_SIZE
#define TZ_MAX_WORK_LOG_SIZE                    16
#endif


/* 使能盲区数据补偿 */
#ifndef TZ_USE_STORAGE_MASS_DATA
#define TZ_USE_STORAGE_MASS_DATA                0
#endif


/* 使能定时重启功能 */
#ifndef TZ_USE_FIX_TIME_RESET
#define TZ_USE_FIX_TIME_RESET                   1
#endif


/* 使能里程统计功能 */  
#ifndef TZ_DISTANCE_STATISTICS
#define TZ_DISTANCE_STATISTICS                  1 
#endif


/* 使能ACC时间累积功能 */
#ifndef TZ_USE_ACC_TIME_ACCUMULATE
#define TZ_USE_ACC_TIME_ACCUMULATE              1
#endif


#if TZ_USE_ACC_TIME_ACCUMULATE == 1 || TZ_DISTANCE_STATISTICS == 1
/* 使能定时保存ACC信息 */
#ifndef TZ_USE_FIX_TIME_SAVE_ACC_INFO
#define TZ_USE_FIX_TIME_SAVE_ACC_INFO           1
#endif
#endif  /* TZ_USE_ACC_TIME_ACCUMULATE */


/* 累计时间定时保存间隔 */
#ifndef ACC_TIME_SAVE_INTERVAL
#define ACC_TIME_SAVE_INTERVAL                  1800    /* 单位：秒 */    
#endif



/* 开关短信报警功能 */
#ifndef TZ_DEFAULT_SMS_ALARM_EN
#define TZ_DEFAULT_SMS_ALARM_EN                 0   /* 关闭短信报警 */
#endif
 


/* 侧翻 */
#ifndef TZ_DEFAULT_VEHICLE_OVERTURN
#define TZ_DEFAULT_VEHICLE_OVERTURN             0  /* 默认为0，表示没有启动侧翻功能 */
												                            /* 当TZ_DEFAULT_VEHICLE_OVERTURN不为0时，表示侧翻判断阈值，单位为秒 */
#endif



/* 停车超时 */
#ifndef TZ_DEFAULT_STOP_CAR_TO
#define TZ_DEFAULT_STOP_CAR_TO                  0   /* 默认为0，表示没有启动停车超时功能 */
												                            /* 当TZ_DEFAULT_STOP_CAR_TO不为0时，表示停车超时阈值，单位为分钟 */
#endif


/* 超速报警阈值 */
#ifndef TZ_DEFAULT_OVER_SPEED
#define TZ_DEFAULT_OVER_SPEED                   0   /* 默认为0，表示没有启动超速报警功能 */
													                           /* 当TZ_DEFAULT_OVER_SPEED不为0时，表示超速阈值，单位为km/s */
#endif


/* 默认重传尝试次数 */  
#ifndef TZ_DEFAULT_RETRY_MAX
#define TZ_DEFAULT_RETRY_MAX                    2
#endif


/* 默认关键指令重发间隔 */
#ifndef TZ_DEFAULT_RETRY_INETRVAL         
#define TZ_DEFAULT_RETRY_INETRVAL					      8   /* 单位：s */
#endif

/* 拍照最短时间间隔 */
#ifndef TZ_PHOTO_MIN_TIME_INTERVAL
#define TZ_PHOTO_MIN_TIME_INTERVAL              15  /* 单位：s */
#endif


/* 号码字段长度 */
#define __PHONE_LENGTH__                        32  /* 单位：byte */

/* IP地址字段长度 */
#define __SERVER_LENGTH__                       16  /* 单位：byte */

/* 域名字段长度 */
#define __DOMAIN_LENGTH__                       64  /* 单位：byte */

/* APN字段长度 */
#define __APN_LENGTH__                          32  /* 单位：byte */

/* 经纬度字段长度 */
#define __LAT_LON_LENGTH__                      4  /* 单位：byte */

/* 海拔高度字段长度 */
#define __ALTI_LENGTH__                         2  /* 单位：byte */

/* 时间字段长度 */
#define __TIME_LENGTH__                         6  /* 单位：byte */

/* 状态位字段长度 */
#define __STATUS_BIT_LENGTH__                   4  /* 单位：byte */

/* 默认SIM卡IMSI长度 */
#define __IMSICODE_LENGTH__                     20  /* 单位：byte */

/* 默认SIM卡号长度 */
#define __SIM_NUMBER_LENGTH__                   12  /* 单位：byte */

/* 默认软件版本号长度 */
#define __SVN_LENGTH__                          32  /* 单位：byte */

/* 默认硬件版本号长度 */
#define __HVN_LENGTH__                          32  /* 单位：byte */

/* 默认厂家名称长度 */
#define __FACTORY_ID_SIZE__                     32  /* 单位：byte */  



/* 登录信息长度 */
#if defined TZ_MSG_LOGIN_LENGTH
#define __LOGIN_DEFAULT_LENGTH__                TZ_MSG_LOGIN_LENGTH
#else
#define __LOGIN_DEFAULT_LENGTH__                64  /* 单位：byte */
#endif


/* 响应信息长度 */
#if defined TZ_MSG_RESPONSE_LENGTH
#define __RESPONSE_DEFAULT_LENGTH__             TZ_MSG_RESPONSE_LENGTH
#else
#define __RESPONSE_DEFAULT_LENGTH__             512  /* 单位：byte */
#endif


/* 定位信息长度 */
#if defined TZ_MSG_POSITION_LENGTH
#define __POSITION_DEFAULT_LENGTH__             TZ_MSG_POSITION_LENGTH
#else
#define __POSITION_DEFAULT_LENGTH__             96  /* 单位：byte */
#endif



/* 工作信息长度 */
#if defined TZ_MSG_WORK_PARAMS_LENGTH
#define __WORK_PARAMS_DEFAULT_LENGTH__          TZ_MSG_WORK_PARAMS_LENGTH
#else
#define __WORK_PARAMS_DEFAULT_LENGTH__          256  /* 单位：byte */
#endif



/* 固件信息长度 */
#if defined TZ_MSG_FIRMWARE_UPDATE_LENGTH
#define __FIRMWARE_UPDATE_DEFAULT_LENGTH__      TZ_MSG_FIRMWARE_UPDATE_LENGTH
#else
#define __FIRMWARE_UPDATE_DEFAULT_LENGTH__      32  /* 单位：byte */
#endif



#endif  /* __TZ_DEFINED_H__ */




