#include "tiza_w25qxx.h"
#include "w25qxx_port.h"
#include <string.h>
#include "tiza_wdg.h"

SPI_FLASH_T sflash;

void Get_FLASH_Info(void);
static void SPI_FLASH_Config(void);
static uint8_t SPI_FLASH_SendByte(uint8_t _ucValue);
static void SPI_FLASH_WriteEnable(void);
static void SPI_FLASH_WriteStatus(uint8_t _ucValue);
static void SPI_FLASH_WaitForWriteEnd(void);
static uint8_t SPI_FLASH_NeedErase(uint8_t * _ucpOldBuf, uint8_t *_ucpNewBuf, uint16_t _uiLen);
static uint8_t SPI_FLASH_CmpData(uint32_t _uiSrcAddr, uint8_t *_ucpTar, uint32_t _uiSize);
static uint8_t SPI_FLASH_AutoWritePage(uint8_t *_ucpSrc, uint32_t _uiWrAddr, uint16_t _usWrLen);

static uint8_t SPI_FLASH_WriteBuffer[SPI_FLASH_PageSize];	






/*
*********************************************************************************************************
*	�� �� ��: bsp_InitSpiFlash
*	����˵��: ��ʼ������FlashӲ���ӿڣ�����STM32��SPIʱ�ӡ�GPIO)
*	��    ��:  ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void SPI_FLASH_Init(void)
{


	/*
		PA4 = CS
		PA5= SCK
		PA6 = MISO
		PA7 = MOSI

		STM32Ӳ��SPI�ӿ� = SPI1
	*/
	{
		GPIO_InitTypeDef GPIO_InitStructure;

		/* ʹ��GPIO ʱ�� */
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

		/* ���� SCK, MISO �� MOSI Ϊ���ù��� */
		GPIO_PinAFConfig(GPIOA, GPIO_PinSource5, GPIO_AF_SPI1);
		GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_SPI1);
		GPIO_PinAFConfig(GPIOA, GPIO_PinSource7, GPIO_AF_SPI1);

		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
		GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_DOWN;

		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
		GPIO_Init(GPIOA, &GPIO_InitStructure);

		/* ����Ƭѡ����Ϊ�������ģʽ */
		SF_CS_HIGH();		/* Ƭѡ�øߣ���ѡ�� */
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
		GPIO_Init(GPIOA, &GPIO_InitStructure);
	}


	/* ����SPIӲ���������ڷ��ʴ���Flash */
	SPI_FLASH_Config();

	Get_FLASH_Info();				/* �Զ�ʶ��оƬ�ͺ� */

	SF_CS_LOW();				/* �����ʽ��ʹ�ܴ���FlashƬѡ */
	SPI_FLASH_SendByte(CMD_DISWR);		/* ���ͽ�ֹд�������,��ʹ�����д���� */
	SF_CS_HIGH();				/* �����ʽ�����ܴ���FlashƬѡ */

	SPI_FLASH_WaitForWriteEnd();		/* �ȴ�����Flash�ڲ�������� */

	SPI_FLASH_WriteStatus(0);			/* �������BLOCK��д���� */
}

/*
*********************************************************************************************************
*	�� �� ��: SPI_FLASH_Config
*	����˵��: ����STM32�ڲ�SPIӲ���Ĺ���ģʽ���ٶȵȲ��������ڷ���SPI�ӿڵĴ���Flash��
*	��    ��:  ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void SPI_FLASH_Config(void)
{
	SPI_InitTypeDef  SPI_InitStructure;

	/* ��SPIʱ�� */
	ENABLE_SPI_RCC();

	/* ����SPIӲ������ */
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;	/* ���ݷ���2��ȫ˫�� */
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;		/* STM32��SPI����ģʽ ������ģʽ */
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;	/* ����λ���� �� 8λ */
	/* SPI_CPOL��SPI_CPHA���ʹ�þ���ʱ�Ӻ����ݲ��������λ��ϵ��
	   ��������: ���߿����Ǹߵ�ƽ,��2�����أ������ز�������)
	*/
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;			/* ʱ�������ز������� */
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;		/* ʱ�ӵĵ�2�����ز������� */
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;			/* Ƭѡ���Ʒ�ʽ��������� */

	/* ���ò�����Ԥ��Ƶϵ�� */
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BAUD;

	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;	/* ����λ������򣺸�λ�ȴ� */
	SPI_InitStructure.SPI_CRCPolynomial = 7;			/* CRC����ʽ�Ĵ�������λ��Ϊ7�������̲��� */
	SPI_Init(SPI_FLASH, &SPI_InitStructure);

	SPI_Cmd(SPI_FLASH, DISABLE);			/* �Ƚ�ֹSPI  */

	SPI_Cmd(SPI_FLASH, ENABLE);				/* ʹ��SPI  */
}

