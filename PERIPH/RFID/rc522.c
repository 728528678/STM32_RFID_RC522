#include "rc522.h"
#include "ledkey.h"
#include "usart.h"
#include "delay.h"
#include "spi.h"

#define CS_0 GPIO_ResetBits(GPIOB,GPIO_Pin_12)
#define CS_1 GPIO_SetBits(GPIOB,GPIO_Pin_12)
#define RST_0 GPIO_ResetBits(GPIOB,GPIO_Pin_11)
#define RST_1 GPIO_SetBits(GPIOB,GPIO_Pin_11)

//RFID_RC522 GPIO SPI初始化
//RFID_NSS(CS)   PB12
//RFID_RST       PC0
//RFID_CLK       PB13
//RFID_MOSI      PB15
//RFID_MISO      PB14

void RFIDGPIO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_PP;//推挽输出
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_12;      //RFID_CS
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOB,&GPIO_InitStructure);
	GPIO_SetBits(GPIOB,GPIO_Pin_12);//拉高RFID_CS

	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_PP;//推挽输出
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_11;//RFID_RST
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOB,&GPIO_InitStructure);
	GPIO_SetBits(GPIOB,GPIO_Pin_11);//拉高
	
//	spi2_init();//SPI2初始化
//	if(PcdReset()==0)
//		usart1_sendstring("RFID_INITOK\r\n");//复位成功
//	else 
//		usart1_sendstring("RFID_INITNO\r\n");//复位失败
	
	spi2_init();    //SPI2初始化
	PcdReset();     //RC522初始化
	PcdAntennaOff();//关天线
	delay_ms(1);    
	PcdAntennaOn(); //开天线
//	M500PcdConfigISOType('A');//针对IOS14443A型卡进行初始化
	
}
//复制数组数据
void trans(unsigned char* table,unsigned char* table1,unsigned char num)
{
	unsigned char i;
	for(i=0;i<num;i++)
	{
		table[i]=table1[i];
	}
}

//功    能：复位RC522
//返    回: 成功返回MI_OK
char PcdReset(void)
{   	
		RST_1;
		delay_us(10);
	  WriteRawRC(CommandReg,PCD_RESETPHASE);//启动命令的执行-执行复位指令
		delay_us(1);

		WriteRawRC(ModeReg,0x3d);//0x29?//0x3d//和Mifare卡通讯，CRC初始值0x6363
		WriteRawRC(TReloadRegL,30);//timer=15ms//30
		WriteRawRC(TReloadRegH,0);
		WriteRawRC(TModeReg,0x8D);//Prescaler=3390
		WriteRawRC(TPrescalerReg,0x3E);
		WriteRawRC(TxAskReg,0x40);		//forced to 100%ASK	 天线驱动
	  
		PcdAntennaOn(); 
		return MI_OK;
}

//开启天线  
//每次启动或关闭天线发射之间应至少有1ms的间隔
void PcdAntennaOn(void)
{
    unsigned char i;
    i = ReadRawRC(TxControlReg);//读天线驱动寄存器
    if(!(i&0x03))
    {
      SetBitMask(TxControlReg, 0x03);
			//将TxControlReg寄存器的0、1位置位
			//X1、TX2输出信号将传递经发送数据调制的13.56MHz的能量载波信
    }
}

//关闭天线
void PcdAntennaOff(void)
{
	ClearBitMask(TxControlReg, 0x03);//清除寄存器位
}

