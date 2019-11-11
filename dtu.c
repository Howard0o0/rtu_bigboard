//////////////////////////////////////////////////////
//     文件名: dtu.c
//   文件版本: 1.0.0
//   创建时间: 09年 11月30日
//   更新内容:  
//       作者: 林智
//       附注: DTU默认情况下处于透明数据状态
//         
//
//
//
//////////////////////////////////////////////////////

#include "msp430common.h"
#include "dtu.h"
#include "uart0.h"
#include "uart3.h"
#include "common.h"
#include "led.h"
#include "rom.h"
#include "flash.h"
#include "rtc.h"
#include "store.h"
#include "Sampler.h"


extern int s_alert_flag[8];
 int _dtu_request_time=0;



//用于组成数据
int _DTU_PackOKData(char * _type,char * _dest)
{
    _dest[0]=_type[0];_dest[1]=_type[1];
    _dest[2]='*';_dest[3]='O';_dest[4]='K';
    //#setend#
    _dest[5]='#';_dest[6]='s';_dest[7]='e';
    _dest[8]='t';_dest[9]='e';_dest[10]='n';
    _dest[11]='d';_dest[12]='#';
    return 13;
}
int _DTU_PackBadData(char * _type,char * _dest)
{
    _dest[0]=_type[0];_dest[1]=_type[1];
    _dest[2]='*';_dest[3]='B';_dest[4]='A';_dest[5]='D';
    //#setend#
    _dest[6]='#';_dest[7]='s';_dest[8]='e';
    _dest[9]='t';_dest[10]='e';_dest[11]='n';
    _dest[12]='d';_dest[13]='#';
    return 14;
}

int _DTU_PackErrorData(char * _type,char * _dest)
{
    _dest[0]=_type[0];_dest[1]=_type[1];
    _dest[2]='*';_dest[3]='E';_dest[4]='R';
    _dest[5]='R';_dest[6]='O';_dest[7]='R';
    //#setend#
    _dest[8]='#';_dest[9]='s';_dest[10]='e';
    _dest[11]='t';_dest[12]='e';_dest[13]='n';
    _dest[14]='d';_dest[15]='#';
    
    return 16;
}
int _DTU_PackFailData(char * _type,char * _dest)
{
    _dest[0]=_type[0];_dest[1]=_type[1];
    _dest[2]='*';_dest[3]='F';_dest[4]='A';
    _dest[5]='I';_dest[6]='L';
    //#setend#
    _dest[7]='#';_dest[8]='s';_dest[9]='e';
    _dest[10]='t';_dest[11]='e';_dest[12]='n';
    _dest[13]='d';_dest[14]='#';
    
    return 15;
}

extern int g_work_mode;
extern int main_time_error;

int  DTU_Open(int _wait)
{
    //模块上电
    //无法控制
    TraceMsg("DTU Open",1);
    
    
    P4DIR |= BIT2; // P4.2 为 BATT,给高电平
    P4OUT &= ~BIT2;
    P4OUT |= BIT2;
    
    
    //开启rs232  电平转换电路
    P4DIR |= BIT0;
    P4OUT |= BIT0;
    //使能UART0
    UART0_Open(UART0_DTU_TYPE);
    
    if(_wait)
    {
        TraceMsg("waiting for 20s .",1);
        System_Delayms(20000);
    }
    return 0;
} 
int  DTU_CheckOK()
{    
    TraceMsg("DTU Check .",1);   
    int _repeats=0;
    char __data[UART0_MAXBUFFLEN];
    char * _data =NULL;
    int _dataLen=0; 
    while(_repeats<2)
    {//发送一条心跳包给中心.
        //服务器端 不愿意 加这样的一条数据类型.
        //我们这边就以时间查询来实现这样的效果
        DTU_SendData("$timehead$#timeend#",19);
        //等10秒
        if(UART0_RecvLineLongWait(__data,UART0_MAXBUFFLEN,&_dataLen)<0)
        {
            //return 0;
            ++ _repeats;
        }
        else
        {
            _data=__data;
            while( *_data <= 0x20 || * _data >0x7A)
            {     ++_data; --_dataLen;}
            if(_data[0]=='$' && _data[_dataLen-1]=='#')
            {//至少能说明这一个从上位机来的数据了.
                //我们处理一下
                //返回0
                DTU_ProcRecvData(_data,_dataLen);
                TraceMsg("Check OK!",1);
                return 0;
            }
            //DTU自身的数据,就不管它.
            TraceMsg("DTU_Check: a bad data  no $#",1);
            TraceMsg(_data,1);
        }
    }
    TraceMsg("Check Fail !",1);
    return -1;
}

void DTU_Close()
{
    TraceMsg("DTU Close ",1);
    DTU_SendData("+++",3);
    DTU_SendData("AT^SMSO",7);
    P4DIR |= BIT2; // P4.2 为 BATT,给高电平
    P4OUT &= ~BIT2;
    System_Delayms(2000);
    //关闭 max3222
    P4DIR |= BIT0; P4OUT |= BIT0;
    UART0_Close();
    return ;
}
 
//
//  $datahead$[数据内容]#dataend#
//
 
