



#include "worker.h"

#include "modem.h"

#include "lwipopts.h"


#define WIP_LWIP_INIT_DEBUG     0
#define USE_HUAWEI_GSM          0



/******************************************/
/*              内部变量[定义]            */
/******************************************/

/* 模块初始化标志 */
static bool lwip_module_init_ok;

/* APN引用 */
static ascii *lwip_apn = NULL;

/* 巡检定时器 */
static tmr_t *lwip_tmr = NULL;

/* 定义工作对象 */
static worker_t netWorker;





/******************************************/
/*              内部函数[声明]            */
/******************************************/

extern void lwip_init ( void );
extern void sys_check_timeouts ( void );







/* 
 * 功能描述：网络巡检定时器
 * 引用参数：
 *         
 * 返回值  ：
 * 
 */
static void wip_NetMonitor ( u8 ID )
{
#if 0
  sys_check_timeouts ();
#else
  WORKER_StartupWork ( ( worker_t * )&netWorker );
#endif
}
/* 
 * 功能描述：网络巡检定时器关闭
 * 引用参数：
 *         
 * 返回值  ：
 * 
 */
extern void wip_NetMonitor_close(void)
{
		if (lwip_tmr)
    {
      /* MOD #001 修改网络监视超时参数 */
			TMR_UnSubscribe(lwip_tmr,( tmr_procTriggerHdlr_t )wip_NetMonitor,TMR_TYPE_100MS);
			lwip_tmr=NULL;
    }
}

/* 
 * 功能描述：网络接口初始化
 * 引用参数：
 *         
 * 返回值  ：
 * 
 */
extern void GL868_PDP_Command_AddContext ( void* ipc_data, int index, const char *APN );

extern void wip_NetInit ( ascii* apn)
{
  if ( lwip_module_init_ok == FALSE )
  {
    lwip_module_init_ok = TRUE;

    /* 网络工作对象初始化 */
    netWorker.data = NULL;
    netWorker.func = ( work_func_t )sys_check_timeouts;

    lwip_init ();
    
    if ( lwip_tmr == NULL )
    {
      /* MOD #001 修改网络监视超时参数 */
      lwip_tmr = TMR_Subscribe ( TRUE, 3, TMR_TYPE_100MS, ( tmr_procTriggerHdlr_t )wip_NetMonitor );
    }
  }

  if ( MDM_QueryFlowStatus () == MDM_FCM_V24_STATE_AT && apn )
  {
    lwip_apn = apn;
		/*电信：用户名+密码 ---> IP +鉴权*/
    //lgsm_pdp_add_context ( NULL, 1, ( const char* )lwip_apn );
    //GL868_PDP_Command_AddContext ( NULL, 1, ( const char* )lwip_apn );

		ctcc_pdp_add_context(NULL,"ctnet@mycdma.cn","vnet.mobi");


#if  USE_HUAWEI_GSM == 1
    lgsm_pdp_activate ( NULL, 1 );
#endif  /* __USE_HUAWEI_GSM__ */

    lgsm_pdp_request_gprs_service ( NULL, 1 );

 
  }
}









/* 
 * 功能描述：网络接口重新初始化
 * 引用参数：
 *         
 * 返回值  ：
 * 
 */
extern void wip_NetDeInit ( void )
{
  lgsm_pdp_deactivate ( NULL, 1 );
	lwip_module_init_ok =FALSE;

  if ( lwip_apn )
  {

#if 1//WIP_LWIP_INIT_DEBUG == 1
    printf ( "\r\n[PDP EVENT] net deinit start\r\n" );
#endif  /* WIP_LWIP_INIT_DEBUG */

    wip_NetInit ( lwip_apn );
  }
}





extern void wip_ReNetInit ( void )
{
  lgsm_pdp_deactivate ( NULL, 1 );
	
	wip_NetMonitor_close();
	
	lwip_module_init_ok =FALSE;
	
	sys_status=PROC_SYSTEM_REISSUEDATA_UPLOAD;

  printf("\r\n[PDP EVENT] net deinit start\r\n");

  wip_NetInit( lwip_apn );
  
}









/* 
 * 功能描述：PDP激活失败
 * 引用参数：
 *         
 * 返回值  ：
 * 
 */
extern void pdp_activate_error ( void* ipc_data, ascii *errorStr )
{
#if WIP_LWIP_INIT_DEBUG == 1
  printf ( "\r\n[PDP EVENT] pdp activate error\r\n" );
#endif  /* WIP_LWIP_INIT_DEBUG */
  wip_NetDeInit ();
}








/* 
 * 功能描述：GPRS拨号失败
 * 引用参数：
 *         
 * 返回值  ：
 * 
 */
extern void pdp_request_gprs_service_error ( void* ipc_data, ascii *errorStr )
{

#if WIP_LWIP_INIT_DEBUG == 1
  printf ( "\r\n[PDP EVENT] request gprs service error\r\n" );
#endif  /* WIP_LWIP_INIT_DEBUG */
  wip_NetDeInit ();
}











/* 
 * 功能描述：网络退出
 * 引用参数：
 *         
 * 返回值  ：
 * 
 */
extern void wip_NetExit ( void )
{
		SIO_Close();
}



