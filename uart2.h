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
#define UART2_CONSOLE_TYPE 1
#define UART2_U485_TYPE    2


//#define NULL (void *)0
//缓冲区行数
#define UART2_MAXIndex 2
//每行长度
#define UART2_MAXBUFFLEN 400


int  UART2_SendtoInt(int num);
int  UART2_Open(int type);
void UART2_Open_9600(int _type);
void UART2_Close();
void UART2_ClearBuffer();
int  UART2_Send(char * data ,int len, int CR); 
int  UART2_RecvLine(char *dest ,int maxNum, int * pNum); 
int  UART2_RecvLineTry(char *dest,const int maxNum, int * pNum); 
int  UART2_RecvLineWait(char *dest ,const int maxNum, int * pNum);
int  UART2_RecvLineLongWait(char *dest,int maxNum, int * pNum);

extern char UART2_Rx_Buffer[UART2_MAXIndex][UART2_MAXBUFFLEN];