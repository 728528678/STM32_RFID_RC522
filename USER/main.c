/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  ** Author	:Lin
  *	 Email	:ljb728528678@qq.com
  *
  *
  ******************************************************************************
  */

#include "stm32f10x.h"
#include "delay.h"
#include "ledkey.h"
#include "usart.h"
#include "rc522.h"



unsigned char Block=0;
unsigned char Card_Buffer[2];//���ڽ��տ����ص���������
unsigned short Card_Type;//��������
unsigned char Card_ID[4];//���ڴ�ſ������к�
unsigned char UID[5],Temp[4];
unsigned char i;
unsigned char Data_Buffer[16];//�������ݻ���
unsigned char table[16]={
'0','1','2','3',
'4','5','6','7',
'8','9','A','B',
'C','D','E','F'};
unsigned char Modify_Key[6]={0xA0,0xA1,0xA2,0xA3,0xA4,0xA5};//�޸ĺ������
unsigned char Default_Key[6]={0xff,0xff,0xff,0xff,0xff,0xff};//��ʼ����
unsigned char DefaultValue[16]={//��ֵ��4��ʼ��0x00000000
0x00,0x00,0x00,0x00,//ֵ����
0xff,0xff,0xff,0xff,//ֵ����
0x00,0x00,0x00,0x00,//ֵ����
0x04,//��ַ����
0xfb,//��ַ����
0x04,//��ַ����
0xfb //��ַ����
};

u8 User_Key[16][6]={
{0xff,0xff,0xff,0xff,0xff,0xff},//0
{0xD0,0x7D,0xA5,0x54,0xDD,0xD0},//{0x58,0x04,0x5D,0x74,0x09,0xFE},//1
{0xD0,0x7D,0xA5,0x54,0xDD,0xD0},//{0x58,0x04,0x5D,0x74,0x09,0xFE},//2
{0xD0,0x7D,0xA5,0x54,0xDD,0xD0},//{0x58,0x04,0x5D,0x74,0x09,0xFE},//3
{0xff,0xff,0xff,0xff,0xff,0xff},//{0x10,0x11,0x12,0x13,0x14,0x15},//4
{0xff,0xff,0xff,0xff,0xff,0xff},//5
{0xff,0xff,0xff,0xff,0xff,0xff},//{0xF9,0xE2,0xEF,0xB4,0x78,0xCD},//6
{0xff,0xff,0xff,0xff,0xff,0xff},//7
{0xff,0xff,0xff,0xff,0xff,0xff},//8
{0xff,0xff,0xff,0xff,0xff,0xff},//{0x2F,0x4D,0x1A,0xA8,0x3A,0xCC},//9
{0xff,0xff,0xff,0xff,0xff,0xff},//10
{0xff,0xff,0xff,0xff,0xff,0xff},//11
{0xff,0xff,0xff,0xff,0xff,0xff},//12
{0xff,0xff,0xff,0xff,0xff,0xff},//13
{0xff,0xff,0xff,0xff,0xff,0xff},//14
{0xff,0xff,0xff,0xff,0xff,0xff}//15
};

//��ȥ����λ����������ȫ������
#define SECTION_NUM	16*3
#define DATA_NUM 	16
u8 Card_Data[SECTION_NUM][DATA_NUM]={0};

unsigned char value_Buf[4]={0x01,0x00,0x00,0x00};//������
unsigned char Write_Buffer[16]={//�޸�����A
0xff,0x00,0xff,0x00,0xff,0x00,//KEYA
0xFF,0x07,0x80,0x69,          //����
0xFF,0xFF,0xFF,0xFF,0xFF,0xFF //KEYB
};
unsigned char Write_Buffer1[16]={//��ԭ����A
0xff,0xff,0xff,0xff,0xff,0xff,//KEYA
0xFF,0x07,0x80,0x69,//����
0xFF,0xFF,0xFF,0xFF,0xFF,0xFF//KEYB
};


void Printing(unsigned char* Buffer,unsigned short len);//������������ݷ��͵�����(�ַ���)
unsigned char Request_Anticoll_Select( unsigned char request_mode,
	       unsigned short card_type,
	       unsigned char* card_buffer,
	       unsigned char* card_id );//Ѱ��������ײ��ѡ��
