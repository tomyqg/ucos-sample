#include <stdio.h>
#include "tiza_params.h"
#include "wip_tcp.h"
#include "xmit_manager.h"
#include "rexmit_strategy.h"
#include "tiza_gprs_protocol.h"
#include "gb_gprs_protocol.h"
#include "buffer_helper.h"
#include "tiza_storage.h"
#include "gb_data_type.h"
#include "modem.h"
#include "tiza_rtc.h"
#include "w25qxx_port.h"
#include "tiza_init.h"
#include "tiza_utility.h"
#include "wip_dns.h"

/*外部变量*/
extern status_param drv_param;

extern bool MDM_SWITCH_TO;

/*
*====================================================================================================
* 全局变量区
*====================================================================================================
*/
//登录流水号
u16 login_serial=0;

/*3级报警动作  1:报警持续发生 0：报警消除     */
u16 alarm_alive = 0;


/******************************************/
/*              内部变量[定义]            */
/******************************************/	

/* GPRS时间设置 */
gprs_tmr_conf_t gprsTmr = {0};

/* 定义工作对象 */
static worker_t tzWorker;



/***************/
/* 定时器配置  */
/***************/

/* GPRS传输定时器句柄 */
tmr_t *tmrGprsProc = NULL;
tmr_t *tmrRealData = NULL;

tmr_t *tmrOffsetData = NULL;
tmr_t *tmrDeadData = NULL;
tmr_t *tmrCrrectTime = NULL;


/******************************************/
/*              内部函数[声明]            */
/******************************************/

static void sysGprsTmrInit ( void );
static void Tmr_GprsApp ( u8 ID );
static void tzWorker_DoTasks ( void* data );




/**
*===============================================================================================
*	计数器相关
*===============================================================================================
*/
/*实时数据时间计数器*/
static vu32 upload_timecount=0;

/*30秒报警计时器*/
static vu16 alarm_timecount=0;

/*心跳计数器 */
static u16 Heart_Counter=0;
static u16 Reissue_Counter=0;

/*应答服务计数器*/
static s32 sysLoginCount =0;
static s32 sysRealDataCount =0;
static s32 sysHeartbeat=0;

/*车辆登出计数器*/
static u16 logout_counter=0;
static u16 LogoutFlag=0;


/*
*==============================================================================================
*盲区数据补发：用于测试时候，盲区数据不够时，追加补发数据
*==============================================================================================
*/
/*盲区数据追加条数*/
static s8 flashbuf_2index=1;  
static u8 OffsetData_time_hh=0;
static u8 OffsetData_time_mm=0;
static u8 OffsetData_time_ss=0;


/*实时数据上传开关：出现3级报警时，实时数据上传时间(周期<=1s）并持续30后恢复；同时开启补发机制：补发故障前30s内的数据（周期30s一发）*/
u8 upload_switch=0;

//3级报警补发开关
s8 ReissueData_Upload_Switch=-1;


//盲区补发时，保证实时数据先于补发数据
static u8 realdata_send_first=0;
static s8 DeadData_Sem=-1;

/*车辆是否处于盲区*/
s8 IsDeadZone_Network=-1;


/*
*==============================================================================================
* 相关标志组
*==============================================================================================
*/

//FIFO初始化标志  
static s8 FIFO_Init_flag =-1 ;
//3级报警触发标志
u8 upload_switch_state=0;

//车辆登出标志

s8 logout_flag=-1;


/*
*==============================================================================================
*	SD卡存储相关
*==============================================================================================
*/
/*SD卡文件系统，文件名*/
char SD_filename[30];

#if  SD_WRITE_ENABLE==1
static u8 SD_RealData_HexToAscii[2*REALDATA_UPLOAD_BUFFER_LEN+2];
static u8 SD_ReissueData_HexToAscii[2*REALDATA_UPLOAD_BUFFER_LEN+2];
static u8 SD_Heartbeet_HexToAscii[60];
#endif



/*
*===================================================================================================
* FIFO、数据变量区
*===================================================================================================
*/
/*补发临时变量*/
static u8 Offsetdata_ss[REALDATA_UPLOAD_BUFFER_LEN]={0};
static u8 Offsetdata_dd[REALDATA_UPLOAD_BUFFER_LEN]={0};

/* 30s报警缓存 */
static u8 FIFO_Buffer[ BUFFER_FIFO_SIZE] = { 0 };
/*环形队列的缓存*/
static u8 FIFO_temp[REALDATA_UPLOAD_BUFFER_LEN]={0};

/*FIFO存储开关： -1：存储数据 1：停止存储数据*/
static s8 FIFO_switch=-1;

/*
*===========================================================================================
* 32960协议命令
*============================================================================================
*/


/*
*=======================================
* 宏定义区
*/
/* 实时数据发送频率：10s  */
#define REALDATA_SEND_PERIOD_TIME       10


/*
*=======================================
* 命令初始化区
*/
/* 登录信息 */
static u8 *msgLogin              = NULL;
/* 登出信息 */
static u8 *msgLogout             = NULL;
/* 实时数据上传 */
static u8 *msgRealData_upload    = NULL;
/* 盲区数据存储 */
static u8 *msgDeadData_Strorage  = NULL;
/* 终端校时 */
static u8 *msgCorrectionTime     = NULL;
/* 心跳信息 */
static u8 *msgHeartBeat          = NULL;
/* 参数查询应答*/
static u8* msgParametersQuery    = NULL;
/* 登陆超时处理 */
static s32 msgLoginHandler            = 0;
/* 登出超时处理 */
static s32 msgLogoutHandler           = 0;



/*
*=======================================
* 盲区数据相关变量
*/

/*盲区配置 */
static s8 DeadData_FlashConfig=-1;
static u16 DeadData_lenth=0;

/*盲区时间--> 产生报警*/
static u8 Alarm_AtDeadtime=0;				//1：产生报警
/*盲区时间--> 产生报警时，盲区已存条数  */
static u32 FlashDeadStoragedIndex_AlarmPoint=0;

/*用于盲区时，产生报警，记录实时数据存储条数*/
static u16 AlarmReal_AtDeadTimeCounter=0;

/*重复数据数目 */
#define DROP_REPEAT_DEADDATA_NUM    3

/*去除重复数据 */
static u8 Drop_RepeatDeadData_Count=DROP_REPEAT_DEADDATA_NUM;


enum  DEADDATA_PTHREAD 
{
	NO_THREAD=0,
	//FLASH线程发送数据
	THREAD_FLASH_SENF=1,
	
	//FIFI线程发送数据
	THREAD_FIFO_SENF,

  /* 大小已经限定，请勿任意修改！ */
  THREAD_OBJ_NB = 3
};

static enum DEADDATA_PTHREAD  Dead_Pthread_states=NO_THREAD;


/*
*=======================================
* 3级报警相关
*/
/* 补发的时间戳校准*/
static u8 offset_data_time_hh=0;
static u8 offset_data_time_mm=0;
static u8 offset_data_time_ss=0;
/* 实时数据的时间戳校准*/
static u8 real_data_time_hh=0;
static u8 real_data_time_mm=0;
static u8 real_data_time_ss=0;


/*
*===========================================================================================
* 32960协议相关功能处理函数
*============================================================================================
*/
/*协议处理相关函数*/
static u16  CAL_DataUnit_length(u8 *buf);
static u8 	BCC_CheckSum(u8 *buf, u16 len);
static u16  Gprs_ProtocolDataEncode ( u8 *data, u16 len );

static u16 data_pack_head(u8 order);//协议的命令包头
/* 车辆登录流水序列号 */
static u16  vehicles_login_serial_number ( void );

static u16 DeadData_Strorage(u16 Counter,u16 StorageCycleTime,u8 alarmstate);
static u16 DeadDataUpload_FromFlash(void);

static void Gprs_VehicleTerminalControl(u16 len,u8* data);
static bool Gprs_VehicleTerminalControl_Response(u16 len,u8* data);

static void Gprs_ParametersQuery(u16 len,u8* data);
static void Gprs_ParametersQuery_Response(u16 len,u8* data);

static void Gprs_ParametersSet(u16 len,u8* data);
static void Gprs_ParametersSet_Response(u16 len,u8* data);

void Set_Comm_Warning(u8 warn_sign,u8 grade);


/*
*===========================================================================================
* 外部函数
*============================================================================================
*/
extern void Gprs_request_time(void);
extern void Gprs_transfer_satellite_version_msg(void);
extern void GL868_Command_UseSyncLED ( void* ipc_data, u8 mode, u8 on_duration, u8 off_duration );









void DeadData_FlashConfigInit(void)
{
		
		DeadData_FlashConfig=-1;
		DeadData_Sem=0;

}


static void Tmr_GprsApp ( u8 ID )		
{
  WORKER_StartupWork(( worker_t *)&tzWorker );
}






/******************************************/
/*              定时器设置响应部分        */
/******************************************/

/* 
 * 功能描述：设置GPRS定时器
 * 引用参数：
 *     
 * 返回值  ：
 * 
 */
static void sysGprsTmrInit ( void )
{
  /* 初始化工作对象 */
  tzWorker.data = NULL;
  tzWorker.func = tzWorker_DoTasks;  
}



/* 
 * 功能描述：心跳
 * 引用参数：
 *     
 * 返回值  ：
 * 
 */
void TmrHeartBeatHdlr(u8 times)    //默认单位：30s
{
		Gprs_Heart();
}
/* 
 * 功能描述：实时数据
 * 引用参数：
 *     
 * 返回值  ：
 * 
 */
void TmrRealDataHdlr(u8 times)    //默认单位：1s
{

		if(upload_switch_state==0||Alarm_AtDeadtime==1)
		{
				if(upload_switch_state==1||upload_switch==1)
				{
						alarm_alive=1;//报警状态保活，需手动清除		
				}
				if(upload_switch==0)
				{
						++upload_timecount;
						if(upload_timecount==REALDATA_SEND_PERIOD_TIME)realdata_send_first=1;
						
						Multi_realdata_upload(upload_timecount,REALDATA_SEND_PERIOD_TIME,0);

						if(realdata_send_first==1)
						{
								DeadData_Sem=1;//作用：盲区时，实时数据优先级大于盲区数据	
								if(pRingBuf->Counter&&Dead_Pthread_states==NO_THREAD )Dead_Pthread_states=THREAD_FLASH_SENF;
						}
				}
				else//进入3级报警
				{
							alarm_timecount++;
							upload_timecount=0;//停止计数
							if(alarm_timecount<=29)//进入报警-->实时数据开始一秒一发
							{
									Multi_realdata_upload(upload_timecount,0,TRUE);//发送周期：1s
									//ReissueData_Upload_Switch=1;//3级报警一秒两条
							}
							else	//30s后，退出报警状态
							{
									ReissueData_Upload_Switch=1;//3级报警一秒一条
									alarm_timecount=0;
									upload_switch=0;
									printf("\r\n[3 LEVEL ALARM]: RealData-->Send Finish  >>>QUIT \r\n");
							}
				}
		}
		else
		if(upload_switch_state==1&&Alarm_AtDeadtime==0)
		{
			if(GPS_WorkState==1)
						printf("\r\n[3 LEVEL ALARM EVENT]: USE GPS Current Time >>>  %02u:%02u:%02u  \r\n",RMC_d.time_hh,RMC_d.time_mm,RMC_d.time_ss);
				else
						printf("\r\n[3 LEVEL ALARM EVENT]: USE RTC Current Time >>>  %02u:%02u:%02u  \r\n",RTC_time.hours,RTC_time.minutes,RTC_time.seconds);					

#if TIME_BIAS_ENABLE==1		
				
	#if SEND_GRADEDOWN	== 1  
			  if(GPS_WorkState==1)
				{
						offset_data_time_hh=RMC_d.time_hh;
						offset_data_time_mm=RMC_d.time_mm;
						offset_data_time_ss=RMC_d.time_ss;
				}
				else
				{
						offset_data_time_hh=RTC_time.hours;
						offset_data_time_mm=RTC_time.minutes;
						offset_data_time_ss=RTC_time.seconds;
				}
	#else	
				if(GPS_WorkState==1)
				{
						offset_data_time_ss=(RMC_d.time_ss+30)%60;//时间戳增序发送
						if(RMC_d.time_ss<=30)
						{
								offset_data_time_mm=RMC_d.time_mm;
								if(RMC_d.time_mm==0)
								{
										offset_data_time_mm=59;
										if(RMC_d.time_hh==0)
												offset_data_time_hh=23;
										else
												offset_data_time_hh=RMC_d.time_hh;
								}
								else
								{
										offset_data_time_mm--;
										offset_data_time_hh=RMC_d.time_hh;
								}
						}
						else
						{
							offset_data_time_mm=RMC_d.time_mm;
							offset_data_time_hh=RMC_d.time_hh;
						}
				}
				else
				{
					  offset_data_time_ss=(RTC_time.seconds+30)%60;//时间戳增序发送
						if(RTC_time.seconds<=30)
						{
								offset_data_time_mm=RTC_time.minutes;
								if(RTC_time.minutes==0)
								{
										offset_data_time_mm=59;
										if(RTC_time.hours==0)
												offset_data_time_hh=23;
										else
												offset_data_time_hh=RTC_time.hours;
								}
								else
								{
										offset_data_time_mm--;
										offset_data_time_hh=RTC_time.hours;
								}
						}
						else
						{
							offset_data_time_mm=RTC_time.minutes;
							offset_data_time_hh=RTC_time.hours;
						}
				}
				printf("\r\n TIME: %d:%d:%d \r\n",offset_data_time_hh,offset_data_time_mm,offset_data_time_ss);
				
	#endif  /* SEND_GRADEDOWN */
				if(GPS_WorkState==1)
				{
						real_data_time_hh=RMC_d.time_hh;
						real_data_time_mm=RMC_d.time_mm;
						real_data_time_ss=RMC_d.time_ss;
				}
				else
				{
						real_data_time_hh=RTC_time.hours;
						real_data_time_mm=RTC_time.minutes;
						real_data_time_ss=RTC_time.seconds;
				}
#endif			  
			  FIFO_switch=1; //FIFO停止存储数据 ,准备补发；
				Multi_realdata_upload(upload_timecount,0,0);
				upload_switch=1;
			  upload_switch_state=0;	
		
		}
	
}

