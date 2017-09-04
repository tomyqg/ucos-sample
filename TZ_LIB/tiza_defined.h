




#ifndef __TZ_DEFINED_H__
#define __TZ_DEFINED_H__



/* ����汾�� */
#ifndef NT_SVN
#define NT_SVN             								"ZT3000-DF-V1.00"
#define SOFTWARE_VERSION     									(NT_SVN) 
#endif


/* Ӳ���汾�� */
#ifndef NT_HVN
#define NT_HVN             										"ZT3000-v1.2"
#define HARDWARE_EDITION     										(NT_HVN) 
#endif


/* �ͻ����� */
#ifndef CUSTOMER_NAME_STRING
#define CUSTOMER_NAME_STRING                    "XXXXXX"
#endif  /* CUSTOMER_NAME_STRING */




/* Ĭ�϶��ŷ������ĺ��� */
#ifndef TZ_DEFAULT_SMS_SERVICE_CENTER
#define TZ_DEFAULT_SMS_SERVICE_CENTER	          "+8615951766741"
#endif

/* Ĭ�϶������ĺ��� */
#ifndef TZ_DEFAULT_SMS_CENTER
#define TZ_DEFAULT_SMS_CENTER	                  "15951766741"
#endif

/* Ĭ��SIM��IMSI�� */
#ifndef TZ_DEFAULT_SIM_IMSICODE
#define TZ_DEFAULT_SIM_IMSICODE                 "XXXXXXXXXXXXXXX"
#endif

/* Ĭ��GPRS-APN */
#ifndef TZ_DEFAULT_GPRS_APN
#define TZ_DEFAULT_GPRS_APN	                    "CMNET"
#endif

/* Ĭ��GPRS�û��� */
#ifndef TZ_DEFAULT_GPRS_USR
#define TZ_DEFAULT_GPRS_USR	                    ""
#endif

/* Ĭ��GPRS���� */
#ifndef TZ_DEFAULT_GPRS_PWD
#define TZ_DEFAULT_GPRS_PWD	                    ""
#endif

/* Ĭ�Ϸ�������ַ1 */
#ifndef TZ_DEFAULT_PEER_ADDR1
#define TZ_DEFAULT_PEER_ADDR1	                  "218.94.153.146"
#endif

/* Ĭ�Ϸ�������ַ2 */
#ifndef TZ_DEFAULT_PEER_ADDR2
#define TZ_DEFAULT_PEER_ADDR2	                  "218.94.153.146"
#endif

/* Ĭ�Ϸ������˿ں� */
#ifndef TZ_DEFAULT_PEER_PORT
#define TZ_DEFAULT_PEER_PORT                    21000
#endif

/* Ĭ�Ϸ������˿ں� */
#ifndef TZ_DEFAULT_PEER_VIN
#define TZ_DEFAULT_PEER_VIN                    "A0000018061716611"
#endif


/* Ĭ�ϲ��Է�������ַ */
#ifndef TZ_DEFAULT_TEST_ADDR
#define TZ_DEFAULT_TEST_ADDR                    "218.94.153.146"
#endif

/* Ĭ�ϲ��Է������˿� */
#ifndef TZ_DEFAULT_TEST_PORT
#define TZ_DEFAULT_TEST_PORT                    4096
#endif

/* Ĭ�������� */
#ifndef TZ_DEFAULT_MS_DOMIAN
#define TZ_DEFAULT_MS_DOMIAN                    "www.baidu.cn"
#endif

/* Ĭ�ϴ����� */
#ifndef TZ_DEFAULT_SS_DOMIAN
#define TZ_DEFAULT_SS_DOMIAN                    "www.baidu.cn"
#endif

/* Ĭ��������������� */
#ifndef TZ_DEFAULT_SERVER_HEART_CHECK_INTERVAL
#define TZ_DEFAULT_SERVER_HEART_CHECK_INTERVAL  180		/* ��λ��s */
#endif

/* Ĭ���ն��ϴ�������� */
#ifndef TZ_DEFAULT_VEHICLE_HEART_INTERVAL
#define TZ_DEFAULT_VEHICLE_HEART_INTERVAL       30	   /* ��λ��s */
#endif  

/* Ĭ��׷�ټ�� */
#ifndef TZ_DEFAULT_GPRS_TRACK_INETRVAL
#define TZ_DEFAULT_GPRS_TRACK_INETRVAL          30			/* ��λ��s */
#endif

/* Ĭ��ä��������� */
#ifndef TZ_DEFAULT_GPRS_COMPENSATOR_INETRVAL
#define TZ_DEFAULT_GPRS_COMPENSATOR_INETRVAL    90    /* ��λ��s */
#endif

/* Ĭ����ʷ����ʱ��ͳ�Ƽ�� */
#ifndef TZ_DEFAULT_GPRS_ACC_STATS_INETRVAL
#define TZ_DEFAULT_GPRS_ACC_STATS_INETRVAL      45    /* ��λ��s */  
#endif

