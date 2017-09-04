







#ifndef __SMS_INTERFACE_H__
#define __SMS_INTERFACE_H__




typedef struct SMSInterface SMS_IfTypedef;


#define lgsm_sms_send_message     SMS_Command_SendMessage



/**
* @brief SMS interface
*/
struct SMSInterface {

  /*********************Replies to methods*****************/

  /**
  * @brief Reply to "send message" method call
  * @param ipc_data ipc data
  */
  void ( *send_message_reply ) ( void* ipc_data );


  /*********************Method errors**********************************/

  /**
  * @brief Error reply to "send message" method call
  * @param ipc_data ipc data
  */
  void ( *send_message_error ) ( void* ipc_data );


  /**********************Signals************************************/
  /* serial port  ---->   service interface */

  /**
  * @brief Message has been sent
  * @param success has message been succesfully sent
  * @param reason reason why message might have not been sent
  */
  void ( *message_sent ) ( bool success, const ascii* reason );


};




/******************************************/
/*              外部函数[声明]            */
/******************************************/

  /**
  * @brief Sends a message
  * @param ipc_data ipc data
  * @param message message's contents
  * @param number recipient's number
  * @param want_report should gsm be told that user wants a notification
  * when the message has been sent succesfully
  */
extern void lgsm_sms_send_message ( void* ipc_data, const ascii *message, const ascii *number, bool want_report );



#endif	/* __SMS_INTERFACE_H__ */



