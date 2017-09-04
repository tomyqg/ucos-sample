#include "project_configure.h"
#include "intermediate_layer.h"
#include "gb_data_type.h"
#include "low_power_proc.h"

#if defined LDFZ_PROJECT
/*
*=====================================================================================
*陆地方舟：
*协议类型：南车 0.4
*=====================================================================================
*/

#define DEBUG_UDS  0
/***********/
/* CAN缓冲 */
/***********/

/* CAN接收数据缓冲区 */
extern ascii CanRecvedBuffer[];



/* CAN数据控制 */
static ascii *pCanData = ( ascii* )&CanRecvedBuffer[ 0 ];

/* CAN发送数据 */
extern ascii CanToSendBuffer[];

/* CAN数据备份缓冲区 */
extern can_data_t canBackupData;


//#define HTONL(v)	( ((v) << 24) | (((v) >> 24) & 255) | (((v) << 8) & 0xff0000) | (((v) >> 8) & 0xff00) )
//#define HTONS(v)	( (((v) << 8) & 0xff00) | (((v) >> 8) & 255) )

/****************/
/* 控制器标志组 */
/****************/

/* CAN协议信息 */
extern can_info_t CanInfo;

/* CAN发送数据轮询定时器 */


/* 
 * 功能描述：CAN发送数据初始化
 * 引用参数：无
 *          
 * 返回值  ：状态码
 * 
 */
extern void LDFZ_CAN_Application_Init ( void )
{
	gb_data_type_init();
	
	Vehicle_Data.run_mode=1;
	Motor_Data.motor_num=1;
	Motor_Data.motor_SN=1;
	Stored_Energy_sys_Data.stored_energy_sys_num=1;
	Stored_Energy_sys_Data.stored_energy_sys_sn=1;
	Stored_Energy_sys_Data.this_frame_InitialBattery_code=1;
	/*单体电池总数*/
	Stored_Energy_sys_Data.single_battery_total_num=192;
	/*本帧电池总数*/
	Stored_Energy_sys_Data.this_frame_battery_num=Stored_Energy_sys_Data.single_battery_total_num;
	/*本帧电池起始序号*/
	Stored_Energy_sys_Data.this_frame_InitialBattery_code=1;
	
	/*温度探针总数*/
	Stored_Energy_sys_Data.temperature_probe_num=48;

}



/* 
 * 功能描述：CAN发送数据备份
 * 引用参数：无
 *          
 * 返回值  ：状态码
 * 
 */
extern void LDFZ_CAN_BackupBufferReset ( void )
{

}


void Get_Candata( u32 FrameID, u16 DataSize, u8 *IPData )
{

}



/* 
 * 功能描述：CAN数据接收处理
 * 引用参数：(1)帧ID
 *           (2)数据长度
 *           (3)数据内容
 *
 * 返回值  ：状态码
 * 
 */
