//////////////////////////////////////////////////////
//     文件名: rtc.h
//   文件版本: 1.0.0
//   创建时间: 09年11月30日
//   更新内容:  
//       作者: 林智
//       附注: 无
//
//////////////////////////////////////////////////////

#ifndef __RTC_h
#define __RTC_h

#include "msp430common.h"

 
#define RTC_RST   BIT4   //P44
 
//   RAM 不安全
//   0  1  2  3  4  CheckTime
//   5  6  7  8  9  SaveTime
//  10 11 12 13 14  ReportTime
//        15        StartIdx  
//        16        EndIdx
//    17 18 19      PULSE1_BYTES
//    20 21 22      PULSE2_BYTES
//    23 24 25      PULSE3_BYTES
//    26 27 28      PULSE4_BYTES
//    

#define CHECKTIME_ADDR 0
#define SAVETIME_ADDR 5
#define REPORTTIME_ADDR 10
#define STARTIDX_ADDR 15
#define ENDIDX_ADDR 16

#define PULSE1_BYTES 17
#define PULSE1_BYTE1 17
#define PULSE1_BYTE2 18
#define PULSE1_BYTE3 19

#define PULSE2_BYTES 20
#define PULSE2_BYTE1 20
#define PULSE2_BYTE2 21
#define PULSE2_BYTE3 22

#define PULSE3_BYTES 23
#define PULSE3_BYTE1 23
#define PULSE3_BYTE2 24
#define PULSE3_BYTE3 25

#define PULSE4_BYTES 26
#define PULSE4_BYTE1 26
#define PULSE4_BYTE2 27
#define PULSE4_BYTE3 28

extern char g_rtc_nowTime[5];
extern char s_RTC_CheckTime[5];
extern char s_RTC_SaveTime[5];
extern char s_RTC_ReportTime[5];
extern char s_RTC_StartIdx;
extern char s_RTC_EndIdx;
//数据标签
int RTC_RetrieveIndex();
int RTC_ReadStartIdx(char * _dest); 
int RTC_SetStartIdx(const char _src);
int RTC_ReadEndIdx(char * _dest);
int RTC_SetEndIdx(const char _src);

//检查时间
int RTC_ReadCheckTimeBytes(unsigned char index, char *dest);
int RTC_ReadCheckTimeStr5_A(char * _dest); 
int RTC_SetCheckTimeBytes(unsigned char index, const char * _src);
//保存时间
int RTC_ReadSaveTimeBytes(unsigned char index, char *dest);
int RTC_ReadSaveTimeStr5_A(char * _dest); 
int RTC_SetSaveTimeBytes(unsigned char index, const char * _src);
//报告时间
int RTC_ReadReportTimeBytes(unsigned char index, char *dest);
int RTC_ReadReportTimeStr5_A(char * _dest); 
int RTC_SetReportTimeBytes(unsigned char index, const char * _src); 

//脉冲1
int RTC_ReadPulseBytes(int _index, char * _dest);
int RTC_SetPulseBytes(int _index, char * _src);
int RTC_IncPulseBytes(int _index);

int RTC_RetrievePulseBytes();
//
//  高级函数
// 
////////////////////////////////////
//    功能: 判断时间是否已经到达
//    参数: char *   格式为年月日时分的char[5]缓存
//  返回值: 1 :  为过去时间
//          0 :  为现在时间
//         -1 :  为未来时间  
//    附注: >>  精确到分   
////////////////////////////////////
int  RTC_IsPassed(char * time);
int  RTC_IsBadTime(char * time, int RTCTime);
////////////////////////////////////
//    功能: 读出当前时间串
//    参数: char * 写入缓存
//  返回值: 无
//    附注: >>   写入格式0903062300
////////////////////////////////////
void RTC_ReadTimeStr5_A(char * dest);
void RTC_ReadTimeStr5_B(char * dest);
void RTC_SetTimeStr5_B(const char *src);

void RTC_ReadTimeStr6_A(char * dest);
void RTC_ReadTimeStr6_B(char * dest);

void RTC_SetTimeStr6_A(const char * src);
void RTC_SetTimeStr6_B(const char *src);

 void RTC_SetTimeStr6_C(const char *src);

////////////////////////////////////
//    功能: 读出当前时间值
//    参数: char * 写入缓存,char[5]
//  返回值: 无
//    附注: >> 依次写入年月日时分的数值
//          >>如:[0]=9 [1]=3 [2]=6 [3]=23 [4]=0
////////////////////////////////////
void RTC_ReadTimeBytes5(char *dest); 
void RTC_ReadTimeBytes6(char *dest);  