void ModifyKey(void);//�޸�����0�ĵ�������ƿ��KEY A
void RecoveryKey(void);//�ָ�����0��KEY A ע���ÿ�
void CZ(void);//��ʼ��ֵ��
void Increment(void);//��ֵ(��ֵ)
void Decrement(void);//��ֵ(�ۿ�)
void BeiFen(unsigned char sourceaddr_p,unsigned char goaladdr_p);//����Ǯ��
void hhh(void);
u8 Auto_Reader(void);
void Change_UID(void);
u8 Copy_ICdate(void);
u8 Write_ICdata(void);
u8 Key_Flag = 0;

int main(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//�жϹ����������
	delay_init();
	led_init();
	key_init();
	usart1_init(9600);
	usart1_sendstring("USART_INITOK\r\n");

	RFIDGPIO_Init();
	delay_ms(10);
	while(1)
	{
		Key_Flag = Read_Key();
		if(Key_Flag == 1)//д��
		{
			Key_Flag = 0;
			if(Write_ICdata() == 1)//д���ɹ�
			{
				LED_G = 0;
				LED_R = 1;
			}
			else
			{
				LED_G = 1;
				LED_R = 0;
			}
		}
		else if(Key_Flag == 2)//����
		{
			Key_Flag = 0;
			if(Copy_ICdate() == 1)//�����ɹ�
			{
				LED_G = 0;
				LED_R = 1;
				LED_DATA = 0;
			}
			else
			{
				LED_G = 1;
				LED_R = 0;
				LED_DATA = 1;
			}
		}
		
	}
}

//������������ݷ��͵�����(�ַ���)
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


//Ѱ��������ײ��ѡ��
unsigned char Request_Anticoll_Select(unsigned char request_mode,
	unsigned short card_type,
	unsigned char* card_buffer,
	unsigned char* card_id)
{
	unsigned char status;
	//Ѱ�� Ѱ������δ�������ߵĿ�
 	if(PcdRequest(request_mode,Card_Buffer)==MI_OK)
	{
		card_type=(card_buffer[0]<<8)|card_buffer[1];
		if(card_type==0x0400)//�ж�ΪM1_S50��
		{
			usart1_sendstring("Request_OK_M1_S50\r\n");
			//����ײ ���ؿ�4���ֽڵ����к�
			if(PcdAnticoll(card_id)==MI_OK)
			{
				usart1_sendstring("Card_ID:");
				Printing(Card_ID,4);//��ӡ4���ֽڵ�ID
				//ѡ���ÿ�Ƭ(����)
				if(PcdSelect(card_id)==MI_OK)
				{
					status=0;
					return status;//�ɹ�
				}
			}
		}
	}
	status=1;
	return status;//ʧ��
}