//
//   要重新修改. 发送的所有数据要一次性发送.
//
int DTU_Report_Process()
{
    TraceMsg("DTU Report .",1);
    //检查数据存储上标和 下标 
    char _startIdx=0x00;
    char _endIdx=0x00;
    char _send[120]; 
    int  _ret=0;   
    int  _seek_num=0;//防止死循环
    
    if(RTC_ReadStartIdx(&_startIdx)<0 || RTC_ReadEndIdx(&_endIdx)<0 )
    {//如果读出数据索引标记失败
        TraceMsg("read idx error ,reGenerate .",1);
        
        if(RTC_RetrieveIndex()<0)
        {
            TraceMsg("reGen failed .",1);
            RTC_SetStartIdx(DATA_MIN_IDX);
            RTC_SetEndIdx(DATA_MIN_IDX);
        }
        TraceMsg("StartIdx:",0); 
        TraceInt4(_startIdx,1);
        TraceMsg("EndIdx:",0);
        TraceInt4(_endIdx,1);
        RTC_ReadStartIdx(&_startIdx);//重新读出
        RTC_ReadEndIdx(&_endIdx);//重新读出
    }
    //下标正确性
    if( _endIdx<DATA_MIN_IDX || _endIdx >DATA_MAX_IDX ||
       _startIdx<DATA_MIN_IDX || _startIdx >DATA_MAX_IDX  )
    {
        TraceMsg("Idx bad .",1);
        return -1;
    }
    TraceMsg("startIdx=",0);
    TraceInt4(_startIdx,0);
    TraceMsg(" ; endIdx=",0);
    TraceInt4(_endIdx,1);
    
    //头部
    Utility_Strncpy(_send,"$datahead$$start$$",18);
    
    if(Store_ReadDeviceNO(&(_send[18]))<0)
    {
         Utility_Strncpy(&_send[18],"00000000000",11);
    }
    _send[29] = '>';
    while(1)
    {
        TraceMsg("read data in :",0);
        TraceInt4(_startIdx,1);
        if(_seek_num>DATA_ITEM_LEN)//寻找的数据条数已经超过最大值就退出
            break;
        //数据
        _ret = Store_ReadDataItem(_startIdx,&_send[30],0);
        if(_ret<0)
        {
            TraceMsg("can't read data ! very bad .",1);
            return -1; //无法读取数据 就直接退了.
        }
        if(_ret==1)
        {//这个是一个已经发送过的数据, 
            TraceMsg("It's sended data",1);
            if(_startIdx == _endIdx)
            {//检查是否到了  _endIdx, 如果是就不继续循环了. 
                break;
            } 
            //继续下一个位置
            if(_startIdx >= DATA_MAX_IDX)  _startIdx=DATA_MIN_IDX;
            else   ++ _startIdx;//下一数据
            
            ++_seek_num;
            continue;
        }
        //是正常的发送数据,就增加_idx
        //增加下一次的 _startIdx    
        
        //并设置该数据已发送
        Store_MarkDataItemSended(_startIdx);
        
        if(_startIdx>=DATA_MAX_IDX) _startIdx=DATA_MIN_IDX;
        else   ++ _startIdx;//下一数据
        
        ++_seek_num;
        
        _ret+=30;//开始填写尾部
        //发送数据##end##dataend#
        _send[_ret++]='#';_send[_ret++]='#';_send[_ret++]='e';_send[_ret++]='n';
        _send[_ret++]='d';_send[_ret++]='#';
        _send[_ret++]='#';_send[_ret++]='d';_send[_ret++]='a';_send[_ret++]='t';
        _send[_ret++]='a';_send[_ret++]='e';_send[_ret++]='n';_send[_ret++]='d';
        _send[_ret++]='#';
        DTU_SendData(_send,_ret);
        //发送完后,要更新_startIdx. 
        RTC_SetStartIdx(_startIdx);
    }
    TraceMsg("Report done.Waiting for config data",1);
    return 0;
}

int  DTU_ProcAllData()
{
    int _dataLen;
    char _data[UART0_MAXBUFFLEN];
    while(UART0_RecvLineWait(_data,UART0_MAXBUFFLEN,&_dataLen)==0)
    {
        DTU_ProcRecvData(_data,_dataLen);
    }
    return 0;
}
  