/*
*********************************************************************************************************
*	�� �� ��: SPI_FLASH_SectorErase
*	����˵��: ����ָ��������
*	��    ��:  _uiSectorAddr : ������ַ
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void SPI_FLASH_SectorErase(uint32_t _uiSectorAddr)
{
	SPI_FLASH_WriteEnable();								/* ����дʹ������ */
	/* ������������ */
	SF_CS_LOW();									/* ʹ��Ƭѡ */
	SPI_FLASH_SendByte(CMD_SE);								/* ���Ͳ������� */
	SPI_FLASH_SendByte((_uiSectorAddr & 0xFF0000) >> 16);	/* ����������ַ�ĸ�8bit */
	SPI_FLASH_SendByte((_uiSectorAddr & 0xFF00) >> 8);		/* ����������ַ�м�8bit */
	SPI_FLASH_SendByte(_uiSectorAddr & 0xFF);				/* ����������ַ��8bit */
	SF_CS_HIGH();									/* ����Ƭѡ */

	SPI_FLASH_WaitForWriteEnd();							/* �ȴ�����Flash�ڲ�д������� */

}

/*
*********************************************************************************************************
*	�� �� ��: SPI_EraseChipALL
*	����˵��: ��������оƬ
*	��    ��:  ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void SPI_EraseChipALL(void)
{
	SPI_FLASH_WriteEnable();								/* ����дʹ������ */

	/* ������������ */
	SF_CS_LOW();									/* ʹ��Ƭѡ */
	SPI_FLASH_SendByte(CMD_BE);							/* ������Ƭ�������� */
	SF_CS_HIGH();									/* ����Ƭѡ */

	SPI_FLASH_WaitForWriteEnd();							/* �ȴ�����Flash�ڲ�д������� */
}

