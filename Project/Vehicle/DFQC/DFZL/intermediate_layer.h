#ifndef  _INTERMEDIATE_LAYER_H_
#define  _INTERMEDIATE_LAYER_H_



/*************************************************************************
*
*��λת��ģʽ��
*
*   32960��� <--------�м��<--------CAN����
*   ����������        ����������     ����������
***************************************************************************
*/
/* �������� */
/*********************************************************************/
//����״̬	  
//���״̬
//���״̬��־λ	  
//����ģʽ	
//����
#define  	VehicleSpeed_Factor_can    						5		
#define  	VehicleSpeed_Offset_can    						0

#define  	VehicleSpeed_Factor_gb    						1		
#define  	VehicleSpeed_Offset_gb    						0

#define  	VehicleSpeed_Factor    						(VehicleSpeed_Factor_can/VehicleSpeed_Factor_gb)		
#define  	VehicleSpeed_Offset    						((VehicleSpeed_Offset_can-VehicleSpeed_Offset_gb)/VehicleSpeed_Factor_gb)
#define 	DATA_VehicleSpeed(_X_)    				(( u16 )( ( (_X_) * VehicleSpeed_Factor ) + VehicleSpeed_Offset ))
//�ۼ����	
#define  	Accumulated_Distance_Factor_can    		1//0.1		
#define  	Accumulated_Distance_Offset_can    		0

#define  	Accumulated_Distance_Factor_gb    		1//0.1		
#define  	Accumulated_Distance_Offset_gb    		0

#define  	Accumulated_Distance_Factor    		(Accumulated_Distance_Factor_can/Accumulated_Distance_Factor_gb)		
#define  	Accumulated_Distance_Offset    		((Accumulated_Distance_Offset_can-Accumulated_Distance_Offset_gb)/Accumulated_Distance_Factor_gb)
#define 	DATA_Accumulated_Distance(_X_)    ( u32 )( ( (_X_) * Accumulated_Distance_Factor ) + Accumulated_Distance_Offset )
//�ܵ�ѹ	
#define  	Total_Voltage_Factor_can    					1//0.1		
#define  	Total_Voltage_Offset_can    					(-1000)

#define  	Total_Voltage_Factor_gb    					  1//0.1		
#define  	Total_Voltage_Offset_gb    					  0

#define  	Total_Voltage_Factor    					(Total_Voltage_Factor_can/Total_Voltage_Factor_gb)		
#define  	Total_Voltage_Offset    					((Total_Voltage_Offset_can-Total_Voltage_Offset_gb)/Total_Voltage_Factor_gb)
#define 	DATA_Total_Voltage(_X_)    				 ( u16 )(( (_X_) * Total_Voltage_Factor ) + Total_Voltage_Offset )
//�ܵ���	
#define  	Total_Current_Factor_can    				  1//0.1		
#define  	Total_Current_Offset_can    					(-1000)

#define  	Total_Current_Factor_gb    					  1//0.1		
#define  	Total_Current_Offset_gb    					  (-1000)


#define  	Total_Current_Factor    					(Total_Current_Factor_can/Total_Current_Factor_gb)		
#define  	Total_Current_Offset    					((Total_Current_Offset_gb-Total_Current_Offset_can)/Total_Current_Factor_gb)
#define 	DATA_Total_Current(_X_)    				( u16 )(( (_X_) * Total_Current_Factor ) + Total_Current_Offset )
//SOC
//DC-DC״̬
//��λ
//��Ե�����

#define  	IR_Factor_can    											1	
#define  	IR_Offset_can    											0

#define  	IR_Factor_gb    											1		
#define  	IR_Offset_gb    											0

#define  	IR_Factor    											(IR_Factor_can/IR_Factor_gb)		
#define  	IR_Offset    											((IR_Offset_can-IR_Offset_gb)/IR_Factor_gb)
#define 	DATA_IR(_X_)    									( ( u16 )( (_X_) * IR_Factor ) + IR_Offset )
//����̤���г�ֵ	 
//�ƶ�̤��״̬ 	 
/*********************************************************************/				


