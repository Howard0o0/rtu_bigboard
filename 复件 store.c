//////////////////////////////////////////////////////
//     文件名: store.c
//   文件版本: 1.0.0
//   创建时间: 09年11月30日
//   更新内容:  
//       作者: 林智
//       附注: 
//
//       1. 数据组织格式.
//       数据条:
//           校验字节(1) + 时间串(10) + 模拟值条(8*2) + 脉冲值条(4*3) + 开关量(1) 
//           保存数据不需要类型码
//           脉冲值  我们 保存 3字节.
//       时间串:
//           年年月月日日时时分分
//       模拟值条: (类型码 A ~ H)
//           类型码(1) + 数值(2字节int类型)
//       脉冲值条: (类型码 I ~ L)
//           类型码(1) + 数值(2字节int类型)
//        
//       2. 上报格式
//       $00000000000>0909091230*A1234B1234C1234D1234E1234F1234G1234H1234I000000J000000K000000L000000
//       M1N1O1P1Q1R1S1T1#
//////////////////////////////////////////////////////
 
#include <msp430x16x.h>

#include "rom.h"
#include "flash.h"
#include "store.h"
#include "common.h" 
#include "rtc.h" 


////////////////////////////////////////
//  ROM区域分配  地址为int类型,11位 
//  起始地址: 0x0000  最高地址: 0x07FF 
////////////////////////////////////////


//    0x0000 ~ 0x0035   设备基本配置
//    0x0036 ~ 0x0079   各系统配置
//    0x00A0 ~ 0x07FF   数据区 

#define           BASE_ADDR    0x0000
 

//设置完整性字节               1字节,  0x0000  
#define         CONFIG_ADDR    BASE_ADDR 

//设备号                       11字节, 0x0001 ~ 0x000B
#define       DEVICENO_ADDR    (BASE_ADDR + 1 )
#define       DEVICENO_LEN     11
//密码                         4字节,  0x000C ~ 0x000F   
#define       PASSWORD_ADDR    (BASE_ADDR + 12)
#define       PASSWORD_LEN     4
//报告 时间间隔模式            2字节,  0x0010 ~ 0x0011
#define REPORTTIME_MODE_ADDR   (BASE_ADDR + 16)
#define REPORTTIME_MODE_LEN    2 
//保存 时间间隔模式            2字节,  0x0012 ~ 0x0013
#define   SAVETIME_MODE_ADDR   (BASE_ADDR + 18)
#define   SAVETIME_MODE_LEN    2 


//数据上下限  数据的基地址  
#define DATA_LIMIT_BASE_ADDR   (BASE_ADDR + 20)
#define DATA_MAX_BASE_ADDR     DATA_LIMIT_BASE_ADDR
//                                    0x0015
#define DATA_MIN_BASE_ADDR     (DATA_LIMIT_BASE_ADDR + 2 )

#define     DATA_LIMIT_LEN     2
//数据1报警上限                2字节,  0x0014~0x0015  
#define     DATA1_MAX_ADDR     DATA_MAX_BASE_ADDR
//数据1报警下限                2字节,  0x0016~0x0017  
#define     DATA1_MIN_ADDR     DATA_MIN_BASE_ADDR
//数据2报警上限                2字节,  0x0018~0x0019  
#define     DATA2_MAX_ADDR     (DATA_MAX_BASE_ADDR+4)
//数据2报警下限                2字节,  0x001A~0x001B  
#define     DATA2_MIN_ADDR     (DATA_MIN_BASE_ADDR+4)
//数据3报警上限                2字节,  0x001C~0x001D  
#define     DATA3_MAX_ADDR     (DATA_MAX_BASE_ADDR+8)
//数据3报警下限                2字节,  0x001E~0x001F  
#define     DATA3_MIN_ADDR     (DATA_MIN_BASE_ADDR+8)
//数据4报警上限                2字节,  0x0020~0x0021  
#define     DATA4_MAX_ADDR     (DATA_MAX_BASE_ADDR+12)
//数据4报警下限                2字节,  0x0022~0x0023  
#define     DATA4_MIN_ADDR     (DATA_MIN_BASE_ADDR+12)
//数据5报警上限                2字节,  0x0024~0x0025  
#define     DATA5_MAX_ADDR     (DATA_MAX_BASE_ADDR+16)
//数据5报警下限                2字节,  0x0026~0x0027  
#define     DATA5_MIN_ADDR     (DATA_MIN_BASE_ADDR+16)
//数据6报警上限                2字节,  0x0028~0x0029  
#define     DATA6_MAX_ADDR     (DATA_MAX_BASE_ADDR+20)
//数据6报警下限                2字节,  0x002A~0x002B  
#define     DATA6_MIN_ADDR     (DATA_MIN_BASE_ADDR+20)
//数据7报警上限                2字节,  0x002C~0x002D  
#define     DATA7_MAX_ADDR     (DATA_MAX_BASE_ADDR+24)
//数据7报警下限                2字节,  0x002E~0x002F  
#define     DATA7_MIN_ADDR     (DATA_MIN_BASE_ADDR+24)
//数据8报警上限                2字节,  0x0030~0x0031  
#define     DATA8_MAX_ADDR     (DATA_MAX_BASE_ADDR+28)
//数据8报警下限                2字节,  0x0032~0x0033  
#define     DATA8_MIN_ADDR     (DATA_MIN_BASE_ADDR+28)

