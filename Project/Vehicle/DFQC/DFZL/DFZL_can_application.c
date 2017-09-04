#include "project_configure.h"
#include "intermediate_layer.h"
#include "gb_data_type.h"
#include "low_power_proc.h"

#if defined DFZL_PROJECT


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

static u16 index_v;

static u16 index_t;
//#define HTONL(v)	( ((v) << 24) | (((v) >> 24) & 255) | (((v) << 8) & 0xff0000) | (((v) >> 8) & 0xff00) )
//#define HTONS(v)	( (((v) << 8) & 0xff00) | (((v) >> 8) & 255) )

/****************/
/* 控制器标志组 */
/****************/

/* CAN协议信息 */
extern can_info_t CanInfo;

/* CAN发送数据轮询定时器 */



//#define  	VehicleSpeed_Factor_can    						0.1		
//#define  	VehicleSpeed_Offset_can    						0

//#define  	VehicleSpeed_Factor_gb    						0.1		
//#define  	VehicleSpeed_Offset_gb    						0

//#define  	VehicleSpeed_Factor    						(VehicleSpeed_Factor_can/VehicleSpeed_Factor_gb)		
//#define  	VehicleSpeed_Offset    						((VehicleSpeed_Offset_can-VehicleSpeed_Offset_gb)/VehicleSpeed_Factor_gb)
//#define 	DATA_VehicleSpeed(_X_)    				(( u16 )(  (_X_) * VehicleSpeed_Factor_can/VehicleSpeed_Factor_gb  + (VehicleSpeed_Offset_can-VehicleSpeed_Offset_gb)/VehicleSpeed_Factor_gb ))

//#define  	Vehi   						1		
//#define  	Veh    						1

//#define 	DATA_VehicleSpeed1(_X_)    				(( u16 )( ( (_X_)*(Vehi*Veh)+ 10)))

/* 
 * 功能描述：CAN发送数据初始化
 * 引用参数：无
 *          
 * 返回值  ：状态码
 * 
 */
extern void DFZL_CAN_Application_Init ( void )
{
	gb_data_type_init();
	
	Vehicle_Data.run_mode=1;
	Motor_Data.motor_num=1;
	Motor_Data.motor_SN=1;
	Stored_Energy_sys_Data.stored_energy_sys_num=1;
	Stored_Energy_sys_Data.stored_energy_sys_sn=1;
	Stored_Energy_sys_Data.this_frame_InitialBattery_code=1;
	/*单体电池总数*/
	Stored_Energy_sys_Data.single_battery_total_num=96;
	/*本帧电池总数*/
	Stored_Energy_sys_Data.this_frame_battery_num=Stored_Energy_sys_Data.single_battery_total_num;
	/*本帧电池起始序号*/
	Stored_Energy_sys_Data.this_frame_InitialBattery_code=1;

	/*温度探针总数*/
	Stored_Energy_sys_Data.temperature_probe_num=24;
}



/* 
 * 功能描述：CAN发送数据备份
 * 引用参数：无
 *          
 * 返回值  ：状态码
 * 
 */
extern void DFZL_CAN_BackupBufferReset ( void )
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
extern bool DFZL_CAN_RecvDataHdlr ( u32 FrameID, u16 DataSize, u8 *IPData )
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
		case 0x18F104D0:
		{
			/*车辆状态*/
			if(IPData[0]>3)
				Vehicle_Data.vehicle_state=(IPData[0]&0xF);
			else
				Vehicle_Data.vehicle_state=IPData[0]&0x0F;
			/*运行模式*/
			if((IPData[0]>>4)>3)
				Vehicle_Data.run_mode=(IPData[0]>>4);
			else
				Vehicle_Data.run_mode=(IPData[0]>>4);
			/*车速*/
			Vehicle_Data.vehicle_speed=(((IPData[2]<<8)|IPData[1]));

			/*累计里程*/
			Vehicle_Data.accumulated_distance=(u32)DATA_Accumulated_Distance((IPData[3]|(IPData[4]<<8)|(IPData[5]<<16)|(IPData[6]<<24)));
			break;		
		}
		case 0x10F8159E:
		{
			/*SOC*/
			Vehicle_Data.soc=IPData[2];
			/*总电流*/
			Vehicle_Data.total_current=(((IPData[4]<<8)|IPData[3])-22000);
			/*总电压*/
      Vehicle_Data.total_voltage=((IPData[6]<<8)|IPData[5]); 

			break;
		}
		case 0x18F105D0:
		{
			/*DC/DC状态*/
			Vehicle_Data.dc_dc_state=IPData[4];
			/*档位*/
			Vehicle_Data.gear=IPData[5];
			break;
		}	
