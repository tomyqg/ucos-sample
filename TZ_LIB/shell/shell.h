





#ifndef __SHELL_H__
#define __SHELL_H__


#include "stm32f4xx.h"
#include "stdbool.h"




/******************************************/
/*              �ӿ�����[����]            */
/******************************************/

/* Command Types */
#define ADL_CMD_TYPE_PARA	  0x0100  /* �������� */
#define ADL_CMD_TYPE_TEST	  0x0200  /* �������� */
#define ADL_CMD_TYPE_READ	  0x0400  /* ��ȡ���� */
#define ADL_CMD_TYPE_ACT    0x0800  /* ִ������ */
#define ADL_CMD_TYPE_ROOT   0x1000  /* ������ */

/* ����ѡ���ȡ�ӿ� */
#define ADL_GET_PARAM(_P_,_i_)      (( ascii * )SHELL_CmdParamsGet ( _P_, ( _i_ ) ))



typedef struct
{
  /* ���� */
  u16 type;

  /* ��Ϣ���� */
  u16 length;

  /* ��Ӧ */
  ascii *rsp;
} SHELL_CmdParserTypedef;


/* Incoming command handler callback type */ 
typedef void ( *shell_atCmdHandler_t ) ( SHELL_CmdParserTypedef * );





/******************************************/
/*              �ⲿ����[����]            */
/******************************************/

/* ע���û��Զ���ָ�� */
extern bool SHELL_CmdSubscribe ( ascii *Cmdstr, shell_atCmdHandler_t Cmdhdlr, u16 Cmdopt );

/* ע���û��Զ���ָ�� */
extern bool SHELL_CmdUnSubscribe ( ascii *Cmdstr, shell_atCmdHandler_t Cmdhdlr );

/* �û�ָ�����ݽ��� */
extern bool SHELL_CmdParser ( u16 DataSize, u8 *Data );

/* �û�ָ�������ȡ */
extern ascii* SHELL_CmdParamsGet ( ascii *response, u32 paramsIndex );



#endif  /* __SHELL_H__ */


