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

/*�ⲿ����*/
extern status_param drv_param;

extern bool MDM_SWITCH_TO;

/*
*====================================================================================================
* ȫ�ֱ�����
*====================================================================================================
*/
//��¼��ˮ��
u16 login_serial=0;

/*3����������  1:������������ 0����������     */
u16 alarm_alive = 0;


/******************************************/
/*              �ڲ�����[����]            */
/******************************************/	

/* GPRSʱ������ */
gprs_tmr_conf_t gprsTmr = {0};

/* ���幤������ */
static worker_t tzWorker;



/***************/
/* ��ʱ������  */
/***************/

/* GPRS���䶨ʱ����� */
tmr_t *tmrGprsProc = NULL;
tmr_t *tmrRealData = NULL;

tmr_t *tmrOffsetData = NULL;
tmr_t *tmrDeadData = NULL;
tmr_t *tmrCrrectTime = NULL;


/******************************************/
/*              �ڲ�����[����]            */
/******************************************/

static void sysGprsTmrInit ( void );
static void Tmr_GprsApp ( u8 ID );
static void tzWorker_DoTasks ( void* data );




/**
*===============================================================================================
*	���������
*===============================================================================================
*/
/*ʵʱ����ʱ�������*/
static vu32 upload_timecount=0;

/*30�뱨����ʱ��*/
static vu16 alarm_timecount=0;

/*���������� */
static u16 Heart_Counter=0;
static u16 Reissue_Counter=0;

/*Ӧ����������*/
static s32 sysLoginCount =0;
static s32 sysRealDataCount =0;
static s32 sysHeartbeat=0;

/*�����ǳ�������*/
static u16 logout_counter=0;
static u16 LogoutFlag=0;


/*
*==============================================================================================
*ä�����ݲ��������ڲ���ʱ��ä�����ݲ���ʱ��׷�Ӳ�������
*==============================================================================================
*/
/*ä������׷������*/
static s8 flashbuf_2index=1;  
static u8 OffsetData_time_hh=0;
static u8 OffsetData_time_mm=0;
static u8 OffsetData_time_ss=0;


/*ʵʱ�����ϴ����أ�����3������ʱ��ʵʱ�����ϴ�ʱ��(����<=1s��������30��ָ���ͬʱ�����������ƣ���������ǰ30s�ڵ����ݣ�����30sһ����*/
u8 upload_switch=0;

//3��������������
s8 ReissueData_Upload_Switch=-1;


//ä������ʱ����֤ʵʱ�������ڲ�������
static u8 realdata_send_first=0;
static s8 DeadData_Sem=-1;

/*�����Ƿ���ä��*/
s8 IsDeadZone_Network=-1;


/*
*==============================================================================================
* ��ر�־��
*==============================================================================================
*/

//FIFO��ʼ����־  
static s8 FIFO_Init_flag =-1 ;
//3������������־
u8 upload_switch_state=0;

//�����ǳ���־

s8 logout_flag=-1;


/*
*==============================================================================================
*	SD���洢���
*==============================================================================================
*/
/*SD���ļ�ϵͳ���ļ���*/
char SD_filename[30];

#if  SD_WRITE_ENABLE==1
static u8 SD_RealData_HexToAscii[2*REALDATA_UPLOAD_BUFFER_LEN+2];
static u8 SD_ReissueData_HexToAscii[2*REALDATA_UPLOAD_BUFFER_LEN+2];
static u8 SD_Heartbeet_HexToAscii[60];
#endif



/*
*===================================================================================================
* FIFO�����ݱ�����
*===================================================================================================
*/
/*������ʱ����*/
static u8 Offsetdata_ss[REALDATA_UPLOAD_BUFFER_LEN]={0};
static u8 Offsetdata_dd[REALDATA_UPLOAD_BUFFER_LEN]={0};

/* 30s�������� */
static u8 FIFO_Buffer[ BUFFER_FIFO_SIZE] = { 0 };
/*���ζ��еĻ���*/
static u8 FIFO_temp[REALDATA_UPLOAD_BUFFER_LEN]={0};

/*FIFO�洢���أ� -1���洢���� 1��ֹͣ�洢����*/
static s8 FIFO_switch=-1;

/*
*===========================================================================================
* 32960Э������
*============================================================================================
*/


/*
*=======================================
* �궨����
*/
/* ʵʱ���ݷ���Ƶ�ʣ�10s  */
#define REALDATA_SEND_PERIOD_TIME       10


/*
*=======================================
* �����ʼ����
*/
/* ��¼��Ϣ */
static u8 *msgLogin              = NULL;
/* �ǳ���Ϣ */
static u8 *msgLogout             = NULL;
/* ʵʱ�����ϴ� */
static u8 *msgRealData_upload    = NULL;
/* ä�����ݴ洢 */
static u8 *msgDeadData_Strorage  = NULL;
/* �ն�Уʱ */
static u8 *msgCorrectionTime     = NULL;
/* ������Ϣ */
static u8 *msgHeartBeat          = NULL;
/* ������ѯӦ��*/
static u8* msgParametersQuery    = NULL;
/* ��½��ʱ���� */
static s32 msgLoginHandler            = 0;
/* �ǳ���ʱ���� */
static s32 msgLogoutHandler           = 0;



/*
*=======================================
* ä��������ر���
*/

/*ä������ */
static s8 DeadData_FlashConfig=-1;
static u16 DeadData_lenth=0;

/*ä��ʱ��--> ��������*/
static u8 Alarm_AtDeadtime=0;				//1����������
/*ä��ʱ��--> ��������ʱ��ä���Ѵ�����  */
static u32 FlashDeadStoragedIndex_AlarmPoint=0;

/*����ä��ʱ��������������¼ʵʱ���ݴ洢����*/
static u16 AlarmReal_AtDeadTimeCounter=0;

/*�ظ�������Ŀ */
#define DROP_REPEAT_DEADDATA_NUM    3

/*ȥ���ظ����� */
static u8 Drop_RepeatDeadData_Count=DROP_REPEAT_DEADDATA_NUM;


enum  DEADDATA_PTHREAD 
{
	NO_THREAD=0,
	//FLASH�̷߳�������
	THREAD_FLASH_SENF=1,
	
	//FIFI�̷߳�������
	THREAD_FIFO_SENF,

  /* ��С�Ѿ��޶������������޸ģ� */
  THREAD_OBJ_NB = 3
};

static enum DEADDATA_PTHREAD  Dead_Pthread_states=NO_THREAD;


/*
*=======================================
* 3���������
*/
/* ������ʱ���У׼*/
static u8 offset_data_time_hh=0;
static u8 offset_data_time_mm=0;
static u8 offset_data_time_ss=0;
/* ʵʱ���ݵ�ʱ���У׼*/
static u8 real_data_time_hh=0;
static u8 real_data_time_mm=0;
static u8 real_data_time_ss=0;


/*
*===========================================================================================
* 32960Э����ع��ܴ�����
*============================================================================================
*/
/*Э�鴦����غ���*/
static u16  CAL_DataUnit_length(u8 *buf);
static u8 	BCC_CheckSum(u8 *buf, u16 len);
static u16  Gprs_ProtocolDataEncode ( u8 *data, u16 len );

static u16 data_pack_head(u8 order);//Э��������ͷ
/* ������¼��ˮ���к� */
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
* �ⲿ����
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
/*              ��ʱ��������Ӧ����        */
/******************************************/

/* 
 * ��������������GPRS��ʱ��
 * ���ò�����
 *     
 * ����ֵ  ��
 * 
 */
static void sysGprsTmrInit ( void )
{
  /* ��ʼ���������� */
  tzWorker.data = NULL;
  tzWorker.func = tzWorker_DoTasks;  
}



/* 
 * ��������������
 * ���ò�����
 *     
 * ����ֵ  ��
 * 
 */
void TmrHeartBeatHdlr(u8 times)    //Ĭ�ϵ�λ��30s
{
		Gprs_Heart();
}
/* 
 * ����������ʵʱ����
 * ���ò�����
 *     
 * ����ֵ  ��
 * 
 */
