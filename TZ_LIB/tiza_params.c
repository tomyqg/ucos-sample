#include <stdio.h>
#include <string.h>
#include "tiza_params.h"
#include "tiza_gprs_protocol.h"
#include "tiza_storage.h"
#include "tiza_gprs_protocol.h"
#include "w25qxx_port.h"





/* �������� */
vechile_Params_t vehicleParams;

/* GPRS���� */
gprs_Params_t  gprsParams;

/* λ�ò���  */
location_t locations;

/* GPS���� */
extern GPS_data RMC_d;

#if TZ_DEFAULT_CONTROLLER_TYPE > TZ_CONTROLLER_TYPE_NONE
/* ���������� */
extern controller_data_t controllerBackupData;
#endif  /* !TZ_CONTROLLER_TYPE_NONE */ 














/******************************************/
/*              ϵͳ����[����]            */
/******************************************/


//stored_energy_sys_data_t 	Stored_Energy_sys_Data;

void ParameterQuery_Init(void)
{
	Parameter_Query_Data.device_storeage_cycletime=1000;//ms
	Parameter_Query_Data.dataupload_cycletime=30;//s
	Parameter_Query_Data.warning_upload_cycletime=1;//s
	strcpy((char*)Parameter_Query_Data.manage_domain_name,"www.baidu.com");
	Parameter_Query_Data.manage_domain_name_len=strlen((char*)Parameter_Query_Data.manage_domain_name);
	Parameter_Query_Data.manage_port=1234;
	strcpy((char*)Parameter_Query_Data.hard_version,"T3K12");
	strcpy((char*)Parameter_Query_Data.soft_version,"Z803A");
	Parameter_Query_Data.heartbeat_cycletime=30;
	Parameter_Query_Data.device_ack_timeout=120;
	Parameter_Query_Data.platform_ack_timeout=300;
	Parameter_Query_Data.login_fail_next_loglin_time=30;//minute
	
	strcpy((char*)Parameter_Query_Data.public_domain_name,"www.sohu.com");
	Parameter_Query_Data.public_domain_name_len=strlen((char*)Parameter_Query_Data.public_domain_name);
	Parameter_Query_Data.public_port=4321;
	Parameter_Query_Data.sample_test=0xff;
}








/* 
 * ����������ϵͳSMS����
 * ���ò�������
 * 
 * ����ֵ  ����
 * 
 */
extern void sysResetVehicle ( void )
{
		vehicleParams.sleepMode = TRUE;
		sprintf ( ( char * )vehicleParams.CarVIN, "%s", TZ_DEFAULT_PEER_VIN );
		STORAGE_WriteData ( VEHICLE_PARAMS, sizeof( vechile_Params_t ), ( u8 * )&vehicleParams );
}










/* 
 * ����������ϵͳGPRS����
 * ���ò�������
 *     
 * ����ֵ  ����
 * 
 */
extern void sysResetGprs ( void )
{
  sprintf ( ( char * )gprsParams.APN, "%s", TZ_DEFAULT_GPRS_APN );
  sprintf ( ( char * )gprsParams.IP1, "%s", TZ_DEFAULT_PEER_ADDR1 );
  sprintf ( ( char * )gprsParams.IP2, "%s", TZ_DEFAULT_PEER_ADDR2 );
  sprintf ( ( char * )gprsParams.ip1_domain, "%s", TZ_DEFAULT_MS_DOMIAN );
  sprintf ( ( char * )gprsParams.ip2_domain, "%s", TZ_DEFAULT_SS_DOMIAN );

  gprsParams.ver = TZ_DEFAULT_PROTOCOL_VERSION; /* Э��汾�� */
  gprsParams.Port1 = TZ_DEFAULT_PEER_PORT;  /* �˿ں�1 */
  gprsParams.Port2 = TZ_DEFAULT_PEER_PORT;  /* �˿ں�2 */
  gprsParams.net_is_bad = 0;  /* ����״̬�������������λ��120s�� */
  gprsParams.net_is_ok = 0; /* ����״̬���ü���������λ��120s�� */
  gprsParams.DeviceIDSet = FALSE; /* �豸ID��δ���� */

  STORAGE_WriteData ( GPRS_PARAMS, sizeof( gprs_Params_t ), ( u8 * )&gprsParams );
}