/* Ĭ��ACC���ϴ���������ʱ���� */
#ifndef TZ_DEFAULT_ACC_OPEN_TIME_INTERVAL
#define TZ_DEFAULT_ACC_OPEN_TIME_INTERVAL       30		/* ��λ��s */
#endif


/* Ĭ��ACC���ϴ���������ʱ���� */
#ifndef TZ_DEFAULT_ACC_CLOSE_TIME_INTERVAL
#define TZ_DEFAULT_ACC_CLOSE_TIME_INTERVAL      600		/* ��λ��s */
#endif


/* Ĭ�Ͻ�������״̬��ʱ���� */
#ifndef TZ_DEFAULT_SLEEP_TIME_INTERVAL
#define TZ_DEFAULT_SLEEP_TIME_INTERVAL          600		/* ��λ��s */
#endif


/* Ĭ���Ǳ�ͨ���жϱ���ʱ���� */
#ifndef TZ_DEFAULT_CTL_INTERRUPT_ALARM_INTERVAL
#define TZ_DEFAULT_CTL_INTERRUPT_ALARM_INTERVAL 120		/* ��λ��s */
#endif


/* Ĭ�ϵ͵�ѹ����ֵ */
#ifndef TZ_DEFAULT_LOW_POWER_VOLTAGE
#define TZ_DEFAULT_LOW_POWER_VOLTAGE            20	/* ��λ��v */
#endif


/* Ĭ��Э��汾�� */
#ifndef TZ_DEFAULT_PROTOCOL_VERSION              
#define TZ_DEFAULT_PROTOCOL_VERSION             4
#endif


/* Ĭ�ϳ��ұ�־ */
#ifndef TZ_DEFAULT_FACTORY_ID  
#define TZ_DEFAULT_FACTORY_ID                   0x26
#endif


/* ���������� */
#ifndef TZ_DEFAULT_CONTROLLER_TYPE 
#define TZ_DEFAULT_CONTROLLER_TYPE              ( TZ_CONTROLLER_TYPE_NONE )
#endif  


/* ���ĳ�ʱ�ش����� */
#ifndef TZ_MAX_TIME_OUT_COUNT
#define TZ_MAX_TIME_OUT_COUNT                   3
#endif


/* �ϴ�ʱ���ĸ��� */
#ifndef TZ_MAX_UPLOAD_TIME_COUNT
#define TZ_MAX_UPLOAD_TIME_COUNT                24
#endif


/* ʹ�ܹ���ʱ��ͳ�� */
#ifndef TZ_USE_ACC_WORK_STATS
#define TZ_USE_ACC_WORK_STATS                   1
#endif  


/* ����ʱ��ͳ����ʷ���� */
#ifndef TZ_MAX_WORK_LOG_SIZE
#define TZ_MAX_WORK_LOG_SIZE                    16
#endif


/* ʹ��ä�����ݲ��� */
#ifndef TZ_USE_STORAGE_MASS_DATA
#define TZ_USE_STORAGE_MASS_DATA                0
#endif


/* ʹ�ܶ�ʱ�������� */
#ifndef TZ_USE_FIX_TIME_RESET
#define TZ_USE_FIX_TIME_RESET                   1
#endif


/* ʹ�����ͳ�ƹ��� */  
#ifndef TZ_DISTANCE_STATISTICS
#define TZ_DISTANCE_STATISTICS                  1 
#endif


/* ʹ��ACCʱ���ۻ����� */
#ifndef TZ_USE_ACC_TIME_ACCUMULATE
#define TZ_USE_ACC_TIME_ACCUMULATE              1
#endif


#if TZ_USE_ACC_TIME_ACCUMULATE == 1 || TZ_DISTANCE_STATISTICS == 1
/* ʹ�ܶ�ʱ����ACC��Ϣ */
#ifndef TZ_USE_FIX_TIME_SAVE_ACC_INFO
#define TZ_USE_FIX_TIME_SAVE_ACC_INFO           1
#endif
#endif  /* TZ_USE_ACC_TIME_ACCUMULATE */


/* �ۼ�ʱ�䶨ʱ������ */
#ifndef ACC_TIME_SAVE_INTERVAL
#define ACC_TIME_SAVE_INTERVAL                  1800    /* ��λ���� */    
#endif



/* ���ض��ű������� */
#ifndef TZ_DEFAULT_SMS_ALARM_EN
#define TZ_DEFAULT_SMS_ALARM_EN                 0   /* �رն��ű��� */
#endif
 


/* �෭ */
#ifndef TZ_DEFAULT_VEHICLE_OVERTURN
#define TZ_DEFAULT_VEHICLE_OVERTURN             0  /* Ĭ��Ϊ0����ʾû�������෭���� */
												                            /* ��TZ_DEFAULT_VEHICLE_OVERTURN��Ϊ0ʱ����ʾ�෭�ж���ֵ����λΪ�� */
