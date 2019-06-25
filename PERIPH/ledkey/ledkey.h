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
#define KEY0 PAin(2)	//д
#define KEY1 PAin(3)	//��

extern unsigned char mode_flag;//�Կ�����ģʽ

void led_init(void);//LEDBEEP��ʼ��
void key_init(void);//������ʼ������
u8 Read_Key(void);
#endif
