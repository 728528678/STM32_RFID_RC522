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
#define KEY0 PAin(2)	//写
#define KEY1 PAin(3)	//读

extern unsigned char mode_flag;//对卡操作模式

void led_init(void);//LEDBEEP初始化
void key_init(void);//按键初始化函数
u8 Read_Key(void);
#endif
