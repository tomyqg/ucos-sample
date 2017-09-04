






#ifndef __TZ_SIM_H__
#define __TZ_SIM_H__


#include "stm32f4xx.h"


#include "sim_interface.h"
#include "network_interface.h"









/******************************************/
/*              外部函数[声明]            */
/******************************************/

extern void sim_get_current_status ( SIM_Status status );
extern void sim_get_imei_code ( void* ipc_data, ascii *imei );
extern void sim_get_imsi_cnum ( void* ipc_data, ascii *imsi, ascii *cnum );
extern void sim_get_info_error ( void* ipc_data );
extern void sim_get_service_center_number ( void* ipc_data, const ascii *number );
extern void sim_set_sim_own_number_reply ( void* ipc_data );
extern void sim_set_sim_own_number_error ( void* ipc_data );
extern void network_query_signal_strength ( void* ipc_data, u8 strength );
extern void network_get_current_status ( u8 type, NetworkStatus status );
extern void GSM_StatusInfoDisplay ( void );


#endif  /* __TZ_SIM_H__ */

