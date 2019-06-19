#include "usart.h"
#include "ledkey.h"
/*
USART：通用同步异步收发器
UART：通用异步收发器
int fputc(int ch,FILE *f)
{
	USART_SendData(USART1,(u8)ch);
	while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)==RESET);
	//把USART_FLAG_TC改为USART_FLAG_TXE解决用printf发送字符串时时不能发送第一位
	return ch;
}
*/

//串口1初始化函数
void usart1_init(u32 baud)
{
	GPIO_InitTypeDef GPIO_initStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_USART1,ENABLE);
	
	GPIO_initStructure.GPIO_Mode=GPIO_Mode_AF_PP;//PA9 TXD复用推挽输出
	GPIO_initStructure.GPIO_Pin=GPIO_Pin_9;
	GPIO_initStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_initStructure);
	
	GPIO_initStructure.GPIO_Mode=GPIO_Mode_IN_FLOATING;//PA10 RXD浮空输入
	GPIO_initStructure.GPIO_Pin=GPIO_Pin_10;
	GPIO_Init(GPIOA,&GPIO_initStructure);
	
	USART_InitStructure.USART_BaudRate=baud;//波特率
	USART_InitStructure.USART_HardwareFlowControl=USART_HardwareFlowControl_None;//硬件流控制失能
	USART_InitStructure.USART_Mode=USART_Mode_Tx|USART_Mode_Rx;//发送和接收使能
	USART_InitStructure.USART_Parity=USART_Parity_No;//无校验位
	USART_InitStructure.USART_StopBits=USART_StopBits_1;//1位停止位
	USART_InitStructure.USART_WordLength=USART_WordLength_8b;//数据长度
	USART_Init(USART1,&USART_InitStructure);//串口初始化
	
	USART_ITConfig(USART1,USART_IT_RXNE,ENABLE);//串口接收中断使能
	
	//串口中断管理
	NVIC_InitStructure.NVIC_IRQChannel=USART1_IRQn;//USART1中断通道
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=1;
	NVIC_Init(&NVIC_InitStructure);
	
	USART_Cmd(USART1,ENABLE);//串口使能
}


//发送字符串
void usart1_sendstring(char *s)//这里的s是指向字符类型变量的指针变量 这里的*是类型说明符
{
	while(*s)//检查字符串的结尾是否为空 这里的*是取内容运算符
	{
		USART_SendData(USART1,*s++);
		//++(自增一) *(取值运算符)他们的运算优先级一样所以从左边开始执行 先取出s的值s再自增一
		while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)!=SET);
		//发送数据寄存器不为空就一直在这等待
		//USART_ClearFlag(USART1,USART_FLAG_TXE);//清除发送寄存器空标志位 有无都可以
	}
}


//串口1中断服务函数
void USART1_IRQHandler(void)
{
	if(USART_GetITStatus(USART1,USART_IT_RXNE)!=RESET)
	{
		if(USART_ReceiveData(USART1)=='U')
		{
			USART_SendData(USART1,'U');
		}
		USART_ClearITPendingBit(USART1,USART_IT_RXNE);//清除串口接收中断标志位
	}
}