//设置RC632的工作方式 
char M500PcdConfigISOType(unsigned char type)
{
   if(type=='A')//ISO14443_A
   { 
     ClearBitMask(Status2Reg,0x08);
		 //包含接收器、发送器和数据模式检测器的状态标志。
		 //内部温度传感器关断时该位置位
     WriteRawRC(ModeReg,0x3D);//3F
     //定义发送和接收的常用模式。6363 CRC
		 
     WriteRawRC(RxSelReg,0x86);//84
     //选择内部接收器设置。
     WriteRawRC(RFCfgReg,0x7F);
		 //该寄存器定义了接收器信号电压的增益因子 48dB

   	 WriteRawRC(TReloadRegL,30);//tmoLength);//TReloadVal = 'h6a =tmoLength(dec) 
	   WriteRawRC(TReloadRegH,0);//定义16位长的定时器重装值,当一个启动事件出现时， TReload 的值装入定时器
     
		 WriteRawRC(TModeReg,0x8D);
	   WriteRawRC(TPrescalerReg,0x3E);
		 //定义内部定时器的设置

	   delay_ms(10);
     PcdAntennaOn();//开启天线
		 delay_ms(1);
   }
   else
		{ 
	    return 1;
		}
   return MI_OK;
}
//功    能：命令卡片进入休眠状态
//返    回: 成功返回MI_OK
char PcdHalt(void)
{
    char status;
    unsigned int  unLen;
    unsigned char ucComMF522Buf[MAXRLEN]; 

    ucComMF522Buf[0] = PICC_HALT;
    ucComMF522Buf[1] = 0;
    CalulateCRC(ucComMF522Buf,2,&ucComMF522Buf[2]);
 
    status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,4,ucComMF522Buf,&unLen);

    return status;
}
/*
* 函数介绍：读RC523寄存器
* 输入参数：Address:寄存器地址
* 输出参数：无
* 返回值  ：读出的值
*/
unsigned char ReadRawRC(unsigned char Address)
{
  unsigned char ucAddr;
  unsigned char ucResult=0;
	CS_0;
	ucAddr=((Address<<1)&0x7E)|0x80;//写 地址最高位为0,读 地址最高位为1。最低位为0。
	spi2_readwritebyte(ucAddr);

  ucResult=spi2_readwritebyte(0xff);//读数据
	CS_1;
	return ucResult;
}
/*
* 函数介绍：写RC523寄存器
* 输入参数：Address:寄存器地址,value:写入的值
* 输出参数：无
* 返回值  ：无
*/
void WriteRawRC(unsigned char Address, unsigned char value)
{  
  unsigned char ucAddr;
	
	CS_0;
	ucAddr = ((Address<<1)&0x7E);
	spi2_readwritebyte(ucAddr);
	
	spi2_readwritebyte(value);
	CS_1;
}
//功    能：置RC522寄存器位
//参数说明：reg[IN]:寄存器地址
//          mask[IN]:置位值
void SetBitMask(unsigned char reg,unsigned char mask)  
{
    char tmp = 0x0;
    tmp = ReadRawRC(reg);
    WriteRawRC(reg,tmp | mask);  // set bit mask
}

//功    能：清RC522寄存器位
//参数说明：reg[IN]:寄存器地址
//          mask[IN]:清位值
void ClearBitMask(unsigned char reg,unsigned char mask)  
{
    char tmp = 0x0;
    tmp = ReadRawRC(reg);//先将值读出
    WriteRawRC(reg, tmp & ~mask);  // clear bit mask
} 
//用MF522计算CRC16函数
void CalulateCRC(unsigned char *pIndata,unsigned char len,unsigned char *pOutData)
{
    unsigned char i,n;
    ClearBitMask(DivIrqReg,0x04);
	  //该寄存器包含中断请求 该位为当CRC命令有效且所有数据被处理时,该位置位。
    WriteRawRC(CommandReg,PCD_IDLE);//取消当前命令
	
    SetBitMask(FIFOLevelReg,0x80);
	  //FIFOLevelReg寄存器用于指示FIFO中保存的字节数 
    for (i=0; i<len; i++)
    {   
		  WriteRawRC(FIFODataReg, *(pIndata+i));//64字节FIFO缓冲器的输入输出   
		}
      WriteRawRC(CommandReg, PCD_CALCCRC);  //CRC计算命令
    i = 0xFF;
    do 
    {
       n = ReadRawRC(DivIrqReg);//读中断请求标志位
       i--;
    }
    while ((i!=0) && !(n&0x04));//判断当CRC命令有效且所有数据被处理 
    pOutData[0] = ReadRawRC(CRCResultRegL);//该寄存器显示了CRC寄存器的低字节的实际值。
    pOutData[1] = ReadRawRC(CRCResultRegM);//该寄存器显示了CRC寄存器的高字节的实际值。
}

