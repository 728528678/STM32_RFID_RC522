#include "stm32f10x.h"
#include "delay.h"
#include "ledkey.h"
#include "usart.h"
#include "rc522.h"
#include "lcd.h"
#include "stdio.h"

/************************************************
RFID_RC522
20180430
************************************************/

int fputc(int ch,FILE *f)
{
	USART_SendData(USART1,(u8)ch);
	while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)==RESET);
	//把USART_FLAG_TC改为USART_FLAG_TXE解决用printf发送字符串时时不能发送第一位
	return ch;
}


unsigned char Block=0;
unsigned char Card_Buffer[2];//用于接收卡返回的类型数据
unsigned short Card_Type;//卡的类型
unsigned char Card_ID[4];//用于存放卡的序列号
unsigned char UID[5],Temp[4];
unsigned char i;
unsigned char Data_Buffer[16];//读块数据缓存
unsigned char table[16]={
'0','1','2','3',
'4','5','6','7',
'8','9','A','B',
'C','D','E','F'};
unsigned char Modify_Key[6]={0xA0,0xA1,0xA2,0xA3,0xA4,0xA5};//修改后的密码
unsigned char Default_Key[6]={0xff,0xff,0xff,0xff,0xff,0xff};//初始密码
unsigned char DefaultValue[16]={//数值块4初始化0x00000000
0x00,0x00,0x00,0x00,//值正码
0xff,0xff,0xff,0xff,//值反码
0x00,0x00,0x00,0x00,//值正码
0x04,//地址正码
0xfb,//地址反码
0x04,//地址正码
0xfb //地址反码
};

const u8 User_Key[16][6]={
{0xff,0xff,0xff,0xff,0xff,0xff},
{0xD0,0x7D,0xA5,0x54,0xDD,0xD0},
{0xD0,0x7D,0xA5,0x54,0xDD,0xD0},
{0xD0,0x7D,0xA5,0x54,0xDD,0xD0},
{0xff,0xff,0xff,0xff,0xff,0xff},
{0xff,0xff,0xff,0xff,0xff,0xff},
{0xff,0xff,0xff,0xff,0xff,0xff},
{0xff,0xff,0xff,0xff,0xff,0xff},
{0xff,0xff,0xff,0xff,0xff,0xff},
{0xff,0xff,0xff,0xff,0xff,0xff},
{0xff,0xff,0xff,0xff,0xff,0xff},
{0xff,0xff,0xff,0xff,0xff,0xff},
{0xff,0xff,0xff,0xff,0xff,0xff},
{0xff,0xff,0xff,0xff,0xff,0xff},
{0xff,0xff,0xff,0xff,0xff,0xff},
{0xff,0xff,0xff,0xff,0xff,0xff}
};

unsigned char value_Buf[4]={0x01,0x00,0x00,0x00};//增减量
unsigned char Write_Buffer[16]={//修改密码A
0xff,0x00,0xff,0x00,0xff,0x00,//KEYA
0xFF,0x07,0x80,0x69,          //控制
0xFF,0xFF,0xFF,0xFF,0xFF,0xFF //KEYB
};
unsigned char Write_Buffer1[16]={//还原密码A
0xff,0xff,0xff,0xff,0xff,0xff,//KEYA
0xFF,0x07,0x80,0x69,//控制
0xFF,0xFF,0xFF,0xFF,0xFF,0xFF//KEYB
};


void Printing(unsigned char* Buffer,unsigned short len);//将数组里的数据发送到串口(字符型)
unsigned char Request_Anticoll_Select( unsigned char request_mode,
	       unsigned short card_type,
	       unsigned char* card_buffer,
	       unsigned char* card_id );//寻卡、防冲撞、选卡
