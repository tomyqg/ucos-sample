




#include "wip_utility.h"
#include "wip_dns.h"


#include "lwipopts.h"
#include "lwip/inet.h"
#include "lwip/dns.h"




	enum{
		FALSE = 0,
		TRUE = !FALSE
	};






/******************************************/
/*              内部变量[定义]            */
/******************************************/

static ip_addr_t dnsserver1   = {0};
static ip_addr_t dnsserver2   = {0};
static ip_addr_t dnsqueryaddr = {0};
 
static dns_procDataHdlr_f dnsprochandler = NULL;








/******************************************/
/*              内部函数[声明]            */
/******************************************/

static void wip_DNSSetUpParams ( void );
static void wip_DNSFound ( const char *name, ip_addr_t *ipaddr, void *callback_arg );















/* 
 * 功能描述：
 * 引用参数：
 *         
 * 返回值  ：
 * 
 */
static void wip_DNSSetUpParams ( void )
{
  if ( dnsserver1.addr == 0 )
  {
    if ( TRUE == wip_UtilityQueryAddress ( WIP_ADDR_TYPE_DNS1_IP, &dnsserver1.addr ) )
    {
      dns_setserver ( 0, &dnsserver1 );
    }  
  }
  

  if ( dnsserver2.addr == 0 )
  {
    if ( TRUE == wip_UtilityQueryAddress ( WIP_ADDR_TYPE_DNS2_IP, &dnsserver2.addr ) )
    {
      dns_setserver ( 0, &dnsserver2 );
    } 
  }
}










/* 
 * 功能描述：
 * 引用参数：
 *         
 * 返回值  ：
 * 
 */
static void wip_DNSFound ( const char *name, ip_addr_t *ipaddr, void *callback_arg )
{
  if ( name && ipaddr ) 
  {
    if ( dnsprochandler )
    {
      dnsprochandler ( name,&ipaddr->addr );
      ipaddr = IP_ADDR_ANY; 
    }
    
    dnsprochandler = NULL; 
  } 
}








/* 
 * 功能描述：
 * 引用参数：
 *         
 * 返回值  ：
 * 
 */
extern void wip_DNSQueryHostIPAddress ( const char *hostname, dns_procDataHdlr_f dnsProcHdlr )
{
  wip_DNSSetUpParams ();

  if ( hostname && dnsProcHdlr )
  {
    s8 errorcode;

    dnsprochandler = dnsProcHdlr;
    errorcode = dns_gethostbyname ( hostname, &dnsqueryaddr, wip_DNSFound, NULL );
    if ( errorcode == ERR_OK )
    {
      if ( dnsprochandler )
      {
        dnsprochandler ( hostname, (u32*)&dnsqueryaddr.addr );
        dnsqueryaddr.addr = 0; 
      }
      
      dnsprochandler = NULL;     
    }
    else
    {
      //printf ( "\r\n***DNS GET HOST NAME ERROR*** code %d\r\n", errorcode );
    }
  }
}