/*驱动电机数据*/
		/*驱动电机个数*/	
		case 0x18F106D0:
		{
			/*驱动电机序号*/
			Motor_Data.motor_SN=IPData[0];
			/*驱动电机状态*/
			Motor_Data.motor_state=IPData[1];
			/*驱动电机控制器温度*/
			Motor_Data.motor_ctrl_temperature=DATA_Motor_Ctrl_Temp(IPData[2]);      
			/*驱动电机转速*/
			Motor_Data.motor_speed=((IPData[4]<<8)|IPData[3]);
			/*驱动电机转矩*/
			Motor_Data.motor_torque=((IPData[6]<<8)|IPData[5]);
			/*驱动电机温度*/
      Motor_Data.motor_temperature=DATA_Motor_Temp(IPData[7]);
			break;
		}
		case 0x18F107D0:
		{
			/*驱动电机序号*/
			Motor_Data.motor_SN=IPData[0];
			/*驱动电机控制器输入电压*/
			Motor_Data.motor_ctrl_voltage=((IPData[2]<<8)|IPData[1]);
			/*电机控制器直流母线电流*/
			Motor_Data.motor_ctrl_current=DATA_Motor_Ctrl_Current((IPData[4]<<8)|IPData[3]);
			/*加速踏板行程值*/
			Vehicle_Data.gas_value=IPData[5];
			/*制动踏板状态*/
			Vehicle_Data.braking_value=IPData[6];

			break;
		}
/*极值数据*/
		case 0x18F8289E:
	  {
			/*最高电压电池子系统号*/
			/*最高电压电池单体代号*/
			/*电池单体电压最高值*/
			Extreme_Value_Data.highestVoltage_Subsys_num=IPData[0];
			Extreme_Value_Data.highestVoltage_Single_code=IPData[1];
			Extreme_Value_Data.SingleBattery_HighestVoltage=((IPData[3]<<8)|IPData[2]);
			/*最低电压电池子系统号*/
			/*最低电压电池单体代号*/
			/*电池单体电压最低值*/
			Extreme_Value_Data.lowestVoltage_Subsys_num=IPData[4];
			Extreme_Value_Data.lowestVoltage_Single_code=IPData[5];
			Extreme_Value_Data.SingleBattery_LowestVoltage=((IPData[7]<<8)|IPData[6]);
			break;
	  }

		/*最高温度子系统号*/
		/*最高温度探针序号*/
		/*最高温度值*/
		case 0x18F8299E:
		{
			/*最高温度子系统号*/
			/*最高温度探针序号*/
			/*最高温度值*/
			Extreme_Value_Data.highestTemperature_Subsys_num=IPData[0];
			Extreme_Value_Data.highestTemperature_Probe_code=IPData[1];
			Extreme_Value_Data.Probe_HighestTemperature=DATA_Probe_HighestTemperature(IPData[2]);
			
			/*最低温度子系统号*/
			/*最低温度探针序号*/
			/*最低温度值*/
			Extreme_Value_Data.lowestTemperature_Subsys_num=IPData[3];
			Extreme_Value_Data.lowestTemperature_Probe_code=IPData[4];
			Extreme_Value_Data.Probe_LowestTemperature=DATA_Probe_LowestTemperature(IPData[5]);
			
			/*绝缘电阻*/
			Vehicle_Data.ir=DATA_IR(IPData[6]|(IPData[7]<<8));
			break;
		}

	/*报警数据*/
		/*最高报警等级*/
		/*通用报警标志*/
		
		case 0x18F82C9E:
		{
			
			/*报警标志*/
		  Com_Warning_Data.warning_sign=(((IPData[1]&0x10)<<16)|((IPData[1]&0x0f)<<8)|IPData[0]);
			/*充电状态*/
			Vehicle_Data.charge_state= IPData[2];
			
			
			break;
		}
		case 0x18F82B9E:
		{
			//最高报警等级
			Com_Warning_Data.highest_warning_grade=IPData[0];		
			break;
		}
		
		