/* 
 * 功能描述：补发数据
 * 引用参数：
 *     
 * 返回值  ：
 * 
 */

void TmrOffsetDataHdlr(u8 times)    //默认单位：1s
{
	/*upload_switch!=-1:出现3级报警；FIFO_switch==-1：允许数据存储 */
	/*环形缓存数据补发（出现3级报警，补发前30s数据）*/
	if(ReissueData_Upload_Switch==1)
	{
				 if(pRingBuf->Counter)	//补发数据
				 {
							printf("\r\n[3 LEVEL ALARM EVENT]: ReissueData-->Sending   %d   ",pRingBuf->Counter);

							FIFO_GetOne(pRingBuf,(u8*)Offsetdata_ss);

							Offset_data_upload_tcp(Offsetdata_ss);
							
							#if SD_WRITE_ENABLE==1
							char ReissueData[60];
							sprintf(ReissueData,"\r\n\r\nReissueData SendTime  20%02u%02u%02u %02u:%02u:%02u\r\n",RTC_time.year, RTC_time.month, RTC_time.date,RMC_d.time_hh,RMC_d.time_mm,RMC_d.time_ss);
							SD_WriteFile(&file,SD_filename,(u8*)ReissueData,strlen(ReissueData));
						  
							HexToAscii(SD_ReissueData_HexToAscii,Offsetdata_ss,DeadData_lenth,true);
							SD_WriteFile(&file,SD_filename,SD_ReissueData_HexToAscii,2*DeadData_lenth+2);
							#endif
					 
				 }
				 else if(pRingBuf->Counter<=0)  //退出补发状态
				 {
							printf("\r\n[3 LEVEL ALARM EVENT]: ReissueData-->Send Finish  >>QUIT \r\n");
							FIFO_switch=-1;  //FIFO恢复存储数据
							ReissueData_Upload_Switch=0;		
				 }
	} 
  //盲区时产生3级报警
  if(Dead_Pthread_states==THREAD_FIFO_SENF)
	{
				 if(pRingBuf->Counter)	//补发数据
				 {
							printf("\r\n[3 LEVEL ALARM EVENT]: ReissueData-->Sending   %d   ",pRingBuf->Counter);

							FIFO_GetOne(pRingBuf,(u8*)Offsetdata_ss);
							Offsetdata_upload_DeadTime(Offsetdata_ss);
							
							#if SD_WRITE_ENABLE==1
							char ReissueData[60];
							sprintf(ReissueData,"\r\n\r\nReissueData SendTime  20%02u%02u%02u %02u:%02u:%02u\r\n",RTC_time.year, RTC_time.month, RTC_time.date,RMC_d.time_hh,RMC_d.time_mm,RMC_d.time_ss);
							SD_WriteFile(&file,SD_filename,(u8*)ReissueData,strlen(ReissueData));
						  
							HexToAscii(SD_ReissueData_HexToAscii,Offsetdata_ss,DeadData_lenth,true);
							SD_WriteFile(&file,SD_filename,SD_ReissueData_HexToAscii,2*DeadData_lenth+2);
							#endif
				 }
				 else if(pRingBuf->Counter<=0)  //退出补发状态
				 {
							//printf("\r\n[3 LEVEL ALARM EVENT]: ReissueData-->Send Finish  >>QUIT \r\n");
							FIFO_switch=-1;  //FIFO恢复存储数据	
							//补发数据切换到flash发送线程
							Dead_Pthread_states=THREAD_FLASH_SENF;
				 }
	 }		
}



/*
*******************************************************************
*函数名：DeadDataUpload_FromFlash
*参  数：无
*功  能：盲区数据发送
*创建人: gl
* 
*******************************************************************
*/
u16 DeadDataUpload_FromFlash(void)
{
	
	 /*FLASH补发（3天内，TCP异常的存储数据补发）*/
	 //补发数据
	 if(pFlashBuf->Counter)
	 { 	
		  if(Alarm_AtDeadtime==1)
			{					

						if(
								pFlashBuf->Head>=FlashDeadStoragedIndex_AlarmPoint&&\
								pFlashBuf->Head<(FlashDeadStoragedIndex_AlarmPoint+DROP_REPEAT_DEADDATA_NUM-1)
							)
						{
								Dead_Pthread_states=THREAD_FIFO_SENF;
							  while(Drop_RepeatDeadData_Count)
								{
										Drop_RepeatDeadData_Count--;
										FIFO_flash_GetOne(pFlashBuf, Offsetdata_dd);
								}	
						}
						if(Dead_Pthread_states==THREAD_FLASH_SENF)
						{ 
								FIFO_flash_GetOne(pFlashBuf, Offsetdata_dd);
								printf("\r\npFlashBuf->Head: %d \r\n",pFlashBuf->Head);
								printf("\r\npFlashBuf->Tail: %d \r\n",pFlashBuf->Tail);
								printf("\r\n[FLASH Reissue EVENT]: Sending >>>  TimeStamp: %02u:%02u:%02u  \r\n ",Offsetdata_dd[27],Offsetdata_dd[28],Offsetdata_dd[29]);
								OffsetData_Upload(Offsetdata_dd);
						}
			}
			else
			{
						printf("\r\npFlashBuf->Head: %d \r\n",pFlashBuf->Head);
						printf("\r\npFlashBuf->Tail: %d \r\n",pFlashBuf->Tail);
						FIFO_flash_GetOne(pFlashBuf, Offsetdata_dd);
						printf("\r\n[FLASH Reissue EVENT]: Sending >>>  TimeStamp: %02u:%02u:%02u  \r\n ",Offsetdata_dd[27],Offsetdata_dd[28],Offsetdata_dd[29]);
						OffsetData_Upload(Offsetdata_dd);
			}
		 
			#if SD_WRITE_ENABLE==1
			char ReissueData[60];
			sprintf(ReissueData,"\r\n\r\nReissueData SendTime  20%02u%02u%02u %02u:%02u:%02u\r\n",RTC_time.year, RTC_time.month, RTC_time.date,RMC_d.time_hh,RMC_d.time_mm,RMC_d.time_ss);
			SD_WriteFile(&file,SD_filename,(u8*)ReissueData,strlen(ReissueData));
			
			HexToAscii(SD_ReissueData_HexToAscii,Offsetdata_dd,DeadData_lenth,true);
			SD_WriteFile(&file,SD_filename,SD_ReissueData_HexToAscii,2*DeadData_lenth+2);
			#endif
			if(pFlashBuf->Counter==0)
			{
					//对补发数据时间戳校准
					OffsetData_time_hh=Offsetdata_dd[27];
					OffsetData_time_mm=Offsetdata_dd[28];
					OffsetData_time_ss=Offsetdata_dd[29];
					
			}
	 }
	 else if(pFlashBuf->Counter==0&&flashbuf_2index>0) //追加补发：车辆过检使用
	 {
			if(flashbuf_2index--)
			{
					printf("\r\n[FLASH Reissue EVENT]:  flash index(ADD To): %d",flashbuf_2index);
					OffsetData_UploadEx(Offsetdata_dd);
					#if SD_WRITE_ENABLE==1
					char ReissueData[60];
					sprintf(ReissueData,"\r\n\r\nReissueData SendTime  20%02u%02u%02u %02u:%02u:%02u\r\n",RTC_time.year, RTC_time.month, RTC_time.date,RMC_d.time_hh,RMC_d.time_mm,RMC_d.time_ss);
					SD_WriteFile(&file,SD_filename,(u8*)ReissueData,strlen(ReissueData));
		
					HexToAscii(SD_ReissueData_HexToAscii,Offsetdata_dd,DeadData_lenth,true);
					SD_WriteFile(&file,SD_filename,SD_ReissueData_HexToAscii,2*DeadData_lenth+2);
					#endif
			}
	 }
	 else
	 {
				flashbuf_2index=1;
				pFlashBuf->IsFinishSend=TRUE;
				/*盲区发送完成后，清除flash信息区，防止下次上电重复补发*/
				//FIFO_FLASH_OffsetDataConfig_Clear();
				Alarm_AtDeadtime=0;
				upload_switch_state=0;
	 }
}


/* 
 * 功能描述：终端校时
 * 引用参数：
 *     
 * 返回值  ：
 * 
 */
void TmrCorrectTimeHdlr(u8 times)    //默认单位：60s
{

}






/* 
 * 功能描述：启动应用
 * 引用参数：无
 *           
 * 返回值  ：无
 * 
 */
extern void App_tmrStart ( void )
{
	
#if  1
		printf("\r\n[APP EVENT]: Everything is ok, ME  is  Run......   .  \r\n");	
#endif
	
//		printf ( "\r\n[STORAGE EVENT]: Flash Storage Init....\r\n");
			FIFO_flash_init(pFlashBuf,DeadData_lenth,Dead_DATA_STORAGE_INDEX);
//		/*读取盲区配置*/
//		FIFO_FLASH_OffsetDataConfig_Read((u8*)&FIFO_FlashBuf,sizeof(FIFO_FLASH_TYPE));
	

#if 0	
		sysGprsTmrInit ();
		if ( tmrGprsProc == NULL )
		{
			tmrGprsProc = TMR_Subscribe ( TRUE, 100, TMR_TYPE_10MS, ( tmr_procTriggerHdlr_t )Tmr_GprsApp );
		}
#else
		if ( tmrGprsProc == NULL )
		{
			tmrGprsProc = TMR_Subscribe ( TRUE, 100, TMR_TYPE_10MS, ( tmr_procTriggerHdlr_t )tzWorker_DoTasks );
		}		
#endif

}

/* 
 * 功能描述：关闭应用
 * 引用参数：无
 *     
 * 返回值  ：无
 * 
 */
extern void App_tmrStop ( void )
{
  if ( tmrGprsProc )
  {
    TMR_UnSubscribe ( tmrGprsProc, ( tmr_procTriggerHdlr_t )Tmr_GprsApp, TMR_TYPE_10MS );
    tmrGprsProc = NULL;
  }
}




/* 
 * 功能描述: 域名解析
 * 引用参数：
 *         
 * 返回值  ：无
 * 
 */
//wip_DNSQueryHostIPAddress("www.baidu.com",(dns_procDataHdlr_f)Dns_procDataHdlr_f);
dns_procDataHdlr_f  Dns_procDataHdlr_f(const char *name, u32 *ipaddr)
{

	u32* ipaddress= ipaddr;
	
	printf("\r\n%s\r\n",name);
	
	printf("\r\n%d\r\n",*ipaddr);

}



tmr_t *RealData_Handler=NULL;
/* 
 * 功能描述：终端上传数据
 * 引用参数：
 *         
 * 返回值  ：无
 * 
 */
