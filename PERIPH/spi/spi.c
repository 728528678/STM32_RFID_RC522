#include "spi.h"

//SPI2��ʼ������
void spi2_init(void)
{	
	GPIO_InitTypeDef GPIO_InitStructure;
	SPI_InitTypeDef SPI_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2,ENABLE);//SPI2������APB1��
	
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_AF_PP;//���츴�����
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_13|GPIO_Pin_15;//RFID_CLK RFID_MOSI
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOB,&GPIO_InitStructure);
	GPIO_SetBits(GPIOB,GPIO_Pin_15);//����
	GPIO_ResetBits(GPIOB,GPIO_Pin_13);//ʱ���߿���ʱΪ�͵�ƽ

	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IN_FLOATING;//��������
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_14;//RFID_MISO
	GPIO_Init(GPIOB,&GPIO_InitStructure);
	
	
	SPI_InitStructure.SPI_BaudRatePrescaler=SPI_BaudRatePrescaler_256;//������Ԥ��Ƶֵ
	SPI_InitStructure.SPI_CPHA=SPI_CPHA_1Edge;//������ʱ�ӵ�1��������ʱ�ɼ�
	SPI_InitStructure.SPI_CPOL=SPI_CPOL_Low;//ʱ���߿���ʱΪ�͵�ƽ
	SPI_InitStructure.SPI_CRCPolynomial=7;//CRCУ��
	SPI_InitStructure.SPI_DataSize=SPI_DataSize_8b;//��������Ϊ8λ
	SPI_InitStructure.SPI_Direction=SPI_Direction_2Lines_FullDuplex;//˫��˫��ȫ˫������
	SPI_InitStructure.SPI_FirstBit=SPI_FirstBit_MSB;//���ݴ����λ��ʼ����
	SPI_InitStructure.SPI_Mode=SPI_Mode_Master;//��ģʽ
	SPI_InitStructure.SPI_NSS=SPI_NSS_Soft;//������� NSS������������
	SPI_Init(SPI2,&SPI_InitStructure);
	
	SPI_Cmd(SPI2,ENABLE);//SPI2ʹ��
	spi2_readwritebyte(0xff);
}
//SPI���ղ�����һ������
u8 spi2_readwritebyte(unsigned char txdata)
{
	u16 i=0;
	while(SPI_I2S_GetFlagStatus(SPI2,SPI_I2S_FLAG_TXE)==RESET)//�ȴ����ͻ���Ϊ��
	{
		i++;
		if(i>=200) return 0; 
	}
	SPI_I2S_SendData(SPI2,txdata);//��������
	i=0;
	while(SPI_I2S_GetFlagStatus(SPI2,SPI_I2S_FLAG_RXNE)==RESET)//�ȴ����ջ������ǿ�
	{
		i++;
		if(i>=200) return 0; 
	}
	return SPI_I2S_ReceiveData(SPI2);
	
}
//����SPI�Ĳ�����
//SPI_BaudRatePrescaler_2   2��Ƶ   
//SPI_BaudRatePrescaler_8   8��Ƶ   
//SPI_BaudRatePrescaler_16  16��Ƶ  
//SPI_BaudRatePrescaler_256 256��Ƶ
void spi_setspeed(unsigned char  SPI_BaudRatePrescaler)
{
	assert_param(IS_SPI_BAUDRATE_PRESCALER(SPI_BaudRatePrescaler));
	SPI2->CR1&=0XFFC7;
	SPI2->CR1|=SPI_BaudRatePrescaler;	//����SPI2�ٶ� 
	SPI_Cmd(SPI2,ENABLE); 
}
	
