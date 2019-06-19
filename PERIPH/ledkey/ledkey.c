#include "ledkey.h"
#include "delay.h"
#include "lcd.h"


//LEDBEEP��ʼ��
void led_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;//����һ��GPIO��ʼ���ṹ��
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOA,ENABLE);

	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_PP;//�������
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;//�������Ƶ��
	GPIO_Init(GPIOC,&GPIO_InitStructure);//����LED
	
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_0 | GPIO_Pin_1;
	GPIO_Init(GPIOA,&GPIO_InitStructure);//�ⲿLED0 LED1
}


//������ʼ������
void key_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IN_FLOATING;//���ⲿ���������ø�������
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_3|GPIO_Pin_2;
	GPIO_Init(GPIOA,&GPIO_InitStructure);//key1 key2
}