#endif



/* ͣ����ʱ */
#ifndef TZ_DEFAULT_STOP_CAR_TO
#define TZ_DEFAULT_STOP_CAR_TO                  0   /* Ĭ��Ϊ0����ʾû������ͣ����ʱ���� */
												                            /* ��TZ_DEFAULT_STOP_CAR_TO��Ϊ0ʱ����ʾͣ����ʱ��ֵ����λΪ���� */
#endif


/* ���ٱ�����ֵ */
#ifndef TZ_DEFAULT_OVER_SPEED
#define TZ_DEFAULT_OVER_SPEED                   0   /* Ĭ��Ϊ0����ʾû���������ٱ������� */
													                           /* ��TZ_DEFAULT_OVER_SPEED��Ϊ0ʱ����ʾ������ֵ����λΪkm/s */
#endif


/* Ĭ���ش����Դ��� */  
#ifndef TZ_DEFAULT_RETRY_MAX
#define TZ_DEFAULT_RETRY_MAX                    2
#endif


/* Ĭ�Ϲؼ�ָ���ط���� */
#ifndef TZ_DEFAULT_RETRY_INETRVAL         
#define TZ_DEFAULT_RETRY_INETRVAL					      8   /* ��λ��s */
#endif

/* �������ʱ���� */
#ifndef TZ_PHOTO_MIN_TIME_INTERVAL
#define TZ_PHOTO_MIN_TIME_INTERVAL              15  /* ��λ��s */
#endif


/* �����ֶγ��� */
#define __PHONE_LENGTH__                        32  /* ��λ��byte */

/* IP��ַ�ֶγ��� */
#define __SERVER_LENGTH__                       16  /* ��λ��byte */

/* �����ֶγ��� */
#define __DOMAIN_LENGTH__                       64  /* ��λ��byte */

/* APN�ֶγ��� */
#define __APN_LENGTH__                          32  /* ��λ��byte */

/* ��γ���ֶγ��� */
#define __LAT_LON_LENGTH__                      4  /* ��λ��byte */

/* ���θ߶��ֶγ��� */
#define __ALTI_LENGTH__                         2  /* ��λ��byte */

/* ʱ���ֶγ��� */
#define __TIME_LENGTH__                         6  /* ��λ��byte */

/* ״̬λ�ֶγ��� */
#define __STATUS_BIT_LENGTH__                   4  /* ��λ��byte */

/* Ĭ��SIM��IMSI���� */
#define __IMSICODE_LENGTH__                     20  /* ��λ��byte */

/* Ĭ��SIM���ų��� */
#define __SIM_NUMBER_LENGTH__                   12  /* ��λ��byte */

/* Ĭ������汾�ų��� */
#define __SVN_LENGTH__                          32  /* ��λ��byte */

/* Ĭ��Ӳ���汾�ų��� */
#define __HVN_LENGTH__                          32  /* ��λ��byte */

/* Ĭ�ϳ������Ƴ��� */
#define __FACTORY_ID_SIZE__                     32  /* ��λ��byte */  



/* ��¼��Ϣ���� */
#if defined TZ_MSG_LOGIN_LENGTH
#define __LOGIN_DEFAULT_LENGTH__                TZ_MSG_LOGIN_LENGTH
#else
#define __LOGIN_DEFAULT_LENGTH__                64  /* ��λ��byte */
#endif


/* ��Ӧ��Ϣ���� */
#if defined TZ_MSG_RESPONSE_LENGTH
#define __RESPONSE_DEFAULT_LENGTH__             TZ_MSG_RESPONSE_LENGTH
#else
#define __RESPONSE_DEFAULT_LENGTH__             512  /* ��λ��byte */
#endif


/* ��λ��Ϣ���� */
#if defined TZ_MSG_POSITION_LENGTH
#define __POSITION_DEFAULT_LENGTH__             TZ_MSG_POSITION_LENGTH
#else
#define __POSITION_DEFAULT_LENGTH__             96  /* ��λ��byte */
#endif



/* ������Ϣ���� */
#if defined TZ_MSG_WORK_PARAMS_LENGTH
#define __WORK_PARAMS_DEFAULT_LENGTH__          TZ_MSG_WORK_PARAMS_LENGTH
#else
#define __WORK_PARAMS_DEFAULT_LENGTH__          256  /* ��λ��byte */
#endif



/* �̼���Ϣ���� */
#if defined TZ_MSG_FIRMWARE_UPDATE_LENGTH
#define __FIRMWARE_UPDATE_DEFAULT_LENGTH__      TZ_MSG_FIRMWARE_UPDATE_LENGTH
#else
#define __FIRMWARE_UPDATE_DEFAULT_LENGTH__      32  /* ��λ��byte */
#endif



#endif  /* __TZ_DEFINED_H__ */




