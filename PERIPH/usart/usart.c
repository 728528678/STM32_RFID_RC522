#include "usart.h"
#include "ledkey.h"
/*
USART��ͨ��ͬ���첽�շ���
UART��ͨ���첽�շ���
int fputc(int ch,FILE *f)
{
	USART_SendData(USART1,(u8)ch);
	while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)==RESET);
	//��USART_FLAG_TC��ΪUSART_FLAG_TXE�����printf�����ַ���ʱʱ���ܷ��͵�һλ
	return ch;
}
*/

//����1��ʼ������
void usart1_init(u32 baud)
{
	GPIO_InitTypeDef GPIO_initStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_USART1,ENABLE);
	
	GPIO_initStructure.GPIO_Mode=GPIO_Mode_AF_PP;//PA9 TXD�����������
	GPIO_initStructure.GPIO_Pin=GPIO_Pin_9;
	GPIO_initStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_initStructure);
	
	GPIO_initStructure.GPIO_Mode=GPIO_Mode_IN_FLOATING;//PA10 RXD��������
	GPIO_initStructure.GPIO_Pin=GPIO_Pin_10;
	GPIO_Init(GPIOA,&GPIO_initStructure);
	
	USART_InitStructure.USART_BaudRate=baud;//������
	USART_InitStructure.USART_HardwareFlowControl=USART_HardwareFlowControl_None;//Ӳ��������ʧ��
	USART_InitStructure.USART_Mode=USART_Mode_Tx|USART_Mode_Rx;//���ͺͽ���ʹ��
	USART_InitStructure.USART_Parity=USART_Parity_No;//��У��λ
	USART_InitStructure.USART_StopBits=USART_StopBits_1;//1λֹͣλ
	USART_InitStructure.USART_WordLength=USART_WordLength_8b;//���ݳ���
	USART_Init(USART1,&USART_InitStructure);//���ڳ�ʼ��
	
	USART_ITConfig(USART1,USART_IT_RXNE,ENABLE);//���ڽ����ж�ʹ��
	
	//�����жϹ���
	NVIC_InitStructure.NVIC_IRQChannel=USART1_IRQn;//USART1�ж�ͨ��
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=1;
	NVIC_Init(&NVIC_InitStructure);
	
	USART_Cmd(USART1,ENABLE);//����ʹ��
}


//�����ַ���
void usart1_sendstring(char *s)//�����s��ָ���ַ����ͱ�����ָ����� �����*������˵����
{
	while(*s)//����ַ����Ľ�β�Ƿ�Ϊ�� �����*��ȡ���������
	{
		USART_SendData(USART1,*s++);
		//++(����һ) *(ȡֵ�����)���ǵ��������ȼ�һ�����Դ���߿�ʼִ�� ��ȡ��s��ֵs������һ
		while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)!=SET);
		//�������ݼĴ�����Ϊ�վ�һֱ����ȴ�
		//USART_ClearFlag(USART1,USART_FLAG_TXE);//������ͼĴ����ձ�־λ ���޶�����
	}
}


//����1�жϷ�����
void USART1_IRQHandler(void)
{
	if(USART_GetITStatus(USART1,USART_IT_RXNE)!=RESET)
	{
		if(USART_ReceiveData(USART1)=='U')
		{
			USART_SendData(USART1,'U');
		}
		USART_ClearITPendingBit(USART1,USART_IT_RXNE);//������ڽ����жϱ�־λ
	}
}

