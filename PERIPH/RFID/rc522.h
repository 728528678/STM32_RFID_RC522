#ifndef __RC522_H
#define __RC522_H
#include "stm32f10x.h"



#define MI_EXE_ERR			98

#define MI_OK           0

#define MI_NOTAGERR     1



#define DEF_FIFO_LENGTH 64   //MF522 FIFO长度定义

#define Key1 0x5c
#define Key2 0xb6
#define MAXRLEN	16

//M1卡的类型
#define  MIFARE_1K_4BYTE_UID       0x01
#define  MIFARE_1K_7BYTE_UID       0x02
#define  MIFARE_ULTRALIGHT          0x03
#define  MIFARE_4K_4BYTE_UID      0x04
#define  MIFARE_4K_7BYTE_UID     0x05
#define  MIFARE_PRO       0x07
#define  MIFARE_DESFIRE_7BYTE_UID  0x06
#define  OTHER      0x0a

/////////////////////////////////////////////////////////////////////
//MF522命令字
/////////////////////////////////////////////////////////////////////
#define PCD_IDLE              0x00               //取消当前命令
#define PCD_AUTHENT           0x0E               //验证密钥
#define PCD_RECEIVE           0x08               //接收数据
#define PCD_TRANSMIT          0x04               //发送数据
#define PCD_TRANSCEIVE        0x0C               //发送并接收数据
#define PCD_RESETPHASE        0x0F               //复位
#define PCD_CALCCRC           0x03               //CRC计算

/////////////////////////////////////////////////////////////////////
//Mifare_One卡片命令字 (M1卡)
/////////////////////////////////////////////////////////////////////
#define PICC_REQIDL           0x26               //寻天线区内未进入休眠状态
#define PICC_REQALL           0x52               //寻天线区内全部卡
#define PICC_ANTICOLL1        0x93               //防冲撞
#define PICC_ANTICOLL2        0x95               //防冲撞
#define PICC_AUTHENT1A        0x60               //验证A密钥
#define PICC_AUTHENT1B        0x61               //验证B密钥
#define PICC_READ             0x30               //读块
#define PICC_WRITE            0xA0               //写块
#define PICC_DECREMENT        0xC0               //扣款
#define PICC_INCREMENT        0xC1               //充值
#define PICC_RESTORE          0xC2               //调块数据到缓冲区
#define PICC_TRANSFER         0xB0               //保存缓冲区中数据
#define PICC_HALT             0x50               //休眠