static void tzWorker_DoTasks ( void* data )
{

		/*****************************/
		/*      看门狗               */
		/*****************************/
		//喂狗
		WDG_KickCmd();
		__RestartGprsCounter=0;
		/**********************************
		 *       终端启动登录过程         *
		 **********************************/
#define LOGIN_TIMEOUT    1	
	
#if LOGIN_TIMEOUT == 1
		/*****************************/
		/*     服务器登陆超时服务    */
		/*****************************/
		#define SYS_LOGIN_MAX_TIMEOUT     100   /* 单位：s */
		extern bool SIO_IsOK ( void );
		if(sys_status<PROC_SYSTEM_SLEEP)
		{
				if ( SIO_IsOK () == TRUE )
				{
						sysLoginCount = 0;						
				}
				else
				{	
						sysLoginCount++;
						if ( sysLoginCount >= SYS_LOGIN_MAX_TIMEOUT )
						{
							/* 链路层出现故障 */
							/*******************************/
							/*     需要考虑进步的操作      */
							/*******************************/
							//sysParamsSave();
							printf("\r\n[LOGIN EVENT]: LOGIN SERVICE TIMEOUT(100s)......\r\n");
							System_Reset();  
						}  
			  }
		}
#endif

	
#define REALDATA_TIMEOUT     		0
#if REALDATA_TIMEOUT == 1
		/*****************************/
		/*    实时数据应答超时       */
		/*****************************/
		if(sys_status >= PROC_SYSTEM_LOGIN_ACK&&sys_status<PROC_SYSTEM_LOGOUT)	
		{	
				/*****************************/
				/*     实时数据应答检测      */
				/*****************************/
				#define SYS_REALDATA_MAX_TIMEOUT     20   /* 单位：s */ 
				sysRealDataCount++;
				if(sysRealDataCount >= SYS_REALDATA_MAX_TIMEOUT)
				{
						if(RealData_Handler==NULL)
						{
								/*实时数据无返回*/
								/* 1.关闭modem*/
								Modem_Stop(PROC_SYSTEM_REISSUEDATA_STORAGE);
								/* 2.重启modem */
								RealData_Handler=TMR_Subscribe ( FALSE, 20, TMR_TYPE_100MS, ( tmr_procTriggerHdlr_t )Modem_Restart );
								//System_Reset();  
						}
				} 		
		}
#endif
	
#define HEARTBEAT_TIMEOUT     		1	
#if HEARTBEAT_TIMEOUT == 1
		/*****************************/
		/*    心跳数据应答超时       */
		/*****************************/
		if(sys_status >= PROC_SYSTEM_LOGIN_ACK&&sys_status<PROC_SYSTEM_LOGOUT)	
		{	
				/*****************************/
				/*     心跳数据应答检测      */
				/*****************************/
				#define SYS_HEARTDATA_MAX_TIMEOUT     60   /* 单位：s */ 
				sysHeartbeat++;
				if ( sysHeartbeat >= SYS_HEARTDATA_MAX_TIMEOUT )
				{
						if(RealData_Handler==NULL)
						{
								printf("\r\n HeartBeat Check......  r\n");
								/*实时数据无返回*/
								/* 1.关闭modem*/
								Modem_Stop(PROC_SYSTEM_REISSUEDATA_STORAGE);
								/* 2.重启modem*/
								RealData_Handler=TMR_Subscribe ( FALSE, 20, TMR_TYPE_100MS, ( tmr_procTriggerHdlr_t )Modem_Restart );
						} 
				} 		
		}
	
#endif	
 


/*****************************/
/*         重传              */
/*****************************/




/*****************************/
/*         盲区数据          */
/*****************************/

/*
*	存储盲区数据
*/
if(IsDeadZone_Network==PROC_SYSTEM_REISSUEDATA_STORAGE)
{ 
		#define  REISSUEDATA_COUNT    10    //盲区结构体存储周期20s
		Reissue_Counter++;
	  
		if(upload_switch_state==1&&Alarm_AtDeadtime==0) 
		{
			FlashDeadStoragedIndex_AlarmPoint=pFlashBuf->Tail-3;
			printf("\r\nFlashDeadStoragedIndex_AlarmPoint: %d \r\n",FlashDeadStoragedIndex_AlarmPoint);
			Alarm_AtDeadtime=1;
			
		}
		/*
	  *===========================================================================
		*	存储盲区数据  存储周期：10s
	  *===========================================================================
		*/
		DeadData_Strorage(Reissue_Counter,REISSUEDATA_COUNT,Alarm_AtDeadtime);
	  
	
	  /*
		* 存储盲区数据结构体	周期：10s
		*/
		if(Reissue_Counter>=REISSUEDATA_COUNT)
		{
				Reissue_Counter=0;
				printf("\r\npFlashBuf->Head: %d \r\n",pFlashBuf->Head);
				printf("\r\npFlashBuf->Tail: %d \r\n",pFlashBuf->Tail);
				printf ( "\r\n[STORAGE EVENT]: FLASH STORAGE---> SEQUENCE: %d \r\n",FIFO_FlashBuf.Counter);
			  FIFO_FlashBuf.IsFinishSend=FALSE;
				FIFO_FLASH_OffsetDataConfig_Write((u8*)&FIFO_FlashBuf,sizeof(FIFO_FLASH_TYPE));		
		}
}

/*
*	读取盲区配置信息
*/
if(DeadData_FlashConfig==-1&&DeadData_Sem==1)
{
		DeadData_FlashConfig=1;
		DeadData_Sem=0;
		/*读取盲区配置*/
		FIFO_FLASH_OffsetDataConfig_Read((u8*)&FIFO_FlashBuf,sizeof(FIFO_FLASH_TYPE));
	  /*是否补发盲区数据*/
		if(FIFO_FlashBuf.offsetdata==0x8080)
		{
				/*需要补发数据*/
				//擦除flash补发配置信息; 作用：防止每次上电后，重复读取旧的配置信息，产生错误补发		

				FIFO_FLASH_OffsetDataConfig_Clear();
				if(tmrDeadData==NULL)
				{
						tmrDeadData=TMR_Subscribe(TRUE,300,TMR_TYPE_10MS,(tmr_procTriggerHdlr_t)DeadDataUpload_FromFlash);			 
  			}
		}
		else
		{	
				printf ( "\r\n[STORAGE EVENT]: Flash Storage Init....\r\n");
				FIFO_flash_init(pFlashBuf,DeadData_lenth,Dead_DATA_STORAGE_INDEX);
		}
}

//盲区补发结束，注销定时器，退出补发
if(FIFO_FlashBuf.offsetdata==0x8080&&FIFO_FlashBuf.IsFinishSend==TRUE)
{
		if(tmrDeadData)
		{
				/*盲区发送结束重新初始化*/
				printf ( "\r\n[STORAGE EVENT]:: Flash Storage Init....\r\n");
				FIFO_flash_init(pFlashBuf,DeadData_lenth,Dead_DATA_STORAGE_INDEX);
				TMR_UnSubscribe(tmrDeadData,( tmr_procTriggerHdlr_t )DeadDataUpload_FromFlash,TMR_TYPE_10MS);
				tmrDeadData=NULL;
		}
}	
//盲区补发中，再次进入盲区
if(IsDeadZone_Network==PROC_SYSTEM_REISSUEDATA_STORAGE)
{
		if(tmrDeadData)
		{
				/*注销盲区补发*/
				TMR_UnSubscribe(tmrDeadData,( tmr_procTriggerHdlr_t )DeadDataUpload_FromFlash,TMR_TYPE_10MS);
				tmrDeadData=NULL;
		}
}
	
		
/*****************************/
/*         心跳数据          */
/*****************************/  
#define  HEART_BEAT_COUNT    30    //心跳间隔
if(sys_status>=PROC_SYSTEM_LOGIN_ACK&&sys_status<PROC_SYSTEM_LOGOUT)
{
		
		Heart_Counter++;
		if(Heart_Counter>=HEART_BEAT_COUNT)
		{
				Heart_Counter=0;
				TmrHeartBeatHdlr(0);		
		}	
}

	
/**********************************************/
/*         实时数据发送（32960）              */
/**********************************************/

	if (sys_status>=PROC_SYSTEM_LOGIN_ACK&&sys_status<PROC_SYSTEM_LOGOUT) //车辆登录成功后，开启定时器 10s
	{
			if(tmrRealData==NULL)
			tmrRealData = TMR_Subscribe ( TRUE, 100, TMR_TYPE_10MS, ( tmr_procTriggerHdlr_t )TmrRealDataHdlr );			 
	}
	else
	if(sys_status<PROC_SYSTEM_LOGIN_ACK||sys_status>=PROC_SYSTEM_LOGOUT)
	{
			if(tmrRealData)
			TMR_UnSubscribe(tmrRealData,( tmr_procTriggerHdlr_t )TmrRealDataHdlr,TMR_TYPE_10MS);
			tmrRealData=NULL;
	}


/**********************************************/
/*         3级报警数据发送（32960）           */
/**********************************************/
	if (sys_status>=PROC_SYSTEM_LOGIN_ACK) //车辆登录成功后，开启定时器 3s
	{
			if(tmrOffsetData==NULL)
			tmrOffsetData = TMR_Subscribe ( TRUE, 100, TMR_TYPE_10MS, ( tmr_procTriggerHdlr_t )TmrOffsetDataHdlr );			 
	}
	else 
	if(sys_status<PROC_SYSTEM_LOGIN_ACK) //预留：当补发数据发送结束，注销定时器
	{
			if(tmrOffsetData)
			TMR_UnSubscribe(tmrOffsetData,( tmr_procTriggerHdlr_t )TmrOffsetDataHdlr,TMR_TYPE_10MS);
			tmrOffsetData=NULL;
	}

/**********************************************/
/*         终端校时发送（32960）              */
/**********************************************/
//	if (sys_status>=PROC_SYSTEM_LOGIN_ACK ) //车辆登录成功后，开启定时器 3s
//	{
//			if(tmrCrrectTime==NULL)
//			tmrCrrectTime = TMR_Subscribe ( TRUE, 3000, TMR_TYPE_10MS, ( tmr_procTriggerHdlr_t )TmrCorrectTimeHdlr );			 
//	}
//	else if(sys_status<PROC_SYSTEM_LOGIN_ACK) //预留：当补发数据发送结束，注销定时器
//	{
//			if(tmrCrrectTime)
//			TMR_UnSubscribe(tmrCrrectTime,( tmr_procTriggerHdlr_t )TmrCorrectTimeHdlr,TMR_TYPE_10MS);
//			tmrCrrectTime=NULL;
//	}

/**********************************************/
/*         车辆登出（32960）              		*/
/**********************************************/
#define LOGOUT_TIME       									10		//车辆登出准备时间 5s	
	
	if(sys_status==PROC_SYSTEM_LOGOUT||logout_flag==1)
	{			
			logout_counter++;
			if(logout_counter==LOGOUT_TIME)
			{
				/*车辆登出*/
				printf("\r\n[LOGINOUT EVENT]: Begin Loginout....\r\n");
				vehicles_logout();
			}
	}
/*****************************/
/*   GSM信号与网络状态查询   */
/*****************************/

/*****************************/
/*          GPS数据          */
/*****************************/

  GPS_ProcDataHdlr ();
}








/* 
 * 功能描述：数据解码 
 * 引用参数：(1)数据
 *           (2)长度
 *         
 * 返回值  ：无
 * 
 */

extern void Gprs_ProtocolDataDecode ( u16 len, u8 *data )
{
	int i;
	u8 len1,tmp[20];
#if DATA_DECODE_DEBUG==1
{
      static ascii recDataSource[ 800 ];     
      MDM_Bytes2String ( ( u8* )recDataSource, ( const u8* )data, len );
      printf ( "\r\n[RX DATA] %s\r\n", recDataSource );
}
#endif 

  //对平台返回数据进行判断
  if ( data[ 0 ]==0x23 && data[1]==0x23 )
  {
  	switch(data[2])	//对命令判断
	{
	//车辆登录回执
	case 0x01:
		if(data[3]==1)
		{
			printf ( "\r\n[DataDecode EVENT]: LOGIN  SUCCESS  \r\n");

			//设置终端状态为：登陆成功
      sys_status = PROC_SYSTEM_LOGIN_ACK;
		}
		else
		{
			printf ( "\r\n[DataDecode EVENT]: LOGIN  Failed ! >>>  Ack: %d \r\n",data[3]);
			for(i=0;i<len;i++)
			{			
			  	printf ( "%02x\t",data[i]);
			}
			printf ( "\r\n");	
		}
		break;
	//实时数据回执
	case 0x02:
		sysRealDataCount=0;	
		//设置终端状态
		if (sys_status>=PROC_SYSTEM_LOGIN_ACK&&sys_status<PROC_SYSTEM_LOGOUT)
    sys_status = PROC_SYSTEM_REALDATA_ACK;
		//printf ( "\r\n[DataDecode EVENT]: RealData  Ack  Success!!! \r\n");
		break;
	//补发数据回执		
	case 0x03:
		//printf ( "\r\n[DataDecode EVENT]: ReissueData  Ack  Success!!!\r\n");
		break;
	//车辆登出回执
	case 0x04:
		sys_status = PROC_SYSTEM_LOGOUT_ACK;
		printf("\r\n[DataDecode EVENT]: LOGIN OUT \r\n");
		break;
	//心跳回执
	case 0x07:
		printf ( "\r\n[DataDecode EVENT]: Heartbeat Success!!!\r\n");
	  sysHeartbeat=0;
	  if (sys_status>=PROC_SYSTEM_LOGIN_ACK&&sys_status<PROC_SYSTEM_LOGOUT)
    sys_status = PROC_SYSTEM_HEART;
		break;
	//终端校时回执
	case 0x08:
		//Gprs_Time_Update();
		printf ( "\r\n********___终端校时回执____!!!\r\n");
		break;
	//平台下发的查询命令
	case 0x80:
		
	  Gprs_ParametersQuery(len,data);
		#if 1
	   	printf ( "\r\n********___收到80回执信息____!!!\r\n");
	   	for(i=0;i<len;i++)
	   	{
	   	 	printf ( "%02x ",data[i]);
	   	}
	   	printf ( "\r\n");	
		#endif
		break;
	//平台下发的设置命令
	case 0x81:
		#if 1
	   	printf ( "\r\n********___收到81回执信息____!!!\r\n");
	   	for(i=0;i<len;i++)
	   	{
	   	 	printf ( "%02x ",data[i]);
	   	}
	   	printf ( "\r\n");
		#endif
			
		Gprs_ParametersSet(len,data);
		break;
	
	case 0x82:
		printf ( "\r\n********___收到82回执信息____ 长度：%d \r\n",len);
		printf ( "\r\n");
		for(i=0;i<len;i++)
		{
			printf ( "%02x",data[i]);
		}
		printf ( "\r\n");
		//平台下发的车载终端控制命令
		//if(Gprs_Vehicle_terminal_control_response(len,data))
		Gprs_VehicleTerminalControl((len-30),&data[30]);

		break;

	default:
		#if 1
	   	printf ( "\r\n********___收到错误回执信息____!!!\r\n");
	   	for(i=0;i<len;i++)
	   	{
	   	 	printf ( "\r\n%02x\r\n",data[i]);
	   	}
	   	printf ( "\r\n");
		#endif
	
	
		break;
	}
  }
  else 
  {

	    printf ( "\r\n********___不识别的应答____!!!\r\n");
  		#if 1
	   	printf ( "\r\n********___收到错误回执信息____!!!\r\n");
	   	for(i=0;i<len;i++)
	   	{
	   	 	printf ( "\r\n%02x\r\n",data[i]);
	   	}
	   	printf ( "\r\n");
		#endif
  }


}




