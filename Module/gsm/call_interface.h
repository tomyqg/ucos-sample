





#ifndef __CALL_INTERFACE_H__
#define __CALL_INTERFACE_H__






/**
* @brief Call status
*/
typedef enum {

  /* Idle */
  CALL_IDLE=0,     

  /* Calling. After send ATD command, before call is connected */
  CALL_CALLING,    

  /* Call established */
  CALL_CONNECTED,  

  /* Wainting for remote to accept our call */
  CALL_WAITING,    

  /* Incoming call */
  CALL_INCOMING,   

  /* Call is on hold */
  CALL_HOLD,       
} CallStatus;






/**
* Call struct, stores information on active calls
*/
typedef struct {
	
  /* Call's current status */
  CallStatus status;

  /* Remote ends phone number */
  ascii *number;

  /* Call's type */
  ascii *type;

  /* Calls id in the gsmd */
  u32 id;
}	Call;








/**
 * @brief voice call interface provice voice call service such as dial accept or
 * hangup a call
 */
typedef struct CallInterface Call_IfTypedef;

struct CallInterface
{
	
  /**************************Replies to method calls*************************/

  /**
   * @brief sends a reply to ipc that phone call has been made
   * @param ipc_data data for ipc
   * @param id newly initiated phone call's id
   */
  void (*initiate_reply) ( void* ipc_data, const s32 id );


  /**
   * @brief sends a reply to ipc that phone call has been activated
   * @param ipc_data data for ipc
   */
  void (*activate_reply) ( void* ipc_data );

  /**
   * @brief sends a reply to ipc that a phone call has been released (hung up)
   * @param call_ipc call IPC interface pointer
   * @param ipc_data data for ipc
   */
  void (*release_reply) ( void* ipc_data );


  /*********************Error replies to method calls************************/

  /**
   * @brief sends an error reply to ipc
   * @param ipc_data ipc data
   */
  void (*initiate_error) ( void* ipc_data );


  /**
   * @brief sends an error reply to ipc
   * @param ipc_data ipc data
   */
  void (*activate_error) ( void* ipc_data );


  /**
   * @brief sends an error reply to ipc
   * @param ipc_data ipc data
   */
  void (*release_error) ( void* ipc_data );


  void (*set_ri_interrupt) ( void* ipc_data );
  /********************************Signals***********************************/

  /**
   * @brief call progress signal is emited to provide additional information
   * on call's progress
   */
  void (*call_status) ( const s32 id, const CallStatus status );

};



#define lgsm_call_release   CALL_Command_Release
#define lgsm_call_initiate  CALL_Command_Initiate
#define lgsm_call_activate  CALL_Command_Activate




/******************************************/
/*              外部函数[声明]            */
/******************************************/

  /***************************Method calls***********************************/

  /**
   * @brief makes a call
   * @param ipc_data data for ipc
   * @param number the phone number to call
   */
extern void lgsm_call_initiate ( void* ipc_data, const ascii* number, const ascii* type );

  /**
   * @brief Activates a call (either incoming or held)
   * @param ipc_data data for ipc
   * @param id call's id
   */
extern void lgsm_call_activate ( void* ipc_data, s32 id );

  /**
   * @brief Hangup a call. Hangup the current call or cancel the call
   * request
   * @param ipc_data data for ipc
   * @param message TODO what's this?
   * @param id index of the call to hang up
   */
extern void lgsm_call_release ( void* ipc_data, const s32 id );




#endif	/* __CALL_INTERFACE_H__ */