void TmrRealDataHdlr(u8 times)    //Ĭ�ϵ�λ��1s
{

		if(upload_switch_state==0||Alarm_AtDeadtime==1)
		{
				if(upload_switch_state==1||upload_switch==1)
				{
						alarm_alive=1;//����״̬������ֶ����		
				}
				if(upload_switch==0)
				{
						++upload_timecount;
						if(upload_timecount==REALDATA_SEND_PERIOD_TIME)realdata_send_first=1;
						
						Multi_realdata_upload(upload_timecount,REALDATA_SEND_PERIOD_TIME,0);

						if(realdata_send_first==1)
						{
								DeadData_Sem=1;//���ã�ä��ʱ��ʵʱ�������ȼ�����ä������	
								if(pRingBuf->Counter&&Dead_Pthread_states==NO_THREAD )Dead_Pthread_states=THREAD_FLASH_SENF;
						}
				}
				else//����3������
				{
							alarm_timecount++;
							upload_timecount=0;//ֹͣ����
							if(alarm_timecount<=29)//���뱨��-->ʵʱ���ݿ�ʼһ��һ��
							{
									Multi_realdata_upload(upload_timecount,0,TRUE);//�������ڣ�1s
									//ReissueData_Upload_Switch=1;//3������һ������
							}
							else	//30s���˳�����״̬
							{
									ReissueData_Upload_Switch=1;//3������һ��һ��
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
						offset_data_time_ss=(RMC_d.time_ss+30)%60;//ʱ���������
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
					  offset_data_time_ss=(RTC_time.seconds+30)%60;//ʱ���������
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
			  FIFO_switch=1; //FIFOֹͣ�洢���� ,׼��������
				Multi_realdata_upload(upload_timecount,0,0);
				upload_switch=1;
			  upload_switch_state=0;	
		
		}
	
}

/* 
 * ������������������
 * ���ò�����
 *     
 * ����ֵ  ��
 * 
 */

void TmrOffsetDataHdlr(u8 times)    //Ĭ�ϵ�λ��1s
{
	/*upload_switch!=-1:����3��������FIFO_switch==-1���������ݴ洢 */
	/*���λ������ݲ���������3������������ǰ30s���ݣ�*/
	if(ReissueData_Upload_Switch==1)
	{
				 if(pRingBuf->Counter)	//��������
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
				 else if(pRingBuf->Counter<=0)  //�˳�����״̬
				 {
							printf("\r\n[3 LEVEL ALARM EVENT]: ReissueData-->Send Finish  >>QUIT \r\n");
							FIFO_switch=-1;  //FIFO�ָ��洢����
							ReissueData_Upload_Switch=0;		
				 }
	} 
  //ä��ʱ����3������
  if(Dead_Pthread_states==THREAD_FIFO_SENF)
	{
				 if(pRingBuf->Counter)	//��������
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
				 else if(pRingBuf->Counter<=0)  //�˳�����״̬
				 {
							//printf("\r\n[3 LEVEL ALARM EVENT]: ReissueData-->Send Finish  >>QUIT \r\n");
							FIFO_switch=-1;  //FIFO�ָ��洢����	
							//���������л���flash�����߳�
							Dead_Pthread_states=THREAD_FLASH_SENF;
				 }
	 }		
}



/*
*******************************************************************
*��������DeadDataUpload_FromFlash
*��  ������
*��  �ܣ�ä�����ݷ���
*������: gl
* 
*******************************************************************
*/
u16 DeadDataUpload_FromFlash(void)
{
	
	 /*FLASH������3���ڣ�TCP�쳣�Ĵ洢���ݲ�����*/
	 //��������
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
					//�Բ�������ʱ���У׼
					OffsetData_time_hh=Offsetdata_dd[27];
					OffsetData_time_mm=Offsetdata_dd[28];
					OffsetData_time_ss=Offsetdata_dd[29];
					
			}
	 }
	 else if(pFlashBuf->Counter==0&&flashbuf_2index>0) //׷�Ӳ�������������ʹ��
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
				/*ä��������ɺ����flash��Ϣ������ֹ�´��ϵ��ظ�����*/
				//FIFO_FLASH_OffsetDataConfig_Clear();
				Alarm_AtDeadtime=0;
				upload_switch_state=0;
	 }
}


/* 
 * �����������ն�Уʱ
 * ���ò�����
 *     
 * ����ֵ  ��
 * 
 */
void TmrCorrectTimeHdlr(u8 times)    //Ĭ�ϵ�λ��60s
{

}






/* 
 * ��������������Ӧ��
 * ���ò�������
 *           
 * ����ֵ  ����
 * 
 */