/*
*******************************************************************
*函数名：BCC_CheckSum
*参  数：ack_order：发送命令
*功  能：计算数据单元字节数
*创建人: gl
*
*******************************************************************
*/
static u16 CAL_DataUnit_length(u8 *buf)
{
	 u16 temp=0;
	
	 temp=buf[52]*buf[53]+30;
	 buf[22]=(u8)(temp>>8);
	 buf[23]=(u8)(temp&0xff);
	 temp-=30;
	 return temp;
}

/*
*******************************************************************
*函数名：BCC_CheckSum
*参  数：ack_order：发送命令
*功  能：BCC异或校验
*创建人: gl
*
*******************************************************************
*/
static u8 BCC_CheckSum(u8 *buf, u16 len)
{
	u32 i;
	u8 checksum = 0;
	 
	for(i = 0; i < len; i++)
	{
	  checksum ^= *buf++;
	  
	}
	 return checksum;
}


/* 
 * 功能描述：数据编码
 * 引用参数：(1)数据
 *           (2)长度
 *
 * 返回值  ：状态码
 * 
 */
static u16 Gprs_ProtocolDataEncode ( u8 *data, u16 len )
{
  u8 i=0;
  u16 temp=0;
  u32 length=0; 
  
     
  //32960的起始符标识
  data[ 0 ] = 0x23;
  data[ 1 ] = 0x23;
  //对不同命令ID的数据单元长度进行处理
  switch(data[2])
  {
   case 1:
		//充电储能子系统
		//根据车场实际情况填充》》》待完成
		data[ 52 ] =0x01;
		data[ 53 ] =0x00;
		//数据单元长度
		temp=CAL_DataUnit_length(data);
		
		for(i=0;i<temp;i++)
		{
		   data[54+i] =i+1;
		}
		length=len+4+temp;
		len=length;
	    break;
   case 2:
   case 3:
	 case 0x80:
   		temp=len-22;    //22：传入Gprs_ProtocolDataEncode的数据长度是不包含2个字节标识符的；
			data[22]=(u8)(temp>>8);
	    data[23]=(u8)(temp&0xff);
			length=len+2;
   		break;
   case 4:   
   case 7:
   case 8:
   		length=len+2;
	    break;
   default:
   		break;
  
  }
  
  //32960的BCC校验
  data[ length++ ] = BCC_CheckSum ( ( data + 2 ), len );
  return length;  
}







/* 
 * 功能描述：登录指令
 * 引用参数：无
 *     
 * 返回值  ：无
 * 
 */

extern void login ( void )
{
	int i=0;

  if ( msgLogin == NULL )
  {
    msgLogin = SHABUF_GetBuffer ( LOGIN_BUFFER_LEN );
  }

  if ( msgLogin )
  {
    u16 totalLength = 0;
    u8 *msg = ( u8* )&msgLogin[ 2 ];
		//登录流水号
		login_serial =vehicles_login_serial_number();
	
    //命令单元
    msg[ 0 ] = 0x01; //命令ID   
		msg[ 1 ] = 0xFE; 

		//VIN   读取方式：目前是通过串口写入，从CAN读取待做	
		memcpy ( ( u8* )&msg[ 2 ], vehicleParams.CarVIN, 17 );
  
    //加密方式
		msg[ 19 ] =0x01;

		//数据单元长度
		msg[ 20 ] =0x00;
		msg[ 21 ] =0x00;


		if(GPS_WorkState!=1)//gps无效时，使用RTC时间
		{
				msg[ 22 ] = RTC_time.year;
				msg[ 23 ] = RTC_time.month;
				msg[ 24 ] = RTC_time.date;
				msg[ 25 ] = RTC_time.hours;
				msg[ 26 ] = RTC_time.minutes;
				msg[ 27 ] = RTC_time.seconds;
		}
		else
		{
				msg[ 22 ] = RMC_d.date_yy;
				msg[ 23 ] = RMC_d.date_mm;
				msg[ 24 ] = RMC_d.date_dd;
				msg[ 25 ] = RMC_d.time_hh;
				msg[ 26 ] = RMC_d.time_mm;
				msg[ 27 ] = RMC_d.time_ss;
		}

	




	//流水登录
  memcpy ( ( u8* )&msg[ 28 ], ( u8* )&login_serial, 2 );
	
	//SMI卡的ICCID
#if	 1
		
	memcpy ( ( u8* )&msg[ 30 ], drv_param.ccid, 20 );	
	printf("\r\nICCIC: %s\r\n",drv_param.ccid);
//	for(i=0;i<20;i++)
//	{
//		msg[ 30+i ] =(char)drv_param.ccid[i];

//	} 
#else
	msg[ 30] =0x38;
	msg[ 31] =0x39;
	msg[ 32] =0x38;
	msg[ 33] =0x36;

	msg[ 34] =0x30;
	msg[ 35] =0x32;
	msg[ 36] ='B';

	msg[ 37] =0x30;
	msg[ 38] =0x31;
	msg[ 39] =0x30;
	msg[ 40] =0x31;

	msg[ 41] =0x35;
	msg[ 42] =0x30;
	msg[ 43] =0x30;
	msg[ 44] =0x30;

	msg[ 45] =0x30;
	msg[ 46] =0x36;
	msg[ 47] =0x39;
	msg[ 48] =0x36;
	msg[ 49] =0x38;
#endif
	
  

    /*数据组包并BCC校验*/
    totalLength = Gprs_ProtocolDataEncode ( msgLogin, 50);//指针元素的个数
#if DEBUG_32960==1	
		printf ( "\r\nmsgLogin \r\n");

		for(i=0;i<totalLength;i++)
		{
			 printf ( "%02x", *(msgLogin+i) );
		
		}
		printf ( "\r\n");
#endif
    /* 登陆数据发送 */
    TCP_DataOutput( msgLogin, totalLength );
		#if SD_WRITE_ENABLE==1
		char LogineData[60];
		u8 SD_Login_HexToAscii[2*totalLength+10];
		sprintf(LogineData,"\r\n\r\nLoginData SendTime  20%02u%02u%02u %02u:%02u:%02u\r\n",RTC_time.year, RTC_time.month, RTC_time.date,RMC_d.time_hh,RMC_d.time_mm,RMC_d.time_ss);
		SD_WriteFile(&file,SD_filename,(u8*)LogineData,strlen(LogineData));
		
		HexToAscii(SD_Login_HexToAscii,msgLogin,totalLength,true);
		SD_WriteFile(&file,SD_filename,SD_Login_HexToAscii,2*totalLength+2);
		#endif
		
	

		/* 释放已分配的缓存 */
		if ( SHABUF_FreeBuffer( msgLogin ) == TRUE )
		{
			msgLogin = NULL;
		}

   }
}





/*
*******************************************************************
*函数名：vehicles_logout
*参  数：无
*功  能：车辆登出
*创建人: gl
* 
*******************************************************************
*/

void vehicles_logout(void)
{
  u8 i=0;

  if ( msgLogout == NULL )
  {
    msgLogout = SHABUF_GetBuffer ( LOGOUT_BUFFER_LEN );
  }

  if(msgLogout )
  {
    u16 totalLength = 0;
		
    u8 *msg = ( u8* )&msgLogout[ 2 ];


		//printf ( " send LOGout 0x%X\r\n", login_serial );
    //命令单元
    msg[ 0 ] = 0x04; //命令ID   
		msg[ 1 ] = 0xFE; 

		//VIN
    memcpy ( ( u8* )&msg[ 2 ], vehicleParams.CarVIN, 17 );

  
    //加密方式
		msg[ 19 ] =0x01;

		//数据单元长度
		msg[ 20 ] =0x00;
		msg[ 21 ] =0x08;

	  if(GPS_WorkState!=1)//gps无效时，使用RTC时间
		{
				msg[ 22 ] = RTC_time.year;
				msg[ 23 ] = RTC_time.month;
				msg[ 24 ] = RTC_time.date;
				msg[ 25 ] = RTC_time.hours;
				msg[ 26 ] = RTC_time.minutes;
				msg[ 27 ] = RTC_time.seconds;
		}
		else
		{
				msg[ 22 ] = RMC_d.date_yy;
				msg[ 23 ] = RMC_d.date_mm;
				msg[ 24 ] = RMC_d.date_dd;
				msg[ 25 ] = RMC_d.time_hh;
				msg[ 26 ] = RMC_d.time_mm;
				msg[ 27 ] = RMC_d.time_ss;
		}

	//流水登录
    memcpy ( ( u8* )&msg[ 28 ], ( u8* )&login_serial, 2 );


    /*数据组包并BCC校验*/
    totalLength = Gprs_ProtocolDataEncode ( msgLogout, 30);//指针元素的个数
#if DEBUG_32960==1	
	printf ("\r\nmsgLogout \r\n");
	for(i=0;i<totalLength;i++)
	{
		 printf ("0x%02x", *(msgLogout+i) );
	
	}
	printf ( "\r\n");
#endif
    /* 登陆数据发送 */
    TCP_DataOutput ( msgLogout, totalLength );
    
		if(SHABUF_FreeBuffer(msgLogout)==TRUE)
		{
			msgLogout=NULL;
		}				
		

    

  }
}





//FIL file;
u16  vehicles_login_serial_number ( void )
{
	
	
	FRESULT result;
	
	/*读取车辆上一次登录时间*/
	/*读取车辆登录流水*/
	STORAGE_ReadData( VEHICLE_PARAMS, sizeof( vechile_Params_t ), ( u8* )&vehicleParams );
	STORAGE_ReadData ( GPRS_PARAMS,   sizeof( gprs_Params_t ),   ( u8 * )&gprsParams );

	printf ( "\r\n^^^^^^^^^GPRS TIME:  20%u/%u/%u\r\n",RMC_d.date_yy, RMC_d.date_mm, RMC_d.date_dd);

	printf ( "\r\n^^^^^^^^^FLASH TIME: 20%u/%u/%u\r\n",vehicleParams.GPS_d.date_yy, vehicleParams.GPS_d.date_mm, vehicleParams.GPS_d.date_dd);

	if(vehicleParams.GPS_d.date_yy==RMC_d.date_yy&&vehicleParams.GPS_d.date_mm==RMC_d.date_mm&&vehicleParams.GPS_d.date_dd==RMC_d.date_dd)
	{
			//日期一致，流水号++
			if(vehicleParams.LoginSerial>0&&vehicleParams.LoginSerial<=65531)
			{
					vehicleParams.LoginSerial++;
			}
			else
			{
				vehicleParams.LoginSerial=1;
			}
			sprintf(( char* )SD_filename,"0:TZ3000_20%02u%02u%02u.txt",RMC_d.date_yy, RMC_d.date_mm, RMC_d.date_dd);
			result=f_open(&file,( char* )SD_filename,FA_CREATE_NEW);
			if(!result)
			f_close(&file);
			
	}
	else
	{

		sprintf(( char* )SD_filename,"0:TZ3000_20%02u%02u%02u.txt",RMC_d.date_yy, RMC_d.date_mm, RMC_d.date_dd);
		result=f_open(&file,( char* )SD_filename,FA_CREATE_NEW);
		f_close(&file);

		//日期不一致：流水号清零 ,更新登入时间
		vehicleParams.LoginSerial=1;
		vehicleParams.GPS_d.date_yy=RMC_d.date_yy;
		vehicleParams.GPS_d.date_mm=RMC_d.date_mm;
		vehicleParams.GPS_d.date_dd=RMC_d.date_dd;
	}
	/*更新车辆登入流水号*/
	/*更新车辆登录时间*/
	
	if(RMC_d.date_yy!=80)
	{
			STORAGE_WriteData (VEHICLE_PARAMS, sizeof( vechile_Params_t ), ( u8* )&vehicleParams );
	}

	printf ( "\r\nVehicles Login SerialNumber :%d        \r\n",vehicleParams.LoginSerial);
	printf ( "\r\n^^^^^^^^^GPRS    TIME: 20%u/%u/%u\r\n",RMC_d.date_yy, RMC_d.date_mm, RMC_d.date_dd);
	printf ( "\r\n^^^^^^^^^flash   TIME: 20%u/%u/%u\r\n",vehicleParams.GPS_d.date_yy, vehicleParams.GPS_d.date_mm, vehicleParams.GPS_d.date_dd);

	return reverseU16 (vehicleParams.LoginSerial);
}








/*
*******************************************************************
*函数名：data_pack_head
*参  数：命令ID
*功  能：数据包的包头
*创建人: gl
* 
*******************************************************************
*/

u16 data_pack_head(u8 order)
{  

  u8 *msg=NULL;
  if ( msgRealData_upload == NULL||msgDeadData_Strorage==NULL )
  {	
		 if(order==0x02)
				msgRealData_upload = SHABUF_GetBuffer ( REALDATA_UPLOAD_BUFFER_LEN );	
		 else
		 if(order==0x03)
				msgDeadData_Strorage = SHABUF_GetBuffer ( REALDATA_UPLOAD_BUFFER_LEN );
  }

  if ( msgRealData_upload ||msgDeadData_Strorage)
  {
		if(order==0x02)
				msg = ( u8* )&msgRealData_upload[ 2 ];
		else
		if(order==0x03)
				msg = ( u8* )&msgDeadData_Strorage[ 2 ];

    //命令单元
    msg[ 0 ] = order; //实时数据ID  
		msg[ 1 ] = 0xFE; 

		//VIN
    memcpy ( ( u8* )&msg[ 2 ], vehicleParams.CarVIN, 17 );
    
    //加密方式
		msg[ 19 ] =0x01;

		//数据单元长度
		msg[ 20 ] =0x00;
		msg[ 21 ] =0x00;
  }

  return 21;
}




