

#include "stm32f4xx.h"
#include "tiza_rtc.h"

#ifndef __TZ_GPS_H__
#define __TZ_GPS_H__




#define PRINTF_GPS_TIME()       Printf_TimeInfoDisplay()    

#define  GPS_MAX_RESTART_COUNTER   120

extern s8 GPS_WorkState;
extern u16 GPS_WorkStateCounter;

//#define GPS_0183					  	//GPS��λϵͳ

#define BDS_Great_0183      	//������λϵͳ


/* RMC�ṹ */
typedef struct  {
	 
	/* ����_�� */
	u32		date_yy;
    	
  /* ����_�� */
	u32		date_mm;	
    	
	/* ����_�� */
	u32		date_dd;	

  /* ʱ��_ʱ */ 
  u32		time_hh;		
  
  /* ʱ��_�� */
  u32		time_mm;		
  
  /* ʱ��_�� */
  u32		time_ss;		
  
  /* ״̬ */
  u8		status_;		
  
  /* γ��_�����ʽ */
  float    fLat_;        
  
  /* γ�� */
  u32    	lat_;			
  
  /* �ϱ�γ */
  u8		lat_NS_;		
  
  /* ����_�����ʽ */
  float    fLong_;       
    
  /* ���� */
	u32   	long_;			
    
  /* ������ */
	u8		long_EW_;	
    
  /* �߶� */
  s16   high_;    	
     	
  /* ������ */
	u32   sat_num;

  /* �����*/
    u8   snr[13];
       
  /* �ٶ� */
  double       speed_;   
  
  /* ���� */
  double   direction_;   
} GPS_data;


/* GPS���� */
extern GPS_data RMC_d;






/* �������ں�ʱ����Ϣ */
typedef struct  
{  
  /* ����_�� */
  u8 date_yy;  
  
  /* ����_�� */
  u8 date_mm;    
                             
  /* ����_�� */
  u8 date_dd;   
  
  /* ʱ��_ʱ */ 
  s8 time_hh;   
         
  /* ʱ��_�� */
  s8 time_mm;
           
  /* ʱ��_�� */
  s8 time_ss;  
    
} localDateTime_t; 


typedef struct
{
	

	/* γ�� */
  u32    	lat_;

	/* ���� */
  u32    	long_;
	/*��γ���Ƿ���Ч*/
	bool isValid;

}gps_data_param_t;

extern gps_data_param_t  gps_data_param;



/******************************************/
/*              �ⲿ����[����]             */
/******************************************/

/* GPSӦ�ýӿڳ�ʼ�� */
extern void GPS_ApplicationIfInit ( void );

/* GPS���������� */
extern void GPS_ProcDataHdlr ( void );

/* GPS״̬��Ϣ��ʾ */
extern void GPS_StatusInfoDisplay ( void );

/* ʱ����Ϣ��ӡ */
extern void Printf_TimeInfoDisplay ( void );


#endif	/* __TZ_GPS_H__ */

