#ifndef __SPI_H
#define __SPI_H
#include "stm32f10x.h"

//宏定义W25Q128片选指令
#define W25QXX_CS_SET GPIO_SetBits(GPIOB,GPIO_Pin_12)
#define W25QXX_CS_CLR GPIO_ResetBits(GPIOB,GPIO_Pin_12)

void spi2_init(void);
u8 spi2_readwritebyte(unsigned char  txdata);//SPI接收并发送一个函数
void spi_setspeed(unsigned char  SPI_BaudRatePrescaler);//设置SPI的波特率

#endif 

