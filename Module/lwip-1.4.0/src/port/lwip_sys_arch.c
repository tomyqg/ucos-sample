



#include <stdio.h>
#include <string.h>

#include "stm32f4xx.h"

//#include "general.h"
#include "GSM_xx.h"
#include "arch/cc.h"
#include "ppp.h"
#include "modem.h"







#define PPP_LINK_STATUS_DEBUG   0





/******************************************/
/*              内部变量[定义]            */
/******************************************/

static sio_fd_t  sio_fd = -1;
static vu32 sys_tick_counter;
static s32 sio_err_status = PPPERR_OPEN;





/******************************************/
/*         函数原型[声明]                 */
/******************************************/

static void ppplinkStatusCB ( void *ctx, int errCode, void *arg );








/* 
 * 功能描述：
 * 引用参数：无
 *          
 * 返回值  ：状态码
 * 
 */
extern void SIO_Open ( void )
{
  if ( sio_fd == -1 )
  {
    pppSetAuth ( PPPAUTHTYPE_PAP, "", "" );
    pppOpen ( 0, ppplinkStatusCB, NULL ); 
    sio_fd = 0;
  }
}








/* 
 * 功能描述：
 * 引用参数：无
 *          
 * 返回值  ：状态码
 * 
 */
extern bool SIO_Read ( u16 DataSize, u8 *Data )
{
  if ( sio_fd >= 0 )
  {
  	pppos_input ( sio_fd, ( u_char * )Data, DataSize );
  }
  return TRUE;
}









/* 
 * 功能描述：
 * 引用参数：无
 *          
 * 返回值  ：状态码
 * 
 */
extern u32 SIO_Write ( sio_fd_t fd, u8 *Data, u32 DataSize )
{
  if ( fd == sio_fd )
  {
		if ( TRUE == MODEM_WriteData ( DATA_Flow_Type, Data, DataSize ) )
    {
    	return DataSize;	
    }
  }
  
  return 0;
}







/* 
 * 功能描述：注意该函数会被执行多次
 * 引用参数：无
 *          
 * 返回值  ：状态码
 * 
 */
extern void SIO_ReadAbort ( sio_fd_t fd )
{
  if ( fd == sio_fd )
  {
    MODEM_SwitchToATStatus ( MDM_Switch_Type_Active );
  }
}







/* 
 * 功能描述：关闭通道
 * 引用参数：无
 * @goly        
 * 返回值  ：状态码
 * 
 */
extern void SIO_Close ( void )
{
  if ( sio_fd >= 0 )
  {
    pppClose ( sio_fd );
		
  }
}









/* 
 * 功能描述：
 * 引用参数：无
 *          
 * 返回值  ：状态码
 * 
 */
extern bool SIO_IsOK ( void )
{
  return ( sio_err_status == PPPERR_NONE ) ? TRUE : FALSE;
}





extern void XMIT_Startup ( u8 id );







/* 
 * 功能描述：
 * 引用参数：无
 *          
 * 返回值  ：状态码
 * 
 */