/*
*********************************************************************************************************
*	�� �� ��: SPI_FLASH_PageWrite
*	����˵��: ��һ��page��д�������ֽڡ��ֽڸ������ܳ���ҳ���С��4K)
*	��    ��:  	_pBuf : ����Դ��������
*				_uiWriteAddr ��Ŀ�������׵�ַ
*				_usSize �����ݸ��������ܳ���ҳ���С
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void SPI_FLASH_PageWrite(uint8_t * _pBuf, uint32_t _uiWriteAddr, uint16_t _usSize)
{
	uint32_t i, j;

	if (sflash.ChipID == SST25VF016B_ID)
	{
		/* AAIָ��Ҫ��������ݸ�����ż�� */
		if ((_usSize < 2) && (_usSize % 2))
		{
			return ;
		}

		SPI_FLASH_WriteEnable();								/* ����дʹ������ */

		SF_CS_LOW();									/* ʹ��Ƭѡ */
		SPI_FLASH_SendByte(CMD_AAI);							/* ����AAI����(��ַ�Զ����ӱ��) */
		SPI_FLASH_SendByte((_uiWriteAddr & 0xFF0000) >> 16);	/* ����������ַ�ĸ�8bit */
		SPI_FLASH_SendByte((_uiWriteAddr & 0xFF00) >> 8);		/* ����������ַ�м�8bit */
		SPI_FLASH_SendByte(_uiWriteAddr & 0xFF);				/* ����������ַ��8bit */
		SPI_FLASH_SendByte(*_pBuf++);							/* ���͵�1������ */
		SPI_FLASH_SendByte(*_pBuf++);							/* ���͵�2������ */
		SF_CS_HIGH();									/* ����Ƭѡ */

		SPI_FLASH_WaitForWriteEnd();							/* �ȴ�����Flash�ڲ�д������� */

		_usSize -= 2;									/* ����ʣ���ֽ��� */

		for (i = 0; i < _usSize / 2; i++)
		{
			SF_CS_LOW();								/* ʹ��Ƭѡ */
			SPI_FLASH_SendByte(CMD_AAI);						/* ����AAI����(��ַ�Զ����ӱ��) */
			SPI_FLASH_SendByte(*_pBuf++);						/* �������� */
			SPI_FLASH_SendByte(*_pBuf++);						/* �������� */
			SF_CS_HIGH();								/* ����Ƭѡ */
			SPI_FLASH_WaitForWriteEnd();						/* �ȴ�����Flash�ڲ�д������� */
		}

		/* ����д����״̬ */
		SF_CS_LOW();
		SPI_FLASH_SendByte(CMD_DISWR);
		SF_CS_HIGH();

		SPI_FLASH_WaitForWriteEnd();							/* �ȴ�����Flash�ڲ�д������� */
	}
	else	/* for MX25L1606E �� W25Q64BV */
	{
		for (j = 0; j < _usSize / 256; j++)
		{
			SPI_FLASH_WriteEnable();								/* ����дʹ������ */

			SF_CS_LOW();									/* ʹ��Ƭѡ */
			SPI_FLASH_SendByte(0x02);								/* ����AAI����(��ַ�Զ����ӱ��) */
			SPI_FLASH_SendByte((_uiWriteAddr & 0xFF0000) >> 16);	/* ����������ַ�ĸ�8bit */
			SPI_FLASH_SendByte((_uiWriteAddr & 0xFF00) >> 8);		/* ����������ַ�м�8bit */
			SPI_FLASH_SendByte(_uiWriteAddr & 0xFF);				/* ����������ַ��8bit */

			for (i = 0; i < 256; i++)
			{
				SPI_FLASH_SendByte(*_pBuf++);					/* �������� */
			}

			SF_CS_HIGH();								/* ��ֹƬѡ */

			SPI_FLASH_WaitForWriteEnd();						/* �ȴ�����Flash�ڲ�д������� */

			_uiWriteAddr += 256;
		}

		/* ����д����״̬ */
		SF_CS_LOW();
		SPI_FLASH_SendByte(CMD_DISWR);
		SF_CS_HIGH();

		SPI_FLASH_WaitForWriteEnd();							/* �ȴ�����Flash�ڲ�д������� */
	}
}

/*
*********************************************************************************************************
*	�� �� ��: SPI_FLASH_BufRead
*	����˵��: ������ȡ�����ֽڡ��ֽڸ������ܳ���оƬ������
*	��    ��:  	_pBuf : ����Դ��������
*				_uiReadAddr ���׵�ַ
*				_usSize �����ݸ���, ���Դ���PAGE_SIZE,���ǲ��ܳ���оƬ������
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void SPI_FLASH_BufRead(uint8_t * _pBuf, uint32_t _uiReadAddr, uint32_t _uiSize)
{
	/* �����ȡ�����ݳ���Ϊ0���߳�������Flash��ַ�ռ䣬��ֱ�ӷ��� */
	if ((_uiSize == 0) ||(_uiReadAddr + _uiSize) > sflash.TotalSize)
	{
		return;
	}
	/* ������������ */
	SF_CS_LOW();									/* ʹ��Ƭѡ */
	SPI_FLASH_SendByte(CMD_READ);							/* ���Ͷ����� */
	SPI_FLASH_SendByte((_uiReadAddr & 0xFF0000) >> 16);	/* ����������ַ�ĸ�8bit */
	SPI_FLASH_SendByte((_uiReadAddr & 0xFF00) >> 8);		/* ����������ַ�м�8bit */
	SPI_FLASH_SendByte(_uiReadAddr & 0xFF);				/* ����������ַ��8bit */
	while (_uiSize--)
	{
		*_pBuf++ = SPI_FLASH_SendByte(DUMMY_BYTE);			/* ��һ���ֽڲ��洢��pBuf�������ָ���Լ�1 */
	}
	SF_CS_HIGH();									/* ����Ƭѡ */
}