void ModifyKey(void);//修改扇区0的第三块控制块的KEY A
void RecoveryKey(void);//恢复扇区0的KEY A 注销该卡
void CZ(void);//初始化值块
void Increment(void);//加值(充值)
void Decrement(void);//减值(扣款)
void BeiFen(unsigned char sourceaddr_p,unsigned char goaladdr_p);//备份钱包
void hhh(void);
u8 Auto_Reader(void);
void Change_UID(void);
				 
int main(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//中断管理分组设置
	delay_init();
	led_init();
	key_init();
	
	usart1_init(9600);
	usart1_sendstring("USART_INITOK\r\n");

	RFIDGPIO_Init();
	delay_ms(10);
	while(1)
	{	
		
		if(Request_Anticoll_Select(PICC_REQALL,Card_Type,Card_Buffer,Card_ID)==MI_OK)
		{
			PCout(13) = 0;
			PAout()
			delay_ms(100);
		}
		else
		{
			PCout(13) = 1;
			delay_ms(100);
		}
		
	}
}
//将数组里的数据发送到串口(字符型)
void Printing(unsigned char* Buffer,unsigned short len)
{
	for(i=0;i<len;i++)
	{
		USART_SendData(USART1,table[Buffer[i]/16]);
		while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)!=SET);
		USART_SendData(USART1,table[Buffer[i]%16]);
		while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)!=SET);
		USART_SendData(USART1,' ');
		while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)!=SET);
	}
	usart1_sendstring("\r\n");
}


//寻卡、防冲撞、选卡
unsigned char Request_Anticoll_Select(unsigned char request_mode,
	unsigned short card_type,
	unsigned char* card_buffer,
	unsigned char* card_id)
{
	unsigned char status;
	//寻卡 寻天线内未进入休眠的卡
 	if(PcdRequest(request_mode,Card_Buffer)==MI_OK)
	{
		card_type=(card_buffer[0]<<8)|card_buffer[1];
		if(card_type==0x0400)//判断为M1_S50卡
		{
			usart1_sendstring("Request_OK_M1_S50\r\n");
			//防冲撞 返回卡4个字节的序列号
			if(PcdAnticoll(card_id)==MI_OK)
			{
				usart1_sendstring("Card_ID:");
				Printing(Card_ID,4);//打印4个字节的ID
				//选定该卡片(激活)
				if(PcdSelect(card_id)==MI_OK)
				{
					status=0;
					return status;//成功
				}
			}
		}
	}
	status=1;
	return status;//失败
}

//修改扇区1的第0块控制块的KEY A
void ModifyKey(void)
{
	Block=7;//1扇区的控制块
	//寻卡、防冲突、选卡
	if(Request_Anticoll_Select(PICC_REQALL,Card_Type,Card_Buffer,Card_ID)==MI_OK)
	{
		//验证卡片 用初始密码A
		if(PcdAuthState(PICC_AUTHENT1A,Block,Default_Key,Card_ID)==MI_OK)
		{
			USART_SendData(USART1,table[Block/10]);
			while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)!=SET);
			USART_SendData(USART1,table[Block%10]);
			while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)!=SET);
			USART_SendData(USART1,' ');
			while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)!=SET);
			usart1_sendstring("PcdAuthState_OK\r\n");
			//读控制块
			if(PcdRead(Block,Data_Buffer)==MI_OK)
			{
				//要不要验证控制块的存取控制的4个字节有没有发生变化????????
				Printing(Data_Buffer,16);//控制块的数据分为发送到串口
				//修改密码
				if(PcdWrite(Block,Write_Buffer)==MI_OK)
				{
					//再次读出
					usart1_sendstring("Modify_KeyOK\r\n");//修改密码成功
					usart1_sendstring("\r\n");
					BEEP_SET;
					delay_ms(500);
					BEEP_CLR;//提示音							
				}
			}
		}
	}	
}
//恢复扇区1的KEY A 注销该卡
void RecoveryKey(void)
{
	Block=7;//1扇区的控制块
	
	//寻卡、防冲突、选卡
	if(Request_Anticoll_Select(PICC_REQALL,Card_Type,Card_Buffer,Card_ID)==MI_OK)
	{
		//用修改后的密码A验证卡片
		if(PcdAuthState(PICC_AUTHENT1A,Block,Modify_Key,Card_ID)==MI_OK)
		{
			USART_SendData(USART1,table[Block/10]);
			while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)!=SET);
			USART_SendData(USART1,table[Block%10]);
			while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)!=SET);
			USART_SendData(USART1,' ');
			while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)!=SET);
			usart1_sendstring("PcdAuthState_OK\r\n");
			////读控制块
			if(PcdRead(Block,Data_Buffer)==MI_OK)
			{
				Printing(Data_Buffer,16);//控制块的数据分为发送到串口
				//将原始KEY写入
				if(PcdWrite(Block,Write_Buffer1)==MI_OK)
				{
					usart1_sendstring("RecoveryKey_OK\r\n");//注销成功
				  //将钱包值块清空
					////////////////////////////////////
					usart1_sendstring("\r\n");	
					BEEP_SET;
					delay_ms(500);
					BEEP_CLR;							
				}
			}
		}
	}	
}

