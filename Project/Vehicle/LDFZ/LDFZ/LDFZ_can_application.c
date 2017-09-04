#include "project_configure.h"
#include "intermediate_layer.h"
#include "gb_data_type.h"
#include "low_power_proc.h"

#if defined LDFZ_PROJECT
/*
*=====================================================================================
*½�ط��ۣ�
*Э�����ͣ��ϳ� 0.4
*=====================================================================================
*/

#define DEBUG_UDS  0
/***********/
/* CAN���� */
/***********/

/* CAN�������ݻ����� */
extern ascii CanRecvedBuffer[];



/* CAN���ݿ��� */
static ascii *pCanData = ( ascii* )&CanRecvedBuffer[ 0 ];

/* CAN�������� */
extern ascii CanToSendBuffer[];

/* CAN���ݱ��ݻ����� */
extern can_data_t canBackupData;


//#define HTONL(v)	( ((v) << 24) | (((v) >> 24) & 255) | (((v) << 8) & 0xff0000) | (((v) >> 8) & 0xff00) )
//#define HTONS(v)	( (((v) << 8) & 0xff00) | (((v) >> 8) & 255) )

/****************/
/* ��������־�� */
/****************/

/* CANЭ����Ϣ */
extern can_info_t CanInfo;

/* CAN����������ѯ��ʱ�� */


/* 
 * ����������CAN�������ݳ�ʼ��
 * ���ò�������
 *          
 * ����ֵ  ��״̬��
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
	/*����������*/
	Stored_Energy_sys_Data.single_battery_total_num=192;
	/*��֡�������*/
	Stored_Energy_sys_Data.this_frame_battery_num=Stored_Energy_sys_Data.single_battery_total_num;
	/*��֡�����ʼ���*/
	Stored_Energy_sys_Data.this_frame_InitialBattery_code=1;
	
	/*�¶�̽������*/
	Stored_Energy_sys_Data.temperature_probe_num=48;

}



/* 
 * ����������CAN�������ݱ���
 * ���ò�������
 *          
 * ����ֵ  ��״̬��
 * 
 */
extern void LDFZ_CAN_BackupBufferReset ( void )
{

}


void Get_Candata( u32 FrameID, u16 DataSize, u8 *IPData )
{

}



/* 
 * ����������CAN���ݽ��մ���
 * ���ò�����(1)֡ID
 *           (2)���ݳ���
 *           (3)��������
 *
 * ����ֵ  ��״̬��
 * 
 */
