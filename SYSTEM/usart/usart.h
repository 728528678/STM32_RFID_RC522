#ifndef __USART_H
#define __USART_H
#include "stm32f10x.h"

void usart1_init(u32 baud);//串口初始化函数 参数baud位波特率
void usart1_sendstring(char *s);//发送字符串

#endif 

