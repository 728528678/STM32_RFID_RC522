#include "spi.h"

//SPI2初始化函数
void spi2_init(void)
{	
	GPIO_InitTypeDef GPIO_InitStructure;
	SPI_InitTypeDef SPI_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2,ENABLE);//SPI2挂载在APB1下
	
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_AF_PP;//推挽复用输出
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_13|GPIO_Pin_15;//RFID_CLK RFID_MOSI
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOB,&GPIO_InitStructure);
	GPIO_SetBits(GPIOB,GPIO_Pin_15);//拉高
	GPIO_ResetBits(GPIOB,GPIO_Pin_13);//时钟线空闲时为低电平

	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IN_FLOATING;//浮空输入
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_14;//RFID_MISO
	GPIO_Init(GPIOB,&GPIO_InitStructure);
	
	
	SPI_InitStructure.SPI_BaudRatePrescaler=SPI_BaudRatePrescaler_256;//波特率预分频值
	SPI_InitStructure.SPI_CPHA=SPI_CPHA_1Edge;//数据在时钟的1个跳变沿时采集
	SPI_InitStructure.SPI_CPOL=SPI_CPOL_Low;//时钟线空闲时为低电平
	SPI_InitStructure.SPI_CRCPolynomial=7;//CRC校验
	SPI_InitStructure.SPI_DataSize=SPI_DataSize_8b;//传输数据为8位
	SPI_InitStructure.SPI_Direction=SPI_Direction_2Lines_FullDuplex;//双线双向全双工传输
	SPI_InitStructure.SPI_FirstBit=SPI_FirstBit_MSB;//数据从最高位开始传输
	SPI_InitStructure.SPI_Mode=SPI_Mode_Master;//主模式
	SPI_InitStructure.SPI_NSS=SPI_NSS_Soft;//软件控制 NSS引脚用作它用
	SPI_Init(SPI2,&SPI_InitStructure);
	
	SPI_Cmd(SPI2,ENABLE);//SPI2使能
	spi2_readwritebyte(0xff);
}
//SPI接收并发送一个函数
u8 spi2_readwritebyte(unsigned char txdata)
{
	u16 i=0;
	while(SPI_I2S_GetFlagStatus(SPI2,SPI_I2S_FLAG_TXE)==RESET)//等待发送缓存为空
	{
		i++;
		if(i>=200) return 0; 
	}
	SPI_I2S_SendData(SPI2,txdata);//发送数据
	i=0;
	while(SPI_I2S_GetFlagStatus(SPI2,SPI_I2S_FLAG_RXNE)==RESET)//等待接收缓存器非空
	{
		i++;
		if(i>=200) return 0; 
	}
	return SPI_I2S_ReceiveData(SPI2);
	
}
//设置SPI的波特率
//SPI_BaudRatePrescaler_2   2分频   
//SPI_BaudRatePrescaler_8   8分频   
//SPI_BaudRatePrescaler_16  16分频  
//SPI_BaudRatePrescaler_256 256分频
void spi_setspeed(unsigned char  SPI_BaudRatePrescaler)
{
	assert_param(IS_SPI_BAUDRATE_PRESCALER(SPI_BaudRatePrescaler));
	SPI2->CR1&=0XFFC7;
	SPI2->CR1|=SPI_BaudRatePrescaler;	//设置SPI2速度 
	SPI_Cmd(SPI2,ENABLE); 
}
	
