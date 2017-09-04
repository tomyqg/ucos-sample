#ifndef  _SD_PORT_H_
#define  _SD_PORT_H_


#include "ff.h"
#include "stm32f4xx.h"

extern UINT bw;
extern UINT br;
extern FIL file;
extern FATFS fats;
extern DIR DirInf;
extern FRESULT result;
extern FILINFO fileinfo;

u8 SD_WriteDataSeek(FIL *fp,char *filename, u8*data,u32 len);
u8 SD_WriteDataSeek_Ext(FIL *fp,char *filename, u8*data,u32 len);


extern FRESULT SD_CloseFile(FIL *fp);
extern FRESULT SD_OpenFile(FIL *fp,char*filename);
extern FRESULT SD_WriteFile(FIL *fp,char*filename, u8*data,u32 len );
extern FRESULT SD_WriteFile_Sync(FIL *fp,char*filename, u8*data,u32 len);
#endif

