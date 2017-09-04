
#ifndef TM_DEFINES_H
#define TM_DEFINES_H

/* Put your global defines for all libraries here used in your project */
#include "hardware_select_config.h"
#include "tiza_usart.h"
#include "tiza_gpio.h"
#include "tiza_can.h"
#include "tiza_timer.h"
#include "tiza_wdg.h"
#include "tiza_rtc.h"
#include "exfuns.h"
#include "sd_port.h"
#include "low_power.h"
#include "tiza_rtc.h"
#include "tiza_power_control.h"
#include "tiza_gprs_protocol.h"
#include "tiza_led.h"
#include "tiza_init.h"
#include "tiza_adc.h"

//接口头文件
#include "tiza_usart_port.h"
#include "tiza_timer_port.h"
#include "tiza_flash_eeprom_port.h"
#include "tiza_gps_port.h"
#include "worker.h"
#include "tiza_w25qxx.h"
#include "tiza_usart_dma.h"
#include "low_power_port.h"
#include "low_power_proc.h"
#include "tiza_params.h"
#include "tiza_acc.h"



//RTOS相关的任务
#include "os_can.h"
#include "os_usart.h"
#include "os_timer.h"
#include "os_worker.h"

//网络相关
#include "xmit_manager.h"

//协议相关任务
#include "gb_gprs_protocol.h"
#endif