//�޸�����1�ĵ�0����ƿ��KEY A
void ModifyKey(void)
{
	Block=7;//1�����Ŀ��ƿ�
	//Ѱ��������ͻ��ѡ��
	if(Request_Anticoll_Select(PICC_REQALL,Card_Type,Card_Buffer,Card_ID)==MI_OK)
	{
		//��֤��Ƭ �ó�ʼ����A
		if(PcdAuthState(PICC_AUTHENT1A,Block,Default_Key,Card_ID)==MI_OK)
		{
			USART_SendData(USART1,table[Block/10]);
			while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)!=SET);
			USART_SendData(USART1,table[Block%10]);
			while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)!=SET);
			USART_SendData(USART1,' ');
			while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)!=SET);
			usart1_sendstring("PcdAuthState_OK\r\n");
			//�����ƿ�
			if(PcdRead(Block,Data_Buffer)==MI_OK)
			{
				//Ҫ��Ҫ��֤���ƿ�Ĵ�ȡ���Ƶ�4���ֽ���û�з����仯????????
				Printing(Data_Buffer,16);//���ƿ�����ݷ�Ϊ���͵�����
				//�޸�����
				if(PcdWrite(Block,Write_Buffer)==MI_OK)
				{
					//�ٴζ���
					usart1_sendstring("Modify_KeyOK\r\n");//�޸�����ɹ�
					usart1_sendstring("\r\n");
					BEEP_SET;
					delay_ms(500);
					BEEP_CLR;//��ʾ��							
				}
			}
		}
	}	
}
//�ָ�����1��KEY A ע���ÿ�
void RecoveryKey(void)
{
	Block=7;//1�����Ŀ��ƿ�
	
	//Ѱ��������ͻ��ѡ��
	if(Request_Anticoll_Select(PICC_REQALL,Card_Type,Card_Buffer,Card_ID)==MI_OK)
	{
		//���޸ĺ������A��֤��Ƭ
		if(PcdAuthState(PICC_AUTHENT1A,Block,Modify_Key,Card_ID)==MI_OK)
		{
			USART_SendData(USART1,table[Block/10]);
			while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)!=SET);
			USART_SendData(USART1,table[Block%10]);
			while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)!=SET);
			USART_SendData(USART1,' ');
			while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)!=SET);
			usart1_sendstring("PcdAuthState_OK\r\n");
			////�����ƿ�
			if(PcdRead(Block,Data_Buffer)==MI_OK)
			{
				Printing(Data_Buffer,16);//���ƿ�����ݷ�Ϊ���͵�����
				//��ԭʼKEYд��
				if(PcdWrite(Block,Write_Buffer1)==MI_OK)
				{
					usart1_sendstring("RecoveryKey_OK\r\n");//ע���ɹ�
				  //��Ǯ��ֵ�����
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

//��ʼ��ֵ��
void CZ(void)
{
  Block=4;//��1���������ݿ�0��ʼ��Ϊֵ�� ��ʼֵΪ0
	//Ѱ��������ͻ��ѡ��
	if(Request_Anticoll_Select(PICC_REQALL,Card_Type,Card_Buffer,Card_ID)==MI_OK)
	{
		//���޸ĺ������A��֤��Ƭ
		if(PcdAuthState(PICC_AUTHENT1A,Block,Modify_Key,Card_ID)==MI_OK)
		{
			//����ǰ�����Ŀ��ӡ����
			USART_SendData(USART1,table[Block/10]);
			while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)!=SET);
			USART_SendData(USART1,table[Block%10]);
			while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)!=SET);
			USART_SendData(USART1,' ');
			while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)!=SET);
			usart1_sendstring("PcdAuthState_OK\r\n");
			
			//����ֵ���ʽд��
			if(PcdWrite(Block,DefaultValue)==MI_OK)
			{
				if(PcdRead(Block,Data_Buffer)==MI_OK)
				{
					Printing(Data_Buffer,16);//��ֵ���ӡ������
					usart1_sendstring("Init_OK\r\n");//�޸�����ɹ�
					usart1_sendstring("\r\n");
					BEEP_SET;
					delay_ms(500);
					BEEP_CLR;	
				}					
			}
		}
	}		
}
void hhh(void)//��
{
	Block=7;
	//Ѱ��������ͻ��ѡ��
	if(Request_Anticoll_Select(PICC_REQALL,Card_Type,Card_Buffer,Card_ID)==MI_OK)
	{
		//���޸ĺ������A��֤��Ƭ
		if(PcdAuthState(PICC_AUTHENT1A,Block,Modify_Key,Card_ID)==MI_OK)
		{
			//����ǰ�����Ŀ��ӡ����
			USART_SendData(USART1,table[Block/10]);
			while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)!=SET);
			USART_SendData(USART1,table[Block%10]);
			while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)!=SET);
			USART_SendData(USART1,' ');
			while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)!=SET);
			usart1_sendstring("PcdAuthState_OK\r\n");
			
			if(PcdRead(Block,Data_Buffer)==MI_OK)
			{
				Printing(Data_Buffer,16);//��ֵ���ӡ������
				BEEP_SET;
				delay_ms(500);
				BEEP_CLR;	
			}
//			//����ֵ���ʽд��
//			if(PcdWrite(Block,DefaultValue)==MI_OK)
//			{
//				if(PcdRead(Block,Data_Buffer)==MI_OK)
//				{
//					Printing(Data_Buffer,16);//��ֵ���ӡ������
//					usart1_sendstring("Init_OK\r\n");//�޸�����ɹ�
//					usart1_sendstring("\r\n");
//					BEEP_SET;
//					delay_ms(500);
//					BEEP_CLR;	
//				}					
//			}
		}
	}		
	
}
//#define PICC_DECREMENT        0xC0               //�ۿ�
//#define PICC_INCREMENT        0xC1               //��ֵ

