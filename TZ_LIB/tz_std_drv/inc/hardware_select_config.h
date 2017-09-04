


#ifndef __HARDWARE_SELECT_CONFIG_H__
#define __HARDWARE_SELECT_CONFIG_H__


#define APP_STACK_CHECK_DEBUG                0

#if	APP_STACK_CHECK_DEBUG	==	1
#define     TZ_OS_TASK_OPT_CHECK       (OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR)
#else
#define     TZ_OS_TASK_OPT_CHECK       (OS_TASK_OPT_STK_CHK)
#endif

// -------- <<< Use Configuration Wizard in Context Menu >>> -----------------


/******************************************/
/*          程序存储区地址[配置]          */
/******************************************/

#ifndef __APPLI_REGION_ALLOCA_H__
#define __APPLI_REGION_ALLOCA_H__

///////////////////////////////////////////////////////////////////////////////////
    /***************** 内部FLASH区域划分 ********************/
/******************************************************************		

BOOT程序 				------>   0x0800 0000 - 0x0800 3FFF	     扇区0 (16K) 


eemprom区 			------>   0x0800 4000 - 0x0800 7FFF	     扇区1 (16K)
(使用3个扇区)						  0x0800 8000 - 0x0800 BFFF	     扇区2 (16K)
													0x0800 C000 - 0x0800 FFFF	     扇区3 (16K)
													0x0801 0000 - 0x0801 FFFF	     扇区4 (64K) (未使用)


APP程序区  			------>   0x0802 0000 - 0x0803 FFFF	     扇区5 (128K)
													0x0804 4000 - 0x0805 FFFF	     扇区6 (128K)

升级文件缓存 	 	------>   0x0806 0000 - 0x0807 FFFF	     扇区7 (128K)
													0x0808 4000 - 0x0809 FFFF	     扇区8 (128K)
						
						
升级信息配置区	------>   0x080A 0000 - 0x080B FFFF	     扇区9 (128K)

													
自定义数据区   	------>   扇区10---扇区11

//
//@@@goly
****************************************************************************/




#ifndef REGION_ALLOCA_CFG
#define REGION_ALLOCA_CFG

// <h> Application vector offset address Configuration
// Tailor this value to your application needs
//   <o> Application vector offset address (in Bytes) <0x01000-0x80000:4>
#define APPLICATION_OFFSET                  0x020000
// </h>

#define BOOTLOAD_ADDRESS_START							ADDR_FLASH_SECTOR_0





#if defined STM32F4XX
               
/* 主程序存储地址 */                                               
#define  APP_ADDRESS_STAET                    (BOOTLOAD_ADDRESS_START+APPLICATION_OFFSET)




//升级文件地址分配
#define APP_STORE_ADDRESS_START								ADDR_FLASH_SECTOR_7
#define APP_STORE_ADDRESS_END									ADDR_FLASH_SECTOR_9
/*程序存储所需页数*/
#define APP_STORE_PAGE_NUM                    2


//程序存储地址
#define APPLICATION_STORE_ADDRESS          	 	APP_STORE_ADDRESS_START

#define APPLICATION_STORE_SECTOR          	 	FLASH_Sector_7
/* 程序镜像大小 */
// Tailor this value to your application needs
//   <o> User Application Image length (in Bytes) <0x00010000-0x00080000:8>
#define PROGRAM_IMAGE_SIZE                  	APP_STORE_ADDRESS_END-APP_STORE_ADDRESS_START    


/* 程序升级配置信息 */
// Tailor this value to your application needs
//   <o> User Application base address (in Bytes) <0x08030000-0x08080000:8>


#define APPLICATION_UPGRADE_CONFIG_ADDRESS  	ADDR_FLASH_SECTOR_9
// </h>

// <h> EEPROM start address AND end address Configuration
/* EEPROM start address in Flash
 * EEPROM emulation start address: after 384KByte of used Flash memory */
//<o> EEPROM start address (in Bytes) <0x08030000-0x080C0000:8>






//EEPROM : 16*3=36K字节
#define EEPROM_START_ADDRESS    						0x08004000 

/* EEPROM emulation end address */
// Tailor this value to your application needs
//   <o> EEPROM end address (in Bytes) <0x08030000-0x080C0000:8>
#define EEPROM_END_ADDRESS      						0x08014000 
// </h> 
#endif  /* STM32F4XX */          
#endif  /* REGION_ALLOCA_CFG */




#if defined STM32F40_41xxx
 #define BLOCK_SIZE                        (0x40000)/* 256 Kbytes */				
 #define PAGE_SIZE                         (0x4000)    /* 16 Kbytes */
 #define FLASH_SIZE                        (0x100000)  /*1024 KBytes */
#else 
 #error "ME is not STM32F407,Please select first the STM32F device to be used"    
#endif



#endif	/* __APPLI_REGION_ALLOCA_H__ */








/******************************************/
/*               ACC参数[配置]            */
/******************************************/

#ifndef __ACC_CFG__
#define __ACC_CFG__


#ifndef __USE_ACC__
#define __USE_ACC__             1
#endif  /* __USE_ACC__ */


#endif  /* __ACC_CFG__ */



/******************************************/
/*               ADC参数[配置]            */
/******************************************/

#ifndef __ADC_CFG__
#define __ADC_CFG__

// <e>ADC1 controller Definitions
// ===================
//   <i>Enable ADC1 controller function
#ifndef __USE_ADC_1__
#define __USE_ADC_1__             1
#endif  /* __USE_ADC_1__ */
// </e>



// <e>ADC2 controller Definitions
// ===================
//   <i>Disable ADC2 controller function
#ifndef __USE_ADC_2__
#define __USE_ADC_2__             0
#endif  /* __USE_ADC_2__ */
// </e>