int  DTU_ProcRecvData(char * _data, int _dataLen)
{ 
    Led2_WARN();
    
    TraceMsg("DTU Got Data:",1);
    TraceStr(_data,_dataLen,1);   
    
    if(_data[0]!='$' || _data[_dataLen-1]!='#')//不以$开头#结尾就直接忽略
    {//非上位机发来的数据,
        //
        TraceMsg("$# bad , discard it.",1);
        return 0;
    }
    
    //服务器->单片机：$sethead$[用户名的长度][用户名][测量点编号]...#setend#[回车符][换行符]
    //服务器->单片机：$gainhead$[用户名的长度][用户名][测量点编号]#gainend#[回车符][换行符]
    //服务器->单片机：$timehead$[时间]#timeend#[回车符][换行符]
    
    char _send[150];   //发送串
    int _send_idx=0;   //发送串长度
    char _buffer[12];  //临时缓冲
    char _nameLen;     //用户名长度 
    
    int _idx=0;        //接收串的下标   
    int _ret=0;        //记录返回值
    int _tempInt=0;    //临时int
    char _tempChar1=0x00;   //临时char1
    char _tempChar2=0x00;   //临时char2
    char _tempChar3=0x00;   //临时char3
    int _tempIdx1=0;        //临时索引1 
    char * _set=NULL;  //接收串的内容部分指针
    
    if(_data[1]=='s')
    {
        TraceMsg("Sethead Data:",1);
        _idx=9;//跳过包头
        _nameLen= _data[_idx++] ;
        _idx += _nameLen; 
        
        if(Utility_Strncmp("00000000000",&_data[_idx],11))
        {//如果不是全0,就判断设备号是否一致
            Store_ReadDeviceNO(_buffer);
            if(Utility_Strncmp(_buffer,&_data[_idx],11))
            {//测量点编号不一样.
                TraceMsg("No this device's data ,discard. ",1);
                return -2;
            }
        }
        _idx += 11;//跳过设备编号 
        
        //开始复制包头部分,直接复制
        Utility_Strncpy(_send,_data,21+ _nameLen);
        _send_idx = 21 + _nameLen ;
        
        //
        // 开始解析设置命令.
        //  0         1 
        //  01234567890123456789
        //  BS*00011100011,00,00
        //  DL*A,0000,4096
        //  PL*I,0
        //  PD*00000011
        //  CS*A,00000011
        //  PR*I,100
        //  PM*I,6
        //  PV*I,0000000
        //  WM*0
        //   
        //  RP*PD CS PR PM PV WM BS DL PL
        // 
        //  进度:   RP未实现  
        
        //a.BS*OK(表示“参数设置成功”)
        //b.BS*BAD(表示“参数错误”)
        //c.BS*ERROR(表示“格式错误”)
        //d.BS*FAIL(表示“参数设置失败”)
        //e.UN*ERROR(表示“数据在传输过程中受到干扰”)
         
        _set=&_data[_idx];
        _ret=0;
        // _idx为设置参数的起始下标 
        // _send_idx 为下一个填写的回复的位置
         if(_set[0]=='B'&&_set[1]=='S')
        {
            TraceMsg("BS data ",1);
            if(_set[2]!='*'||_set[14]!=','||_set[17]!=',' || Utility_CheckDigital(_set,3,13) 
               || Utility_CheckDigital(_set,15,16) || Utility_CheckDigital(_set,18,19))
            {
                _ret=_DTU_PackErrorData("BS",&_send[_send_idx]);
                _send_idx +=_ret;
                DTU_SendData(_send,_send_idx);
                return 0;
            }
            
            if( _set[11] != '0' || _set[12] != '0' || _set[13] != '0' )
            {
                //_ret += Store_SetDeviceNO(&(_set[3]));
            } 
            if( _set[15] !='0' || _set[16] != '0' )
            {
                _tempInt= (_set[15]-'0')*10 + _set[16]-'0';
                if( _tempInt < 1 || _tempInt >16)
                {
                    _ret=_DTU_PackBadData("BS",&_send[_send_idx]);
                    _send_idx +=_ret;
                    DTU_SendData(_send,_send_idx);
                    return 0;
                }
                _ret +=Store_SetSaveTimeMode(&(_set[15]));
                //同时修正下一次保存时间
                Utility_CalculateNextSaveTimeBytes(_buffer);
                RTC_SetSaveTimeBytes(SAVETIME_ADDR,_buffer);
            }
            if( _set[18] != '0' || _set[19] != '0')
            {
                _tempInt = (_set[18]-'0')*10 + _set[19]-'0';
                if( _tempInt <=0 || _tempInt >16)
                { 
                    _ret=_DTU_PackBadData("BS",&_send[_send_idx]);
                    _send_idx +=_ret;
                    DTU_SendData(_send,_send_idx);
                    return 0;
                }
                _ret += Store_SetReportTimeMode(&(_set[18]));
                //同时修正下一次报告时间
                Utility_CalculateNextReportTimeBytes(_buffer);
                RTC_SetReportTimeBytes(REPORTTIME_ADDR,_buffer);
            }
            if(_ret<0)
            {
                _ret=_DTU_PackFailData("BS",&_send[_send_idx]);
                _send_idx +=_ret;
                DTU_SendData(_send,_send_idx);
                return 0;
            }
            else
            {
                _ret=_DTU_PackOKData("BS",&_send[_send_idx]);
                _send_idx +=_ret;
                DTU_SendData(_send,_send_idx);
                return 0;
            }
        }
        //  01234567890123456
        //  DL*A,0000,4096
        if(_set[0]=='D'&&_set[1]=='L')
        {
            if(_set[2]!='*' || _set[3]<'A'|| _set[3]>'H'|| _set[4]!=','||Utility_CheckDigital(_set,5,8)||_set[9]!=','
               ||Utility_CheckDigital(_set,10,13))
            {
                _ret=_DTU_PackErrorData("DL",&_send[_send_idx]);
                _send_idx +=_ret;
                DTU_SendData(_send,_send_idx);
                return 0;
            } 
            
            TraceMsg("Data limit Setting .",1); 
            //需要检查max是否大于min
            int _min=0; int _max=0; int _no=_set[3]-'A'+1;
            int _ret=0;
            if(_set[5]!='9' ||_set[6]!='9' ||_set[7]!='9' ||_set[8]!='9')
            {
                _min += (_set[5]-'0')*1000;
                _min += (_set[6]-'0')*100;
                _min += (_set[7]-'0')*10;
                _min += (_set[8]-'0');
            }
            else
            {//如果不对这个值设置,我们就取保存值
                if(Store_ReadDataMinInt(_no,&_min)<0) 
                    _min=0;//读不出来就算了.不检查max>min了
            }
        
            if(_set[10]!='9' ||_set[11]!='9' ||_set[12]!='9' ||_set[13]!='9')
            {
                _max += (_set[10]-'0')*1000;
                _max += (_set[11]-'0')*100;
                _max += (_set[12]-'0')*10;
                _max += (_set[13]-'0');
             }
             else
             {//如果不对这个值设置,我们就取保存值
                 if(Store_ReadDataMaxInt(_no,&_max)<0) 
                     _max=4096;//读不出来就算了.不检查max>min了
             }
             if(_min > _max)
             {
                 //返回错误信息.    
                 //组装完成,发送
                 _ret=_DTU_PackBadData("DL",&_send[_send_idx]);
                 _send_idx +=_ret;
                 DTU_SendData(_send,_send_idx);
                 return 0;
             }
             s_alert_flag[_no-1]=0;//报警标识要去掉
            
             //写回rom
             _ret += Store_SetDataMaxInt(_no,_max);
             _ret += Store_SetDataMinInt(_no,_min);
             if(_ret<0)
             {
                 //返回错误信息.    
                 _ret=_DTU_PackFailData("DL",&_send[_send_idx]);
                 _send_idx +=_ret;
                 DTU_SendData(_send,_send_idx);
                 return 0;
             }
             else
             {
                 //返回成功信息.    
                 _ret=_DTU_PackOKData("DL",&_send[_send_idx]);
                 _send_idx +=_ret;
                 DTU_SendData(_send,_send_idx);
                 return 0;
             }
             
        }
        //012345
        //PL*M,0
        // PL*M,0     成功返回 PL*OK   错误返回 PL*FAIL
        if(_set[0]=='P' && _set[1]=='L' )
        {
            if(_set[2]!='*'||_set[3]<'M' || _set[3]>'T'||_set[4]!=','|| ( _set[5]!='0'&&_set[5]!='1') )
            {
                _ret=_DTU_PackErrorData("PL",&_send[_send_idx]);
                _send_idx +=_ret;
                DTU_SendData(_send,_send_idx);
                return 0;
            }
            TraceMsg("Port level Setting .",1); 
            int _no = _set[3]-'M'+1;
            int _level = _set[5]-'0';
            int _ret = Sampler_IO_Level(_no,_level);
            //复制包头
            if(_ret<0)
            {
                _ret=_DTU_PackFailData("PL",&_send[_send_idx]);
                _send_idx +=_ret;
                DTU_SendData(_send,_send_idx);
                return 0;
            }
            else
            {   //  我们要发2条, 一个是OK, 一个是写数据库数据.
                //  为了让客户端那边 能在获得OK时就能获得到数据变化
                //  我们就先发数据 再发OK.
                //
                // 然后再发条只有这个端口数据 让上位机更新数据库
                //
                Utility_Strncpy(_send,"$datahead$$start$$",18);
    
                if(Store_ReadDeviceNO(&(_send[18]))<0)
                {
                    Utility_Strncpy(&_send[18],"00000000000",11);
                }
                _send[29] = '>';
                RTC_ReadTimeStr5_B(&_send[30]);
                _send[40] = '*';
                _send[41] = _set[3];
                _send[42] = _set[5];
                Utility_Strncpy(&_send[43],"##end##dataend#",15);
                DTU_SendData(_send,58);
                
                //发送OK
                //开始复制包头部分,直接复制
                Utility_Strncpy(_send,_data,21+ _nameLen);
                _send_idx = 21 + _nameLen ;
                
                _ret=_DTU_PackOKData("PL",&_send[_send_idx]);
                _send_idx +=_ret;
                DTU_SendData(_send,_send_idx);
                
                return 0;
            }
        }
        //  01234567890123
        //  PD*00000011
        if(_set[0]=='P' && _set[1]=='D')
        {
            if(_set[2]!='*'||Utility_CheckBinary(_set,3,10) )
            {
                _ret=_DTU_PackErrorData("PD",&_send[_send_idx]);
                _send_idx +=_ret;
                DTU_SendData(_send,_send_idx);
                return 0;
            }
            TraceMsg("Port direction Setting .",1); 
            _tempChar1=0x00;
            for(int i=3; i<11;++i)
            {
                _tempChar1>>=1;
                if(_set[i]=='0')
                {
                    _tempChar1 &= ~BIT7;//最高位清0
                }
                else
                {
                    _tempChar1 |= BIT7;//最高位置1
                }
            }
            _ret = Store_SetIoDirConfig(_tempChar1);
            if(_ret<0)
            {
                _ret=_DTU_PackFailData("PD",&_send[_send_idx]);
                _send_idx +=_ret;
                DTU_SendData(_send,_send_idx);
                return 0;
            }
            else
            {
                _ret=_DTU_PackOKData("PD",&_send[_send_idx]);
                _send_idx +=_ret;
                DTU_SendData(_send,_send_idx);
                return 0;
            }
        }
        //  01234567890123
        //  CS*A,00000011
        if(_set[0]=='C' && _set[1]=='S')
        {
            if( _set[2]!='*'|| (_set[3]!='A'&&_set[3]!='P'&&_set[3]!='I')|| _set[4]!=',' 
               ||Utility_CheckBinary(_set,5,12))
            {
                _ret=_DTU_PackErrorData("CS",&_send[_send_idx]);
                _send_idx +=_ret;
                DTU_SendData(_send,_send_idx);
                return 0;
            } 
            TraceMsg("selection Setting .",1); 
            if(_set[3]=='A')
            {
                for(int i=5;i<13;++i)
                {//CS*A,11000000#
                    _tempChar1 >>=1;
                    if(_set[i]!='0')
                        _tempChar1 |= BIT7;
                    else
                        _tempChar1 &= ~BIT7;
                }
                _ret = Store_SetAnalogSelect(_tempChar1);
                if(_ret<0)
                {
                    _ret=_DTU_PackFailData("CS",&_send[_send_idx]);
                    _send_idx +=_ret;
                    DTU_SendData(_send,_send_idx);
                    return 0;
                }
                else
                {
                    _ret=_DTU_PackOKData("CS",&_send[_send_idx]);
                    _send_idx +=_ret;
                    DTU_SendData(_send,_send_idx);
                    return 0;
                }
            }
            else if(_set[3]=='P')
            {
                _tempChar1=0x00;
                if(_set[5]=='0')
                    _tempChar1 &= ~BIT7;
                else
                    _tempChar1 |= BIT7;
                if(_set[6]=='0')
                    _tempChar1 &= ~BIT6;
                else
                    _tempChar1 |= BIT6;
                if(_set[7]=='0')
                    _tempChar1 &= ~BIT5;
                else
                    _tempChar1 |= BIT5;
                if(_set[8]=='0')
                    _tempChar1 &= ~BIT4;
                else
                    _tempChar1 |= BIT4;
                
                _ret =Store_SetPulseSelect(_tempChar1);
                if(_ret<0)
                {
                    _ret=_DTU_PackFailData("CS",&_send[_send_idx]);
                    _send_idx +=_ret;
                    DTU_SendData(_send,_send_idx);
                    return 0;
                }
                else
                {
                    _ret=_DTU_PackOKData("CS",&_send[_send_idx]);
                    _send_idx +=_ret;
                    DTU_SendData(_send,_send_idx);
                    return 0;
                }
            }
            else if(_set[3]=='I')
            {
                for(int i=5;i<13;++i)
                {//CS*A,11000000#
                    _tempChar1 >>=1;
                    if(_set[i]!='0')
                        _tempChar1 |= BIT7;
                    else
                        _tempChar1 &= ~BIT7;
                }
                _ret = Store_SetIoSelect(_tempChar1);
                if(_ret<0)
                {
                    _ret=_DTU_PackFailData("CS",&_send[_send_idx]);
                    _send_idx +=_ret;
                    DTU_SendData(_send,_send_idx);
                    return 0;
                }
                else
                {
                    _ret=_DTU_PackOKData("CS",&_send[_send_idx]);
                    _send_idx +=_ret;
                    DTU_SendData(_send,_send_idx);
                    return 0;
                }  
            }
        }
        //  0123456789
        //  PR*I,100
        if(_set[0]=='P' && _set[1]=='R')
        {
            if(_set[2]!='*'||  _set[3]<'I'||_set[3]>'L'||_set[4]!=','||Utility_CheckDigital(_set,5,7))
            {
                _ret=_DTU_PackErrorData("PR",&_send[_send_idx]);
                _send_idx +=_ret;
                DTU_SendData(_send,_send_idx);
                return 0;
            }
            TraceMsg("Pulse rate Setting .",1); 
            _idx=_set[3]-'I'+1;
            //  _tempChar1 为数值
            //  因为过255的会溢出, 所以我们先拿int 来装, 没超过的话.就OK.
            _tempInt = (_set[5]-'0')*100;
            _tempInt += (_set[6]-'0')*10;
            _tempInt += (_set[7]-'0');
            if(_tempInt>255 || _tempInt==0 )
            {
                _ret=_DTU_PackBadData("PR",&_send[_send_idx]);
                _send_idx +=_ret;
                DTU_SendData(_send,_send_idx);
                return 0;
            }
            _ret = Store_SetPulseRate(_idx, _tempInt);
            if(_ret<0)
            {
                _ret=_DTU_PackFailData("PR",&_send[_send_idx]);
                _send_idx +=_ret;
                DTU_SendData(_send,_send_idx);
                return 0;
            }
            else
            {
                //修改内存值
                g_pulse_rate[_idx-1] = _tempInt;
                
                _ret=_DTU_PackOKData("PR",&_send[_send_idx]);
                _send_idx +=_ret;
                DTU_SendData(_send,_send_idx);
                return 0;
            }  
        } 
        //  0123456
        //  PM*I,6
        if(_set[0]=='P' && _set[1]=='M')
        {
            if(_set[2]!='*' ||_set[3]<'I'|| _set[3]>'L'||_set[4]!=',')
            {
                _ret=_DTU_PackErrorData("PM",&_send[_send_idx]);
                _send_idx +=_ret;
                DTU_SendData(_send,_send_idx);
                return 0;
            }
            TraceMsg("Pulse range settting ",1);
            if(  _set[5]<'1' || _set[5] >'7')
            {
                _ret = _DTU_PackBadData("PM",&_send[_send_idx]);
                _send_idx += _ret;
                DTU_SendData(_send,_send_idx);
                return 0;
            }
            _idx= _set[3]-'I'+1;
            _ret = Store_SetPulseRange(_idx,_set[5]-'0'); 
            if(_ret<0)
            {
                _ret=_DTU_PackFailData("PM",&_send[_send_idx]);
                _send_idx +=_ret;
                DTU_SendData(_send,_send_idx);
                return 0;
            }
            else
            {
                //再修改内存值
                Store_ReadPulseRangeBytes(_idx,g_pulse_range[_idx-1]);
                
                _ret = _DTU_PackOKData("PM",&_send[_send_idx]);
                _send_idx +=_ret;
                DTU_SendData(_send,_send_idx);
                return 0;
            }
        }
        //  01234567890123456
        //  PV*I,1234567
        if(_set[0]=='P' && _set[1]=='V')
        {
            if(_set[2]!='*'||_set[3]<'I'||_set[3]>'L'||_set[4]!=','||Utility_CheckDigital(_set,5,11))
            {
                _ret=_DTU_PackErrorData("PV",&_send[_send_idx]);
                _send_idx +=_ret;
                DTU_SendData(_send,_send_idx);
                return 0;
            }
            TraceMsg("Pulse value setting. ",1);
            _idx = _set[3]-'I'+1;
            Utility_DecStr7ToBytes3(&_set[5],_buffer);
            
            if(Utility_BytesCompare3(_buffer,g_pulse_range[_idx-1])==1)
            {
                _ret = _DTU_PackBadData("PV",&_send[_send_idx]);
                _send_idx += _ret;
                DTU_SendData(_send,_send_idx);
                return 0;
            }
            RTC_SetPulseBytes(_idx,_buffer);
             
            
            
            _ret = _DTU_PackOKData("PV",&_send[_send_idx]);
            _send_idx +=_ret;
            DTU_SendData(_send,_send_idx);
            return 0;  
        }
        // 0123456
        // WM*1
        if(_set[0]=='W'&&_set[1]=='M')
        {
            if(_set[2]!='*'|| (_set[3]!='0'&& _set[3]!='1'))
            {
                _ret=_DTU_PackErrorData("WM",&_send[_send_idx]);
                _send_idx +=_ret;
                DTU_SendData(_send,_send_idx);
                return 0;
            }
            TraceMsg("Work mode setting . ",1);
            _ret=  Store_SetWorkMode(_set[3]);
            if(_ret<0)
            {
                 _ret = _DTU_PackFailData("WM",&_send[_send_idx]);
                _send_idx += _ret;
                DTU_SendData(_send,_send_idx);
                return 0;
            }
            else
            {
                _ret = _DTU_PackOKData("WM",&_send[_send_idx]);
                _send_idx += _ret;
                DTU_SendData(_send,_send_idx);
                if( g_work_mode != (_data[9]-'0') )
                {//如果模式不一样, 那么就重启. 
                    DTU_Close();//关闭DTU
                    System_Delayms(1000);
                    System_Reset();
                }
                return 0;
            }
        }
        if(_set[0]=='T'&&_set[1]=='M')
        {
            _dtu_request_time=1;
            DTU_SendData("$timehead$#timeend#",19);
            return 0;
        }
        //  01234567890123456
        //  RP*BS
        if(_set[0]=='R' && _set[1]=='P'&& _set[2]=='*')
        {// RP*PD CS PR PM PV WM BS DL PL
            
            //我们使用_send_idx记录长度, 用_tempIdx1用于发生错误时生成错误串
            if(_set[3]=='B'&&_set[4]=='S')
            {//返回RP*BS*01,01
                _ret=0;
                _tempIdx1=_send_idx;
                Utility_Strncpy(&_send[_send_idx],"RP*BS*",6);
                _send_idx+=6;
                _ret =  Store_ReadDeviceNO(&_send[_send_idx]);
                _send_idx +=11;
                _send[_send_idx++]=',';
                _ret += Store_ReadSaveTimeMode(&_send[_send_idx]);
                _send_idx += 2;
                _send[_send_idx++]=',';
                _ret += Store_ReadReportTimeMode(&_send[_send_idx]);
                _send_idx += 2;
                if(_ret<0)
                {
                    _ret = _DTU_PackFailData("RP",&_send[_tempIdx1]);
                    _tempIdx1 += _ret;
                    DTU_SendData(_send,_tempIdx1);
                    return 0;
                }
                else
                {//继续填写#setend#
                    Utility_Strncpy(&_send[_send_idx],"#setend#",8);
                    _send_idx+=8;
                    DTU_SendData(_send,_send_idx);
                    return 0;
                }
            }
            if(_set[3]=='D'&&_set[4]=='L')   
            {   //    0         1         2         3         4         5         6         7         8 
                //    012345678901234567890123456789012345678901234567890123456789012345678901234567890123
                //返回RP*DL*0000,4096,0000,4096,0000,4096,0000,4096,0000,4096,0000,4096,0000,4096,0000,4096
                _ret = 0;
                _tempIdx1 = _send_idx;
                Utility_Strncpy(&_send[_send_idx],"RP*DL*",6);
                _send_idx+=6;
                for(int i=1;i<9;++i)
                {
                    _ret+=Store_ReadDataMinStr(i,&_send[_send_idx]);
                    _send_idx+=4;
                    _send[_send_idx++]=',';
                    _ret+=Store_ReadDataMaxStr(i,&_send[_send_idx]);
                    _send_idx+=4;
                    _send[_send_idx++]=',';
                }
                if(_ret<0)
                {
                    _ret = _DTU_PackFailData("RP",&_send[_tempIdx1]);
                    _tempIdx1 += _ret;
                    DTU_SendData(_send,_tempIdx1);
                    return 0;
                }
                else
                {//去掉最后一个多余的,号,继续填写#setend#
                    --_send_idx;//去掉,号
                    Utility_Strncpy(&_send[_send_idx],"#setend#",8);
                    _send_idx+=8;
                    DTU_SendData(_send,_send_idx);
                    return 0;
                }
            }
            if(_set[3]=='P'&&_set[4]=='L')
            {//返回RP*PL*00000000
                _ret = 0;
                _tempIdx1 = _send_idx;
                _ret=Store_ReadIoLevelConfig(&_tempChar1);
                if(_ret<0)
                {
                    _ret = _DTU_PackFailData("RP",&_send[_tempIdx1]);
                    _tempIdx1 += _ret;
                    DTU_SendData(_send,_tempIdx1);
                    return 0;
                }
                //既然读出来了,就再配置一次,更确保运行正常.
                P5OUT = _tempChar1; //P5为IO口
                Utility_Strncpy(&_send[_send_idx],"RP*PL*",6);
                _send_idx+=6;
                for(int i=0;i<8;++i)
                {
                    if(_tempChar1&BIT0) 
                        _send[_send_idx ++] = '1'; 
                    else
                        _send[_send_idx ++] = '0';
                    _tempChar1>>=1;
                }
                //继续填写#setend#
                Utility_Strncpy(&_send[_send_idx],"#setend#",8);
                _send_idx+=8;
                DTU_SendData(_send,_send_idx);
                return 0;
            }
            if(_set[3]=='P'&&_set[4]=='D')
            {//返回RP*PD*00000000
                _ret = 0;
                _tempIdx1 = _send_idx;
                _ret=Store_ReadIoDirConfig(&_tempChar1);
                if(_ret<0)
                {
                    _ret = _DTU_PackFailData("RP",&_send[_tempIdx1]);
                    _tempIdx1 += _ret;
                    DTU_SendData(_send,_tempIdx1);
                    return 0;
                }
                //既然读出来了,就再配置一次,更确保运行正常.
                P5DIR = _tempChar1; //P5为IO口
                Utility_Strncpy(&_send[_send_idx],"RP*PD*",6);
                _send_idx+=6;
                for(int i=0;i<8;++i)
                {
                    if(_tempChar1&BIT0)
                        _send[_send_idx ++]='1';
                    else
                        _send[_send_idx ++]='0';
                    _tempChar1>>=1;
                }
                //继续填写#setend#
                Utility_Strncpy(&_send[_send_idx],"#setend#",8);
                _send_idx+=8;
                DTU_SendData(_send,_send_idx);
                return 0;
            }
            if(_set[3]=='P'&&_set[4]=='V')
            {//返回RP*PV*1234567,1234567,1234567,1234567
                _ret = 0;
                _tempIdx1 = _send_idx;
                Utility_Strncpy(&_send[_send_idx],"RP*PV*",6);
                _send_idx+=6; 
                _ret = RTC_ReadPulseBytes(1,_buffer);
                Utility_Bytes3ToDecStr7(_buffer,&_send[_send_idx]);
                _send_idx+=7;
                _send[_send_idx++]=',';
                _ret += RTC_ReadPulseBytes(2,_buffer);
                Utility_Bytes3ToDecStr7(_buffer,&_send[_send_idx]);
                _send_idx+=7;
                _send[_send_idx++]=',';
                _ret += RTC_ReadPulseBytes(3,_buffer);
                Utility_Bytes3ToDecStr7(_buffer,&_send[_send_idx]);
                _send_idx+=7;
                _send[_send_idx++]=',';
                _ret += RTC_ReadPulseBytes(4,_buffer);
                Utility_Bytes3ToDecStr7(_buffer,&_send[_send_idx]);
                _send_idx+=7;
                if(_ret<0)
                {
                    _ret = _DTU_PackFailData("RP",&_send[_tempIdx1]);
                    _tempIdx1 += _ret;
                    DTU_SendData(_send,_tempIdx1);
                    return 0;
                }
                //继续填写#sethead#
                Utility_Strncpy(&_send[_send_idx],"#setend#",8);
                _send_idx+=8;
                DTU_SendData(_send,_send_idx);
                return 0;
            }
            
            if(_set[3]=='C'&&_set[4]=='S')
            {//返回RP*CS*00000000,11110000,00000000
                _ret = 0;
                _tempIdx1 = _send_idx;
                _ret+=Store_ReadAnalogSelect(&_tempChar1);
                _ret+=Store_ReadPulseSelect(&_tempChar2);
                _ret+=Store_ReadIoSelect(&_tempChar3);
                if(_ret<0)
                {
                    _ret = _DTU_PackFailData("RP",&_send[_tempIdx1]);
                    _tempIdx1 += _ret;
                    DTU_SendData(_send,_tempIdx1);
                    return 0;
                }
                _send[_send_idx++]='R';
                _send[_send_idx++]='P';
                _send[_send_idx++]='*';
                _send[_send_idx++]='C';
                _send[_send_idx++]='S';
                _send[_send_idx++]='*';
                //A
                for(int i=0;i<8;++i)
                {
                    if(_tempChar1 & BIT0) 
                        _send[_send_idx++]='1';
                    else
                        _send[_send_idx++]='0'; 
                    _tempChar1 >>=1;
                }
                _send[_send_idx++]=',';//A之后的,号
                //P
                if(_tempChar2 & BIT7)
                    _send[_send_idx++]='1';
                else
                    _send[_send_idx++]='0'; 
                if(_tempChar2 & BIT6)
                    _send[_send_idx++]='1';
                else
                    _send[_send_idx++]='0'; 
                if(_tempChar2 & BIT5)
                    _send[_send_idx++]='1';
                else
                    _send[_send_idx++]='0';
                if(_tempChar2 & BIT4)
                    _send[_send_idx++]='1';
                else
                    _send[_send_idx++]='0';
                _send[_send_idx++]='0';
                _send[_send_idx++]='0';
                _send[_send_idx++]='0';
                _send[_send_idx++]='0';
                _send[_send_idx++]=',';
                //I
                for(int i=0;i<8;++i)
                {
                    if(_tempChar3 & BIT0)
                        _send[_send_idx++]='1';
                    else
                        _send[_send_idx++]='0';
                    _tempChar3 >>=1;
                }
                //继续填写#setend
                Utility_Strncpy(&_send[_send_idx],"#setend#",8);
                _send_idx+=8;
                DTU_SendData(_send,_send_idx);
                return 0; 
            }
            if(_set[3]=='P'&&_set[4]=='R')
            {//返回RP*PR*000,100,010,001
                _ret = 0;
                _tempIdx1 = _send_idx;
                Utility_Strncpy(&_send[_send_idx],"RP*PR*",6);
                _send_idx+=6;
                for(int i=1;i<=4;++i)
                {
                    _ret += Store_ReadPulseRate(i,& _tempChar1);
                    //再读一内存.
                    Store_ReadPulseRate(i,&(g_pulse_rate[i-1]));
                    
                    _send[_send_idx++]=_tempChar1/100+'0';
                    _tempChar1 %=100;
                    _send[_send_idx++]=_tempChar1/10+'0';
                    _send[_send_idx++]=_tempChar1%10+'0';
                    _send[_send_idx++]=',';
                }
                if(_ret<0)
                {
                    _ret = _DTU_PackFailData("RP",&_send[_tempIdx1]);
                    _tempIdx1 += _ret;
                    DTU_SendData(_send,_tempIdx1);
                    return 0;
                }
                //去掉多余的,号 填写#setend#
                --_send_idx;
                Utility_Strncpy(&_send[_send_idx],"#setend#",8);
                _send_idx+=8;
                DTU_SendData(_send,_send_idx);
                return 0;
            }
            if(_set[3]=='P'&&_set[4]=='M')
            {//返回PM*5,5,5,5
                _ret = 0;
                _tempIdx1 = _send_idx;
                Utility_Strncpy(&_send[_send_idx],"RP*PM*",6);
                _send_idx+=6; 
                for(int i=1;i<=4;++i)
                {
                    _ret +=Store_ReadPulseRange(i,&_tempChar1);
                    _send[_send_idx++] = _tempChar1 + '0';
                    _send[_send_idx++] =',';
                }
                if(_ret<0)
                {
                    _ret = _DTU_PackFailData("RP",&_send[_tempIdx1]);
                    _tempIdx1 += _ret;
                    DTU_SendData(_send,_tempIdx1);
                    return 0;
                }
                //去掉多余的,号 填写#setend#
                --_send_idx;
                Utility_Strncpy(&_send[_send_idx],"#setend#",8);
                _send_idx+=8;
                DTU_SendData(_send,_send_idx);
                return 0;
            } 
            if(_set[3]=='W'&&_set[4]=='M')
            {//返回WM*0
                _ret = 0;
                _tempIdx1 = _send_idx;
                Utility_Strncpy(&_send[_send_idx],"RP*WM*",6);
                _send_idx+=6; 
                _ret = Store_ReadWorkMode(&_tempChar1);
                if(_ret<0)
                {
                    _ret = _DTU_PackFailData("RP",&_send[_tempIdx1]);
                    _tempIdx1 += _ret;
                    DTU_SendData(_send,_tempIdx1);
                    return 0;
                }
                _send[_send_idx++]=_tempChar1;
                //填写#setend#
                Utility_Strncpy(&_send[_send_idx],"#setend#",8);
                _send_idx+=8;
                DTU_SendData(_send,_send_idx);
                return 0;
            }
            
            _ret=_DTU_PackBadData("RP",&_send[_send_idx]);
            _send_idx +=_ret;
            DTU_SendData(_send,_send_idx);
            return 0;
        }
        TraceMsg("Bad Setting Data .",1);
        //其他无关数据 不管它.
        _ret=_DTU_PackErrorData("UN",&_send[_send_idx]);
        _send_idx +=_ret;
        DTU_SendData(_send,_send_idx); 
        return 0;
    }     
    //$gainhead$[用户名的长度][用户名][测量点编号]#gainend#
    //$gainhead$112300000000111
    if(_data[1]=='g')
    {
        TraceMsg("GainHead Data .",1);
        _idx=10;//跳过包头
        _nameLen= _data[_idx++] ;
        TraceMsg("nameLen :",0);
        TraceInt4(_nameLen,1);
        _idx += _nameLen;
        //比较device_no
        Store_ReadDeviceNO(_buffer);
        if(Utility_Strncmp(_buffer,&_data[_idx],11))
        {//测量点编号不一样.
            TraceMsg("No this device's data ,discard. ",1);
            return -2;
        }
        //向服务器发送40条数据
        //头部
        Utility_Strncpy(_send,"$datahead$$start$$",18);
        if(Store_ReadDeviceNO(&(_send[18]))<0)
        {
         Utility_Strncpy(&_send[18],"00000000000",11);
        }
        _send[29] = '>';
        int _startIdx=DATA_MIN_IDX;
        int _ret=0;
        while(_startIdx<=DATA_MAX_IDX)
        { 
            TraceMsg("Idx is :",0);
            TraceInt4(_startIdx,1);
            _ret = Store_ReadDataItem(_startIdx,&_send[30],1);
            if(_ret<0)
            {//读不出来 就不读啦.结束.
                break;
            }
            /*
            if(_ret==1)
            {
                //继续下一个位置
                if(_startIdx >= DATA_MAX_IDX)  
                    break; //到最后一个下标就结束了.
                else   
                    ++ _startIdx;//下一数据
                continue;
            }
            */  
            //如果是正常数据
            _ret +=30;
            Utility_Strncpy(&_send[_ret],"##end##dataend#",15);
            _ret+=15;
            DTU_SendData(_send,_ret);
            
            //继续下一个位置
            if(_startIdx >= DATA_MAX_IDX)  
                break; //到最后一个下标就结束了.
            else   
                ++ _startIdx;//下一数据 
        }
        // 因为实时数据库会因为我们 发补调数据而 变旧, 我们再补发一条最新的数据,让实时库数据最新.
        char _endIdx=0;
        if(RTC_ReadEndIdx(&_endIdx)>=0)
        {//无法找到最新的数据? 那就不管了
            
            //要退一索引才是
            if(_endIdx==DATA_MIN_IDX)
                _endIdx=DATA_MAX_IDX;
            else
                --_endIdx;
               //读取发送
            _ret = Store_ReadDataItem(_endIdx,&_send[30],1);
            if(_ret>=0)
            {
                _ret +=30;
                Utility_Strncpy(&_send[_ret],"##end##dataend#",15);
                _ret+=15;
                DTU_SendData(_send,_ret);
            }   
        }
        //然后发送gainhead  OK  gainend 
        
        TraceMsg("GainHead Proc Over .",1);
        //复制包头数据//发送OK数据
        Utility_Strncpy(_send,_data,22 + _nameLen);      
        _idx=22+_nameLen;
        Utility_Strncpy(&_send[_idx],"OK#gainend#",11);
        DTU_SendData(_send,33+_nameLen); 
        return 0;
        
    }
    //服务器->单片机：$timehead$[时间]#timeend#[回车符][换行符]
    //012345678901234567890
    //2009/10/14/10:00:00
    //该指令不回复任何东西.
    if(_data[1]=='t'&& !Utility_CheckDigital(_data,10,13)&&_data[14]=='/'&&_data[17]=='/'&&
       _data[20]=='/'&&_data[23]==':'&&_data[26]==':')
    {
        TraceMsg("TimeHead data .",1);
        if(_dtu_request_time)
        {
            TraceMsg("setting time .",1);
            _dtu_request_time=0; 
            //检查时间有效性
            int _temp = (_data[15]-'0')*10+_data[16]-'0';
            if(_temp==0 || _temp>12)
            { TraceMsg("month is bad  .",1);   return 0;}
            _temp= (_data[18]-'0')*10 + _data[19]-'0';
            if(_temp==0 || _temp>31)
            { TraceMsg("date is bad  .",1);   return 0;}
            _temp= (_data[21]-'0')*10 + _data[22]-'0';
            if(_temp>23)
            { TraceMsg("hour is bad  .",1);   return 0;}
            _temp= (_data[24]-'0')*10+_data[25]-'0';
            if(_temp>59)
            { TraceMsg("minute is bad  .",1);   return 0;}
            _temp = (_data[27]-'0')*10+_data[28]-'0';
            if(_temp>59)
            { TraceMsg("second is bad  .",1);   return 0;}
            //时间有效,那么就设置
            RTC_SetTimeStr6_A(&_data[12]);
            RTC_ReadTimeBytes5(g_rtc_nowTime); //重新读出时间
            //重新设置检查时间
            Utility_CalculateNextCheckTimeBytes(_buffer);
            RTC_SetCheckTimeBytes(CHECKTIME_ADDR,_buffer);
            //重新设置保存时间
            Utility_CalculateNextSaveTimeBytes(_buffer);
            RTC_SetSaveTimeBytes(SAVETIME_ADDR,_buffer);
            //重新设置报告时间
            Utility_CalculateNextReportTimeBytes(_buffer);
            RTC_SetReportTimeBytes(REPORTTIME_ADDR,_buffer);   
            main_time_error =0;
        }
        
        return 0; 
    }
    TraceMsg("ERROR data .",1);
    //什么都不是 就不管
    return 0;
}

