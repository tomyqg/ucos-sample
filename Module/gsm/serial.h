






#ifndef __SERIAL_H__
#define __SERIAL_H__




#include "stdbool.h"

/******************************************/
/*            数据流类型[定义]            */
/******************************************/

#define AT_Flow_Type			0     /* AT指令流 */
#define DATA_Flow_Type		1     /* 数据流 */







typedef struct Serial Serial_IfTypedef;

struct Serial {
	
  /************************Method calls*******************************/
  /**
   * @brief makes a call
   * @param ipc_data data for ipc
   * @param number the phone number to call
   */
  bool ( *write_data ) ( u8 FlowType, u8 Data[], u16 DataSize );


  /**********************Signals************************************/
  /* serial port  ---->   service interface */
  /**
  * @brief Message has been sent
  * @param success has message been succesfully sent
  * @param reason reason why message might have not been sent
  */
  bool ( *read_data ) ( u16 DataSize, u8 *Data );


  /**
  * @brief Error reply to "no_response_error" method
  *
  * @param modem pointer to modem struct
  * @param ipc_data ipc data
  */
  void ( *no_response_error ) ( void* ipc_data );

};








#endif  /* __SERIAL_H__ */

