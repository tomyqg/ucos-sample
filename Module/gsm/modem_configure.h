
#ifndef __MODEM_CONFIGURE_H__
#define __MODEM_CONFIGURE_H__


/*GSM模块选择器*/
#define 	USE_MODEM_XX









/*模块配置器*/

#ifndef 	USE_MODEM_XX
#include 	"GSM_xx.h"
#define 	USE_MODEM_XX         1
#endif




#endif














/******************************************/
/*              外部函数[声明]            */
/******************************************/

extern void MODEM_Start ( void );
extern void MODEM_Reset ( void );
extern bool MODEM_WriteData ( u8 FlowType, u8 Data[], u16 DataSize );
extern bool MODEM_ReadData ( u16 DataSize, u8 *Data );
extern bool MODEM_SwitchToDataStatus ( void );
extern bool MODEM_SwitchToATStatus ( u8 switchType );


#endif  /* __MODEM_CONFIGURE_H__ */


