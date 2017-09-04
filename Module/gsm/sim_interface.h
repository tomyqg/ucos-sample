






                                                 
#ifndef __SIM_INTERFACE_H__
#define __SIM_INTERFACE_H__






/**
* @brief Call status
*/
typedef enum 
{
  /* PIN status hasn't been checked yet */
  SIM_UNKNOWN=0,      

  /* init sim card */
  SIM_INIT, 

  /* Error reading sim card */
  SIM_ERROR,          

  /* SIM card is missing */
  SIM_MISSING,        

  /* PIN code needs to be entered */
  SIM_NEED_PIN,       

  /* PUK code needs to be entered */
  SIM_NEED_PUK,       

  /* Get RING with CLIP */
  SIM_READY,          
} SIM_Status;





#define lgsm_sim_set_sim_own_number           SIM_Command_SetSIMOwnNumber
#define lgsm_sim_get_sim_info                 SIM_Command_GetSIMinfo
#define lgsm_sim_get_imsi                     SIM_Command_GetImsi
#define lgsm_sim_get_auth_status              SIM_Command_QueryPINstatus
#define lgsm_sim_retrieve_message             SIM_Command_Read
#define lgsm_sim_list_preferred_message       SIM_Command_List
#define lgsm_sim_delete_message               SIM_Command_Delete
#define lgsm_sim_delete_all_messages          SIM_Command_DeleteAllMessages
#define lgsm_sim_set_service_center_number    SIM_Command_SetServiceCenter
#define lgsm_sim_get_service_center_number    SIM_Command_QueryServiceCenter
#define lgsm_sim_select_phone_book_memory     SIM_Command_SelectPhoneBookMemory




typedef struct SIMInterface SIM_IfTypedef;

/**
 * @brief SIM interface
 */
struct SIMInterface
{

  /**************************Replies to method calls*************************/

  /**
   * @brief Reply to "set_sim_own_number" method call
   * @param ipc_data data for ipc
   */
  void (*set_sim_own_number_reply) ( void* ipc_data);

  /**
    * @brief Reply to "get_auth_status" method call
    * @param ipc_data data for ipc
    * @param status authorization status
    */
  void (*get_auth_status_reply) ( void* ipc_data,
                                 const ascii *status);

  /**
   * @brief Reply to "send_auth_code" method call
   * @param ipc_data data for ipc
   */
  void (*send_auth_code_reply) ( void* ipc_data);

  /**
   * @brief Reply to "unlock" method call
   * @param ipc_data data for ipc
   */
  void (*unlock_reply) ( void* ipc_data);

  /**
   * @brief Reply to "change_auth_code" method call
   * @param ipc_data data for ipc
   */
  void (*change_auth_code_reply) ( void* ipc_data);

  /**
   * @brief Reply to "get_sim_info" method call
   * @param ipc_data data for ipc
   * @param info information hash table
   */
  void (*get_sim_info_reply) ( void* ipc_data,
                              ascii *imsi);

  /**
  * @brief Reply to "get service center number" method call
  * @param ipc_data ipc data
  * @param number service center number
  */
  void (*get_service_center_number_reply) ( void* ipc_data,
                                           const ascii *number);

  /**
  * @brief Reply to "set service center number" method call
  *
  * @param modem pointer to modem struct
  * @param ipc_data ipc data
  */
  void (*set_service_center_number_reply) ( void* ipc_data);


  /**
  * @brief Reply to "store a message" method call
  * @param ipc_data ipc data
  * @param index stored message's index
  */
  void (*store_message_reply) ( void* ipc_data,
                               const int index);

  /**
  * @brief Reply to "List SMS messages from preferred store" method call
  * @param ipc_data ipc data
  * @param sender_number message sender's number
  * @param content messages contents
  */
  void (*list_preferred_message_reply) ( void* ipc_data,
                                        s16 msgindex,
                                        const ascii *sender_number,
                                        const ascii *contents);

  /**
  * @brief Reply to "retrieve message" method call
  * @param ipc_data ipc data
  * @param sender_number message sender's number
  * @param content messages contents
  */
  void (*retrieve_message_reply) ( void* ipc_data,
                                  const ascii *sender_number,
                                  const ascii *contents);

  /**
  * @brief Reply to "delete entry" method call
  * @param ipc_data ipc data
  */
  void (*delete_message_reply) ( void* ipc_data);

  /**
  * @brief Reply to "delete entry" method call
  * @param ipc_data ipc data
  */
  void (*delete_all_messages_reply) ( void* ipc_data);

  /**
  * @brief Reply to "send messages" method call
  * @param ipc_data ipc data
  */
  void (*send_stored_message_reply) ( void* ipc_data,
                                     s32 transaction_index);

  /**
  * @brief Reply to "send generic sim command" method call
  * @param ipc_data ipc data
  * @param result The result of the command
  */
  void (*send_generic_sim_command_reply) ( void* ipc_data,
                                          const ascii *result);


  /*********************Error replies to method calls********************/

  /**
   * @brief Error reply to "unlock" method call
   * @param ipc_data data for ipc
   */
  void (*set_sim_own_number_error) ( void* ipc_data );

  /**
  * @brief Error reply to "get service center number" method call
  * @param ipc_data ipc data
  */
  void (*get_service_center_number_error) ( void* ipc_data );

  /**
  * @brief Error reply to "set service center number" method call
  * @param ipc_data ipc data
  */
  void (*set_service_center_number_error) ( void* ipc_data );

  /**
   * @brief Error reply to "get_auth_status" method call
   * @param ipc_data data for ipc
   */
  void (*get_auth_status_error) ( void* ipc_data );


  /**
   * @brief Error reply to "send_auth_code" method call
   * @param ipc_data data for ipc
   */
  void (*send_auth_code_error) ( void* ipc_data );

