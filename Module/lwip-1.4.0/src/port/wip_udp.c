


#include "tiza_timer_port.h"

#include "lwipopts.h"
#include "lwip/inet.h"
#include "lwip/udp.h"

#include "wip_udp.h"






/* UDP���Կ��� */
#ifndef LWIP_UDP_DEBUG
#define LWIP_UDP_DEBUG     0
#endif  /* LWIP_UDP_DEBUG */



typedef struct
{
  /* �Ƿ���Ч? */
	bool isValid;

  /* �׽������� */
  struct udp_pcb *socket;

  /* �˿� */
  u16 ipPort;

  /* ��ַ */
	struct ip_addr ipAddr;

  /* �¼����մ��� */
  wip_eventHandler_f  procHandler;

  /* �¼� */
  wip_event_t ev;

  /* ��ʱ���� */
  struct pbuf *pbuf;
}	UDP_IfTypedef;






/******************************************/
/*              �ڲ�����[����]            */
/******************************************/

static UDP_IfTypedef udpList[ MEMP_NUM_UDP_PCB ];
static vu8 udpNumber = 0;
static bool udpMsgNotify = FALSE;
static int udpTempSid = 0;
static tmr_t *udp_state_tmr = NULL;






/******************************************/
/*              �ڲ�����[����]            */
/******************************************/

static void wip_UDPSetStatus ( u8 ID );
static void wip_UDPRecvData ( void *arg, struct udp_pcb *pcb, struct pbuf *p,
       struct ip_addr *addr, u16_t port );





/* 
 * ����������
 * ���ò�����
 *         
 * ����ֵ  ��
 * 
 */
static void wip_UDPSetStatus ( u8 ID )
{
  int sock_id = 0;

  if ( udpMsgNotify == TRUE )
  {
    udpMsgNotify = FALSE;
    sock_id = udpTempSid;
  }

  if ( udp_state_tmr )
  {
    TMR_UnSubscribe ( udp_state_tmr, ( tmr_procTriggerHdlr_t )wip_UDPSetStatus, TMR_TYPE_10MS );
    udp_state_tmr = NULL;
  }

  if ( ( sock_id >= 0 )&&( sock_id < MEMP_NUM_UDP_PCB )&&( udpList[ sock_id ].isValid == TRUE ) )
  {
    if ( udpList[ sock_id ].procHandler )
    {
      udpList[ sock_id ].procHandler ( udpList[ sock_id ].ev, NULL );
    }
  }
}









/* 
 * ����������
 * ���ò�����
 *         
 * ����ֵ  ��
 * 
 */
extern s8 wip_UDPCreateOpts ( ascii* remoteIP, u16 remotePort, wip_eventHandler_f evHandler, void *ctx )
{
	if ( remoteIP && remotePort && evHandler )
  {
    if ( udpNumber < MEMP_NUM_UDP_PCB )
    {
      u8 i = 0;

      for ( ; ( i < MEMP_NUM_UDP_PCB )&&( udpList[ i ].isValid == TRUE ); i++ );
      if ( i < MEMP_NUM_UDP_PCB )
      {
        udpList[ i ].socket = udp_new ();
      	if ( udpList[ i ].socket )
      	{
      		s8 errorcode;
          u32 address = inet_addr ( ( const char* )remoteIP );

          //IP4_ADDR ( &udpList[ i ].ipAddr, 218, 94, 153, 146 );
          ip4_addr_set_u32 ( &udpList[ i ].ipAddr, address );
          udpList[ i ].isValid = TRUE;
          udpList[ i ].ipPort = remotePort;
          errorcode = ( s8 )udp_connect ( udpList[ i ].socket, &udpList[ i ].ipAddr, udpList[ i ].ipPort );	
          if ( errorcode != ERR_OK )
          {

#if LWIP_UDP_DEBUG == 1
            printf ( "\r\n***UDP CONN ERROR*** code %d\r\n", errorcode );
#endif  /* LWIP_UDP_DEBUG */

            wip_UDPDestroy ( i );
          }
          else
          {
            udp_recv ( udpList[ i ].socket, wip_UDPRecvData, NULL );
            udpList[ i ].procHandler = evHandler;
            udpList[ i ].ev = WIP_CEV_OPEN;
            udpList[ i ].pbuf = NULL;
            udpNumber++;

            udpMsgNotify = TRUE;
            udpTempSid = i;
            if ( udp_state_tmr == NULL )
            {
              udp_state_tmr = TMR_Subscribe ( FALSE, 1, TMR_TYPE_10MS, ( tmr_procTriggerHdlr_t )wip_UDPSetStatus );

#if LWIP_UDP_DEBUG == 1
              if ( udp_state_tmr > 0 )
              {
                printf ( "\r\nUDP STATE TMR START OK\r\n" );
              }
              else
              {
                printf ( "\r\n***UDP STATE TMR START ERR***\r\n" );  
              }
#endif  /* LWIP_UDP_DEBUG */
            }

            return i;         
          }
      	}
      }
    }
  }

  return -1;
}







