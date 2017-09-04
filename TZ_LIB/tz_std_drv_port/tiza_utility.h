





#ifndef __TZ_UTILITY_H__
#define __TZ_UTILITY_H__






#include "stm32f4xx.h"
#include "stdbool.h"








/******************************************/
/*              外部函数[声明]            */
/******************************************/

extern u32 inet_addr ( const char *str );
extern char *inet_ntoa ( u32 addr );
extern u32 HexToInt ( ascii *str, u16 len );
extern u32 HexToAscii(u8 *pDst,u8 *pSrc,u32 len,bool newline);
extern u32 DecToInt ( ascii *str, u16 len );
extern unsigned long long DecToLongInt ( ascii *str, u16 len );
extern s32 IsNumber ( const ascii* cNum );
extern s32 ToInt ( const ascii* cNum );
extern s32 stringToInt ( const ascii* cNum,u16 len );
extern float ToFloat ( const ascii *cNum );
extern ascii* strstr_bin ( ascii *str, ascii *part, s32 str_len, s32 part_len );
extern u32 reverseU32 ( u32 i );
extern u16 reverseU16 ( u16 i );



#endif  /* __TZ_UTILITY_H__ */