/* ����������� */
/*********************************************************************/
//����������� 	  
//����������
//�������״̬	  
//��������������¶�
#define  	Motor_Ctrl_Temp_Factor_can    						1		
#define  	Motor_Ctrl_Temp_Offset_can    						(-40)

#define  	Motor_Ctrl_Temp_Factor_gb    							1		
#define  	Motor_Ctrl_Temp_Offset_gb    							(-40)

#define  	Motor_Ctrl_Temp_Factor    						(Motor_Ctrl_Temp_Factor_can/Motor_Ctrl_Temp_Factor_gb)		
#define  	Motor_Ctrl_Temp_Offset    						((Motor_Ctrl_Temp_Offset_can-Motor_Ctrl_Temp_Offset_gb)/Motor_Ctrl_Temp_Factor_gb)
#define 	DATA_Motor_Ctrl_Temp(_X_)    					(( u8 )( (_X_) * Motor_Ctrl_Temp_Factor ) + Motor_Ctrl_Temp_Offset )
//�������ת��
#define  	Motor_Speed_Factor_can    								1		
#define  	Motor_Speed_Offset_can    								(0)

#define  	Motor_Speed_Factor_gb    								  2		
#define  	Motor_Speed_Offset_gb    								  (-2000)

#define  	Motor_Speed_Factor    								(Motor_Speed_Factor_can/Motor_Speed_Factor_gb)		
#define  	Motor_Speed_Offset    								((Motor_Speed_Offset_can-Motor_Speed_Offset_gb)/Motor_Speed_Factor_gb)
#define 	DATA_Motor_Speed(_X_)    							(( u16 )( (_X_) * Motor_Speed_Factor ) + Motor_Speed_Offset )
//�������ת��
#define  	Motor_Torque_Factor_can    								10//0.1		
#define  	Motor_Torque_Offset_can    								(-3200)

#define  	Motor_Torque_Factor_gb   								  1//0.1		
#define  	Motor_Torque_Offset_gb    								(-2000)

#define  	Motor_Torque_Factor    								(Motor_Torque_Factor_can/Motor_Torque_Factor_gb)	
#define  	Motor_Torque_Offset    								((Motor_Torque_Offset_can-Motor_Torque_Offset_gb)/Motor_Torque_Factor_gb)
#define 	DATA_Motor_Torque(_X_)    						(( u16 )( (_X_) * Motor_Torque_Factor ) + Motor_Torque_Offset )
//��������¶�
#define  	Motor_Temp_Factor_can    									1		
#define  	Motor_Temp_Offset_can    									(-40)

#define  	Motor_Temp_Factor_gb    									1		
#define  	Motor_Temp_Offset_gb    									(-40)

#define  	Motor_Temp_Factor    									(Motor_Temp_Factor_can/Motor_Temp_Factor_gb)		
#define  	Motor_Temp_Offset    									((Motor_Temp_Offset_can-Motor_Temp_Offset_gb)/Motor_Temp_Factor_gb)
#define 	DATA_Motor_Temp(_X_)    							(( u8 )( (_X_) * Motor_Temp_Factor ) + Motor_Temp_Offset )
//��������������ѹ
#define  	Motor_Ctrl_Voltage_Factor_can    					1//0.1		
#define  	Motor_Ctrl_Voltage_Offset_can    					(-1000)

#define  	Motor_Ctrl_Voltage_Factor_gb    					1//0.1		
#define  	Motor_Ctrl_Voltage_Offset_gb    					0

#define  	Motor_Ctrl_Voltage_Factor    					(Motor_Ctrl_Voltage_Factor_can/Motor_Ctrl_Voltage_Factor_gb)		
#define  	Motor_Ctrl_Voltage_Offset    					((Motor_Ctrl_Voltage_Offset_can-Motor_Ctrl_Voltage_Offset_gb)/Motor_Ctrl_Voltage_Factor_gb)
#define 	DATA_Motor_Ctrl_Voltage(_X_)    			(( u16 )( (_X_) * Motor_Ctrl_Voltage_Factor ) + Motor_Ctrl_Voltage_Offset )
//���������ֱ��ĸ�ߵ���

