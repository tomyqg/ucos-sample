
#ifndef _BSP_SPI_FLASH_H
#define _BSP_SPI_FLASH_H
#include "stm32f4xx.h"





#define CMD_AAI       0xAD  	/* AAI 连续编程指令(FOR SST25VF016B) */
#define CMD_DISWR	  	0x04		/* 禁止写, 退出AAI状态 */
#define CMD_EWRSR	  	0x50		/* 允许写状态寄存器的命令 */
#define CMD_WRSR      0x01  	/* 写状态寄存器命令 */
#define CMD_WREN      0x06		/* 写使能命令 */
#define CMD_READ      0x03  	/* 读数据区命令 */
#define CMD_RDSR      0x05		/* 读状态寄存器命令 */
#define CMD_RDID      0x9F		/* 读器件ID命令 */
#define CMD_SE        0x20		/* 擦除扇区命令 */
#define CMD_BE        0xC7		/* 批量擦除命令 */
#define DUMMY_BYTE    0xA5		/* 哑命令，可以为任意值，用于读操作 */
#define WIP_FLAG      0x01		/* 状态寄存器中的正在编程标志（WIP) */



#define SPI_FLASH			SPI1

#define ENABLE_SPI_RCC() 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE)

#define SPI_BAUD				SPI_BaudRatePrescaler_2//SPI_BaudRatePrescaler_4		/* 选择4分频时, SCK时钟 = 21M */

/* 片选口线置低选中  */
#define SF_CS_LOW()			GPIOA->BSRRH = GPIO_Pin_4

/* 片选口线置高不选中 */
#define SF_CS_HIGH()		GPIOA->BSRRL = GPIO_Pin_4






/* 定义串行Flash ID */
enum
{
	SST25VF016B_ID = 0xBF2541,
	MX25L1606E_ID  = 0xC22015,
	W25Q64BV_ID    = 0xEF4017
};

typedef struct
{
	uint32_t ChipID;			/* 芯片ID */
	char ChipName[16];		/* 芯片型号字符串，主要用于显示 */
	uint32_t TotalSize;		/* 总容量 */
	uint16_t PageSize;		/* 页面大小 */
}SPI_FLASH_T;

void SPI_FLASH_Init(void);
uint32_t SPI_FLASH_ReadID(void);
void SPI_EraseChipALL(void);
void SPI_FLASH_PageWrite(uint8_t * _pBuf, uint32_t _uiWriteAddr, uint16_t _usSize);

/*外部使用*/
void SPI_FLASH_SectorErase(uint32_t _uiSectorAddr);
uint8_t SPI_FLASH_BufWrite(uint32_t _uiWriteAddr, uint8_t* _pBuf, uint16_t _usWriteSize);
void SPI_FLASH_BufRead(uint8_t * _pBuf, uint32_t _uiReadAddr, uint32_t _uiSize);


#endif