/*
*******************************************************************
*函数名：Vehicle_data_upload
*参  数：无
*功  能：整车数据上传
*创建人: gl
* 
*******************************************************************
*/

u16 Vehicle_data_upload(u16 lenth)
{
	u8 *msg=NULL;

	vehicle_data_t * VehicleData_Upload=&Vehicle_Data;
	if ( msgRealData_upload||msgDeadData_Strorage)
  {
		
		 if(msgRealData_upload)
				msg = ( u8* )&msgRealData_upload[ 2 ];
		 else
		 if(msgDeadData_Strorage)
			  msg = ( u8* )&msgDeadData_Strorage[ 2 ];
		 
		 //整车数据命令ID
		 msg[++lenth]=0x01;
		 //车辆状态
		 msg[++lenth]=VehicleData_Upload->vehicle_state;
		 //充电状态
			msg[++lenth]=VehicleData_Upload->charge_state;
		 //运行模式
		 msg[++lenth]=VehicleData_Upload->run_mode;
		 //车速	
		 msg[++lenth]=(VehicleData_Upload->vehicle_speed>>8)&0xff;	
		 msg[++lenth]=VehicleData_Upload->vehicle_speed&0xff;
		 //累计里程
		 msg[++lenth]=(VehicleData_Upload->accumulated_distance>>24)&0xff;
		 msg[++lenth]=(VehicleData_Upload->accumulated_distance>>16)&0xff;
		 msg[++lenth]=(VehicleData_Upload->accumulated_distance>>8)&0xff;
		 msg[++lenth]=VehicleData_Upload->accumulated_distance&0xff;
		 //总电压
		 msg[++lenth]=(VehicleData_Upload->total_voltage>>8)&0xff;
		 msg[++lenth]=VehicleData_Upload->total_voltage&0xff;
		 //总电流
		 msg[++lenth]=(VehicleData_Upload->total_current>>8)&0xff;
		 msg[++lenth]=VehicleData_Upload->total_current&0xff;
		 //SOC
		 msg[++lenth]=VehicleData_Upload->soc;
		 //DC-DC状态
		 msg[++lenth]=VehicleData_Upload->dc_dc_state;
		 //档位
		 msg[++lenth]=VehicleData_Upload->gear;
		 //绝缘电阻
		 msg[++lenth]=(VehicleData_Upload->ir>>8)&0xff;
		 msg[++lenth]=VehicleData_Upload->ir&0xff;
		 //加速踏板行程值
		 msg[++lenth]=VehicleData_Upload->gas_value;
		 //制动踏板状态
		 msg[++lenth]=VehicleData_Upload->braking_value;
		 
	}
	VehicleData_Upload=NULL;
	return lenth;
}


/*
*******************************************************************
*函数名：Drive_motor_data_upload
*参  数：无
*功  能：驱动电机数据上传
*创建人: gl
* 
*******************************************************************
*/

u16 Drive_motor_data_upload(u16 lenth)
{
	u8 templenth=0;
	u8 *msg=NULL;
	
	motor_data_t *MotorData_Upload=&Motor_Data;
	if ( msgRealData_upload||msgDeadData_Strorage )
  {
		 if(msgRealData_upload)
				msg = ( u8* )&msgRealData_upload[ 2 ];
		 else
		 if(msgDeadData_Strorage)
			  msg = ( u8* )&msgDeadData_Strorage[ 2 ];

		 //驱动电机数据命令ID
		 msg[++lenth]=0x02;

		 msg[++lenth]=MotorData_Upload->motor_num;	//驱动电机个数
		 templenth=lenth;

		 switch(msg[templenth])
		 {
		 case 1: //1号驱动电机数据
		 	 	msg[++lenth]=MotorData_Upload->motor_SN;//驱动电机顺序号
				//驱动电机状态
				msg[++lenth]=MotorData_Upload->motor_state;
				//驱动电机控制器温度
				msg[++lenth]=MotorData_Upload->motor_ctrl_temperature;
				//驱动电机转速
				msg[++lenth]=(MotorData_Upload->motor_speed>>8)&0xff;
				msg[++lenth]=MotorData_Upload->motor_speed&0xff;
				//驱动电机转矩
				msg[++lenth]=(MotorData_Upload->motor_torque>>8)&0xff;
				msg[++lenth]=MotorData_Upload->motor_torque&0xff;
				//驱动电机温度
				msg[++lenth]=MotorData_Upload->motor_temperature&0xff;
				//电机控制器输入电压
				msg[++lenth]=(MotorData_Upload->motor_ctrl_voltage>>8)&0xff;	
				msg[++lenth]=MotorData_Upload->motor_ctrl_voltage&0xff;
				//电机控制器直流母线电流
				msg[++lenth]=(MotorData_Upload->motor_ctrl_current>>8)&0xff;
				msg[++lenth]=MotorData_Upload->motor_ctrl_current&0xff;

		 		break;
		 case 2:
		 		break;
		 }
	}
	MotorData_Upload=NULL;
	return (msg[templenth]*12+templenth);
	

}





/*
*******************************************************************
*函数名：Fuel_cell_data_upload
*参  数：无
*功  能：燃料电池数据上传
*创建人: gl
* 
*******************************************************************
*/

u16 Fuel_cell_data_upload(u16 lenth)
{
	u8 i;
	u16 fuel_cell_num=0;
	u8 *msg=NULL;
	if ( msgRealData_upload )
  {

     msg = ( u8* )&msgRealData_upload[ 2 ];

		 //燃料电池数据命令ID
		 msg[++lenth]=0x03;
		 msg[++lenth]=0x01;
		 msg[++lenth]=0x02;
		 msg[++lenth]=0x03;
		 msg[++lenth]=0x04;
		 msg[++lenth]=0x05;
		 msg[++lenth]=0x06;

		 //燃料电池探针数量
		 msg[++lenth]=0x00;
		 fuel_cell_num=(msg[lenth]<<8)&0xff00;
		 msg[++lenth]=0x01;
		 fuel_cell_num+=msg[lenth];

		 ++lenth;
		 for(i=0;i<fuel_cell_num;i++)
		 {
			msg[++lenth]=i;
		 	
		 }
		 msg[++lenth]=0x01;
		 msg[++lenth]=0x02;
		 msg[++lenth]=0x03;
		 msg[++lenth]=0x04;
		 msg[++lenth]=0x05;
		 msg[++lenth]=0x06;
		 msg[++lenth]=0x07;
		 msg[++lenth]=0x08;
		 msg[++lenth]=0x09;
		 msg[++lenth]=0x0a;

		 return lenth;
	}
	

}



/*
*******************************************************************
*函数名：Engine_data_upload
*参  数：无
*功  能：发动机数据上传
*创建人: gl
* 
*******************************************************************
*/

u16 Engine_data_upload(u16 length)
{

	u8 *msg=NULL;
	engine_sys_data_t* engine_sys_data_upload=&Engine_Sys_Data;
	if (( msgRealData_upload)&&(engine_sys_data_upload->used>0))
	{

     msg = ( u8* )&msgRealData_upload[ 2 ];

		 msg[++length]=0x04; 	//发动机命令ID
		 /*发动机状态*/
		 msg[++length]=engine_sys_data_upload->engine_state;
		 /*发动机曲轴转速*/
		 msg[++length]=(engine_sys_data_upload->crankshaft_speed>>8)&0xff;
		 msg[++length]=engine_sys_data_upload->crankshaft_speed&0xff;
		 /*发动机燃料消耗率*/
		 msg[++length]=(engine_sys_data_upload->fuel_consumption>>8)&0xff;
		 msg[++length]=engine_sys_data_upload->fuel_consumption&0xff;
	}
	return length;

}


/*
*******************************************************************
*函数名：Vehicle_position_data_upload
*参  数：无
*功  能：车辆位置数据上传
*创建人: gl
* 
*******************************************************************
*/

u16 Vehicle_position_data_upload(u16 lenth )
{
	int i;
	u8 *msg=NULL;
	if ( msgRealData_upload||msgDeadData_Strorage)
  {	
		 if(msgRealData_upload)
				msg = ( u8* )&msgRealData_upload[ 2 ];
		 else
		 if(msgDeadData_Strorage)
			  msg = ( u8* )&msgDeadData_Strorage[ 2 ];
		 //车辆位置数据命令ID
		 msg[++lenth]=0x05;

		 #if 0
		 printf("\r\n经度信息：\r\n");
		 for(i=0;i<4;i++)
		 {
		 	printf("0x%02x\t",locations.lon[i]);
		 }
		 printf("\r\n纬度信息：\r\n");
		 for(i=0;i<4;i++)
		 {
		 	printf("0x%02x\t",locations.lat[i]);
		 }
		 printf("\r\n");
		 #endif
		 
#if  1 	 
		 if ( RMC_d.status_ == 'A'&&RMC_d.long_!=0&&RMC_d.lat_!=0)
					msg[++lenth]=0x00;//设置GPS是否有效，及东经北纬设置
		 else
					msg[++lenth]=0x01;//设置GPS是否有效，及东经北纬设置

				 
		 //经度信息   udpPositon.lon[0](高字节)-->udpPositon.lon[3]（低字节） 
		 msg[++lenth]=locations.lon[0];
		 msg[++lenth]=locations.lon[1];
		 msg[++lenth]=locations.lon[2];
		 msg[++lenth]=locations.lon[3];
		 
		 //纬度信息   udpPositon.lat[0](高字节)-->udpPositon.lat[3]（低字节）
		 msg[++lenth]=locations.lat[0];
		 msg[++lenth]=locations.lat[1];
		 msg[++lenth]=locations.lat[2];
		 msg[++lenth]=locations.lat[3];
		 
#else
		 		 //经度信息   udpPositon.lon[0](高字节)-->udpPositon.lon[3]（低字节） 
		 msg[++lenth]=0x07;
		 msg[++lenth]=0x15;
		 msg[++lenth]=0x3a;
		 msg[++lenth]=0xbf;

		 //纬度信息   udpPositon.lat[0](高字节)-->udpPositon.lat[3]（低字节）
		 msg[++lenth]=0x01;
		 msg[++lenth]=0xe8;
		 msg[++lenth]=0xa8;
		 msg[++lenth]=0x57;
		
#endif

		
	}
	return lenth;

}




/*
*******************************************************************
*函数名：Extreme_value_data_upload
*参  数：无
*功  能：极值数据上传
*创建人: gl
* 
*******************************************************************
*/

u16 Extreme_value_data_upload(u16 lenth)
{
	u8 *msg=NULL;

	extreme_value_data_t * ExtremeValue_Data_Upload=&Extreme_Value_Data;

	if ( msgRealData_upload||msgDeadData_Strorage )
  {

     if(msgRealData_upload)
				msg = ( u8* )&msgRealData_upload[ 2 ];
		 else
		 if(msgDeadData_Strorage)
			  msg = ( u8* )&msgDeadData_Strorage[ 2 ];


		 //极值数据命令ID
		 msg[++lenth]=0x06;

		 //最高电压电池子系统号
		 msg[++lenth]=ExtremeValue_Data_Upload->highestVoltage_Subsys_num;
		 //最高电压电池单体代号
		 msg[++lenth]=ExtremeValue_Data_Upload->highestVoltage_Single_code;
		 //电池单体电压最高值
		 msg[++lenth]=(ExtremeValue_Data_Upload->SingleBattery_HighestVoltage>>8)&0xff;
		 msg[++lenth]=ExtremeValue_Data_Upload->SingleBattery_HighestVoltage&0xff;

		 //最低电压电池子系统号
		 msg[++lenth]=ExtremeValue_Data_Upload->lowestVoltage_Subsys_num;
		 //最低电压电池单体代号
		 msg[++lenth]=ExtremeValue_Data_Upload->lowestVoltage_Single_code;
		 //电池单体电压最低值  
		 msg[++lenth]=(ExtremeValue_Data_Upload->SingleBattery_LowestVoltage>>8)&0xff;
		 msg[++lenth]=ExtremeValue_Data_Upload->SingleBattery_LowestVoltage&0xff;

		 //最高温度子系统号
		 msg[++lenth]=ExtremeValue_Data_Upload->highestTemperature_Subsys_num;
		 //最高温度探针号
		 msg[++lenth]=ExtremeValue_Data_Upload->highestTemperature_Probe_code;
		 //最高温度值
		 msg[++lenth]=ExtremeValue_Data_Upload->Probe_HighestTemperature&0xff;

		 //最低温度子系统号
		 msg[++lenth]=ExtremeValue_Data_Upload->lowestTemperature_Subsys_num;
		 //最低温度探针序号
		 msg[++lenth]=ExtremeValue_Data_Upload->lowestTemperature_Probe_code;
		 //最低温度值
		 msg[++lenth]=ExtremeValue_Data_Upload->Probe_LowestTemperature&0xff;

		 
	}
	ExtremeValue_Data_Upload=NULL;
	return lenth;

}

/*
*******************************************************************
*函数名：Set_Comm_Warning
*参  数：无
*功  能：设置或清除报警标志及报警等级
*创建人: gl
* 
*******************************************************************
*/
static u8 grade_temp;
static u32 warn_sign_bit_temp;
void Set_Comm_Warning(u8 warn_sign,u8 grade)
{ 
	if((warn_sign<=18)&&(grade<=3))
	{
			if(grade==0)
			{
					warn_sign_bit_temp&=~(1<<warn_sign);
			}
			else
			{
					warn_sign_bit_temp|=(1<<warn_sign);
			}
			grade_temp=grade;
	}	
}
/*
*******************************************************************
*函数名：Extreme_value_data_upload
*参  数：无
*功  能：极值数据上传
*创建人: gl
* 
*******************************************************************
*/

