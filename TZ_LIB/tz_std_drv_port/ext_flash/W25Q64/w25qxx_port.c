#include "tiza_w25qxx.h"
#include "w25qxx_port.h"    
#include "defines.h"



FIFO_FLASH_TYPE  FIFO_FlashBuf; //FIFO缓存
FIFO_FLASH_TYPE *pFlashBuf=&FIFO_FlashBuf; //FIFO指针










  /* 
 ******************************************************************************
 * 功能描述： 读取备份参数
 * 引用参数：
 * 返回值  ： 
 * 创建人  ： goly
 *******************************************************************************
 */
u8 STORAGE_FlashParams_ReadData(u8* pdata,u16 len,u32 address)
{
	u8 buf[4];

	SPI_FLASH_BufRead(buf,address,4);
	if((buf[0]==0xaa)&&(buf[1]==0x55))
	{
		if(len==((buf[2]<<8)|buf[3]))
		{
				SPI_FLASH_BufRead(pdata,address+4,len);
				return 1;
		}
	}
	return 0;	
}
 /* 
 ******************************************************************************
 * 功能描述：写备份参数
 * 引用参数：
 * 返回值  ：
 * 创建人  ：goly
 *******************************************************************************
 */
u8 STORAGE_FlashParams_WriteData(u8* pdata,u16 len,u32 address)
{
	
		INT8U MARK[4]={0xaa,0x55};
		MARK[2]=(len>>8)&0xff;
		MARK[3]=len&0xff;
		SPI_FLASH_SectorErase(address);
	  SPI_FLASH_BufWrite(address,MARK,4);
    SPI_FLASH_BufWrite(address+4,pdata,len);
		return 1;
}


 /* 
 ******************************************************************************
 * 功能描述：写入FIFO FLASH  配置信息
 * 引用参数：
 *		   pFIFO     ：FIFI指针
 * 返回值  ：0:写入失败 1：写入成功
 * 创建人  ：goly
 *******************************************************************************
 */
u8 FIFO_FLASH_OffsetDataConfig_Write(u8* pdata,u16 len)
{ 
		INT8U MARK[4]={0xaa,0x55};
		MARK[2]=(len>>8)&0xff;
		MARK[3]=len&0xff;
		SPI_FLASH_SectorErase(DATA_CONFIG_ADDRESS);
	  SPI_FLASH_BufWrite(DATA_CONFIG_ADDRESS,MARK,4);
    SPI_FLASH_BufWrite(DATA_CONFIG_ADDRESS+4,pdata,len);
		return 1;
}
  

 /* 
 ******************************************************************************
 * 功能描述： 读取FIFO FLASH  配置信息
 * 引用参数：
 *		   			pFIFO：FIFI指针
 * 返回值  ： 0:读取失败 1：读取成功
 * 创建人  ： goly
 *******************************************************************************
 */
u8 FIFO_FLASH_OffsetDataConfig_Read(u8* pdata,u16 len)
{ 
	u8 buf[4];

	SPI_FLASH_BufRead(buf,DATA_CONFIG_ADDRESS,4);
	if((buf[0]==0xaa)&&(buf[1]==0x55))
	{
		if(len==((buf[2]<<8)|buf[3]))
		{
				SPI_FLASH_BufRead(pdata,DATA_CONFIG_ADDRESS+4,len);
				return 1;
		}
	}
	return 0;	
}

 /* 
 ******************************************************************************
 * 功能描述：FIFO FLASH 擦除
 * 引用参数：
 *		   
 * 返回值  ：无
 * 创建人  ：goly
 *******************************************************************************
 */
void FIFO_FLASH_OffsetDataConfig_Clear(void)
{ 
 	SPI_FLASH_SectorErase(DATA_CONFIG_ADDRESS);
}



/****************************************************************************************
 *																																											*
 *	32960补发存储   外部flash FIFO缓存区接口函数  @@@goly																*									
 *																																											*
 ****************************************************************************************
 * 功能描述： 初始化环形缓存
 * 引用参数：
 *		   			pFIFO     ：FIFI指针
 *		   			DataBytes ：每条数据的字节数
 *         		depth	 		：FIFO的深度（可存储的数据条数）
 * 返回值  ： 无
 * 创建人  ： goly
 * *****************************************************************************
 */
 void FIFO_flash_init(FIFO_FLASH_TYPE* pFIFO,u16 data_bytes,u16 flash_depth)
 {
	
	pFIFO->offsetdata=0;
	pFIFO->IsFinishSend=FALSE;
	pFIFO->Depth=flash_depth;  
 	pFIFO->Head=0;
	pFIFO->Tail=0;
	pFIFO->Counter=0;
	pFIFO->DataBytes=data_bytes;
 }

 
 
 /* 
 ******************************************************************************
 * 功能描述：判断FIFO是否为空
 * 引用参数：
 *		   pFIFO ：FIFI指针
 *
 * 返回值  ：1-TRUE ; 0-FALSE;
 * 创建人  ：goly
 *******************************************************************************
 */
