
#ifndef __SPI__
#define __SPI__

/*
*********************************************************************************************************
* 必须实现
* SPI发送一个字节的宏定义方式
*********************************************************************************************************
*/
#define SPI_Send(data) \
	{while((SPI1->SR & SPI_I2S_FLAG_TXE)==RESET);\
	 SPI1->DR = data;\
	 while((SPI1->SR & SPI_I2S_FLAG_RXNE)==RESET);\
     SPI1->DR;}

/*
*********************************************************************************************************
* 必须实现
* SPI读一个字节的宏定义方式
*********************************************************************************************************
*/
#define SPI_Read(data) \
	{while((SPI1->SR & SPI_I2S_FLAG_TXE)==RESET);\
	 SPI1->DR = 0xff;\
	 while((SPI1->SR & SPI_I2S_FLAG_RXNE)==RESET);\
	 data = SPI1->DR;}

/*
*********************************************************************************************************
* 必须实现
* SPI CS信号的控制，宏定义方式
*********************************************************************************************************
*/
#define SPI_CS_ENABLE           {GPIOA->BRR  = GPIO_Pin_4;}
#define SPI_CS_DISABLE          {GPIOA->BSRR = GPIO_Pin_4;}


//-------------------------------以下为SPI FLASH的宏定义，不需要修改-----------------------------------//
// SPI_FLASH 命令
#define SPI_FLASH_READ          (0x03)
#define SPI_FLASH_64KSE         (0xD8)
#define SPI_FLASH_32KSE         (0x52)
#define SPI_FLASH_4KSE          (0x20)
#define SPI_FLASH_BE            (0xc7)  
#define SPI_FLASH_RDSR          (0x05)
#define SPI_FLASH_RDID          (0x9f)
#define SPI_FLASH_RES           (0x03)
#define SPI_FLASH_EWSR          (0x50)
#define SPI_FLASH_WRSR          (0x01)
#define SPI_FLASH_WREN          (0x06)
#define SPI_FLASH_WRDI          (0x04)
#define SPI_FLASH_WRITE         (0x02)
#define SPI_FLASH_WIP           (0x01<<0)
#define SPI_FLASH_WEL           (0x01<<1)
#define SPI_FLASH_BP0           (0x01<<2)
#define SPI_FLASH_BP1           (0x01<<3)
#define SPI_FLASH_BP2           (0x01<<4)
#define SPI_FLASH_SRWD          (0x01<<7)
#define UINT32     u32
#define UINT16     u16
#define UINT8      u8

void SPIFlash_CS_Enable(void);
void SPIFlash_CS_Disable(void);
char SPIFlash_BurstRead(void);

void SPIFlash_ReadStart(UINT32 flashaddress);
UINT8 SPIFlash_RDID(void);
void SPIFlash_ReleaseWP(void);
UINT8 SPIFlash_ReadOneByte(UINT32 flashaddress);
void SPIFlash_ReadNByte(UINT32 flashaddress, UINT8* buf, UINT32 n);
void SPIFlash_WriteOneByte(UINT32 flashaddress, UINT8 data);
void SPIFlash_WriteNByte(UINT32 flashaddress, UINT8* buf, UINT32 n);
void SPIFlash_4KSE(UINT32 address);
void SPIFlash_32KSE(UINT32 address);

void Init_SPI(void);

#endif /* __SPI__ */