static void ppplinkStatusCB ( void *ctx, int errCode, void *arg )
{
  switch ( errCode )
  {
    case PPPERR_NONE:
#if PPP_LINK_STATUS_DEBUG == 1
      printf ( "\r\n[PPP EVENT] ERR_NONE\r\n" );
#endif  /* PPP_LINK_STATUS_DEBUG */

      sio_err_status = PPPERR_NONE;

#if 1
      /* 危险!!!!!!! */
     XMIT_Startup ( 0 );	 

#endif 
      break;

    case PPPERR_PARAM:
#if PPP_LINK_STATUS_DEBUG == 1
      printf ( "\r\n[PPP EVENT] ERR_PARAM\r\n" );
#endif  /* PPP_LINK_STATUS_DEBUG */

      sio_err_status = PPPERR_PARAM;
      sio_fd = -1;
      break;

    case PPPERR_OPEN:
#if PPP_LINK_STATUS_DEBUG == 1
      printf ( "\r\n[PPP EVENT] ERR_OPEN\r\n" );
#endif  /* PPP_LINK_STATUS_DEBUG */

      sio_err_status = PPPERR_OPEN;
      sio_fd = -1;
      break;

    case PPPERR_DEVICE:
#if PPP_LINK_STATUS_DEBUG == 1
      printf ( "\r\n[PPP EVENT] ERR_DEVICE\r\n" );
#endif  /* PPP_LINK_STATUS_DEBUG */

      sio_err_status = PPPERR_DEVICE;
      sio_fd = -1;
      break;

    case PPPERR_ALLOC:
#if PPP_LINK_STATUS_DEBUG == 1
      printf ( "\r\n[PPP EVENT] ERR_ALLOC\r\n" );
#endif  /* PPP_LINK_STATUS_DEBUG */

      sio_err_status = PPPERR_ALLOC;
      sio_fd = -1;
      break;

    case PPPERR_USER:
#if PPP_LINK_STATUS_DEBUG == 1
      printf ( "\r\n[PPP EVENT] ERR_USER\r\n" );
#endif  /* PPP_LINK_STATUS_DEBUG */

      sio_err_status = PPPERR_USER;
      sio_fd = -1;
      break;

    case PPPERR_CONNECT:
#if PPP_LINK_STATUS_DEBUG == 1
      printf ( "\r\n[PPP EVENT] ERR_CONNECT\r\n" );
#endif  /* PPP_LINK_STATUS_DEBUG */

      sio_err_status = PPPERR_CONNECT;
      break;

    case PPPERR_AUTHFAIL:
#if PPP_LINK_STATUS_DEBUG == 1
      printf ( "\r\n[PPP EVENT] ERR_AUTHFAIL\r\n" );
#endif  /* PPP_LINK_STATUS_DEBUG */

      sio_err_status = PPPERR_AUTHFAIL;
      break;

    case PPPERR_PROTOCOL:
#if PPP_LINK_STATUS_DEBUG == 1
      printf ( "\r\n[PPP EVENT] ERR_PROTOCOL\r\n" );
#endif  /* PPP_LINK_STATUS_DEBUG */

      sio_err_status = PPPERR_PROTOCOL;
      break;
  }   
}








/* 
 * 功能描述：
 * 引用参数：无
 *          
 * 返回值  ：状态码
 * 
 */
extern u32 sys_jiffies ( void )
{
  return sys_tick_counter;
}





/* 
 * 功能描述：
 * 引用参数：无
 *          
 * 返回值  ：状态码
 * 
 */
extern u32 sys_now ( void )
{
  return sys_tick_counter;
}






/* 
 * 功能描述：
 * 引用参数：无
 *          
 * 返回值  ：状态码
 * 
 */
extern void sys_tick ( void )
{
  if ( sys_tick_counter >= 0xFFFFFFFA )
  {
    sys_tick_counter = 0;

#if defined NO_SYS_NO_TIMERS
#if NO_SYS_NO_TIMERS == 0
    sys_restart_timeouts (); 
#endif
#endif  /* NO_SYS_NO_TIMERS */
 
  }
  sys_tick_counter++;
}






#include "lwip/sys.h"




/* 
 * 功能描述：
 * 引用参数：无
 *          
 * 返回值  ：状态码
 * 
 */
extern sys_prot_t sys_arch_protect ( void )
{

#if OS_CRITICAL_METHOD == 3                           /* Allocate storage for CPU status register      */
    OS_CPU_SR  cpu_sr = 0;
#endif

	OS_ENTER_CRITICAL();
	return 1;
}








/* 
 * 功能描述：
 * 引用参数：无
 *          
 * 返回值  ：状态码
 * 
 */
extern void sys_arch_unprotect ( sys_prot_t pval )
{

#if OS_CRITICAL_METHOD == 3                           /* Allocate storage for CPU status register      */
    OS_CPU_SR  cpu_sr = 0;
#endif
	( void ) pval;

	OS_EXIT_CRITICAL();
}


