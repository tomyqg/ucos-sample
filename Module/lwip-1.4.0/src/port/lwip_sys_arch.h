




#ifndef __LWIP_SYS_ARCH_H__
#define __LWIP_SYS_ARCH_H__


#include "stm32f4xx.h"
#include "stdbool.h"



#define __sio_fd_t_defined
typedef signed char sio_fd_t;

#define sio_open        SIO_Open
#define sio_write       SIO_Write
#define sio_read        SIO_Read
#define sio_read_abort  SIO_ReadAbort








///////////////////////////////////////////////////////////////
//#define LWIP_DEBUG
//////////////////////////////////////////////////////////////










/******************************************/
/*              外部函数[声明]            */
/******************************************/

extern void SIO_Open ( void );
extern u32  SIO_Write ( sio_fd_t fd, u8 *Data, u32 DataSize );
extern bool SIO_Read ( u16 DataSize, u8 *Data );
extern void SIO_ReadAbort ( sio_fd_t fd );
extern void SIO_Close ( void );
extern bool SIO_IsOK ( void );

extern void sys_tick ( void );


extern u32  sys_now ( void );
extern u32  sys_jiffies ( void );




#endif  /* __LWIP_SYS_ARCH_H__ */