////////////////////////////////////
//    功能: 设置当前时间
//    参数: char * 时间数值缓存
//  返回值: 0 : 成功
//         -1 : 参数错误
//    附注: >> 依次读取年月日时分的数值
//          >>如:[0]=9 [1]=3 [2]=6 [3]=23 [4]=0
//          >> 代码未实现 !!!
////////////////////////////////////
int RTC_SetTimeBytes5(const char *dest);
int RTC_SetTimeBytes6(const char *dest);

void clearAlarmAFlag();                                                     //清除和设置RTC Alarm标志
int getAlarmAFlag();
int RTC_SetAlarm(const char hour, const char min, const char sec);

////////////////////////////////////
//    功能: 设置小时
//    参数: char  小时数值
//  返回值: 无
//    附注: >>
////////////////////////////////////
//void _RTC_SetHour(const unsigned  char hour);

////////////////////////////////////
//    功能: 设置分钟
//    参数: char  分钟数值
//  返回值: 无
//    附注: >>
////////////////////////////////////
//void _RTC_SetMinute(const unsigned  char minute);

////////////////////////////////////
//    功能: 设置秒钟
//    参数: char  秒钟数值
//  返回值: 无
//    附注: >>
////////////////////////////////////
//void _RTC_SetSecond(const unsigned  char second);

////////////////////////////////////
//    功能: 设置星期
//    参数: char  星期数值
//  返回值: 无
//    附注: >>
////////////////////////////////////
//void _RTC_SetDay(const unsigned char Day);

////////////////////////////////////
//    功能: 设置日期
//    参数: char  日期数值
//  返回值: 无
//    附注: >>
////////////////////////////////////
//void _RTC_SetDate(const  unsigned char Date);

////////////////////////////////////
//    功能: 设置月份
//    参数: char  月份数值
//  返回值: 无
//    附注: >>
////////////////////////////////////
//void _RTC_SetMonth(const unsigned  char month);

////////////////////////////////////
//    功能: 设置年份
//    参数: char  年份数值
//  返回值: 无
//    附注: >>
////////////////////////////////////
//void _RTC_SetYear(const unsigned  char year);

////////////////////////////////////
//    功能: 一次性设置时间
//    参数: char 依次的各个时间寄存器的值
//  返回值: 0 : 成功
//         -1 : 参数错误
//    附注: >>
////////////////////////////////////
int _RTC_SetTime(const char second,const char minute,
                 const char hour,const char date,const char month,
                 const char day,const char year,const char control);

////////////////////////////////////
//    功能: 一次性设置时间
//    参数: char* 依次包含各个时间寄存器值的数组
//  返回值: 0 : 成功
//         -1 : 参数错误
//    附注: >>
////////////////////////////////////
//int _RTC_SetRTCTimeBytes(const char * src);

////////////////////////////////////
//    功能: 读取小时
//    参数: 无
//  返回值: 小时值
//    附注: >>
////////////////////////////////////
//char _RTC_ReadHour();

////////////////////////////////////
//    功能: 读取分钟
//    参数: 无
//  返回值: 分钟值
//    附注: >>
////////////////////////////////////
//char _RTC_ReadMinute();

////////////////////////////////////
//    功能: 读取秒钟
//    参数: 无
//  返回值: 秒钟值
//    附注: >>
////////////////////////////////////
 char _RTC_ReadSecond();

////////////////////////////////////
//    功能: 读取星期
//    参数: 无
//  返回值: 星期值
//    附注: >>
////////////////////////////////////
//char _RTC_ReadDay();

////////////////////////////////////
//    功能: 读取日期
//    参数: 无
//  返回值: 日期值
//    附注: >>
////////////////////////////////////
//char _RTC_ReadDate();


////////////////////////////////////
//    功能: 读取月份
//    参数: 无
//  返回值: 月份值
//    附注: >>
////////////////////////////////////
//char _RTC_ReadMonth();

////////////////////////////////////
//    功能: 读取年份
//    参数: 无
//  返回值: 年份值
//    附注: >>
////////////////////////////////////
//char _RTC_ReadYear();

////////////////////////////////////
//    功能: 一次性读取时间
//    参数: 依次盛装各个寄存器值的缓存
//  返回值: 无
//    附注: >>
////////////////////////////////////
void _RTC_ReadTime(char *second,char *minute,
                 char *hour,char *date,char *month,
                 char *day, char *year,char *control);