//
//  接受到数据就传递给处理函数.
//  等待一定时间  
//  1 30s  2 60s 3 90s 4 120s 
//
int  DTU_Waiting_Process(int _type)
{
    int _waits=100;//30秒
    int _repeats;
    int _dataLen=0;
    char _data[UART0_MAXBUFFLEN];
    switch(_type)
    {
      case 1:
        _waits=10;
        break;
      case 2:
        _waits=20;//1分钟
        break;
      case 3:
        _waits=30;//1.5分钟
        break;
      case 4:
        _waits=40;//2分钟
        break;
      default:
        _waits=10;//30s
        break;
    }
    while(1)
    {//只要一直有消息 就会一直处理,
        //没有消息超时  就会退出
        _repeats=0;
        while(UART0_RecvLineWait(_data,UART0_MAXBUFFLEN,&_dataLen)<0)
        {
            if( _repeats > _waits)    
            {
                return 0;
            }
            ++ _repeats;
        }
        DTU_ProcRecvData(_data,_dataLen);
    }
}

//  0123456789012345678901234567890123456789
//  $alerthead$2:A=1278;G=1400;#alertend#
//
int  DTU_Alert_Process()
{
    TraceMsg("Alert!",1);
    char _send[UART0_MAXBUFFLEN];
    int _ret=0;
    Utility_Strncpy(_send,"$alerthead$",11);
    //复制设备号
    Store_ReadDeviceNO(&_send[11]);
    _ret = Sampler_DTU_ReadAlertString(&_send[22]);
    _ret +=22;
    //填写尾部
    Utility_Strncpy(&_send[_ret],"#alertend#",10);
    _ret +=10;
    DTU_SendData(_send,_ret);
    return 0;
}

int  DTU_SendData( char * _data, char _len)
{
    UART0_Send(_data,_len,0);//
    TraceStr("DTU_Send:",9,0);
    TraceStr(_data,_len,1);
    System_Delayms(2000);//等待一定的延时.避免粘包.
    return 0;
}
