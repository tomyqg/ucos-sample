#ifndef _GB_DATA_TYPE_H_

#define _GB_DATA_TYPE_H_


#include "stm32f4xx.h"


#define  LOGIN_BUFFER_LEN   						64
#define  LOGOUT_BUFFER_LEN  						64
#define  TIME_CORRECT_BUFFER_LEN        64
#define  GPRS_HEART_BUFFER_LEN    			30
#define  REALDATA_UPLOAD_BUFFER_LEN     600
//Ƭ��flash ä���洢����
#define  Dead_DATA_STORAGE_INDEX       10000     

typedef struct
{
	u8 	vehicle_state;	  				//����״̬	  
	u8 	charge_state;		    			//���״̬
	u8 	charge_state_flag;				//���״̬��־λ	  
	u8 	run_mode;			   					//����ģʽ	  
	u16 vehicle_speed;						//����
	u32 accumulated_distance;			//�ۼ����	
	u16 total_voltage;						//�ܵ�ѹ��
	u16 total_current;						//�ܵ�����
	u8 	soc;											//SOC
	u8 	dc_dc_state;							//DC-DC״̬
	u8 	gear;											//��λ
	u16 ir;												//��Ե�����
	u8  gas_value;								//����̤���г�ֵ	  
	u8  braking_value; 					  //�ƶ�̤��״̬ 	      
}vehicle_data_t;

typedef struct
{
	u8 motor_num;									//�����������
	u8 motor_SN;									//�������˳���
	u8 motor_state;			   				//�������״̬
	u8 motor_ctrl_temperature;		//��������������¶�
	u16 motor_speed;							//�������ת�ٸ�
	u16 motor_torque;		   				//�������ת�ظ�
	u8 motor_temperature;					//��������¶�
	u16 motor_ctrl_voltage;				//��������������ѹ��
	u16 motor_ctrl_current;				//���������ֱ��ĸ�ߵ�����

}motor_data_t;


typedef struct
{
	u8  highestVoltage_Subsys_num;							//��ߵ�ѹ�����ϵͳ��
	u8  highestVoltage_Single_code;	   					//��ߵ�ѹ��ص������
	u16 SingleBattery_HighestVoltage;	  				//��ص����ѹ���ֵ

	u8  lowestVoltage_Subsys_num;		   					//��͵�ѹ�����ϵͳ��
	u8  lowestVoltage_Single_code;							//��͵�ѹ��ص������
	u16 SingleBattery_LowestVoltage;						//��ص����ѹ���ֵ

	u8 highestTemperature_Subsys_num;						//����¶���ϵͳ��
	u8 highestTemperature_Probe_code;						//����¶�̽�����      
	u8 Probe_HighestTemperature;								//����¶�ֵ

	u8 lowestTemperature_Subsys_num;						//����¶���ϵͳ��
	u8 lowestTemperature_Probe_code;						//����¶�̽�����		
	u8 Probe_LowestTemperature;									//����¶�ֵ
}extreme_value_data_t;



typedef struct
{
	u16 	device_storeage_cycletime;		//�ն˱��ش洢����
	u16 	dataupload_cycletime;					//������Ϣ�ϱ�����
	u16 	warning_upload_cycletime;			//��������µ��ϱ�����
	u8  	manage_domain_name_len;				//����ƽ̨��������
	u8  	manage_domain_name[128];			//����ƽ̨����
	u16	 	manage_port;									//����ƽ̨�˿�
  u8  	hard_version[5+1];							//Ӳ���汾
	u8    soft_version[5+1];							//����汾
	u8  	heartbeat_cycletime;					//��������
	u16 	device_ack_timeout;						//�ն�Ӧ��ʱʱ��
	u16 	platform_ack_timeout;					//ƽ̨Ӧ��ʱʱ��
	u8  	login_fail_next_loglin_time;	//���ε�¼ʧ�ܺ�ĵ�¼ʱ����
	u8  	public_domain_name_len;				//����ƽ̨��������
	u8  	public_domain_name[128];			//����ƽ̨����
	u16 	public_port;									//����ƽ̨�˿�
	u8  	sample_test;                  //�Ƿ�������
}parameter_query_data_t;

typedef struct
{
	u8 	stored_energy_sys_num;			 				//������ϵͳ����
	u8 	stored_energy_sys_sn;			 				  //������ϵͳ��
	u16 stored_energy_voltage;							//����װ�õ�ѹ
	u16 stored_energy_current;			 				//����װ�õ���
	u16 single_battery_total_num;					 	//����������
//��ѹ�б�
/*************************************************************************/	
	//����������			 			
	u16 this_frame_InitialBattery_code;			//��֡��ʼ������
	u16 this_frame_battery_num;							//��֡��������
	u16  singlebattery_voltage_list[200];		//�����ص�ѹ
/*************************************************************************/
//�¶��б�
/*************************************************************************/
	u16 temperature_probe_num;					 		//�¶�̽������
	u8  probe_temperature_list[64];			 	//�¶�̽���¶��б�
/**************************************************************************/
	
}stored_energy_sys_data_t;

typedef struct
{
	u8 	highest_warning_grade;						//��߱����ȼ�
	u32 warning_sign;											//ͨ�ñ�����־
	u8  warning_grade_list[32]; 					//
/*	
	u8 	chargeable_Fault_num;             //���װ�ù��ϣ������Զ���
	u8  chargeable_Fault_list[32];
	u8 	other2_warning_num;
	u8 	other3_warning_num;
	u8 	other4_warning_num;
	*/
}warning_sys_data_t;

typedef struct
{
u8  used;															//������ʹ��ʹ��
u8  engine_state;											//������״̬
u16 crankshaft_speed;									//����ת��
u16 fuel_consumption;									//ȼ��������
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


