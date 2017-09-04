#include <time.h>
#include "stdlib.h"
#include "tiza_utility.h"
#include "gps.h"
#include "tiza_gps_port.h"
#include "tiza_include.h"
#include "tiza_params.h"
#include "tiza_gprs_protocol.h"
#include "tiza_rtc.h"
#include "tiza_storage.h"
#include "w25qxx_port.h"


/* GPS数据 */
GPS_data RMC_d={0};

TM_RTC_Time_t RTC_time={0};
/*GPS经纬度信息*/
gps_data_param_t  gps_data_param;

/*GPS工作状态指示器  */
s8 GPS_WorkState=-1;//0:无效  1：GPS有效

static tmr_t *tmrGps_WorkState = NULL;

u16 GPS_WorkStateCounter=0;

/*GPS对RTC校时计数器*/
static u16 rtc_time_counter=0;

/* LED-IO */
extern s32 led_io_handler;


///*外部变量*/
status_param drv_param = {1,0,0,0,0,0,0,0,0,0,0};


/******************************************/
/*              内部函数[声明]            */
/******************************************/

static void gprmc_callout ( nmeap_context_t *context, void *data, void *user_data );
static void gpgsv_callout ( nmeap_context_t *context, void *data, void *user_data );

#if 0
static void gprmc_callout ( nmeap_context_t *context, void *data, void *user_data );
static void gpgsv_callout ( nmeap_context_t *context, void *data, void *user_data );
static void gpgga_callout ( nmeap_context_t *context, void *data, void *user_data );
static void gpgsv_callout ( nmeap_context_t *context, void *data, void *user_data );
static void gpvtg_callout ( nmeap_context_t *context, void *data, void *user_data );
#endif





/******************************************/
/*              内部变量[定义]            */
/******************************************/

/* GPS调用 */
static GPS_DevTypedef gps = { //NULL,
                              gprmc_callout,
                              //NULL
                            };


/* 
 * 功能描述：
 * 引用参数：无
 *          
 * 返回值  ：状态码
 * 
 */
extern void GPS_ApplicationIfInit ( void )
{
  GPS_IfInit ( ( GPS_DevTypedef* )&gps );
}







extern void Gl868_M2mLocate_Request(u8 act_flag);
extern void TmrGprsHdlr_BasePosition ( u8 ID );
/**
 * do something with the RMC data
 */
