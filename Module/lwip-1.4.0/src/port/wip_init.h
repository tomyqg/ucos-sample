

#ifndef __WIP_INIT_H__
#define __WIP_INIT_H__






/******************************************/
/*              外部函数[声明]            */
/******************************************/

/* 网络协议栈初始化 */
extern void wip_NetInit ( ascii* apn );

/* 网络协议栈二次初始化 */
extern void wip_NetDeInit ( void );
extern void wip_ReNetInit(void);

/* 网络协议栈退出 */
extern void wip_NetExit ( void );


extern void pdp_activate_error ( void* ipc_data, ascii *errorStr );
extern void pdp_request_gprs_service_error ( void* ipc_data, ascii *errorStr );




#endif  /* __WIP_INIT_H__ */