extern void App_tmrStart ( void )
{
	
#if  1
		printf("\r\n[APP EVENT]: Everything is ok, ME  is  Run......   .  \r\n");	
#endif
	
//		printf ( "\r\n[STORAGE EVENT]: Flash Storage Init....\r\n");
			FIFO_flash_init(pFlashBuf,DeadData_lenth,Dead_DATA_STORAGE_INDEX);
//		/*��ȡä������*/
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
 * �����������ر�Ӧ��
 * ���ò�������
 *     
 * ����ֵ  ����
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
 * ��������: ��������
 * ���ò�����
 *         
 * ����ֵ  ����
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
 * �����������ն��ϴ�����
 * ���ò�����
 *         
 * ����ֵ  ����
 * 
 */
static void tzWorker_DoTasks ( void* data )
{

		/*****************************/
		/*      ���Ź�               */
		/*****************************/
		//ι��
		WDG_KickCmd();
		__RestartGprsCounter=0;
		/**********************************
		 *       �ն�������¼����         *
		 **********************************/
#define LOGIN_TIMEOUT    1	
	
#if LOGIN_TIMEOUT == 1
		/*****************************/
		/*     ��������½��ʱ����    */
		/*****************************/
		#define SYS_LOGIN_MAX_TIMEOUT     100   /* ��λ��s */
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
							/* ��·����ֹ��� */
							/*******************************/
							/*     ��Ҫ���ǽ����Ĳ���      */
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
		/*    ʵʱ����Ӧ��ʱ       */
		/*****************************/
		if(sys_status >= PROC_SYSTEM_LOGIN_ACK&&sys_status<PROC_SYSTEM_LOGOUT)	
		{	
				/*****************************/
				/*     ʵʱ����Ӧ����      */
				/*****************************/
				#define SYS_REALDATA_MAX_TIMEOUT     20   /* ��λ��s */ 
				sysRealDataCount++;
				if(sysRealDataCount >= SYS_REALDATA_MAX_TIMEOUT)
				{
						if(RealData_Handler==NULL)
						{
								/*ʵʱ�����޷���*/
								/* 1.�ر�modem*/
								Modem_Stop(PROC_SYSTEM_REISSUEDATA_STORAGE);
								/* 2.����modem */
								RealData_Handler=TMR_Subscribe ( FALSE, 20, TMR_TYPE_100MS, ( tmr_procTriggerHdlr_t )Modem_Restart );
								//System_Reset();  
						}
				} 		
		}
#endif
	
#define HEARTBEAT_TIMEOUT     		1	
#if HEARTBEAT_TIMEOUT == 1
		/*****************************/
		/*    ��������Ӧ��ʱ       */
		/*****************************/
		if(sys_status >= PROC_SYSTEM_LOGIN_ACK&&sys_status<PROC_SYSTEM_LOGOUT)	
		{	
				/*****************************/
				/*     ��������Ӧ����      */
				/*****************************/
				#define SYS_HEARTDATA_MAX_TIMEOUT     60   /* ��λ��s */ 
				sysHeartbeat++;
				if ( sysHeartbeat >= SYS_HEARTDATA_MAX_TIMEOUT )
				{
						if(RealData_Handler==NULL)
						{
								printf("\r\n HeartBeat Check......  r\n");
								/*ʵʱ�����޷���*/
								/* 1.�ر�modem*/
								Modem_Stop(PROC_SYSTEM_REISSUEDATA_STORAGE);
								/* 2.����modem*/
								RealData_Handler=TMR_Subscribe ( FALSE, 20, TMR_TYPE_100MS, ( tmr_procTriggerHdlr_t )Modem_Restart );
						} 
				} 		
		}
	
#endif	
 


/*****************************/
/*         �ش�              */
/*****************************/




/*****************************/
/*         ä������          */
/*****************************/

/*
*	�洢ä������
*/
if(IsDeadZone_Network==PROC_SYSTEM_REISSUEDATA_STORAGE)
{ 
		#define  REISSUEDATA_COUNT    10    //ä���ṹ��洢����20s
		Reissue_Counter++;
	  
		if(upload_switch_state==1&&Alarm_AtDeadtime==0) 
		{
			FlashDeadStoragedIndex_AlarmPoint=pFlashBuf->Tail-3;
			printf("\r\nFlashDeadStoragedIndex_AlarmPoint: %d \r\n",FlashDeadStoragedIndex_AlarmPoint);
			Alarm_AtDeadtime=1;
			
		}
		/*
	  *===========================================================================
		*	�洢ä������  �洢���ڣ�10s
	  *===========================================================================
		*/
		DeadData_Strorage(Reissue_Counter,REISSUEDATA_COUNT,Alarm_AtDeadtime);
	  
	
	  /*
		* �洢ä�����ݽṹ��	���ڣ�10s
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
*	��ȡä��������Ϣ
*/
if(DeadData_FlashConfig==-1&&DeadData_Sem==1)
{
		DeadData_FlashConfig=1;
		DeadData_Sem=0;
		/*��ȡä������*/
		FIFO_FLASH_OffsetDataConfig_Read((u8*)&FIFO_FlashBuf,sizeof(FIFO_FLASH_TYPE));
	  /*�Ƿ񲹷�ä������*/
		if(FIFO_FlashBuf.offsetdata==0x8080)
		{
				/*��Ҫ��������*/
				//����flash����������Ϣ; ���ã���ֹÿ���ϵ���ظ���ȡ�ɵ�������Ϣ���������󲹷�		

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

//ä������������ע����ʱ�����˳�����
if(FIFO_FlashBuf.offsetdata==0x8080&&FIFO_FlashBuf.IsFinishSend==TRUE)
{
		if(tmrDeadData)
		{
				/*ä�����ͽ������³�ʼ��*/
				printf ( "\r\n[STORAGE EVENT]:: Flash Storage Init....\r\n");
				FIFO_flash_init(pFlashBuf,DeadData_lenth,Dead_DATA_STORAGE_INDEX);
				TMR_UnSubscribe(tmrDeadData,( tmr_procTriggerHdlr_t )DeadDataUpload_FromFlash,TMR_TYPE_10MS);
				tmrDeadData=NULL;
		}
}	
//ä�������У��ٴν���ä��
if(IsDeadZone_Network==PROC_SYSTEM_REISSUEDATA_STORAGE)
{
		if(tmrDeadData)
		{
				/*ע��ä������*/
				TMR_UnSubscribe(tmrDeadData,( tmr_procTriggerHdlr_t )DeadDataUpload_FromFlash,TMR_TYPE_10MS);
				tmrDeadData=NULL;
		}
}
	
		
/*****************************/
/*         ��������          */
/*****************************/  
#define  HEART_BEAT_COUNT    30    //�������
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
/*         ʵʱ���ݷ��ͣ�32960��              */
/**********************************************/

	if (sys_status>=PROC_SYSTEM_LOGIN_ACK&&sys_status<PROC_SYSTEM_LOGOUT) //������¼�ɹ��󣬿�����ʱ�� 10s
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
/*         3���������ݷ��ͣ�32960��           */
/**********************************************/
	if (sys_status>=PROC_SYSTEM_LOGIN_ACK) //������¼�ɹ��󣬿�����ʱ�� 3s
	{
			if(tmrOffsetData==NULL)
			tmrOffsetData = TMR_Subscribe ( TRUE, 100, TMR_TYPE_10MS, ( tmr_procTriggerHdlr_t )TmrOffsetDataHdlr );			 
	}
	else 
	if(sys_status<PROC_SYSTEM_LOGIN_ACK) //Ԥ�������������ݷ��ͽ�����ע����ʱ��
	{
			if(tmrOffsetData)
			TMR_UnSubscribe(tmrOffsetData,( tmr_procTriggerHdlr_t )TmrOffsetDataHdlr,TMR_TYPE_10MS);
			tmrOffsetData=NULL;
	}

/**********************************************/
/*         �ն�Уʱ���ͣ�32960��              */
/**********************************************/
//	if (sys_status>=PROC_SYSTEM_LOGIN_ACK ) //������¼�ɹ��󣬿�����ʱ�� 3s
//	{
//			if(tmrCrrectTime==NULL)
//			tmrCrrectTime = TMR_Subscribe ( TRUE, 3000, TMR_TYPE_10MS, ( tmr_procTriggerHdlr_t )TmrCorrectTimeHdlr );			 
//	}
//	else if(sys_status<PROC_SYSTEM_LOGIN_ACK) //Ԥ�������������ݷ��ͽ�����ע����ʱ��
//	{
//			if(tmrCrrectTime)
//			TMR_UnSubscribe(tmrCrrectTime,( tmr_procTriggerHdlr_t )TmrCorrectTimeHdlr,TMR_TYPE_10MS);
//			tmrCrrectTime=NULL;
//	}

/**********************************************/
/*         �����ǳ���32960��              		*/
/**********************************************/
#define LOGOUT_TIME       									10		//�����ǳ�׼��ʱ�� 5s	
	
	if(sys_status==PROC_SYSTEM_LOGOUT||logout_flag==1)
	{			
			logout_counter++;
			if(logout_counter==LOGOUT_TIME)
			{
				/*�����ǳ�*/
				printf("\r\n[LOGINOUT EVENT]: Begin Loginout....\r\n");
				vehicles_logout();
			}
	}
/*****************************/
/*   GSM�ź�������״̬��ѯ   */
/*****************************/

/*****************************/
/*          GPS����          */
/*****************************/

  GPS_ProcDataHdlr ();
}








/* 
 * �������������ݽ��� 
 * ���ò�����(1)����
 *           (2)����
 *         
 * ����ֵ  ����
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

  //��ƽ̨�������ݽ����ж�
  if ( data[ 0 ]==0x23 && data[1]==0x23 )
  {
  	switch(data[2])	//�������ж�
	{
	//������¼��ִ
	case 0x01:
		if(data[3]==1)
		{
			printf ( "\r\n[DataDecode EVENT]: LOGIN  SUCCESS  \r\n");

			//�����ն�״̬Ϊ����½�ɹ�
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
	//ʵʱ���ݻ�ִ
	case 0x02:
		sysRealDataCount=0;	
		//�����ն�״̬
		if (sys_status>=PROC_SYSTEM_LOGIN_ACK&&sys_status<PROC_SYSTEM_LOGOUT)
    sys_status = PROC_SYSTEM_REALDATA_ACK;
		//printf ( "\r\n[DataDecode EVENT]: RealData  Ack  Success!!! \r\n");
		break;
	//�������ݻ�ִ		
	case 0x03:
		//printf ( "\r\n[DataDecode EVENT]: ReissueData  Ack  Success!!!\r\n");
		break;
	//�����ǳ���ִ
	case 0x04:
		sys_status = PROC_SYSTEM_LOGOUT_ACK;
		printf("\r\n[DataDecode EVENT]: LOGIN OUT \r\n");
		break;
	//������ִ
	case 0x07:
		printf ( "\r\n[DataDecode EVENT]: Heartbeat Success!!!\r\n");
	  sysHeartbeat=0;
	  if (sys_status>=PROC_SYSTEM_LOGIN_ACK&&sys_status<PROC_SYSTEM_LOGOUT)
    sys_status = PROC_SYSTEM_HEART;
		break;
	//�ն�Уʱ��ִ
	case 0x08:
		//Gprs_Time_Update();
		printf ( "\r\n********___�ն�Уʱ��ִ____!!!\r\n");
		break;
	//ƽ̨�·��Ĳ�ѯ����
	case 0x80:
		
	  Gprs_ParametersQuery(len,data);
		#if 1
	   	printf ( "\r\n********___�յ�80��ִ��Ϣ____!!!\r\n");
	   	for(i=0;i<len;i++)
	   	{
	   	 	printf ( "%02x ",data[i]);
	   	}
	   	printf ( "\r\n");	
		#endif
		break;
	//ƽ̨�·�����������
	case 0x81:
		#if 1
	   	printf ( "\r\n********___�յ�81��ִ��Ϣ____!!!\r\n");
	   	for(i=0;i<len;i++)
	   	{
	   	 	printf ( "%02x ",data[i]);
	   	}
	   	printf ( "\r\n");
		#endif
			
		Gprs_ParametersSet(len,data);
		break;
	
	case 0x82:
		printf ( "\r\n********___�յ�82��ִ��Ϣ____ ���ȣ�%d \r\n",len);
		printf ( "\r\n");
		for(i=0;i<len;i++)
		{
			printf ( "%02x",data[i]);
		}
		printf ( "\r\n");
		//ƽ̨�·��ĳ����ն˿�������
		//if(Gprs_Vehicle_terminal_control_response(len,data))
		Gprs_VehicleTerminalControl((len-30),&data[30]);

		break;

	default:
		#if 1
	   	printf ( "\r\n********___�յ������ִ��Ϣ____!!!\r\n");
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

	    printf ( "\r\n********___��ʶ���Ӧ��____!!!\r\n");
  		#if 1
	   	printf ( "\r\n********___�յ������ִ��Ϣ____!!!\r\n");
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
*��������BCC_CheckSum
*��  ����ack_order����������
*��  �ܣ��������ݵ�Ԫ�ֽ���
*������: gl
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
*��������BCC_CheckSum
*��  ����ack_order����������
*��  �ܣ�BCC���У��
*������: gl
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
 * �������������ݱ���
 * ���ò�����(1)����
 *           (2)����
 *
 * ����ֵ  ��״̬��
 * 
 */
static u16 Gprs_ProtocolDataEncode ( u8 *data, u16 len )
{
  u8 i=0;
  u16 temp=0;
  u32 length=0; 
  
     
  //32960����ʼ����ʶ
  data[ 0 ] = 0x23;
  data[ 1 ] = 0x23;
  //�Բ�ͬ����ID�����ݵ�Ԫ���Ƚ��д���
  switch(data[2])
  {
   case 1:
		//��索����ϵͳ
		//���ݳ���ʵ�������䡷���������
		data[ 52 ] =0x01;
		data[ 53 ] =0x00;
		//���ݵ�Ԫ����
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
   		temp=len-22;    //22������Gprs_ProtocolDataEncode�����ݳ����ǲ�����2���ֽڱ�ʶ���ģ�
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
  
  //32960��BCCУ��
  data[ length++ ] = BCC_CheckSum ( ( data + 2 ), len );
  return length;  
}







/* 
 * ������������¼ָ��
 * ���ò�������
 *     
 * ����ֵ  ����
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
		//��¼��ˮ��
		login_serial =vehicles_login_serial_number();
	
    //���Ԫ
    msg[ 0 ] = 0x01; //����ID   
		msg[ 1 ] = 0xFE; 

		//VIN   ��ȡ��ʽ��Ŀǰ��ͨ������д�룬��CAN��ȡ����	
		memcpy ( ( u8* )&msg[ 2 ], vehicleParams.CarVIN, 17 );
  
    //���ܷ�ʽ
		msg[ 19 ] =0x01;

		//���ݵ�Ԫ����
		msg[ 20 ] =0x00;
		msg[ 21 ] =0x00;


		if(GPS_WorkState!=1)//gps��Чʱ��ʹ��RTCʱ��
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

	




	//��ˮ��¼
  memcpy ( ( u8* )&msg[ 28 ], ( u8* )&login_serial, 2 );
	
	//SMI����ICCID
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
	
  

    /*���������BCCУ��*/
    totalLength = Gprs_ProtocolDataEncode ( msgLogin, 50);//ָ��Ԫ�صĸ���
#if DEBUG_32960==1	
		printf ( "\r\nmsgLogin \r\n");

		for(i=0;i<totalLength;i++)
		{
			 printf ( "%02x", *(msgLogin+i) );
		
		}
		printf ( "\r\n");
#endif
    /* ��½���ݷ��� */
    TCP_DataOutput( msgLogin, totalLength );
		#if SD_WRITE_ENABLE==1
		char LogineData[60];
		u8 SD_Login_HexToAscii[2*totalLength+10];
		sprintf(LogineData,"\r\n\r\nLoginData SendTime  20%02u%02u%02u %02u:%02u:%02u\r\n",RTC_time.year, RTC_time.month, RTC_time.date,RMC_d.time_hh,RMC_d.time_mm,RMC_d.time_ss);
		SD_WriteFile(&file,SD_filename,(u8*)LogineData,strlen(LogineData));
		
		HexToAscii(SD_Login_HexToAscii,msgLogin,totalLength,true);
		SD_WriteFile(&file,SD_filename,SD_Login_HexToAscii,2*totalLength+2);
		#endif
		
	

		/* �ͷ��ѷ���Ļ��� */
		if ( SHABUF_FreeBuffer( msgLogin ) == TRUE )
		{
			msgLogin = NULL;
		}

   }
}





/*
*******************************************************************
*��������vehicles_logout
*��  ������
*��  �ܣ������ǳ�
*������: gl
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
    //���Ԫ
    msg[ 0 ] = 0x04; //����ID   
		msg[ 1 ] = 0xFE; 

		//VIN
    memcpy ( ( u8* )&msg[ 2 ], vehicleParams.CarVIN, 17 );

  
    //���ܷ�ʽ
		msg[ 19 ] =0x01;

		//���ݵ�Ԫ����
		msg[ 20 ] =0x00;
		msg[ 21 ] =0x08;

	  if(GPS_WorkState!=1)//gps��Чʱ��ʹ��RTCʱ��
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

	//��ˮ��¼
    memcpy ( ( u8* )&msg[ 28 ], ( u8* )&login_serial, 2 );


    /*���������BCCУ��*/
    totalLength = Gprs_ProtocolDataEncode ( msgLogout, 30);//ָ��Ԫ�صĸ���
#if DEBUG_32960==1	
	printf ("\r\nmsgLogout \r\n");
	for(i=0;i<totalLength;i++)
	{
		 printf ("0x%02x", *(msgLogout+i) );
	
	}
	printf ( "\r\n");
#endif
    /* ��½���ݷ��� */
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
	
	/*��ȡ������һ�ε�¼ʱ��*/
	/*��ȡ������¼��ˮ*/
	STORAGE_ReadData( VEHICLE_PARAMS, sizeof( vechile_Params_t ), ( u8* )&vehicleParams );
	STORAGE_ReadData ( GPRS_PARAMS,   sizeof( gprs_Params_t ),   ( u8 * )&gprsParams );

	printf ( "\r\n^^^^^^^^^GPRS TIME:  20%u/%u/%u\r\n",RMC_d.date_yy, RMC_d.date_mm, RMC_d.date_dd);

	printf ( "\r\n^^^^^^^^^FLASH TIME: 20%u/%u/%u\r\n",vehicleParams.GPS_d.date_yy, vehicleParams.GPS_d.date_mm, vehicleParams.GPS_d.date_dd);

	if(vehicleParams.GPS_d.date_yy==RMC_d.date_yy&&vehicleParams.GPS_d.date_mm==RMC_d.date_mm&&vehicleParams.GPS_d.date_dd==RMC_d.date_dd)
	{
			//����һ�£���ˮ��++
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

		//���ڲ�һ�£���ˮ������ ,���µ���ʱ��
		vehicleParams.LoginSerial=1;
		vehicleParams.GPS_d.date_yy=RMC_d.date_yy;
		vehicleParams.GPS_d.date_mm=RMC_d.date_mm;
		vehicleParams.GPS_d.date_dd=RMC_d.date_dd;
	}
	/*���³���������ˮ��*/
	/*���³�����¼ʱ��*/
	
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
*��������data_pack_head
*��  ��������ID
*��  �ܣ����ݰ��İ�ͷ
*������: gl
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

    //���Ԫ
    msg[ 0 ] = order; //ʵʱ����ID  
		msg[ 1 ] = 0xFE; 

		//VIN
    memcpy ( ( u8* )&msg[ 2 ], vehicleParams.CarVIN, 17 );
    
    //���ܷ�ʽ
		msg[ 19 ] =0x01;

		//���ݵ�Ԫ����
		msg[ 20 ] =0x00;
		msg[ 21 ] =0x00;
  }

  return 21;
}




/*
*******************************************************************
*��������Vehicle_data_upload
*��  ������
*��  �ܣ����������ϴ�
*������: gl
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
		 
		 //������������ID
		 msg[++lenth]=0x01;
		 //����״̬
		 msg[++lenth]=VehicleData_Upload->vehicle_state;
		 //���״̬
			msg[++lenth]=VehicleData_Upload->charge_state;
		 //����ģʽ
		 msg[++lenth]=VehicleData_Upload->run_mode;
		 //����	
		 msg[++lenth]=(VehicleData_Upload->vehicle_speed>>8)&0xff;	
		 msg[++lenth]=VehicleData_Upload->vehicle_speed&0xff;
		 //�ۼ����
		 msg[++lenth]=(VehicleData_Upload->accumulated_distance>>24)&0xff;
		 msg[++lenth]=(VehicleData_Upload->accumulated_distance>>16)&0xff;
		 msg[++lenth]=(VehicleData_Upload->accumulated_distance>>8)&0xff;
		 msg[++lenth]=VehicleData_Upload->accumulated_distance&0xff;
		 //�ܵ�ѹ
		 msg[++lenth]=(VehicleData_Upload->total_voltage>>8)&0xff;
		 msg[++lenth]=VehicleData_Upload->total_voltage&0xff;
		 //�ܵ���
		 msg[++lenth]=(VehicleData_Upload->total_current>>8)&0xff;
		 msg[++lenth]=VehicleData_Upload->total_current&0xff;
		 //SOC
		 msg[++lenth]=VehicleData_Upload->soc;
		 //DC-DC״̬
		 msg[++lenth]=VehicleData_Upload->dc_dc_state;
		 //��λ
		 msg[++lenth]=VehicleData_Upload->gear;
		 //��Ե����
		 msg[++lenth]=(VehicleData_Upload->ir>>8)&0xff;
		 msg[++lenth]=VehicleData_Upload->ir&0xff;
		 //����̤���г�ֵ
		 msg[++lenth]=VehicleData_Upload->gas_value;
		 //�ƶ�̤��״̬
		 msg[++lenth]=VehicleData_Upload->braking_value;
		 
	}
	VehicleData_Upload=NULL;
	return lenth;
}


/*
*******************************************************************
*��������Drive_motor_data_upload
*��  ������
*��  �ܣ�������������ϴ�
*������: gl
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

		 //���������������ID
		 msg[++lenth]=0x02;

		 msg[++lenth]=MotorData_Upload->motor_num;	//�����������
		 templenth=lenth;

		 switch(msg[templenth])
		 {
		 case 1: //1�������������
		 	 	msg[++lenth]=MotorData_Upload->motor_SN;//�������˳���
				//�������״̬
				msg[++lenth]=MotorData_Upload->motor_state;
				//��������������¶�
				msg[++lenth]=MotorData_Upload->motor_ctrl_temperature;
				//�������ת��
				msg[++lenth]=(MotorData_Upload->motor_speed>>8)&0xff;
				msg[++lenth]=MotorData_Upload->motor_speed&0xff;
				//�������ת��
				msg[++lenth]=(MotorData_Upload->motor_torque>>8)&0xff;
				msg[++lenth]=MotorData_Upload->motor_torque&0xff;
				//��������¶�
				msg[++lenth]=MotorData_Upload->motor_temperature&0xff;
				//��������������ѹ
				msg[++lenth]=(MotorData_Upload->motor_ctrl_voltage>>8)&0xff;	
				msg[++lenth]=MotorData_Upload->motor_ctrl_voltage&0xff;
				//���������ֱ��ĸ�ߵ���
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
*��������Fuel_cell_data_upload
*��  ������
*��  �ܣ�ȼ�ϵ�������ϴ�
*������: gl
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

		 //ȼ�ϵ����������ID
		 msg[++lenth]=0x03;
		 msg[++lenth]=0x01;
		 msg[++lenth]=0x02;
		 msg[++lenth]=0x03;
		 msg[++lenth]=0x04;
		 msg[++lenth]=0x05;
		 msg[++lenth]=0x06;

		 //ȼ�ϵ��̽������
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
*��������Engine_data_upload
*��  ������
*��  �ܣ������������ϴ�
*������: gl
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

		 msg[++length]=0x04; 	//����������ID
		 /*������״̬*/
		 msg[++length]=engine_sys_data_upload->engine_state;
		 /*����������ת��*/
		 msg[++length]=(engine_sys_data_upload->crankshaft_speed>>8)&0xff;
		 msg[++length]=engine_sys_data_upload->crankshaft_speed&0xff;
		 /*������ȼ��������*/
		 msg[++length]=(engine_sys_data_upload->fuel_consumption>>8)&0xff;
		 msg[++length]=engine_sys_data_upload->fuel_consumption&0xff;
	}
	return length;

}


