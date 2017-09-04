






#ifndef __PDP_INTERFACE_H__
#define __PDP_INTERFACE_H__

#include "stm32f4xx.h"



typedef struct PDPInterface PDP_IfTypedef;

struct PDPInterface {

  /*********************Replies to methods*****************/


  /**
  * @brief Reply to "list_gprs_classes" method
  *
  * @param modem pointer to modem struct
  * @param ipc_data ipc data
  */
  void (*attach_gprs_network_reply) (void* ipc_data);

  
  /**
  * @brief Reply to "list_gprs_classes" method
  *
  * @param modem pointer to modem struct
  * @param ipc_data ipc data
  */
  void (*detach_gprs_network_reply) (void* ipc_data);
  
  /**
   * @brief Reply to "list_gprs_classes" method
   *
   * @param modem pointer to modem struct
   * @param ipc_data ipc data
   */
   void (*gprs_context_activation) (void* ipc_data);
  
  /**
   * @brief Reply to "list_gprs_classes" method
   *
   * @param modem pointer to modem struct
   * @param ipc_data ipc data
   */
   void (*gprs_context_no_activation) (void* ipc_data);

   /**
   * @brief Reply to "list_gprs_classes" method
   *
   * @param modem pointer to modem struct
   * @param ipc_data ipc data
   */
   void (*socket_configuration_extend) (void* ipc_data);

	/**
	 * @brief Reply to "list_gprs_classes" method
	 *
	 * @param modem pointer to modem struct
	 * @param ipc_data ipc data
	 */
  void (*ppp_param_config) (void* ipc_data);

   	
   /**
	 * @brief Reply to "list_gprs_classes" method
	 *
	 * @param modem pointer to modem struct
	 * @param ipc_data ipc data
	 */
	 void (*socket_configuration) (void* ipc_data);

   /**
	 * @brief Reply to "list_gprs_classes" method
	 *
	 * @param modem pointer to modem struct
	 * @param ipc_data ipc data
	 */
   void (*socket_dial) (void* ipc_data);

    /**
	 * @brief Reply to "list_gprs_classes" method
	 *
	 * @param modem pointer to modem struct
	 * @param ipc_data ipc data
	 */
   void (*ftp_open) (void* ipc_data);

   /**
	 * @brief Reply to "list_gprs_classes" method
	 *
	 * @param modem pointer to modem struct
	 * @param ipc_data ipc data
	 */
    void (*ftp_close) (void* ipc_data);
    /**
	 * @brief Reply to "list_gprs_classes" method
	 *
	 * @param modem pointer to modem struct
	 * @param ipc_data ipc data
	 */
   void (*ftp_time_out) (void* ipc_data);
	/**
		* @brief Reply to "list_gprs_classes" method
		*
		* @param modem pointer to modem struct
		* @param ipc_data ipc data
		*/
   void (*ftp_get_filesize) (void* ipc_data);
     
    /**
	 * @brief Reply to "list_gprs_classes" method
	 *
	 * @param modem pointer to modem struct
	 * @param ipc_data ipc data
	 */
   void (*socket_shutdown) (void* ipc_data);

	
   void (*socket_restore) (void* ipc_data);

	 /**
	 * @brief Reply to "list_gprs_classes" method
	 *
	 * @param modem pointer to modem struct
	 * @param ipc_data ipc data
	 */
   void (*socket_inactive_timeout) (void* ipc_data);

	  /**
	 * @brief Reply to "list_gprs_classes" method
	 *
	 * @param modem pointer to modem struct
	 * @param ipc_data ipc data
	 */
   void (*socket_set) (void* ipc_data);

	  /**
		  * @brief Reply to "list_gprs_classes" method
		  *
		  * @param modem pointer to modem struct
		  * @param ipc_data ipc data
		  */
   void (*socket_open) (void* ipc_data);

	  /**
			   * @brief Reply to "list_gprs_classes" method
			   *
			   * @param modem pointer to modem struct
			   * @param ipc_data ipc data
			   */
  void (*selet_flow_control) (void* ipc_data);

	  
  /**
  * @brief Reply to "list_gprs_classes" method
  *
  * @param modem pointer to modem struct
  * @param ipc_data ipc data
  */
  void (*list_gprs_classes_reply) (void* ipc_data,
                                   const char **classes);

  /**
  * @brief Reply to "select_gprs_class" method
  *
  * @param modem pointer to modem struct
  * @param ipc_data ipc data
  */
  void (*select_gprs_class_reply) (void* ipc_data);

