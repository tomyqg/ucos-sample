#include "sd_port.h"
#include "stm32f4xx.h"
#include "defines.h"
#include "gb_gprs_protocol.h"

UINT bw;
UINT br;
FIL file;
FATFS fats;
DIR DirInf;
FRESULT result;
FILINFO fileinfo;

static TCHAR path[30];






FRESULT SD_Format(BYTE drv,FATFS *fs)
{
	FRESULT res=f_mount(drv, fs);
	if(res)
	{
		printf("\r\n[SD_Format]: f_mount0: %d \r\n",res);
		goto ERROR;
	}
	res=f_mkfs(drv, 0, 0);
	if(res)
	{
		printf("\r\n[SD_Format]: f_mkfs: %d \r\n",res);
		goto ERROR;
	}
	res=f_mount(drv, NULL);
	if(res)
	{
		printf("\r\n[SD_Format]: f_mount1: %d \r\n",res);
		goto ERROR;
	}
	else
	{
		return res;
	}
	ERROR:
	return res;
}




FRESULT SD_OpenFile(FIL *fp,char*filename)
{
	FRESULT res;
	UINT bw;
	TCHAR path[30];

	sprintf(( char* )path,"0:%s",filename);
	res=f_open(fp,(char*)path,FA_WRITE|FA_READ|FA_OPEN_ALWAYS);
	if(res)
	{
		printf("\r\n[SD_OpenFile]: f_open: %d \r\n",res);
		goto ERROR;
	}
	else
	{
		return res;
	}
	ERROR:	
	return res;
}

FRESULT SD_CloseFile(FIL *fp)
{
	
	FRESULT res=f_close(fp);
	if(res)
	{
		printf("\r\n[SD_OpenFile]: f_close: %d \r\n",res);
		goto ERROR;
	}
	else
	{
		return res;
	}
	ERROR:	
	return res;
}


FRESULT SD_WriteFile(FIL *fp,char*filename, u8*data,u32 len )
{
	UINT bw;
	FRESULT res=f_open(fp,(char*)filename,FA_WRITE|FA_OPEN_ALWAYS);
	if(res)
	{
		printf("\r\n[SD_WriteFile]: f_open: %d \r\n",res);
		goto ERROR;
	}
	DWORD filelen=f_size(fp);
	res = f_lseek(fp,filelen);
	if(res)
	{
		printf("\r\n[SD_WriteFile]: f_lseek: %d \r\n",res);
		goto ERROR;
	}
	res = f_write(fp,data,len,&bw);
	if(res)
	{
		printf("\r\n[SD_WriteFile]: f_write: %d \r\n",res);
		goto ERROR;
	}
	res=f_close(fp);
	if(res)
	{
		printf("\r\n[SD_WriteFile]: f_close: %d \r\n",res);
		goto ERROR;
	}
	else
	{
		return  res;
	}
	ERROR:	
	res=f_close(fp);
	return  res;
}


//FRESULT SD_WriteFile_Sync(FIL *fp,char*filename, u8*data,u32 len)
//{
//	UINT bw;
//  DWORD filelen=f_size(fp);
//	FRESULT res = f_lseek(fp,filelen);
//	if(res)
//	{
//		printf("\r\n[SD_WriteFile_Sync]: f_lseek: %d \r\n",res);
//		goto ERROR;
//	}
//	res = f_write(fp, data, len, &bw); 
//	if(res)
//	{
//		printf("\r\n[SD_WriteFile_Sync]: f_write: %d \r\n",res);
//		goto ERROR;
//	}
//	res=f_sync(fp);
//	if(res)
//	{
//		printf("\r\n[SD_WriteFile_Sync]: f_sync: %d \r\n",res);
//		goto ERROR;
//	}
//	else
//	{
//		return res;
//	}
//	ERROR:
//	return res;
//}