//通道配置                     3字节
#define CHANNEL_CONFIG_ADDR    (DATA_LIMIT_BASE_ADDR + 32 )
//                             0x0034
#define     ANALOG_SEL_ADDR    CHANNEL_CONFIG_ADDR
//                             0x0035  低4位忽略
#define     PULSE_SEL_ADDR     (CHANNEL_CONFIG_ADDR + 1)
//                             0x0036
#define     ONOFF_SEL_ADDR     (CHANNEL_CONFIG_ADDR + 2)
//                             0x0037
#define     WORK_MODE_ADDR     (CHANNEL_CONFIG_ADDR + 3)
//                             0x0038
#define     SYSTEM_TYPE_ADDR   (CHANNEL_CONFIG_ADDR + 4) 
//                             0x0039
#define PULSE_RATE_BASE_ADDR   (CHANNEL_CONFIG_ADDR + 5)
#define     PULSE1_RATE_ADDR   PULSE_RATE_BASE_ADDR
//                             0x003A 
#define     PULSE2_RATE_ADDR   (CHANNEL_CONFIG_ADDR + 6)
//                             0x003B
#define     PULSE3_RATE_ADDR   (CHANNEL_CONFIG_ADDR + 7)
//                             0x003C
#define     PULSE4_RATE_ADDR   (CHANNEL_CONFIG_ADDR + 8)

 
//  以几个9来表示 脉冲表的量程

//                             0x003D 
#define PULSE_RANGE_BASE_ADDR  (CHANNEL_CONFIG_ADDR + 9)
#define   PULSE1_RANGE_ADDR    PULSE_RANGE_BASE_ADDR    
//                             0x003E
#define   PULSE2_RANGE_ADDR    (PULSE_RANGE_BASE_ADDR + 1)
//                             0x003F   
#define   PULSE3_RANGE_ADDR    (PULSE_RANGE_BASE_ADDR + 2)
//                             0x0040   
#define   PULSE4_RANGE_ADDR    (PULSE_RANGE_BASE_ADDR + 3)

//                             0x0041
#define      IO_DIR_CFG_ADDR   (CHANNEL_CONFIG_ADDR + 13)
//                             0x0042
#define    IO_LEVEL_CFG_ADDR   (CHANNEL_CONFIG_ADDR + 14)

#define     Config_END_ADDR    0x004F


//////////////////////////////////////
//
//  GSM 系统的分配
//
///////////////////////////////////////
//                                     0x0050
#define     GSM_START_ADDR             0x0050


#define     GSM_CENTERPHONE_BASE_ADDR  GSM_START_ADDR
#define     GSM_CENTER_PHONE_LEN       11

//                                     0x0050    
//   数据中心1                         11字节,   0x0050 -5A
#define     GSM_CENTER1_PHONE_ADDR     ( GSM_CENTERPHONE_BASE_ADDR )
//   数据中心2                         11字节,   0x005B 
#define     GSM_CENTER2_PHONE_ADDR     ( GSM_CENTERPHONE_BASE_ADDR + 11)
//   数据中心3                         11字节,   0x0066
#define     GSM_CENTER3_PHONE_ADDR     ( GSM_CENTERPHONE_BASE_ADDR + 22)
//   数据中心4                         11字节,   0x0071
#define     GSM_CENTER4_PHONE_ADDR     ( GSM_CENTERPHONE_BASE_ADDR + 33)
//   配置中心                          11字节,   0x007C
#define      GSM_CONFIG_PHONE_ADDR     ( GSM_CENTERPHONE_BASE_ADDR + 44)
//   负责人电话                        11字节,   0x0087
#define      GSM_LEADER_PHONE_ADDR     ( GSM_CENTERPHONE_BASE_ADDR + 55) 
//                                     0x0091  
#define     GSM_CENTERPHONE_END_ADDR   ( GSM_CENTERPHONE_BASE_ADDR + 65 )



///////////////////////////////////////
//
//  DTU 系统的分配 
//
///////////////////////////////////////