/*
*********************************************************************************************************
*	�� �� ��: SPI_FLASH_CmpData
*	����˵��: �Ƚ�Flash������.
*	��    ��:  	_ucpTar : ���ݻ�����
*				_uiSrcAddr ��Flash��ַ
*				_uiSize �����ݸ���, ���Դ���PAGE_SIZE,���ǲ��ܳ���оƬ������
*	�� �� ֵ: 0 = ���, 1 = ����
*********************************************************************************************************
*/
static uint8_t SPI_FLASH_CmpData(uint32_t _uiSrcAddr, uint8_t *_ucpTar, uint32_t _uiSize)
{
	uint8_t ucValue;

	/* �����ȡ�����ݳ���Ϊ0���߳�������Flash��ַ�ռ䣬��ֱ�ӷ��� */
	if ((_uiSrcAddr + _uiSize) > sflash.TotalSize)
	{
		return 1;
	}

	if (_uiSize == 0)
	{
		return 0;
	}

	SF_CS_LOW();									/* ʹ��Ƭѡ */
	SPI_FLASH_SendByte(CMD_READ);							/* ���Ͷ����� */
	SPI_FLASH_SendByte((_uiSrcAddr & 0xFF0000) >> 16);		/* ����������ַ�ĸ�8bit */
	SPI_FLASH_SendByte((_uiSrcAddr & 0xFF00) >> 8);		/* ����������ַ�м�8bit */
	SPI_FLASH_SendByte(_uiSrcAddr & 0xFF);					/* ����������ַ��8bit */
	while (_uiSize--)
	{
		/* ��һ���ֽ� */
		ucValue = SPI_FLASH_SendByte(DUMMY_BYTE);
		if (*_ucpTar++ != ucValue)
		{
			SF_CS_HIGH();
			return 1;
		}
	}
	SF_CS_HIGH();
	return 0;
}

/*
*********************************************************************************************************
*	�� �� ��: SPI_FLASH_NeedErase
*	����˵��: �ж�дPAGEǰ�Ƿ���Ҫ�Ȳ�����
*	��    ��:   _ucpOldBuf �� ������
*			   _ucpNewBuf �� ������
*			   _uiLen �����ݸ��������ܳ���ҳ���С
*	�� �� ֵ: 0 : ����Ҫ������ 1 ����Ҫ����
*********************************************************************************************************
*/
static uint8_t SPI_FLASH_NeedErase(uint8_t * _ucpOldBuf, uint8_t *_ucpNewBuf, uint16_t _usLen)
{
	uint16_t i;
	uint8_t ucOld;

	/*
	�㷨��1����old ��, new ����
	      old    new
		  1101   0101
	~     1111
		= 0010   0101

	�㷨��2��: old �󷴵Ľ���� new λ��
		  0010   old
	&	  0101   new
		 =0000

	�㷨��3��: ���Ϊ0,���ʾ�������. �����ʾ��Ҫ����
	*/

	for (i = 0; i < _usLen; i++)
	{
		ucOld = *_ucpOldBuf++;
		ucOld = ~ucOld;

		/* ע������д��: if (ucOld & (*_ucpNewBuf++) != 0) */
		if ((ucOld & (*_ucpNewBuf++)) != 0)
		{
			return 1;
		}
	}
	return 0;
}