/* 
 * ������������λӦ��״̬��Ϣ
 * ���ò�������
 *     			
 * ����ֵ  ����
 * 
 */
extern void sysResetState ( void )
{

}









/* 
 * ������������λACC��������������Ϣ
 * ���ò�������
 *     			
 * ����ֵ  ����
 * 
 */
extern void sysResetAccWorkPara ( void )
{

}







#if APPLI_PARAMTER_DEBUG == 1

/* 
 * ������������ʾACC��������������Ϣ
 * ���ò�������
 *     			
 * ����ֵ  ����
 * 
 */
extern void sysDisplayAccWorkPara ( void )
{
  PRINTF_GPS_TIME ();
  printf ( "[PARA EVENT] Acc info total-time %u; open-time %u; close-time %u; upload-time count %u; history-log count %u\r\n",
           accInfo.total_time, 
           accInfo.open_time,
           accInfo.close_time,
           accInfo.upload_time_count,
           accInfo.work_history_log.count );
}

#endif  /* APPLI_PARAMTER_DEBUG */








/* 
 * ����������
 * ���ò�������
 * 
 * ����ֵ  ����
 * 
 */
extern void sysResetAccWorkStatsInfo ( void )
{

}












/* 
 * ����������ϵͳSMS����
 * ���ò�������
 * 
 * ����ֵ  ����
 * 
 */
extern void sysResetDebug ( void )
{
  vehicleParams.DebugCtroller.debugIsValid = FALSE;
  vehicleParams.DebugCtroller.debugPort = 0;
  vehicleParams.DebugCtroller.debugType = 0;
  //STORAGE_WriteData ( VEHICLE_PARAMS, sizeof( vechile_Params_t ), ( u8 * )&vehicleParams );
}








/* 
 * ����������ϵͳGPRS����
 * ���ò�������
 *     			
 * ����ֵ  ����
 * 
 */
extern void sysResetControllerPara ( void )
{

} 


          







#if TZ_USE_STORAGE_MASS_DATA == 1

/* 
 * ����������ϵͳGPRS����
 * ���ò�������
 *     			
 * ����ֵ  ����
 * 
 */
extern void sysResetMassDataCompensator ( void )
{
  s32 i = 0;

  StoreDataObject.objectCount = 0;
  StoreDataObject.workParamCount = 0;
  StoreDataObject.UniversalCount = 0;
  for ( i = 0; i < STROAGE_DATA_PACKET_MAX_NUMBER; i++ )
  {
    StoreDataObject.workParamID[ i ] = 0;
    StoreDataObject.UniversalID[ i ] = 0;
  }

}
#endif  /* TZ_USE_STORAGE_MASS_DATA */










/* 
 * ������������ȡӦ�ò���
 * ���ò�������
 *          
 * ����ֵ  ����
 * 
 */
