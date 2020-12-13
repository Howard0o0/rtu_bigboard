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
#define UART3_CONSOLE_TYPE 1
#define UART3_U485_TYPE    2


//#define NULL (void *)0
//缓冲区行数
#define UART3_MAXIndex 2
//每行长度
#define UART3_MAXBUFFLEN 400


int  UART3_SendtoInt(int num);
int  UART3_Open(int type);
void UART3_Open_9600(int _type);
void UART3_Close();
void UART3_ClearBuffer();
int  UART3_Send(char * data ,int len, int CR); 
int  UART3_RecvLine(char *dest ,int maxNum, int * pNum); 
int  UART3_RecvLineTry(char *dest,const int maxNum, int * pNum); 
int  UART3_RecvLineWait(char *dest ,const int maxNum, int * pNum);
int  UART3_RecvLineLongWait(char *dest,int maxNum, int * pNum);

extern char UART3_Rx_Buffer[UART3_MAXIndex][UART3_MAXBUFFLEN];