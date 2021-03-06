/*
*********************************************************************************************************
*
*	模块名称 : 底层驱动模块
*	文件名称 : bsp.h
*	版    本 : V1.0
*	说    明 : 这是底层驱动模块所有的h文件的汇总文件。
*	 	       应用程序只需 #include bsp.h 即可，不需要#include 每个模块的 h 文件
*
*	修改记录 :
*		版本号  日期         作者    说明
*		v1.0    2012-12-17  Eric2013  ST固件库V1.0.2版本。
*	
*********************************************************************************************************
*/

#ifndef _BSP_H_
#define _BSP_H


#define STM32_X4


#include "stm32f4xx.h"
#include <stdio.h>			/* 因为用到了printf函数，所以必须包含这个文件 */

/* 通过取消注释或者添加注释的方式控制是否包含底层驱动模块 */
#include "bsp_sdio_sd.h"



void bsp_Init(void);
void bsp_DelayUS(uint32_t _ulDelayTime);
void BSP_Tick_Init (void);
void NVIC_Configuration(void);

#endif
