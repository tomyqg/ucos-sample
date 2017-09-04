
#ifndef __STM32_LOW_POWER_PORT_H__
#define __STM32_LOW_POWER_PORT_H__







/******************************************/
/*           低层接口函数[声明]           */
/******************************************/

extern void LPWR_OSTaskSuspend ( void );
extern void LPWR_OSTaskResume ( void );
extern void RCC_SysClkConfigFromSTOPMode(void);



#endif  /* __STM32_LOW_POWER_PORT_H__ */

