




#ifndef __DEVICE_INTERFACE_H__
#define __DEVICE_INTERFACE_H__



#define lgsm_get_info             Device_Command_GetInfo
#define lgsm_query_imei           Device_Command_QueryIMEI
#define lgsm_set_antenna_power    Device_Command_SetAntennaPower
#define lgsm_get_antenna_power    Device_Command_GetAntennaPower


/**
* @brief Device interface
*/
typedef struct DeviceInterface Device_IfTypedef;

struct DeviceInterface {


  /*********************Replies to methods*****************/

  /**
  * @brief Reply to "get service bearer" method call
  * @param ipc_data ipc data
  * @param info hash table containing device information
  */
  void (*get_info_reply) ( void* ipc_data, ascii **info );

	 /**
  * @brief Reply to "get service bearer" method call
  * @param ipc_data ipc data
  * @param info hash table containing device information
  */
  void (*get_imei_reply) ( void* ipc_data, ascii *imei );

  /**
  * @brief Reply to "set antenna power" method call
  * @param ipc_data ipc data
  */
  void (*set_antenna_power_reply) ( void* ipc_data );

  /**
  * @brief Reply to "get antenna power" method call
  * @param ipc_data ipc data
  */
  void (*get_antenna_power_reply) ( void* ipc_data, bool antenna_power );


  /*********************Method errors**********************************/

  /**
  * @brief Error reply to "get info" method call
  * @param ipc_data ipc data
  */
  void (*get_info_error) ( void* ipc_data );

  /**
  * @brief Error reply to "set antenna power" method call
  *
  * @param modem pointer to modem struct
  * @param ipc_data ipc data
  */
  void (*set_antenna_power_error) ( void* ipc_data );

  /**
  * @brief Error reply to "get antenna power" method call
  * @param ipc_data ipc data
  */
  void (*get_antenna_power_error) ( void* ipc_data );

};




/******************************************/
/*              外部函数[声明]            */
/******************************************/

  /**
  * @brief ??
  * @param ipc_data ipc data
  * @param antenna_power ??
  */
extern void lgsm_get_date_time ( void* ipc_data );

/* 
@param date_time
ASCII string of format:
yy/MM/dd,hh:mm:ss±zz
or
yy/MM/dd,hh:mm:ss
yy - 2-digit year [2000-2069]
MM - 2-digit month [01-12]
dd - 2-digit day of month [00-31]
hh - 2-digit hour [00-23]
mm - 2-digit minute [00-59]
ss - 2-digit seconds [00-59]
zz - (optional) time zone offset from GMT, in quarter-hours [-47...+48]. If this value is 
not specified, the time zone offset will be 0.*/
extern void lgsm_set_date_time ( void* ipc_data, const char* date_time );

  /**
  * @brief Gets information on gsm device
  * @param ipc_data ipc data
  */
extern void lgsm_get_info ( void* ipc_data );


  /**
  * @brief Gets information on gsm device
  * @param ipc_data ipc data
  */
extern void lgsm_query_imei ( void* ipc_data );

  /**
  * @brief ??
  * @param ipc_data ipc data
  * @param antenna_power ??
  */
extern void lgsm_set_antenna_power ( void* ipc_data, bool antenna_power );

  /**
  * @brief ??
  * @param ipc_data ipc data
  * @param antenna_power ??
  */
extern void lgsm_get_antenna_power ( void* ipc_data );




#endif	/* __DEVICE_INTERFACE_H__ */