//追加写入
u8 SD_WriteDataSeek(FIL *fp,char*filename, u8*data,u32 len )
{
	FRESULT res;
	UINT bw;

	printf("\r\n [SD WRITE EVENT] everything is ok \r\n");
	
	sprintf(( char* )path,"0:%s",filename);
	//printf("\r\n [SD WRITE EVENT] path:%s\r\n",path);
	res=f_open(fp,(char*)path,FA_WRITE|FA_OPEN_ALWAYS);
	if(!res)
	{
			DWORD filelen=f_size(fp);
			//printf("\r\n filelen : %d  \r\n",(int)filelen);
			res = f_lseek(fp,filelen);	
			if(!res)
			{
					res = f_write(fp,data,len,&bw);
					if(!res)
					{
							//res=f_close(fp);
							res=f_sync(fp);
							if(!res)goto SD_WRITE_OK;
							else 
							{
									printf("\r\n [SD WRITE EVENT] f_close  err : %d \r\n",res);
									goto SD_ERROR;
							}
							
					}else
					{
							printf("\r\n [SD WRITE EVENT] f_write  err : %d \r\n",res);
							goto SD_ERROR;
					}	
			}
			else
			{
					printf("\r\n [SD WRITE EVENT] f_lseek  err : %d \r\n",res);
					goto SD_ERROR;
			}
	}
	else
	{
		
			DWORD filelen=f_size(fp);
			//printf("\r\n filelen : %d  \r\n",(int)filelen);
			res = f_lseek(fp,filelen);	
			if(!res)
			{
					res = f_write(fp,data,len,&bw);
					if(!res)
					{
							//res=f_close(fp);
							res=f_sync(fp);
							if(!res)goto SD_WRITE_OK;
							else 
							{
									printf("\r\n [SD WRITE EVENT] f_close  err : %d \r\n",res);
									goto SD_ERROR;
							}
							
					}else
					{
							printf("\r\n [SD WRITE EVENT] f_write  err : %d \r\n",res);
							goto SD_ERROR;
					}	
			}
			else
			{
					goto SD_ERROR;
			}
			printf("\r\n [SD WRITE EVENT] f_open  err : %d \r\n",res);
			goto SD_ERROR;
	}
	SD_ERROR:	
	//res=f_close(fp);
	return  res;
	SD_WRITE_OK:
	return FR_OK;
}

//追加写入
u8 SD_WriteDataSeek_Ext(FIL *fp,char*filename, u8*data,u32 len )
{
	FRESULT res;
	UINT bw;
	TCHAR path[30];
	printf("\r\n [SD WRITE EVENT] everything is ok \r\n");
	sprintf(( char* )path,"0:%s",filename);
	//printf("\r\n [SD WRITE EVENT] path:%s\r\n",path);
	res=f_open(fp,(char*)path,FA_WRITE|FA_OPEN_ALWAYS);
	if(!res)
	{
			DWORD filelen=f_size(fp);
			//printf("\r\n filelen : %d  \r\n",(int)filelen);
			res = f_lseek(fp,filelen);	
			if(!res)
			{
					res = f_write(fp,data,len,&bw);
					if(!res)
					{
							res=f_sync(fp);
							if(!res)goto SD_WRITE_OK;
							else 
							{
									printf("\r\n [SD WRITE EVENT] f_close  err : %d \r\n",res);
									goto SD_ERROR;
							}
							
					}else
					{
							printf("\r\n [SD WRITE EVENT] f_write  err : %d \r\n",res);
							goto SD_ERROR;
					}	
			}
			else
			{
					printf("\r\n [SD WRITE EVENT] f_lseek  err : %d \r\n",res);
					goto SD_ERROR;
			}
	}
	else
	{
			printf("\r\n [SD WRITE EVENT] f_open  err : %d \r\n",res);
			goto SD_ERROR;
	}
	SD_ERROR:	
	
	res=f_close(fp);
	return  res;
	SD_WRITE_OK:
	return FR_OK;
}