/*
*******************************************************************
*��������Vehicle_position_data_upload
*��  ������
*��  �ܣ�����λ�������ϴ�
*������: gl
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
		 //����λ����������ID
		 msg[++lenth]=0x05;

		 #if 0
		 printf("\r\n������Ϣ��\r\n");
		 for(i=0;i<4;i++)
		 {
		 	printf("0x%02x\t",locations.lon[i]);
		 }
		 printf("\r\nγ����Ϣ��\r\n");
		 for(i=0;i<4;i++)
		 {
		 	printf("0x%02x\t",locations.lat[i]);
		 }
		 printf("\r\n");
		 #endif
		 
#if  1 	 
		 if ( RMC_d.status_ == 'A'&&RMC_d.long_!=0&&RMC_d.lat_!=0)
					msg[++lenth]=0x00;//����GPS�Ƿ���Ч����������γ����
		 else
					msg[++lenth]=0x01;//����GPS�Ƿ���Ч����������γ����

				 
		 //������Ϣ   udpPositon.lon[0](���ֽ�)-->udpPositon.lon[3]�����ֽڣ� 
		 msg[++lenth]=locations.lon[0];
		 msg[++lenth]=locations.lon[1];
		 msg[++lenth]=locations.lon[2];
		 msg[++lenth]=locations.lon[3];
		 
		 //γ����Ϣ   udpPositon.lat[0](���ֽ�)-->udpPositon.lat[3]�����ֽڣ�
		 msg[++lenth]=locations.lat[0];
		 msg[++lenth]=locations.lat[1];
		 msg[++lenth]=locations.lat[2];
		 msg[++lenth]=locations.lat[3];
		 
#else
		 		 //������Ϣ   udpPositon.lon[0](���ֽ�)-->udpPositon.lon[3]�����ֽڣ� 
		 msg[++lenth]=0x07;
		 msg[++lenth]=0x15;
		 msg[++lenth]=0x3a;
		 msg[++lenth]=0xbf;

		 //γ����Ϣ   udpPositon.lat[0](���ֽ�)-->udpPositon.lat[3]�����ֽڣ�
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
*��������Extreme_value_data_upload
*��  ������
*��  �ܣ���ֵ�����ϴ�
*������: gl
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


		 //��ֵ��������ID
		 msg[++lenth]=0x06;

		 //��ߵ�ѹ�����ϵͳ��
		 msg[++lenth]=ExtremeValue_Data_Upload->highestVoltage_Subsys_num;
		 //��ߵ�ѹ��ص������
		 msg[++lenth]=ExtremeValue_Data_Upload->highestVoltage_Single_code;
		 //��ص����ѹ���ֵ
		 msg[++lenth]=(ExtremeValue_Data_Upload->SingleBattery_HighestVoltage>>8)&0xff;
		 msg[++lenth]=ExtremeValue_Data_Upload->SingleBattery_HighestVoltage&0xff;

		 //��͵�ѹ�����ϵͳ��
		 msg[++lenth]=ExtremeValue_Data_Upload->lowestVoltage_Subsys_num;
		 //��͵�ѹ��ص������
		 msg[++lenth]=ExtremeValue_Data_Upload->lowestVoltage_Single_code;
		 //��ص����ѹ���ֵ  
		 msg[++lenth]=(ExtremeValue_Data_Upload->SingleBattery_LowestVoltage>>8)&0xff;
		 msg[++lenth]=ExtremeValue_Data_Upload->SingleBattery_LowestVoltage&0xff;

		 //����¶���ϵͳ��
		 msg[++lenth]=ExtremeValue_Data_Upload->highestTemperature_Subsys_num;
		 //����¶�̽���
		 msg[++lenth]=ExtremeValue_Data_Upload->highestTemperature_Probe_code;
		 //����¶�ֵ
		 msg[++lenth]=ExtremeValue_Data_Upload->Probe_HighestTemperature&0xff;

		 //����¶���ϵͳ��
		 msg[++lenth]=ExtremeValue_Data_Upload->lowestTemperature_Subsys_num;
		 //����¶�̽�����
		 msg[++lenth]=ExtremeValue_Data_Upload->lowestTemperature_Probe_code;
		 //����¶�ֵ
		 msg[++lenth]=ExtremeValue_Data_Upload->Probe_LowestTemperature&0xff;

		 
	}
	ExtremeValue_Data_Upload=NULL;
	return lenth;

}

/*
*******************************************************************
*��������Set_Comm_Warning
*��  ������
*��  �ܣ����û����������־�������ȼ�
*������: gl
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
*��������Extreme_value_data_upload
*��  ������
*��  �ܣ���ֵ�����ϴ�
*������: gl
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
			  /*�ֶ�����3������*/
				Set_Comm_Warning(14,3);
		}
		
		msg[++length]=0x07;	//��������
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
*��������User_defined_data_upload
*��  ������
*��  �ܣ��ɳ�索��װ�õ�ѹ����
*������: gl
* 
*******************************************************************
*/