/*
*********************************************************************************************************
*	�� �� ��: SPI_FLASH_AutoWritePage
*	����˵��: д1��PAGE��У��,�������ȷ������д���Ρ��������Զ���ɲ���������
*	��    ��:  	_pBuf : ����Դ��������
*				_uiWriteAddr ��Ŀ�������׵�ַ
*				_usSize �����ݸ��������ܳ���ҳ���С
*	�� �� ֵ: 0 : ���� 1 �� �ɹ�
*********************************************************************************************************
*/
static uint8_t SPI_FLASH_AutoWritePage(uint8_t *_ucpSrc, uint32_t _uiWrAddr, uint16_t _usWrLen)
{
	uint16_t i;
	uint16_t j;					/* ������ʱ */
	uint32_t uiFirstAddr;		/* ������ַ */
	uint8_t ucNeedErase;		/* 1��ʾ��Ҫ���� */
	uint8_t cRet;

	/* ����Ϊ0ʱ����������,ֱ����Ϊ�ɹ� */
	if (_usWrLen == 0)
	{
		return 1;
	}

	/* ���ƫ�Ƶ�ַ����оƬ�������˳� */
	if (_uiWrAddr >= sflash.TotalSize)
	{
		return 0;
	}

	/* ������ݳ��ȴ����������������˳� */
	if (_usWrLen > sflash.PageSize)
	{
		return 0;
	}

	/* ���FLASH�е�����û�б仯,��дFLASH */
	SPI_FLASH_BufRead(SPI_FLASH_WriteBuffer, _uiWrAddr, _usWrLen);
	if (memcmp(SPI_FLASH_WriteBuffer, _ucpSrc, _usWrLen) == 0)
	{
		return 1;
	}

	/* �ж��Ƿ���Ҫ�Ȳ������� */
	/* ����������޸�Ϊ�����ݣ�����λ���� 1->0 ���� 0->0, ���������,���Flash���� */
	ucNeedErase = 0;
	if (SPI_FLASH_NeedErase(SPI_FLASH_WriteBuffer, _ucpSrc, _usWrLen))
	{
		ucNeedErase = 1;
	}

	uiFirstAddr = _uiWrAddr & (~(sflash.PageSize - 1));

	if (_usWrLen == sflash.PageSize)		/* ������������д */
	{
		for	(i = 0; i < sflash.PageSize; i++)
		{
			SPI_FLASH_WriteBuffer[i] = _ucpSrc[i];
		}
	}
	else						/* ��д�������� */
	{
		/* �Ƚ��������������ݶ��� */
		SPI_FLASH_BufRead(SPI_FLASH_WriteBuffer, uiFirstAddr, sflash.PageSize);

		/* ���������ݸ��� */
		i = _uiWrAddr & (sflash.PageSize - 1);
		memcpy(&SPI_FLASH_WriteBuffer[i], _ucpSrc, _usWrLen);
	}

	/* д��֮�����У�飬�������ȷ����д�����3�� */
	cRet = 0;
	for (i = 0; i < 3; i++)
	{

		/* ����������޸�Ϊ�����ݣ�����λ���� 1->0 ���� 0->0, ���������,���Flash���� */
		if (ucNeedErase == 1)
		{
			SPI_FLASH_SectorErase(uiFirstAddr);		/* ����1������ */
		}

		/* ���һ��PAGE */
		SPI_FLASH_PageWrite(SPI_FLASH_WriteBuffer, uiFirstAddr, sflash.PageSize);

		if (SPI_FLASH_CmpData(_uiWrAddr, _ucpSrc, _usWrLen) == 0)
		{
			cRet = 1;
			break;
		}
		else
		{
			if (SPI_FLASH_CmpData(_uiWrAddr, _ucpSrc, _usWrLen) == 0)
			{
				cRet = 1;
				break;
			}

			/* ʧ�ܺ��ӳ�һ��ʱ�������� */
			for (j = 0; j < 10000; j++);
		}
	}

	return cRet;
}