//初始化值块
void CZ(void)
{
  Block=4;//将1扇区的数据块0初始化为值块 初始值为0
	//寻卡、防冲突、选卡
	if(Request_Anticoll_Select(PICC_REQALL,Card_Type,Card_Buffer,Card_ID)==MI_OK)
	{
		//用修改后的密码A验证卡片
		if(PcdAuthState(PICC_AUTHENT1A,Block,Modify_Key,Card_ID)==MI_OK)
		{
			//将当前操作的块打印出来
			USART_SendData(USART1,table[Block/10]);
			while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)!=SET);
			USART_SendData(USART1,table[Block%10]);
			while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)!=SET);
			USART_SendData(USART1,' ');
			while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)!=SET);
			usart1_sendstring("PcdAuthState_OK\r\n");
			
			//按照值块格式写入
			if(PcdWrite(Block,DefaultValue)==MI_OK)
			{
				if(PcdRead(Block,Data_Buffer)==MI_OK)
				{
					Printing(Data_Buffer,16);//将值块打印到串口
					usart1_sendstring("Init_OK\r\n");//修改密码成功
					usart1_sendstring("\r\n");
					BEEP_SET;
					delay_ms(500);
					BEEP_CLR;	
				}					
			}
		}
	}		
}
void hhh(void)//将
{
	Block=7;
	//寻卡、防冲突、选卡
	if(Request_Anticoll_Select(PICC_REQALL,Card_Type,Card_Buffer,Card_ID)==MI_OK)
	{
		//用修改后的密码A验证卡片
		if(PcdAuthState(PICC_AUTHENT1A,Block,Modify_Key,Card_ID)==MI_OK)
		{
			//将当前操作的块打印出来
			USART_SendData(USART1,table[Block/10]);
			while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)!=SET);
			USART_SendData(USART1,table[Block%10]);
			while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)!=SET);
			USART_SendData(USART1,' ');
			while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)!=SET);
			usart1_sendstring("PcdAuthState_OK\r\n");
			
			if(PcdRead(Block,Data_Buffer)==MI_OK)
			{
				Printing(Data_Buffer,16);//将值块打印到串口
				BEEP_SET;
				delay_ms(500);
				BEEP_CLR;	
			}
//			//按照值块格式写入
//			if(PcdWrite(Block,DefaultValue)==MI_OK)
//			{
//				if(PcdRead(Block,Data_Buffer)==MI_OK)
//				{
//					Printing(Data_Buffer,16);//将值块打印到串口
//					usart1_sendstring("Init_OK\r\n");//修改密码成功
//					usart1_sendstring("\r\n");
//					BEEP_SET;
//					delay_ms(500);
//					BEEP_CLR;	
//				}					
//			}
		}
	}		
	
}
//#define PICC_DECREMENT        0xC0               //扣款
//#define PICC_INCREMENT        0xC1               //充值