// <e>ADC3 controller Definitions
// ===================
//   <i>Disable ADC3 controller function
#ifndef __USE_ADC_3__
#define __USE_ADC_3__             0
#endif  /* __USE_ADC_3__ */
// </e>

#endif  /* __ADC_CFG__ */

#if ( __USE_ADC_1__ == 1 )||( __USE_ADC_2__ == 1 )||( __USE_ADC_3__ == 1 )

#ifndef __USE_ADC__
#define __USE_ADC__     1
#endif
#endif







/******************************************/
/*               CAN参数[配置]            */
/******************************************/

#ifndef	__USE_CAN__
// <e>CAN controller Definitions
// ===================
//   <i>default Disable CAN controller function
/* Can模块启用    */
#define __USE_CAN__                       1       

#if __USE_CAN__ == 1
#ifndef	__DEV_CAN_VCC_AUTO__
/* Can模块电源默认配置 */
#define __DEV_CAN_VCC_AUTO__              1       
#endif

//#ifndef	CAN_IDE_TYPE
///* Can数据帧类型 0x00: CAN_FRAME_EXT_AND_STD; 0x01: CAN_FRAME_EXT 扩展;  0x02: CAN_FRAME_STD 标准 */
//#define CAN_IDE_TYPE                      0x00      
//#endif
 
#ifndef __USE_CAN_ERROR_MAMNAGEMENT__                                                 
#define __USE_CAN_ERROR_MAMNAGEMENT__     0		    /* Can模块异常管理  */
#endif
#endif  /* __USE_CAN__ */
// </e>
#endif







/******************************************/
/*               EXTI参数[配置]           */
/******************************************/

#ifndef __USE_EXTI_CFG__

// <e>EXTI controller configure Definitions
// ===================
//   <i>Enable EXTI controller configure function
#define __USE_EXTI_CFG__    0

#ifndef __USE_EXTI0__
// <e>EXTI0 controller Definitions
// ===================
//   <i>Enable EXTI0 controller function
#define __USE_EXTI0__   1
// </e>
#endif  /* __USE_EXTI1__ */

#ifndef __USE_EXTI1__
// <e>EXTI1 controller Definitions
// ===================
//   <i>Enable EXTI1 controller function
#define __USE_EXTI1__   0
// </e>
#endif  /* __USE_EXTI1__ */


#ifndef __USE_EXTI2__
// <e>EXTI2 controller Definitions
// ===================
//   <i>Enable EXTI2 controller function
#define __USE_EXTI2__   0
// </e>
#endif  /* __USE_EXTI2__ */  

#ifndef __USE_EXTI3__
// <e>EXTI3 controller Definitions
// ===================
//   <i>Enable EXTI3 controller function
#define __USE_EXTI3__   0
// </e>
#endif  /* __USE_EXTI3__ */

#ifndef __USE_EXTI4__
// <e>EXTI4 controller Definitions
// ===================
//   <i>Enable EXTI4 controller function
#define __USE_EXTI4__   0
// </e>
#endif  /* __USE_EXTI4__ */

#ifndef __USE_EXTI5_9__
// <e>EXTI5_9 controller Definitions
// ===================
//   <i>Enable EXTI5_9 controller function
#define __USE_EXTI5_9__   0
// </e>
#endif  /* __USE_EXTI1__ */

#ifndef __USE_EXTI10_15__
// <e>EXTI10_15 controller Definitions
// ===================
//   <i>Enable EXTI10_15 controller function
#define __USE_EXTI10_15__   1
// </e>
#endif  /* __USE_EXTI10_15__ */

// </e>
#endif  /* __USE_EXTI_CFG__ */






/******************************************/
/*              FLASH参数[配置]           */
/******************************************/

#ifndef __USE_FLASH__
#define __USE_FLASH__		  1

#if __USE_FLASH__ > 0
#define __DEBUG_FLASH__	      0		 /* FLASH模块调试 */
#endif

#endif  /* __USE_FLASH__ */







/******************************************/
/*            工作队列参数[配置]          */
/******************************************/

#ifndef __USE_WORKQUEUE__
#define __USE_WORKQUEUE__   1

#endif  /* __USE_WORKQUEUE__ */









/******************************************/
/*              GPIO参数[配置]            */
/******************************************/

#ifndef __USE_GPIO_CFG__

// <e>GPIO controller configure Definitions
// ===================
//   <i>Enable GPIO controller configure function
#define __USE_GPIO_CFG__    1

#ifndef __USE_PORTA__
// <e>GPIOB controller Definitions
// ===================
//   <i>Enable GPIOB controller function
#define __USE_PORTA__   1
// </e>
#endif  /* __USE_PORTA__ */


#ifndef __USE_PORTB__
// <e>GPIOA controller Definitions
// ===================
//   <i>Enable GPIOA controller function
#define __USE_PORTB__   1
// </e> 
#endif  /* __USE_PORTB__ */  


#ifndef __USE_PORTC__
// <e>GPIOC controller Definitions
// ===================
//   <i>Enable GPIOC controller function
#define __USE_PORTC__   1
// </e>  
#endif  /* __USE_PORTC__ */ 


#ifndef __USE_PORTD__
// <e>GPIOD controller Definitions
// ===================
//   <i>Enable GPIOD controller function
#define __USE_PORTD__   1
// </e>
#endif  /* __USE_PORTD__ */


#ifndef __USE_PORTE__
// <e>GPIOE controller Definitions
// ===================
//   <i>Enable GPIOE controller function
#define __USE_PORTE__   1
// </e> 
#endif  /* __USE_PORTE__ */


#ifndef __USE_PORTF__
// <e>GPIOF controller Definitions
// ===================
//   <i>Enable GPIOF controller function
#define __USE_PORTF__   1
// </e> 
#endif  /* __USE_PORTF__ */