u16 Warning_Data_Upload(u16 length)
{
  u8 *msg=NULL;
	warning_sys_data_t* warning_sys_data_upload=&Com_Warning_Data;
	if( msgRealData_upload||msgDeadData_Strorage )
	{
		
		if(msgRealData_upload)
				msg = ( u8* )&msgRealData_upload[2];
		else
		if(msgDeadData_Strorage)
			  msg = ( u8* )&msgDeadData_Strorage[2];
		
		grade_temp=warning_sys_data_upload->highest_warning_grade;
		warn_sign_bit_temp=warning_sys_data_upload->warning_sign;

		
		if(upload_switch_state==1||upload_switch==1||alarm_alive==1)
		{
			  /*手动触发3级报警*/
				Set_Comm_Warning(14,3);
		}
		
		msg[++length]=0x07;	//报警数据
		msg[++length]=grade_temp;
		msg[++length]=(warn_sign_bit_temp>>24)&0xff;
		msg[++length]=(warn_sign_bit_temp>>16)&0xff;
		msg[++length]=(warn_sign_bit_temp>>8)&0xff;
		msg[++length]=warn_sign_bit_temp&0xff;
		msg[++length]=0;
		msg[++length]=0;
		msg[++length]=0;
		msg[++length]=0;
	}
	return length;
}

/*
*******************************************************************
*函数名：User_defined_data_upload
*参  数：无
*功  能：可充电储能装置电压数据
*创建人: gl
* 
*******************************************************************
*/

u16 User_defined_data_upload(u16 lenth,u8 order)
{
	if ( msgRealData_upload)
  	{
         u8 *msg = ( u8* )&msgRealData_upload[ 2 ];
		 //自定义数据命令ID
		 msg[++lenth]=order;

		 msg[++lenth]=0x01;
		 msg[++lenth]=0x02;
		 msg[++lenth]=0x03;
		 msg[++lenth]=0x04;
		 msg[++lenth]=0x05;	 
	}
	return lenth;
}

/*
*******************************************************************
*函数名：Voltage_data_for_rechargeable_devices
*参  数：无
*功  能：可充电储能装置电压数据
*创建人: gl
* 
*******************************************************************
*/
u16 Voltage_data_for_rechargeable_devices(u16 length)
{
		u8 *msg=NULL;
		u16 battery_num,charge_sys_num;

		stored_energy_sys_data_t* stored_energy_sys_data_upload=&Stored_Energy_sys_Data;
		//无充电子系统号，则不传电压列表
		if (( msgRealData_upload||msgDeadData_Strorage)&&(stored_energy_sys_data_upload->stored_energy_sys_sn>0))
		{

			if(msgRealData_upload)
				msg = ( u8* )&msgRealData_upload[ 2 ];
			else
			if(msgDeadData_Strorage)
			  msg = ( u8* )&msgDeadData_Strorage[ 2 ];

			msg[++length]=0x08;	//电压列表命令ID
			/*可充电装置子系统个数 */
			msg[++length]=stored_energy_sys_data_upload->stored_energy_sys_num;
			charge_sys_num=length;
			switch(msg[charge_sys_num])
			{
				case 1:
					/*可充电装置子系统号*/
					msg[++length]=stored_energy_sys_data_upload->stored_energy_sys_sn;
					/*可充电装置电压*/
					msg[++length]=(stored_energy_sys_data_upload->stored_energy_voltage>>8)&0xff;
					msg[++length]=stored_energy_sys_data_upload->stored_energy_voltage&0xff;
					/*可充电装置电流*/
					msg[++length]=(stored_energy_sys_data_upload->stored_energy_current>>8)&0xff;
					msg[++length]=stored_energy_sys_data_upload->stored_energy_current&0xff;
					/*单体电池总数*/
					msg[++length]=(stored_energy_sys_data_upload->single_battery_total_num>>8)&0xff;
					msg[++length]=stored_energy_sys_data_upload->single_battery_total_num&0xff;
					
				
					//本帧起始电池序号和本帧单体电池数，需要根据实际情况修改
					/********************************************************************************************/
					/*本帧起始电池序号;(ps:一般是不可变的)*/
					msg[++length]=(stored_energy_sys_data_upload->this_frame_InitialBattery_code>>8)&0xff;
					msg[++length]=stored_energy_sys_data_upload->this_frame_InitialBattery_code&0xff;
					/*本帧单体电池数*///一般与单体总数相同
					msg[++length]=stored_energy_sys_data_upload->this_frame_battery_num;
					/********************************************************************************************/
					/*单体电池电压列表*/
					battery_num=length;
					for(u8 i=0;i<msg[battery_num];i++)
					{
						/*单体电池电压高字节*/
						/*单体电池电压低字节*/
						msg[++length]=(stored_energy_sys_data_upload->singlebattery_voltage_list[i]>>8)&0xff;
						msg[++length]=stored_energy_sys_data_upload->singlebattery_voltage_list[i]&0xff;
					}
					break;

			} 
		}
		return length;


}

/*
*******************************************************************
*函数名：Temperature_data_for_rechargeable_devices
*参  数：无
*功  能：可充电储能装置温度数据
*创建人: gl
* 
*******************************************************************
*/
u16 Temperature_data_for_rechargeable_devices(u16 length)
{
	
		u8 *msg=NULL;
		u16 temp_length;

		stored_energy_sys_data_t* stored_energy_sys_data_upload=&Stored_Energy_sys_Data;
		//无充电子系统号，则不传温度列表
		if(( msgRealData_upload||msgDeadData_Strorage )&&(stored_energy_sys_data_upload->stored_energy_sys_sn>0))
		{
			 if(msgRealData_upload)
				  msg = ( u8* )&msgRealData_upload[ 2 ];
			 else
			 if(msgDeadData_Strorage)
					msg = ( u8* )&msgDeadData_Strorage[ 2 ];
		 
			msg[++length]=0x09; 	//温度列表命令ID
			/*可充电储能子系统个数（温度）*/
			msg[++length]=stored_energy_sys_data_upload->stored_energy_sys_num;
			temp_length=length;
			
			switch(msg[temp_length])
			{
				case 1:

					/*可充电储能子系统号（温度）*/
					msg[++length]=stored_energy_sys_data_upload->stored_energy_sys_sn;
					/*温度探针个数*/
					msg[++length]=(stored_energy_sys_data_upload->temperature_probe_num>>8)&0xff;
					msg[++length]=stored_energy_sys_data_upload->temperature_probe_num&0xff;
					temp_length=length;
					/*温度探针列表*/
					{
							for(u16 i=0;i<msg[temp_length];i++)
							{
									msg[++length]=stored_energy_sys_data_upload->probe_temperature_list[i];
							}
					}
					break;

			}
		}
		return length;

}



/*
*******************************************************************
*函数名：Multi_realdata_upload
*参  数：无
*功  能：多包实时数据上传
*创建人: gl
* 
*******************************************************************
*/

u16 Multi_realdata_upload(u16 Counter,u8 UploadTime,bool enable)
{	

  OS_CPU_SR   cpu_sr;
	  
	u16 pack_head_length=data_pack_head(0x02);
  //获取数据起始地址长度
	u16 templenth=pack_head_length;
	if ( msgRealData_upload )
  {
     u16 totalLength = 0;
     u8 *msg = ( u8* )&msgRealData_upload[ 2 ];

//		if(GPS_WorkState==0)//gps无效时，使用RTC时间
		{
				//数据采集时间	//ID:22
				msg[ ++templenth ] = RTC_time.year;
				msg[ ++templenth ] = RTC_time.month;
				msg[ ++templenth ] = RTC_time.date;
				msg[ ++templenth ] = RTC_time.hours;
				msg[ ++templenth ] = RTC_time.minutes;
				msg[ ++templenth ] = RTC_time.seconds;

		}
//		else
//		{

//				//数据采集时间	//ID:22
//				msg[ ++templenth ] = RMC_d.date_yy;
//				msg[ ++templenth ] = RMC_d.date_mm;
//				msg[ ++templenth ] = RMC_d.date_dd;	
//				msg[ ++templenth ] = RMC_d.time_hh;
//				msg[ ++templenth ] = RMC_d.time_mm;
//				msg[ ++templenth ] = RMC_d.time_ss;

//		}

		
#if TIME_BIAS_ENABLE ==1		
		if(upload_switch==1)//3级报警出现时的时间戳
		{  
			  if(real_data_time_ss==59)
				{
					real_data_time_ss=0;
					real_data_time_mm+=1;
				}
				else
				{
					real_data_time_ss++;
				}
				msg[pack_head_length+5]=real_data_time_mm;
				msg[pack_head_length+6]=real_data_time_ss;
		 }
#endif		 
		 /*整车数据*/
		 templenth=Vehicle_data_upload(templenth);
		 
		 /*驱动电机数据*/
		 templenth=Drive_motor_data_upload(templenth);
		 
		 ///*发动机数据*/
		 //templenth=Engine_data_upload(templenth);
		 //templenth=Engine_Sys_Temp(templenth);
		 
		 /*车辆位置*/
		 templenth=Vehicle_position_data_upload(templenth);
		 
		 /*极值数据*/
		 templenth=Extreme_value_data_upload(templenth);
		 
		 /*报警数据*/
		 templenth=Warning_Data_Upload(templenth);
		 
		 /*用户自定义数据*/
		 //templenth=User_defined_data_upload(templenth,0x80);
		 
		 /*可充电装置电压数据*/
		 templenth=Voltage_data_for_rechargeable_devices(templenth);
		 
		 /*可充电装置温度数据*/
		 templenth=Temperature_data_for_rechargeable_devices(templenth);
		
		 /*数据组包并BCC校验*/
	   totalLength = Gprs_ProtocolDataEncode ( msgRealData_upload, templenth+1);//指针元素的个数
		 DeadData_lenth=totalLength;
		 DeadData_lenth=totalLength;
#if DEBUG_32960==1
		 printf ( "\r\n");
		 for(i=0;i<totalLength;i++)
		 {
			 printf ( "%02x", *(msgRealData_upload+i) );
		
		 }
		 printf ( "\r\n");
#endif

#if FIFO_RING_SW==1	
		 if(FIFO_Init_flag==-1)
		 {
					/*登录成功后，建立FIFO  ：容量30条数据，数据长度：totalLength */
					FIFO_Init(pRingBuf,FIFO_Buffer,totalLength,30);	
					FIFO_Init_flag=1;
		 }
#endif

#if FIFO_RING_SW==1

		/*FIFO缓存区：每秒存一次数据*/
		if(upload_switch==0&&FIFO_switch!=1&&Alarm_AtDeadtime!=1)  //打开环形缓存
		{
			
				if(FIFO_IsFull(pRingBuf))
				{
					FIFO_GetOne(pRingBuf, FIFO_temp);
				}
				//printf("\r\n FIFI Storage Stamp :%d :%d :%d \r\n",msgRealData_upload[27],msgRealData_upload[28],msgRealData_upload[29]);
				FIFO_AddOne(pRingBuf,(u8*)msgRealData_upload);
		}
		
#if DEBUG_32960==1 
		printf ( "\r\nFIFO_IsEmpty__FIFO计数器数据量：%d\r\n",pRingBuf->Counter); 
		printf ( "\r\nFIFO_IsEmpty__FIFO        针头：%d\r\n",pRingBuf->Head);
	  printf ( "\r\nFIFO_IsEmpty__FIFO        针尾：%d\r\n",pRingBuf->Tail);

		printf ( "\r\n");
		for(i=0;i<totalLength;i++)
		{
			printf("%02x\t",msgRealData_upload[i]);		
		}
		printf ( "\r\n");
#endif
		
		
#endif
	   /* 实时数据发送 */
		 if(Counter>=UploadTime)
		 {
				upload_timecount=0;	//数据上传计数器清零
				TCP_DataOutput( msgRealData_upload, totalLength );	
				if(upload_switch==1)
				{
						printf("\r\n[3 LEVEL ALARM EVENT]: RealData-->Sending  %d , TimeStamp: %02u:%02u:%02u \r\n",alarm_timecount,msgRealData_upload[27],msgRealData_upload[28],msgRealData_upload[29]);
				}
				#if   SD_WRITE_ENABLE==1
				char RealData_info[60];
				sprintf(RealData_info,"\r\n\r\nRealData SendTime  20%02u%02u%02u %02u:%02u:%02u\r\n",RTC_time.year, RTC_time.month, RTC_time.date,RMC_d.time_hh,RMC_d.time_mm,RMC_d.time_ss);
				SD_WriteFile(&file,SD_filename,(u8* )RealData_info,strlen(RealData_info));
			
				HexToAscii(SD_RealData_HexToAscii,msgRealData_upload,totalLength,true);
				SD_WriteFile(&file,SD_filename,SD_RealData_HexToAscii,2*totalLength+2);
				#endif	
		 }
		 
		 if ( SHABUF_FreeBuffer( msgRealData_upload ) == TRUE )
		 {
				msgRealData_upload = NULL;
		 }

	}else
	{
			printf("\r\n[ERROR ISSUES]: filename: %s  line:%d \t",__FILE__,__LINE__);
			printf("msgRealData_upload free error \r\n");
	}

}




/*
*******************************************************************
*函数名：DeadData_Strorage
*参  数：无
*功  能：盲区数据存储
*创建人: gl
* 
*******************************************************************
*/