  /**
  * @brief Reply to "activate" method
  *
  * @param modem pointer to modem struct
  * @param ipc_data ipc data
  */
  void (*activate_reply) (void* ipc_data);

  /**
  * @brief Reply to "deactivate" method
  *
  * @param modem pointer to modem struct
  * @param ipc_data ipc data
  */
  void (*deactivate_reply) (void* ipc_data);

  /**
  * @brief Reply to "select_context" method
  *
  * @param modem pointer to modem struct
  * @param ipc_data ipc data
  */
  void (*select_context_reply) (void* ipc_data);

  /**
  * @brief Reply to "add_context" method
  *
  * @param modem pointer to modem struct
  * @param ipc_data ipc data
  */
  void (*add_context_reply) (void* ipc_data,
                             bool result);

  /**
  * @brief Reply to "delete_context" method
  *
  * @param modem pointer to modem struct
  * @param ipc_data ipc data
  */
  void (*delete_context_reply) (void* ipc_data);

  /**
  * @brief Reply to "list_contexts" method
  *
  * @param modem pointer to modem struct
  * @param ipc_data ipc data
  */
  void (*list_contexts_reply) (void* ipc_data,
                               const char **contexts);

  /**
  * @brief Reply to "list_contexts" method
  *
  * @param modem pointer to modem struct
  * @param ipc_data ipc data
  */
  void (*request_gprs_service_reply) (void* ipc_data,
                               const char **contexts);

  
  /**
	* @brief Reply to "list_contexts" method
	*
	* @param modem pointer to modem struct
	* @param ipc_data ipc data
	*/
  void (*pdp_context_act_config) (void* ipc_data,
							   bool result);
  

  /**
  * @brief Reply to "list_contexts" method
  *
  * @param modem pointer to modem struct
  * @param ipc_data ipc data
  */
   void (*pdp_context_activation) (void* ipc_data,
                             bool result);


   /**
  * @brief Reply to "list_contexts" method
  *
  * @param modem pointer to modem struct
  * @param ipc_data ipc data
  */
   void (*dns_response_cach) (void* ipc_data,
                             bool result);
  

  /**
   * @brief Reply to "list_contexts" method
   *
   * @param modem pointer to modem struct
   * @param ipc_data ipc data
   */
  void (*cfg_http_params) (void* ipc_data);
  
  /*********************Method errors**********************************/

  /**
  * @brief Error reply to "list_gprs_classes" method
  *
  * @param modem pointer to modem struct
  * @param ipc_data ipc data
  */
  void (*attach_gprs_network_error) (void* ipc_data,
                                   ascii *error);

  /**
  * @brief Error reply to "list_gprs_classes" method
  *
  * @param modem pointer to modem struct
  * @param ipc_data ipc data
  */
  void (*list_gprs_classes_error) (void* ipc_data,
                                   ascii *error);

  /**
  * @brief Error reply to "select_gprs_class" method
  *
  * @param modem pointer to modem struct
  * @param ipc_data ipc data
  */
  void (*select_gprs_class_error) (void* ipc_data,
                                   ascii *error);

  /**
  * @brief Error reply to "activate" method
  *
  * @param modem pointer to modem struct
  * @param ipc_data ipc data
  */
  void (*activate_error) (void* ipc_data,
                          ascii *error);

  /**
  * @brief Error reply to "deactivate" method
  *
  * @param modem pointer to modem struct
  * @param ipc_data ipc data
  */
  void (*deactivate_error) (void* ipc_data,
                            ascii *error);

  /**
  * @brief Error reply to "select_context" method
  *
  * @param modem pointer to modem struct
  * @param ipc_data ipc data
  */
  void (*select_context_error) (void* ipc_data,
                                ascii *error);

  /**
  * @brief Error reply to "add_context" method
  *
  * @param modem pointer to modem struct
  * @param ipc_data ipc data
  */
  void (*add_context_error) (void* ipc_data,
                             ascii *error);

  /**
  * @brief Error reply to "delete_context" method
  *
  * @param modem pointer to modem struct
  * @param ipc_data ipc data
  */
  void (*delete_context_error) (void* ipc_data,
                                ascii *error);

  /**
  * @brief Error reply to "list_contexts" method
  *
  * @param modem pointer to modem struct
  * @param ipc_data ipc data
  */
  void (*list_contexts_error) (void* ipc_data,
                               ascii *error);

