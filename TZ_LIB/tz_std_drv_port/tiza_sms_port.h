





#ifndef __TZ_SMS_H__
#define __TZ_SMS_H__




#include "stm32f4xx.h"




extern ascii fromPhone[ 15 ];



/******************************************/
/*              外部函数[声明]            */
/******************************************/

extern void sim_read_all_message_reply ( void* ipc_data, s16 index, const ascii *sender_number, const ascii *contents );
extern void sim_read_message_reply ( void* ipc_data, const ascii *sender_number, const ascii *contents );
extern void sim_incoming_message ( s32 index );



#endif  /* __TZ_SMS_H__ */