//��ֵ(��ֵ)
void Increment(void)
{
	Block=4;
	//Ѱ��������ͻ��ѡ��
	if(Request_Anticoll_Select(PICC_REQALL,Card_Type,Card_Buffer,Card_ID)==MI_OK)
	{
		//���޸ĺ������A��֤��Ƭ
		if(PcdAuthState(PICC_AUTHENT1A,Block,Modify_Key,Card_ID)==MI_OK)
		{
			//����ǰ�����Ŀ��ӡ����
			USART_SendData(USART1,table[Block/10]);
			while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)!=SET);
			USART_SendData(USART1,table[Block%10]);
			while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)!=SET);
			USART_SendData(USART1,' ');
			while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)!=SET);
			usart1_sendstring("PcdAuthState_OK\r\n");
			
			//��֮ǰ��һ��Ǯ��
			if(PcdRead(Block,Data_Buffer)==MI_OK)
			{
				Printing(Data_Buffer,16);//��֮ǰ�Ķ�������ӡ
				
				if(Data_Buffer[0]>=10)//�ж��Ƿ����ټ� 
				{
					BEEP_SET;delay_ms(50);BEEP_CLR;delay_ms(50);	
					BEEP_SET;delay_ms(50);BEEP_CLR;delay_ms(50);
					BEEP_SET;delay_ms(50);BEEP_CLR;//��ֵ����
					return;
				}
				//��ֵ ��ַ 
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
//��ֵ(�ۿ�)
void Decrement(void)
{
	Block=4;
	//Ѱ��������ͻ��ѡ��
	if(Request_Anticoll_Select(PICC_REQALL,Card_Type,Card_Buffer,Card_ID)==MI_OK)
	{
		//���޸ĺ������A��֤��Ƭ
		if(PcdAuthState(PICC_AUTHENT1A,Block,Modify_Key,Card_ID)==MI_OK)
		{
			//����ǰ�����Ŀ��ӡ����
			USART_SendData(USART1,table[Block/10]);
			while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)!=SET);
			USART_SendData(USART1,table[Block%10]);
			while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)!=SET);
			USART_SendData(USART1,' ');
			while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)!=SET);
			usart1_sendstring("PcdAuthState_OK\r\n");
			
			//��ǰ��һ��Ǯ��
			if(PcdRead(Block,Data_Buffer)==MI_OK)
			{
				Printing(Data_Buffer,16);
				
				if(Data_Buffer[0]<=0)//�ж��Ƿ����ټ� 
				{
					BEEP_SET;delay_ms(50);BEEP_CLR;delay_ms(50);	
					BEEP_SET;delay_ms(50);BEEP_CLR;delay_ms(50);
					BEEP_SET;delay_ms(50);BEEP_CLR;//��ֵ����
					return;
				}
				//��ֵ  
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

//����Ǯ��
void BeiFen(unsigned char sourceaddr_p,unsigned char goaladdr_p)
{
	//Ѱ��������ͻ��ѡ��
	if(Request_Anticoll_Select(PICC_REQALL,Card_Type,Card_Buffer,Card_ID)==MI_OK)
	{
		//���޸ĺ������A��֤��Ƭ
		if(PcdAuthState(PICC_AUTHENT1A,sourceaddr_p,Modify_Key,Card_ID)==MI_OK)
		{
			//����ǰ�����Ŀ��ӡ����
			USART_SendData(USART1,table[sourceaddr_p/10]);
			while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)!=SET);
			USART_SendData(USART1,table[sourceaddr_p%10]);
			while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)!=SET);
			USART_SendData(USART1,' ');
			while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)!=SET);
			usart1_sendstring("PcdAuthState_OK\r\n");

			//����ǰ��һ��ÿ� 
			if(PcdRead(goaladdr_p,Data_Buffer)==MI_OK)
			{
				Printing(Data_Buffer,16);
				//��֮ǰ�Ķ�������ӡ
				//����1���ݵ� ����2Ŀ���ַ  ͬһ������
				if(PcdBakValue(sourceaddr_p,goaladdr_p)==MI_OK)
				{
					usart1_sendstring("BeiFen_OK\r\n");
					//�����ݵ��Ŀ�
					if(PcdRead(goaladdr_p,Data_Buffer)==MI_OK)
					{
						Printing(Data_Buffer,16);//���ݺ��ӡ�����Ա�
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
INCREMENT �Դ洢����ֵ���е���ֵ���ӷ���������������浽��ʱ���ݼĴ���
DECREMENT �Դ洢����ֵ���е���ֵ��������������������浽��ʱ���ݼĴ���
TRANSFER  ����ʱ���ݼĴ���������д����ֵ��??????????????????
RESTORE   ����ֵ�����ݴ�����ʱ���ݼĴ���??????????????????

����Ǯ����ֵ��������
ѯ��-����ͻ-ѡ��-������֤-��ֵ-����-����

����Ǯ����ֵ��������
ѯ��-����ͻ-ѡ��-������֤-��ֵ-����-����

��ֵʱһ��������4�ֽ�ֵ�����ֵ�����һ���ֽڱ�������͵ĵ�ַ�У���������1ʱ��
4�ֽ����ӵ�ֵ����Ӧ����0��01 0��00 0��00 0��00,����0��00 0��00 0��00 0��01
*/

void Change_UID(void)
{
	unsigned int  unLen;
	unsigned char ucComMF522Buf[MAXRLEN];
	unsigned char bcc,block0_buffer[18]={0};
	if(Request_Anticoll_Select(PICC_REQALL,Card_Type,Card_Buffer,Card_ID)==MI_OK)//Ѱ��������ײ��ѡ��
	{
		if(PcdAuthState(PICC_AUTHENT1A,0,Default_Key,Card_ID)==MI_OK)//��֤��Կ
		{
			if(PcdRead(0,ucComMF522Buf) == MI_OK)//��һ��������
			{
				for(i=0;i<16;i++)
				{
					block0_buffer[i] = ucComMF522Buf[i];
				}
				CalulateCRC(ucComMF522Buf,16,&block0_buffer[16]);
				PcdReset();     //RC522��ʼ��
				PcdAntennaOff();//������
				delay_ms(1);    
				PcdAntennaOn(); //������
				delay_ms(200);
				block0_buffer[0] = 0x33;
				if(Request_Anticoll_Select(PICC_REQALL,Card_Type,Card_Buffer,Card_ID)==MI_OK)
				{
					PcdHalt();//����
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
								//�ɹ�
								ucComMF522Buf[0] = 0x00;//
							}
						}
					}
				}
			}
		}
	}
}