//MF522寄存器定义
// PAGE 0
#define     RFU00                 0x00    
#define     CommandReg            0x01//启动和停止命令执行  
#define     ComIEnReg             0x02//启用和禁用中断请求控制位
#define     DivlEnReg             0x03    
#define     ComIrqReg             0x04//中断请求位
#define     DivIrqReg             0x05//包含中断请求标志
#define     ErrorReg              0x06    
#define     Status1Reg            0x07    
#define     Status2Reg            0x08//接收机和发射机状态位 
#define     FIFODataReg           0x09//64字节FIFO缓冲器的输入输出
#define     FIFOLevelReg          0x0A//存储在FIFO缓冲器中的字节数
#define     WaterLevelReg         0x0B
#define     ControlReg            0x0C
#define     BitFramingReg         0x0D//面向位的帧调整
#define     CollReg               0x0E//定义在RF接口上检测到的第一位冲突。
#define     RFU0F                 0x0F
// PAGE 1     
#define     RFU10                 0x10
#define     ModeReg               0x11//定义发送和接收的常用模式
#define     TxModeReg             0x12
#define     RxModeReg             0x13
#define     TxControlReg          0x14//控制天线驱动器引脚TX1和TX2的逻辑行为
#define     TxAskReg              0x15
#define     TxSelReg              0x16
#define     RxSelReg              0x17//选择内部的接收器设置
#define     RxThresholdReg        0x18
#define     DemodReg              0x19
#define     RFU1A                 0x1A
#define     RFU1B                 0x1B
#define     MifareReg             0x1C
#define     RFU1D                 0x1D
#define     RFU1E                 0x1E
#define     SerialSpeedReg        0x1F
// PAGE 2    
#define     RFU20                 0x20  
#define     CRCResultRegM         0x21//显示CRC计算的实际MSB和LSB值。
#define     CRCResultRegL         0x22
#define     RFU23                 0x23
#define     ModWidthReg           0x24
#define     RFU25                 0x25
#define     RFCfgReg              0x26//配置接收器增益
#define     GsNReg                0x27
#define     CWGsCfgReg            0x28
#define     ModGsCfgReg           0x29
#define     TModeReg              0x2A//定义内部定时器的设置
#define     TPrescalerReg         0x2B//
#define     TReloadRegH           0x2C//描述 16 位长的定时器重装值
#define     TReloadRegL           0x2D
#define     TCounterValueRegH     0x2E//显示 16 位长的实际定时器值
#define     TCounterValueRegL     0x2F
// PAGE 3      
#define     RFU30                 0x30
#define     TestSel1Reg           0x31
#define     TestSel2Reg           0x32
#define     TestPinEnReg          0x33
#define     TestPinValueReg       0x34
#define     TestBusReg            0x35
#define     AutoTestReg           0x36
#define     VersionReg            0x37
#define     AnalogTestReg         0x38
#define     TestDAC1Reg           0x39  
#define     TestDAC2Reg           0x3A   
#define     TestADCReg            0x3B   
#define     RFU3C                 0x3C   
#define     RFU3D                 0x3D   
#define     RFU3E                 0x3E   
#define     RFU3F		  0x3F


void spi2_init(void);//SPI初始化函数
u8 spi2_readwritebyte(unsigned char txdata);//SPI接收并发送一个函数
void spi_setspeed(unsigned char  SPI_BaudRatePrescaler);//设置波特率
void trans(unsigned char* table,unsigned char* table1,unsigned char num);//数组数组数据复制数据

void RFIDGPIO_Init(void);//RFID_RC522 GPIO SPI初始化
char PcdReset(void);//复位RC522
void PcdAntennaOn(void);//开启天线  
void PcdAntennaOff(void);//关闭天线
char M500PcdConfigISOType(unsigned char type);//设置RC632的工作方式
char PcdHalt(void);//命令卡片进入休眠状态

void CalulateCRC(unsigned char *pIndata,unsigned char len,unsigned char *pOutData);//用MF522计算CRC16函数
unsigned char ReadRawRC(unsigned char Address);//读RC523寄存器
void WriteRawRC(unsigned char Address, unsigned char value);//写RC523寄存器
void SetBitMask(unsigned char reg,unsigned char mask);//置RC522寄存器位						 
void ClearBitMask(unsigned char reg,unsigned char mask);//清RC522寄存器位
char PcdComMF522(unsigned char Command,//通过RC522和ISO14443卡通讯
                 unsigned char *pInData, 
                 unsigned char InLenByte,
                 unsigned char *pOutData, 
                 unsigned int  *pOutLenBit);
								 						 
char PcdRequest(unsigned char req_code,unsigned char *pTagType);//寻卡
char PcdAnticoll(unsigned char *pSnr);//防冲撞
char PcdSelect(unsigned char *pSnr);//选定卡片
char PcdAuthState(unsigned char auth_mode,unsigned char addr,unsigned char *pKey,unsigned char *pSnr);//验证卡片密码   
char PcdRead(unsigned char addr,unsigned char *pData);//读取M1卡一块数据   
char PcdWrite(unsigned char addr,unsigned char *pData);//写M1卡一块数据 
char PcdValue(unsigned char dd_mode,unsigned char addr,unsigned char *pValue);//M1扣款和充值
char PcdBakValue(unsigned char sourceaddr, unsigned char goaladdr);//M1卡备份钱包 

void MoveBits(unsigned char  *Source,unsigned char  Len,unsigned char  BitCnt);//移动
void DataDecrypt(unsigned char  *Source,unsigned char  Len);//解密
								 
#endif
