#include "defines.h"

acc_info_t accInfo = {0};

/* ACC信号检测脚 */
/*0：ACC有效，1：ACC断开*/
s8 acc_io_handler    = -1;




//ACC initation pin
//@goly
void TZ_ACC_Init(void)
{	
	TM_GPIO_Init(GPIOE,GPIO_Pin_2,TM_GPIO_Mode_IN,TM_GPIO_OType_PP,TM_GPIO_PuPd_NOPULL,TM_GPIO_Speed_Fast);
}





u8 TZ_Get_ACC_State(void)
{	
		return TM_GPIO_GetInputPinValue(GPIOE,GPIO_Pin_2);
}









