







#ifndef __WIP_CHANNEL_H__
#define __WIP_CHANNEL_H__






/******************************************/
/*               事件参数[配置]           */
/******************************************/

typedef enum  { 
  WIP_CEV_DONE,
  WIP_CEV_OPEN,
  WIP_CEV_ERROR,
  WIP_CEV_PEER_CLOSE,
  WIP_CEV_PING,
  WIP_CEV_READ,
  WIP_CEV_WRITE,

  WIP_CEV_LAST = WIP_CEV_WRITE
} wip_event_t;


/* 事件接收处理 */
typedef void ( *wip_eventHandler_f )( wip_event_t ev, void *ctx );









#endif  /* __WIP_CHANNEL_H__ */


