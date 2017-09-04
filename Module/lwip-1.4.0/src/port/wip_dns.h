





#ifndef __WIP_DNS_H__
#define __WIP_DNS_H__



#include "stm32f4xx.h"










/******************************************/
/*               DNS����[����]            */
/******************************************/

/* DNS���ݴ��� */
typedef bool ( *dns_procDataHdlr_f ) ( const char *name, u32 *ipaddr );








/******************************************/
/*              �ⲿ����[����]            */
/******************************************/

/* ��ѯ����IP��ַ */
extern void wip_DNSQueryHostIPAddress ( const char *hostname, dns_procDataHdlr_f dnsProcHdlr );



#endif  /* __WIP_DNS_H__ */




