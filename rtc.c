//////////////////////////////////////////////////////
//     文件名: rtc.c
//   文件版本: 1.0.0
//   创建时间: 09年 11月30日
//   更新内容:  
//       作者: 林智
//       附注: 无
//
//////////////////////////////////////////////////////

/*
 * author:  朱厚望
 * time:    2019-08-28
 * changes:  
 *          int  RTC_Open(); 
            char _RTC_Read_OneByte(char regAddr);
            void _RTC_Write_OneByte(char regAddr,char data);
            void _RTC_DisableWrite();
            void _RTC_EnableWrite();
            void _RTC_Go();
            int  _RTC_Check();
            void _RTC_WriteRAM(unsigned char index,const char data);
            char _RTC_ReadRAM(unsigned char index); 
 * abandon:
            void _RTC_DisableWrite();
            void _RTC_EnableWrite();
            void _RTC_EnableCharge();
            void _RTC_DisableCharge(); 
 */
 
#include "msp430common.h"
#include "rtc.h"
#include "common.h"
#include "led.h" 
#include "Store.h"
#include "Sampler.h"
#include "rom.h"



//高级函数

extern char g_pulse1_range[3];
extern char g_pulse2_range[3];
extern char g_pulse3_range[3];
extern char g_pulse4_range[3];

//全局变量  开机的时间
char g_rtc_nowTime[5]={0,0,0,0,0};




//  关键数据的保存策略.内存一份,RTC一份
//  内存初始化值全为一个显然错误的值.
//  1.写数据时,更新内存和RTC
//  2.读数据时,读内存 X,  读RTC值 Y
//    若X,Y皆正确, X!=Y,则以X为准,并修改Y.
//    若X,Y皆错误,报错或设为默认值
//    若X为错误值.Y正确.则以Y为准,并修改X.
//  3.RTC初始化的时候,根据XY关系 更正XY.
//

char s_RTC_lastTime[5]={0x00,0x00,0x00,0x00,0x00};
char s_RTC_CheckTime[5]={0x00,0x00,0x00,0x00,0x00};//初始化为错误值
char s_RTC_SaveTime[5]={0x00,0x00,0x00,0x00,0x00};//初始化为错误值
char s_RTC_ReportTime[5]={0x00,0x00,0x00,0x00,0x00};//初始化为错误值
char s_RTC_StartIdx=240;//初始化为错误值  范围为1~200
char s_RTC_EndIdx=240;//初始化为错误值    范围为1~200

//储存脉冲设备度数(对应 脉冲计数设备 上面的值)
char s_RTC_PulseBytes[4][3]={0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF}; //第一字节为FF被认为是错误
//因为不可能有那么大, 程序最高支持7个'9'的度数

//
//  RTC_RetrieveIndex   :    0表示未发送   1表示已发送  
//
//   索引         第一种情况               第二种情况                第三种情况      第四种情况
//    1              0                         1                          0               1  
//    2              0                         1                          0               1
//    3              0                         1                          0               1
//    .              1  <-  _endIdx            0  <- _startIdx            0               1
//    .              1                         0                          0               1
//    .              1                         0                          0               1
//    .              0  <-  _startIdx          1  <- _endIdx              0               1
//   MAX-1           0                         1                          0               1
//   MAX             0                         1                          0               1
//
//  对于满和空的判断,我们 1比较 _startIdx 和 _endIdx , 2 相等时判断_startIdx为已发送还是未发送
//  


//
//   RTC函数会在中断服务中被调用, 因此要进行 中断关闭 进行保护
// 

//
//   该函数不能很好应对发送数据中隐藏着未发送的数据. 待更好的实现.
//  
int RTC_RetrieveIndex()
{
    DownInt();
    char _idx=DATA_MIN_IDX; 
    int _ret=0;
    _ret=Store_CheckDataItemSended(_idx);//取第一个
    if(_ret==-2)
    {
        UpInt();
        return -1;
    }
    if(_ret==1)
    {//1起头的第2种情况
        while(1)
        {
            _ret=Store_CheckDataItemSended(++ _idx); 
            if(_ret==1)
                continue;
            if( _ret==-2)//出边界了
            {//全1,则设 _startIdx = DATA_MIN_IDX   _endIdx = DATA_MIN_IDX
                RTC_SetStartIdx(DATA_MIN_IDX);
                RTC_SetEndIdx(DATA_MIN_IDX);
                UpInt();
                return 0;
            }
            if(_ret==0)
            {//找到_StartIdx了
                RTC_SetStartIdx(_idx);
                break;
            }
        }
        //从前一个循环跳出来了.我们现在开始寻找_endIdx 找1出来
        while(1)
        {//遇到1了 就是 _endIdx, 
            _ret=Store_CheckDataItemSended(++ _idx);
            if(_ret == 0)
                continue;
            if(_ret==-2)//出边界了
            {//则_EndIdx 就是 DATA_MIN_IDX
                RTC_SetEndIdx(DATA_MIN_IDX);
                UpInt();
                return 0;
            }
            if(_ret==1)
            {
                RTC_SetEndIdx(_idx);
                UpInt();
                return 0;
            }
        }
    }
    else //_ret==0
    {//道理同上
        while(1)
        {
            _ret=Store_CheckDataItemSended(++ _idx); 
            if(_ret==0)
                continue;
            if( _ret==-2)//出边界了
            {//全 0 ,则设 _startIdx = DATA_MIN_IDX   _endIdx = DATA_MIN_IDX
                RTC_SetStartIdx(DATA_MIN_IDX);
                RTC_SetEndIdx(DATA_MIN_IDX);
                UpInt();
                return 0;
            }
            if(_ret==1)
            {//找到_endIdx了
                RTC_SetEndIdx(_idx);
                break;
            }
        }
        while(1)
        {//遇到0则为  _startIdx
            _ret=Store_CheckDataItemSended(++ _idx); 
            if(_ret==1)
                continue; 
            if(_ret==-2)//出边界了
            {
                RTC_SetStartIdx(DATA_MIN_IDX);
                UpInt();
                return 0;
            }
            if(_ret==0)
            {
                RTC_SetStartIdx(_idx);
                UpInt();
                return 0;
            }
        }
    }
}

int RTC_ReadStartIdx(char *dest)
{
    DownInt();
    if(s_RTC_StartIdx<=DATA_MAX_IDX && s_RTC_StartIdx >= DATA_MIN_IDX)
    {//内存值正确
        *dest=s_RTC_StartIdx;
        //写入RTC,保证RTC里的值的正确
        _RTC_WriteRAM(STARTIDX_ADDR,s_RTC_StartIdx);
        UpInt();
        return 0;
    }
    else
    {//内存值错误
        *dest= _RTC_ReadRAM(STARTIDX_ADDR);
        if((*dest) <= DATA_MAX_IDX && (*dest) >= DATA_MIN_IDX )
        {//RTC值可能正确 
            s_RTC_StartIdx=(*dest); //更新内存值
            UpInt();
            return 0;
        }
        else
        {   
            UpInt();
            return -1;
        }
    }
}

