





#ifndef __SHELL_H__
#define __SHELL_H__


#include "stm32f4xx.h"
#include "stdbool.h"




/******************************************/
/*              接口类型[配置]            */
/******************************************/

/* Command Types */
#define ADL_CMD_TYPE_PARA	  0x0100  /* 参数类型 */
#define ADL_CMD_TYPE_TEST	  0x0200  /* 测试类型 */
#define ADL_CMD_TYPE_READ	  0x0400  /* 读取类型 */
#define ADL_CMD_TYPE_ACT    0x0800  /* 执行类型 */
#define ADL_CMD_TYPE_ROOT   0x1000  /* 类类型 */

/* 参数选项获取接口 */
#define ADL_GET_PARAM(_P_,_i_)      (( ascii * )SHELL_CmdParamsGet ( _P_, ( _i_ ) ))



typedef struct
{
  /* 类型 */
  u16 type;

  /* 消息长度 */
  u16 length;

  /* 响应 */
  ascii *rsp;
} SHELL_CmdParserTypedef;


/* Incoming command handler callback type */ 
typedef void ( *shell_atCmdHandler_t ) ( SHELL_CmdParserTypedef * );





/******************************************/
/*              外部函数[声明]            */
/******************************************/

/* 注册用户自定义指令 */
extern bool SHELL_CmdSubscribe ( ascii *Cmdstr, shell_atCmdHandler_t Cmdhdlr, u16 Cmdopt );

/* 注销用户自定义指令 */
extern bool SHELL_CmdUnSubscribe ( ascii *Cmdstr, shell_atCmdHandler_t Cmdhdlr );

/* 用户指令数据解析 */
extern bool SHELL_CmdParser ( u16 DataSize, u8 *Data );

/* 用户指令参数获取 */
extern ascii* SHELL_CmdParamsGet ( ascii *response, u32 paramsIndex );



#endif  /* __SHELL_H__ */