u16 DeadData_Strorage(u16 Counter,u16 StorageCycleTime,u8 FIFI_Stop_Enable)
{	
  
	u16 pack_head_length=data_pack_head(0x03);
  //获取数据起始地址长度
	u16 templenth=pack_head_length;
	if (msgDeadData_Strorage )
  {
			 u16 totalLength = 0;
			 u8 *msg = ( u8* )&msgDeadData_Strorage[ 2 ];

			 //使用RTC时间
			 {
					 //数据采集时间	//ID:22
					 msg[ ++templenth ] = RTC_time.year;
					 msg[ ++templenth ] = RTC_time.month;
					 msg[ ++templenth ] = RTC_time.date;	
					 msg[ ++templenth ] = RTC_time.hours;
					 msg[ ++templenth ] = RTC_time.minutes;
					 msg[ ++templenth ] = RTC_time.seconds;
			 }
		
			 /*整车数据*/
			 templenth=Vehicle_data_upload(templenth);
			 
			 /*驱动电机数据*/
			 templenth=Drive_motor_data_upload(templenth);
			 
			 ///*发动机数据*/
			 //templenth=Engine_data_upload(templenth);
			 //templenth=Engine_Sys_Temp(templenth);
			 
			 /*车辆位置*/
			 templenth=Vehicle_position_data_upload(templenth);
			 
			 /*极值数据*/
			 templenth=Extreme_value_data_upload(templenth);
			 
			 /*报警数据*/
			 templenth=Warning_Data_Upload(templenth);
			 
			 /*用户自定义数据*/
			 //templenth=User_defined_data_upload(templenth,0x80);
			 
			 /*可充电装置电压数据*/
			 templenth=Voltage_data_for_rechargeable_devices(templenth);
			 
			 /*可充电装置温度数据*/
			 templenth=Temperature_data_for_rechargeable_devices(templenth);
			
			 /*数据组包并BCC校验*/
			 totalLength = Gprs_ProtocolDataEncode ( msgDeadData_Strorage, templenth+1);//指针元素的个数

			 FIFO_FlashBuf.DataBytes=totalLength;
		   
			 #define ALARMREAL_ATDEADTIMECOUNT     30
			 
			 if((FIFI_Stop_Enable==1)&&(AlarmReal_AtDeadTimeCounter<ALARMREAL_ATDEADTIMECOUNT+1))
			 {
						AlarmReal_AtDeadTimeCounter++;
			 }
			 if(
				 (Counter%StorageCycleTime==0)||\
					(FIFI_Stop_Enable==1&&AlarmReal_AtDeadTimeCounter<=ALARMREAL_ATDEADTIMECOUNT))
			 {

					 if(FIFO_flash_IsFull(pFlashBuf))
					 {
							 FIFO_flash_GetOne(pFlashBuf, FIFO_temp);
					 }
					 FIFO_flash_AddOne(pFlashBuf,(u8*)msgDeadData_Strorage);
					 /*补发数据标志*/
					 if(pFlashBuf->Counter!=0)
					 {
							 pFlashBuf->offsetdata=0x8080;
					 }
					 //printf("\r\n[STORAGE EVENT]:DeadData_Strorage\r\n");		
			 }
			 /*FIFO缓存区：每秒存一次数据*/	
			 if(!FIFI_Stop_Enable)//离线状态下，出现三级报警，停止存储
			 {	
						if(FIFO_IsFull(pRingBuf))
						{
							FIFO_GetOne(pRingBuf, FIFO_temp);
						}
						//printf("\r\n[STORAGE EVENT]:AlarmData_Strorage\r\n");
						FIFO_AddOne(pRingBuf,(u8*)msgDeadData_Strorage);
			 }

			 if ( SHABUF_FreeBuffer( msgDeadData_Strorage ) == TRUE )
			 {
					 msgDeadData_Strorage = NULL;
			 }

	}else
	{
			printf("\r\n[STORAGE EVENT]:DeadData_Strorage --> Point is error \r\n");
	
	}

}


/*
*******************************************************************
*函数名：Offset_data_upload
*参  数：无
*功  能： 1.多包补发数据上传（32960） ----盲区补偿
*				  2.时间补偿作用
*创建人: gl
* 
*******************************************************************
*/
u16 OffsetData_UploadEx(u8 *ringbuf)
{
	int i;	
	ringbuf[2]=0x03;
	if(OffsetData_time_ss>=50)
	{
		OffsetData_time_ss%=50;
		if(OffsetData_time_mm==59)
		{
			OffsetData_time_mm=0;
			OffsetData_time_hh+=1;
		}	
		else
		OffsetData_time_mm+=1;
	}
	else
	{
		OffsetData_time_ss+=10;
	}

	//对补发数据时间戳校准
	ringbuf[28]=OffsetData_time_hh;
	ringbuf[28]=OffsetData_time_mm;
	ringbuf[29]=OffsetData_time_ss;	
	
	printf("      TimeStamp: %02u:%02u:%02u  \r\n",ringbuf[27],ringbuf[28],ringbuf[29]);

	ringbuf[DeadData_lenth-1]=BCC_CheckSum((ringbuf+2),(DeadData_lenth-3));


	return TCP_DataOutput (ringbuf, DeadData_lenth );

}

/*
*******************************************************************
*函数名：Offset_data_upload
*参  数：无
*功  能：多包补发数据上传（32960） ----盲区数据
*创建人: gl
* 
*******************************************************************
*/
u16 OffsetData_Upload(u8 *ringbuf)
{								  
	int i;	
	ringbuf[2]=0x03;	
	
	
	
	printf("\r\nAlarm grede: %d  sign:%x  \r\n",ringbuf[91],ringbuf[94]);
	
#if 0
	printf ( "\r\n^^^^^^^^^^^^^^^^^^^^^^^^\r\n");
	for(i=0;i<DeadData_lenth;i++)
	{
		printf("%02x\t",ringbuf[i]);		
	}
	printf ( "\r\n");	
#endif	
	ringbuf[DeadData_lenth-1]=BCC_CheckSum((ringbuf+2),(DeadData_lenth-3));
	return TCP_DataOutput(ringbuf, DeadData_lenth);

}


/*
*******************************************************************
*函数名：Offset_data_upload_tcp
*参  数：无
*功  能：
*					1.多包补发数据上传（32960） ----适用于3级报警补发
*         2.时间补偿作用
*创建人: gl
* 
*******************************************************************
*/
u16 Offset_data_upload_tcp(u8 *ringbuf)
{								  
	int i;	
	ringbuf[2]=0x03;	

#if TIME_BIAS_ENABLE ==  1

#if SEND_GRADEDOWN	== 1      //降序发送数据		
			if(offset_data_time_ss==0)
			{
				offset_data_time_ss=59;
				if(offset_data_time_mm==0)
				{
					offset_data_time_mm=59;
					offset_data_time_hh-=1;
				}
				else
				offset_data_time_mm-=1;
			}
			else
			{
				offset_data_time_ss--;
			}
			ringbuf[27]=offset_data_time_hh;
			ringbuf[28]=offset_data_time_mm;
			ringbuf[29]=offset_data_time_ss;
			
#else	//升序发送数据
			ringbuf[27]=offset_data_time_hh;
			ringbuf[28]=offset_data_time_mm;
			ringbuf[29]=offset_data_time_ss;
			if(offset_data_time_ss==59)
			{
				offset_data_time_ss=0;
				if(offset_data_time_mm==59)
				{
					offset_data_time_mm=0;
					offset_data_time_hh+=1;
				}	
				else
					offset_data_time_mm+=1;
			}
			else
			{
				offset_data_time_ss++;
			}		
#endif


#endif
	printf("      TimeStamp:  %02u:%02u:%02u  \r\n",ringbuf[27],ringbuf[28],ringbuf[29]);
	ringbuf[DeadData_lenth-1]=BCC_CheckSum((ringbuf+2),(DeadData_lenth-3));
	return  TCP_DataOutput(ringbuf, DeadData_lenth);

}


/*
*******************************************************************
*函数名：Offsetdata_upload_DeadTime
*参  数：无
*功  能：
*					1.多包补发数据上传（32960） ----适用于3级报警补发
*         2.时间补偿作用
*创建人: gl
* 
*******************************************************************
*/
u16 Offsetdata_upload_DeadTime(u8 *ringbuf)
{								  
	int i;	
	ringbuf[2]=0x03;	
	
	printf("\r\n\t\t\t\t Alarm grede: %d  sign:%x  \r\n",ringbuf[91],ringbuf[94]);

#if 0
	printf ( "\r\n^^^^^^^^^^^^^^^^^^^^^^^^\r\n");
	for(i=0;i<DeadData_lenth;i++)
	{
		printf("%02x\t",ringbuf[i]);		
	}
	printf ( "\r\n");	
#endif	

	printf("\r\n\t\t\t\t\t\t\t\t TimeStamp:  %02u:%02u:%02u  \r\n",ringbuf[27],ringbuf[28],ringbuf[29]);
	ringbuf[DeadData_lenth-1]=BCC_CheckSum((ringbuf+2),(DeadData_lenth-3));
	return  TCP_DataOutput(ringbuf, DeadData_lenth);

}
/*
*******************************************************************
*函数名：User_defined_data_upload
*参  数：无
*功  能：终端校时
*创建人: gl
* 
*******************************************************************
*/

u16 Terminal_correction_time(u16 lenth,u8 order)
{
	if ( msgCorrectionTime == NULL )
  {
    msgCorrectionTime = SHABUF_GetBuffer ( TIME_CORRECT_BUFFER_LEN );
  }

  if ( msgCorrectionTime )
  {
    u16 totalLength = 0;
    u8 *msg = ( u8* )&msgCorrectionTime[ 2 ];

	//命令单元
    msg[ 0 ] = 0x08; //命令ID   
		msg[ 1 ] = 0xFE; 

	//VIN
    memcpy ( ( u8* )&msg[ 2 ], vehicleParams.CarVIN, 17 );

    //加密方式
	msg[ 19 ] =0x01;

	//数据单元长度
	msg[ 20 ] =0x00;
	msg[ 21 ] =0x00;


  
    /*数据格式化并校验*/
    totalLength = Gprs_ProtocolDataEncode(msgCorrectionTime, 22 );
#if DEBUG_32960==1
	printf ( "\r\n********_______msgCorrectionTime!!!\r\n");
#endif
    /*心跳数据发送*/
    TCP_DataOutput ( msgCorrectionTime, totalLength );

    /* 释放已分配的缓存 */
    if ( SHABUF_FreeBuffer( msgCorrectionTime ) == TRUE )
    {
      msgCorrectionTime = NULL;
    }
  }
}




/* 
 * 功能描述：终端上传心跳
 * 引用参数：无
 *     
 * 返回值  ：无
 * 
 */
extern void Gprs_Heart ( void )
{
  if ( msgHeartBeat == NULL )
  {
    msgHeartBeat = SHABUF_GetBuffer ( GPRS_HEART_BUFFER_LEN );
  }

  if ( msgHeartBeat )
  {
    u16 totalLength = 0;
    u8 *msg = ( u8* )&msgHeartBeat[ 2 ];

		//命令单元
    msg[ 0 ] = 0x07; //命令ID   
		msg[ 1 ] = 0xFE; 

		//VIN
    memcpy ( ( u8* )&msg[ 2 ], vehicleParams.CarVIN, 17 );

    //加密方式
		msg[ 19 ] =0x01;

		//数据单元长度
		msg[ 20 ] =0x00;
		msg[ 21 ] =0x00;

  
    /* 数据格式化并校验 */
    totalLength = Gprs_ProtocolDataEncode ( msgHeartBeat, 22 );

#if DEBUG_32960==1
	printf ( "\r\n********_______send_heartbeat!!!\r\n");
#endif
    /* 心跳数据发送 */ 
    TCP_DataOutput ( msgHeartBeat, totalLength );
		
		#if SD_WRITE_ENABLE==1
		if(upload_switch==0) //正常发送数据SD写入心跳 ，3级报警时，不写入心跳
		{
				char HeartData[60];
				sprintf(HeartData,"\r\n\r\nHeartData SendTime  20%02u%02u%02u %02u:%02u:%02u\r\n",RTC_time.year, RTC_time.month, RTC_time.date,RMC_d.time_hh,RMC_d.time_mm,RMC_d.time_ss);
				SD_WriteFile(&file,SD_filename,(u8*)HeartData,strlen(HeartData));
			
				HexToAscii(SD_Heartbeet_HexToAscii,msgHeartBeat,totalLength,true);
				SD_WriteFile(&file,SD_filename,SD_Heartbeet_HexToAscii,2*totalLength+2);
		}
    #endif
		
		/* 释放已分配的缓存 */
    if ( SHABUF_FreeBuffer( msgHeartBeat ) == TRUE )
    {
				msgHeartBeat = NULL;
    }
		
  }
}

/*
*******************************************************************
*函数名：Gprs_Vehicle_terminal_control_response
*参  数：无
*功  能：终端控制命令的应答
*描  述：对平台下发的数据判断，并应答
*创建人: gl
* 
*******************************************************************
*/
static bool Gprs_VehicleTerminalControl_Response(u16 len,u8* data)
{
  int i=0;
  bool result=FALSE;
  u8 * pResponse=NULL;
  if ( msgLogin == NULL )
  {
    pResponse = SHABUF_GetBuffer ( len );
  }
  if ( pResponse )
  {
    u16 totalLength = 0;

		memcpy(pResponse,data,len);

		pResponse[3]=((BCC_CheckSum(&pResponse[2],len-3)==data[len-1])?1:2);
		if(pResponse[3]==1)result=TRUE;
    /*数据组包并BCC校验*/
		pResponse[len-1]=BCC_CheckSum(&pResponse[2],len-3);

    /* 登陆数据发送 */
    TCP_DataOutput ( pResponse, len );
		printf ( "\r\n");
		for(i=0;i<len;i++)
   	{
   	 	printf ( "%02x ",pResponse[i]);
   	}
		printf ( "\r\n");
  	/* 释放已分配的缓存 */
  	if(SHABUF_FreeBuffer( pResponse ) == TRUE )
  	{
  		pResponse = NULL;
		
  	}
  }
  return  result;
}