  /**
  * @brief Error reply to "list_contexts" method
  *
  * @param modem pointer to modem struct
  * @param ipc_data ipc data
  */
  void (*request_gprs_service_error) (void* ipc_data,
                               ascii *error);


  /*********************Signals**********************************/
  /**
  * @brief Context activated ??
  *
  * @param modem pointer to modem struct
  * @param id ??
  */
  void (*context_activated) (void* ipc_data, int id);

  /**
  * @brief Context deactivated ??
  *
  * @param modem pointer to modem struct
  * @param id ??
  */
  void (*context_deactivated) (void* ipc_data, int id);

  /**
  * @brief Context changed ??
  *
  * @param modem pointer to modem struct
  * @param id ??
  */
  void (*context_changed) (void* ipc_data, int id);

};






#define lgsm_pdp_attach_gprs_network       PDP_Command_GprsAttach
#define lgsm_gl868_gprs_context_activation Gl868_Command_GprsContextActivation
#define lgsm_pdp_list_gprs_classes         PDP_Command_ListGprsClasses
#define lgsm_pdp_select_gprs_class         PDP_Command_SelectGprsClass
#define lgsm_pdp_activate                  PDP_Command_Activate
#define lgsm_pdp_deactivate                PDP_Command_Deactivate
#define lgsm_pdp_select_context            PDP_Command_SelectContext

#define lgsm_pdp_add_context               PDP_Command_AddContext
#define gl868_lgsm_pdp_add_context         GL868_PDP_Command_AddContext
#define ctcc_pdp_add_context 		  	 			 CTCC_PDP_Command_AddContext

#define lgsm_pdp_delete_context            PDP_Command_DeleteContext
#define lgsm_pdp_list_contexts             PDP_Command_ListContexts
#define lgsm_pdp_request_gprs_service      PDP_Command_RequestGPRSservice








/******************************************/
/*              外部函数[声明]            */
/******************************************/

/***************************Method calls*******************************/


/**
* @brief Lists gprs classes
*
* @param modem pointer to modem struct
* @param ipc_data ipc data
*/
extern void lgsm_pdp_attach_gprs_network (void* ipc_data);


/**
* @brief Lists gprs classes
*
* @param modem pointer to modem struct
* @param ipc_data ipc data
*/
extern void lgsm_gl868_gprs_context_activation (void* ipc_data);


/**
* @brief Lists gprs classes
*
* @param modem pointer to modem struct
* @param ipc_data ipc data
*/
extern void lgsm_pdp_list_gprs_classes (void* ipc_data);

/**
* @brief Selects gprs class
*
* @param modem pointer to modem struct
* @param ipc_data ipc data
*/
extern void lgsm_pdp_select_gprs_class (void* ipc_data,
                           const char *class);

/**
* @brief Activates ??
*
* @param modem pointer to modem struct
* @param ipc_data ipc data
*/
extern void lgsm_pdp_activate (void* ipc_data,
                  int index);

/**
* @brief Deactivates ??
*
* @param modem pointer to modem struct
* @param ipc_data ipc data
*/
extern void lgsm_pdp_deactivate (void* ipc_data,
                    int index);

/**
* @brief Selects context ??
*
* @param modem pointer to modem struct
* @param ipc_data ipc data
*/
extern void lgsm_pdp_select_context (void* ipc_data,
                        const char *context);

/**
* @brief Adds context ??
*
* @param modem pointer to modem struct
* @param ipc_data ipc data
*/
extern void lgsm_pdp_add_context (void* ipc_data,
                                  int index, const char *APN);

/*电信专用*/
extern void ctcc_pdp_add_context (void* ipc_data ,
																	const char *UserName, 
																	const char *Password);
/**
* @brief Deletes context ??
*
* @param modem pointer to modem struct
* @param ipc_data ipc data
*/
extern void lgsm_pdp_delete_context (void* ipc_data,
                        int index);

/**
* @brief Lists contexts ??
*
* @param modem pointer to modem struct
* @param ipc_data ipc data
*/
extern void lgsm_pdp_list_contexts (void* ipc_data);



/**
* @brief Lists contexts ??
*
* @param modem pointer to modem struct
* @param ipc_data ipc data
*/
extern void lgsm_pdp_request_gprs_service (void* ipc_data, int index);



#endif  /* __PDP_INTERFACE_H__ */



