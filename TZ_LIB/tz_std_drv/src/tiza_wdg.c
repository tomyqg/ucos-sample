#include "tiza_wdg.h"
#include "hardware_select_config.h"
#include "defines.h"

#if __USE_WDG__ == 1
#ifndef __USE_WDG_TYPE__ 
#error ***ERROR __USE_WDG_TYPE__ MACRO MUST DEFINED***
#else
#if __USE_WDG_TYPE__ <= 0 || __USE_WDG_TYPE__ >= 4
#error ***ERROR __USE_WDG_TYPE__ MACRO TYPE DEFINED*** 
#endif
#endif  /* __USE_WDG_TYPE__ */
#endif  /* __USE_WDG__ */










/******************************************/
/*              内部变量[定义]            */
/******************************************/







/******************************************/
/*              内部函数[声明]            */
/******************************************/

#if __USE_WDG_TYPE__ == WDG_TYPE_EXTERNAL
static void EWDG_ApplicationConfigure ( void );
#endif  /* __USE_WDG__ */

#if __USE_WDG_TYPE__ == WDG_TYPE_WINDOWS
static void WWDG_ApplicationConfigure ( void );
#endif  /* __USE_WDG__ */









#if __USE_WDG_TYPE__ == WDG_TYPE_EXTERNAL
/* 
 * 功能描述: 外部看门狗
 * 引用参数:
 *          
 * 返回值  :
 * 
 */
static void EWDG_ApplicationConfigure ( void )
{
  TM_GPIO_Init(GPIOD,GPIO_Pin_14,TM_GPIO_Mode_OUT,TM_GPIO_OType_PP,TM_GPIO_PuPd_NOPULL,TM_GPIO_Speed_Fast);		
}

#endif  /* __USE_WDG__ */






#if __USE_WDG_TYPE__ == WDG_TYPE_WINDOWS

/* 
 * 功能描述: 
 * 引用参数:
 *          
 * 返回值  :
 * 
 */
static void WWDG_ApplicationConfigure ( void )
{
  /* WWDG configuration */
  /* Enable WWDG clock */
  RCC_APB1PeriphClockCmd ( RCC_APB1Periph_WWDG, ENABLE );

  /* WWDG clock counter = (PCLK1/4096)/8 = 244 Hz (~4 ms)  */
  WWDG_SetPrescaler ( WWDG_Prescaler_8 );

  /* Set Window value to 65 */
  WWDG_SetWindowValue ( 65 );

  /* Enable WWDG and set counter value to 127, WWDG timeout = ~4 ms * 64 = 262 ms */
  WWDG_Enable ( 127 );

  /* Clear EWI flag */
  WWDG_ClearFlag ();

  /* Enable EW interrupt */
  WWDG_EnableIT ();
}

#endif  /* __USE_WDG__ */



static void IWDG_configuration ( void )
{
	IWDG_WriteAccessCmd (IWDG_WriteAccess_Enable);
  /* IWDG counter clock: 40KHz(LSI) / 256 = 156.2 Hz */
  IWDG_SetPrescaler (IWDG_Prescaler_256);
  /* Set counter reload value to mux 4096*/
  IWDG_SetReload (3905);                     //  25S
  /* Reload IWDG counter */
  IWDG_ReloadCounter ();
  /* Enable IWDG (the LSI oscillator will be enabled by hardware) */
  IWDG_Enable ();
}



/* 
 * 功能描述: 看门狗参数配置
 * 引用参数:
 *          
 * 返回值  :
 * 
 */
extern void WDG_configuration ( void )
{

#if __USE_WDG_TYPE__ == WDG_TYPE_INDEPENDENCY
		IWDG_configuration();
#endif
#if __USE_WDG_TYPE__ == WDG_TYPE_EXTERNAL	
		EWDG_ApplicationConfigure();
#endif

}


/* 
 * 功能描述: 独立看门狗喂食
 * 引用参数:
 *          
 * 返回值  :
 * 
 */
static void IWDG_KickCmd ( void )
{
  /* Reload IWDG counter */
  IWDG_ReloadCounter();
	
}

/* 
 * 功能描述: 独立看门狗喂食
 * 引用参数:
 *          
 * 返回值  :
 * 
 */
static void EWDG_KickCmd ( void )
{
	TM_GPIO_TogglePinValue(GPIOD,GPIO_Pin_14);
  
}

/* 
 * 功能描述: 看门狗喂食
 * 引用参数:
 *          
 * 返回值  :
 * 
 */
extern void WDG_KickCmd ( void )
{
#if __USE_WDG_TYPE__ == WDG_TYPE_INDEPENDENCY
	/* Reload IWDG counter */
  IWDG_KickCmd();
#endif
	
#if __USE_WDG_TYPE__ == WDG_TYPE_EXTERNAL	
  EWDG_KickCmd();
#endif	
	
}










/* 
 * 功能描述: 看门狗中断
 * 引用参数:
 *          
 * 返回值  :
 * 
 */
extern void WWDG_IRQHandler ( void )
{
  /* Update WWDG counter */
  WWDG_SetCounter ( 0x7F );
  /* Clear EWI flag */
  WWDG_ClearFlag ();
}