int RTC_SetStartIdx(char src)
{
    DownInt();
    s_RTC_StartIdx=src;//更新内存
    _RTC_WriteRAM(STARTIDX_ADDR,src);//更新RTC
    UpInt();
    return 0;
}

int RTC_ReadEndIdx(char *dest)
{
    DownInt();
    if(s_RTC_EndIdx <= DATA_MAX_IDX && s_RTC_EndIdx >= DATA_MIN_IDX )
    {//内存值正确
        *dest=s_RTC_EndIdx;
        //写入RTC,保证RTC里的值的正确
        _RTC_WriteRAM(ENDIDX_ADDR,s_RTC_EndIdx);
        UpInt();
        return 0;
    }
    else
    {//内存值错误
        *dest= _RTC_ReadRAM(ENDIDX_ADDR);
        if((*dest) <= DATA_MAX_IDX && (*dest) >= DATA_MIN_IDX )
        {//RTC值可能正确 
            s_RTC_EndIdx=(*dest); //更新内存值
            UpInt();
            return 0;
        }
        else
        {  
            UpInt();
            return -1;
        }
    } 
}

int RTC_SetEndIdx(const char src)
{
    DownInt();
    s_RTC_EndIdx=src;
    _RTC_WriteRAM(ENDIDX_ADDR,src);
    UpInt();
    return 0;
}
int RTC_ReadCheckTimeBytes(unsigned char index, char *dest, char *lastchecktime)
{
    DownInt();
    if(RTC_IsBadTime(lastchecktime,0)==0)
    {//内存值是正确的.
        dest[0]=lastchecktime[0];
        dest[1]=lastchecktime[1];
        dest[2]=lastchecktime[2];
        dest[3]=lastchecktime[3];
        dest[4]=lastchecktime[4];
        //将这个值写入到RTC中.保证RTC里也是正确的
        _RTC_WriteRAM(index,lastchecktime[0]);
        _RTC_WriteRAM(index+1,lastchecktime[1]);
        _RTC_WriteRAM(index+2,lastchecktime[2]);
        _RTC_WriteRAM(index+3,lastchecktime[3]);
        _RTC_WriteRAM(index+4,lastchecktime[4]);
        UpInt();
        return 0;
    }
    else
    {//内存值是错误的.
        dest[0]= _RTC_ReadRAM(index);
        dest[1]=_RTC_ReadRAM(index+1);
        dest[2]=_RTC_ReadRAM(index+2);
        dest[3]=_RTC_ReadRAM(index+3);
        dest[4]=_RTC_ReadRAM(index+4);
        if(RTC_IsBadTime(dest,0)==0)
        {//RTC是正确的. 
            lastchecktime[0]=dest[0];
            lastchecktime[1]=dest[1];
            lastchecktime[2]=dest[2];
            lastchecktime[3]=dest[3];
            lastchecktime[4]=dest[4];
            UpInt();
            return 0;
        }
        else
        {//都是错误的,就返回-1  
            UpInt();
            return -1;          
        }
    } 
}

int RTC_ReadCheckTimeStr5_A(char * _dest)
{
    DownInt();
    char _temp[5];
    if(RTC_ReadCheckTimeBytes(CHECKTIME_ADDR,_temp,s_RTC_CheckTime))
    {
        UpInt();
        return -1;
    }
    UpInt();
    _dest[0]=_temp[0]/10+'0';
    _dest[1]=_temp[0]%10+'0';
    _dest[2]='/';
    _dest[3]=_temp[1]/10+'0';
    _dest[4]=_temp[1]%10+'0';
    _dest[5]='/';
    _dest[6]=_temp[2]/10+'0';
    _dest[7]=_temp[2]%10+'0';
    _dest[8]='/';
    _dest[9]=_temp[3]/10+'0';
    _dest[10]=_temp[3]%10+'0';
    _dest[11]=':';
    _dest[12]=_temp[4]/10+'0';
    _dest[13]=_temp[4]%10+'0';
    
    return 0;
}

int RTC_SetCheckTimeBytes(unsigned char index, const char *src)
{
    
    s_RTC_CheckTime[0]=src[0];
    s_RTC_CheckTime[1]=src[1];
    s_RTC_CheckTime[2]=src[2];
    s_RTC_CheckTime[3]=src[3];
    s_RTC_CheckTime[4]=src[4];
    DownInt();
    _RTC_WriteRAM(index,src[0]);
    _RTC_WriteRAM(index+1,src[1]);
    _RTC_WriteRAM(index+2,src[2]);
    _RTC_WriteRAM(index+3,src[3]);
    _RTC_WriteRAM(index+4,src[4]);
    UpInt();
    return 0;
}

int RTC_ReadSaveTimeBytes(unsigned char index, char *dest, char *lastsavetime)
{
    DownInt();
    if(RTC_IsBadTime(lastsavetime,0)==0)
    {//内存值是正确的. 
        dest[0]=lastsavetime[0];
        dest[1]=lastsavetime[1];
        dest[2]=lastsavetime[2];
        dest[3]=lastsavetime[3];
        dest[4]=lastsavetime[4];
        
        //将这个值写入到RTC中.保证RTC里也是正确的
        _RTC_WriteRAM(index,lastsavetime[0]);
        _RTC_WriteRAM(index+1,lastsavetime[1]);
        _RTC_WriteRAM(index+2,lastsavetime[2]);
        _RTC_WriteRAM(index+3,lastsavetime[3]);
        _RTC_WriteRAM(index+4,lastsavetime[4]);
        UpInt();
        return 0;
    }
    else
    {//内存值是错误的.
        dest[0]= _RTC_ReadRAM(index);
        dest[1]=_RTC_ReadRAM(index+1);
        dest[2]=_RTC_ReadRAM(index+2);
        dest[3]=_RTC_ReadRAM(index+3);
        dest[4]=_RTC_ReadRAM(index+4);
        if(RTC_IsBadTime(dest,0)==0)
        {//RTC是正确的. 
            lastsavetime[0]=dest[0];
            lastsavetime[1]=dest[1];
            lastsavetime[2]=dest[2];
            lastsavetime[3]=dest[3];
            lastsavetime[4]=dest[4];
            UpInt();
            return 0;
        }
        else
        {//都是错误的,就返回-1  
            UpInt();
            return -1;          
        }
    }
}
int RTC_ReadSaveTimeStr5_A(char *dest)
{
    DownInt();
    char temp[5];
    if(RTC_ReadSaveTimeBytes(SAVETIME_ADDR,temp,s_RTC_SaveTime))
    {
        UpInt();
        return -1;
    }
    UpInt();
    dest[0]=temp[0]/10+'0';
    dest[1]=temp[0]%10+'0';
    dest[2]='/';
    dest[3]=temp[1]/10+'0';
    dest[4]=temp[1]%10+'0';
    dest[5]='/';
    dest[6]=temp[2]/10+'0';
    dest[7]=temp[2]%10+'0';
    dest[8]='/';
    dest[9]=temp[3]/10+'0';
    dest[10]=temp[3]%10+'0';
    dest[11]=':';
    dest[12]=temp[4]/10+'0';
    dest[13]=temp[4]%10+'0';
    
    return 0;
}
int RTC_SetSaveTimeBytes(unsigned char index, const char *src)
{
    
    s_RTC_SaveTime[0]=src[0];
    s_RTC_SaveTime[1]=src[1];
    s_RTC_SaveTime[2]=src[2];
    s_RTC_SaveTime[3]=src[3];
    s_RTC_SaveTime[4]=src[4];
    DownInt();
    _RTC_WriteRAM(index,src[0]);
    _RTC_WriteRAM(index+1,src[1]);
    _RTC_WriteRAM(index+2,src[2]);
    _RTC_WriteRAM(index+3,src[3]);
    _RTC_WriteRAM(index+4,src[4]);
    UpInt();
    return 0;
}