  /**
   * @brief Error reply to "unlock" method call
   * @param ipc_data data for ipc
   */
  void (*unlock_error) ( void* ipc_data );

  /**
   * @brief Error reply to "change_auth_code" method call
   * @param ipc_data data for ipc
   */
  void (*change_auth_code_error) ( void* ipc_data );

  /**
   * @brief Error reply to "get_sim_info" method call
   * @param ipc_data data for ipc
   */
  void (*get_sim_info_error) ( void* ipc_data );

  /**
  * @brief Error reply to "store a message" method call
  * @param ipc_data ipc data
  */
  void (*store_message_error) ( void* ipc_data );

  /**
  * @brief Error reply to "retrieve message" method call
  * @param ipc_data ipc data
  */
  void (*retrieve_message_error) ( void* ipc_data );

  /**
  * @brief Error reply to "delete entry" method call
  * @param ipc_data ipc data
  */
  void (*delete_message_error) ( void* ipc_data );

  /**
  * @brief Error reply to "delete entry" method call
  * @param ipc_data ipc data
  */
  void (*delete_all_messages_error) ( void* ipc_data );

  /**
  * @brief Error reply to "send stored message" method call
  * @param ipc_data ipc data
  */
  void (*send_stored_message_error) ( void* ipc_data );

  /**
  * @brief Error reply to "send generic sim command" method call
  * @param ipc_data ipc data
  */
  void (*send_generic_sim_command_error) ( void* ipc_data );


  /********************************Signals***********************************/

  /**
   * @brief Authorization status signal
   * @param status authorization stauts
   */
  void (*auth_status) ( SIM_Status status );

  /**
  * @brief Message has arrived
  * @param message's index
  */
  void (*incoming_message) ( s32 index );

};


/* 定义消息状态 */
#define SMS_STAT_REC_UNREAD       0
#define SMS_STAT_REC_READ         1
#define SMS_STAT_STO_UNSENT       2
#define SMS_STAT_STO_SENT         3
#define SMS_STAT_ALL              4



/******************************************/
/*              外部函数[声明]            */
/******************************************/

  /***************************Method calls*******************************/

  /**
   * @brief Gets authorization status
   * @param ipc_data data for ipc
   */
extern void lgsm_sim_set_sim_own_number ( void* ipc_data, const ascii *simNumber );

  /**
   * @brief Gets authorization status
   * @param ipc_data data for ipc
   */
extern void lgsm_sim_get_auth_status ( void* ipc_data);

  /**
   * @brief Sends pin code
   * @param ipc_data data for ipc
   * @param pin pin code
   */
extern void lgsm_sim_send_auth_code ( void* ipc_data, const ascii* pin);

  /**
   * @brief Unlocks sim card with puk code and sets a new pin code
   * @param ipc_data data for ipc
   * @param puk puk code
   * @param new_pin new pin code
   */
extern void lgsm_sim_unlock ( void* ipc_data,
                  const ascii* puk,
                  const ascii* new_pin);

  /**
   * @brief Changes pin code
   * @param ipc_data data for ipc
   * @param old_pin old pin code
   * @param new_pin new pin code
   */
extern void lgsm_sim_change_auth_code ( void* ipc_data,
                            const ascii* old_pin,
                            const ascii* new_pin);

  /**
   * @brief Gets SIM info: IMSI(International Mobile Subscriber
   * Identity), subscriber number and country code
   * @param sim sim interface
   * pointer @param ipc_data data for ipc
   */
extern void lgsm_sim_get_sim_info ( void* ipc_data);

extern void lgsm_sim_get_imsi ( void* ipc_data);


  /**
  * @brief Gets service center number
  * @param ipc_data ipc data
  */
extern void lgsm_sim_get_service_center_number ( void* ipc_data);

  /**
  * @brief Sets service center number
  * @param ipc_data ipc data
  * @param number service center number
  */
extern void lgsm_sim_set_service_center_number ( void* ipc_data, const ascii *number);

  /**
  * @brief Stores a message
  * @param ipc_data ipc data
  * @param recipient messages recipient
  * @param contents messages contents
  */
extern void lgsm_sim_store_message ( void* ipc_data,
                         const ascii *message,
                         const ascii *number );

  /**
  * @brief Retrieves message
  * @param ipc_data ipc data
  * @param index message's index
  */
extern void lgsm_sim_retrieve_message ( void* ipc_data,
                            s32 index );

  /**
  * @brief List SMS messages from preferred store
  * @param ipc_data ipc data
  * @param index message's index
  */
extern void lgsm_sim_list_preferred_message ( void* ipc_data,
                            s32 msgStat );

  /**
  * @brief Deletes a message
  * @param ipc_data ipc data
  * @param index of the message to delete
  */
extern void lgsm_sim_delete_message ( void* ipc_data,
                          s32 index );

  /**
  * @brief Deletes a message
  * @param ipc_data ipc data
  */
extern void lgsm_sim_delete_all_messages ( void* ipc_data );

  /**
  * @brief Sends a message from sim card
  * @param ipc_data ipc data
  * @param index of the message to delete
  */
extern void lgsm_sim_send_stored_message ( void* ipc_data,
                               s32 index );

  /**
  * @brief Send a generic SIM command to the SIM card.
  * @param ipc_data ipc data
  * @param command The command to send
  */
extern void lgsm_sim_send_generic_sim_command ( void* ipc_data,
                                    const ascii *command );


/**
 * @brief Creates the at command to read a sms message
 * @param pos index of the message to read
 */
extern void lgsm_sim_select_phone_book_memory ( void* ipc_data, 
                                      const ascii *memory );




#endif	/* __SIM_INTERFACE_H__ */