/*
*******************************************************************
*函数名：Gprs_VehicleTerminalControl
*参  数：无
*功  能：终端控制命令
*创建人: gl
* 
*******************************************************************
*/
static void Gprs_VehicleTerminalControl(u16 len,u8* data)
{
	u8* msg=data;;
	u16 temp_len=len;
	switch(msg[0])
	{
		case 0x00:
			break;
		case 0x01:
			printf("\r\n************命令ID:%d　***********\r\n" ,msg[0]);
			u16 UpGrade_datalen=(len-2);
		  ftp_parseDataHdlr(UpGrade_datalen,&msg[1]);
			break;
	    case 0x02:
			printf("\r\n************命令ID:%d　***********\r\n" ,msg[0]);

			break;
		case 0x03:
			printf("\r\n************命令ID:%d　***********\r\n" ,msg[0]);
			System_Reset();
			break;
		case 0x04:
			printf("\r\n************命令ID:%d　***********\r\n" ,msg[0]);
			break;
		case 0x05:
			printf("\r\n************命令ID:%d　***********\r\n" ,msg[0]);
			XMIT_Stop();
			break;
		case 0x06:
			printf("\r\n************命令ID:%d　***********\r\n" ,msg[0]);
			break;
		case 0x07:
			printf("\r\n************命令ID:%d　***********\r\n" ,msg[0]);
			break;
		default:
			printf("\r\n************命令ID异常***********\r\n");
			break;
	}

}

/*
*******************************************************************
*函数名：Gprs_Parameters_Query
*参  数：无
*功  能：参数查询
*创建人: gl
* 
*******************************************************************
*/

void Gprs_ParametersQuery(u16 len,u8* data)
{
	
	u8* msg=NULL;
	u16 length=0;
	u16 ServiceDomainNameLen =strlen((char*)Parameter_Query_Data.manage_domain_name);
	u16 PublicDomainNameLen  =strlen((char*)Parameter_Query_Data.public_domain_name);
	
	if(msgParametersQuery==NULL)
	{
		msgParametersQuery=SHABUF_GetBuffer(100+ServiceDomainNameLen+PublicDomainNameLen);
	}

	if(msgParametersQuery)
  {
		/* 获取平台数据的前31个字节，到参数命令总数截止 */
		memcpy(msgParametersQuery,data,31);
		//命令标志格式化
		msg=&msgParametersQuery[2];
    msg[1]=0x01;	

		
		/*  获取平台所需查询的参数个数  */
    u16 query_cmdtotal=msgParametersQuery[30];
		
    //指针下标偏移至命令参总数的下一个地址处
		length=28;
/*
*===================================================================================	
*	多参查询逻辑
*             @goly
*===================================================================================
*/
		for(u16 p=0;p<query_cmdtotal;p++)
		{
				switch(data[31+p])
				{
					case 0x01:
						msg[++length]=0x01;
						msg[++length]=(Parameter_Query_Data.device_storeage_cycletime>>8)&0xff;
						msg[++length]=Parameter_Query_Data.device_storeage_cycletime&0xff;
						continue;
					case 0x02:
						msg[++length]=0x02;
						msg[++length]=(Parameter_Query_Data.dataupload_cycletime>>8)&0xff;
						msg[++length]=Parameter_Query_Data.dataupload_cycletime&0xff;
						continue;
					case 0x03:
						msg[++length]=0x03;
						msg[++length]=(Parameter_Query_Data.warning_upload_cycletime>>8)&0xff;
						msg[++length]=Parameter_Query_Data.warning_upload_cycletime&0xff;
						continue;
					case 0x04:
						msg[++length]=0x04;
						msg[++length]=Parameter_Query_Data.manage_domain_name_len;
						continue;
					case 0x05:
						msg[++length]=0x05;
						for(u16 i=0;i<Parameter_Query_Data.manage_domain_name_len;i++)
						{
								msg[++length]=Parameter_Query_Data.manage_domain_name[i];
						}
						continue;
					case 0x06:
						msg[++length]=0x06;
						msg[++length]=(Parameter_Query_Data.manage_port>>8)&0xff;
						msg[++length]=Parameter_Query_Data.manage_port&0xff;
						continue;
					case 0x07:
						msg[++length]=0x07;
						msg[++length]=Parameter_Query_Data.hard_version[0];
						msg[++length]=Parameter_Query_Data.hard_version[1];
						msg[++length]=Parameter_Query_Data.hard_version[2];
						msg[++length]=Parameter_Query_Data.hard_version[3];
						msg[++length]=Parameter_Query_Data.hard_version[4];
						continue;
					case 0x08:
						msg[++length]=0x08;
						msg[++length]=Parameter_Query_Data.soft_version[0];
						msg[++length]=Parameter_Query_Data.soft_version[1];
						msg[++length]=Parameter_Query_Data.soft_version[2];
						msg[++length]=Parameter_Query_Data.soft_version[3];
						msg[++length]=Parameter_Query_Data.soft_version[4];
						continue;
					case 0x09:
						msg[++length]=0x09;
						msg[++length]=Parameter_Query_Data.heartbeat_cycletime;
						continue;
					case 0x0A:
						msg[++length]=0x0A;
						msg[++length]=(Parameter_Query_Data.device_ack_timeout>>8)&0xff;
						msg[++length]=Parameter_Query_Data.manage_port&0xff;
						continue;
					case 0x0B:
						msg[++length]=0x0B;
						msg[++length]=(Parameter_Query_Data.platform_ack_timeout>>8)&0xff;
						msg[++length]=Parameter_Query_Data.platform_ack_timeout&0xff;
						continue;
					case 0x0C:
						msg[++length]=0x0C;
						msg[++length]=Parameter_Query_Data.login_fail_next_loglin_time;
						continue;
					case 0x0D:
						msg[++length]=0x0D;
						msg[++length]=Parameter_Query_Data.public_domain_name_len;
						continue;
					case 0x0E:
						msg[++length]=0x0E;
						for(u16 i=0;i<Parameter_Query_Data.public_domain_name_len;i++)
						{
								msg[++length]=Parameter_Query_Data.public_domain_name[i];
						}	
						continue;
					case 0x0F:
						msg[++length]=0x0F;
						msg[++length]=(Parameter_Query_Data.public_port>>8)&0xff;
						msg[++length]=Parameter_Query_Data.public_port&0xff;
						continue;
					case 0x10:
						msg[++length]=0x10;
						msg[++length]=Parameter_Query_Data.sample_test;
						continue;
					default:
						continue;
				}																									
		}

		/* 数据格式化并校验 */
    u16 totalLength = Gprs_ProtocolDataEncode( msgParametersQuery,length+1);

		for(u16 i=0;i<totalLength;i++)
	  {
	   	 	printf ( "%02x ",msgParametersQuery[i]);
	  }
	  printf ( "\r\n");
		
		TCP_DataOutput(msgParametersQuery, totalLength );
		
		/* 释放已分配的缓存 */
    if( SHABUF_FreeBuffer( msgParametersQuery) == TRUE )
    {
				msgParametersQuery = NULL;
    }
	
	}
}

/*
*******************************************************************
*函数名：Gprs_ParametersQuery_Response
*参  数：无
*功  能：参数查询应答
*创建人: gl
* 
*******************************************************************
*/

void Gprs_ParametersQuery_Response(u16 len,u8* data)
{

	
	
}

/*
*******************************************************************
*函数名：Gprs_Parameters_Set
*参  数：无
*功  能：参数设置
*创建人: gl
* 
*******************************************************************
*/

void Gprs_ParametersSet(u16 len,u8* data)
{
	
	
		u8 *msg=NULL;
		u16 dataUnitLen=0;
		dataUnitLen=(data[22]<<8)|data[23];	
		//获取设置命令参数个数
		u16 setting_cmd_total=data[30];
		//指针指向第一个参数ID
		msg=&data[31];
		
		u16 length=0;
	
		for(u16 p=0;p<setting_cmd_total;p++)
		{
				switch(*msg)
				{
					case 0x01:
						Parameter_Query_Data.device_storeage_cycletime=((*(msg+1))<<8)|(*(msg+2));
						msg+=3;
						printf("\r\n device_storeage_cycletime:%d \r\n",Parameter_Query_Data.device_storeage_cycletime);
						continue;
					case 0x02:
					
						Parameter_Query_Data.dataupload_cycletime=((*(msg+1))<<8)|(*(msg+2));
						printf("\r\n dataupload_cycletime:%d \r\n",Parameter_Query_Data.dataupload_cycletime);
						msg+=3;
						continue;
					case 0x03:
					
						Parameter_Query_Data.warning_upload_cycletime=((*(msg+1))<<8)|(*(msg+2));
						printf("\r\n dataupload_cycletime:%d \r\n",Parameter_Query_Data.warning_upload_cycletime);
						msg+=3;
						continue;
					case 0x04:
						
						Parameter_Query_Data.manage_domain_name_len=(*(msg+1));
						printf("\r\n manage_domain_name_len:%d \r\n",Parameter_Query_Data.manage_domain_name_len);
						msg+=2;
						continue;
					case 0x05:
						//平台下发的有问题是域名，不是IP
						memset(Parameter_Query_Data.manage_domain_name,0,sizeof(Parameter_Query_Data.manage_domain_name));
						for(u16 i=0;i<Parameter_Query_Data.manage_domain_name_len;i++)
						{
								Parameter_Query_Data.manage_domain_name[i]=(*(msg+i+1));
						}
						printf("\r\n manage_domain_name:%s \r\n",Parameter_Query_Data.manage_domain_name);
						msg+=Parameter_Query_Data.manage_domain_name_len+1;
						continue;
					case 0x06:
			
						Parameter_Query_Data.manage_port=((*(msg+1))<<8)|(*(msg+2));
						printf("\r\n manage_port:%d \r\n",Parameter_Query_Data.manage_port);
						msg+=3;
						continue;
					case 0x07:
						memset(Parameter_Query_Data.hard_version,0,sizeof(Parameter_Query_Data.hard_version));
						for(u8 q=0;q<5;q++)
						{
								Parameter_Query_Data.hard_version[q]=(*(msg+q+1));
						}
						printf("\r\n hard_version:%s \r\n",Parameter_Query_Data.hard_version);
						msg+=6;
						continue;
					case 0x08:
						memset(Parameter_Query_Data.soft_version,0,sizeof(Parameter_Query_Data.soft_version));
						for(u8 p=0;p<5;p++)
						{
							Parameter_Query_Data.soft_version[p]=(*(msg+p+1));
						}
						printf("\r\n soft_version:%s \r\n",Parameter_Query_Data.soft_version);
						msg+=6;
						continue;
					case 0x09:
						
						Parameter_Query_Data.heartbeat_cycletime=(*(msg+1));
						printf("\r\n heartbeat_cycletime:%d \r\n",Parameter_Query_Data.heartbeat_cycletime);
						msg+=2;
						continue;
					case 0x0A:
	
						Parameter_Query_Data.device_ack_timeout=((*(msg+1))<<8)|(*(msg+2));
						printf("\r\n device_ack_timeout:%d \r\n",Parameter_Query_Data.device_ack_timeout);
						msg+=3;
						continue;
					case 0x0B:
						
					  Parameter_Query_Data.platform_ack_timeout=((*(msg+1))<<8)|(*(msg+2));
						printf("\r\n platform_ack_timeout:%d \r\n",Parameter_Query_Data.platform_ack_timeout);
						msg+=3;
						continue;
					case 0x0C:
						
						Parameter_Query_Data.login_fail_next_loglin_time=(*(msg+1));
						printf("\r\n login_fail_next_loglin_time:%d \r\n",Parameter_Query_Data.login_fail_next_loglin_time);
						msg+=2;
						continue;
					case 0x0D:
						
						Parameter_Query_Data.public_domain_name_len=(*(msg+1));
						printf("\r\n public_domain_name_len:%d \r\n",Parameter_Query_Data.public_domain_name_len);
						msg+=2;
						continue;
					case 0x0E:
						//平台下发的有问题是域名，不是IP
						memset(Parameter_Query_Data.public_domain_name,0,sizeof(Parameter_Query_Data.public_domain_name));
						for(u16 i=0;i<Parameter_Query_Data.public_domain_name_len;i++)
						{
								Parameter_Query_Data.public_domain_name[i]=(*(msg+i+1));
						}	
						msg+=Parameter_Query_Data.public_domain_name_len+1;
						printf("\r\n public_domain_name:%s \r\n",Parameter_Query_Data.public_domain_name);
						continue;
					case 0x0F:
						
						Parameter_Query_Data.public_port=((*(msg+1))<<8)|(*(msg+2));
						printf("\r\n public_port:%d \r\n",Parameter_Query_Data.public_port);
						msg+=3;
						continue;
					case 0x10:
						
						Parameter_Query_Data.sample_test=(*(msg+1));
						printf("\r\n sample_test:%d \r\n",Parameter_Query_Data.sample_test);
						msg+=2;
						continue;
					default:
						continue;
				}																									
		}










}
/*
*******************************************************************
*函数名：Gprs_ParametersSet_Response
*参  数：无
*功  能：参数设置应答
*创建人: gl
* 
*******************************************************************
*/

void Gprs_ParametersSet_Response(u16 len,u8* data)
{









}
