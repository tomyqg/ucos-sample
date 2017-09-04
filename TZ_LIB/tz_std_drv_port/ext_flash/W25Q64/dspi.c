
#include "general.h"

/*
*********************************************************************************************************
* 必须实现
* SPI接口初始化
*********************************************************************************************************
*/
void Init_SPI(void)
{
   	GPIO_InitTypeDef  GPIO_InitStructure;
	SPI_InitTypeDef   SPI_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB |
		RCC_APB2Periph_SPI1, ENABLE);
	/* Configure SPI1 pins: SCK, MISO and MOSI ---------------------------------*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	//CS
		  /* Configure Pa8as outputs push-pull, max speed 50 MHz               */
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	/* SPI1 Config -------------------------------------------------------------*/
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
	// SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
	//SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;     //NSS使用软件模拟
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;//SPI18M
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	// SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_LSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;
	SPI_Init(SPI1, &SPI_InitStructure);
	/* Enable peripheral clocks --------------------------------------------------*/
	/* GPIOA, GPIOB and SPI1 clock enable */	
	/* Enable SPI1 */
	SPI_Cmd(SPI1, ENABLE);
}

/*
*********************************************************************************************************
* 必须实现
* SPI的CS信号控制
*********************************************************************************************************
*/
void SPIFlash_CS_Enable(void)
{
    SPI_CS_ENABLE;
}

void SPIFlash_CS_Disable(void)
{
    SPI_CS_DISABLE;
}

/*
*********************************************************************************************************
* 必须实现
* SPI读取一个字节数据
*********************************************************************************************************
*/
char SPIFlash_BurstRead(void)
{
    while((SPI1->SR & SPI_I2S_FLAG_TXE)==RESET);
    SPI1->DR = 0xff;
    while((SPI1->SR & SPI_I2S_FLAG_RXNE)==RESET);
    return SPI1->DR;
}

//--------------------------------以下为SPI FLASH的驱动，不需要修改------------------------------------//

/*
*********************************************************************************************************
*                                    读FLASH WIND ID
*********************************************************************************************************
*/
UINT8 SPIFlash_RDID(void)
{   
    UINT8 Snum = 0; 
    
    SPI_CS_DISABLE;
    SPI_CS_ENABLE;
    SPI_Send(SPI_FLASH_RDID); // 写命令字 

    //SPI_Send(0x0000); // 写地址
    //SPI_Send(0x0000);
    //SPI_Send(0x0000);
    SPI_Read(Snum);
    SPI_CS_DISABLE;
    
    return Snum;
}

void SPIFlash_EWSR(void)
{
    UINT8 temp = 0xff;
    (void)temp;

    SPI_CS_DISABLE;
    SPI_CS_ENABLE;
    SPI_Send(SPI_FLASH_EWSR);
    SPI_CS_DISABLE; 
}

void SPIFlash_WRSR(UINT8 data)
{
    UINT8 temp;

    SPIFlash_EWSR();
    for(temp=0; temp<100;){temp++;}

    SPI_CS_DISABLE;
    SPI_CS_ENABLE;
    SPI_Send(SPI_FLASH_WRSR);
    SPI_Send(data);
    SPI_CS_DISABLE;
}

/*
*********************************************************************************************************
*                                    解除FLASH写保护
*********************************************************************************************************
*/
void SPIFlash_ReleaseWP(void)
{
    SPIFlash_WRSR(0x02); //解除SPI写保护状态
}

/*
*********************************************************************************************************
*                                   读写使能及命令函数
*********************************************************************************************************
*/

UINT8 SPIFlash_RDSR(void)
{   
    UINT8 temp;
    
    for(temp=0; temp<50;){temp++;}
   
    SPI_CS_DISABLE;
    SPI_CS_ENABLE;
    SPI_Send(SPI_FLASH_RDSR); // 写命令字 
    SPI_Read(temp); // 返回状态
    SPI_CS_DISABLE;
    
    return temp;
}

UINT8 SPIFlash_WREN(void)
{
    UINT8 temp;
    (void)temp;
        
    SPI_CS_DISABLE;
    SPI_CS_ENABLE;
    SPI_Send(SPI_FLASH_WREN);
    SPI_CS_DISABLE;
    
    while((SPIFlash_RDSR() & SPI_FLASH_WEL) == 0)
    {
        SPI_CS_ENABLE;
        SPI_Send(SPI_FLASH_WREN);
        SPI_CS_DISABLE;
    }

    return 1;
}