//                                     0x0050 
#define     DTU_START_ADDR             0x0050
#define     DTU_CENTERIP_BASE_ADDR     DTU_START_ADDR
#define     CENTER_IP_LEN              6      
//   数据中心1                         6字节,  0x0050 ~  
#define     CENTER1_IP_ADDR           (DTU_CENTERIP_BASE_ADDR)
//   数据中心2                         6字节,  0x0056 ~  
#define     CENTER2_IP_ADDR            (DTU_CENTERIP_BASE_ADDR + 6)
//   数据中心3                         6字节,  0x005B ~  
#define     CENTER3_IP_ADDR            (DTU_CENTERIP_BASE_ADDR + 12)
//   数据中心4                         6字节,  0x0050 ~  
#define     CENTER4_IP_ADDR            (DTU_CENTERIP_BASE_ADDR + 18)
//   数据中心5                         6字节,  0x0055 ~  
#define     CENTER5_IP_ADDR            (DTU_CENTERIP_BASE_ADDR + 24)


//////////////////////////////////////
//
//  485 系统的分配 
//
///////////////////////////////////////


//数据区
//                                    0x00A0
#define     DATA_START_ADDR           0x00A0
#define     DATA_ITEM_LEN             40
#define     DATA_ITEM_MAX             DATA_MAX_IDX
//#define     DATA_END_ADDR        
//( DATA_START_ADDR + DATA_ITEM_MAX * DATA_ITEM_LEN)

//初始化
void Store_Init()
{
    ROM_Init();
} 
//  读出 4096这样的字串
int Store_ReadDataMaxStr(int _index ,char * _dest)
{
    if( _index < 1 || _index >8)
        return -2;
    int _addr = DATA_MAX_BASE_ADDR + ( _index - 1 ) * DATA_LIMIT_LEN * 2;
    int _repeats=0;char _temp[2]; 
    while(ROM_ReadBytes( _addr , _temp,DATA_LIMIT_LEN )!=0)
    {
        if(_repeats>2)
        {
            return -1;
        }
        ++_repeats;
    }
    unsigned int _value=(((unsigned int) _temp[0])<<8); _value += _temp[1];
    Utility_UintToStr4(_value,_dest);
    return 0;
}

int Store_ReadDataMaxInt(int _index, int * _pDestInt)
{
    if( _index < 1 || _index >8)
        return -2;
    int _addr = DATA_MAX_BASE_ADDR + ( _index - 1 ) * DATA_LIMIT_LEN * 2;
    int _repeats=0; char _temp[2];
    while(ROM_ReadBytes(_addr,_temp, DATA_LIMIT_LEN )!=0)
    {
        if( _repeats > 2 )
        { 
            return -1;
        }
        ++ _repeats;
    }//转化为int
    (*_pDestInt)= (((unsigned int)_temp[0])<<8 ) ;(*_pDestInt)+=_temp[1];//高低位字节 
    return 0;
}


int Store_SetDataMaxInt(int _index, const int _max)
{
    if( _max > 4096 || _max < 0 )
        return -2;
    if( _index < 1 || _index > 8 )
        return -2;
    int _addr = DATA_MAX_BASE_ADDR + ( _index - 1 ) * DATA_LIMIT_LEN * 2 ;
    int _repeats=0; char _temp[2];
    _temp[0]= (_max>>8); _temp[1]=_max&0x00FF; 
    while(ROM_WriteBytes(_addr,_temp, DATA_LIMIT_LEN )!=0)
    {
        if(_repeats>2)
        {
            return -1;
        }
        ++ _repeats;
    }
    return 0;
}

//4字节字符数字
int Store_ReadDataMinStr(int _index, char * _dest)
{
    if( _index < 1  || _index > 8 )
        return -2;
    int _addr = DATA_MIN_BASE_ADDR + ( _index - 1 ) * DATA_LIMIT_LEN * 2;
    int _repeats=0;char _temp[2]; 
    while(ROM_ReadBytes(_addr, _temp,DATA_LIMIT_LEN )!=0)
    {
        if(_repeats>2)
        { 
            return -1;
        }
        ++_repeats;
    }
    unsigned int _value=(((unsigned int)_temp[0])<<8); _value += _temp[1];
    Utility_UintToStr4(_value,_dest);  
    return 0;
}


int Store_ReadDataMinInt(int _index, int * _pDestInt)
{
    if( _index < 1  || _index > 8 )
        return -2;
    int _addr = DATA_MIN_BASE_ADDR + ( _index - 1 ) * DATA_LIMIT_LEN * 2;
    int _repeats=0;char _temp[2];
    while(ROM_ReadBytes(_addr, _temp, DATA_LIMIT_LEN )!=0)
    {
        if(_repeats>2)
        {
            return -1;
        }
        ++ _repeats;
    }//转化为Int
    (*_pDestInt)=((unsigned int)_temp[0])<<8;  (*_pDestInt) += _temp[1];//高低位字节 
    return 0;
}

