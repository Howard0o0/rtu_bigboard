//////////////////////////////////////////////////////
//     文件名: uart0.h
//   文件版本: 1.0.0
//   创建时间: 09年 11月30日
//   更新内容:  
//       作者: 林智
//       附注: 无
//
//////////////////////////////////////////////////////

#pragma once 


//#define NULL   (void *)0
//缓冲区行数
#define UART0_MAXIndex 3
//每行长度
#define UART0_MAXBUFFLEN 215
 
#define UART0_GSM_TYPE    1
#define UART0_DTU_TYPE    2
#define UART0_GTM900_TYPE 3

void UART0_Open(int _type); 
void UART0_Open_9600(int _type);
void Wifi_Uart0_Open();//给wifi调试用
void UART0_Close();
void UART0_ClearBuffer();
int  UART0_Send(char * data ,int len, int CR);
int GPRS_RecvLineTry(char *dest, int maxNum, int * pNum);
int  UART0_RecvLine(char *dest ,int maxNum, int * pNum);
int  UART0_RecvLineTry(char *dest,const int maxNum, int * pNum);
int  UART0_RecvLineWait(char *dest ,const int maxNum, int * pNum);
int  UART0_RecvLineLongWait(char *dest,int maxNum, int * pNum); 
int  UART0_WaitInput();
int  UART0_ClearInput();
int New_UART0_RecvLineWait(); 


extern char UART0_Rx_Buffer[UART0_MAXIndex][UART0_MAXBUFFLEN];