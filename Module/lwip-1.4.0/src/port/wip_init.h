

#ifndef __WIP_INIT_H__
#define __WIP_INIT_H__






/******************************************/
/*              �ⲿ����[����]            */
/******************************************/

/* ����Э��ջ��ʼ�� */
extern void wip_NetInit ( ascii* apn );

/* ����Э��ջ���γ�ʼ�� */
extern void wip_NetDeInit ( void );
extern void wip_ReNetInit(void);

/* ����Э��ջ�˳� */
extern void wip_NetExit ( void );


extern void pdp_activate_error ( void* ipc_data, ascii *errorStr );
extern void pdp_request_gprs_service_error ( void* ipc_data, ascii *errorStr );




#endif  /* __WIP_INIT_H__ */