u8 FIFO_flash_IsEmpty(FIFO_FLASH_TYPE *pFIFO)  
{  
    return (pFIFO->Counter == 0);  
}

/* 
 ******************************************************************************
 * 功能描述：判断FIFO是否已满
 * 引用参数：
 *		   pFIFO ：FIFI指针
 *
 * 返回值  ：1-TRUE ; 0-FALSE;
 * 创建人  ：goly
 *******************************************************************************
 */
u8 FIFO_flash_IsFull(FIFO_FLASH_TYPE *pFIFO)  
{  
    return (pFIFO->Counter == pFIFO->Depth);  
}

 /* 
 ******************************************************************************
 * 功能描述：清空FIFO
 * 引用参数：
 *		   pFIFO     ：FIFI指针
 * 返回值  ：无
 * 创建人  ：goly
 *******************************************************************************
 */
void FIFO_flash_Clear(FIFO_FLASH_TYPE *pFIFO)  
{ 
    
		pFIFO->Counter = 0;  
    pFIFO->Head = 0;  
    pFIFO->Tail = 0;  
}
 /* 
 ******************************************************************************
 * 功能描述：向flash添加一条记录
 * 引用参数：
 *		   
 * 返回值  ：无
 * 创建人  ：goly
 *******************************************************************************
 */
void FIFO_Add_Flash(u32 index,u16 len,u8* data)
{
		u32 address=0;
	  address=index*DATA_BLOCK_SIZE+DATA_STORAGE_ADDRESS;
		SPI_FLASH_BufWrite(address,data,len);
}

 /* 
 ******************************************************************************
 * 功能描述：从flash取出一条记录
 * 引用参数：
 *		   
 * 返回值  ：无
 * 创建人  ：goly
 *******************************************************************************
 */
void FIFO_Get_Flash(u32 index,u16 len,u8* data)
{	
		u32 address=0;
	  address=index*DATA_BLOCK_SIZE+DATA_STORAGE_ADDRESS;
		SPI_FLASH_BufRead(data,address,len);
}
/* 
 ******************************************************************************
 * 功能描述：向FIFO FLASH添加一条数据
 * 引用参数：
 *		   pFIFO     ：FIFI指针
 *		   pValue    ：要添加的数据指针
 *         
 * 返回值  ：1-TRUE or 0-FALSE
 * 创建人  ：goly
 *******************************************************************************
 */ 
u8 FIFO_flash_AddOne(FIFO_FLASH_TYPE *pFIFO, void *pValue)  
{   
  
    if (FIFO_flash_IsFull(pFIFO))  
    {  
        return 0;  
    }  
	  FIFO_Add_Flash(pFIFO->Tail,pFIFO->DataBytes,(u8 *)pValue);
    pFIFO->Tail ++;  
    if (pFIFO->Tail >= pFIFO->Depth)  
    {  
        pFIFO->Tail = 0;  
    }  
    pFIFO->Counter ++;  
    return 1;  
}

/* 
 ******************************************************************************
 * 功能描述：从FIFO_flash读取一条数据
 * 引用参数：
 *		   pFIFO     ：FIFI指针
 *		   pValue    ：存放已读数据的指针
 *         
 * 返回值  ：1-TRUE or 0-FALSE
 * 创建人  ：goly
 *******************************************************************************
 */
u8 FIFO_flash_GetOne(FIFO_FLASH_TYPE *pFIFO, void *pValue)  
{   
    if (FIFO_flash_IsEmpty(pFIFO))  
    {  
        return 0;  
    }  
  	FIFO_Get_Flash(pFIFO->Head,pFIFO->DataBytes,(u8*)pValue);
    pFIFO->Head ++;  
    if (pFIFO->Head >= pFIFO->Depth)  
    {  
        pFIFO->Head = 0;  
    }  
    pFIFO->Counter --;  
  
    return 1;  
} 