////////////////////////////////////
//    功能: 一次性读取时间
//    参数: 依次盛装各个寄存器值的数组缓存
//  返回值: 无
//    附注: >>
////////////////////////////////////
//void  _RTC_ReadRTCTimeBytes(char * dest);


/*
 *  
 */
void _RTC_Init(void);

////////////////////////////////////
//    功能: 操作RTC的底层函数群
//    参数: *****
//  返回值: *****
//    附注: >> 具体见DS1302芯片手册
////////////////////////////////////
//中级函数
int  _RTC_SendByte(char data);
char _RTC_RecvByte(void);
char _RTC_Read_OneByte(char regAddr);
void _RTC_Write_OneByte(char regAddr,char data);
int  _RTC_MultiRead(char cmd, char * dest, int length);
int  _RTC_MultiWrite(char cmd ,const char * src, int length);
void _RTC_DisableWrite();
void _RTC_EnableWrite();
void _RTC_Go();
void _RTC_Pause();
int  _RTC_Check();
// void _RTC_EnableCharge();
// void _RTC_DisableCharge(); 
//RAM
void _RTC_WriteRAM(unsigned char index,const char data);
char _RTC_ReadRAM(unsigned char index); 



#define CPU_CLOCK       8000000
#define Delay_us(us)    __delay_cycles(CPU_CLOCK/1000000*(us))
#define u8 unsigned char

//IO方向设置
#define SDA_IN()  {P9DIR &= ~(1<<1);}	//P9.1输入模式
#define SDA_OUT() {P9DIR |= (1<<1);} //P9.1输出模式

#define RTC_SCLK  BIT2  // p9.2
#define RTC_IO    BIT1  // p9.1

/* cmd */
#define WRITE_CMD          0xD0
#define READ_CMD           0xD1
/* base address in ROM */
#define  RTC_RAM_BASE       (long)524256


/* reg addr */
typedef enum{
  RegAddr_Sec,    // Seconds              00-59
  RegAddr_Min,    // Minutes              00-59
  RegAddr_Hour,   // Hours                1–12 + AM/PM 00–23
  RegAddr_Day,    // Day                  1 - 7
  RegAddr_Date,   // Data                 01-31
  RegAddr_CMon,   // Century and month    Century + 01–12
  RegAddr_Year,   // Year                 00 - 99
  RegAddr_Sec_A1, // Alarm 1 Seconds      00-59
  RegAddr_Min_A1, // Alarm 1 Minutes      00-59
  RegAddr_Hour_A1,// Alarm 1 Hours        1–12 + AM/PM 00–23
  RegAddr_Da_A1,  // Alarm 1 Day/Date     1 - 7 / 1 – 31
  RegAddr_Min_A2, // Alarm 2 Minutes      00-59
  RegAddr_Hour_A2,// Alarm 2 Hours        1–12 + AM/PM 00–23
  RegAddr_Da_A2,  // Alarm 2 Day/Date     1 - 7 / 1 – 31
  RegAddr_Control,// Control
  RegAddr_CtlStat,// Control/Status
  RegAddr_AgOfset,// Aging offset
  RegAddr_TempMSB,// MSB of Temp
  RegAddr_TempLSB,// LSB of Temp
}DS3231REG_ADDR;

 
//辅助函数
char _BCDtoHEX(char bcd);
char _HEXtoBCD(char dec); 
//低级函数
int  RTC_Open(); 
int  RTC_Close();
void _RTC_Set_RST_low();
void _RTC_Set_RST_high();
void _RTC_Set_SCLK_low();
void _RTC_Set_SCLK_high();
void _RTC_Set_IO_low();
void _RTC_Set_IO_high();
void _RTC_Set_IO_OUT();
void _RTC_Set_IO_IN(); 
void _RTC_delay();

//IIC所有操作函数
void IIC_SCL_HIGH(void);
void IIC_SCL_LOW(void);
void IIC_SDA_HIGH(void);
void IIC_SDA_LOW(void);
int READ_SDA(void);
void IIC_Init(void);                //初始化IIC的IO口				 
void IIC_Start(void);				//发送IIC开始信号
void IIC_Stop(void);	  			//发送IIC停止信号
void IIC_Send_Byte(u8 txd);			//IIC发送一个字节
u8 IIC_Read_Byte(unsigned char ack);//IIC读取一个字节
u8 IIC_Wait_Ack(void); 				//IIC等待ACK信号
void IIC_Ack(void);					//IIC发送ACK信号
void IIC_NAck(void);				//IIC不发送ACK信号 

  
#endif