//功    能：通过RC522和ISO14443卡通讯
//参数说明：Command[IN]:RC522命令字
//          pInData[IN]:通过RC522发送到卡片的数据
//          InLenByte[IN]:发送数据的字节长度
//          pOutData[OUT]:接收到的卡片返回数据
//          *pOutLenBit[OUT]:返回数据的位长度
char PcdComMF522(unsigned char Command, 
                 unsigned char *pInData, 
                 unsigned char InLenByte,
                 unsigned char *pOutData, 
                 unsigned int  *pOutLenBit)
{
    char status = MI_EXE_ERR;//MI_EXE_ERR=98
    unsigned char irqEn   = 0x00;
    unsigned char waitFor = 0x00;
    unsigned char lastBits;
    unsigned char n;
    unsigned int i;
    switch (Command)//RC_522命令字
    {
       case PCD_AUTHENT://验证密钥命令
          irqEn= 0x12;
          waitFor=0x10;
          break;
       case PCD_TRANSCEIVE://发送并接收数据
          irqEn   = 0x77;
          waitFor = 0x30;
          break;
       default:
         break;
    }
    WriteRawRC(ComIEnReg,irqEn|0x80);//invert IRQ引脚IRQ是一个标准CMOS输出引脚
    ClearBitMask(ComIrqReg,0x80);//clear all interrupt request bits清除所有中断请求位
    WriteRawRC(CommandReg,PCD_IDLE);//发送启动和停止命令
		
    SetBitMask(FIFOLevelReg,0x80);//clear the pointer立即清除内部FIFO缓冲器的读写指针
		
    for (i=0;i<InLenByte;i++)//要发1Byte
    {   
		  WriteRawRC(FIFODataReg,pInData[i]);// 
	}
	
    WriteRawRC(CommandReg,Command);//先发送启动和停止命令 后面紧跟命令字
    
    if (Command == PCD_TRANSCEIVE)
    {    
		  SetBitMask(BitFramingReg,0x80);  
	}	//start the PCD_TRANSCEIVE command 启动PCDL收发命令
    i=2000;	//根据时钟频率调整，操作M1卡最大等待时间25ms
    do 
    {
        n = ReadRawRC(ComIrqReg);
        i--;
    }
    while ((i!=0) && !(n&0x01) && !(n&waitFor));		
    ClearBitMask(BitFramingReg,0x80);	
    if (i!=0)
    {    
         if(!(ReadRawRC(ErrorReg)&0x1B))
         {
             status = MI_OK;
             if (n & irqEn & 0x01)
             {   
							 status = MI_NOTAGERR;   
				     }
             if (Command == PCD_TRANSCEIVE)
             {
               	n = ReadRawRC(FIFOLevelReg);
              	lastBits = ReadRawRC(ControlReg) & 0x07;		
                if (lastBits)
                {   *pOutLenBit = (n-1)*8 + lastBits;   }
                else
                {   *pOutLenBit = n*8;   }
                if (n == 0)
                {   n = 1;    }
                if (n > MAXRLEN)
                {   n = MAXRLEN;   }
                for (i=0; i<n; i++)
                {   pOutData[i] = ReadRawRC(FIFODataReg);    }
            }
         }
         else
         {   status = MI_EXE_ERR;   }
   } 
   else
   {
   	//LED_R_On();
   }
   SetBitMask(ControlReg,0x80);           // stop timer now
   WriteRawRC(CommandReg,PCD_IDLE);    
   
   return status;
}

/*************************************************************************************************/
/*************************************************************************************************/