#ifndef __USE_PORTG__
// <e>GPIOG controller Definitions
// ===================
//   <i>Enable GPIOG controller function
#define __USE_PORTG__   1
// </e>
#endif  /* __USE_PORTG__ */

// </e>
#endif  /* __USE_GPIO_CFG__ */






/******************************************/
/*           HARDTIMER参数[配置]          */
/******************************************/

#ifndef __USE_TIM_1__
// <e>TIM1 controller Definitions
// ===================
//   <i>Enable TIM1 controller function
#define __USE_TIM_1__   1

// <h> TIM1 clock Configuration
//   <o>Timer clock value [Hz] <1-36000000>
//   <i> Set the timer clock value for selected timer.
//   <i> Default: 1000000  (1MHz)
#define TIM1_CLK          400000      //400000

//   <o>Timer trigger frequency value [Hz] <1-36000000>
//   <i> Set the timer trigger frequency for selected timer.
//   <i> Default: 1000  (1kHz)
#define TIM1_UPDATE_FRE   100
 
// </h>
// </e>
#endif  /* __USE_TIM_1__ */



#ifndef __USE_TIM_2__
// <e>TIM2 controller Definitions
// ===================
//   <i>Enable TIM2 controller function
#define __USE_TIM_2__    0

// <h> TIM2 clock Configuration
//   <o>Timer clock value [Hz] <1-36000000>
//   <i> Set the timer clock value for selected timer.
//   <i> Default: 1000000  (40kHz)
#define TIM2_CLK          40000

//   <o>Timer trigger frequency value [Hz] <1-36000000>
//   <i> Set the timer trigger frequency for selected timer.
//   <i> Default: 1000  (1kHz)
#define TIM2_UPDATE_FRE   1000

// <e>PWM input-controller Definitions
//   <i>Enable PWM input-controller function
#define __USE_PWM_INPUT__   0
// </e>
// </h>
// </e>
#endif  /* __USE_TIM_2__ */



#ifndef __USE_TIM_3__
// <e>TIM3 controller Definitions
// ===================
//   <i>Enable TIM3 controller function
#define __USE_TIM_3__     0

// <h> TIM3 clock Configuration
//   <o>Timer clock value [Hz] <1-36000000>
//   <i> Set the timer clock value for selected timer.
//   <i> Default: 1000000  (40kHz)
#define TIM3_CLK          40000

//   <o>Timer trigger frequency value [Hz] <1-36000000>
//   <i> Set the timer trigger frequency for selected timer.
//   <i> Default: 1000  (1kHz)
#define TIM3_UPDATE_FRE   1000

// <e>PWM output-controller Definitions
//   <i>Enable PWM output-controller function
#define __USE_PWM_OUTPUT__  0

//   <o>PWM output frequency value [Hz] <1-1000>
//   <i> Set the PWM out-frequency.
//   <i> Default: 1000  (1kHz)
#define PWM_OUTPUT_FREQUENCY  1000  

//   <o>PWM output dutycycle value [%] <1-100>
//   <i> Set the PWM out-dutycycle.
//   <i> Default: 50  (50%)
#define PWM_OUTPUT_DUTYCYCLE  50
// </e>
// </h>
// </e>
#endif  /* __USE_TIM_3__ */



#ifndef __USE_TIM_4__
// <e>TIM4 controller Definitions
// ===================
//   <i>Enable TIM4 controller function
#define __USE_TIM_4__     0

// <h> TIM4 clock Configuration
//   <o>Timer clock value [Hz] <1-36000000>
//   <i> Set the timer clock value for selected timer.
//   <i> Default: 1000000  (1MHz)
#define TIM4_CLK          2000000


#ifndef __USE_TIM_4_TIMEBASE1__
// <e>TIM4 controller time base 1 Definitions
// ===================
//   <i>Enable TIM4 controller time base function
#define __USE_TIM_4_TIMEBASE1__     0

//   <o>Timer trigger frequency value [Hz] <1-1000>
//   <i> Set the timer trigger frequency for selected timer.
//   <i> Default: 1000  (1kHz)
#define TIM4_TIMEBASE_UPDATE_FRE1   100

#define TCU_TIM4_BASE1_CAPTURE   
#endif
// </e>


#ifndef __USE_TIM_4_TIMEBASE2__
// <e>TIM4 controller time base 2 Definitions
// ===================
//   <i>Enable TIM4 controller time base function
#define __USE_TIM_4_TIMEBASE2__     0

//   <o>Timer trigger frequency value [Hz] <1-1000>
//   <i> Set the timer trigger frequency for selected timer.
//   <i> Default: 1000  (1kHz)
#define TIM4_TIMEBASE_UPDATE_FRE2   50


#define TCU_TIM4_BASE2_CAPTURE    
#endif
// </e>


#ifndef __USE_TIM_4_TIMEBASE3__
// <e>TIM4 controller time base 3 Definitions
// ===================
//   <i>Enable TIM4 controller time base function
#define __USE_TIM_4_TIMEBASE3__     0

//   <o>Timer trigger frequency value [Hz] <1-1000>
//   <i> Set the timer trigger frequency for selected timer.
//   <i> Default: 1000  (1kHz)
#define TIM4_TIMEBASE_UPDATE_FRE3   100

#define TCU_TIM4_BASE3_CAPTURE    
#endif
// </e>


#ifndef __USE_TIM_4_TIMEBASE4__
// <e>TIM4 controller time base 4 Definitions
// ===================
//   <i>Enable TIM4 controller time base function
#define __USE_TIM_4_TIMEBASE4__     0

//   <o>Timer trigger frequency value [Hz] <1-36000000>
//   <i> Set the timer trigger frequency for selected timer.
//   <i> Default: 1000  (1kHz)
#define TIM4_TIMEBASE_UPDATE_FRE4   100

#define TCU_TIM4_BASE4_CAPTURE    
#endif
// </e>

// </h>
// </e>
#endif  /* __USE_TIM_4__ */