static void gprmc_callout ( nmeap_context_t *context, void *data, void *user_data )
{
  NmeaRmc *rmc = (NmeaRmc *)data;
  struct tm* time;    
  //static u32 glong_, glat_;
  u8 state    = 0;
#if OS_CRITICAL_METHOD == 3                           /* Allocate storage for CPU status register      */
    OS_CPU_SR  cpu_sr = 0;
#endif

  if ( rmc != NULL )
  {
    RMC_d.lat_ = abs ( rmc->latitude );

    if ( rmc->latitude > 0 )
    {
      RMC_d.lat_NS_ = 'N';  
    } 
    else
    {
      RMC_d.lat_NS_ = 'S';  
    }
		
    RMC_d.long_ = abs( rmc->longitude );
		
    if ( rmc->longitude > 0 )
    {
      RMC_d.long_EW_ = 'E';
    }
    else
    {
      RMC_d.long_EW_ = 'W';
    }  
    RMC_d.status_ = rmc->warn;
    rmc->time += 28800;   /* 增加8小时 */ 
    time = localtime ( ( const time_t *)&rmc->time );
		//OS_ENTER_CRITICAL();
    if ( time != NULL )
    {
      RMC_d.time_hh = time->tm_hour;
      RMC_d.time_mm = time->tm_min; 
      RMC_d.time_ss = time->tm_sec;
      RMC_d.date_yy = time->tm_year - 100;
      RMC_d.date_mm = time->tm_mon + 1;
      RMC_d.date_dd = time->tm_mday; 						
			memcpy ( ( u8* )&locations,( u8* )&RMC_d, 25 );		
    }
		//OS_EXIT_CRITICAL (); 
  }


  if ( RMC_d.status_ == 'A' )
  {
			u32 long_=reverseU32(RMC_d.long_);
			u32 lat_=reverseU32(RMC_d.lat_);
		
			gps_data_param.lat_=lat_;
			gps_data_param.long_=long_;
			gps_data_param.isValid=FALSE;
			if( rmc->speed<137)
			{
					memcpy ( ( u8* )&locations.lon[ 0 ],( u8* )&long_, 4 );
					memcpy ( ( u8* )&locations.lat[ 0 ],( u8* )&lat_, 4 );
			}
			
			GPS_LED_ON;
			GPS_WorkStateCounter=0;
			GPS_WorkState=1;
			
		
			if(rtc_time_counter<1000)//上电后，RTC与GPS校时3次，校时间隔5分钟
			rtc_time_counter++;
			if(rtc_time_counter%300==2)
			{
					
					TM_RTC_Result_t res;
					TM_RTC_Time_t TM_RTC_GPS; 
					TM_RTC_GPS.hours= time->tm_hour;
					TM_RTC_GPS.minutes=time->tm_min;
					TM_RTC_GPS.seconds=time->tm_sec;
					TM_RTC_GPS.year= time->tm_year - 100;
					TM_RTC_GPS.month=time->tm_mon + 1;
					TM_RTC_GPS.date=time->tm_mday;
					TM_RTC_GPS.day= (time->tm_wday+6)%7+1;		
		#if __USE_RTC__==1
					OS_CPU_SR     cpu_sr;
				  OS_ENTER_CRITICAL();
					res=TM_RTC_SetDateTime(&TM_RTC_GPS,TM_RTC_Format_BIN);
				  if(res==TM_RTC_Result_Ok)
					{
						  //printf("\r\n[RTC EVENT]:RTC_DateTime SETTING IS OK......\r\n");
					}
					else
					{
						res=TM_RTC_SetDateTime(&TM_RTC_GPS,TM_RTC_Format_BIN);
						if(res!=TM_RTC_Result_Ok)
						{
						}
					}
					OS_EXIT_CRITICAL();
		#endif	
			}
  }
	else
	{
		
		
		
				if(sys_status>=PROC_SYSTEM_LOGIN_ACK&&sys_status<PROC_SYSTEM_LOGOUT&&gps_data_param.isValid==FALSE)
				{
					  gps_data_param.isValid=TRUE;
						STORAGE_FlashParams_WriteData(( u8 * )&gps_data_param ,sizeof( gps_data_param_t ),FLASH_GPS_PARAMS);
						//printf("\r\nglat_:%d  glong_:%d \r\n",gps_data_param.lat_,gps_data_param.long_);
				}
				if(sys_status == PROC_SYSTEM_LOGIN)
				{	
				
						//printf("\r\nlat_:%d  lon_:%d \r\n",gps_data_param.lat_ , gps_data_param.long_);
						memcpy ( ( u8* )&locations.lon[ 0 ],( u8* )&gps_data_param.long_, 4 );
						memcpy ( ( u8* )&locations.lat[ 0 ],( u8* )&gps_data_param.lat_, 4 );
				}

		
/*GPS故障处理，目前未使用*/	
#if 1	
			GPS_LED_OFF;
			rtc_time_counter=0;
			GPS_WorkState=0;

#else
			GPS_LED_OFF;
			GPS_WorkStateCounter++;
			rtc_time_counter=0;
			if(GPS_WorkStateCounter>=GPS_MAX_RESTART_COUNTER)
			{
					GPS_WorkState=0;
				  GPS_WorkStateCounter=GPS_MAX_RESTART_COUNTER;
			}
#endif
	}


}


/**
 * do something with the GSV data
 */
