//////////////////////////////////////////////////////
//     文件名: common.h
//   文件版本: 1.0.0
//   创建时间: 09年 11月30日
//   更新内容:  
//       作者: 林智
//       附注: 无
//
//////////////////////////////////////////////////////

#pragma once 

//#define __CONSOLE__DEBUG__ 1
extern int trace_open;
extern int IsDebug;
#define XG_SUCCESS                               0
#define XG_FAILED                                1


#define TraceHexMsg(_str, len) \
    TraceHexMsgFuncLine(_str, len,__FUNCTION__,__LINE__)

#define TraceMsg(_str,ln) \
    TraceMsgFuncLine(_str,ln,__FUNCTION__,__LINE__)
    
#define TraceStr(_str, _len, _ln) \
    TraceStrFuncLine(_str,_len,_ln, __FUNCTION__,__LINE__) 

#define TraceInt4(_val,_ln) \
    TraceInt4FuncLine(_val, _ln, __FUNCTION__,__LINE__)



void TraceOpen();
void TracePulseValue(char * str,int _ln);

int hex_2_ascii(char *data, char *buffer, int len);

void System_Delayms(unsigned int nValue);//  延时函数 
void System_Delayus( unsigned int nValue);
void System_Reset();//  重启函数

void DownInt();  //关中断
void UpInt();    //开中断
void DsP2Int(int i);//0~7
void EnP2Int(int i);
void DsInt();
void EnInt();
 
void IntTo0xInt(unsigned int num,int count);
char ConvertHexChar(char ch);
char ConvertAscIItoHex(char* ascii,char* hex,int asciilen);

//  时钟设置函数 
void Clock_Init(); 
void Clock_Use8MHZ(); 
void Clock_ACLK_DIV(int i);//ACLK分频函数 
void Set_Vcore(unsigned int level);

//232电平转换电路
void Max3222_Open();
void Max3222_Close();

void Select_Debug_Mode(int type);
   
//  字符串辅助函数 
int  Utility_Strlen(char *); 
void Utility_Strncpy(char * _dest,char * _src, int _len); //不负责字符串结尾
int  Utility_Strncmp(const char * _src,const  char * _dest ,int _len);  //不负责字符串结尾 
int  Utility_CheckDigital(const char * _str,int _start ,int _end);
int  Utility_CheckAlphabet(const char * _str,int _start ,int _end);
int  Utility_CheckHexChar(const char * _str,int _start ,int _end);
int  Utility_CheckBinary(const char * _str,int _start,int _end); 
int  Utility_BytesCompare3(const char * _bytes1,const char * _bytes2);
int  Utility_CharToHex(char _src, char * _dest); 
int  Utility_Bytes3ToDecStr7(char * _src ,char * _dest);
int  Utility_DecStr7ToBytes3(char * _src, char * _dest); 
int  Utility_UintToStr4(unsigned int _src, char *_dest);
int  Utility_UintToStr3(unsigned int _src, char *_dest);
int  Utility_UintToStr2(unsigned int _src, char *_dest);
int  Utility_UintToStr1(unsigned int _src, char *_dest);
int Utility_atoi(char *str, int len);
int AsciiStr_to_ASCII(char *input_buffer, int input_buffer_len, char *output_buffer);  //将字符串中每两个字符拼成一个字符的ASCII形式，最终转成新的长度为原来的一半的字符串
int ASCII_to_AsciiStr(char *input_buffer, int input_buffer_len, char *output_buffer);
char Char_to_Hex(char input_chr);

//   时间辅助函数
//   保证要传递一个正确的时间值
//   年月日时分 char[5] 数值 
void Utility_Time_AddSecond(char *dest, int second);// second不可以大于60秒
void Utility_Time_AddMinute(char *dest, int minute);//minute不可以大于60分钟 
void Utility_Time_AddHour(char *dest, int hour);//hour不可以大于24小时
void Utility_Time_AddDay(char *dest,int Day);//Day不可以大于28
void Utility_Time_AddMonth(char *dest,int Month);//Month不可以大于12

//    功能: 计算下一次报告时间  
void Utility_CalculateNextReportTimeBytes(char *dest);

//   功能: 计算下一次拍照时间
void Utility_CalculateNextCameraGoTimes(char *dest);

//    功能: 计算下一次保存时间
void Utility_CalculateNextSaveTimeBytes(char *dest);

//    功能: 计算下一次检查时间
void Utility_CalculateNextCheckTimeBytes(char *dest);

//   判断一个时间按当前模式 是否是检查时间 
int  Utility_Is_A_CheckTime(char * _time);
//   判断一个时间按当前模式 是否是保存时间
int  Utility_Is_A_SaveTime(char * _time); 
//   判断一个时间按当前模式 是否是报告时间
int  Utility_Is_A_ReportTime(char *  _time);

int  Utility_PackRuestTimeMsg(char *data);
int  Utility_PackOKMsg(const char *typeStr,char * _buffer);
int  Utility_PackFailMsg(const char *typeStr,char * _buffer);
int  Utility_PackRejectMsg(const char * typeStr,char * _buffer);
int  Utility_PackErrorMsg(const char * typeStr,char * buffer); 
int  Utility_PackBadMsg(const char * _typeStr,char * _buffer);

int Utility_CheckIp(const char* _str,int _start,int _end);


void TraceFunctionLine(char const *_funcname,int _linename);

void TraceHexMsgFuncLine(char* _str,int len,char const*_funcname, int _linename);

void TraceMsgFuncLine(char * _str,int _ln,char const* _funcname,int _linename);

void TraceStrFuncLine(char * _str,int _len,int _ln,char const* _funcname,int _linename);

void TraceInt4FuncLine(int _val, int _ln,char const* _funcname,int _linename);


int Utility_GetGprsServerIp(char* _str,int* len);
int Utility_GetGprsServerPort(char* _str,int* len);
int Utility_GetGprsServerApn(char* _str,int * len);


void Clear_ExternWatchdog();

