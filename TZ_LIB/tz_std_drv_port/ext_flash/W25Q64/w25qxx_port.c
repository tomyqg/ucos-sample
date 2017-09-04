#include "tiza_w25qxx.h"
#include "w25qxx_port.h"    
#include "defines.h"



FIFO_FLASH_TYPE  FIFO_FlashBuf; //FIFO����
FIFO_FLASH_TYPE *pFlashBuf=&FIFO_FlashBuf; //FIFOָ��










  /* 
 ******************************************************************************
 * ���������� ��ȡ���ݲ���
 * ���ò�����
 * ����ֵ  �� 
 * ������  �� goly
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
 * ����������д���ݲ���
 * ���ò�����
 * ����ֵ  ��
 * ������  ��goly
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
 * ����������д��FIFO FLASH  ������Ϣ
 * ���ò�����
 *		   pFIFO     ��FIFIָ��
 * ����ֵ  ��0:д��ʧ�� 1��д��ɹ�
 * ������  ��goly
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
 * ���������� ��ȡFIFO FLASH  ������Ϣ
 * ���ò�����
 *		   			pFIFO��FIFIָ��
 * ����ֵ  �� 0:��ȡʧ�� 1����ȡ�ɹ�
 * ������  �� goly
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
 * ����������FIFO FLASH ����
 * ���ò�����
 *		   
 * ����ֵ  ����
 * ������  ��goly
 *******************************************************************************
 */
void FIFO_FLASH_OffsetDataConfig_Clear(void)
{ 
 	SPI_FLASH_SectorErase(DATA_CONFIG_ADDRESS);
}



/****************************************************************************************
 *																																											*
 *	32960�����洢   �ⲿflash FIFO�������ӿں���  @@@goly																*									
 *																																											*
 ****************************************************************************************
 * ���������� ��ʼ�����λ���
 * ���ò�����
 *		   			pFIFO     ��FIFIָ��
 *		   			DataBytes ��ÿ�����ݵ��ֽ���
 *         		depth	 		��FIFO����ȣ��ɴ洢������������
 * ����ֵ  �� ��
 * ������  �� goly
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
 * �����������ж�FIFO�Ƿ�Ϊ��
 * ���ò�����
 *		   pFIFO ��FIFIָ��
 *
 * ����ֵ  ��1-TRUE ; 0-FALSE;
 * ������  ��goly
 *******************************************************************************
 */
u8 FIFO_flash_IsEmpty(FIFO_FLASH_TYPE *pFIFO)  
{  
    return (pFIFO->Counter == 0);  
}

/* 
 ******************************************************************************
 * �����������ж�FIFO�Ƿ�����
 * ���ò�����
 *		   pFIFO ��FIFIָ��
 *
 * ����ֵ  ��1-TRUE ; 0-FALSE;
 * ������  ��goly
 *******************************************************************************
 */
u8 FIFO_flash_IsFull(FIFO_FLASH_TYPE *pFIFO)  
{  
    return (pFIFO->Counter == pFIFO->Depth);  
}

 /* 
 ******************************************************************************
 * �������������FIFO
 * ���ò�����
 *		   pFIFO     ��FIFIָ��
 * ����ֵ  ����
 * ������  ��goly
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
 * ������������flash���һ����¼
 * ���ò�����
 *		   
 * ����ֵ  ����
 * ������  ��goly
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
 * ������������flashȡ��һ����¼
 * ���ò�����
 *		   
 * ����ֵ  ����
 * ������  ��goly
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
 * ������������FIFO FLASH���һ������
 * ���ò�����
 *		   pFIFO     ��FIFIָ��
 *		   pValue    ��Ҫ��ӵ�����ָ��
 *         
 * ����ֵ  ��1-TRUE or 0-FALSE
 * ������  ��goly
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
 * ������������FIFO_flash��ȡһ������
 * ���ò�����
 *		   pFIFO     ��FIFIָ��
 *		   pValue    ������Ѷ����ݵ�ָ��
 *         
 * ����ֵ  ��1-TRUE or 0-FALSE
 * ������  ��goly
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
