//////////////////////////////////////////////////////
//     �ļ���: Console.h
//   �ļ��汾: 1.0.0 
//   ����ʱ��: 09�� 11��30��
//   ��������: �ޡ� 
//       ����: ����
//       ��ע: 
// 
//////////////////////////////////////////////////////
#ifndef _CONSOLE_H_
#define _CONSOLE_H_
   
//ÿ�г���
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