#define  	Motor_Ctrl_Current_Factor_can    					1//0.1		
#define  	Motor_Ctrl_Current_Offset_can    					(-1000)

#define  	Motor_Ctrl_Current_Factor_gb    					1//0.1		
#define  	Motor_Ctrl_Current_Offset_gb    					(-1000)
#define  	Motor_Ctrl_Current_Factor    					(Motor_Ctrl_Current_Factor_can/Motor_Ctrl_Current_Factor_gb)		
#define  	Motor_Ctrl_Current_Offset    					((Motor_Ctrl_Current_Offset_can-Motor_Ctrl_Current_Offset_gb)/Motor_Ctrl_Current_Factor_gb)
#define 	DATA_Motor_Ctrl_Current(_X_)    			(( u16 )( (_X_) * Motor_Ctrl_Current_Factor ) + Motor_Ctrl_Current_Offset )
/*********************************************************************/				


/*��ֵ����*/
/*********************************************************************/
//��ߵ�ѹ�����ϵͳ��
//��ߵ�ѹ��ص������
//��ص����ѹ���ֵ

#define  	SingleBattery_HighestVoltage_Factor_can    						1//0.001		
#define  	SingleBattery_HighestVoltage_Offset_can    						(-10000)

#define  	SingleBattery_HighestVoltage_Factor_gb    						1//0.001		
#define  	SingleBattery_HighestVoltage_Offset_gb   						  0

#define   MU_Factor_SB_HV								                        10

#define  	SingleBattery_HighestVoltage_Factor    						(SingleBattery_HighestVoltage_Factor_can/SingleBattery_HighestVoltage_Factor_gb)		
#define  	SingleBattery_HighestVoltage_Offset    						((SingleBattery_HighestVoltage_Offset_can-SingleBattery_HighestVoltage_Offset_gb)/SingleBattery_HighestVoltage_Factor_gb*MU_Factor_SB_HV)
#define 	DATA_SingleBattery_HighestVoltage(_X_)    				(( u16 )( (_X_) * SingleBattery_HighestVoltage_Factor ) + SingleBattery_HighestVoltage_Offset )   

//��͵�ѹ�����ϵͳ��
//��͵�ѹ��ص������
//��ص����ѹ���ֵ
#define  	SingleBattery_LowestVoltage_Factor_can    						1//0.001		
#define  	SingleBattery_LowestVoltage_Offset_can    						(-10000)

#define  	SingleBattery_LowestVoltage_Factor_gb   						  1//0.001		
#define  	SingleBattery_LowestVoltage_Offset_gb    						  0

#define   MU_Factor_SingleBattery_LowestVoltage									10

#define  	SingleBattery_LowestVoltage_Factor    						(SingleBattery_LowestVoltage_Factor_can/SingleBattery_LowestVoltage_Factor_gb)		
#define  	SingleBattery_LowestVoltage_Offset    						((SingleBattery_LowestVoltage_Offset_can-SingleBattery_LowestVoltage_Offset_gb)/SingleBattery_LowestVoltage_Factor_gb*MU_Factor_SingleBattery_LowestVoltage)
#define 	DATA_SingleBattery_LowestVoltage(_X_)    					(( u16 )( (_X_) * SingleBattery_LowestVoltage_Factor ) + SingleBattery_LowestVoltage_Offset )   
			
//����¶���ϵͳ��
//����¶�̽�����   
//�¶�̽������¶�ֵ
#define  	Probe_HighestTemperature_Factor_can    								1		
#define  	Probe_HighestTemperature_Offset_can    								(-40)