//����Ĭ����Կ��ȡIC������
u8 Read_ICdata(void)
{
	u8 i,j;
	for(i=0;i<16;i++)//16����
	{
		j=0;
		if(PcdAuthState(PICC_AUTHENT1A,i*4,&User_Key[i][0],Card_ID)==MI_OK)//��֤i������A��Կ
		{
			for(j=0;j<3;j++)//����Կ��ʣ3����
			{
				if(PcdRead(i*4+j,&Card_Data[i*3+j][0]) == MI_OK)
				{
					delay_ms(15);
					LED_G = 0;
					delay_ms(20);
					LED_G = 1;
				}
				else
				{
					return 0;
				}
			}
		}
		else
		{
			return 0;//ʧ��
		}
	}
	return 1;
}

u8 Write_ICdata(void)
{
	u8 i,j;
	if(Request_Anticoll_Select(PICC_REQALL,Card_Type,Card_Buffer,Card_ID)==MI_OK)//Ѱ��������ײ��ѡ��
	{
		for(i=1;i<16;i++)//16����
		{
			j=0;
			if(PcdAuthState(PICC_AUTHENT1A,i*4,&User_Key[i][0],Card_ID)==MI_OK)//��֤i������A��Կ
			{
				for(j=0;j<3;j++)//����Կ��ʣ3����
				{
					if(PcdWrite(i*4+j,&Card_Data[i*3+j][0]) == MI_OK)
					{
						delay_ms(15);
						LED_G = 0;
						delay_ms(20);
						LED_G = 1;
					}
					else
					{
						return 0;
					}
				}
			}
			else
			{
				return 0;//ʧ��
			}
		}
		return 1;
	}
	else
	{
		return 0;
	}
}

//���Ƴ�ID���������ݵ�Card_Buffer����
u8 Copy_ICdate(void)
{
	if(Request_Anticoll_Select(PICC_REQALL,Card_Type,Card_Buffer,Card_ID)==MI_OK)//Ѱ��������ײ��ѡ��
	{
		return Read_ICdata();
	}
	return 0;
}