//设置格式为4068
int Store_SetDataMinInt(int _index, const int _min)
{
    if( _index < 1  || _index > 8 )
        return -2;
    int _addr = DATA_MIN_BASE_ADDR + ( _index - 1 ) * DATA_LIMIT_LEN * 2;
    int _repeats=0;char _temp[2];
    _temp[0]=(_min>>8); _temp[1]=_min & 0x00FF ; //低位字节  
    while(ROM_WriteBytes(_addr,_temp, DATA_LIMIT_LEN )!=0)
    {
        if(_repeats>2)
        {
            return -1;
        }
        ++_repeats;
    }
    return 0;
}


int Store_ReadConfig(char * _dest)//  1个字节
{ 
    int _repeats=0;
    while(ROM_ReadByte(CONFIG_ADDR,_dest)!=0)
    {
        if(_repeats>2)
        { 
            return -1;
        }
        ++ _repeats;
    } 
    return 0;
}


int Store_SetConfig(char _config)
{
     int _repeats=0;
     while(ROM_WriteByte(CONFIG_ADDR,_config)!=0)
     {
         if(_repeats>2)
         { 
             return -1;
         }
         ++_repeats;
     } 
     return 0;
} 
// 11个字符型字节
int Store_ReadDeviceNO(char *_dest)
{
  int _repeats=0;
  while(ROM_ReadBytes(DEVICENO_ADDR,_dest,DEVICENO_LEN )!=0)
  {
    if(_repeats>2)
    {  
      return -1;
    }
    ++_repeats;
  }
  return 0;
}
// 3个字符型字节
int Store_SetDeviceNO(const char * _src)
{
  int _repeats=0;
  while(ROM_WriteBytes(DEVICENO_ADDR, _src,DEVICENO_LEN )!=0)
  {
    if(_repeats>2)
    { 
      return -1;
    }
    ++_repeats;
  }  
  return 0;  
}
// 4个字符型字节
int Store_ReadPassword(char *_dest)
{ 
  int _repeats=0;
  while(ROM_ReadBytes(PASSWORD_ADDR,_dest,PASSWORD_LEN )!=0)
  {
    if(_repeats>2)
    {  
      return -1;
    }
    ++_repeats;
  }
  return 0;  
} 


// 4个字符型字节
int Store_SetPassword(const char * _src)
{
    int _repeats=0;
    while(ROM_WriteBytes(PASSWORD_ADDR,_src, PASSWORD_LEN )!=0)
    {
        if(_repeats>2)
        { 
            return -1;
        }
        ++_repeats;
    } 
    return 0;  
} 


//1个字符型字节
int  Store_ReadWorkMode(char * _dest)
{
    int _repeats=0;
    while(ROM_ReadByte(WORK_MODE_ADDR , _dest)<0)
    {
        if(_repeats>2)
            return -1;
        ++ _repeats;

    }
    return 0;  
}

// '1'  '0'
int  Store_SetWorkMode(char _src)
{
    int _repeats=0; 
    while(ROM_WriteByte(WORK_MODE_ADDR , _src)<0)
    {
        
        if( _repeats >2)
            return -1;
        ++ _repeats;
         
    }
    return 0; 
}
int  Store_ReadSystemType(char * _dest)
{
    int _repeats=0;
    while(ROM_ReadByte(SYSTEM_TYPE_ADDR, _dest)<0)
    {
        if( _repeats >2)
            return -1;
        ++ _repeats;
    }
    return 0;
}
int  Store_SetSystemType(char _src)
{
    int  _repeats=0;
    while(ROM_WriteByte(SYSTEM_TYPE_ADDR, _src)<0)
    {
        if( _repeats<2)
            return -1;
        ++ _repeats;
    }
    return 0;
}

int  Store_ReadAnalogSelect(char * _dest)
{
    int _repeats=0;
    while(ROM_ReadByte(ANALOG_SEL_ADDR, _dest)<0)
    {
        if( _repeats >2)
            return -1;
        ++ _repeats;
    }
    return 0;
}
int  Store_SetAnalogSelectStr(const char * _src)
{
    char _temp=0x00;
    //由_src形成一个char写入
    for(int i=7;i>=0;--i)
    {
        //下一位;
        _temp<<=1;
        if(_src[i]=='0')
            _temp &= 0xFE;//将最后一位清0;
        else
            _temp |= 0x01;//将最后一位置1;
        
    } 
    if(Store_SetAnalogSelect(_temp)<0)
        return -1;
    return 0;
}
int  Store_SetAnalogSelect(char _src)
{
    int  _repeats=0;
    while(ROM_WriteByte(ANALOG_SEL_ADDR, _src)<0)
    {
        if( _repeats<2)
            return -1;
        ++ _repeats;
    }
    return 0;
}

int  Store_ReadPulseSelect(char * _dest)
{
    int _repeats=0;
    while(ROM_ReadByte(PULSE_SEL_ADDR, _dest)<0)
    { 
        if( _repeats >2)
            return -1;
        ++ _repeats;
    }
    return 0;
}

