#include "ledkey.h"
#include "delay.h"



//LEDBEEP初始化
void led_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;//定义一个GPIO初始化结构体
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOA,ENABLE);

	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_PP;//推挽输出
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;//设置输出频率
	GPIO_Init(GPIOC,&GPIO_InitStructure);//板载LED
	LED_R = 1;
	LED_G = 1;
	
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_0 | GPIO_Pin_1;
	GPIO_Init(GPIOA,&GPIO_InitStructure);//外部LED0 LED1
	LED_DATA = 1;
}


//按键初始化函数
void key_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IN_FLOATING;//有外部上拉，配置浮空输入
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_3|GPIO_Pin_2;
	GPIO_Init(GPIOA,&GPIO_InitStructure);//key1 key2
}

u8 Read_Key(void)
{
	if(KEY0 == 0)
	{
		delay_ms(10);
		if(KEY0==0)
		{
			return 1;
		}
	}
	else if(KEY1 == 0)
	{
		delay_ms(10);
		if(KEY1==0)
		{
			return 2;
		}
	}
	return 0;
}

