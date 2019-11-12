//////////////////////////////////////////////////////
//     文件名: Console.h
//   文件版本: 1.0.0 
//   创建时间: 09年 11月30日
//   更新内容: 无。 
//       作者: 林智
//       附注: 
// 
//////////////////////////////////////////////////////
#ifndef _CONSOLE_H_
#define _CONSOLE_H_
   
//每行长度
#define MAXBUFFLEN 130 

#define Console_WriteErrorStringln(str) \
    Console_WriteErrorStringlnFuncLine(str,__FUNCTION__, __LINE__)

int Console_Open();  

int Console_Close(); 
 
int Console_WriteString(char * string);
int Console_WriteStringln(char * string);

int Console_WriteBytes(char * bytes, int len);
int Console_WriteBytesln(char * Bytes, int len); 

int Console_WriteHexCharln(char * _str,int len);

int Console_WriteErrorStringlnFuncLine(char* str, char const* _funcname,int _linename);

int Console_WriteInt(int val);

#endif 