int  Store_SetPulseSelectStr(const char * _src)
{
    char _temp=0x00;
    //由_src形成一个char写入
    for(int i=0;i<4;++i)
    {
        
        
        if(_src[i]=='0')
            _temp &= 0xF7;//将倒数第4位清0;
        else
            _temp |= 0x08;//将倒数第4位置1;
        
        //下一位;
        _temp<<=1;
    }
    //前4位对应的为选择
    if(Store_SetPulseSelect(_temp)<0)
        return -1;
    return 0;
}

int  Store_SetPulseSelect(char _src)
{
    int  _repeats=0;
    while(ROM_WriteByte(PULSE_SEL_ADDR, _src)<0)
    {
        if( _repeats<2)
            return -1;
        ++ _repeats;
    }
    return 0;
}

int  Store_ReadIoSelect(char * _dest)
{
    int _repeats=0;
    while(ROM_ReadByte(ONOFF_SEL_ADDR, _dest)<0)
    {
        if( _repeats >2)
            return -1;
        ++ _repeats;
    }
    return 0;
}
int  Store_SetIoSelect(char _src)
{
    int  _repeats=0;
    while(ROM_WriteByte(ONOFF_SEL_ADDR, _src)<0)
    {
        if( _repeats<2)
            return -1;
        ++ _repeats;
    }
    return 0;
}
int  Store_SetIoSelectStr(const char * _src)
{
    char _temp=0x00;
    //由_src形成一个char写入
    for(int i=7;i>=0;--i)
    {
        //下一位;
        _temp<<=1;
        
        if(_src[i]=='0')
            _temp &= 0xFE;//将最后一位清0;
        else
            _temp |= 0x01;//将最后一位置1;
        
    } 
    if(Store_SetIoSelect(_temp)<0)
        return -1;
    return 0;
}



int  Store_ReadPulseRate(int _index, char * _dest)
{
    if( _index < 1 || _index>4)
        return -2;
    int _addr = PULSE_RATE_BASE_ADDR + (_index-1);
    int _repeats=0;
    while(ROM_ReadByte( _addr , _dest)<0)
    {
        if( _repeats >2)
            return -1;
        ++ _repeats;
    }
    return 0; 
}

int  Store_SetPulseRate(int _index, char _src)
{
    if( _index < 1 || _index>4)
        return -2;
    int _addr = PULSE_RATE_BASE_ADDR + (_index-1); 
    int  _repeats=0;
    while(ROM_WriteByte( _addr , _src)<0)
    {
        if( _repeats<2)
            return -1;
        ++ _repeats;
    }
    return 0;
}


int  Store_ReadPulseRange(int _index, char *_dest)
{//读出9的个数  2进制
    if( _index < 1 || _index>4)
        return -2;
    int _addr = PULSE_RANGE_BASE_ADDR + (_index-1);
    int _repeats=0;
    while(ROM_ReadByte(_addr,_dest)<0)
    {
        if(_repeats>2)
            return -1;
        ++ _repeats;
    }
    return 0;
}
int  Store_SetPulseRange(int _index, char  _src)
{//写入 9的个数 2进制
    if( _index < 1 || _index>4)
        return -2;
    int _addr = PULSE_RANGE_BASE_ADDR + (_index-1); 
    int  _repeats=0;
    while(ROM_WriteByte( _addr , _src)<0)
    {
        if( _repeats<2)
            return -1;
        ++ _repeats;
    }
    return 0;
}
int  Store_ReadPulseRangeBytes(int _index, char *_dest)
{//读出 对应的 3个字节
    if( _index < 1|| _index>4)
        return -2;
    char _temp=0x00;
    int _addr = PULSE_RANGE_BASE_ADDR + (_index-1);
    int _repeats=0;
    while(ROM_ReadByte( _addr , &_temp)<0)
    {
        if( _repeats >2)
            return -1;
        ++ _repeats;
    }
    switch(_temp)
    {
      case 0://0个9,用于错误或者未设置的时候.
        _dest[0]=0x00;_dest[1]=0x00;_dest[2]=0x00;
        break;
      case 1:
        _dest[0]=0x00;_dest[1]=0x00;_dest[2]=0x09;
        break;
      case 2:
        _dest[0]=0x00;_dest[1]=0x00;_dest[2]=0x63;
        break;
      case 3:
        _dest[0]=0x00;_dest[1]=0x03;_dest[2]=0xE7;
        break;
      case 4:
        _dest[0]=0x00;_dest[1]=0x27;_dest[2]=0x0F;
        break;
      case 5:
        _dest[0]=0x01;_dest[1]=0x86;_dest[2]=0x9F;
        break;
      case 6:
        _dest[0]=0x0F;_dest[1]=0x42;_dest[2]=0x3F;
        break;
      case 7:
        _dest[0]=0x98;_dest[1]=0x96;_dest[2]=0x7F;
        break;
      default:
        //错误的9的个数,我们重置为0
        Store_SetPulseRange(_index,0);
        _dest[0]=0x00;_dest[1]=0x00;_dest[2]=0x00;
        break;
    }
    return 0;
}

