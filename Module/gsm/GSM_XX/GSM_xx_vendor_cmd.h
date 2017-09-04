



#ifndef __GSM_xx_VENDOR_CMD_H__
#define __GSM_xx_VENDOR_CMD_H__


#include "stm32f4xx.h"




typedef struct GL868VendorInterface {

  /**
  * @brief Reply to "list_gprs_classes" method
  *
  * @param modem pointer to modem struct
  * @param ipc_data ipc data
  */
  void (*init_before_sim_ready ) (void* ipc_data);

  /**
  * @brief Reply to "list_gprs_classes" method
  *
  * @param modem pointer to modem struct
  * @param ipc_data ipc data
  */
  void (*init_after_sim_ready ) (void* ipc_data);


  /*********************Replies to methods*****************/

  /* In this mode, the serial interface is shortly enabled while CTS is active. If char-
   * acters are recognized on the serial interface, the ME stays active for 2 seconds
   * after the last character was sent or received. */
  /**
  * @brief Reply to "list_gprs_classes" method
  *
  * @param modem pointer to modem struct
  * @param ipc_data ipc data
  */
  void (*cyclic_sleep_reply) (void* ipc_data);

  /**
  * @brief Reply to "Switch off mobile station" method
  *
  * @param modem pointer to modem struct
  * @param ipc_data ipc data
  */
  void (*switch_off_ms_reply) (void* ipc_data);

  /**
  * @brief Reply to "" method
  *
  * @param modem pointer to modem struct
  * @param ipc_data ipc data
  */
  void (*set_audio_mode_reply) (void* ipc_data);

  /**
  * @brief Reply to "" method
  *
  * @param modem pointer to modem struct
  * @param ipc_data ipc data
  */
  void (*get_audio_mode_reply) (void* ipc_data, u8 audmode);

  /**
  * @brief Reply to "" method
  *
  * @param modem pointer to modem struct
  * @param ipc_data ipc data
  */
  void (*set_loudspeaker_volume_reply) (void* ipc_data);

  /**
  * @brief Reply to "" method
  *
  * @param modem pointer to modem struct
  * @param ipc_data ipc data
  */
  void (*get_loudspeaker_volume_reply) (void* ipc_data, u8 vloume);


  /*********************Method errors**********************************/

  /**
  * @brief Error reply to "list_gprs_classes" method
  *
  * @param modem pointer to modem struct
  * @param ipc_data ipc data
  */
  void (*cyclic_sleep_error) (void* ipc_data,
                                   ascii *error);

  /**
  * @brief Error reply to "Switch off mobile station" method
  *
  * @param modem pointer to modem struct
  * @param ipc_data ipc data
  */
  void (*switch_off_ms_error) (void* ipc_data);

  /**
  * @brief Error reply to "" method
  *
  * @param modem pointer to modem struct
  * @param ipc_data ipc data
  */
  void (*set_audio_mode_error) (void* ipc_data);

  /**
  * @brief Error reply to "" method
  *
  * @param modem pointer to modem struct
  * @param ipc_data ipc data
  */
  void (*get_audio_mode_error) (void* ipc_data);

  /**
  * @brief Error reply to "" method
  *
  * @param modem pointer to modem struct
  * @param ipc_data ipc data
  */
  void (*set_loudspeaker_volume_error) (void* ipc_data);

  /**
  * @brief Error reply to "" method
  *
  * @param modem pointer to modem struct
  * @param ipc_data ipc data
  */
  void (*get_loudspeaker_volume_error) (void* ipc_data);


  /*********************Signals**********************************/
  /**
  * @brief Context activated ??
  *
  * @param modem pointer to modem struct
  * @param id ??
  */

}Vendor_IfTypedef;

//typedef struct GL868VendorInterface Vendor_IfTypedef;



#define lgsm_vendor_switch_off_mobile_station       VDR_Command_SwitchOffMobileStation
#define lgsm_vendor_use_sync_led                    VDR_Command_UseSyncLED
#define lgsm_vendor_get_loudspeaker_vloume          VDR_Command_GetLoudspeakerVolume
#define lgsm_vendor_set_loudspeaker_vloume          VDR_Command_SetLoudspeakerVolume
#define lgsm_vendor_get_audio_mode                  VDR_Command_GetAudioHardware
#define lgsm_vendor_set_audio_mode                  VDR_Command_SetAudioHardware





/******************************************/
/*              外部函数[声明]            */
/******************************************/


  /**
  * @brief ??
  * @param ipc_data ipc data
  * @param antenna_power ??
  */
extern void lgsm_vendor_use_sync_led ( void* ipc_data );


  /**
  * @brief ??
  * @param ipc_data ipc data
  * @param antenna_power ??
  */
extern void lgsm_vendor_switch_off_mobile_station ( void* ipc_data );



  /**
  * @brief ??
  * @param ipc_data ipc data
  * @param antenna_power ??
  */
extern void lgsm_vendor_set_audio_mode ( void* ipc_data, u8 audMode );


  /**
  * @brief ??
  * @param ipc_data ipc data
  * @param antenna_power ??
  */
extern void lgsm_vendor_get_audio_mode ( void* ipc_data );


  /**
  * @brief ??
  * @param ipc_data ipc data
  * @param antenna_power ??
  */
extern void lgsm_vendor_set_loudspeaker_vloume ( void* ipc_data, u8 outStep );


  /**
  * @brief ??
  * @param ipc_data ipc data
  * @param antenna_power ??
  */
extern void lgsm_vendor_get_loudspeaker_vloume ( void* ipc_data );


#endif  /* __MC52I_VENDOR_CMD_H__ */




