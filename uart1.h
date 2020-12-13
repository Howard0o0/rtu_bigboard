//////////////////////////////////////////////////////
//     文件名: uart3.h
//   文件版本: 1.0.0
//   创建时间: 09年11月30日
//   更新内容:  
//       作者: 林智
//       附注: 无
//
//////////////////////////////////////////////////////

#pragma once 


//串口1 主要为3方面使用.这3方面是互相排斥的.PC串口通信,DTU串口通信,485串口通信
//   
extern int s_uart1_type; 
#define UART1_CONSOLE_TYPE 1
#define UART1_U485_TYPE    2
#define UART1_BT_TYPE      1

//#define NULL (void *)0
//缓冲区行数
#define UART1_MAXIndex 2
//每行长度
#define UART1_MAXBUFFLEN 400


int  UART1_SendtoInt(int num);
int  UART1_Open(int type);
void UART1_Open_9600(int _type);
void UART1_Close();
void UART1_ClearBuffer();
int  UART1_Send(char * data ,int len, int CR); 
int  UART1_RecvLine(char *dest ,int maxNum, int * pNum); 
int  UART1_RecvLineTry(char *dest,const int maxNum, int * pNum); 
int  UART1_RecvLineWait(char *dest ,const int maxNum, int * pNum);
int  UART1_RecvLineLongWait(char *dest,int maxNum, int * pNum);

extern char UART1_Rx_Buffer[UART1_MAXIndex][UART1_MAXBUFFLEN];