#ifndef __USE_TIM_5__
// <e>TIM5 controller Definitions
// ===================
//   <i>Enable TIM5 controller function
#define __USE_TIM_5__     0

// <h> TIM5 clock Configuration
//   <o>Timer clock value [Hz] <1-36000000>
//   <i> Set the timer clock value for selected timer.
//   <i> Default: 1000000  (1MHz)
#define TIM5_CLK          1000000

//   <o>Timer trigger frequency value [Hz] <1-36000000>
//   <i> Set the timer trigger frequency for selected timer.
//   <i> Default: 1000  (1kHz)
#define TIM5_UPDATE_FRE   1000
// </h>
// </e>
#endif  /* __USE_TIM_5__ */



#ifndef __USE_TIM_6__
// <e>TIM6 controller Definitions
// ===================
//   <i>Enable TIM6 controller function
#define __USE_TIM_6__     0

// <h> TIM6 clock Configuration
//   <o>Timer clock value [Hz] <1-36000000>
//   <i> Set the timer clock value for selected timer.
//   <i> Default: 1000000  (1MHz)
#define TIM6_CLK          1000000

//   <o>Timer trigger frequency value [Hz] <1-36000000>
//   <i> Set the timer trigger frequency for selected timer.
//   <i> Default: 1000  (1kHz)
#define TIM6_UPDATE_FRE   1000
// </h>
// </e>
#endif  /* __USE_TIM_6__ */



#ifndef __USE_TIM_7__
// <e>TIM7 controller Definitions
// ===================
//   <i>Enable TIM7 controller function
#define __USE_TIM_7__     0

// <h> TIM7 clock Configuration
//   <o>Timer clock value [Hz] <1-36000000>
//   <i> Set the timer clock value for selected timer.
//   <i> Default: 1000000  (1MHz)
#define TIM7_CLK          1000000

//   <o>Timer trigger frequency value [Hz] <1-36000000>
//   <i> Set the timer trigger frequency for selected timer.
//   <i> Default: 1000  (1kHz)
#define TIM7_UPDATE_FRE   1000
// </h>
// </e>
#endif  /* __USE_TIM_7__ */



#ifndef __USE_TIM_8__
// <e>TIM8 controller Definitions
// ===================
//   <i>Enable TIM8 controller function
#define __USE_TIM_8__     0

// <h> TIM8 clock Configuration
//   <o>Timer clock value [Hz] <1-36000000>
//   <i> Set the timer clock value for selected timer.
//   <i> Default: 1000000  (1MHz)
#define TIM8_CLK          1000000

//   <o>Timer trigger frequency value [Hz] <1-36000000>
//   <i> Set the timer trigger frequency for selected timer.
//   <i> Default: 1000  (1kHz)
#define TIM8_UPDATE_FRE   1000
// </h>
// </e>
#endif  /* __USE_TIM_8__ */

#if __USE_TIM_1__ == 1 || __USE_TIM_2__ == 1 || __USE_TIM_3__ == 1 || __USE_TIM_4__ == 1 || __USE_TIM_5__ == 1 || __USE_TIM_6__ == 1 || __USE_TIM_7__ == 1 || __USE_TIM_8__ == 1 
#ifndef __USE_TCU__
#define __USE_TCU__   1
#endif
#endif






/******************************************/
/*               I2C参数[配置]            */
/******************************************/

#ifndef __I2C_CFG__
#define __I2C_CFG__

#ifndef __USE_I2C1__
// <e>I2C1 controller
// ===================
//   <i>Enable I2C1 controller function
#define __USE_I2C1__		            0

//   <o>Config I2C1 controller Clock      
//      <0=> 200K    <1=> 400K
//   <i> Default I2C1 controller clock 200K    
#ifndef __I2C1_CLK__
#define __I2C1_CLK__                0
#endif
// </e>
#endif  /* __USE_I2C1__ */


#ifndef __USE_I2C2__
// <e>I2C2 controller
// ===================
//   <i>Enable I2C2 controller function
#define __USE_I2C2__		            0

//   <o>Config I2C2 controller Clock      
//      <0=> 200K    <1=> 400K
//   <i> Default I2C2 controller clock 200K    
#ifndef __I2C2_CLK__
#define __I2C2_CLK__                0
#endif
// </e>
#endif  /* __USE_I2C2__ */

#endif  /* __I2C_CFG__ */







/******************************************/
/*          LOWPOWER参数[配置]            */
/******************************************/

// <e>low-power controller

#define __USE_LOW_PWR__            1







/******************************************/
/*               PVD参数[配置]            */
/******************************************/

#ifndef __USE_PWR_VOLT_DETECTOR__
// <e>power voltage detector controller
// ===================
//   <i>Enable power voltage detector controller function
#define __USE_PWR_VOLT_DETECTOR__       0
// </e>
#endif  /* __USE_LOW_PWR__ */






/******************************************/
/*               RCC参数[配置]            */
/******************************************/

#ifndef SYS_CLK_CFG
#define SYS_CLK_CFG     1

#if SYS_CLK_CFG == 1
// <h>System clock configuration 
// =================== 

/* 系统时钟源设置 
 */
#ifndef SYS_CLK_SOURCE 
//   <o>System clock source      
//      <0=> Clock_8M_HSI    <1=> Clock_8M_HSE
//   <i> Default system clock controller source 0 
#define SYS_CLK_SOURCE            0
#endif	/* SYS_CLK_SOURCE */



/* 系统时钟频率设置 
 */
#ifndef SYS_CLK_FREQUENCY 
//   <o>System clock frequency    
//      <8=> SYSCLK_FREQ_8MHz
//      <24=> SYSCLK_FREQ_24MHz    <32=> SYSCLK_FREQ_32MHz
//      <48=> SYSCLK_FREQ_48MHz    <72=> SYSCLK_FREQ_72MHz
//   <i> Default system clock controller frequency [36M]
#define SYS_CLK_FREQUENCY         48
#endif	/* SYS_CLK_FREQUENCY */