//功    能：寻卡
//参数说明: req_code[IN]:寻卡方式
//                0x52 = 寻感应区内所有符合14443A标准的卡
//                0x26 = 寻未进入休眠状态的卡
//          pTagType[OUT]：卡片类型代码
//                0x4400 = Mifare_UltraLight
//                0x0400 = Mifare_One(S50)
//                0x0200 = Mifare_One(S70)
//                0x0800 = Mifare_Pro(X)
//                0x4403 = Mifare_DESFire
//返    回: 成功返回MI_OK
char PcdRequest(unsigned char req_code,unsigned char *pTagType)
{
   char status;  
   unsigned int  unLen;
   unsigned char ucComMF522Buf[MAXRLEN]; //#define MAXRLEN	16

   ClearBitMask(Status2Reg,0x08);	//not encrypte  //不加密
	 WriteRawRC(BitFramingReg,0x07);//0x00?//0x07		//不启动数据发送
   SetBitMask(TxControlReg,0x03);
	//TX1、TX2输出信号将传递经发送数据调制的13.56MHz的能量载波信(打开天线)
 
   ucComMF522Buf[0] = req_code;
/*将寻卡命令装填到要发送的数组中 寻卡命令如下:
#define PICC_REQIDL 0x26 //寻天线区内未进入休眠状态
#define PICC_REQALL 0x52 //寻天线区内全部卡
区别:第一条命令是读取完卡后还会再次读取。(除非在某次读取完成后系统进入休眠(Halt))。
	   第二条命令是读取完卡后会等待卡离开开线作用范围，直到再次进入。
*/
   status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,1,ucComMF522Buf,&unLen);
/*通过RC522和ISO14443卡通讯    &unLen=把unLen变量的赋给指针变量unLen
  参数说明：Command[IN]:RC522命令字 PCD_TRANSCEIVE发送并接收数据
            pInData[IN]:通过RC522发送到卡片的数据 ucComMF522Buf数据组
            InLenByte[IN]:发送数据的字节长度 1
            pOutData[OUT]:接收到的卡片返回数据 ucComMF522Buf
            *pOutLenBit[OUT]:返回数据的位长度
*/
	if((status == MI_OK)&&(unLen==0x10)) //MI_OK=0
		//若函数返回MI_OK，并且ulen为0x10（16bit）为两个字节则说明寻卡成功
	  //返回的两字节被装填入CardRevBuf数组。
	 {    
     *pTagType     = ucComMF522Buf[0];
     *(pTagType+1) = ucComMF522Buf[1];//获取卡的类型
		  //实际上这两个数组表示的是所寻到卡的类型，它们与字节的对应关系如
		  //0x4400 = Mifare_UltraLight
			//0x0400 = Mifare_One(S50)
			//0x0200 = Mifare_One(S70)
			//0x0800 = Mifare_Pro(X)
			//0x4403 = Mifare_DESFire
   }
   else
   {   
		 status = MI_EXE_ERR;//寻卡失败 返回MI_EXE_ERR=98=0x62='b'
   }   															
   return status;
}

