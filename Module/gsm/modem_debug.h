



#ifndef __MDM_DEBUG_H__
#define __MDM_DEBUG_H__








/******************************************/
/*              MDM����[����]             */
/******************************************/
/*������ʾATָ��*/
#define    AT_DISPLAY_DEBUG           1


/* MODEM�����ܿ��� */
#ifndef MDM_INFO_DEBUG
#define MDM_INFO_DEBUG                0

/* ��ʼ������ */
#define MDM_INIT_DEBUG                0

/* ���������� */
#define MDM_MONITOR_DEBUG             0

/* ATָ������ */
#define MDM_AT_SEND_DEBUG             1

/* ATָ����Ӧ���� */
#define MDM_AT_HANDLER_DEBUG          0
#endif  /* MDM_INFO_DEBUG */





/* �л����� */
#ifndef MDM_SWCTX_DEBUG
#define MDM_SWCTX_DEBUG               0

/* �л�������� */
#define SWCTX_PROCESS_DEBUG           0
#endif  /* MDM_SWCTX_DEBUG */


#endif  /* __MDM_DEBUG_H__ */