//加值(充值)
void Increment(void)
{
	Block=4;
	//寻卡、防冲突、选卡
	if(Request_Anticoll_Select(PICC_REQALL,Card_Type,Card_Buffer,Card_ID)==MI_OK)
	{
		//用修改后的密码A验证卡片
		if(PcdAuthState(PICC_AUTHENT1A,Block,Modify_Key,Card_ID)==MI_OK)
		{
			//将当前操作的块打印出来
			USART_SendData(USART1,table[Block/10]);
			while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)!=SET);
			USART_SendData(USART1,table[Block%10]);
			while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)!=SET);
			USART_SendData(USART1,' ');
			while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)!=SET);
			usart1_sendstring("PcdAuthState_OK\r\n");
			
			//加之前读一遍钱包
			if(PcdRead(Block,Data_Buffer)==MI_OK)
			{
				Printing(Data_Buffer,16);//加之前的读出来打印
				
				if(Data_Buffer[0]>=10)//判断是否还能再加 
				{
					BEEP_SET;delay_ms(50);BEEP_CLR;delay_ms(50);	
					BEEP_SET;delay_ms(50);BEEP_CLR;delay_ms(50);
					BEEP_SET;delay_ms(50);BEEP_CLR;//超值报警
					return;
				}
				//加值 地址 
				if(PcdValue(PICC_INCREMENT,Block,value_Buf)==MI_OK)
				{
					if(PcdRead(Block,Data_Buffer)==MI_OK)
					{
						Printing(Data_Buffer,16);
						usart1_sendstring("+_OK\r\n");
						usart1_sendstring("\r\n");
						BEEP_SET;
						delay_ms(50);
						BEEP_CLR;	
					}			
				}
			}
		}
	}			
}
//减值(扣款)
void Decrement(void)
{
	Block=4;
	//寻卡、防冲突、选卡
	if(Request_Anticoll_Select(PICC_REQALL,Card_Type,Card_Buffer,Card_ID)==MI_OK)
	{
		//用修改后的密码A验证卡片
		if(PcdAuthState(PICC_AUTHENT1A,Block,Modify_Key,Card_ID)==MI_OK)
		{
			//将当前操作的块打印出来
			USART_SendData(USART1,table[Block/10]);
			while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)!=SET);
			USART_SendData(USART1,table[Block%10]);
			while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)!=SET);
			USART_SendData(USART1,' ');
			while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)!=SET);
			usart1_sendstring("PcdAuthState_OK\r\n");
			
			//减前读一遍钱包
			if(PcdRead(Block,Data_Buffer)==MI_OK)
			{
				Printing(Data_Buffer,16);
				
				if(Data_Buffer[0]<=0)//判断是否还能再减 
				{
					BEEP_SET;delay_ms(50);BEEP_CLR;delay_ms(50);	
					BEEP_SET;delay_ms(50);BEEP_CLR;delay_ms(50);
					BEEP_SET;delay_ms(50);BEEP_CLR;//超值报警
					return;
				}
				//减值  
				if(PcdValue(PICC_DECREMENT,Block,value_Buf)==MI_OK)
				{
					if(PcdRead(Block,Data_Buffer)==MI_OK)
					{
						Printing(Data_Buffer,16);
						usart1_sendstring("-_OK\r\n");
						usart1_sendstring("\r\n");
						BEEP_SET;
						delay_ms(50);
						BEEP_CLR;	
					}			
				}
			}
		}
	}
}