#ifndef SYSTEM_RESTART__
#define SYSTEM_RESTART__            1

/* 定义系统重启调试开关 */
#define SYSTEM_RESTART_DEBUG        1
#endif  /* SYSTEM_RESTART__ */

#endif
// </h>

#endif  /* SYS_CLK_CFG */






/******************************************/
/*               RTC参数[配置]            */
/******************************************/

#ifndef __USE_RTC__

// <e>RTC controller Definitions
// ===================
//   <i>Enable RTC controller function
#define __USE_RTC__   1

/* RTC时钟源设置 
 */
#ifndef RTC_CLK_SOURCE
//   <o>RTC clock configuration      
//      <0=> Clock LSE    <1=> Clock LSI
//   <i> Default RTC clock LSI
#define RTC_CLK_SOURCE            0
#endif	/* RTC_CLK_SOURCE */

// </e>
#endif  /* __USE_RTC__ */







/******************************************/
/*              TIMER参数[配置]           */
/******************************************/

//软件定时器配置
#ifndef __USE_TMR__
#define __USE_TMR__     1
#endif  /* __USE_TMR__ */




/******************************************/
/*               SPI参数[配置]            */
/******************************************/

#ifndef __USE_SPI_1__
// <e>SPI1 controller
// ===================
//   <i>Enable SPI1 controller function
#define __USE_SPI_1__       1

//   <o>SPI1 direction configuration      
//      <0=> Direction_2Lines_FullDuplex    <1024=> Direction_2Lines_RxOnly
//      <32768=> Direction_1Line_Rx         <49152=> Direction_1Line_Tx 
#ifndef SPI1_Direction
#define SPI1_Direction   0
#endif  /* SPI1_Direction */

//   <o>SPI1 mode configuration      
//      <260=> Mode_Master    <0=> Mode_Slave
#ifndef SPI1_Mode
#define SPI1_Mode        260
#endif  /* SPI1_Mode */

//   <o>SPI1 Datasize configuration      
//      <2048=> DataSize_16b    <0=> DataSize_8b
#ifndef SPI1_DataSize
#define SPI1_DataSize    0
#endif  /* SPI1_DataSize */

//   <o>SPI1 CPOL configuration      
//      <0=> CPOL_Low    <2=> CPOL_High
#ifndef SPI1_CPOL
#define SPI1_CPOL        2
#endif  /* SPI1_CPOL */

//   <o>SPI1 CPHA configuration      
//      <0=> CPHA_1Edge    <1=> CPHA_2Edge
#ifndef SPI1_CPHA
#define SPI1_CPHA        1
#endif  /* SPI1_CPHA */

//   <o>SPI1 NSS configuration      
//      <0=> NSS_Hard    <512=> NSS_Soft
#ifndef SPI1_NSS
#define SPI1_NSS         512
#endif  /* SPI1_NSS */

//   <o>SPI1 BaudRate Prescaler configuration      
//      <0=> BaudRatePrescaler_2       <8=> BaudRatePrescaler_4
//      <16=> BaudRatePrescaler_8      <24=> BaudRatePrescaler_16
//      <32=> BaudRatePrescaler_32     <40=> BaudRatePrescaler_64
//      <48=> BaudRatePrescaler_128    <56=> BaudRatePrescaler_256
#ifndef SPI1_BaudRatePrescaler
#define SPI1_BaudRatePrescaler       8
#endif  /* SPI1_BaudRatePrescaler */

//   <o>SPI1 FirstBit configuration      
//      <0=> FirstBit_MSB    <128=> FirstBit_LSB
#ifndef SPI1_FirstBit
#define SPI1_FirstBit                0
#endif  /* SPI1_FirstBit */
// </e>
#endif  /* __USE_SPI_1__ */



#ifndef __USE_SPI_2__
// <e>SPI2 controller
// ===================
//   <i>Enable SPI controller function
#define __USE_SPI_2__       0

//   <o>SPI2 direction configuration      
//      <0=> Direction_2Lines_FullDuplex    <1024=> Direction_2Lines_RxOnly
//      <32768=> Direction_1Line_Rx         <49152=> Direction_1Line_Tx 
#ifndef SPI2_Direction
#define SPI2_Direction   0
#endif  /* SPI2_Direction */

//   <o>SPI2 mode configuration      
//      <260=> Mode_Master    <0=> Mode_Slave
#ifndef SPI2_Mode
#define SPI2_Mode        260
#endif  /* SPI2_Mode */

//   <o>SPI2 Datasize configuration      
//      <2048=> DataSize_16b    <0=> DataSize_8b
#ifndef SPI2_DataSize
#define SPI2_DataSize    0
#endif  /* SPI2_DataSize */

//   <o>SPI2 CPOL configuration      
//      <0=> CPOL_Low    <2=> CPOL_High
#ifndef SPI2_CPOL
#define SPI2_CPOL        0
#endif  /* SPI2_CPOL */

//   <o>SPI2 CPHA configuration      
//      <0=> CPHA_1Edge    <1=> CPHA_2Edge
#ifndef SPI2_CPHA
#define SPI2_CPHA        0
#endif  /* SPI2_CPHA */

//   <o>SPI2 NSS configuration      
//      <0=> NSS_Hard    <512=> NSS_Soft
#ifndef SPI2_NSS
#define SPI2_NSS         512
#endif  /* SPI2_NSS */

