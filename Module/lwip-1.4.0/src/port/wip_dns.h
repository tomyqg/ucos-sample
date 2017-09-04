





#ifndef __WIP_DNS_H__
#define __WIP_DNS_H__



#include "stm32f4xx.h"










/******************************************/
/*               DNS参数[配置]            */
/******************************************/

/* DNS数据处理 */
typedef bool ( *dns_procDataHdlr_f ) ( const char *name, u32 *ipaddr );








/******************************************/
/*              外部函数[声明]            */
/******************************************/

/* 查询主机IP地址 */
extern void wip_DNSQueryHostIPAddress ( const char *hostname, dns_procDataHdlr_f dnsProcHdlr );



#endif  /* __WIP_DNS_H__ */




