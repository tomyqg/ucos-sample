#ifndef _GB_DATA_TYPE_H_

#define _GB_DATA_TYPE_H_


#include "stm32f4xx.h"


#define  LOGIN_BUFFER_LEN   						64
#define  LOGOUT_BUFFER_LEN  						64
#define  TIME_CORRECT_BUFFER_LEN        64
#define  GPRS_HEART_BUFFER_LEN    			30
#define  REALDATA_UPLOAD_BUFFER_LEN     600
//片外flash 盲区存储条数
#define  Dead_DATA_STORAGE_INDEX       10000     

typedef struct
{
	u8 	vehicle_state;	  				//车辆状态	  
	u8 	charge_state;		    			//充电状态
	u8 	charge_state_flag;				//充电状态标志位	  
	u8 	run_mode;			   					//运行模式	  
	u16 vehicle_speed;						//车速
	u32 accumulated_distance;			//累计里程	
	u16 total_voltage;						//总电压高
	u16 total_current;						//总电流高
	u8 	soc;											//SOC
	u8 	dc_dc_state;							//DC-DC状态
	u8 	gear;											//档位
	u16 ir;												//绝缘电阻高
	u8  gas_value;								//加速踏板行程值	  
	u8  braking_value; 					  //制动踏板状态 	      
}vehicle_data_t;

typedef struct
{
	u8 motor_num;									//驱动电机个数
	u8 motor_SN;									//驱动电机顺序号
	u8 motor_state;			   				//驱动电机状态
	u8 motor_ctrl_temperature;		//驱动电机控制器温度
	u16 motor_speed;							//驱动电机转速高
	u16 motor_torque;		   				//驱动电机转矩高
	u8 motor_temperature;					//驱动电机温度
	u16 motor_ctrl_voltage;				//电机控制器输入电压高
	u16 motor_ctrl_current;				//电机控制器直流母线电流高

}motor_data_t;


typedef struct
{
	u8  highestVoltage_Subsys_num;							//最高电压电池子系统号
	u8  highestVoltage_Single_code;	   					//最高电压电池单体代号
	u16 SingleBattery_HighestVoltage;	  				//电池单体电压最高值

	u8  lowestVoltage_Subsys_num;		   					//最低电压电池子系统号
	u8  lowestVoltage_Single_code;							//最低电压电池单体代号
	u16 SingleBattery_LowestVoltage;						//电池单体电压最低值

	u8 highestTemperature_Subsys_num;						//最高温度子系统号
	u8 highestTemperature_Probe_code;						//最高温度探针序号      
	u8 Probe_HighestTemperature;								//最高温度值

	u8 lowestTemperature_Subsys_num;						//最低温度子系统号
	u8 lowestTemperature_Probe_code;						//最低温度探针序号		
	u8 Probe_LowestTemperature;									//最低温度值
}extreme_value_data_t;



typedef struct
{
	u16 	device_storeage_cycletime;		//终端本地存储周期
	u16 	dataupload_cycletime;					//正常信息上报周期
	u16 	warning_upload_cycletime;			//报警情况下的上报周期
	u8  	manage_domain_name_len;				//管理平台域名长度
	u8  	manage_domain_name[128];			//管理平台域名
	u16	 	manage_port;									//管理平台端口
  u8  	hard_version[5+1];							//硬件版本
	u8    soft_version[5+1];							//软件版本
	u8  	heartbeat_cycletime;					//心跳周期
	u16 	device_ack_timeout;						//终端应答超时时间
	u16 	platform_ack_timeout;					//平台应答超时时间
	u8  	login_fail_next_loglin_time;	//三次登录失败后的登录时间间隔
	u8  	public_domain_name_len;				//公共平台域名长度
	u8  	public_domain_name[128];			//公共平台域名
	u16 	public_port;									//公共平台端口
	u8  	sample_test;                  //是否抽样检测
}parameter_query_data_t;

typedef struct
{
	u8 	stored_energy_sys_num;			 				//储能子系统个数
	u8 	stored_energy_sys_sn;			 				  //储能子系统号
	u16 stored_energy_voltage;							//储能装置电压
	u16 stored_energy_current;			 				//储能装置电流
	u16 single_battery_total_num;					 	//单体电池总数
//电压列表
/*************************************************************************/	
	//单体电池总数			 			
	u16 this_frame_InitialBattery_code;			//本帧起始电池序号
	u16 this_frame_battery_num;							//本帧单体电池数
	u16  singlebattery_voltage_list[200];		//单体电池电压
/*************************************************************************/
//温度列表
/*************************************************************************/
	u16 temperature_probe_num;					 		//温度探针数量
	u8  probe_temperature_list[64];			 	//温度探针温度列表
/**************************************************************************/
	
}stored_energy_sys_data_t;

typedef struct
{
	u8 	highest_warning_grade;						//最高报警等级
	u32 warning_sign;											//通用报警标志
	u8  warning_grade_list[32]; 					//
/*	
	u8 	chargeable_Fault_num;             //充电装置故障：厂商自定义
	u8  chargeable_Fault_list[32];
	u8 	other2_warning_num;
	u8 	other3_warning_num;
	u8 	other4_warning_num;
	*/
}warning_sys_data_t;

typedef struct
{
u8  used;															//发动机使用使能
u8  engine_state;											//发动机状态
u16 crankshaft_speed;									//曲轴转速
u16 fuel_consumption;									//燃料消耗率
} engine_sys_data_t;




extern vehicle_data_t 						Vehicle_Data;
extern motor_data_t  							Motor_Data;
extern extreme_value_data_t 			Extreme_Value_Data;
extern warning_sys_data_t					Com_Warning_Data;
extern stored_energy_sys_data_t 	Stored_Energy_sys_Data;

extern parameter_query_data_t 		Parameter_Query_Data;
extern engine_sys_data_t					Engine_Sys_Data;




extern void gb_data_type_init(void);


#endif