int RTC_ReadReportTimeBytes(unsigned char index, char *dest, char *lastreporttime)
{
    DownInt();
    if(RTC_IsBadTime(lastreporttime,0)==0)
    {//内存值是正确的. 
        dest[0]=lastreporttime[0];
        dest[1]=lastreporttime[1];
        dest[2]=lastreporttime[2];
        dest[3]=lastreporttime[3];
        dest[4]=lastreporttime[4];
        
        //将这个值写入到RTC中.保证RTC里也是正确的
        _RTC_WriteRAM(index,   lastreporttime[0]);
        _RTC_WriteRAM(index+1, lastreporttime[1]);
        _RTC_WriteRAM(index+2, lastreporttime[2]);
        _RTC_WriteRAM(index+3, lastreporttime[3]);
        _RTC_WriteRAM(index+4, lastreporttime[4]);
        UpInt();
        return 0;
    }
    else
    {//内存值是错误的.
        dest[0]=_RTC_ReadRAM(index);
        dest[1]=_RTC_ReadRAM(index+1);
        dest[2]=_RTC_ReadRAM(index+2);
        dest[3]=_RTC_ReadRAM(index+3);
        dest[4]=_RTC_ReadRAM(index+4);
        if(RTC_IsBadTime(dest,0)==0)
        {//RTC是正确的. 
            lastreporttime[0]=dest[0];
            lastreporttime[1]=dest[1];
            lastreporttime[2]=dest[2];
            lastreporttime[3]=dest[3];
            lastreporttime[4]=dest[4];
            UpInt();
            return 0;
        }
        else
        {//都是错误的,就返回-1  
            UpInt();
            return -1;          
        }
    } 
}


int RTC_ReadReportTimeStr5_A(char *dest)
{
    DownInt();
    char temp[5];
    if(RTC_ReadReportTimeBytes(REPORTTIME_ADDR,temp,s_RTC_ReportTime))
    {
        UpInt();
        return -1;
    }
    UpInt();
    dest[0]=temp[0]/10+'0';
    dest[1]=temp[0]%10+'0';
    dest[2]='/';
    dest[3]=temp[1]/10+'0';
    dest[4]=temp[1]%10+'0';
    dest[5]='/';
    dest[6]=temp[2]/10+'0';
    dest[7]=temp[2]%10+'0';
    dest[8]='/';
    dest[9]=temp[3]/10+'0';
    dest[10]=temp[3]%10+'0';
    dest[11]=':';
    dest[12]=temp[4]/10+'0';
    dest[13]=temp[4]%10+'0';
    
    return 0;
}

int RTC_SetReportTimeBytes(unsigned char index, const char *src)
{
    
    s_RTC_ReportTime[0]=src[0];
    s_RTC_ReportTime[1]=src[1];
    s_RTC_ReportTime[2]=src[2];
    s_RTC_ReportTime[3]=src[3];
    s_RTC_ReportTime[4]=src[4];
    DownInt();
    _RTC_WriteRAM(index,src[0]);
    _RTC_WriteRAM(index+1,src[1]);
    _RTC_WriteRAM(index+2,src[2]);
    _RTC_WriteRAM(index+3,src[3]);
    _RTC_WriteRAM(index+4,src[4]); 
    UpInt();
    return 0;
}

int RTC_RetrievePulseBytes()
{
    DownInt();
    s_RTC_PulseBytes[0][0]=_RTC_ReadRAM(PULSE1_BYTE1);
    s_RTC_PulseBytes[0][1]=_RTC_ReadRAM(PULSE1_BYTE2);
    s_RTC_PulseBytes[0][2]=_RTC_ReadRAM(PULSE1_BYTE3);
    s_RTC_PulseBytes[1][0]=_RTC_ReadRAM(PULSE2_BYTE1);
    s_RTC_PulseBytes[1][1]=_RTC_ReadRAM(PULSE2_BYTE2);
    s_RTC_PulseBytes[1][2]=_RTC_ReadRAM(PULSE2_BYTE3);
    s_RTC_PulseBytes[2][0]=_RTC_ReadRAM(PULSE3_BYTE1);
    s_RTC_PulseBytes[2][1]=_RTC_ReadRAM(PULSE3_BYTE2);
    s_RTC_PulseBytes[2][2]=_RTC_ReadRAM(PULSE3_BYTE3);
    s_RTC_PulseBytes[3][0]=_RTC_ReadRAM(PULSE4_BYTE1);
    s_RTC_PulseBytes[3][1]=_RTC_ReadRAM(PULSE4_BYTE2);
    s_RTC_PulseBytes[3][2]=_RTC_ReadRAM(PULSE4_BYTE3);
    UpInt();
     
    return 0;
}
//脉冲1
int RTC_ReadPulseBytes(int _index, char * _dest)
{
    
    if( _index < 1 || _index > 4 )
    { 
        return -2;
    }
    DownInt();
    if(s_RTC_PulseBytes[_index-1][0]==0xFF && s_RTC_PulseBytes[_index-1][1]==0xFF && s_RTC_PulseBytes[_index-1][2]==0xFF)
    {//内存值错误
        s_RTC_PulseBytes[_index-1][0]=_RTC_ReadRAM(PULSE1_BYTE1 + (_index-1)*3 );
        s_RTC_PulseBytes[_index-1][1]=_RTC_ReadRAM(PULSE1_BYTE2 + (_index-1)*3 );
        s_RTC_PulseBytes[_index-1][2]=_RTC_ReadRAM(PULSE1_BYTE3 + (_index-1)*3 );
    }
    _dest[0] = s_RTC_PulseBytes[_index-1][0];
    _dest[1] = s_RTC_PulseBytes[_index-1][1];
    _dest[2] = s_RTC_PulseBytes[_index-1][2];
    UpInt();
    return 0;
}