static void gpgsv_callout ( nmeap_context_t *context, void *data, void *user_data )
{
    NmeaGsv *gsv = (NmeaGsv *)data;
	u8 i = 0,j = 0;

	if( gsv != NULL )
	{
        RMC_d.sat_num = gsv->tot_svv;

				//printf("\r\n   gpgsv_callout----->   RMC_d.sat_num :%d \r\n",RMC_d.sat_num);
		
		
        if((RMC_d.sat_num > 0)&&(RMC_d.sat_num < 13))
        {
					for(j = RMC_d.sat_num; j < 13 - RMC_d.sat_num; j++)
					RMC_d.snr[j] = 0;
        }
		
		if((RMC_d.sat_num > 0)&&(RMC_d.sat_num <= 4))
		{
            if(1 == gsv->message_num )
			{
                 for(i = 0; i < RMC_d.sat_num; i++)
					  RMC_d.snr[i] = gsv->info[i].snr;
				 //printf("\r\n (%d)RMC_d.snr[0] = %d,RMC_d.snr[1] = %d,RMC_d.snr[2] = %d,RMC_d.snr[3] = %d\r\n",__LINE__,RMC_d.snr[0],RMC_d.snr[1],RMC_d.snr[2],RMC_d.snr[3]);
            }
		}
		else if((RMC_d.sat_num > 4)&&(RMC_d.sat_num <= 8))
		{
             if(1 == gsv->message_num )
             {
                 for(i = 0; i < 4; i++)
					  RMC_d.snr[i] = gsv->info[i].snr;
				 //printf("\r\n (%d)RMC_d.snr[0] = %d,RMC_d.snr[1] = %d,RMC_d.snr[2] = %d,RMC_d.snr[3] = %d\r\n",__LINE__,RMC_d.snr[0],RMC_d.snr[1],RMC_d.snr[2],RMC_d.snr[3]);
             }
			 else
			 {
                 for(i = 0; i < RMC_d.sat_num - 4; i++)
				      RMC_d.snr[i + 4] = gsv->info[i].snr;
				 //printf("\r\n (%d)RMC_d.snr[4] = %d,RMC_d.snr[5] = %d,RMC_d.snr[6] = %d,RMC_d.snr[7] = %d\r\n",__LINE__,RMC_d.snr[4],RMC_d.snr[5],RMC_d.snr[6],RMC_d.snr[7]);
			 }
		}
		else
		{
             if(1 == gsv->message_num )
             {
                 for(i = 0; i < 4; i++)
					  RMC_d.snr[i] = gsv->info[i].snr;
				  //printf("\r\n (%d)RMC_d.snr[0] = %d,RMC_d.snr[1] = %d,RMC_d.snr[2] = %d,RMC_d.snr[3] = %d\r\n",__LINE__,RMC_d.snr[0],RMC_d.snr[1],RMC_d.snr[2],RMC_d.snr[3]);
             }
			 else if(2 == gsv->message_num )
			 {
                 for(i = 0; i < RMC_d.sat_num - 4; i++)
				      RMC_d.snr[i + 4] = gsv->info[i].snr;
				 //printf("\r\n (%d)RMC_d.snr[4] = %d,RMC_d.snr[5] = %d,RMC_d.snr[6] = %d,RMC_d.snr[7] = %d\r\n",__LINE__,RMC_d.snr[4],RMC_d.snr[5],RMC_d.snr[6],RMC_d.snr[7]);
			 }
             else if(3 == gsv->message_num )
             {
                 for(i = 0; i < RMC_d.sat_num - 8; i++)
				     RMC_d.snr[i + 8] = gsv->info[i].snr;
				 //printf("\r\n (%d)RMC_d.snr[8] = %d,RMC_d.snr[9] = %d,RMC_d.snr[10] = %d,RMC_d.snr[11] = %d\r\n",__LINE__,RMC_d.snr[8],RMC_d.snr[9],RMC_d.snr[10],RMC_d.snr[11]);
             }
			 else
			 {
                 for(i = 0; i < RMC_d.sat_num - 12; i++)
				     RMC_d.snr[i + 12] = gsv->info[i].snr;
				 //printf("\r\n (%d)RMC_d.snr[12] = %d\r\n",__LINE__,RMC_d.snr[12]);
			 }
		}
			
		 
	}
}