/*可充电储能电压*/
		case 0x18F82D9E:
	  {
			/*可充电储能子系统个数*/
			Stored_Energy_sys_Data.stored_energy_sys_num=IPData[0];
			
			/*可充电储能子系统号*/
			Stored_Energy_sys_Data.stored_energy_sys_sn=IPData[1];
			
			/*可充电装置电压*/
			Stored_Energy_sys_Data.stored_energy_voltage=DATA_StoredEnergyVoltage((IPData[3]<<8)|IPData[2]);
			
			/*可充电装置电流*/
			Stored_Energy_sys_Data.stored_energy_current=DATA_StoredEnergyCurrent((IPData[5]<<8)|IPData[4]);
			break;
	  }
/*可充电储能温度探针*/
		case 0x18F82E9E:
	  {
			/*可充电储能子系统号*/
			Stored_Energy_sys_Data.stored_energy_sys_sn=IPData[0];
			
			/*可充电储能温度探针个数*/
			Stored_Energy_sys_Data.temperature_probe_num=(IPData[2]<<8)|IPData[1];
			
			/*单体电池总数*/
			Stored_Energy_sys_Data.single_battery_total_num=(IPData[4]<<8)|IPData[3];
			
			break;
	  }
		default:
	  {
			/*可充电储能装置电压数据*/
			if(	FrameID>=LIST_Voltage_START&&FrameID<=LIST_Voltage_END&&\
					(FrameID&0xFFFF)==(LIST_Voltage_START&0xFFFF))
			{
				
				IsLPWR_Counter=0;//CAN休眠条件
				index_v=((FrameID-LIST_Voltage_START)>>16);

				Stored_Energy_sys_Data.singlebattery_voltage_list[4*index_v+0]=(IPData[1]<<8)|IPData[0];
				Stored_Energy_sys_Data.singlebattery_voltage_list[4*index_v+1]=(IPData[3]<<8)|IPData[2];
				Stored_Energy_sys_Data.singlebattery_voltage_list[4*index_v+2]=(IPData[5]<<8)|IPData[4];
				Stored_Energy_sys_Data.singlebattery_voltage_list[4*index_v+3]=(IPData[7]<<8)|IPData[6];
		
			}
			/*可充电储能装置温度数据*/
			if(	FrameID>=LIST_Temperature_START&&FrameID<=LIST_temperature_END&&\
					(FrameID&0xFFFF)==(LIST_Temperature_START&0xFFFF)
			)
			{
				index_t =((FrameID-LIST_Temperature_START)>>16);
				Stored_Energy_sys_Data.probe_temperature_list[OneFrameProbe*index_t+0]=IPData[0];
				Stored_Energy_sys_Data.probe_temperature_list[OneFrameProbe*index_t+1]=IPData[1];
				Stored_Energy_sys_Data.probe_temperature_list[OneFrameProbe*index_t+2]=IPData[2];
				Stored_Energy_sys_Data.probe_temperature_list[OneFrameProbe*index_t+3]=IPData[3];
				Stored_Energy_sys_Data.probe_temperature_list[OneFrameProbe*index_t+4]=IPData[4];
				Stored_Energy_sys_Data.probe_temperature_list[OneFrameProbe*index_t+5]=IPData[5];
				Stored_Energy_sys_Data.probe_temperature_list[OneFrameProbe*index_t+6]=IPData[6];
				Stored_Energy_sys_Data.probe_temperature_list[OneFrameProbe*index_t+7]=IPData[7];
				
			}
	    break;
	  }
	     
	}
	return TRUE;
}





#endif  /* NT_PROJECT */