int RTC_SetPulseBytes(int _index, char * _src)
{
    
    if( _index < 1 || _index > 4 )
    { 
        return -2;
    }
    s_RTC_PulseBytes[_index-1][0] = _src[0];
    s_RTC_PulseBytes[_index-1][1] = _src[1];
    s_RTC_PulseBytes[_index-1][2] = _src[2];
    
    DownInt();
    //再写RTC RAM
    _RTC_WriteRAM(PULSE1_BYTE1 + (_index-1)*3 ,_src[0]);
    _RTC_WriteRAM(PULSE1_BYTE2 + (_index-1)*3 ,_src[1]);
    _RTC_WriteRAM(PULSE1_BYTE3 + (_index-1)*3 ,_src[2]);
    UpInt();
    return 0;
}

//
//  此函数只在中断中运行, 而中断服务中 是关闭中断的,因此不调用DownInt();
//
int RTC_IncPulseBytes(int _index)
{// 此函数 在中断中调用,
   // ( 所以执行要迅速  , 有空再优化)
    
    if( _index <=0 || _index > 4 )
    {
        return -2;
    } 
    //一切为了速度.
    char  _byte1 = s_RTC_PulseBytes[_index-1][0];
    char  _old_byte1 = _byte1;
    char  _byte2 = s_RTC_PulseBytes[_index-1][1];
    char  _old_byte2 = _byte2;
    char  _byte3 = s_RTC_PulseBytes[_index-1][2];
    char  _old_byte3 = _byte3;
    char  _max_byte1 = g_pulse_range[_index-1][0];
    char  _max_byte2 = g_pulse_range[_index-1][1];
    char  _max_byte3 = g_pulse_range[_index-1][2];
   
    if(_byte3==255)
    {//第3字节溢出的话
        _byte3=0;
        if( _byte2==255)
        {//第2字节溢出的话
            _byte2=0; 
            if(_byte1==255)//第1字节溢出的话 
                 _byte1=0; //实际上这个不可能发生,但就丢这吧.反正不影响速度. 
            else 
                ++ _byte1;  
        }
        else 
            ++ _byte2;
    }
    else
        ++ _byte3; 
    //
    //  要判断是否超过MAX了
    // 
    if( _byte1 < _max_byte1)
        goto Update_And_Return;
    if(_byte1 > _max_byte1)
    {//归0
        _byte1=0;_byte2=0;_byte3=0;
        goto Update_And_Return;
    }
    else
    {// _byte1==_max_byte1
        if( _byte2 < _max_byte2)
            goto Update_And_Return;
        if( _byte2 > _max_byte2)
        {
            _byte1=0;_byte2=0;_byte3=0;
            goto Update_And_Return;
        }
        else
        {// _byte2 == _max_byte2
            if( _byte3 > _max_byte3 )
            {
                _byte1=0;_byte2=0;_byte3=0;
                goto Update_And_Return;
            }
            //对于小于等于 就是 不超过MAX了
        }
    }
Update_And_Return: 
    // 然后更新 RTC里的值.
    // 注意: 根据前面的逻辑,只有修改了 低位的字节,高位的字节才有修改的可能
    if(_byte3 != _old_byte3)
    {
        s_RTC_PulseBytes[_index-1][2] = _byte3;
        _RTC_WriteRAM(PULSE1_BYTE3 + (_index-1)*3 ,_byte3 );
        if(_byte2 != _old_byte2)
        {
            s_RTC_PulseBytes[_index-1][1] = _byte2;
            _RTC_WriteRAM(PULSE1_BYTE2 + (_index-1)*3 ,_byte2 );
            if(_byte1 != _old_byte1)
            {
                s_RTC_PulseBytes[_index-1][0] = _byte1;
                _RTC_WriteRAM(PULSE1_BYTE1 + (_index-1)*3 ,_byte1 );
            }
        }
    }
    return 0;
}

 
int  RTC_IsBadTime(char * time, int isTime)
{//无错返回0,否则返回-1.
  //判断原则
  //  首先年份必须要大于等于09年
  //  年份必须小于20年, 
  //  
  //  月份必须为是1~12月之间 
  //  日期必须为是1~31之间
  //  时必须是0~23之间
  //  分必须是0~59之间
  //
  //
   if(time[0]<9)
       return -1;
   if(time[0]>30)//30年
       return -1;
   if(time[1]<1)
       return -1;
   if(time[1]>12)
       return -1;
   
   if(time[2]<1)
       return -1;
   if(time[2]>31)
       return -1;
   if(time[3]>23)
       return -1;
   if(time[4]>59)
       return -1; 
   
   
   
   if(isTime==0)
   {
     return 0;
   }
   
   //与前一个时间进行对比
   //天数太复杂暂时就不比较了.
   //首先这个lastTime自己要是正确的
   //主要是怕这个变量被错误修改.概率非常的小,但还是防范下
   //我们主要在乎 年  月 和 时
   if(s_RTC_lastTime[0] >= 9  && s_RTC_lastTime[1] <= 12 && s_RTC_lastTime[1] >= 1
      && s_RTC_lastTime[3]<=23)
   {
     //开始比较
     
     //年
     if( time[0]!=s_RTC_lastTime[0] && time[0]!=s_RTC_lastTime[0]+1)
     {//即不为今年 也不为去年.那么就是个错误
       return -1;
     }
     
     //月
     //考虑1月的情况
     if( time[1]!=s_RTC_lastTime[1] && time[1]!=s_RTC_lastTime[1]+1)
     {//即不为当月,也不为前一个月,
       //考虑为1月的情况,它的前一个月为12月
       //而且月不为1月, 为1月时,last不为12.
       if(time[1]!=1)
       {
         return -1;
       }
       else
       {
         if(s_RTC_lastTime[1]!=12)
         {
           return -1;
         }
       }
     }
     
     //时
     //考虑00时的情况
     if( time[3]!=s_RTC_lastTime[3] && time[3]!=s_RTC_lastTime[3]+1)
     {
       if(time[3]!=0)
       {
         return -1;
       }
       else
       {
         if(s_RTC_lastTime[3]!=23)
         {
           return -1;
         }
       }
     }
   }
   //此时我们认为当前时间是正确的
   //保存起来
   for(int i=0;i<5;++i)
     s_RTC_lastTime[i]=time[i];
   return 0;
}