//   <o>SPI2 BaudRate Prescaler configuration      
//      <0=> BaudRatePrescaler_2       <8=> BaudRatePrescaler_4
//      <16=> BaudRatePrescaler_8      <24=> BaudRatePrescaler_16
//      <32=> BaudRatePrescaler_32     <40=> BaudRatePrescaler_64
//      <48=> BaudRatePrescaler_128    <56=> BaudRatePrescaler_256
#ifndef SPI2_BaudRatePrescaler
#define SPI2_BaudRatePrescaler       16
#endif  /* SPI2_BaudRatePrescaler */

//   <o>SPI2 FirstBit configuration      
//      <0=> FirstBit_MSB    <128=> FirstBit_LSB
#ifndef SPI2_FirstBit
#define SPI2_FirstBit                0
#endif  /* SPI2_FirstBit */
// </e>
#endif  /* __USE_SPI_2__ */



#ifndef __USE_SPI_3__
// <e>SPI3 controller
// ===================
//   <i>Enable SPI3 controller function
#define __USE_SPI_3__       0

//   <o>SPI3 direction configuration      
//      <0=> Direction_2Lines_FullDuplex    <1024=> Direction_2Lines_RxOnly
//      <32768=> Direction_1Line_Rx         <49152=> Direction_1Line_Tx 
#ifndef SPI3_Direction
#define SPI3_Direction   0
#endif  /* SPI3_Direction */

//   <o>SPI3 mode configuration      
//      <260=> Mode_Master    <0=> Mode_Slave
#ifndef SPI3_Mode
#define SPI3_Mode        0
#endif  /* SPI3_Mode */

//   <o>SPI3 Datasize configuration      
//      <2048=> DataSize_16b    <0=> DataSize_8b
#ifndef SPI3_DataSize
#define SPI3_DataSize    0
#endif  /* SPI3_DataSize */

//   <o>SPI3 CPOL configuration      
//      <0=> CPOL_Low    <2=> CPOL_High
#ifndef SPI3_CPOL
#define SPI3_CPOL        0
#endif  /* SPI3_CPOL */

//   <o>SPI3 CPHA configuration      
//      <0=> CPHA_1Edge    <1=> CPHA_2Edge
#ifndef SPI3_CPHA
#define SPI3_CPHA        0
#endif  /* SPI3_CPHA */


//   <o>SPI3 NSS configuration      
//      <0=> NSS_Hard    <512=> NSS_Soft
#ifndef SPI3_NSS
#define SPI3_NSS         0
#endif  /* SPI3_NSS */

//   <o>SPI3 BaudRate Prescaler configuration      
//      <0=>  BaudRatePrescaler_2      <8=>  BaudRatePrescaler_4
//      <16=> BaudRatePrescaler_8      <24=> BaudRatePrescaler_16
//      <32=> BaudRatePrescaler_32     <40=> BaudRatePrescaler_64
//      <48=> BaudRatePrescaler_128    <56=> BaudRatePrescaler_256
#ifndef SPI3_BaudRatePrescaler
#define SPI3_BaudRatePrescaler       0
#endif  /* SPI3_BaudRatePrescaler */

//   <o>SPI3 FirstBit configuration      
//      <0=> FirstBit_MSB    <128=> FirstBit_LSB
#ifndef SPI3_FirstBit
#define SPI3_FirstBit                0
#endif  /* SPI3_FirstBit */
// </e>
#endif  /* __USE_SPI_3__ */


#if ( __USE_SPI_1__ == 1 )||( __USE_SPI_2__ == 1 )||( __USE_SPI_3__ == 1 )
#ifndef __USE_SPI__
#define __USE_SPI__   1
#endif  /* __USE_SPI__ */
#endif  /* __USE_SPI__ */









/******************************************/
/*            UPGRADE参数[配置]           */
/******************************************/

#ifndef __USE_UPGRADE__
#define __USE_UPGRADE__   1
#endif  /* __USE_UPGRADE__ */








/******************************************/
/*              USART参数[配置]           */
/******************************************/



#ifndef __USE_USART_1__
// <e>USART1 controller Definitions
// ===================
//   <i>Enable USART1 controller function
#define __USE_USART_1__		        1



/* 配置USART1 */
#if __USE_USART_1__ == 1
//   <o>Config USART1 controller baudrate 
//      <4800=> 4800bps        <9600=> 9600bps        <25600=> 25600bps
//      <38400=> 38400bps      <115200=> 115200bps
//   <i> Default USART1 baudrate 115200        
#define USART1_BaudRate					      115200						  /* 115200bps */

//   <o>Config USART1 controller world length 
//      <0=> Word Length 8bits        <4096=> Word Length 9bits
//   <i> Default USART1 world length 8bits     
#define USART1_WordLength				      0					          /* 8位数据位 */

//   <o>Config USART1 controller Stop Bits
//      <0=> StopBits 1           <4096=> StopBits 0.5
//      <8192=> StopBits 2        <12288=> StopBits 1.5
//   <i> Default USART1 Stop Bits 1   
#define USART1_StopBits					      0					          /* 1位停止位 */

//   <o>Config USART1 controller Parity 
//      <0=> Parity None    <1024=> Parity Even    <1536=> Parity Odd
//   <i> Default USART1 Parity None   
#define USART1_Parity					        0						        /* 无校验 */

//   <o>Config USART1 controller Tx Control 
//      <0=> Disable Tx    <8=> Enable Tx
//   <i> Default USART1 Enable Tx  
#define USART1_Mode_Tx                8

//   <o>Config USART1 controller Rx Control 
//      <0=> Disable Rx    <4=> Enable Rx
//   <i> Default USART1 Enable Rx 
#define USART1_Mode_Rx                4     

//   <o>Config USART1 controller Hardware Flow Control 
//      <0=> HardwareFlowControl_None      <256=> HardwareFlowControl_RTS
//      <512=> HardwareFlowControl_CTS     <768=> HardwareFlowControl_RTS_CTS
//   <i> Default USART1 Parity None   
#define USART1_HardwareFlowControl		0		                /* 无硬件流控制 */