extern bool LDFZ_CAN_RecvDataHdlr ( u32 FrameID, u16 DataSize, u8 *IPData )
{

//	printf("\r\n   CAN_ID :  %x   ֡���ȣ�%d    \r\n",  FrameID, DataSize);	
//	printf(" \r\n");
//	for(u8 i=0;i<DataSize;i++)
//	{
//		printf(" %02x  ", IPData[i]);	
//	}
//	printf(" \r\n");


	//��ʼ����
	switch ( FrameID )
	{
/*��������*/
		case 0x0C01A70A:
		{
			/*�ɳ�索����ϵͳ����*/
			//Stored_Energy_sys_Data.stored_energy_sys_num=IPData[0];//
			/*����״̬*/
			Vehicle_Data.vehicle_state=IPData[1];//
			/*���״̬*/
			Vehicle_Data.charge_state= IPData[2];//
			/*����ģʽ*/
			Vehicle_Data.run_mode=IPData[3];//
			/*�����������*/
			//Motor_Data.motor_num=IPData[4];//����CAN���ȡ

			break;		
		}
		case 0x0C19A7A1:
		{
			/*����*/
			Vehicle_Data.vehicle_speed=(u16)DATA_VehicleSpeed(IPData[3]);//

			break;		
		}
		
		case 0x0C1AA7A1:
		{
			/*�ۼ����*/
			Vehicle_Data.accumulated_distance=(u32)DATA_Accumulated_Distance((IPData[0]|(IPData[1]<<8)|(IPData[2]<<16)|(IPData[3]<<24)));//
			break;		
		}

/*�����������*/
		/*�����������*/	
		case 0x0C04A1A7:
		{
			/*��������¶�*/
			
      Motor_Data.motor_temperature=IPData[0];//
			/*��������������¶�*/
			Motor_Data.motor_ctrl_temperature=IPData[1];// 
		
			break;
		}
		case 0x0C03A70A:
		{
//			/*����������*/
//			Motor_Data.motor_SN=IPData[0];
			/*�������״̬*/
			Motor_Data.motor_state=IPData[5]; //    

			break;
		}
		
		//case 0x0C08A79A:
//		case 0xFFFFFFFF:
//		{
//			/*�ܵ�ѹ*/
//      Vehicle_Data.total_voltage=DATA_Total_Voltage((IPData[1]<<8)|IPData[0]); //
//			/*�ܵ���*/
//			Vehicle_Data.total_current=DATA_Total_Current((IPData[3]<<8)|IPData[2]);//

//			break;
//		}
		case 0x0C03A1A7:
		{	
			/*������������������ѹ*/
			Motor_Data.motor_ctrl_voltage=(((IPData[1]<<8)|IPData[0])-10000);//
			/*���������ֱ��ĸ�ߵ���*/
			Motor_Data.motor_ctrl_current=((IPData[3]<<8)|IPData[2]);//
			/*�������ת��*/
			Motor_Data.motor_torque=(((IPData[5]<<8)|IPData[4])-30000)*10;//
			/*�������ת��*/
			Motor_Data.motor_speed=((IPData[7]<<8)|IPData[6])/2+20000;//

			break;
		}
		case 0x0C06A1A7:
		{		
			/*����̤���г�ֵ*/
			Vehicle_Data.gas_value=IPData[0]*2/5;//
			
			/*�ƶ�̤��״̬*/
			Vehicle_Data.braking_value=IPData[1]*2/5;//

			break;
		}
	
/*��ֵ����*/
		case 0x181ED0F3:
	  {
			/*��ߵ�ѹ�����ϵͳ��*/
			/*��ߵ�ѹ��ص������*/
			Extreme_Value_Data.highestVoltage_Subsys_num=IPData[0];//
			Extreme_Value_Data.highestVoltage_Single_code=IPData[1];//
			/*��͵�ѹ�����ϵͳ��*/
			/*��͵�ѹ��ص������*/
			Extreme_Value_Data.lowestVoltage_Subsys_num=IPData[2];//
			Extreme_Value_Data.lowestVoltage_Single_code=IPData[3];//
			break;
	  }

		case 0x181AD0F3:
	  {
			/*��Ե����*/
			Vehicle_Data.ir=DATA_IR(IPData[0]|(IPData[1]<<8));//
			/*��ص����ѹ���ֵ*/
		  Extreme_Value_Data.SingleBattery_HighestVoltage=(((IPData[5]<<8)|IPData[4])-10000)*10;//
			
			/*��ص����ѹ���ֵ*/
			Extreme_Value_Data.SingleBattery_LowestVoltage=(((IPData[7]<<8)|IPData[6])-10000)*10;//

			break;
	  }

		/*����¶���ϵͳ��*/
		/*����¶�̽�����*/
		/*����¶�ֵ*/
		case 0x181DD0F3:
		{
			/*����¶���ϵͳ��*/
			/*����¶�̽�����*/
			/*����¶�ֵ*/
			Extreme_Value_Data.highestTemperature_Subsys_num=IPData[1];//
			Extreme_Value_Data.highestTemperature_Probe_code=IPData[2];//
			Extreme_Value_Data.Probe_HighestTemperature=DATA_Probe_HighestTemperature(IPData[0]);//
			
			/*����¶���ϵͳ��*/
			/*����¶�̽�����*/
			/*����¶�ֵ*/
			Extreme_Value_Data.lowestTemperature_Subsys_num=IPData[4];//
			Extreme_Value_Data.lowestTemperature_Probe_code=IPData[5];//
			Extreme_Value_Data.Probe_LowestTemperature=DATA_Probe_LowestTemperature(IPData[3]);//
			
			
			break;
		}

	/*��������*/
		/*��߱����ȼ�*/
		/*ͨ�ñ�����־*/	
		case 0x0C02A70A:
		{
			/*��λ*/
			Vehicle_Data.gear=IPData[4];//
			/*DC/DC״̬*/
			Vehicle_Data.dc_dc_state=IPData[6];//
			
			/*������־*/	
			Com_Warning_Data.warning_sign=( ((IPData[1]&0x07)<<16)|(IPData[1]<<8)|IPData[0] );
			
			break;
		}	
		case 0x18F82B9E:
		{
			//��߱����ȼ�
			//Com_Warning_Data.highest_warning_grade=IPData[0];//û��		
			break;
		}
		
/*�ɳ�索�ܵ�ѹ*/
		case 0x1818D0F3:
	  {		
			
//		/*�ɳ�索����ϵͳ��*/
//		Stored_Energy_sys_Data.stored_energy_sys_sn=IPData[1];
			
			/*�ܵ�ѹ*/
      Vehicle_Data.total_voltage=(((IPData[1]<<8)|IPData[0])-10000); //
			/*�ܵ���*/
			Vehicle_Data.total_current=DATA_Total_Current((IPData[3]<<8)|IPData[2]);//
			
			/*�ɳ��װ�õ�ѹ*/
			Stored_Energy_sys_Data.stored_energy_voltage=((IPData[1]<<8)|IPData[0])-10000;//
			
			/*�ɳ��װ�õ���*/
			Stored_Energy_sys_Data.stored_energy_current=((IPData[3]<<8)|IPData[2]);//
			
			/*SOC*/
			Vehicle_Data.soc=IPData[4]*2/5;//
			break;
	  }
/*�ɳ�索���¶�̽��*/
		case 0x181CD0F:
	  {
			/*����������*/
			Stored_Energy_sys_Data.single_battery_total_num=IPData[6];//
			/*�ɳ�索���¶�̽�����*/
			Stored_Energy_sys_Data.temperature_probe_num=IPData[7];//		
			break;
	  }
		/*�ɳ�索���¶��б�: ͨ����һID���Ľ���*/
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
		/*�ɳ�索�ܵ�ѹ�б�: ͨ����һID���Ľ���*/
		case 0x180528F3: 
	  {
			IsLPWR_Counter=0;//CAN��������
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



