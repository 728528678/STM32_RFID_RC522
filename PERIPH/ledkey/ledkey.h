#ifndef __LEDKEY_H  //if no define
#define __LEDKEY_H
#include "stm32f10x.h"
#include "sys.h"

//����LED
#define LED_DATA PCout(13)
#define LED_G PAout(0)
#define LED_R PAout(1)

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