//判断时间是否是过去 ,格式为 年月日时分 的char[5]类型 

// 1   为过去时间
// 0   为现在时间
// -1  为未来时间  

int  RTC_IsPassed(char * time)
{   
    if( g_rtc_nowTime[0] != time[0])
    {
       if(g_rtc_nowTime[0]>time[0])
          return 1;
       else
          return -1;
    } 
    if( g_rtc_nowTime[1] != time[1])
    { 
       if(g_rtc_nowTime[1] > time[1])
          return 1;
       else
          return -1;
    } 
    if( g_rtc_nowTime[2] != time[2])
    { 
       if(g_rtc_nowTime[2] > time[2])
          return 1;
       else
          return -1;
    }  
    if( g_rtc_nowTime[3] != time[3])
    { 
       if( g_rtc_nowTime[3] > time[3])
          return 1;
       else
          return -1;
    } 
    if( g_rtc_nowTime[4] != time[4])
    {
       if( g_rtc_nowTime[4] > time[4])
          return 1;
       else
          return -1;
    }
    else
       return 0; //分钟数一样 
}   

//读出  09/03/05/23:00:00 格式的字符串
void RTC_ReadTimeStr6_A(char * dest)
{ 
    char year;
    char day;
    char month;
    char date;
    char hour;
    char minute;
    char second;
    char control;
    DownInt();
    _RTC_ReadTime(&second,&minute,&hour,&date,&month,&day,&year,&control); 
    UpInt();
    dest[0]=year/10+'0';
    dest[1]=year%10+'0';
    dest[2]='/'; 
    dest[3]=month/10+'0';
    dest[4]=month%10+'0';
    dest[5]='/'; 
    dest[6]=date/10+'0';
    dest[7]=date%10+'0';
    dest[8]='/'; 
    dest[9]=hour/10+'0';
    dest[10]=hour%10+'0';
    dest[11]=':';
    dest[12]=minute/10+'0';
    dest[13]=minute%10+'0'; 
    dest[14]=':';
    dest[15]=second/10+'0';
    dest[16]=second%10+'0';
    
} 
//      01234567 8 9 10  11 12 13  
//读出  09/03/05   2  3  :  0  0 格式的字符串
void RTC_ReadTimeStr5_A(char * dest)
{ 
    
    char year;
    char day;
    char month;
    char date;
    char hour;
    char minute;
    char second;
    char control;
    DownInt();
    _RTC_ReadTime(&second,&minute,&hour,&date,&month,&day,&year,&control);
    UpInt();
    dest[0]=year/10+'0';
    dest[1]=year%10+'0';
    dest[2]='/';
    dest[3]=month/10+'0';
    dest[4]=month%10+'0';
    dest[5]='/';
    dest[6]=date/10+'0';
    dest[7]=date%10+'0';
    dest[8]='/';
    dest[9]=hour/10+'0';
    dest[10]=hour%10+'0';
    dest[11]=':';
    dest[12]=minute/10+'0';
    dest[13]=minute%10+'0';
    
}


//12个字符.
void RTC_ReadTimeStr6_B(char * dest)
{   
    char year;
    char day;
    char month;
    char date;
    char hour;
    char minute;
    char second;
    char control;
    DownInt();
    _RTC_ReadTime(&second,&minute,&hour,&date,&month,&day,&year,&control);
    UpInt();
    dest[0]=year/10+'0';
    dest[1]=year%10+'0'; 
    dest[2]=month/10+'0';
    dest[3]=month%10+'0'; 
    dest[4]=date/10+'0';
    dest[5]=date%10+'0'; 
    dest[6]=hour/10+'0';
    dest[7]=hour%10+'0'; 
    dest[8]=minute/10+'0';
    dest[9]=minute%10+'0';  
    dest[10]=second/10+'0';
    dest[11]=second%10+'0';
    
}
void RTC_ReadTimeStr5_B(char * dest)
{  
    
    char year;
    char day;
    char month;
    char date;
    char hour;
    char minute;
    char second;
    char control;
    DownInt();
    _RTC_ReadTime(&second,&minute,&hour,&date,&month,&day,&year,&control); 
    UpInt();
    dest[0]=year/10+'0';
    dest[1]=year%10+'0'; 
    dest[2]=month/10+'0';
    dest[3]=month%10+'0'; 
    dest[4]=date/10+'0';
    dest[5]=date%10+'0'; 
    dest[6]=hour/10+'0';
    dest[7]=hour%10+'0'; 
    dest[8]=minute/10+'0';
    dest[9]=minute%10+'0';
    
}
//01234567890123456
//09/10/14/10:00:00
void RTC_SetTimeStr6_A(const char * src)
{
    DownInt();
    _RTC_SetTime( (src[15]-'0')*10 + src[16]-'0',(src[12]-'0')*10 + src[13]-'0',
                 (src[9]-'0')*10 + src[10]-'0',(src[6]-'0')*10+src[7]-'0',
                 (src[3]-'0')*10+src[4]-'0',1,(src[0]-'0')*10 +src[1]-'0',0);
    UpInt();
}

void RTC_SetTimeStr6_B(const char *src)
{
    DownInt();
    _RTC_SetTime((src[10]-'0')*10+src[11]-'0', (src[8]-'0')*10+src[9]-'0',
              (src[6]-'0')*10+src[7]-'0' ,(src[4]-'0')*10+src[5]-'0' ,
              (src[2]-'0')*10+src[3]-'0' ,1,(src[0]-'0')*10+src[1]-'0',0);
    UpInt();
}
//以 年年月月日日时时分分 的格式来设置时间
//   0 1 2 3 4 5 6 7 8 9
//  090509101123
// 星期 全部设置为1,星期1
void RTC_SetTimeStr5_B(const char *src)
{ 
    DownInt();
    char second=_RTC_ReadSecond();
    _RTC_SetTime(second , (src[8]-'0')*10+src[9]-'0',
              (src[6]-'0')*10+src[7]-'0' ,(src[4]-'0')*10+src[5]-'0' ,
              (src[2]-'0')*10+src[3]-'0' ,1,(src[0]-'0')*10+src[1]-'0',0);
    UpInt();
}
void RTC_ReadTimeBytes5(char * dest)
{
    
    char second;
    char control;
    char day;
    DownInt();
    _RTC_ReadTime(&second,&(dest[4]),&(dest[3]),&(dest[2]),&(dest[1]),&day,&(dest[0]),&control);
    UpInt();
}
int RTC_SetTimeBytes5(const char * src)
{
    DownInt();
    char second = _RTC_ReadSecond();
    int ret=_RTC_SetTime(second,src[4],src[3],src[2],src[1],1,src[0],0); 
    UpInt();
    return ret;
}
void RTC_ReadTimeBytes6(char * dest )
{
#if 0

    char year;
    char day;
    char month;
    char date;
    char hour;
    char minute;
    char second;
    char control;
    DownInt();
    _RTC_ReadTime(&second,&minute,&hour,&date,&month,&day,&year,&control); 
    UpInt();
    dest[0]=year/10+'0';
    dest[1]=year%10+'0';

    dest[2]=month/10+'0';
    dest[3]=month%10+'0';

    dest[4]=date/10+'0';
    dest[5]=date%10+'0';

    dest[6]=hour/10+'0';
    dest[7]=hour%10+'0';

    dest[8]=minute/10+'0';
    dest[9]=minute%10+'0'; 

    dest[10]=second/10+'0';
    dest[11]=second%10+'0';
#else
    char control;
    char day;
    DownInt();

    _RTC_ReadTime(&(dest[5]),&(dest[4]),&(dest[3]),&(dest[2]),&(dest[1]),&day,&(dest[0]),&control);

    UpInt();
#endif
}
int RTC_SetTimeBytes6(const char * src)
{ 
    DownInt();
    int ret= _RTC_SetTime(src[5],src[4],src[3],src[2],src[1],1,src[0],0); 
    UpInt();
    return ret;
}
 