/* 
 * ����������
 * ���ò�����
 *         
 * ����ֵ  ��
 * 
 */
static void wip_UDPRecvData ( void *arg, struct udp_pcb *pcb, struct pbuf *p,
     struct ip_addr *addr, u16_t port )
{
  u8 i;

  for ( i = 0; ( i < MEMP_NUM_UDP_PCB ); i++ )
  {
    if ( udpList[ i ].ipAddr.addr == addr->addr && udpList[ i ].ipPort == port )
    {
      udpList[ i ].ev = WIP_CEV_READ;
      udpList[ i ].pbuf = p;
      udpMsgNotify = TRUE;
      udpTempSid = i;
      wip_UDPSetStatus ( NULL );
      udpList[ i ].ev = WIP_CEV_DONE; 
      break;     
    }  
  }

  if ( p )
  {
    pbuf_free ( p );
    p = NULL; 
  }  
}






/* 
 * ����������
 * ���ò�����
 *         
 * ����ֵ  ��
 * 
 */
extern u16 wip_UDPReadData ( s8 sock_id, void *buffer, u16 bufsize )
{
  if ( ( sock_id >= 0 )&&( sock_id < MEMP_NUM_UDP_PCB )&&( udpList[ sock_id ].isValid == TRUE ) )
  {
  	printf("\r\n/* ��---sock_id��%d\r\n",sock_id);
    if ( buffer && bufsize && udpList[ sock_id ].pbuf )
    {
      u16 length = pbuf_copy_partial ( udpList[ sock_id ].pbuf, buffer, bufsize, 0 );
       
      pbuf_free( udpList[ sock_id ].pbuf );
      udpList[ sock_id ].pbuf = NULL;
      return length; 
    }  
  }

  return 0;
}





/* 
 * ����������
 * ���ò�����
 *         
 * ����ֵ  ��
 * 
 */
extern bool wip_UDPWriteData ( s8 sock_id, void *buffer, u16 bufsize )
{
  if ( ( sock_id >= 0 )&&( sock_id < MEMP_NUM_UDP_PCB )&&( udpList[ sock_id ].isValid == TRUE ) )
  {
    s8 errorcode;
    struct pbuf *p = pbuf_alloc ( PBUF_RAW, bufsize, PBUF_ROM );

    if ( p )
    {
      p->payload = buffer;
      p->tot_len = bufsize;

      errorcode = ( s8 )udp_sendto ( udpList[ sock_id ].socket, p, &udpList[ sock_id ].ipAddr, udpList[ sock_id ].ipPort );
      if ( errorcode != ERR_OK ) 
      {

#if LWIP_UDP_DEBUG == 1
        printf ( "\r\n***UDP SEND ERROR*** code %d\r\n", errorcode ); 
#endif  /* LWIP_UDP_DEBUG */

      }
      pbuf_free ( p );
      p = NULL;
      udpList[ sock_id ].ev = WIP_CEV_WRITE;
//      udpMsgNotify = TRUE;
//      udpTempSid = sock_id;
//      wip_UDPSetStatus ( NULL );
      udpList[ sock_id ].ev = WIP_CEV_DONE;  
      return TRUE;    
    }
  }

  return FALSE;
}





/* 
 * ����������
 * ���ò�����
 *         
 * ����ֵ  ��
 * 
 */
extern bool wip_UDPDestroy ( s8 sock_id )
{
  if ( ( sock_id >= 0 )&&( sock_id < MEMP_NUM_UDP_PCB )&&( udpList[ sock_id ].isValid == TRUE ) )
  {
    udp_disconnect ( udpList[ sock_id ].socket );
    udp_remove ( udpList[ sock_id ].socket );

    udpList[ sock_id ].procHandler = NULL;
    udpList[ sock_id ].isValid = FALSE;
    udpList[ sock_id ].socket = NULL;
    udpList[ sock_id ].ipPort = 0;
    IP4_ADDR ( &udpList[ sock_id ].ipAddr, 0, 0, 0, 0 );

    if ( udpList[ sock_id ].pbuf )
    {
      pbuf_free ( udpList[ sock_id ].pbuf );
      udpList[ sock_id ].pbuf = NULL;  
    }
    udpNumber--;
    return TRUE;
  }

  return FALSE;
}