#if 0
static void gpgga_callout ( nmeap_context_t *context, void *data, void *user_data )
{
  NmeaGga *gga = (NmeaGga *)data;

	struct tm* time;    
	

  if( gga != NULL )
	{
		gga->time += 28800;   /* 增加8小时 */ 
    time=localtime ( ( const time_t *)&gga->time );
		if ( time != NULL )
    {
      RMC_d.time_hh = time->tm_hour;
      RMC_d.time_mm = time->tm_min; 
      RMC_d.time_ss = time->tm_sec;
      RMC_d.date_yy = time->tm_year - 100;
      RMC_d.date_mm = time->tm_mon + 1;
      RMC_d.date_dd = time->tm_mday; 
			
 
    }
	
	}

}


/**
 * do something with the GGA data
 */
static void gpgga_callout ( nmeap_context_t *context, void *data, void *user_data )
{
//  NmeaGga *gga = (NmeaGga *)data;
}



/**
 * do something with the GSV data
 */
static void gpgsv_callout ( nmeap_context_t *context, void *data, void *user_data )
{
//  NmeaGsv *gsv = (NmeaGsv *)data;
}





/**
 * do something with the VTG data
 */
static void gpvtg_callout ( nmeap_context_t *context, void *data, void *user_data )
{
//  NmeaVtg *vtg = (NmeaVtg *)data;
}



#endif







void GPS_PowerResetHdlr(void)
{ 
		GPS_POWER_ON;
}
/* 
 * 功能描述：gps数据处理
 * 引用参数：
 *         
 * 返回值  ：无
 * 
 */
extern void GPS_ProcDataHdlr ( void )
{

/***********************
 *  GPS模块故障处理    *
 ***********************/
	//外完待续：判断GPS是否故障，输出GPS灯、GPS电源复位重启
	
	if(GPS_WorkState==0)
	{
			
#if 0
		GPS_WorkState=-1;
		GPS_WorkStateCounter=0;
		GPS_POWER_OFF;
		if(tmrGps_WorkState==NULL)
		tmrGps_WorkState=TMR_Subscribe(FALSE,200,TMR_TYPE_10MS,(tmr_procTriggerHdlr_t)GPS_PowerResetHdlr);
#endif
		
	}


 
}








/* 
 * 功能描述：状态信息显示
 * 引用参数：
 *         
 * 返回值  ：无
 * 
 */
extern void GPS_StatusInfoDisplay ( void )
{
	OS_CPU_SR     cpu_sr;
	OS_ENTER_CRITICAL();
	printf ( "\r\n RTC DATA: 20%u/%u/%u, %u:%u:%u, Week:%d  \r\n", 
           RTC_time.year, RTC_time.month, RTC_time.date,
           RTC_time.hours, RTC_time.minutes, RTC_time.seconds,
           RTC_time.day );
	OS_EXIT_CRITICAL();

}











/* 
 * 功能描述：时间显示
 * 引用参数：
 *         
 * 返回值  ：无
 * 
 */
extern void Printf_TimeInfoDisplay ( void )
{
  OS_CPU_SR     cpu_sr;
	OS_ENTER_CRITICAL();
  printf ( "\r\nGPS DATA: 20%u/%u/%u, %u:%u:%u, %u%c, %u%c ,%c  \r\n", 
           RMC_d.date_yy, RMC_d.date_mm, RMC_d.date_dd,
           RMC_d.time_hh, RMC_d.time_mm, RMC_d.time_ss,
           RMC_d.long_, RMC_d.long_EW_, RMC_d.lat_, RMC_d.lat_NS_, RMC_d.status_);	
	
	printf ( "RTC DATA: 20%u/%u/%u, %u:%u:%u, Week:%d  \r\n", 
           RTC_time.year, RTC_time.month, RTC_time.date,
           RTC_time.hours, RTC_time.minutes, RTC_time.seconds,
           RTC_time.day );
	OS_EXIT_CRITICAL();

}