//定义待设置的时间：秒、分、时、日、月、星期、年、控制字
int _RTC_SetTime(const char second,const char minute,
                 const char hour,const char date,const char month,
                 const char day,const char year,const char control)   //设置99年12月31日 星期7 23点59分00秒
{
  if(second >59 || minute >59 || hour >23|| day>7 || day ==0 || date>31 
     || date==0 || month >12 || month ==0 || year >99)
    return -1;
  char temp[8];
  temp[0]=_DECtoBCD(second);
  temp[1]=_DECtoBCD(minute);
  temp[2]=_DECtoBCD(hour);
  temp[3]=_DECtoBCD(date);
  temp[4]=_DECtoBCD(month);
  temp[5]=_DECtoBCD(day);
  temp[6]=_DECtoBCD(year);
  temp[7]=_DECtoBCD(control);
  
//  _RTC_EnableWrite();
//  _RTC_MultiWrite(CMD_WRITE_BATCH,temp,8);
//  _RTC_DisableWrite();
  _RTC_Write_OneByte(RegAddr_Sec,temp[0]);
  _RTC_Write_OneByte(RegAddr_Min,temp[1]);
  _RTC_Write_OneByte(RegAddr_Hour,temp[2]);
  _RTC_Write_OneByte(RegAddr_Day,temp[5]);
  _RTC_Write_OneByte(RegAddr_Date,temp[3]);
  _RTC_Write_OneByte(RegAddr_CMon,temp[4]);
  _RTC_Write_OneByte(RegAddr_Year,temp[6]);
  
  
  
  //时间数据在放到s_RTC_lastTime里保存一下
  
  s_RTC_lastTime[0]=year;
  s_RTC_lastTime[1]=month;
  s_RTC_lastTime[2]=date;
  s_RTC_lastTime[3]=hour;
  s_RTC_lastTime[4]=minute;
  return 0;
} 


char _RTC_ReadSecond()
{
  char temp;
  //_RTC_MultiRead(CMD_READ_SECOND,&temp,1);
  temp = _RTC_Read_OneByte(RegAddr_Sec);
  return  _BCDtoDEC(temp);
}



void _RTC_ReadTime(char *second,char *minute,
                 char *hour,char *date,char *month,
                 char *day, char *year,char *control)
{
  char temp[8];
//  _RTC_MultiRead(CMD_READ_BATCH,temp,8);
  
  temp[0] = _RTC_Read_OneByte(RegAddr_Sec);
  temp[1] = _RTC_Read_OneByte(RegAddr_Min);
  temp[2] = _RTC_Read_OneByte(RegAddr_Hour);
  temp[5] = _RTC_Read_OneByte(RegAddr_Day);
  temp[3] = _RTC_Read_OneByte(RegAddr_Date);
  temp[4] = _RTC_Read_OneByte(RegAddr_CMon);
  temp[6] = _RTC_Read_OneByte(RegAddr_Year);
  
  temp[0] &= ~BIT7;
  *second =_BCDtoDEC(temp[0]);
  *minute =_BCDtoDEC(temp[1]);
  *hour =_BCDtoDEC(temp[2]);
  *date =_BCDtoDEC(temp[3]);
  *month =_BCDtoDEC(temp[4]);
  *day =_BCDtoDEC(temp[5]);
  *year =_BCDtoDEC(temp[6]);
  *control =_BCDtoDEC(temp[7]);
}
/*
void RTC_ReadRTCTimeBytes(char * dest)
{
  char temp[8];
  _RTC_MultiRead(CMD_READ_BATCH,temp,8);
  temp[0] &= ~BIT7;
  dest[0]=_BCDtoDEC(temp[0]);
  dest[1]=_BCDtoDEC(temp[1]);
  dest[2]=_BCDtoDEC(temp[2]);
  dest[3]=_BCDtoDEC(temp[3]);
  dest[4]=_BCDtoDEC(temp[4]);
  dest[5]=_BCDtoDEC(temp[5]);
  dest[6]=_BCDtoDEC(temp[6]);
  dest[7]=_BCDtoDEC(temp[7]);
} 
*/
int _RTC_SendByte(char data)
{
    _RTC_Set_IO_OUT(); //io线设置为输出。
      
    for(int i=0;i<8;i++)
    {  
       //从低到高 发送到 IO上
        if(data & 0x01)
        {  
          _RTC_Set_IO_high(); 
        }
        else
        {
          _RTC_Set_IO_low();
        }
        // IO线上的电平确定好后，
        //给时钟线上一个上升的脉冲，DS1302就会去接收该数据,
        //时钟下降沿，发送数据有效
        
        _RTC_Set_SCLK_low();
        _RTC_Set_SCLK_low();
        _RTC_Set_SCLK_high();
        _RTC_Set_SCLK_high();
        
        //等待DS1302 接收这个数据
         _RTC_delay(40); 
         
        //移位
        data>>=1;
    } 
    return 0;
}

char _RTC_RecvByte(void)
{ 
    char temp=0;
    //把IO线的输出设置为高，
    //  可以帮助驱动DS1302拉高IO线的电平
    _RTC_Set_IO_high();
    //io线设置为输入，接收数据
    _RTC_Set_IO_IN();
   
    for(int i=0; i<8;++i)
    { 
        //   给时钟线上一个下降的脉冲，表明数据已经接收,
       
        //      DS1302会继续发下一位数据到 IO上 。
        
        //给个下降沿延时一下  就开始接收 数据了
        _RTC_delay(40);
        _RTC_Set_SCLK_high();   
        _RTC_Set_SCLK_high();
        _RTC_Set_SCLK_low();
        _RTC_Set_SCLK_low();
     
        //总共只需移7位，而不是8，所以放在读取之前
        temp >>=1;
        // 发送读控制字节后，IO线上就会获得数据
        // 从IO上一个个读取，从低到高，最后一个写在最高位。
        if((P1IN & RTC_IO)==RTC_IO)
        {
            temp |=0x80;
        }
        // IO线上的电平读取完毕之后，
    }
    return temp;
}

