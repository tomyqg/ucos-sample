
#ifndef _BSP_SPI_FLASH_H
#define _BSP_SPI_FLASH_H
#include "stm32f4xx.h"





#define CMD_AAI       0xAD  	/* AAI �������ָ��(FOR SST25VF016B) */
#define CMD_DISWR	  	0x04		/* ��ֹд, �˳�AAI״̬ */
#define CMD_EWRSR	  	0x50		/* ����д״̬�Ĵ��������� */
#define CMD_WRSR      0x01  	/* д״̬�Ĵ������� */
#define CMD_WREN      0x06		/* дʹ������ */
#define CMD_READ      0x03  	/* ������������ */
#define CMD_RDSR      0x05		/* ��״̬�Ĵ������� */
#define CMD_RDID      0x9F		/* ������ID���� */
#define CMD_SE        0x20		/* ������������ */
#define CMD_BE        0xC7		/* ������������ */
#define DUMMY_BYTE    0xA5		/* ���������Ϊ����ֵ�����ڶ����� */
#define WIP_FLAG      0x01		/* ״̬�Ĵ����е����ڱ�̱�־��WIP) */



#define SPI_FLASH			SPI1

#define ENABLE_SPI_RCC() 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE)

#define SPI_BAUD				SPI_BaudRatePrescaler_2//SPI_BaudRatePrescaler_4		/* ѡ��4��Ƶʱ, SCKʱ�� = 21M */

/* Ƭѡ�����õ�ѡ��  */
#define SF_CS_LOW()			GPIOA->BSRRH = GPIO_Pin_4

/* Ƭѡ�����ø߲�ѡ�� */
#define SF_CS_HIGH()		GPIOA->BSRRL = GPIO_Pin_4






/* ���崮��Flash ID */
enum
{
	SST25VF016B_ID = 0xBF2541,
	MX25L1606E_ID  = 0xC22015,
	W25Q64BV_ID    = 0xEF4017
};

typedef struct
{
	uint32_t ChipID;			/* оƬID */
	char ChipName[16];		/* оƬ�ͺ��ַ�������Ҫ������ʾ */
	uint32_t TotalSize;		/* ������ */
	uint16_t PageSize;		/* ҳ���С */
}SPI_FLASH_T;

void SPI_FLASH_Init(void);
uint32_t SPI_FLASH_ReadID(void);
void SPI_EraseChipALL(void);
void SPI_FLASH_PageWrite(uint8_t * _pBuf, uint32_t _uiWriteAddr, uint16_t _usSize);

/*�ⲿʹ��*/
void SPI_FLASH_SectorErase(uint32_t _uiSectorAddr);
uint8_t SPI_FLASH_BufWrite(uint32_t _uiWriteAddr, uint8_t* _pBuf, uint16_t _usWriteSize);
void SPI_FLASH_BufRead(uint8_t * _pBuf, uint32_t _uiReadAddr, uint32_t _uiSize);


#endif