UINT8 SPIFlash_WRDI(void)
{
    UINT8 temp;
    
    SPI_CS_DISABLE;
    SPI_CS_ENABLE;
    SPI_Send(SPI_FLASH_WRDI);
    for(temp=0; temp<100;){temp++;}
    SPI_CS_DISABLE;
    
    if((SPIFlash_RDSR() & SPI_FLASH_WEL) == 0)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

/*
*********************************************************************************************************
*                                    读数据
*********************************************************************************************************
*/
void SPIFlash_ReadStart(UINT32 flashaddress)
{
    UINT8 temp;
    (void)temp;

    SPI_CS_DISABLE;
    SPI_CS_ENABLE;
    SPI_Send(SPI_FLASH_READ);
    SPI_Send((flashaddress>>16)&0xff);
    SPI_Send((flashaddress>>8)&0xff);
    SPI_Send((flashaddress)&0xff);
}

void SPIFlash_ReadEnd(void)
{
    SPI_CS_DISABLE;
}


UINT8 SPIFlash_ReadOneByte(UINT32 flashaddress)
{
    UINT8 readdata;

    SPIFlash_ReadStart(flashaddress);
    SPI_Read(readdata);
    SPI_CS_DISABLE;
    return(readdata);
}


void SPIFlash_ReadNByte(UINT32 flashaddress, UINT8* buf, UINT32 n)
{
    UINT32 i;

    SPIFlash_ReadStart(flashaddress);
    for(i=0; i<n; i++)
    {
        SPI_Read(buf[i]);
    }
    SPIFlash_ReadEnd();
}

/*
*********************************************************************************************************
*                                    写数据
*********************************************************************************************************
*/
void SPIFlash_WriteStart(UINT32 flash_addr)
{
    UINT8 temp;
    (void)temp;

    SPIFlash_WREN();
    SPI_CS_DISABLE;
    SPI_CS_ENABLE;
    SPI_Send(SPI_FLASH_WRITE);
    SPI_Send((flash_addr>>16)&0xff);
    SPI_Send((flash_addr>>8)&0xff);
    SPI_Send((flash_addr)&0xff);
}

void SPIFlash_WriteEnd(void)
{
    UINT8 temp = 0xff;
    
    SPI_CS_DISABLE;
    temp = SPIFlash_RDSR();
    while(temp & SPI_FLASH_WIP) //查询WIP  
    {
        temp = SPIFlash_RDSR();
    }
}

void SPIFlash_WriteOneByte(UINT32 flashaddress, UINT8 data)
{
    UINT8 temp = 0xff;
    (void)temp;
    SPIFlash_WREN();
    SPI_CS_DISABLE;
    SPI_CS_ENABLE;
    SPI_Send(SPI_FLASH_WRITE);
    SPI_Send((flashaddress>>16)&0xff);
    SPI_Send((flashaddress>>8)&0xff);
    SPI_Send((flashaddress)&0xff);
    SPI_Send(data);
    SPI_CS_DISABLE;
    
    while(SPIFlash_RDSR()&SPI_FLASH_WIP){}
}

void SPIFlash_WriteNByte(UINT32 flashaddress, UINT8* buf, UINT32 n)
{
    UINT32 i;

    if(n == 0)
    {
        n = strlen((const char*)buf);
    }

    for(i=0; i<n; i++)
    {
        SPIFlash_WriteOneByte(flashaddress+i, buf[i]);
    }   
}


/*
*********************************************************************************************************
*                                    擦除FLASH
*********************************************************************************************************
*/
void SPIFlash_4KSE(UINT32 address)
{
    UINT8 temp = 0xff;
    (void)temp;

    if(SPIFlash_WREN() == 0)
    {
    }
    SPI_CS_DISABLE;
    SPI_CS_ENABLE;
    
    SPI_Send(SPI_FLASH_4KSE);
    SPI_Send(((UINT8)(address>>16)));
    SPI_Send(((UINT8)(address>>8)));
    SPI_Send(((UINT8)(address)));

    SPI_CS_DISABLE;

    while(SPIFlash_RDSR() & SPI_FLASH_WIP){}    
}

void SPIFlash_32KSE(UINT32 address)
{
    UINT8 temp = 0xff;
    (void)temp;

    if(SPIFlash_WREN() == 0)
    {
    }
    SPI_CS_DISABLE;
    SPI_CS_ENABLE;

    SPI_Send(SPI_FLASH_32KSE);
    SPI_Send((UINT8)(address>>16));
    SPI_Send((UINT8)(address>>8));
    SPI_Send((UINT8)(address));

    SPI_CS_DISABLE;

    while(SPIFlash_RDSR() & SPI_FLASH_WIP){}

}

void SPIFlash_ALLSE(void)
{
    UINT8 temp = 0xff;
    UINT16 i;
    (void)temp;

    if(SPIFlash_WREN()==0){}

    for(i=0; i<256;){i++;}
    SPI_CS_DISABLE;
    SPI_CS_ENABLE;
    for(i=0; i<256;){i++;}

    SPI_Send(SPI_FLASH_BE);

    for(i=0; i<256;){i++;}
    SPI_CS_DISABLE;
    for(i=0; i<256;){i++;}
    
    while(SPIFlash_RDSR() & SPI_FLASH_WIP){}
}