/*
防冲突操作就是将防冲突命令通过PcdComMF522函数与PICC卡进行交互。
防冲突命令是两个字节，其中第一字节为Mifare_One卡的防冲突命令字PICC_ANTICOLL1 （0x93），第二个字节为0x20。
关于这两个字节在ISO14443中有解释，这里做一下介绍。
*/  
//功    能：防冲撞 
//参数说明: pSnr[OUT]:卡片序列号，4字节
//返    回: 成功返回MI_OK
char PcdAnticoll(unsigned char *pSnr)
{
    char status;
    unsigned char i,snr_check=0;
    unsigned int  unLen;
    unsigned char ucComMF522Buf[MAXRLEN]; 

    ClearBitMask(Status2Reg,0x08);//not encrypte寄存器包含接收器和发送器和数据模式检测器的状态标志
    WriteRawRC(BitFramingReg,0x00);
	  //不启动数据发送，接收的LSB位存放在位0，接收到的第二位放在位1，定义发送的最后一个字节的位数为8  
    ClearBitMask(CollReg,0x80);//clear if collision所有接收的位在冲突后将被清除。
 
	  /*
	  将防冲突命令装填到发送数组
		所以我们选择了SEL为093表明串联级别1，NVB为0x20表明PCD发送字节数为整两个字节。
		该值定义了该PCD将不发送UID CLn的任何部分。
		因此该命令迫使工作场内的所有PICC以其完整的UID CLn表示响应。
	  */
    ucComMF522Buf[0] = PICC_ANTICOLL1;//选择代码SEL（1个字节）。 SEL规定了串联级别CLn。
    ucComMF522Buf[1] = 0x20;//有效位的数目NVB（1个字节）。NVB规定了PCD所发送的CLn的有效位的数目。
     
    status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,2,ucComMF522Buf,&unLen);
    //当我们发送93与0x20后，PICC返回5个字节其中前4个字节是UID，最后一个字节是校验它是4个先前字节的“异或”值。
    
		if (status == MI_OK)
    {
    	 for (i=0; i<4; i++)
         {   
            *(pSnr+i)  = ucComMF522Buf[i];//将卡片的序列号存入数组
            snr_check ^= ucComMF522Buf[i];//将四个UID依次异或得到校验值
         }
         if (snr_check != ucComMF522Buf[i])//校验
         {  
				  status = MI_EXE_ERR;//校验失败 
				 }
     }
    SetBitMask(CollReg,0x80);
    return status;
}
//功    能：选定卡片 
//参数说明: pSnr[IN]:卡片序列号，4字节（传入检测到卡的序列号）
//返    回: 成功返回MI_OK
char PcdSelect(unsigned char *pSnr)
{
    char status;
    unsigned char i;
    unsigned int  unLen;
    unsigned char ucComMF522Buf[MAXRLEN]; 
	 
	  //将要发给卡的数据装填到数组 
	  //0_防冲突命令 1_NVB 2_UID1 3_UID2 4_UID_3 5_UID_4 6_UIDCheck 7_CRC 8_CRC
    ucComMF522Buf[0] = PICC_ANTICOLL1;//放冲突命令
    ucComMF522Buf[1] = 0x70; //NVB
    ucComMF522Buf[6] = 0;
    for (i=0; i<4; i++)
    {
    	ucComMF522Buf[i+2] = *(pSnr+i);//防重装获得卡识别号
    	ucComMF522Buf[6]  ^= *(pSnr+i);
    }
    CalulateCRC(ucComMF522Buf,7,&ucComMF522Buf[7]);//计算CRC装填至数组

    ClearBitMask(Status2Reg,0x08);//寄存器包含接收器和发送器和数据模式检测器的状态标志

    status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,9,ucComMF522Buf,&unLen);
    
		//?????????????返回卡的容量????????????????????????
    if((status==MI_OK)&&(unLen==0x18))//成功，返回SAK，包括1字节的SAK和2字节的CRC_A
    {  
 		  status=MI_OK;
		}
    else
    {   
		  status=MI_EXE_ERR; 
		}
    return status;
}
//功    能：验证卡片密码
//参数说明: auth_mode[IN]: 密码验证模式
//          0x60=验证A密钥
//          0x61=验证B密钥 
//          addr[IN]：块地址
//          pKey[IN]：密码
//          pSnr[IN]：卡片序列号，4字节
//返    回: 成功返回MI_OK               
char PcdAuthState(unsigned char auth_mode,
	                unsigned char addr,
									unsigned char *pKey,
									unsigned char *pSnr )
{
    char status;
    unsigned int  unLen;//要发送的长度
    unsigned char ucComMF522Buf[MAXRLEN];//发送数据缓存数组

	  ucComMF522Buf[0] = auth_mode;//将密码验证模式
    ucComMF522Buf[1] = addr;//块地址

    trans(ucComMF522Buf+2,pKey,6);//将6byte密钥装填到要发送的数组中
    trans(ucComMF522Buf+8,pSnr,4);//将卡的序列号装填到要发送的数组中
    
    status = PcdComMF522(PCD_AUTHENT,ucComMF522Buf,12,ucComMF522Buf,&unLen);//与卡进行交互
	
    if((status != MI_OK)||(!(ReadRawRC(Status2Reg)&0x08)))
    {  
     //ReadRawRC的该位用来指示MIFARE Cypto1单元接通和因此所有卡的数据通信被加密的情况。
     //只有成功执行 MFAuthent 命令后，该位才能置位。
     //该位只在 MIFARE标准卡的读写器模式中有效。	
		  status = MI_EXE_ERR; 
		}
    return status;
}
//功    能：读取M1卡一块数据
//参数说明: addr[IN]：块地址
//          pData[OUT]：读出的数据，16字节
//返    回: 成功返回MI_OK 
char PcdRead(unsigned char addr,unsigned char *pData)
{
    char status;
    unsigned int  unLen;
    unsigned char ucComMF522Buf[MAXRLEN]; 

    ucComMF522Buf[0] = PICC_READ;//读块
    ucComMF522Buf[1] = addr;//读的块地址
	
    CalulateCRC(ucComMF522Buf,2,&ucComMF522Buf[2]);//获得CRC
    status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,4,ucComMF522Buf,&unLen);
	
    if((status==MI_OK)&&(unLen==0x90))
    {
    	trans(pData,ucComMF522Buf,16);
    }
    else
    {   
			status=MI_EXE_ERR;   
    }
    return status;
}

