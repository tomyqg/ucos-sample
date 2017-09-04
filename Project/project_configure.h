

#ifndef __PROJECT_CONFIGURE_H__
#define __PROJECT_CONFIGURE_H__


#include "tiza_can_port.h"

typedef unsigned char  ascii;

/***************************************************************
 *                     硬件支持选择器                          *
 ***************************************************************/
#define   USE_GSM_MODEM_XXX   1     //GMS型号


/***************************************************************
 *                       项 目 选 择 器                        *
 ***************************************************************/

#define	  DFZL_PROJECT		  1
//#define	  LDFZ_PROJECT		  1



/***************************************************************
 *                   2 3 2 控 制 器 项 目                      *
 ***************************************************************/



/***************************************************************
 *                   C A N 控 制 器 项 目                      *
 ***************************************************************/



/*东风汽车*/
#if defined DFZL_PROJECT
 #if DFZL_PROJECT == 1
#include "DFZL_configure.h"
#endif
#endif  /* DFZL_PROJECT */

/*陆地方舟*/
#if defined  LDFZ_PROJECT
#if LDFZ_PROJECT == 1
#include "LDFZ_configure.h"
#endif
#endif  /* LDFZ_PROJECT */





#endif  /* __PROJECT_CONFIGURE_H__ */