//8个字符数据
int Store_ReadIoDirConfigStr(char * _dest)
{
    char _temp;
    if(Store_ReadIoDirConfig(&_temp)<0)
        return -1;
    for(int i=0;i<8;++i)
    {
        if(_temp & 0x01)
            _dest[i]='1';
        else
            _dest[i]='0';
        _temp >>= 1;
    }
    return 0;
}
int  Store_ReadIoDirConfig(char * _dest)
{
    int _repeats=0;
    while(ROM_ReadByte(IO_DIR_CFG_ADDR, _dest)<0)
    {
        if( _repeats >2)
            return -1;
        ++ _repeats;
    }
    return 0;
}
int  Store_SetIoDirConfig(char _src)
{
    int  _repeats=0;
    while(ROM_WriteByte(IO_DIR_CFG_ADDR, _src)<0)
    {
        if( _repeats<2)
            return -1;
        ++ _repeats;
    }
    return 0;
}
int  Store_SetIoDirConfigStr(char * _src)
{
    char _temp=0x00;
    for(int i=7;i>=0;--i)
    {
        if(_src[i]=='0')
            _temp &= 0xFE;//清最后一位为0
        else
            _temp |= 0x01;//置最后一位为1
        
        _temp <<=1;
    }
    if(Store_SetIoDirConfig(_temp)<0)
        return -1;
    return 0;
}
//8个字符数据
int  Store_ReadIoLevelConfigStr(char * _dest)
{
    char _temp;
    if(Store_ReadIoLevelConfig(&_temp)<0)
        return -1;
    for(int i=0;i<8;++i)
    {
        if(_temp & 0x01)
            _dest[i]='1';
        else
            _dest[i]='0';
        _temp >>= 1;
    }
    return 0;
}

int  Store_ReadIoLevelConfig(char * _dest)
{
    int _repeats=0;
    while(ROM_ReadByte(IO_LEVEL_CFG_ADDR, _dest)<0)
    {
        if( _repeats >2)
            return -1;
        ++ _repeats;
    }
    return 0;
}
int  Store_SetIoLevelConfig(char _src)
{
    int  _repeats=0;
    while(ROM_WriteByte(IO_LEVEL_CFG_ADDR, _src)<0)
    {
        if( _repeats<2)
            return -1;
        ++ _repeats;
    }
    return 0;
}
int  Store_SetIoLevelConfigStr(char * _src)
{    
    char _temp=0x00;
    for(int i=7;i>=0;--i)
    {
        if(_src[i]=='0')
            _temp &= 0xFE;//清最后一位为0
        else
            _temp |= 0x01;//置最后一位为1
        
        _temp <<=1;
    }
    if(Store_SetIoLevelConfig(_temp)<0)
        return -1;
    return 0;
}

//2个字符型字节
int Store_ReadReportTimeMode(char * _dest)
{
  int _repeats=0;
  while(ROM_ReadBytes(REPORTTIME_MODE_ADDR,_dest,REPORTTIME_MODE_LEN)!=0)
  {
    if(_repeats>2)
    { 
      return -1;
    }
    ++_repeats;
  } 
  return 0;  
}


//2个字符型字节
int Store_SetReportTimeMode(const char * _src)
{
  int _repeats=0;
  while(ROM_WriteBytes(REPORTTIME_MODE_ADDR,_src,REPORTTIME_MODE_LEN)!=0)
  {
    if(_repeats>2)
    {  
      return -1;
    }
    ++_repeats;
  } 
  return 0;  
}


//2个字符型字节
int Store_ReadSaveTimeMode(char *_dest)
{
  int _repeats=0;
  while(ROM_ReadBytes(SAVETIME_MODE_ADDR,_dest,SAVETIME_MODE_LEN)!=0)
  {
    if(_repeats>2)
    {
      return -1;
    }
    ++_repeats;
  }
  return 0;
}

//2个字符型字节
int Store_SetSaveTimeMode(const char * _src)
{
    int _repeats=0;
    while(ROM_WriteBytes(SAVETIME_MODE_ADDR , _src , SAVETIME_MODE_LEN )!=0)
    {
        if(_repeats>3)
        { 
            return -1;
        }
        ++_repeats;
    } 
    return 0;
}
//////////////////////////////////////////////////////////////
//     GSM 函数
//////////////////////////////////////////////////////////////
int  Store_GSM_ReadCenterPhone(int _index, char *_dest)
{
    if(_index< 1||_index>6)
        return -2;
    int _repeats=0; 
    int _addr=GSM_CENTERPHONE_BASE_ADDR + (_index-1)*GSM_CENTER_PHONE_LEN;
    while(ROM_ReadBytes(_addr,_dest,GSM_CENTER_PHONE_LEN)!=0)
    {
        if(_repeats>2)
        { 
            return -1;
        }
        ++_repeats;
    }
    return 0;
}