u16 User_defined_data_upload(u16 lenth,u8 order)
{
	if ( msgRealData_upload)
  	{
         u8 *msg = ( u8* )&msgRealData_upload[ 2 ];
		 //�Զ�����������ID
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
*��������Voltage_data_for_rechargeable_devices
*��  ������
*��  �ܣ��ɳ�索��װ�õ�ѹ����
*������: gl
* 
*******************************************************************
*/
u16 Voltage_data_for_rechargeable_devices(u16 length)
{
		u8 *msg=NULL;
		u16 battery_num,charge_sys_num;

		stored_energy_sys_data_t* stored_energy_sys_data_upload=&Stored_Energy_sys_Data;
		//�޳����ϵͳ�ţ��򲻴���ѹ�б�
		if (( msgRealData_upload||msgDeadData_Strorage)&&(stored_energy_sys_data_upload->stored_energy_sys_sn>0))
		{

			if(msgRealData_upload)
				msg = ( u8* )&msgRealData_upload[ 2 ];
			else
			if(msgDeadData_Strorage)
			  msg = ( u8* )&msgDeadData_Strorage[ 2 ];

			msg[++length]=0x08;	//��ѹ�б�����ID
			/*�ɳ��װ����ϵͳ���� */
			msg[++length]=stored_energy_sys_data_upload->stored_energy_sys_num;
			charge_sys_num=length;
			switch(msg[charge_sys_num])
			{
				case 1:
					/*�ɳ��װ����ϵͳ��*/
					msg[++length]=stored_energy_sys_data_upload->stored_energy_sys_sn;
					/*�ɳ��װ�õ�ѹ*/
					msg[++length]=(stored_energy_sys_data_upload->stored_energy_voltage>>8)&0xff;
					msg[++length]=stored_energy_sys_data_upload->stored_energy_voltage&0xff;
					/*�ɳ��װ�õ���*/
					msg[++length]=(stored_energy_sys_data_upload->stored_energy_current>>8)&0xff;
					msg[++length]=stored_energy_sys_data_upload->stored_energy_current&0xff;
					/*����������*/
					msg[++length]=(stored_energy_sys_data_upload->single_battery_total_num>>8)&0xff;
					msg[++length]=stored_energy_sys_data_upload->single_battery_total_num&0xff;
					
				
					//��֡��ʼ�����źͱ�֡������������Ҫ����ʵ������޸�
					/********************************************************************************************/
					/*��֡��ʼ������;(ps:һ���ǲ��ɱ��)*/
					msg[++length]=(stored_energy_sys_data_upload->this_frame_InitialBattery_code>>8)&0xff;
					msg[++length]=stored_energy_sys_data_upload->this_frame_InitialBattery_code&0xff;
					/*��֡��������*///һ���뵥��������ͬ
					msg[++length]=stored_energy_sys_data_upload->this_frame_battery_num;
					/********************************************************************************************/
					/*�����ص�ѹ�б�*/
					battery_num=length;
					for(u8 i=0;i<msg[battery_num];i++)
					{
						/*�����ص�ѹ���ֽ�*/
						/*�����ص�ѹ���ֽ�*/
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
*��������Temperature_data_for_rechargeable_devices
*��  ������
*��  �ܣ��ɳ�索��װ���¶�����
*������: gl
* 
*******************************************************************
*/
u16 Temperature_data_for_rechargeable_devices(u16 length)
{
	
		u8 *msg=NULL;
		u16 temp_length;

		stored_energy_sys_data_t* stored_energy_sys_data_upload=&Stored_Energy_sys_Data;
		//�޳����ϵͳ�ţ��򲻴��¶��б�
		if(( msgRealData_upload||msgDeadData_Strorage )&&(stored_energy_sys_data_upload->stored_energy_sys_sn>0))
		{
			 if(msgRealData_upload)
				  msg = ( u8* )&msgRealData_upload[ 2 ];
			 else
			 if(msgDeadData_Strorage)
					msg = ( u8* )&msgDeadData_Strorage[ 2 ];
		 
			msg[++length]=0x09; 	//�¶��б�����ID
			/*�ɳ�索����ϵͳ�������¶ȣ�*/
			msg[++length]=stored_energy_sys_data_upload->stored_energy_sys_num;
			temp_length=length;
			
			switch(msg[temp_length])
			{
				case 1:

					/*�ɳ�索����ϵͳ�ţ��¶ȣ�*/
					msg[++length]=stored_energy_sys_data_upload->stored_energy_sys_sn;
					/*�¶�̽�����*/
					msg[++length]=(stored_energy_sys_data_upload->temperature_probe_num>>8)&0xff;
					msg[++length]=stored_energy_sys_data_upload->temperature_probe_num&0xff;
					temp_length=length;
					/*�¶�̽���б�*/
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
*��������Multi_realdata_upload
*��  ������
*��  �ܣ����ʵʱ�����ϴ�
*������: gl
* 
*******************************************************************
*/

u16 Multi_realdata_upload(u16 Counter,u8 UploadTime,bool enable)
{	

  OS_CPU_SR   cpu_sr;
	  
	u16 pack_head_length=data_pack_head(0x02);
  //��ȡ������ʼ��ַ����
	u16 templenth=pack_head_length;
	if ( msgRealData_upload )
  {
     u16 totalLength = 0;
     u8 *msg = ( u8* )&msgRealData_upload[ 2 ];

//		if(GPS_WorkState==0)//gps��Чʱ��ʹ��RTCʱ��
		{
				//���ݲɼ�ʱ��	//ID:22
				msg[ ++templenth ] = RTC_time.year;
				msg[ ++templenth ] = RTC_time.month;
				msg[ ++templenth ] = RTC_time.date;
				msg[ ++templenth ] = RTC_time.hours;
				msg[ ++templenth ] = RTC_time.minutes;
				msg[ ++templenth ] = RTC_time.seconds;

		}
//		else
//		{

//				//���ݲɼ�ʱ��	//ID:22
//				msg[ ++templenth ] = RMC_d.date_yy;
//				msg[ ++templenth ] = RMC_d.date_mm;
//				msg[ ++templenth ] = RMC_d.date_dd;	
//				msg[ ++templenth ] = RMC_d.time_hh;
//				msg[ ++templenth ] = RMC_d.time_mm;
//				msg[ ++templenth ] = RMC_d.time_ss;

//		}

		
#if TIME_BIAS_ENABLE ==1		
		if(upload_switch==1)//3����������ʱ��ʱ���
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
		 /*��������*/
		 templenth=Vehicle_data_upload(templenth);
		 
		 /*�����������*/
		 templenth=Drive_motor_data_upload(templenth);
		 
		 ///*����������*/
		 //templenth=Engine_data_upload(templenth);
		 //templenth=Engine_Sys_Temp(templenth);
		 
		 /*����λ��*/
		 templenth=Vehicle_position_data_upload(templenth);
		 
		 /*��ֵ����*/
		 templenth=Extreme_value_data_upload(templenth);
		 
		 /*��������*/
		 templenth=Warning_Data_Upload(templenth);
		 
		 /*�û��Զ�������*/
		 //templenth=User_defined_data_upload(templenth,0x80);
		 
		 /*�ɳ��װ�õ�ѹ����*/
		 templenth=Voltage_data_for_rechargeable_devices(templenth);
		 
		 /*�ɳ��װ���¶�����*/
		 templenth=Temperature_data_for_rechargeable_devices(templenth);
		
		 /*���������BCCУ��*/
	   totalLength = Gprs_ProtocolDataEncode ( msgRealData_upload, templenth+1);//ָ��Ԫ�صĸ���
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
					/*��¼�ɹ��󣬽���FIFO  ������30�����ݣ����ݳ��ȣ�totalLength */
					FIFO_Init(pRingBuf,FIFO_Buffer,totalLength,30);	
					FIFO_Init_flag=1;
		 }
#endif

#if FIFO_RING_SW==1

		/*FIFO��������ÿ���һ������*/
		if(upload_switch==0&&FIFO_switch!=1&&Alarm_AtDeadtime!=1)  //�򿪻��λ���
		{
			
				if(FIFO_IsFull(pRingBuf))
				{
					FIFO_GetOne(pRingBuf, FIFO_temp);
				}
				//printf("\r\n FIFI Storage Stamp :%d :%d :%d \r\n",msgRealData_upload[27],msgRealData_upload[28],msgRealData_upload[29]);
				FIFO_AddOne(pRingBuf,(u8*)msgRealData_upload);
		}
		
#if DEBUG_32960==1 
		printf ( "\r\nFIFO_IsEmpty__FIFO��������������%d\r\n",pRingBuf->Counter); 
		printf ( "\r\nFIFO_IsEmpty__FIFO        ��ͷ��%d\r\n",pRingBuf->Head);
	  printf ( "\r\nFIFO_IsEmpty__FIFO        ��β��%d\r\n",pRingBuf->Tail);

		printf ( "\r\n");
		for(i=0;i<totalLength;i++)
		{
			printf("%02x\t",msgRealData_upload[i]);		
		}
		printf ( "\r\n");
#endif
		
		
#endif
	   /* ʵʱ���ݷ��� */
		 if(Counter>=UploadTime)
		 {
				upload_timecount=0;	//�����ϴ�����������
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
*��������DeadData_Strorage
*��  ������
*��  �ܣ�ä�����ݴ洢
*������: gl
* 
*******************************************************************
*/

u16 DeadData_Strorage(u16 Counter,u16 StorageCycleTime,u8 FIFI_Stop_Enable)
{	
  
	u16 pack_head_length=data_pack_head(0x03);
  //��ȡ������ʼ��ַ����
	u16 templenth=pack_head_length;
	if (msgDeadData_Strorage )
  {
			 u16 totalLength = 0;
			 u8 *msg = ( u8* )&msgDeadData_Strorage[ 2 ];

			 //ʹ��RTCʱ��
			 {
					 //���ݲɼ�ʱ��	//ID:22
					 msg[ ++templenth ] = RTC_time.year;
					 msg[ ++templenth ] = RTC_time.month;
					 msg[ ++templenth ] = RTC_time.date;	
					 msg[ ++templenth ] = RTC_time.hours;
					 msg[ ++templenth ] = RTC_time.minutes;
					 msg[ ++templenth ] = RTC_time.seconds;
			 }
		
			 /*��������*/
			 templenth=Vehicle_data_upload(templenth);
			 
			 /*�����������*/
			 templenth=Drive_motor_data_upload(templenth);
			 
			 ///*����������*/
			 //templenth=Engine_data_upload(templenth);
			 //templenth=Engine_Sys_Temp(templenth);
			 
			 /*����λ��*/
			 templenth=Vehicle_position_data_upload(templenth);
			 
			 /*��ֵ����*/
			 templenth=Extreme_value_data_upload(templenth);
			 
			 /*��������*/
			 templenth=Warning_Data_Upload(templenth);
			 
			 /*�û��Զ�������*/
			 //templenth=User_defined_data_upload(templenth,0x80);
			 
			 /*�ɳ��װ�õ�ѹ����*/
			 templenth=Voltage_data_for_rechargeable_devices(templenth);
			 
			 /*�ɳ��װ���¶�����*/
			 templenth=Temperature_data_for_rechargeable_devices(templenth);
			
			 /*���������BCCУ��*/
			 totalLength = Gprs_ProtocolDataEncode ( msgDeadData_Strorage, templenth+1);//ָ��Ԫ�صĸ���

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
					 /*�������ݱ�־*/
					 if(pFlashBuf->Counter!=0)
					 {
							 pFlashBuf->offsetdata=0x8080;
					 }
					 //printf("\r\n[STORAGE EVENT]:DeadData_Strorage\r\n");		
			 }
			 /*FIFO��������ÿ���һ������*/	
			 if(!FIFI_Stop_Enable)//����״̬�£���������������ֹͣ�洢
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
*��������Offset_data_upload
*��  ������
*��  �ܣ� 1.������������ϴ���32960�� ----ä������
*				  2.ʱ�䲹������
*������: gl
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

	//�Բ�������ʱ���У׼
	ringbuf[28]=OffsetData_time_hh;
	ringbuf[28]=OffsetData_time_mm;
	ringbuf[29]=OffsetData_time_ss;	
	
	printf("      TimeStamp: %02u:%02u:%02u  \r\n",ringbuf[27],ringbuf[28],ringbuf[29]);

	ringbuf[DeadData_lenth-1]=BCC_CheckSum((ringbuf+2),(DeadData_lenth-3));


	return TCP_DataOutput (ringbuf, DeadData_lenth );

}

/*
*******************************************************************
*��������Offset_data_upload
*��  ������
*��  �ܣ�������������ϴ���32960�� ----ä������
*������: gl
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
*��������Offset_data_upload_tcp
*��  ������
*��  �ܣ�
*					1.������������ϴ���32960�� ----������3����������
*         2.ʱ�䲹������
*������: gl
* 
*******************************************************************
*/
u16 Offset_data_upload_tcp(u8 *ringbuf)
{								  
	int i;	
	ringbuf[2]=0x03;	

#if TIME_BIAS_ENABLE ==  1

#if SEND_GRADEDOWN	== 1      //����������		
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
			
#else	//����������
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
*��������Offsetdata_upload_DeadTime
*��  ������
*��  �ܣ�
*					1.������������ϴ���32960�� ----������3����������
*         2.ʱ�䲹������
*������: gl
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
*��������User_defined_data_upload
*��  ������
*��  �ܣ��ն�Уʱ
*������: gl
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

	//���Ԫ
    msg[ 0 ] = 0x08; //����ID   
		msg[ 1 ] = 0xFE; 

	//VIN
    memcpy ( ( u8* )&msg[ 2 ], vehicleParams.CarVIN, 17 );

    //���ܷ�ʽ
	msg[ 19 ] =0x01;

	//���ݵ�Ԫ����
	msg[ 20 ] =0x00;
	msg[ 21 ] =0x00;


  
    /*���ݸ�ʽ����У��*/
    totalLength = Gprs_ProtocolDataEncode(msgCorrectionTime, 22 );
#if DEBUG_32960==1
	printf ( "\r\n********_______msgCorrectionTime!!!\r\n");
#endif
    /*�������ݷ���*/
    TCP_DataOutput ( msgCorrectionTime, totalLength );

    /* �ͷ��ѷ���Ļ��� */
    if ( SHABUF_FreeBuffer( msgCorrectionTime ) == TRUE )
    {
      msgCorrectionTime = NULL;
    }
  }
}




/* 
 * �����������ն��ϴ�����
 * ���ò�������
 *     
 * ����ֵ  ����
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

		//���Ԫ
    msg[ 0 ] = 0x07; //����ID   
		msg[ 1 ] = 0xFE; 

		//VIN
    memcpy ( ( u8* )&msg[ 2 ], vehicleParams.CarVIN, 17 );

    //���ܷ�ʽ
		msg[ 19 ] =0x01;

		//���ݵ�Ԫ����
		msg[ 20 ] =0x00;
		msg[ 21 ] =0x00;

  
    /* ���ݸ�ʽ����У�� */
    totalLength = Gprs_ProtocolDataEncode ( msgHeartBeat, 22 );

#if DEBUG_32960==1
	printf ( "\r\n********_______send_heartbeat!!!\r\n");
#endif
    /* �������ݷ��� */ 
    TCP_DataOutput ( msgHeartBeat, totalLength );
		
		#if SD_WRITE_ENABLE==1
		if(upload_switch==0) //������������SDд������ ��3������ʱ����д������
		{
				char HeartData[60];
				sprintf(HeartData,"\r\n\r\nHeartData SendTime  20%02u%02u%02u %02u:%02u:%02u\r\n",RTC_time.year, RTC_time.month, RTC_time.date,RMC_d.time_hh,RMC_d.time_mm,RMC_d.time_ss);
				SD_WriteFile(&file,SD_filename,(u8*)HeartData,strlen(HeartData));
			
				HexToAscii(SD_Heartbeet_HexToAscii,msgHeartBeat,totalLength,true);
				SD_WriteFile(&file,SD_filename,SD_Heartbeet_HexToAscii,2*totalLength+2);
		}
    #endif
		
		/* �ͷ��ѷ���Ļ��� */
    if ( SHABUF_FreeBuffer( msgHeartBeat ) == TRUE )
    {
				msgHeartBeat = NULL;
    }
		
  }
}

/*
*******************************************************************
*��������Gprs_Vehicle_terminal_control_response
*��  ������
*��  �ܣ��ն˿��������Ӧ��
*��  ������ƽ̨�·��������жϣ���Ӧ��
*������: gl
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
    /*���������BCCУ��*/
		pResponse[len-1]=BCC_CheckSum(&pResponse[2],len-3);

    /* ��½���ݷ��� */
    TCP_DataOutput ( pResponse, len );
		printf ( "\r\n");
		for(i=0;i<len;i++)
   	{
   	 	printf ( "%02x ",pResponse[i]);
   	}
		printf ( "\r\n");
  	/* �ͷ��ѷ���Ļ��� */
  	if(SHABUF_FreeBuffer( pResponse ) == TRUE )
  	{
  		pResponse = NULL;
		
  	}
  }
  return  result;
}

/*
*******************************************************************
*��������Gprs_VehicleTerminalControl
*��  ������
*��  �ܣ��ն˿�������
*������: gl
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
			printf("\r\n************����ID:%d��***********\r\n" ,msg[0]);
			u16 UpGrade_datalen=(len-2);
		  ftp_parseDataHdlr(UpGrade_datalen,&msg[1]);
			break;
	    case 0x02:
			printf("\r\n************����ID:%d��***********\r\n" ,msg[0]);

			break;
		case 0x03:
			printf("\r\n************����ID:%d��***********\r\n" ,msg[0]);
			System_Reset();
			break;
		case 0x04:
			printf("\r\n************����ID:%d��***********\r\n" ,msg[0]);
			break;
		case 0x05:
			printf("\r\n************����ID:%d��***********\r\n" ,msg[0]);
			XMIT_Stop();
			break;
		case 0x06:
			printf("\r\n************����ID:%d��***********\r\n" ,msg[0]);
			break;
		case 0x07:
			printf("\r\n************����ID:%d��***********\r\n" ,msg[0]);
			break;
		default:
			printf("\r\n************����ID�쳣***********\r\n");
			break;
	}

}

/*
*******************************************************************
*��������Gprs_Parameters_Query
*��  ������
*��  �ܣ�������ѯ
*������: gl
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
		/* ��ȡƽ̨���ݵ�ǰ31���ֽڣ�����������������ֹ */
		memcpy(msgParametersQuery,data,31);
		//�����־��ʽ��
		msg=&msgParametersQuery[2];
    msg[1]=0x01;	

		
		/*  ��ȡƽ̨�����ѯ�Ĳ�������  */
    u16 query_cmdtotal=msgParametersQuery[30];
		
    //ָ���±�ƫ�����������������һ����ַ��
		length=28;
/*
*===================================================================================	
*	��β�ѯ�߼�
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

		/* ���ݸ�ʽ����У�� */
    u16 totalLength = Gprs_ProtocolDataEncode( msgParametersQuery,length+1);

		for(u16 i=0;i<totalLength;i++)
	  {
	   	 	printf ( "%02x ",msgParametersQuery[i]);
	  }
	  printf ( "\r\n");
		
		TCP_DataOutput(msgParametersQuery, totalLength );
		
		/* �ͷ��ѷ���Ļ��� */
    if( SHABUF_FreeBuffer( msgParametersQuery) == TRUE )
    {
				msgParametersQuery = NULL;
    }
	
	}
}

/*
*******************************************************************
*��������Gprs_ParametersQuery_Response
*��  ������
*��  �ܣ�������ѯӦ��
*������: gl
* 
*******************************************************************
*/

void Gprs_ParametersQuery_Response(u16 len,u8* data)
{

	
	
}

/*
*******************************************************************
*��������Gprs_Parameters_Set
*��  ������
*��  �ܣ���������
*������: gl
* 
*******************************************************************
*/

void Gprs_ParametersSet(u16 len,u8* data)
{
	
	
		u8 *msg=NULL;
		u16 dataUnitLen=0;
		dataUnitLen=(data[22]<<8)|data[23];	
		//��ȡ���������������
		u16 setting_cmd_total=data[30];
		//ָ��ָ���һ������ID
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
						//ƽ̨�·���������������������IP
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
						//ƽ̨�·���������������������IP
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
*��������Gprs_ParametersSet_Response
*��  ������
*��  �ܣ���������Ӧ��
*������: gl
* 
*******************************************************************
*/

void Gprs_ParametersSet_Response(u16 len,u8* data)
{









}
