/*
*********************************************************************************************************
*                                              EXAMPLE CODE
*
*                          (c) Copyright 2003-2013; Micrium, Inc.; Weston, FL
*
*               All rights reserved.  Protected by international copyright laws.
*               Knowledge of the source code may NOT be used to develop a similar product.
*               Please help us continue to provide the Embedded community with the finest
*               software available.  Your honesty is greatly appreciated.
*********************************************************************************************************
*/
#ifndef  __INCLUDES_H__
#define  __INCLUDES_H__

/*
*********************************************************************************************************
*                                         STANDARD LIBRARIES
*********************************************************************************************************
*/

#include  <stdarg.h>
#include  <stdio.h>
#include  <stdlib.h>
#include  <math.h>
#include "stdbool.h"

/*
*********************************************************************************************************
*                                              LIBRARIES
*********************************************************************************************************
*/

#include  <cpu.h>
#include  <lib_def.h>
#include  <lib_ascii.h>
#include  <lib_math.h>
#include  <lib_mem.h>
#include  <lib_str.h>


/*
*********************************************************************************************************
*                                              APP / BSP
*********************************************************************************************************
*/

#include  <bsp.h>


/*
*********************************************************************************************************
*                                                 OS
*********************************************************************************************************
*/

#include  <ucos_ii.h>


enum
{
	MSG_BackPic = 0,		
    MSG_TaskInfo,
    MSG_SaveSR,
    MSG_BackLight,
	MSG_BackPicPressed
};


/* 定义一个邮箱，    这只是一个邮箱指针，  OSMboxCreate函数会创建邮箱必需的资源 */
extern OS_EVENT *AppUserIFMbox;
extern OS_EVENT *AppMsgMbox;

/* 定义一个信号量，  这只是一个信号量指针，OSSEMCreate函数会创建邮箱必需的资源 */
extern OS_EVENT *Semp;
extern OS_EVENT *AppBackpicSemp;


#endif
