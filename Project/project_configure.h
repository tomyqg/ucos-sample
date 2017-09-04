

#ifndef __PROJECT_CONFIGURE_H__
#define __PROJECT_CONFIGURE_H__


#include "tiza_can_port.h"

typedef unsigned char  ascii;

/***************************************************************
 *                     Ӳ��֧��ѡ����                          *
 ***************************************************************/
#define   USE_GSM_MODEM_XXX   1     //GMS�ͺ�


/***************************************************************
 *                       �� Ŀ ѡ �� ��                        *
 ***************************************************************/

#define	  DFZL_PROJECT		  1
//#define	  LDFZ_PROJECT		  1



/***************************************************************
 *                   2 3 2 �� �� �� �� Ŀ                      *
 ***************************************************************/



/***************************************************************
 *                   C A N �� �� �� �� Ŀ                      *
 ***************************************************************/



/*��������*/
#if defined DFZL_PROJECT
 #if DFZL_PROJECT == 1
#include "DFZL_configure.h"
#endif
#endif  /* DFZL_PROJECT */

/*½�ط���*/
#if defined  LDFZ_PROJECT
#if LDFZ_PROJECT == 1
#include "LDFZ_configure.h"
#endif
#endif  /* LDFZ_PROJECT */





#endif  /* __PROJECT_CONFIGURE_H__ */