/*
 * function: read one byte from RTC
 * params: regAddr=>RTC reg address
 * return: char 
*/
char _RTC_Read_OneByte(char regAddr)
{
    char read_data;
    
    //start 
    IIC_Start(); 
    //send write cmd
    IIC_Send_Byte(WRITE_CMD);   
    //wait ack
    IIC_Wait_Ack();
    //send reg addr
    IIC_Send_Byte(regAddr);   
    //wait ack
    IIC_Wait_Ack();	
    //iic_start
    IIC_Start(); 
    //send read cmd
    IIC_Send_Byte(READ_CMD);          
    //wait ack
    IIC_Wait_Ack();
    //receive data byte
    read_data=IIC_Read_Byte(0);
    //iic stop
    IIC_Stop();
    
    return read_data;
 
}

/*
 * func: write one byte into RTC 
*/
void _RTC_Write_OneByte(char regAddr,char data)
{
     //start 
    IIC_Start(); 
    //send write cmd
    IIC_Send_Byte(WRITE_CMD);   //发送器件地址0XA0,写数据 
    //wait ack
    IIC_Wait_Ack();
    //send reg addr
    IIC_Send_Byte(regAddr);   //发送低地址
    //wait ack
    IIC_Wait_Ack();	
    //send write_data
    IIC_Send_Byte(data);
    //wait ack
    IIC_Wait_Ack();
    //iic stop
    IIC_Stop();
  
}


int _RTC_MultiRead(char cmd, char * dest, int length)
{
    //打开DS1302,之前将 时钟线设置为低
    //_RTC_Set_RST_low(); 
    _RTC_Set_SCLK_low();
    _RTC_Set_SCLK_low();
    //////////_RTC_Set_RST_high();
    //发送命令
    _RTC_SendByte(cmd);
   //开始接收数据
    for(int i=0;i<length;++i)
    {
       dest[i]=_RTC_RecvByte();
     }
    //读完之后关闭DS1302 
    //_RTC_Set_RST_low();
    return 0;
}

int _RTC_MultiWrite(char cmd ,const char * src, int length)
{
    //打开DS1302,之前将 时钟线设置为低
    
    
    //_RTC_Set_RST_low(); 
    _RTC_Set_SCLK_low();
    _RTC_Set_SCLK_low();
    //////////_RTC_Set_RST_high();
    //发送cmd
    _RTC_SendByte(cmd);
    //连续发送数据
    for(int i=0;i<length;++i)
    {
       _RTC_SendByte(src[i]);
    }
    //写完之后关闭DS1302
    //RTC_Set_RST_low(); 
    
    //_RTC_Set_RST_low();
    
     
   // _ENT();
    return 0;
}

/* not used in RTC DS3231 */
void _RTC_DisableWrite()
{
//    char temp=0x80;
//    _RTC_MultiWrite(CMD_WRITE_CONTROL,&temp,1);
}
/* not used in RTC DS3231 */
void _RTC_EnableWrite()
{
//    char temp=0x00;
//    _RTC_MultiWrite(CMD_WRITE_CONTROL,&temp,1);
}
void _RTC_Go()
{  
    //应该先读出秒钟，再清高位，再写进去，这样秒钟不会错
    char temp;
    //_RTC_MultiRead(CMD_READ_SECOND,&temp,1);
    temp = _RTC_Read_OneByte(RegAddr_Sec);
    temp &= ~BIT7;  
    _RTC_EnableWrite();
    //_RTC_MultiWrite(CMD_WRITE_SECOND,&temp,1);
    _RTC_Write_OneByte(RegAddr_Sec,temp);
    _RTC_DisableWrite();
}
void _RTC_Pause()
{
    //应该先读出秒钟，再置高位，再写进去，这样秒钟不会错
    char temp;
    //_RTC_MultiRead(CMD_READ_SECOND,&temp,1);
    temp = _RTC_Read_OneByte(RegAddr_Sec);
    temp |= BIT7;
    _RTC_EnableWrite();
    //_RTC_MultiWrite(CMD_WRITE_SECOND,&temp,1);
    _RTC_Write_OneByte(RegAddr_Sec,temp);
    _RTC_DisableWrite();
}

/*
 * func: check if Oscillator stop
 * return: Oscillator stop=>1/2  Normal=>0
*/
int _RTC_Check()
{
    //RegAddr_Control,// Control
    //RegAddr_CtlStat,// Control/Status
    char temp;
    
    if(_RTC_Read_OneByte(RegAddr_Control) & 0x80) //晶振停止工作了
        return 1;
    else if(_RTC_Read_OneByte(RegAddr_CtlStat) & 0x80) //或者 EOSC被禁止了
        return 2;
    else                //normal
        return 0;
}
// void _RTC_DisableCharge()
// {
//     char temp=0x50;
//     _RTC_EnableWrite();
//     _RTC_MultiWrite(CMD_WRITE_CHARGE,&temp,1);
//     _RTC_DisableWrite();
// }
// void _RTC_EnableCharge()
// {
//   char temp=0xA5;
//   _RTC_EnableWrite();
//   _RTC_MultiWrite(CMD_WRITE_CHARGE,&temp,1);
//   _RTC_DisableWrite();
// }

/*
 * 存到RTC_RAM中的值:
 * StartIdx,EndIdx   ==>  发送状态队列的队首队尾
 * LastCheckTime  ==>   全局变量，内存中有一份，RTC的RAM中也存一份
 * NextCheckTime   ==>  下一次检查时间
 * 
*/

/*
 * instruction: there is no RAM in DS3231,assign 31 bytes from EEPROM as RTC_RAM
 * param: index=>(RAM offset,range from 0 to 30)
*/
void _RTC_WriteRAM(unsigned char index,const char data)
{
    if(index > 30)
        return;
//   _RTC_EnableWrite();
//   _RTC_MultiWrite(RTC_RAM_BASE+index,&data,1);
//   _RTC_DisableWrite();
    
    ROM_WriteByte_RTC(RTC_RAM_BASE+(long)index,data); 

}

/*
 * instruction: there is no RAM in DS3231,assign 31 bytes from EEPROM as RTC_RAM
 * param: index=>(RAM offset,range from 0 to 30)
*/
char _RTC_ReadRAM(unsigned char index)
{
  if(index >30)
    return 0;
  char temp;
//   _RTC_MultiRead(CMD_READ_RAM_BASE+(index<<1),&temp,1);
  ROM_ReadByte(RTC_RAM_BASE+(long)index,&temp);
  return temp;
}
  
