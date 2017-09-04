





#ifndef __WIP_UTILITY_H__
#define __WIP_UTILITY_H__



#include "stm32f4xx.h"
#include "stdbool.h"



/******************************************/
/*               地址参数[配置]           */
/******************************************/

#define WIP_ADDR_TYPE_OUR_IP             0x01   /* 本地IP */
#define WIP_ADDR_TYPE_DNS1_IP            0x02   /* DNS1地址 */
#define WIP_ADDR_TYPE_DNS2_IP            0x03   /* DNS2地址 */






/******************************************/
/*           TCP工具参数[配置]            */
/******************************************/

extern bool wip_UtilityTcpCommandStart ( s32 socket, bool debugOpt );
extern bool wip_UtilityTcpCommandStop ( void );
extern bool wip_UtilityTcpCommandSend ( ascii *cmd, ascii *arg );
extern bool wip_UtilityQueryAddress ( u8 addrType, u32 *addr );
extern u32  wip_UtilityInetAddr ( const char *str );


#endif  /* __WIP_UTILITY_H__ */

