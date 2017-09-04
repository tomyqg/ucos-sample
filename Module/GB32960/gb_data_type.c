#include "gb_data_type.h"

#include<string.h>










/******************************************/
/*            32960数据类型		            */
/******************************************/

vehicle_data_t 							Vehicle_Data;
motor_data_t  		    			Motor_Data;
extreme_value_data_t   			Extreme_Value_Data;
warning_sys_data_t					Com_Warning_Data;
stored_energy_sys_data_t 		Stored_Energy_sys_Data;


//发动机数据
engine_sys_data_t						Engine_Sys_Data;
 
parameter_query_data_t 			Parameter_Query_Data;
 
 
 
void gb_data_type_init(void)
{

	memset ( ( u8 * )&Vehicle_Data, 0, sizeof( vehicle_data_t ) );
	memset ( ( u8 * )&Motor_Data, 0, sizeof( motor_data_t ) );
	memset ( ( u8 * )&Extreme_Value_Data, 0, sizeof( extreme_value_data_t ) );
	memset ( ( u8 * )&Stored_Energy_sys_Data, 0, sizeof( warning_sys_data_t ) );
}	
 
 