#define  	Probe_HighestTemperature_Factor_gb    								1		
#define  	Probe_HighestTemperature_Offset_gb    								(-40)
#define  	Probe_HighestTemperature_Factor    								(Probe_HighestTemperature_Factor_can/Probe_HighestTemperature_Factor_gb)		
#define  	Probe_HighestTemperature_Offset    								((Probe_HighestTemperature_Offset_can-Probe_HighestTemperature_Offset_gb)/Probe_HighestTemperature_Factor_gb)
#define 	DATA_Probe_HighestTemperature(_X_)    						(( u8 )( (_X_) * Probe_HighestTemperature_Factor ) + Probe_HighestTemperature_Offset )   
		
//����¶���ϵͳ��
//����¶�̽�����	
//����¶�ֵ		
#define  	Probe_LowestTemperature_Factor_can    						1		
#define  	Probe_LowestTemperature_Offset_can    						(-40)

#define  	Probe_LowestTemperature_Factor_gb    							1		
#define  	Probe_LowestTemperature_Offset_gb    							(-40)
#define  	Probe_LowestTemperature_Factor    								(Probe_LowestTemperature_Factor_can/Probe_LowestTemperature_Factor_gb)		
#define  	Probe_LowestTemperature_Offset    								((Probe_LowestTemperature_Offset_can-Probe_LowestTemperature_Offset_gb)/Probe_LowestTemperature_Factor_gb)
#define 	DATA_Probe_LowestTemperature(_X_)    							(( u8 )( (_X_) * Probe_LowestTemperature_Factor ) + Probe_LowestTemperature_Offset )   	

/*********************************************************************/



 

/*�ɳ�索�ܵ�ѹ�б�*/

#define LIST_Voltage_START  			0X1801D1F3

#define LIST_Voltage_END    			0X1819D1F3

/*��������*/
#define SINGLE_TOTAL_number       0


//����¶���ϵͳ��
//����¶�̽�����	
//����¶�ֵ	

#define  	StoredEnergy_Voltage_Factor_can    					1//0.1		
#define  	StoredEnergy_Voltage_Offset_can    					(-1000)

#define  	StoredEnergy_Voltage_Factor_gb    					1//0.1		
#define  	StoredEnergy_Voltage_Offset_gb    					(-1000)

#define  	StoredEnergy_Voltage_Factor    					(StoredEnergy_Voltage_Factor_can/StoredEnergy_Voltage_Factor_gb)		
#define  	StoredEnergy_Voltage_Offset    					((StoredEnergy_Voltage_Offset_can-StoredEnergy_Voltage_Offset_gb)/StoredEnergy_Voltage_Factor_gb)
#define 	DATA_StoredEnergyVoltage(_X_)    			(( u16 )( (_X_) * StoredEnergy_Voltage_Factor ) + StoredEnergy_Voltage_Offset )



 
/*̽������*/
#define PROBE_TOTAL_number        0

//ÿ֡�¶�̽����
#define OneFrameProbe             8

/*�ɳ�索���¶��б�*/

#define LIST_Temperature_START 		0X183DD1F3

#define LIST_temperature_END      0X183FD1F3



#define  	StoredEnergy_Current_Factor_can    					1//0.1		
#define  	StoredEnergy_Current_Offset_can    					(-1000)

#define  	StoredEnergy_Current_Factor_gb    					1//0.1		
#define  	StoredEnergy_Current_Offset_gb    					(-1000)
#define  	StoredEnergy_Current_Factor    					(StoredEnergy_Current_Factor_can/StoredEnergy_Current_Factor_gb)		
#define  	StoredEnergy_Current_Offset    					((StoredEnergy_Current_Offset_can-StoredEnergy_Current_Offset_gb)/StoredEnergy_Current_Factor_gb)
#define 	DATA_StoredEnergyCurrent(_X_)    			(( u16 )( (_X_) * StoredEnergy_Current_Factor ) + StoredEnergy_Current_Offset )
/*********************************************************************/			














#endif  /*INTERMEDIATE_LAYER_H*/