//功    能：写M1卡一块数据
//参数说明: addr[IN]：块地址
//          pData[IN]：写入的数据，16字节
//返    回: 成功返回MI_OK 
char PcdWrite(unsigned char addr,unsigned char *pData)
{
    char  status;
    unsigned int  unLen;
    unsigned char ucComMF522Buf[MAXRLEN]; //MAXRLEN=16 
    ucComMF522Buf[0] = PICC_WRITE;
    ucComMF522Buf[1] = addr;
	
    CalulateCRC(ucComMF522Buf,2,&ucComMF522Buf[2]);//用MF522计算CRC16函数
	
    status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,4,ucComMF522Buf,&unLen);
	  //命令字PCD_TRANSCEIVE=0x0C发送并接收 
    if((status!= MI_OK)||(unLen!=4)||((ucComMF522Buf[0]&0x0F)!=0x0A))
    {
		  status=MI_EXE_ERR; 
		}
    if(status==MI_OK)
    {
        trans(ucComMF522Buf, pData, 16);//将要写入的数据装入发送数组
			
        CalulateCRC(ucComMF522Buf,16,&ucComMF522Buf[16]);
        status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,18,ucComMF522Buf,&unLen);	
        if ((status != MI_OK) || (unLen != 4) || ((ucComMF522Buf[0] & 0x0F) != 0x0A))
        {   status = MI_EXE_ERR;   } 
	}
    return status;
}

//功    能：M1扣款和充值。
//参数说明:	
//					dd_mode[IN] :模式 0xc0:减,0xc1:加	    
//          addr[IN]：数值地址地址
//          pData[IN]：增加或减少的值，四个字节
//返    回: 成功返回MI_OK 
char PcdValue(unsigned char dd_mode,unsigned char addr,unsigned char *pValue)
{
    char  status;
    unsigned int   unLen;
    unsigned char   ucComMF522Buf[MAXRLEN]; //MAXRLEN=16

    ucComMF522Buf[0]=dd_mode;
    ucComMF522Buf[1]=addr;
    CalulateCRC(ucComMF522Buf,2,&ucComMF522Buf[2]);
	  
    status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,4,ucComMF522Buf,&unLen);

    if ((status != MI_OK)||(unLen!=4)||((ucComMF522Buf[0] & 0x0F) != 0x0A))
    {   
		  status = MI_EXE_ERR;  
		}
    if (status == MI_OK)
    {
        trans(ucComMF522Buf, pValue, 4);
        CalulateCRC(ucComMF522Buf,4,&ucComMF522Buf[4]);//CRC校验
			
        unLen = 0;
        status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,6,ucComMF522Buf,&unLen);
        if (status != MI_EXE_ERR)
        {    
				  status = MI_OK;  
				}
    }
    if (status == MI_OK)
    {
        ucComMF522Buf[0] = PICC_TRANSFER;
        ucComMF522Buf[1] = addr;
        CalulateCRC(ucComMF522Buf,2,&ucComMF522Buf[2]); 
				
        status=PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,4,ucComMF522Buf,&unLen);
        if ((status != MI_OK) || (unLen != 4) || ((ucComMF522Buf[0] & 0x0F) != 0x0A))
        {   
				  status = MI_EXE_ERR;  
				}
    }
    return status;
}