extern void sysParamsInit ( void )
{
	/* ����Ӧ������״̬ */
  sys_status = PROC_SYSTEM_INIT;
	DeadData_FlashConfigInit();
	memset ( ( u8 * )&locations, 0, sizeof( location_t ) );
	STORAGE_ReadData ( GPRS_PARAMS, sizeof( gprs_Params_t ), ( u8* )&gprsParams );	
	STORAGE_ReadData ( VEHICLE_PARAMS,  sizeof( vechile_Params_t ), ( u8 * )&vehicleParams ); 
	/* �ն����߿��ƿ���: Ĭ�ϴ� */
	vehicleParams.sleepMode=TRUE; 
	/*��ȡƬ��FLASH�о�γ����Ϣ*/
	STORAGE_FlashParams_ReadData(( u8 * )&gps_data_param ,sizeof( gps_data_param_t ),FLASH_GPS_PARAMS);
	
	
	u16 flhLen = 0;
  u16 flhLen2 = 0;
	/* �������� */
	flhLen = STORAGE_ObjectExist ( VEHICLE_PARAMS );
	

	if ( flhLen == sizeof ( vechile_Params_t )&& (vehicleParams.CarVIN[0]=='A'||vehicleParams.CarVIN[0]=='L') )
	{
				STORAGE_FlashParams_WriteData(( u8 * )&vehicleParams ,sizeof( vechile_Params_t ),FLASH_VEHICLE_PARAMS);
		    printf("\r\n vechile_Params_t : %s  \r\n",vehicleParams.CarVIN);
	}
	else
	{
		  vechile_Params_t vechile_ParamsTemp={0};
			STORAGE_FlashParams_ReadData(( u8 * )&vechile_ParamsTemp ,sizeof( vechile_Params_t ),FLASH_VEHICLE_PARAMS);
			//printf("\r\n vechile_ParamsTemp : %s  \r\n",vechile_ParamsTemp.CarVIN);
			if (vechile_ParamsTemp.CarVIN[0]=='A'||vechile_ParamsTemp.CarVIN[0]=='L')
			{
					memcpy((u8*)&vehicleParams,(u8*)&vechile_ParamsTemp,sizeof(vechile_Params_t));
					STORAGE_WriteData ( VEHICLE_PARAMS,  sizeof( vechile_Params_t ), ( u8 * )&vehicleParams ); 
					printf("\r\n STORAGE_FlashParams_ReadData ---> vehicleParams  \r\n");
			}
			else
			{
					sysParamsReset();
			}	
	}

	/* GPRSͨ�Ų��� */
	flhLen2 = STORAGE_ObjectExist(GPRS_PARAMS);
	if ( flhLen2 == sizeof ( gprs_Params_t )&&(gprsParams.devID[0]=='A'||gprsParams.devID[0]=='L' ) )
	{
			STORAGE_FlashParams_WriteData(( u8* )&gprsParams ,sizeof( gprs_Params_t ),FLASH_GPRS_PARAMS);
	}
	else
	{		
			gprs_Params_t gprs_ParamsTemp={0};
			STORAGE_FlashParams_ReadData(( u8* )&gprsParams ,sizeof( gprs_Params_t ),FLASH_GPRS_PARAMS);
		  if ( gprs_ParamsTemp.devID[0]=='A'||gprs_ParamsTemp.devID[0]=='L' )
			{
					memcpy((u8*)&gprsParams,(u8*)&gprs_ParamsTemp,sizeof(gprs_Params_t));
					STORAGE_WriteData( GPRS_PARAMS, sizeof( gprs_Params_t ), ( u8* )&gprsParams );
					printf("\r\n STORAGE_FlashParams_ReadData ---> gprsParams  \r\n");
			}
			else
			{
					sysResetGprs();
			}

	}

	
}







/* 
 * ��������������Ӧ�ò���
 * ���ò�������
 *        
 * ����ֵ  ����
 * 
 */
extern void sysParamsSave ( void )
{
  /* �ն����߿��ƿ���: Ĭ�ϴ� */
//	vehicleParams.sleepMode=TRUE; 
  /*��������*/
  STORAGE_WriteData ( VEHICLE_PARAMS,  sizeof( vechile_Params_t ), ( u8 * )&vehicleParams );
  /* GPRSͨ�Ų��� */
  STORAGE_WriteData ( GPRS_PARAMS, sizeof( gprs_Params_t ), ( u8* )&gprsParams );
	
	STORAGE_FlashParams_WriteData(( u8* )&gprsParams ,sizeof( gprs_Params_t ),FLASH_GPRS_PARAMS);
	
	STORAGE_FlashParams_WriteData(( u8 * )&vehicleParams ,sizeof( vechile_Params_t ),FLASH_VEHICLE_PARAMS);

}










/* 
 * �����������ָ�����ֵ
 * ���ò�������
 *        
 * ����ֵ  ����
 * 
 */
extern void sysParamsReset ( void )
{


    PRINTF_GPS_TIME ();
    printf("\r\n[PARA EVENT] reset system parameters\r\n" );

		sysResetVehicle ();
		sysResetGprs ();
  	sysResetState ();
		sysResetAccWorkPara ();
  	sysResetAccWorkStatsInfo();
		sysResetControllerPara ();


}