//备份钱包
void BeiFen(unsigned char sourceaddr_p,unsigned char goaladdr_p)
{
	//寻卡、防冲突、选卡
	if(Request_Anticoll_Select(PICC_REQALL,Card_Type,Card_Buffer,Card_ID)==MI_OK)
	{
		//用修改后的密码A验证卡片
		if(PcdAuthState(PICC_AUTHENT1A,sourceaddr_p,Modify_Key,Card_ID)==MI_OK)
		{
			//将当前操作的块打印出来
			USART_SendData(USART1,table[sourceaddr_p/10]);
			while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)!=SET);
			USART_SendData(USART1,table[sourceaddr_p%10]);
			while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)!=SET);
			USART_SendData(USART1,' ');
			while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)!=SET);
			usart1_sendstring("PcdAuthState_OK\r\n");

			//备份前读一遍该块 
			if(PcdRead(goaladdr_p,Data_Buffer)==MI_OK)
			{
				Printing(Data_Buffer,16);
				//加之前的读出来打印
				//参数1备份地 参数2目标地址  同一扇区内
				if(PcdBakValue(sourceaddr_p,goaladdr_p)==MI_OK)
				{
					usart1_sendstring("BeiFen_OK\r\n");
					//读备份到的块
					if(PcdRead(goaladdr_p,Data_Buffer)==MI_OK)
					{
						Printing(Data_Buffer,16);//备份后打印出来对比
						BEEP_SET;
						delay_ms(500);
						BEEP_CLR;	
					}
				}
			}
		}
	}			
}



/*
INCREMENT 对存储在数值块中的数值做加法操作，并将结果存到临时数据寄存器
DECREMENT 对存储在数值块中的数值做减法操作，并将结果存到临时数据寄存器
TRANSFER  将临时数据寄存器的内容写入数值块??????????????????
RESTORE   将数值块内容存入临时数据寄存器??????????????????

电子钱包增值函数流程
询卡-防冲突-选卡-密码验证-增值-传送-挂起

电子钱包减值函数流程
询卡-防冲突-选卡-密码验证-减值-传送-挂起

数值时一个带符号4字节值，这个值的最低一个字节保存在最低的地址中，所以增加1时，
4字节增加的值数组应该是0×01 0×00 0×00 0×00,不是0×00 0×00 0×00 0×01
*/

void Change_UID(void)
{
	unsigned int  unLen;
	unsigned char ucComMF522Buf[MAXRLEN];
	unsigned char bcc,block0_buffer[18]={0};
//	if(Request_Anticoll_Select(PICC_REQALL,Card_Type,Card_Buffer,Card_ID)==MI_OK)
//	{
//		if(PcdAuthState(PICC_AUTHENT1A,0,Default_Key,Card_ID)==MI_OK)
//		{
//			if(PcdRead(0,ucComMF522Buf) == MI_OK)
//			{
//				for(i=0;i<16;i++)
//				{
//					block0_buffer[i] = ucComMF522Buf[i];
//				}
//				CalulateCRC(ucComMF522Buf,16,&block0_buffer[16]);
//				PcdReset();     //RC522初始化
//				PcdAntennaOff();//关天线
//				delay_ms(1);    
//				PcdAntennaOn(); //开天线
//				delay_ms(200);
				block0_buffer[0] = 0x33;
				if(Request_Anticoll_Select(PICC_REQALL,Card_Type,Card_Buffer,Card_ID)==MI_OK)
				{
					PcdHalt();//休眠
					WriteRawRC(BitFramingReg,0x07);
					ucComMF522Buf[0] = 0x40;
					if(PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,1,ucComMF522Buf,&unLen) == MI_OK)
					{
						ucComMF522Buf[0] = 0x43;
						WriteRawRC(BitFramingReg,0x00);
						if(PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,1,ucComMF522Buf,&unLen) == MI_OK)
						{
							bcc =block0_buffer[0];
							for(i=1;i<4;i++)
							{
								bcc^=block0_buffer[i];
							}
							block0_buffer[i] = bcc;
							if(PcdWrite(0x00,block0_buffer) == MI_OK)
							{
								//成功
								ucComMF522Buf[0] = 0x00;//
							}
						}
					}
				}
//			}
//		}
//	}
}

u8 Copy_ICdate(void)
{
	
}
