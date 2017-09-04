



#ifndef __GL868_H__
#define __GL868_H__




#include "stm32f4xx.h"

#include "GSM_xx_vendor_cmd.h"
#include "modem.h"
#include "modem_debug.h"





/******************************************/
/*            定时器参数[配置]            */
/******************************************/

#define GL868_SwitchToATStatus			MDM_SwitchToATStatus
#define GL868_SwitchToDataStatus  	MDM_SwitchToDataStatus

#define MODEM_Start                 GL868_PowerSwitchON
#define MODEM_OFF   								GL868_SwitchOFF
#define MODEM_WriteData             GL868_WriteData      
#define MODEM_ReadData              GL868_ReadData   
#define MODEM_Reset                 MDM_Reset 


#define MODEM_SwitchToATStatus      MDM_SwitchToATStatus
#define MODEM_SwitchToDataStatus    MDM_SwitchToDataStatus








/******************************************/
/*              外部函数[声明]            */
/******************************************/

extern bool GL868_WriteData ( u8 FlowType, u8 Data[], u16 DataSize );
extern bool GL868_ReadData ( u16 DataSize, u8 *Data );
extern bool RING_Handler ( u8 ID );
extern bool GL868_SwitchToATStatus ( u8 switchType );
extern void GL868_PowerSwitchON(void);
extern void GL868_SwitchOFF(void);
extern void GSM_AT_Reinit(void);
extern void MDM_ResetPIN(void);

#endif  /* __MC52I_H__ */


