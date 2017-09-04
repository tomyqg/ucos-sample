






#ifndef __WIP_FTP_CLIENT_H__
#define __WIP_FTP_CLIENT_H__



#include "stm32f4xx.h"

#include "wip_channel.h"






/******************************************/
/*               FTP����[����]            */
/******************************************/


/* Error codes */
#define FTP_BAD             -2  /* Catch all, socket errors etc. */
#define FTP_NOSUCHHOST      -3  /* The server does not exist. */
#define FTP_BADUSER         -4  /* Username/Password failed */
#define FTP_TOOBIG          -5  /* Out of buffer space or disk space */ 
#define FTP_BADFILENAME     -6  /* The file does not exist */
#define FTP_NOMEMORY        -7  /* Unable to allocate memory for internal buffers */



/* The "User name" and the "Password" parameters can be up to 24 characters each. 
 * The "APN"/ "Phone number" parameters can be up to 50 characters each. */	
#define FTP_SERV_ADDRESS_MAX      64
#define FTP_LOGIN_MAX             24
#define FTP_PASSWORD_MAX          24
#define FTP_FILE_PATH_NAME_MAX    96

/* FTP���ݴ��� */
typedef bool ( *ftp_recvDataHdlr_f ) ( u16 DataSize, u8 *Data );




/******************************************/
/*              �ⲿ����[����]            */
/******************************************/
extern u32 ftp_restart_count;



/* FTP�ͻ������ӷ����� */
extern bool wip_FTPStartConnect ( ascii* ftpSevrAddress, u16 ftpSevrPort, 
                                  ascii* userName, ascii* userPass,
                                  wip_eventHandler_f evHandler, void *ctx ); 

/* FTP�ļ����� */
extern bool wip_FTPDownloadFile ( ascii* file, ftp_recvDataHdlr_f ftpRcvDataHdlr );

/* FTP�Ͽ����� */
extern void wip_FTPDisconnected ( void );

/* ��ȡ�ļ���С */
extern s32  wip_FTPGetFileSize  ( ascii* file );




#endif  /* __WIP_FTP_CLIENT_H__ */