extern bool LDFZ_CAN_RecvDataHdlr ( u32 FrameID, u16 DataSize, u8 *IPData )
{

//	printf("\r\n   CAN_ID :  %x   帧长度：%d    \r\n",  FrameID, DataSize);	
//	printf(" \r\n");
//	for(u8 i=0;i<DataSize;i++)
//	{
//		printf(" %02x  ", IPData[i]);	
//	}
//	printf(" \r\n");


	//初始化区
	switch ( FrameID )
	{
/*整车数据*/
		case 0x0C01A70A:
		{
			/*可充电储能子系统个数*/
			//Stored_Energy_sys_Data.stored_energy_sys_num=IPData[0];//
			/*车辆状态*/
			Vehicle_Data.vehicle_state=IPData[1];//
			/*充电状态*/
			Vehicle_Data.charge_state= IPData[2];//
			/*运行模式*/
			Vehicle_Data.run_mode=IPData[3];//
			/*驱动电机个数*/
			//Motor_Data.motor_num=IPData[4];//不从CAN里获取

			break;		
		}
		case 0x0C19A7A1:
		{
			/*车速*/
			Vehicle_Data.vehicle_speed=(u16)DATA_VehicleSpeed(IPData[3]);//

			break;		
		}
		
		case 0x0C1AA7A1:
		{
			/*累计里程*/
			Vehicle_Data.accumulated_distance=(u32)DATA_Accumulated_Distance((IPData[0]|(IPData[1]<<8)|(IPData[2]<<16)|(IPData[3]<<24)));//
			break;		
		}

/*驱动电机数据*/
		/*驱动电机个数*/	
		case 0x0C04A1A7:
		{
			/*驱动电机温度*/
			
      Motor_Data.motor_temperature=IPData[0];//
			/*驱动电机控制器温度*/
			Motor_Data.motor_ctrl_temperature=IPData[1];// 
		
			break;
		}
		case 0x0C03A70A:
		{
//			/*驱动电机序号*/
//			Motor_Data.motor_SN=IPData[0];
			/*驱动电机状态*/
			Motor_Data.motor_state=IPData[5]; //    

			break;
		}
		
		//case 0x0C08A79A:
//		case 0xFFFFFFFF:
//		{
//			/*总电压*/
//      Vehicle_Data.total_voltage=DATA_Total_Voltage((IPData[1]<<8)|IPData[0]); //
//			/*总电流*/
//			Vehicle_Data.total_current=DATA_Total_Current((IPData[3]<<8)|IPData[2]);//

//			break;
//		}
		case 0x0C03A1A7:
		{	
			/*驱动电机控制器输入电压*/
			Motor_Data.motor_ctrl_voltage=(((IPData[1]<<8)|IPData[0])-10000);//
			/*电机控制器直流母线电流*/
			Motor_Data.motor_ctrl_current=((IPData[3]<<8)|IPData[2]);//
			/*驱动电机转矩*/
			Motor_Data.motor_torque=(((IPData[5]<<8)|IPData[4])-30000)*10;//
			/*驱动电机转速*/
			Motor_Data.motor_speed=((IPData[7]<<8)|IPData[6])/2+20000;//

			break;
		}
		case 0x0C06A1A7:
		{		
			/*加速踏板行程值*/
			Vehicle_Data.gas_value=IPData[0]*2/5;//
			
			/*制动踏板状态*/
			Vehicle_Data.braking_value=IPData[1]*2/5;//

			break;
		}
	
/*极值数据*/
		case 0x181ED0F3:
	  {
			/*最高电压电池子系统号*/
			/*最高电压电池单体代号*/
			Extreme_Value_Data.highestVoltage_Subsys_num=IPData[0];//
			Extreme_Value_Data.highestVoltage_Single_code=IPData[1];//
			/*最低电压电池子系统号*/
			/*最低电压电池单体代号*/
			Extreme_Value_Data.lowestVoltage_Subsys_num=IPData[2];//
			Extreme_Value_Data.lowestVoltage_Single_code=IPData[3];//
			break;
	  }

		case 0x181AD0F3:
	  {
			/*绝缘电阻*/
			Vehicle_Data.ir=DATA_IR(IPData[0]|(IPData[1]<<8));//
			/*电池单体电压最高值*/
		  Extreme_Value_Data.SingleBattery_HighestVoltage=(((IPData[5]<<8)|IPData[4])-10000)*10;//
			
			/*电池单体电压最低值*/
			Extreme_Value_Data.SingleBattery_LowestVoltage=(((IPData[7]<<8)|IPData[6])-10000)*10;//

			break;
	  }

		/*最高温度子系统号*/
		/*最高温度探针序号*/
		/*最高温度值*/
		case 0x181DD0F3:
		{
			/*最高温度子系统号*/
			/*最高温度探针序号*/
			/*最高温度值*/
			Extreme_Value_Data.highestTemperature_Subsys_num=IPData[1];//
			Extreme_Value_Data.highestTemperature_Probe_code=IPData[2];//
			Extreme_Value_Data.Probe_HighestTemperature=DATA_Probe_HighestTemperature(IPData[0]);//
			
			/*最低温度子系统号*/
			/*最低温度探针序号*/
			/*最低温度值*/
			Extreme_Value_Data.lowestTemperature_Subsys_num=IPData[4];//
			Extreme_Value_Data.lowestTemperature_Probe_code=IPData[5];//
			Extreme_Value_Data.Probe_LowestTemperature=DATA_Probe_LowestTemperature(IPData[3]);//
			
			
			break;
		}

	/*报警数据*/
		/*最高报警等级*/
		/*通用报警标志*/	
		case 0x0C02A70A:
		{
			/*档位*/
			Vehicle_Data.gear=IPData[4];//
			/*DC/DC状态*/
			Vehicle_Data.dc_dc_state=IPData[6];//
			
			/*报警标志*/	
			Com_Warning_Data.warning_sign=( ((IPData[1]&0x07)<<16)|(IPData[1]<<8)|IPData[0] );
			
			break;
		}	
		case 0x18F82B9E:
		{
			//最高报警等级
			//Com_Warning_Data.highest_warning_grade=IPData[0];//没有		
			break;
		}
		
/*可充电储能电压*/
		case 0x1818D0F3:
	  {		
			
//		/*可充电储能子系统号*/
//		Stored_Energy_sys_Data.stored_energy_sys_sn=IPData[1];
			
			/*总电压*/
      Vehicle_Data.total_voltage=(((IPData[1]<<8)|IPData[0])-10000); //
			/*总电流*/
			Vehicle_Data.total_current=DATA_Total_Current((IPData[3]<<8)|IPData[2]);//
			
			/*可充电装置电压*/
			Stored_Energy_sys_Data.stored_energy_voltage=((IPData[1]<<8)|IPData[0])-10000;//
			
			/*可充电装置电流*/
			Stored_Energy_sys_Data.stored_energy_current=((IPData[3]<<8)|IPData[2]);//
			
			/*SOC*/
			Vehicle_Data.soc=IPData[4]*2/5;//
			break;
	  }
/*可充电储能温度探针*/
		case 0x181CD0F:
	  {
			/*单体电池总数*/
			Stored_Energy_sys_Data.single_battery_total_num=IPData[6];//
			/*可充电储能温度探针个数*/
			Stored_Energy_sys_Data.temperature_probe_num=IPData[7];//		
			break;
	  }
		/*可充电储能温度列表: 通过单一ID报文接收*/
		case 0x180328F4:
	  {
				if( (IPData[0]!=0xFF)&&(IPData[1]!=0xFF) )
				Stored_Energy_sys_Data.probe_temperature_list[IPData[0]]=IPData[1];
				if( (IPData[2]!=0xFF)&&(IPData[3]!=0xFF) )
				Stored_Energy_sys_Data.probe_temperature_list[IPData[2]]=IPData[3];
				if( (IPData[4]!=0xFF)&&(IPData[5]!=0xFF) )
				Stored_Energy_sys_Data.probe_temperature_list[IPData[4]]=IPData[5];
				if( (IPData[6]!=0xFF)&&(IPData[7]!=0xFF) )
				Stored_Energy_sys_Data.probe_temperature_list[IPData[6]]=IPData[7];
			break;
	  }
		/*可充电储能电压列表: 通过单一ID报文接收*/
		case 0x180528F3: 
	  {
			IsLPWR_Counter=0;//CAN休眠条件
      if( (IPData[0]!=0xFF)&&(IPData[1]!=0xFF)&&(IPData[2]!=0xFF)&&(IPData[6]!=0xFF) )
			Stored_Energy_sys_Data.singlebattery_voltage_list[((IPData[6]<<8)|IPData[0])]=(IPData[2]<<8)|IPData[1];
			if( (IPData[3]!=0xFF)&&(IPData[4]!=0xFF)&&(IPData[5]!=0xFF)&&(IPData[7]!=0xFF) )
			Stored_Energy_sys_Data.singlebattery_voltage_list[((IPData[7]<<8)|IPData[3])]=(IPData[5]<<8)|IPData[4];
			break;
	  }
		
	}
	return TRUE;
}





#endif  /* NT_PROJECT */