/*
*********************************************************************************************************
*	�� �� ��: SPI_FLASH_BufWrite
*	����˵��: д1��������У��,�������ȷ������д���Ρ��������Զ���ɲ���������
*	��    ��:  	_pBuf : ����Դ��������
*				_uiWrAddr ��Ŀ�������׵�ַ
*				_usSize �����ݸ��������ܳ���ҳ���С
*	�� �� ֵ: 1 : �ɹ��� 0 �� ʧ��
*********************************************************************************************************
*/
uint8_t SPI_FLASH_BufWrite( uint32_t _uiWriteAddr, uint8_t* _pBuf, uint16_t _usWriteSize)
{
	uint16_t NumOfPage = 0, NumOfSingle = 0, Addr = 0, count = 0, temp = 0;

	Addr = _uiWriteAddr % sflash.PageSize;
	count = sflash.PageSize - Addr;
	NumOfPage =  _usWriteSize / sflash.PageSize;
	NumOfSingle = _usWriteSize % sflash.PageSize;


	if (Addr == 0) /* ��ʼ��ַ��ҳ���׵�ַ  */
	{
		
		if (NumOfPage == 0) /* ���ݳ���С��ҳ���С */
		{
			if (SPI_FLASH_AutoWritePage(_pBuf, _uiWriteAddr, _usWriteSize) == 0)
			{
				return 0;
			}
		}
		else 	/* ���ݳ��ȴ��ڵ���ҳ���С */
		{
				while (NumOfPage--)
				{
					
					if (SPI_FLASH_AutoWritePage(_pBuf, _uiWriteAddr, sflash.PageSize) == 0)
					{
						return 0;
					}
					_uiWriteAddr +=  sflash.PageSize;
					_pBuf += sflash.PageSize;
				}
				if (SPI_FLASH_AutoWritePage(_pBuf, _uiWriteAddr, NumOfSingle) == 0)
				{
					return 0;
				}
		}
	}
	else  /* ��ʼ��ַ����ҳ���׵�ַ  */
	{
		if (NumOfPage == 0) /* ���ݳ���С��ҳ���С */
		{
			if (NumOfSingle > count) /* (_usWriteSize + _uiWriteAddr) > SPI_FLASH_PAGESIZE */
			{
				temp = NumOfSingle - count;
       
				if (SPI_FLASH_AutoWritePage(_pBuf, _uiWriteAddr, count) == 0)
				{
					return 0;
				}

				_uiWriteAddr +=  count;
				_pBuf += count;

				if (SPI_FLASH_AutoWritePage(_pBuf, _uiWriteAddr, temp) == 0)
				{
					return 0;
				}
			}
			else
			{
				if (SPI_FLASH_AutoWritePage(_pBuf, _uiWriteAddr, _usWriteSize) == 0)
				{
					return 0;
				}
			}
		}
		else	/* ���ݳ��ȴ��ڵ���ҳ���С */
		{
			_usWriteSize -= count;
			NumOfPage =  _usWriteSize / sflash.PageSize;
			NumOfSingle = _usWriteSize % sflash.PageSize;

			if (SPI_FLASH_AutoWritePage(_pBuf, _uiWriteAddr, count) == 0)
			{
				return 0;
			}

			_uiWriteAddr +=  count;
			_pBuf += count;

			while (NumOfPage--)
			{
				if (SPI_FLASH_AutoWritePage(_pBuf, _uiWriteAddr, sflash.PageSize) == 0)
				{
					return 0;
				}
				_uiWriteAddr +=  sflash.PageSize;
				_pBuf += sflash.PageSize;
			}

			if (NumOfSingle != 0)
			{
				if (SPI_FLASH_AutoWritePage(_pBuf, _uiWriteAddr, NumOfSingle) == 0)
				{
					return 0;
				}
			}
		}
	}
	return 1;	/* �ɹ� */
}

/*
*********************************************************************************************************
*	�� �� ��: SPI_FLASH_ReadID
*	����˵��: ��ȡ����ID
*	��    ��:  ��
*	�� �� ֵ: 32bit������ID (���8bit��0����ЧIDλ��Ϊ24bit��
*********************************************************************************************************
*/
uint32_t SPI_FLASH_ReadID(void)
{
	uint32_t uiID;
	uint8_t id1, id2, id3;

	SF_CS_LOW();									/* ʹ��Ƭѡ */
	SPI_FLASH_SendByte(CMD_RDID);								/* ���Ͷ�ID���� */
	id1 = SPI_FLASH_SendByte(DUMMY_BYTE);					/* ��ID�ĵ�1���ֽ� */
	id2 = SPI_FLASH_SendByte(DUMMY_BYTE);					/* ��ID�ĵ�2���ֽ� */
	id3 = SPI_FLASH_SendByte(DUMMY_BYTE);					/* ��ID�ĵ�3���ֽ� */
	SF_CS_HIGH();									/* ����Ƭѡ */

	uiID = ((uint32_t)id1 << 16) | ((uint32_t)id2 << 8) | id3;

	return uiID;
}

/*
*********************************************************************************************************
*	�� �� ��: Get_FLASH_Info
*	����˵��: ��ȡ����ID,�������������
*	��    ��:  ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void Get_FLASH_Info(void)
{
	/* �Զ�ʶ����Flash�ͺ� */
	{
		sflash.ChipID = SPI_FLASH_ReadID();	/* оƬID */

		switch (sflash.ChipID)
		{
			case SST25VF016B_ID:
				strcpy(sflash.ChipName, "SST25VF016B");
				sflash.TotalSize = 2 * 1024 * 1024;	/* ������ = 2M */
				sflash.PageSize = 4 * 1024;			/* ҳ���С = 4K */
				break;

			case MX25L1606E_ID:
				strcpy(sflash.ChipName, "MX25L1606E");
				sflash.TotalSize = 2 * 1024 * 1024;	/* ������ = 2M */
				sflash.PageSize = 4 * 1024;			/* ҳ���С = 4K */
				break;

			case W25Q64BV_ID:
				strcpy(sflash.ChipName, "W25Q64BV");
				sflash.TotalSize = 8 * 1024 * 1024;	/* ������ = 8M */
				sflash.PageSize = 4 * 1024;			/* ҳ���С = 4K */
				break;

			default:
				strcpy(sflash.ChipName, "Unknow Flash");
				sflash.TotalSize = 2 * 1024 * 1024;
				sflash.PageSize = 4 * 1024;
				break;
		}
	}
}