// 低级函数 和  辅助函数
char _BCDtoDEC(char val)
{
    val = (val >> 4) *10+(val &0x0f); ///将BCD码转换为10进制数
    return val; ///返回10进制数
}
char _DECtoBCD(char val)
{
  return ((val / 10) *16+val % 10);
}
void _RTC_delay(int j)
{ //时序长短很重要
  for(int i=0 ;i <j;i++);//以前为20
}

/*
 * function: init pins,enable RTC inside Oscillator
 * pins: P9.2=>RTC_SCLK  P9.1=>RTC_SDA
*/ 
int RTC_Open()
{  
  
    /* set pins */
    DownInt();
    System_Delayms(20);
    P9DIR |= RTC_SCLK; // set p9.2 output
    P9DIR |= RTC_IO;   // set p9.1 output
    IIC_SCL_HIGH();
    IIC_SDA_HIGH();
    System_Delayms(500);  //delay to wait RTC ready
    UpInt();

    /* enable Oscillator and clear flags*/
    _RTC_Write_OneByte(RegAddr_Control,0);
    _RTC_Write_OneByte(RegAddr_CtlStat,0);
    
    return 0;
}


int RTC_Close()
{  
    return 0;
}

void _RTC_Set_RST_low()
{ 
    P4OUT &= ~(RTC_RST);   
    _RTC_delay(20); 
    return ;
}
void _RTC_Set_RST_high()
{   
    P4OUT |= RTC_RST;  
    _RTC_delay(20);
    return ;
}
void _RTC_Set_SCLK_low()
{
    
    P1OUT &= ~(RTC_SCLK);
    _RTC_delay(10);
    return ;
}
void _RTC_Set_SCLK_high()
{
    P1OUT |= RTC_SCLK;
    _RTC_delay(10);
    return ;
}
void _RTC_Set_IO_low()
{
    P1OUT &= ~(RTC_IO);
    _RTC_delay(10);
    return ;
}
void _RTC_Set_IO_high()
{
    P1OUT |= RTC_IO;
    _RTC_delay(10);
    return ;
} 
void _RTC_Set_IO_IN()
{
    P1DIR &= ~RTC_IO ;
    _RTC_delay(10);
}
void _RTC_Set_IO_OUT()
{
    P1DIR |= RTC_IO ; 
    _RTC_delay(10);
}

/* IIC 操作 */

void IIC_SCL_HIGH(void)
{
    P9OUT |= 1<<2;
}

void IIC_SCL_LOW(void)
{
    P9OUT &= ~(1<<2);
}

void IIC_SDA_HIGH(void)
{
    P9OUT |= 1<<1;
}

void IIC_SDA_LOW(void)
{
    P9OUT &= ~(1<<1);
}

int READ_SDA(void)
{
  
    if(P9IN & (1<<1))
        return 1;
    else
        return 0;
}

//初始化IIC
void IIC_Init(void)
{					     
//	RCC->AHB1ENR|=1<<7;    //使能PORTH时钟	   	  
//	GPIO_Set(GPIOH,PIN4|PIN5,GPIO_MODE_OUT,GPIO_OTYPE_PP,GPIO_SPEED_50M,GPIO_PUPD_PU);//PH4/PH5设置 
//	IIC_SCL_HIGH();
//	IIC_SDA_HIGH();
}
//产生IIC起始信号
void IIC_Start(void)
{
	SDA_OUT();     //sda线输出
	IIC_SDA_HIGH();	  	  
	IIC_SCL_HIGH();
	Delay_us(4);
 	IIC_SDA_LOW();//START:when CLK is high,DATA change form high to low 
	Delay_us(4);
	IIC_SCL_LOW();//钳住I2C总线，准备发送或接收数据 
}	  
//产生IIC停止信号
void IIC_Stop(void)
{
	SDA_OUT();//sda线输出
	IIC_SCL_LOW();
	IIC_SDA_LOW();//STOP:when CLK is high DATA change form low to high
 	Delay_us(4);
	IIC_SCL_HIGH(); 
	IIC_SDA_HIGH();//发送I2C总线结束信号
	Delay_us(4);							   	
}
//等待应答信号到来
//返回值：1，接收应答失败
//        0，接收应答成功
u8 IIC_Wait_Ack(void)
{
	u8 ucErrTime=0;
	SDA_IN();      //SDA设置为输入  
	IIC_SDA_HIGH();Delay_us(1);	   
	IIC_SCL_HIGH();Delay_us(1);	 
	while(READ_SDA())
	{
		ucErrTime++;
		if(ucErrTime>250)
		{
			IIC_Stop();
			return 1;
		}
	}
	IIC_SCL_LOW();//时钟输出0 	   
	return 0;  
} 
//产生ACK应答
void IIC_Ack(void)
{
	IIC_SCL_LOW();
	SDA_OUT();
	IIC_SDA_LOW();
	Delay_us(2);
	IIC_SCL_HIGH();
	Delay_us(2);
	IIC_SCL_LOW();
}
//不产生ACK应答		    
void IIC_NAck(void)
{
	IIC_SCL_LOW();
	SDA_OUT();
	IIC_SDA_HIGH();
	Delay_us(2);
	IIC_SCL_HIGH();
	Delay_us(2);
	IIC_SCL_LOW();
}					 				     
//IIC发送一个字节
//返回从机有无应答
//1，有应答
//0，无应答			  
void IIC_Send_Byte(u8 txd)
{                        
    u8 t;   
    SDA_OUT(); 	    
    IIC_SCL_LOW();//拉低时钟开始数据传输
    for(t=0;t<8;t++)
    {   
        if((txd&0x80)>>7)
            IIC_SDA_HIGH();
        else
            IIC_SDA_LOW();
        txd<<=1; 	  
        Delay_us(2);   //对TEA5767这三个延时都是必须的
        IIC_SCL_HIGH();
        Delay_us(2); 
        IIC_SCL_LOW();	
        Delay_us(2);
    }	 
} 	    
//读1个字节，ack=1时，发送ACK，ack=0，发送nACK   
u8 IIC_Read_Byte(unsigned char ack)
{
	unsigned char i,receive=0;
	SDA_IN();//SDA设置为输入
    for(i=0;i<8;i++ )
	{
        IIC_SCL_LOW(); 
        Delay_us(2);
        IIC_SCL_HIGH();
        receive<<=1;
        if(READ_SDA())receive++;   
        Delay_us(1); 
    }					 
    if (!ack)
        IIC_NAck();//发送nACK
    else
        IIC_Ack(); //发送ACK   
    return receive;
}
