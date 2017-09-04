

#include "stm32f4xx.h"
#include "tiza_rtc.h"

#ifndef __TZ_GPS_H__
#define __TZ_GPS_H__




#define PRINTF_GPS_TIME()       Printf_TimeInfoDisplay()    

#define  GPS_MAX_RESTART_COUNTER   120

extern s8 GPS_WorkState;
extern u16 GPS_WorkStateCounter;

//#define GPS_0183					  	//GPS定位系统

#define BDS_Great_0183      	//北斗定位系统


/* RMC结构 */
typedef struct  {
	 
	/* 日期_年 */
	u32		date_yy;
    	
  /* 日期_月 */
	u32		date_mm;	
    	
	/* 日期_日 */
	u32		date_dd;	

  /* 时间_时 */ 
  u32		time_hh;		
  
  /* 时间_分 */
  u32		time_mm;		
  
  /* 时间_秒 */
  u32		time_ss;		
  
  /* 状态 */
  u8		status_;		
  
  /* 纬度_浮点格式 */
  float    fLat_;        
  
  /* 纬度 */
  u32    	lat_;			
  
  /* 南北纬 */
  u8		lat_NS_;		
  
  /* 经度_浮点格式 */
  float    fLong_;       
    
  /* 经度 */
	u32   	long_;			
    
  /* 东西经 */
	u8		long_EW_;	
    
  /* 高度 */
  s16   high_;    	
     	
  /* 卫星数 */
	u32   sat_num;

  /* 信噪比*/
    u8   snr[13];
       
  /* 速度 */
  double       speed_;   
  
  /* 方向 */
  double   direction_;   
} GPS_data;


/* GPS数据 */
extern GPS_data RMC_d;






/* 本地日期和时间信息 */
typedef struct  
{  
  /* 日期_年 */
  u8 date_yy;  
  
  /* 日期_月 */
  u8 date_mm;    
                             
  /* 日期_日 */
  u8 date_dd;   
  
  /* 时间_时 */ 
  s8 time_hh;   
         
  /* 时间_分 */
  s8 time_mm;
           
  /* 时间_秒 */
  s8 time_ss;  
    
} localDateTime_t; 


typedef struct
{
	

	/* 纬度 */
  u32    	lat_;

	/* 经度 */
  u32    	long_;
	/*经纬度是否有效*/
	bool isValid;

}gps_data_param_t;

extern gps_data_param_t  gps_data_param;



/******************************************/
/*              外部函数[声明]             */
/******************************************/

/* GPS应用接口初始化 */
extern void GPS_ApplicationIfInit ( void );

/* GPS周期事务处理 */
extern void GPS_ProcDataHdlr ( void );

/* GPS状态信息显示 */
extern void GPS_StatusInfoDisplay ( void );

/* 时间信息打印 */
extern void Printf_TimeInfoDisplay ( void );


#endif	/* __TZ_GPS_H__ */

