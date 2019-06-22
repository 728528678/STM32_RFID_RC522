#ifndef __LEDKEY_H  //if no define
#define __LEDKEY_H
#include "stm32f10x.h"
#include "sys.h"

//板载LED
#define LED_DATA PCout(13)
#define LED_G PAout(0)
#define LED_R PAout(1)

//宏定义蜂鸣器
#define BEEP_SET GPIO_SetBits(GPIOB,GPIO_Pin_8)
#define BEEP_CLR GPIO_ResetBits(GPIOB,GPIO_Pin_8)
//宏定义读取按键输入值
#define KEY0 GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_4)
#define KEY1 GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_3)
#define KEY2 GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_2)
#define KEY_UP GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0)

extern unsigned char mode_flag;//对卡操作模式

void led_init(void);//LEDBEEP初始化
void key_init(void);//按键初始化函数
void key(void);//按键函数
#endif