int  Store_GSM_SetCenterPhone(int _index, char *_src)
{
    if(_index< 1 || _index>6)
        return -2;
    int _repeats=0;
    int _addr=GSM_CENTERPHONE_BASE_ADDR + (_index-1)*GSM_CENTER_PHONE_LEN;
    while(ROM_WriteBytes(_addr,_src,GSM_CENTER_PHONE_LEN)!=0)
    {
        if(_repeats>2)
        {
            return -1;
        }
        ++_repeats;
    }
    return 0;
}


//
//
//  数据保存区 操作函数
//
//
//
int  Store_CheckDataItemSended(int _index) //检查发送标记
{
    if( _index < DATA_MIN_IDX || _index > DATA_MAX_IDX)
        return -2;
    char _temp;
    int _addr=DATA_START_ADDR + (_index -1)*DATA_ITEM_LEN; //
    if(ROM_ReadByte(_addr,&_temp)<0)
    {
        if(ROM_ReadByte(_addr,&_temp)<0)
            return -1;
    }
    if( _temp == 0x0F )
        return 1;
    else
        return 0; 
}
int  Store_ClearWork()
{
    //数据上下限全部设为默认
    for(int i=1;i<=8;++i)
    {
        if(Store_SetDataMaxInt(i,4096)<0)
            return -1;
        if(Store_SetDataMinInt(i,0)<0)
            return -1;
    }
    //数据区全部认为已经发送
    for(int i=DATA_MIN_IDX;i<=DATA_MAX_IDX;++i)
    {
        if(Store_MarkDataItemSended(i)<0)
            return -1;
    }
    return 0;
}

int  Store_MarkDataItemSended(int _index)   //设置发送标记
{
    if( _index < DATA_MIN_IDX || _index > DATA_MAX_IDX)
        return -2;
    int _addr=DATA_START_ADDR + (_index -1)*DATA_ITEM_LEN; //
    if(ROM_WriteByte(_addr,0x0F)<0) 
    {
         if(ROM_WriteByte(_addr,0x0F)<0)
             return -1;
    }
    return 0;
}
//  写数据函数 直接写
int  Store_WriteDataItem(char _index, const char * _src)
{//DATA_ITEM_LEN=40
    if( _index < DATA_MIN_IDX || _index > DATA_MAX_IDX)
        return -2;
    //第一个未读标记(或校验字节)由_src提供
    int _addr = DATA_START_ADDR + ( _index -1 ) * DATA_ITEM_LEN; 
    int _repeats=0;
    while(ROM_WriteBytes(_addr,_src,16)<0)
    {
        if(_repeats>2)
            return -1;
        ++ _repeats;
    }
    _repeats=0;
    _addr += 16;
    _src +=16;
    while(ROM_WriteBytes(_addr,_src,16)<0)
    {
        if(_repeats>2)
            return -1;
        ++ _repeats;
    }
    _repeats=0;
    _addr +=16;
    _src +=16;
    while(ROM_WriteBytes(_addr,_src,DATA_ITEM_LEN-32)<0)
    {
        if(_repeats>2)
            return -1;
        ++ _repeats;
    }
    return 0;
}
int  Store_WriteDataItemAuto(const char * _src)
{
    char _endIdx=0x00;
    if(RTC_ReadEndIdx(&_endIdx)<0)//
    {//读取填写标记错误
        if(RTC_RetrieveIndex()<0)
            return -1;//尝试生成标记失败
        if(RTC_ReadEndIdx(&_endIdx)<0)
            return -1;//仍旧错误
    }
    if(Store_WriteDataItem(_endIdx,_src)<0)
        return -1;
    if( _endIdx == DATA_MAX_IDX)
        _endIdx = DATA_MIN_IDX;
    else
        ++_endIdx;
    RTC_SetEndIdx(_endIdx);//更新_endIdx
    return 0;
}
/*
int  Store_ReadDataItemAuto(char * _dest)
{
    char _startIdx=0x00;
    if(RTC_ReadStartIdx(&_startIdx)<0)   
    {
        if(RTC_RetrieveIndex()<0)
            return -1;//尝试生成标记失败
        if(RTC_ReadStartIdx(&_startIdx)<0)
            return -1;//仍旧错误
    }
    if(Store_ReadDataItem(_startIdx,_dest)<0)
        return -1;
    if( _startIdx == DATA_MAX_IDX)
        _startIdx = DATA_MIN_IDX;
    else
        ++ _startIdx;
    RTC_SetStartIdx(_startIdx);//更新_startIdx;
    return 0;
}
*/

