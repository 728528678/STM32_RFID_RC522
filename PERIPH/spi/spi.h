#ifndef __SPI_H
#define __SPI_H
#include "stm32f10x.h"

//�궨��W25Q128Ƭѡָ��
#define W25QXX_CS_SET GPIO_SetBits(GPIOB,GPIO_Pin_12)
#define W25QXX_CS_CLR GPIO_ResetBits(GPIOB,GPIO_Pin_12)

void spi2_init(void);
u8 spi2_readwritebyte(unsigned char  txdata);//SPI���ղ�����һ������
void spi_setspeed(unsigned char  SPI_BaudRatePrescaler);//����SPI�Ĳ�����

#endif 