//   <o>Config USART1 controller Receive Timeout (4ms) <2-30:1>
//   <i> Default USART1 Receive Timeout  6ms
#define USART1_RxTimeout              4

//   <o>Config USART1 controller Rx buffer size (bytes) <64-3000:2> 
//   <i> Default USART1 Rx buffer size 128 bytes   
#define USART1_RxBufferSize           256
#endif

// </e>
#endif  /* __USE_USART_1__ */




#ifndef __USE_USART_2__
// <e>USART2 controller Definitions
// ===================
//   <i>Enable USART2 controller function
#define __USE_USART_2__		            0

/* 配置USART2 */
#if __USE_USART_2__ == 1
//   <o>Config USART2 controller baudrate 
//      <4800=> 4800bps        <9600=> 9600bps        <25600=> 25600bps
//      <38400=> 38400bps      <115200=> 115200bps 
//   <i> Default USART2 baudrate 115200        
#define USART2_BaudRate					      9600						  /* 115200bps */

//   <o>Config USART2 controller world length 
//      <0=> Word Length 8bits        <4096=> Word Length 9bits
//   <i> Default USART2 world length 8bits     
#define USART2_WordLength				      0					          /* 8位数据位 */

//   <o>Config USART2 controller Stop Bits 
//      <0=> StopBits 1           <4096=> StopBits 0.5
//      <8192=> StopBits 2        <12288=> StopBits 1.5
//   <i> Default USART2 Stop Bits 1   
#define USART2_StopBits					      0					          /* 1位停止位 */

//   <o>Config USART2 controller Parity 
//      <0=> Parity None   <1024=> Parity Even   <1536=> Parity Odd
//   <i> Default USART2 Parity None   
#define USART2_Parity					        0						        /* 无校验 */

//   <o>Config USART2 controller Tx Control 
//      <0=> Disable Tx    <8=> Enable Tx
//   <i> Default USART2 Enable Tx  
#define USART2_Mode_Tx                0

//   <o>Config USART2 controller Rx Control 
//      <0=> Disable Rx    <4=> Enable Rx
//   <i> Default USART2 Enable Rx 
#define USART2_Mode_Rx                4 

//   <o>Config USART2 controller Hardware Flow Control 
//      <0=> HardwareFlowControl_None      <256=> HardwareFlowControl_RTS
//      <512=> HardwareFlowControl_CTS     <768=> HardwareFlowControl_RTS_CTS
//   <i> Default USART2 Parity None   
#define USART2_HardwareFlowControl		0		                /* 无硬件流控制 */

//   <o>Config USART2 controller Receive Timeout (4ms) <1-30:1>
//   <i> Default USART2 Receive Timeout  6ms
#define USART2_RxTimeout              2

//   <o>Config USART2 controller Rx buffer size (bytes) <64-3000:2> 
//   <i> Default USART2 Rx buffer size 128 bytes   
#define USART2_RxBufferSize           512
#endif

// </e>
#endif  /* __USE_USART_2__ */




#ifndef __USE_USART_3__
// <e>USART3 controller Definitions
// ===================
//   <i>Enable USART3 controller function
#define __USE_USART_3__		            1

#define USE_GSM_UART_DMA							0

/* 配置USART3 */
#if __USE_USART_3__ == 1
//   <o>Config USART3 controller baudrate 
//      <4800=> 4800bps        <9600=> 9600bps        <25600=> 25600bps
//      <38400=> 38400bps      <115200=> 115200bps     
//   <i> Default USART3 baudrate 115200        
#define USART3_BaudRate					      115200//9600						  /* 115200bps */

//   <o>Config USART3 controller world length 
//      <0=> Word Length 8bits        <4096=> Word Length 9bits
//   <i> Default USART3 world length 8bits     
#define USART3_WordLength				      0					          /* 8位数据位 */

//   <o>Config USART3 controller Stop Bits 
//      <0=> StopBits 1           <4096=> StopBits 0.5
//      <8192=> StopBits 2        <12288=> StopBits 1.5
//   <i> Default USART3 Stop Bits 1   
#define USART3_StopBits					      0					          /* 1位停止位 */

//   <o>Config USART3 controller Parity 
//      <0=> Parity None   <1024=> Parity Even   <1536=> Parity Odd
//   <i> Default USART3 Parity None   
#define USART3_Parity					        0						        /* 无校验 */

//   <o>Config USART3 controller Tx Control 
//      <0=> Disable Tx    <8=> Enable Tx
//   <i> Default USART3 Enable Tx  
#define USART3_Mode_Tx                8

//   <o>Config USART3 controller Rx Control 
//      <0=> Disable Rx    <4=> Enable Rx
//   <i> Default USART3 Enable Rx 
#define USART3_Mode_Rx                4 

//   <o>Config USART3 controller Hardware Flow Control 
//      <0=> HardwareFlowControl None      <256=> HardwareFlowControl RTS
//      <512=> HardwareFlowControl CTS     <768=> HardwareFlowControl RTS && CTS
//   <i> Default USART3 Parity None   
#define USART3_HardwareFlowControl		0		                /* 无硬件流控制 */

//   <o>Config USART3 controller Receive Timeout (4ms) <2-30:1>
//   <i> Default USART3 Receive Timeout  6ms
#define USART3_RxTimeout              8

//   <o>Config USART3 controller Rx buffer size (bytes) <64-3000:2> 
//   <i> Default USART3 Rx buffer size 128 bytes   
#define USART3_RxBufferSize           6*1024
#endif

// </e>
#endif  /* __USE_USART_3__ */




#ifndef __USE_USART_4__
// <e>USART4 controller Definitions
// ===================
//   <i>Enable USART4 controller function
#define __USE_USART_4__		            1


//#define TM_UART4_USE_CUSTOM_IRQ