// 根据当前配置读出数据串
// 需要95字节的buffer
//  返回值为实际填写的个数(即可写的下一个下标)
//  _dest填写为 时间串*A0000B0000
//  
//  供发送流程直接使用
//  保存内容为
//  校验字节 0909011230字节A1字节A2字节B1字节B2字节..字节I1字节I2...开关字节
//  如下:
//  0           1          2          3     
//  0  1234567890 1234567890123456 789012345678  9
//  校 0909011230 AABBCCDDEEFFGGHH IIIJJJKKKLLL 开关
// 
//  _buffer(样例)
//  0         1         2         3         4
//  01234567890123456789012345678901234567890 
//  0909011230*A4096B4096I0000F0J000000M1N0R1 
int  Store_ReadDataItem(char _index , char * _dest, int _flag)
{
    if( _index < DATA_MIN_IDX || _index > DATA_MAX_IDX)
        return -2;
    int _addr=DATA_START_ADDR + (_index -1)*DATA_ITEM_LEN; //
    char _tempChar1=0x00;
    char _tempChar2=0x00;
    unsigned int  _tempInt=0; 
    char _buffer[50];
    int  _repeats=0;
    int  _idx1=0;//_buffer的索引
    int  _idx2=0;//_dest的索引
    int  _read_flag=0;
    while(1)
    {//重试3次
        if(_repeats>2)
            return -1;
        //读取内容
        //一次读不完.
        //  每次16字节
        //  DATA_ITEM_LEN = 40
        //
        if(ROM_ReadBytes(_addr,_buffer,16)<0)
            continue;
        if(ROM_ReadBytes(_addr+16,&(_buffer[16]),16)<0)
            continue;
        if(ROM_ReadBytes(_addr+32,&(_buffer[32]),DATA_ITEM_LEN-32)<0)
            continue;
        //读完了.
        // 校验码现在并不使用,该字节被用来作为 发送标记
        //if(Utility_VerifyCrcCode(_buffer,_buffer[0]))
        //    return -3; //校验码错误就返回
        if(_buffer[0]==0x0F)
        {
            _read_flag =1;
        }
        //填写时间串
        Utility_Strncpy(_dest, &(_buffer[1]),10);
        
        //定位
        _dest[10]='*';
        _idx2=10;//后续使用前缀++
        _idx1=11;
        
        //读出模拟量配置,根据配置挑选内容
        if(Store_ReadAnalogSelect(&_tempChar1)<0)
            return -1;
        //添加模拟量
        for(int i=0;i<8;++i)
        {
            if(_tempChar1 & 0x01)
            {//如果该位有
                //则添上类型码和数据
                _dest[++ _idx2] = 'A'+i;
                _tempInt = (((unsigned int)_buffer[_idx1 + 2*i])<<8) + _buffer[_idx1 + 2*i+1];
                
                Utility_UintToStr4(_tempInt,&_dest[_idx2+1]);
                _idx2+=4;
            }
            //然后是下一个
            _tempChar1 >>= 1 ;//左移一位
        }
        
        //添加脉冲量
        _idx1=27;//
        //读出脉冲量配置,根据配置挑选内容
        if(Store_ReadPulseSelect(&_tempChar1)<0)
            return -1;
        for(int i=0;i<4;++i)
        {
            if( _tempChar1 & 0x80)
            {//如果该位有才添加该位
                _dest[++ _idx2] = 'I' + i ;
                ++ _idx2;
                Utility_CharToHex( _buffer[_idx1+3*i],&(_dest[_idx2]));
                _idx2+=2;//函数填写2位.
                Utility_CharToHex( _buffer[_idx1+3*i+1],&(_dest[_idx2]));
                _idx2+=2;
                Utility_CharToHex( _buffer[_idx1+3*i+2],&(_dest[_idx2]));
                _idx2+=1; //因为下面的代码紧接着就是 ++ _idx2所以这里只加1.
            }
            _tempChar1<<=1;
        } 
        //开关量
        _idx1 = 39;
        //_tempChar1装开关量数据
        //_tempChar2装开关量的配置
        _tempChar1 = _buffer[ _idx1];
        if(Store_ReadIoSelect(&_tempChar2)<0)
            return -1;
        for(int i=0;i<8;++i)
        {//对于8个位
            if(_tempChar2&0x01)
            {//为1的位要记录0或1
                _dest[++ _idx2] = 'M' + i;
                if(_tempChar1&0x01)
                {   
                    _dest[++ _idx2]='1';
                }
                else
                {
                    _dest[++ _idx2]='0';
                }
            }
            //判断下一个
            _tempChar1 >>=1;
            _tempChar2 >>=1;
        }
        //结束  对于#号,发送程序自己加 
        break;
    }
    //根据 已发送标记进行返回
    if(_read_flag && !_flag)
        return 1;
    else
        return (_idx2+1); //发送程序需要将此位置再额外加13.
}
 










