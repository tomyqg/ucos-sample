#include <stdio.h>
#include <string.h>
#include "tiza_params.h"
#include "tiza_gprs_protocol.h"
#include "tiza_storage.h"
#include "tiza_gprs_protocol.h"
#include "w25qxx_port.h"





/* 车机参数 */
vechile_Params_t vehicleParams;

/* GPRS参数 */
gprs_Params_t  gprsParams;

/* 位置参数  */
location_t locations;

/* GPS数据 */
extern GPS_data RMC_d;

#if TZ_DEFAULT_CONTROLLER_TYPE > TZ_CONTROLLER_TYPE_NONE
/* 控制器数据 */
extern controller_data_t controllerBackupData;
#endif  /* !TZ_CONTROLLER_TYPE_NONE */ 














/******************************************/
/*              系统参数[设置]            */
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
 * 功能描述：系统SMS参数
 * 引用参数：无
 * 
 * 返回值  ：无
 * 
 */
extern void sysResetVehicle ( void )
{
		vehicleParams.sleepMode = TRUE;
		sprintf ( ( char * )vehicleParams.CarVIN, "%s", TZ_DEFAULT_PEER_VIN );
		STORAGE_WriteData ( VEHICLE_PARAMS, sizeof( vechile_Params_t ), ( u8 * )&vehicleParams );
}










/* 
 * 功能描述：系统GPRS参数
 * 引用参数：无
 *     
 * 返回值  ：无
 * 
 */
extern void sysResetGprs ( void )
{
  sprintf ( ( char * )gprsParams.APN, "%s", TZ_DEFAULT_GPRS_APN );
  sprintf ( ( char * )gprsParams.IP1, "%s", TZ_DEFAULT_PEER_ADDR1 );
  sprintf ( ( char * )gprsParams.IP2, "%s", TZ_DEFAULT_PEER_ADDR2 );
  sprintf ( ( char * )gprsParams.ip1_domain, "%s", TZ_DEFAULT_MS_DOMIAN );
  sprintf ( ( char * )gprsParams.ip2_domain, "%s", TZ_DEFAULT_SS_DOMIAN );

  gprsParams.ver = TZ_DEFAULT_PROTOCOL_VERSION; /* 协议版本号 */
  gprsParams.Port1 = TZ_DEFAULT_PEER_PORT;  /* 端口号1 */
  gprsParams.Port2 = TZ_DEFAULT_PEER_PORT;  /* 端口号2 */
  gprsParams.net_is_bad = 0;  /* 网络状态：差计数器（单位：120s） */
  gprsParams.net_is_ok = 0; /* 网络状态：好计数器（单位：120s） */
  gprsParams.DeviceIDSet = FALSE; /* 设备ID号未设置 */

  STORAGE_WriteData ( GPRS_PARAMS, sizeof( gprs_Params_t ), ( u8 * )&gprsParams );
}








/* 
 * 功能描述：复位应用状态信息
 * 引用参数：无
 *     			
 * 返回值  ：无
 * 
 */
extern void sysResetState ( void )
{

}









/* 
 * 功能描述：复位ACC工作参数配置信息
 * 引用参数：无
 *     			
 * 返回值  ：无
 * 
 */
extern void sysResetAccWorkPara ( void )
{

}







#if APPLI_PARAMTER_DEBUG == 1

/* 
 * 功能描述：显示ACC工作参数配置信息
 * 引用参数：无
 *     			
 * 返回值  ：无
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
 * 功能描述：
 * 引用参数：无
 * 
 * 返回值  ：无
 * 
 */
extern void sysResetAccWorkStatsInfo ( void )
{

}












/* 
 * 功能描述：系统SMS参数
 * 引用参数：无
 * 
 * 返回值  ：无
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
 * 功能描述：系统GPRS参数
 * 引用参数：无
 *     			
 * 返回值  ：无
 * 
 */
extern void sysResetControllerPara ( void )
{

} 


          







#if TZ_USE_STORAGE_MASS_DATA == 1

/* 
 * 功能描述：系统GPRS参数
 * 引用参数：无
 *     			
 * 返回值  ：无
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
 * 功能描述：读取应用参数
 * 引用参数：无
 *          
 * 返回值  ：无
 * 
 */
extern void sysParamsInit ( void )
{
	/* 设置应用运行状态 */
  sys_status = PROC_SYSTEM_INIT;
	DeadData_FlashConfigInit();
	memset ( ( u8 * )&locations, 0, sizeof( location_t ) );
	STORAGE_ReadData ( GPRS_PARAMS, sizeof( gprs_Params_t ), ( u8* )&gprsParams );	
	STORAGE_ReadData ( VEHICLE_PARAMS,  sizeof( vechile_Params_t ), ( u8 * )&vehicleParams ); 
	/* 终端休眠控制开关: 默认打开 */
	vehicleParams.sleepMode=TRUE; 
	/*读取片外FLASH中经纬度信息*/
	STORAGE_FlashParams_ReadData(( u8 * )&gps_data_param ,sizeof( gps_data_param_t ),FLASH_GPS_PARAMS);
	
	
	u16 flhLen = 0;
  u16 flhLen2 = 0;
	/* 车辆参数 */
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

	/* GPRS通信参数 */
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
 * 功能描述：保存应用参数
 * 引用参数：无
 *        
 * 返回值  ：无
 * 
 */
extern void sysParamsSave ( void )
{
  /* 终端休眠控制开关: 默认打开 */
//	vehicleParams.sleepMode=TRUE; 
  /*车机参数*/
  STORAGE_WriteData ( VEHICLE_PARAMS,  sizeof( vechile_Params_t ), ( u8 * )&vehicleParams );
  /* GPRS通信参数 */
  STORAGE_WriteData ( GPRS_PARAMS, sizeof( gprs_Params_t ), ( u8* )&gprsParams );
	
	STORAGE_FlashParams_WriteData(( u8* )&gprsParams ,sizeof( gprs_Params_t ),FLASH_GPRS_PARAMS);
	
	STORAGE_FlashParams_WriteData(( u8 * )&vehicleParams ,sizeof( vechile_Params_t ),FLASH_VEHICLE_PARAMS);

}










/* 
 * 功能描述：恢复出厂值
 * 引用参数：无
 *        
 * 返回值  ：无
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



