#ifndef __LEDKEY_H  //if no define
#define __LEDKEY_H
#include "stm32f10x.h"
 
//�궨��LED
#define LED0_SET GPIO_SetBits(GPIOB,GPIO_Pin_5)
#define LED0_CLR GPIO_ResetBits(GPIOB,GPIO_Pin_5)
#define LED1_SET GPIO_SetBits(GPIOE,GPIO_Pin_5)
#define LED1_CLR GPIO_ResetBits(GPIOE,GPIO_Pin_5)
//�궨�������
#define BEEP_SET GPIO_SetBits(GPIOB,GPIO_Pin_8)
#define BEEP_CLR GPIO_ResetBits(GPIOB,GPIO_Pin_8)
//�궨���ȡ��������ֵ
#define KEY0 GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_4)
#define KEY1 GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_3)
#define KEY2 GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_2)
#define KEY_UP GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0)

extern unsigned char mode_flag;//�Կ�����ģʽ

void led_init(void);//LEDBEEP��ʼ��
void key_init(void);//������ʼ������
void key(void);//��������
#endif
