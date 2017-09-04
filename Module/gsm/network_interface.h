


#include "stdbool.h"


#ifndef __NETWORK_INTERFACE_H__
#define __NETWORK_INTERFACE_H__



#define GSM_NETWORK_TYPE    0
#define GPRS_NETWORK_TYPE   1


/**
* @brief Network status
*/
typedef enum 
{
  /* Not registered, not searching a new operator to register to. */
  NETWORK_UNREGISTERED = 0,

  /* Registered, home network */
  NETWORK_REGISTERED,

  /* Not registered, but currently searching a new operator to register to */
  NETWORK_BUSY,

  /* Registration denied */
  NETWORK_DENIED,

  /* Unknown */
  NETWORK_UNKNOWN,

  /* Registered, roaming */
  NETWORK_ROAMING,
} NetworkStatus;




typedef struct {
	s32 index;
	ascii *status;
	ascii *name;
	ascii *nickname;
} NetworkProvider;









typedef struct NetworkInterface Network_IfTypedef;

/**
* @brief Network interface
*/
struct NetworkInterface {

  /**
  * @brief Provider's name
  */
  ascii *provider_name;

  /**
  * @brief Provider status
  */
  NetworkStatus network_status;



  /*********************Replies to methods*****************/

  /**
  * @brief Reply to "register_network" method call
  * @param ipc_data ipc data
  */
  void (*register_network_reply) ( void* ipc_data );

  /**
  * @brief Reply to "unregister" method call
  * @param ipc_data ipc data
  */
  void (*unregister_reply) ( void* ipc_data );

  /**
  * @brief Reply to "get_status" method call
  * @param ipc_data ipc data
  * @param status
  * @param strength
  */
  void (*get_status_reply) ( void* ipc_data, NetworkStatus status, u8 strength );

  /**
  * @brief Reply to "list_providers" method call
  * @param ipc_data ipc data
  * @param providers list of network providers
  */
  void (*list_providers_reply) ( void* ipc_data, ascii *providers[] );

  /**
  * @brief Reply to "register_with_provider" method call
  * @param ipc_data ipc data
  */
  void (*register_with_provider_reply) ( void* ipc_data );

  /**
  * @brief Reply to "Query signal strength" method call
  * @param ipc_data ipc data
  */
  void (*query_signal_strength_reply) ( void* ipc_data, u8 strength );



  /*********************Method errors**********************************/

  /**
  * @brief Error reply to "register_network" method call
  * @param ipc_data ipc data
  */
  void (*register_network_error) ( void* ipc_data );

  /**
  * @brief Error reply to "unregister" method call
  * @param ipc_data ipc data
  */
  void (*unregister_error) ( void* ipc_data );

  /**
  * @brief Error reply to "get_status" method call
  * @param ipc_data ipc data
  */
  void (*get_status_error) ( void* ipc_data );

  /**
  * @brief Error reply to "list_providers" method call
  * @param ipc_data ipc data
  */
  void (*list_providers_error) ( void* ipc_data );

  /**
  * @brief Error reply to "register_with_provider" method call
  * @param ipc_data ipc data
  */
  void (*register_with_provider_error) ( void* ipc_data );



  /*********************Signals**********************************/
  /**
  * @brief Status signal
  * @param provider_name
  * @param status
  * @param strength signal strength
  */
  void (*status) ( u8 type, NetworkStatus status );

  /**
  * @brief Subscriber numbers
  * @param number
  */
  void (*subscriber_numbers) ( const char **number );

                            
};


#define lgsm_network_register_network       NETWORK_Command_NetworkRegister
#define lgsm_network_get_status             NETWORK_Command_QueryGprsNetworkRegistration
#define lgsm_network_query_current_operator NETWORK_Command_QueryCurrentOperator
#define lgsm_network_query_signal_strength  NETWORK_Command_QuerySignalStrength
#define lgsm_network_list_providers         NETWORK_Command_QueryAvailableOperators


/******************************************/
/*              外部函数[声明]            */
/******************************************/

  /**
  * @brief Query current operator
  * @param ipc_data ipc data
  */
extern void lgsm_network_set_operator ( void* ipc_data, const ascii* operator, bool locked );

  /**
  * @brief Query current operator
  * @param ipc_data ipc data
  */
extern void lgsm_network_query_current_operator ( void* ipc_data );

  /**
  * @brief Query signal strength
  * @param ipc_data ipc data
  */
extern void lgsm_network_query_signal_strength ( void* ipc_data );

  /**
  * @brief Registers to network
  * @param ipc_data ipc data
  */
extern void lgsm_network_register_network ( void* ipc_data );

  /**
  * @brief Unregisters from network
  * @param ipc_data ipc data
  */
extern void lgsm_network_unregister ( void* ipc_data );

  /**
  * @brief Gets network status
  * @param ipc_data ipc data
  */
extern void lgsm_network_get_status ( void* ipc_data );

  /**
  * @brief Lists providers
  * @param ipc_data ipc data
  */
extern void lgsm_network_list_providers ( void* ipc_data );

  /**
  * @brief Registers with provider
  * @param ipc_data ipc data
  * @param index provider's index
  */
extern void lgsm_network_register_with_provider ( void* ipc_data, int index );


#endif	/* __NETWORK_INTERFACE_H__ */