//功    能：M1卡备份钱包
//参数说明: sourceaddr[IN]：备份地址
//          goaladdr[IN]：目标地址
//返    回: 成功返回MI_OK 
char PcdBakValue(unsigned char sourceaddr,unsigned char goaladdr)
{
    char status;
    unsigned int  unLen;
    unsigned char ucComMF522Buf[MAXRLEN]; 

    ucComMF522Buf[0] = PICC_RESTORE;
	  //PICC_RESTORE(恢复)   0xC2     //调块数据到缓冲区
    ucComMF522Buf[1] = sourceaddr;
    CalulateCRC(ucComMF522Buf,2,&ucComMF522Buf[2]);
 
    status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,4,ucComMF522Buf,&unLen);
    if((status != MI_OK)||(unLen != 4)||((ucComMF522Buf[0]&0x0F)!= 0x0A))
    {   
		  status = MI_EXE_ERR;   
		}
		
    if (status == MI_OK)
    {
        ucComMF522Buf[0] = 0;
        ucComMF522Buf[1] = 0;
        ucComMF522Buf[2] = 0;
        ucComMF522Buf[3] = 0;
        CalulateCRC(ucComMF522Buf,4,&ucComMF522Buf[4]);
 
        status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,6,ucComMF522Buf,&unLen);
        if (status != MI_EXE_ERR)
        {    
				  status = MI_OK;    
				}
    }
    
    if (status != MI_OK)
    {    
		  return MI_EXE_ERR;   
		}
    ucComMF522Buf[0] = PICC_TRANSFER;
		//PICC_TRANSFER(转移)    0xB0 //保存缓冲区中数据
    ucComMF522Buf[1] = goaladdr;

    CalulateCRC(ucComMF522Buf,2,&ucComMF522Buf[2]);
    status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,4,ucComMF522Buf,&unLen);

    if ((status != MI_OK) || (unLen != 4) || ((ucComMF522Buf[0] & 0x0F) != 0x0A))
    {   status = MI_EXE_ERR;   }

    return status;
}
void DataDecrypt(unsigned char  *Source,unsigned char  Len)//解密
{
    unsigned char  i;
    
    for(i=0;i<Len;i++)
    {  
        Source[i] ^= Key2;
    }    
    MoveBits(Source,Len,5);    
    for(i=0;i<9;i++)
    {  
        Source[i] ^= Key1;
    }    
}

//==============================================================================
//
//==============================================================================
void MoveBits(unsigned char  *Source,unsigned char  Len,unsigned char  BitCnt)//移动
{
    unsigned char  i,j,MSB;

    MSB = Source[Len];
    for(i=Len;i>0;i--)
    { Source[i] = Source[i-1]; }	

    for(i=0;i<BitCnt;i++)
    {
        Source[0] = Source[Len];
        for(j=Len;j>0;j--)
        {
            Source[j] >>= 1;

            if(Source[j-1]&0x01) Source[j] |= 0x80;                
            else Source[j] &= 0x7f;                
        }
    }	
    //for(i=0;i<Len;i++)
    //	Source[i]=Source[i+1];
    trans(Source,Source+1,Len);	
    Source[Len] = MSB;
}