/* 配置USART4 */
#if __USE_USART_4__ == 1
//   <o>Config USART4 controller baudrate 
//      <4800=> 4800bps        <9600=> 9600bps        <25600=> 25600bps
//      <38400=> 38400bps      <115200=> 115200bps     
//   <i> Default USART4 baudrate 115200        
#define USART4_BaudRate					      115200						  /* 115200bps */

//   <o>Config USART4 controller world length 
//      <0=> Word Length 8bits        <4096=> Word Length 9bits
//   <i> Default USART4 world length 8bits     
#define USART4_WordLength				      0					          /* 8位数据位 */

//   <o>Config USART4 controller Stop Bits 
//      <0=> StopBits 1           <4096=> StopBits 0.5
//      <8192=> StopBits 2        <12288=> StopBits 1.5
//   <i> Default USART4 Stop Bits 1   
#define USART4_StopBits					      0					          /* 1位停止位 */

//   <o>Config USART4 controller Parity 
//      <0=> Parity None   <1024=> Parity Even   <1536=> Parity Odd
//   <i> Default USART4 Parity None   
#define USART4_Parity					        0						        /* 无校验 */

//   <o>Config USART4 controller Tx Control 
//      <0=> Disable Tx    <8=> Enable Tx
//   <i> Default USART4 Enable Tx  
#define USART4_Mode_Tx                8

//   <o>Config USART4 controller Rx Control 
//      <0=> Disable Rx    <4=> Enable Rx
//   <i> Default USART4 Enable Rx 
#define USART4_Mode_Rx                4 

//   <o>Config USART4 controller Hardware Flow Control 
//      <0=> HardwareFlowControl None      <256=> HardwareFlowControl RTS
//      <512=> HardwareFlowControl CTS     <768=> HardwareFlowControl RTS && CTS
//   <i> Default USART4 Parity None   
#define USART4_HardwareFlowControl		0		                /* 无硬件流控制 */

//   <o>Config USART4 controller Receive Timeout (4ms) <2-30:1>
//   <i> Default USART4 Receive Timeout  6ms
#define USART4_RxTimeout              4

//   <o>Config USART4 controller Rx buffer size (bytes) <64-3000:2> 
//   <i> Default USART4 Rx buffer size 128 bytes   
#define USART4_RxBufferSize           2048
#endif

// </e>
#endif  /* __USE_USART_4__ */



#ifndef __USE_USART_5__
// <e>USART5 controller Definitions
// ===================
//   <i>Enable USART5 controller function
#define __USE_USART_5__		            0

/* 配置USART5 */
#if __USE_USART_5__ == 1
//   <o>Config USART5 controller baudrate 
//      <4800=> 4800bps        <9600=> 9600bps        <25600=> 25600bps
//      <38400=> 38400bps      <115200=> 115200bps     
//   <i> Default USART5 baudrate 115200        
#define USART5_BaudRate					      115200						  /* 115200bps */

//   <o>Config USART5 controller world length 
//      <0=> Word Length 8bits        <4096=> Word Length 9bits
//   <i> Default USART5 world length 8bits     
#define USART5_WordLength				      0					          /* 8位数据位 */

//   <o>Config USART5 controller Stop Bits 
//      <0=> StopBits 1           <4096=> StopBits 0.5
//      <8192=> StopBits 2        <12288=> StopBits 1.5
//   <i> Default USART5 Stop Bits 1   
#define USART5_StopBits					      0					          /* 1位停止位 */

//   <o>Config USART5 controller Parity 
//      <0=> Parity None   <1024=> Parity Even   <1536=> Parity Odd
//   <i> Default USART5 Parity None   
#define USART5_Parity					        0						        /* 无校验 */

//   <o>Config USART5 controller Tx Control 
//      <0=> Disable Tx    <8=> Enable Tx
//   <i> Default USART5 Enable Tx  
#define USART5_Mode_Tx                8

//   <o>Config USART5 controller Rx Control 
//      <0=> Disable Rx    <4=> Enable Rx
//   <i> Default USART5 Enable Rx 
#define USART5_Mode_Rx                4 

//   <o>Config USART5 controller Hardware Flow Control 
//      <0=> HardwareFlowControl None      <256=> HardwareFlowControl RTS
//      <512=> HardwareFlowControl CTS     <768=> HardwareFlowControl RTS && CTS
//   <i> Default USART5 Parity None   
#define USART5_HardwareFlowControl		0		                /* 无硬件流控制 */

//   <o>Config USART5 controller Receive Timeout (4ms) <2-30:1>
//   <i> Default USART5 Receive Timeout  6ms
#define USART5_RxTimeout              2

//   <o>Config USART5 controller Rx buffer size (bytes) <64-3000:2> 
//   <i> Default USART5 Rx buffer size 128 bytes   
#define USART5_RxBufferSize           128
#endif

// </e>
#endif  /* __USE_USART_5__ */


#if ( __USE_USART_1__ == 1 )||( __USE_USART_2__ == 1 )||( __USE_USART_3__ == 1 )||( __USE_USART_4__ == 1 )||( __USE_USART_5__ == 1 )
#define __USE_USART__		        1

#define USART_FCM_MODEM         2
#else
#define __USE_USART__		0
#endif










/******************************************/
/*               WDG参数[配置]            */
/******************************************/


/* 看门狗设置
 */
#ifndef __USE_WDG__
#define __USE_WDG__               1
#endif 

#if __USE_WDG__ == 1
//   <o>enable watch-dog controller    
//      <1=> WDG_EXTERNAL
//      <2=> WDG_INDEPENDENCY    
//      <3=> WDG_WINDOWS
//   <i> Default watch-dog controller type [1]

#define __USE_WDG_TYPE__         2


#endif  /* __USE_WDG__ */





// ------------- <<< end of configuration section >>> -----------------------
#endif	/* __HARDWARE_RESOURCE_ALLOCA_H__ */




