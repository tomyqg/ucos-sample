



#ifndef __MDM_DEBUG_H__
#define __MDM_DEBUG_H__








/******************************************/
/*              MDM调试[配置]             */
/******************************************/
/*串口显示AT指令*/
#define    AT_DISPLAY_DEBUG           1


/* MODEM调试总开关 */
#ifndef MDM_INFO_DEBUG
#define MDM_INFO_DEBUG                0

/* 初始化调试 */
#define MDM_INIT_DEBUG                0

/* 监视器调试 */
#define MDM_MONITOR_DEBUG             0

/* AT指令发射调试 */
#define MDM_AT_SEND_DEBUG             1

/* AT指令响应调试 */
#define MDM_AT_HANDLER_DEBUG          0
#endif  /* MDM_INFO_DEBUG */





/* 切换调试 */
#ifndef MDM_SWCTX_DEBUG
#define MDM_SWCTX_DEBUG               0

/* 切换处理调试 */
#define SWCTX_PROCESS_DEBUG           0
#endif  /* MDM_SWCTX_DEBUG */


#endif  /* __MDM_DEBUG_H__ */