/*
*********************************************************************************************************
*	�� �� ��: SPI_FLASH_SendByte
*	����˵��: ����������һ���ֽڣ�ͬʱ��MISO���߲����������ص�����
*	��    ��:  _ucByte : ���͵��ֽ�ֵ
*	�� �� ֵ: ��MISO���߲����������ص�����
*********************************************************************************************************
*/
static uint8_t SPI_FLASH_SendByte(uint8_t _ucValue)
{
	/* �ȴ��ϸ�����δ������� */
	while (SPI_I2S_GetFlagStatus(SPI_FLASH, SPI_I2S_FLAG_TXE) == RESET);

	/* ͨ��SPIӲ������1���ֽ� */
	SPI_I2S_SendData(SPI_FLASH, _ucValue);

	/* �ȴ�����һ���ֽ�������� */
	while (SPI_I2S_GetFlagStatus(SPI_FLASH, SPI_I2S_FLAG_RXNE) == RESET);

	/* ���ش�SPI���߶��������� */
	return SPI_I2S_ReceiveData(SPI_FLASH);
}

/*
*********************************************************************************************************
*	�� �� ��: SPI_FLASH_WriteEnable
*	����˵��: ����������дʹ������
*	��    ��:  ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void SPI_FLASH_WriteEnable(void)
{
	SF_CS_LOW();									/* ʹ��Ƭѡ */
	SPI_FLASH_SendByte(CMD_WREN);								/* �������� */
	SF_CS_HIGH();									/* ����Ƭѡ */
}

/*
*********************************************************************************************************
*	�� �� ��: SPI_FLASH_WriteStatus
*	����˵��: д״̬�Ĵ���
*	��    ��:  _ucValue : ״̬�Ĵ�����ֵ
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void SPI_FLASH_WriteStatus(uint8_t _ucValue)
{

	if (sflash.ChipID == SST25VF016B_ID)
	{
		/* ��1������ʹ��д״̬�Ĵ��� */
		SF_CS_LOW();									/* ʹ��Ƭѡ */
		SPI_FLASH_SendByte(CMD_EWRSR);							/* ������� ����д״̬�Ĵ��� */
		SF_CS_HIGH();									/* ����Ƭѡ */

		/* ��2������д״̬�Ĵ��� */
		SF_CS_LOW();									/* ʹ��Ƭѡ */
		SPI_FLASH_SendByte(CMD_WRSR);							/* ������� д״̬�Ĵ��� */
		SPI_FLASH_SendByte(_ucValue);							/* �������ݣ�״̬�Ĵ�����ֵ */
		SF_CS_HIGH();									/* ����Ƭѡ */
	}
	else
	{
		SF_CS_LOW();									/* ʹ��Ƭѡ */
		SPI_FLASH_SendByte(CMD_WRSR);							/* ������� д״̬�Ĵ��� */
		SPI_FLASH_SendByte(_ucValue);							/* �������ݣ�״̬�Ĵ�����ֵ */
		SF_CS_HIGH();									/* ����Ƭѡ */
	}
}

/*
*********************************************************************************************************
*	�� �� ��: SPI_FLASH_WaitForWriteEnd
*	����˵��: ����ѭ����ѯ�ķ�ʽ�ȴ������ڲ�д�������
*	��    ��:  ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void SPI_FLASH_WaitForWriteEnd(void)
{
	SF_CS_LOW();									/* ʹ��Ƭѡ */
	SPI_FLASH_SendByte(CMD_RDSR);							/* ������� ��״̬�Ĵ��� */
	while((SPI_FLASH_SendByte(DUMMY_BYTE) & WIP_FLAG) == SET);	/* �ж�״̬�Ĵ�����æ��־λ */
	SF_CS_HIGH();									/* ����Ƭѡ */
}

