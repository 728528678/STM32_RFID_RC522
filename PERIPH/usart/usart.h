#ifndef __USART_H
#define __USART_H
#include "stm32f10x.h"

void usart1_init(u32 baud);//���ڳ�ʼ������ ����baudλ������
void usart1_sendstring(char *s);//�����ַ���

#endif 

