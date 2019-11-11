//////////////////////////////////////////////////////
//     文件名: common.c
//   文件版本: 1.0.0
//   创建时间: 09年 11月30日
//   更新内容:   
//       作者: 林智
//       附注: 无
////////////////////////////////////////////////////////
//   文件版本: 3.0.0
//   创建时间: 19年 9月4日
//   更新内容: 更换msp4305438A的时钟初始化函数  
//////////////////////////////////////////////////////
  
#include "msp430common.h"
#include "common.h"
#include "rtc.h"
#include "store.h"
#include "sampler.h"
#include "Console.h"
#include "wifi_config.h"
#include "led.h"

char switcher,anahigh,analow,pulsehigh,pulsemedium,pulselow,vthigh,vtlow,tthex;
int trace_open=0;
static int s_clock=0;   //用来指示当前频率 
static unsigned int _int = 0; //中断禁用DownInt() 的层数static unsigned int s_reset_pin =0;
static unsigned int s_reset_pin =0;

void TraceOpen()
{//调试打开
  if(trace_open){
       Console_Open(); 
       
  }
   else
   {
       Console_Close();//纯粹为了保险,串口开着会浪费很多电
   }
}

void TraceFunctionLine(char const*_funcname,int _linename)
{
    if(trace_open)
    {
        Console_WriteString((char*)_funcname);
        Console_WriteString(",");
        Console_WriteInt(_linename);
    }
}

void TraceHexMsgFuncLine(char* _str,int len,char const*_funcname, int _linename)
{
    if(trace_open)
    {
        TraceFunctionLine(_funcname, _linename);
        Console_WriteHexCharln(_str,len);
    }
}

void TraceMsgFuncLine(char * _str,int _ln,char const* _funcname,int _linename)
{//发送一个字符串
  trace_open=1;
 if(trace_open)
 {
    if(_ln)
    {
        TraceFunctionLine(_funcname, _linename);
        Console_WriteStringln(_str);
    }
    else
    {   
        TraceFunctionLine(_funcname, _linename);       
        Console_WriteString(_str);
    }
 }
} 

void TraceStrFuncLine(char * _str,int _len,int _ln,char const* _funcname,int _linename)
{//发送指定长度的字符串
 if(trace_open)
 {
    if(_ln)
    {   
        TraceFunctionLine(_funcname,_linename);
        Console_WriteBytesln(_str,_len);
    }
    else
    {   
        TraceFunctionLine(_funcname,_linename); 
        Console_WriteBytes(_str,_len);
    }
 }
}
void TracePulseValue(char * _bytes3,int _ln)
{//输出脉冲值
 if(trace_open)
 {
    char _temp[7];
    Utility_Bytes3ToDecStr7(_bytes3,_temp);
    if(_ln)
        Console_WriteBytesln(_temp,7);
    else
        Console_WriteBytes(_temp,7);
 }
}

void TraceInt4FuncLine(int _val, int _ln,char const* _funcname,int _linename)
{//输出一个int 4位
  if(trace_open)
 {
    char _temp[4];
    
    Utility_UintToStr4(_val,_temp);
    
    if(_ln)
    {   
        TraceFunctionLine(_funcname, _linename);
        Console_WriteBytesln(_temp,4);
    }
    else
    {
        TraceFunctionLine(_funcname, _linename);
        Console_WriteBytes(_temp,4);
    }
 }
} 

int hex_2_ascii(char *data, char *buffer, int len)
{
    const char ascTable[17] = {"0123456789ABCDEF"};
    char *tmp_p = buffer;
    int i, pos;
    pos = 0;
    for(i = 0; i < len; i++)
    {
        tmp_p[pos++] = ascTable[data[i] >> 4];
        tmp_p[pos++] = ascTable[data[i] & 0x0f];
    }
    tmp_p[pos] = '\0';
    return pos;
}
/*ascii to int*/
int Utility_atoi(char *str, int len)  
{  
    int res = 0;
    int i = 0;
    
    for(i = 0;i < len;i++)  
    {  
        res = res * 10 + (*str - '0');  
        str++;  
    }  
    return res;  
} 

void System_Delayms( unsigned int nValue)
{//函数参数是寄存器,操作速度过快,所以我们不用nValue
    unsigned long nCount = 1150;
    unsigned long i;
    unsigned long j;
    if(s_clock==32)
    {
        nCount = 3;
    }
    for(i = nValue;i > 0;--i)
    {
        for(j = nCount;j > 0;--j);
    }
    return ;
}

void System_Delayus( unsigned int nValue)
{//函数参数是寄存器,操作速度过快,所以我们不用nValue
    unsigned long nCount = 1;
    unsigned long i;
    unsigned long j;
    if(s_clock==32)
    {
        nCount = 3;
    }
    for(i = nValue;i > 0;--i)
    {
        for(j = nCount;j > 0;--j);
    }
    return ;
}


//  关中断 立即起效,
//  开中断,则要看前面关闭了多少次中断
//  两者要严密的 对称使用.
//
void DownInt()
{
    ++ _int;
    _DINT();
}
void UpInt()
{
    if(_int)
        --_int;
    if(_int==0)
        _EINT();
} 
//2438 将P2对应的端口的中断功能禁用或使能
//脉冲中断中使用 
//5438 P1
void DsP2Int(int i)//0~7
{//将该位清0 
    P1IFG &= ~(BIT0<<i);
    P1IE &= ~(BIT0<<i);
}
void EnP2Int(int i)
{//将该位置1
    //使能前清除原先有的中断标记 
    P1IFG &= ~(BIT0<<i);
    P1IE  |=  (BIT0<<i);
}

void DsInt()//不建议调用
{
    _DINT();
}
void EnInt()//不建议调用
{
    _EINT();
}

void Max3222_Open()
{
//    //开启rs232  电平转换电路  output再拉高
//     P4DIR |= BIT0;
//     P4OUT |= BIT0; 
       P9DIR |= BIT7;
       P9OUT |= BIT7; 
}

void Max3222_Close()
{
    //关闭rs232转换电路

    P9DIR |= BIT7;
    P9OUT &= ~BIT7;
}

//1表示wifi,0表示串口
void Select_Debug_Mode(int type)
{

    if (type == 1)
    {
        System_Delayms(100);
        WIFI_Open();
        System_Delayms(100);
        Max3222_Close();
    }
    else
    {
        WIFI_Close();
//        System_Delayms(1000);
        Max3222_Open();
        System_Delayms(100);
    }
}

void System_Reset()
{
    TraceMsg("reset system",1);
    System_Delayms(1000);
    //重启系统
    WDTCTL =WDTCNTCL;
}
/*2418分频函数*/
/*
void Clock_SMCLK_DIV(int i)
{
    switch(i)
    {
      case 1:
        BCSCTL2 &= ~ DIVS1;
        BCSCTL2 &= ~ DIVS0; //ACLK 不分频
        break;
      case 2:
        BCSCTL2 &= ~ DIVS1;   //ACLK 2分频
        BCSCTL2 |=  DIVS0;  
        break;
      case 4:
        BCSCTL2 |=  DIVS1;   //ACLK 4分频
        BCSCTL2 &= ~ DIVS0;
        break;        
      case 8:
        BCSCTL2 |= DIVS1 +DIVS0; //ACLK 8分频
        break;
      default:
        BCSCTL2 &= ~ DIVS1;
        BCSCTL2 &= ~ DIVS0; //ACLK 不分频
    }
    //等待时钟正常
    do
    {
        //等待时钟稳定
        IFG1 &= ~OFIFG ;
        for(int i=0x20; i >0; i--);
    }while( (IFG1 & OFIFG)==OFIFG ); 
} 
void Clock_ACLK_DIV(int i)
{
    switch(i)
    {
      case 1:
        BCSCTL1 &= ~ DIVA1;
        BCSCTL1 &= ~ DIVA0; //ACLK 不分频
        break;
      case 2:
        BCSCTL1 &= ~ DIVA1;   //ACLK 2分频
        BCSCTL1 |=  DIVA0;  
        break;
      case 4:
        BCSCTL1 |=  DIVA1;   //ACLK 4分频
        BCSCTL1 &= ~ DIVA0;
        break;        
      case 8:
        BCSCTL1 |= DIVA1 +DIVA0; //ACLK 8分频
        break;
      default:
        BCSCTL1 &= ~ DIVA1;
        BCSCTL1 &= ~ DIVA0; //ACLK 不分频
    }
    //等待时钟正常
    do
    {
        //等待时钟稳定
        SFRIFG1 &= ~OFIFG ;
        for(int i=0x20; i >0; i--);
    }while( (SFRIFG1 & OFIFG)==OFIFG ); 
} 
*/
//
//void Clock_Use8MHZ()
//{
//    s_clock=8;
//    unsigned int i;
//    BCSCTL1= 0x00; //开启XT2
//    do
//    {
//     //等待时钟稳定
//        IFG1 &= ~OFIFG ;
//        for(i=0x20; i >0; i--) ;
//    }while( (IFG1 & OFIFG)==OFIFG );
//  
//    BCSCTL2= 0x00;
//    BCSCTL2 |= SELM1;       // MCLK  使用XT2   8M
//    BCSCTL2 |= SELS;        // SMCLK 使用XT2   8M
//                            // ACLK  使用XT1   32K
//}

/*2418分频函数*/
void Clock_SMCLK_DIV(int i)
{
    switch(i)
    {
      case 1:
        UCSCTL5 &= ~ DIVS1;
        UCSCTL5 &= ~ DIVS0; //ACLK 不分频
        break;
      case 2:
        UCSCTL5 &= ~ DIVS1;   //ACLK 2分频
        UCSCTL5 |=  DIVS0;  
        break;
      case 4:
        UCSCTL5 |=  DIVS1;   //ACLK 4分频
        UCSCTL5 &= ~ DIVS0;
        break;        
      case 8:
        UCSCTL5 |= DIVS1 +DIVS0; //ACLK 8分频
        break;
      default:
        UCSCTL5 &= ~ DIVS1;
        UCSCTL5 &= ~ DIVS0; //ACLK 不分频
    }
    //等待时钟正常
    do
    {
        //等待时钟稳定
        SFRIFG1 &= ~OFIFG ;
        for(int i=0x20; i >0; i--);
    }while( (SFRIFG1 & OFIFG)==OFIFG ); 
} 
void Clock_ACLK_DIV(int i)
{
    switch(i)
    {
      case 1:
        UCSCTL5 &= ~ DIVA1;
        UCSCTL5 &= ~ DIVA0; //ACLK 不分频
        break;
      case 2:
        UCSCTL5 &= ~ DIVA1;   //ACLK 2分频
        UCSCTL5 |=  DIVA0;  
        break;
      case 4:
        UCSCTL5 |=  DIVA1;   //ACLK 4分频
        UCSCTL5 &= ~ DIVA0;
        break;        
      case 8:
        UCSCTL5 |= DIVA1 +DIVA0; //ACLK 8分频
        break;
      default:
        UCSCTL5 &= ~ DIVA1;
        UCSCTL5 &= ~ DIVA0; //ACLK 不分频
    }
    //等待时钟正常
    do
    {
        //等待时钟稳定
        SFRIFG1 &= ~OFIFG ;
        for(int i=0x20; i >0; i--);
    }while( (SFRIFG1 & OFIFG)==OFIFG ); 
} 
/******************************************************************************
功能：设置内核电压
详细：Power Management Module (PMM).The PMM manages all functions related to the power supply and its supervision for the device. Its primary
functions are first to generate a supply voltage for the core logic, and second, provide several
mechanisms for the supervision and monitoring of both the voltage applied to the device (DVCC) and thevoltage generated for the core (VCORE).
The PMM uses an integrated low-dropout voltage regulator (LDO) to produce a secondary core voltage(VCORE) from the primary one applied to the device (DVCC).
In general, VCOREsupplies the CPU, memories(flash and RAM), and the digital modules, while DVCCsupplies the I/Os and all analog modules (includingthe 
oscillators). The VCOREoutput is maintained using a dedicated voltage reference. VCORE is programmable up to four steps, 
to provide only as much power as is needed for the speed that has been selected for the CPU. 
This enhances power efficiency of the system. The input or primary side of the regulator is referred to in this chapter as its high side. 
The output or secondary side is referred to in this chapter as its low side.
******************************************************************************/

void Set_Vcore(unsigned int level)
{  
  PMMCTL0_H = PMMPW_H;                    // Open PMM registers for write  
  SVSMHCTL = SVSHE + SVSHRVL0 * level + SVMHE + SVSMHRRL0 * level;  // Set SVS/SVM high side new level  
  SVSMLCTL = SVSLE + SVMLE + SVSMLRRL0 * level;   // Set SVM low side to new level  
  while ((PMMIFG & SVSMLDLYIFG) == 0);    // Wait till SVM is settled  
  PMMIFG &= ~(SVMLVLRIFG + SVMLIFG);      // Clear already set flags  
  PMMCTL0_L = PMMCOREV0 * level;          // Set VCore to new level  
  if ((PMMIFG & SVMLIFG))                 // Wait till new level reached
  while ((PMMIFG & SVMLVLRIFG) == 0);
  SVSMLCTL = SVSLE + SVSLRVL0 * level + SVMLE + SVSMLRRL0 * level;  // Set SVS/SVM low side to new level  
  PMMCTL0_H = 0x00;                       // Lock PMM registers for write access
}

/* for msp430f5438
 * Assume XT1=32K XT2=8M
 * MCLK,SMCLK时钟源选择XT2振荡器(8MHz)， ACLK时钟源选择XT1振荡器(32k)
 * MCLK,SMCLK = 1MHz, ACLK = 4k
 */

void Clock_Init(){
    unsigned char i;
    
    WDTCTL=WDTPW+WDTHOLD;                 //关闭看门狗定时
    
    P5SEL |= BIT2 + BIT3;                   //P5.2和P5.3选择为晶振XT2输入  
    P7SEL |= BIT0 + BIT1;                   //P7.0和P7.1选择为晶振XT1输入  
    Set_Vcore(PMMCOREV_3);                  // Set frequency up to 25MHz
    UCSCTL6 &= ~(XT1OFF + XT2OFF);          // Set XT1 & XT2 On 
    
    UCSCTL6 |= XCAP_3;                      // Internal load XT1 cap 12pF，MSP430F5438A V4.0最小系统XT1未接外部晶振
    
    UCSCTL6 |= XT2BYPASS;                   //选择外部激励振动
    UCSCTL6 |= XT1BYPASS;
    UCSCTL4 |= SELA__XT1CLK + SELS__XT2CLK + SELM__XT2CLK;    //选择MCLK、SMCLK为XT2,
    do                                      // Loop until XT1,XT2 & DCO stabilizes
    {
      UCSCTL7 &= ~(XT2OFFG + XT1LFOFFG + XT1HFOFFG + DCOFFG);
      SFRIFG1 &= ~OFIFG;                    // 清除振荡器失效标志
//      for (i = 0xFF; i > 0; i--);           // 延时，等待XT2起振
    } while (SFRIFG1 & OFIFG);              // 判断XT2是否起振
    //Delay_ms(50);
     System_Delayms(50);
    Clock_ACLK_DIV(8);  
    Clock_SMCLK_DIV(8);
        
}

/* for msp430f2418 */

//void Clock_Init()
//{//初始化时钟
//#if 0
//    Clock_Use8MHZ();
//    Clock_ACLK_DIV(8);
//    Clock_SMCLK_DIV(8);
//  
//#else
//
//   /*网上找了一段适合2418和2169 CPU时钟初始化的的代码*/
//    unsigned long i=0xffff;
//    _BIC_SR(0xFFFF);                      //将SR中的各位清零，内联函数。通过看它的汇编代码可以知道。一个直接后果是GIE被复位，所有的可屏蔽中断关断。
//
//    WDTCTL=WDTPW+WDTHOLD;                 //关闭看门狗定时
//
//    BCSCTL1&=~XT2OFF;                     //打开XT2  
//
//    BCSCTL2= 0x00;
//    BCSCTL2 |= SELM1;       // MCLK  使用XT2   8M
//    BCSCTL2 |= SELS;        // SMCLK 使用XT2   8M
//                    // ACLK  使用XT1   32K
//
//
//    // BCSCTL2 |= (SELM_2+DIVM_2+SELS);        //MCLK、SMCLK选择xt2（5MHz，所以MCLK=1.25MHz，SMCLK=5MHz
//   // BCSCTL3 |=  (XT2S_2+LFXT1S_2+XCAP_1);                  //6pF电容 
//    IFG1 &= ~OFIFG;
//
//    IFG1&=~(WDTIFG+OFIFG+NMIIFG+PORIFG+RSTIFG);  //中断标志清零
//    FCTL3=FWKEY;                          //ACCVIFG清零
//    //IE1|=OFIE+NMIIE+ACCVIE;               //中断初始化设置
//
//    while(i)
//    {
//       i--;
//    }
//
//
//    BCSCTL3 = (XT2S_2+LFXT1S_0+XCAP_1);
//
//    do
//    {
//       IFG1 &= ~OFIFG;                      // Clear OSCFault flag
//        for (i = 0xFF; i > 0; i--);          // Time for flag to set
//    }
//    while (IFG1 & OFIFG);                 // OSCFault flag still set  
//
//   Clock_ACLK_DIV(8);
//    Clock_SMCLK_DIV(8);
//    
//#endif
//   
//}  

int Utility_GetGprsServerIp(char* _str,int* len)
{
    int _ret = 0;
    hydrologyConfigPara configpara = {0};
    
    _ret = Store_ReadHydrologyConfigData((char*)&configpara);

    if(0 != _ret)
    {
       Console_WriteErrorStringln("get gprs server ip faied");
       return _ret;
    }

    *len = Utility_Strlen(configpara.serverip);

    if (*len > REMOTE_GPRS_SERVER_IP_ADDR_LEN)
    {
        Console_WriteErrorStringln("get gprs server ip faied,len is out of limit");
        return _ret;
    }

    Utility_Strncpy(_str,configpara.serverip, *len);

    return 0;
    
}

int Utility_GetGprsServerPort(char* _str,int *len)
{
    int _ret = 0;
    hydrologyConfigPara configpara = {0};
    
    _ret = Store_ReadHydrologyConfigData((char*)&configpara);

    if(0 != _ret)
    {
       Console_WriteErrorStringln("get gprs server port faied");
       return _ret;
    }
    
    *len = Utility_Strlen(configpara.serverport);

    if (*len > REMOTE_GRPS_SERVER_PORT_ADDR_LEN)
    {
        Console_WriteErrorStringln("get gprs server port faied,len is out of limit");
        return _ret;
    }

    Utility_Strncpy(_str,configpara.serverport, *len);

    return 0;
    
}

int Utility_GetGprsServerApn(char* _str,int* len)
{
    int _ret = 0;
    hydrologyConfigPara configpara = {0};
    
    _ret = Store_ReadHydrologyConfigData((char*)&configpara);

    if(0 != _ret)
    {
       Console_WriteErrorStringln("get gprs server apn faied");
       return _ret;
    }

    *len = Utility_Strlen(configpara.serverapn);

    if (*len > REMOTE_GPRS_SERVER_APN_ADDR_LEN)
    {
        Console_WriteErrorStringln("get gprs server apn faied,len is out of limit");
        return _ret;
    }

    Utility_Strncpy(_str,configpara.serverapn, *len);

    return 0;
    
}


int  Utility_CheckDigital(const char * _str,int _start ,int _end)
{
    for(int i=_start; i <= _end; ++i)
    {
        if( _str[i]<'0' || _str[i]>'9')
            return -1;
    }
    return 0;
}

int  Utility_CheckAlphabet(const char * _str,int _start ,int _end)
{
    for(int i=_start; i <= _end; ++i)
    {
        if( (_str[i]>='A' && _str[i]<='Z') || (_str[i]>='a' && _str[i]<='z'))
            return 0;
    }
    return -1;
}

int  Utility_CheckHexChar(const char * _str,int _start ,int _end)
{
    for(int i=_start; i <= _end; ++i)
    {
        if( (_str[i]>='0' && _str[i]<='9') || (_str[i]>='A' && _str[i]<='Z') || (_str[i]>='a' && _str[i]<='z'))
            return 0;
    }
    return -1;
}

int  Utility_CheckBinary(const char * _str,int _start,int _end)
{
    for(int i= _start; i <= _end; ++i)
    {
        if( _str[i]!='0' && _str[i]!='1')
            return -1;
    }
    return 0;
} 

//这个check ip并不完善，目前校验范围是[0-9].[0-9].[0-9].[0-9]
int Utility_CheckIp(const char* _str,int _start,int _end)
{
    int section = 0;  //每一节的十进制值 
    int dot = 0;       //几个点分隔符 

    for(int i= _start; i <= _end; ++i)
    { 
        if(_str[i] == '.')
        { 
            dot++; 
            if(dot > 3)
            { 
                goto IPErrorHandle; 
            } 
            if(section >= 0 && section <=255)
            { 
                section = 0; 
            }
            else
            { 
                goto IPErrorHandle; 
            } 
        }
        else if(_str[i] >= '0' && _str[i] <= '9')
        { 
            section = section * 10 + _str[i] - '0'; 
        }
        else
        { 
            goto IPErrorHandle;
        } 
    }

    if(section >= 0 && section <=255)
    { 
        if(3 == dot)
        {
            section = 0; 
            return 0;
        }
    } 
IPErrorHandle:
    Console_WriteErrorStringln("IP address invalid!\n");       
    return -1; 
}

int  Utility_BytesCompare3(const char * _bytes1,const char * _bytes2)
{// 判断3字节数据的大小,脉冲中断中使用 
    if(_bytes1[0] > _bytes2[0]) 
        return  1;
    if(_bytes1[0] < _bytes2[0]) 
        return  -1;
    else
    {
        if(_bytes1[1] > _bytes2[1]) 
            return  1;
        if(_bytes1[1] < _bytes2[1]) 
            return  -1;
        else
        {
            if(_bytes1[2] > _bytes2[2]) 
                return  1;
            if(_bytes1[2] < _bytes2[2])
                return -1;
            else
                return 0;
        }
    }
}

int  Utility_Strlen(char * str)
{
    int temp=0;
    while(*(str++)!='\0')
        ++temp;
    return temp;
} 
void Utility_Strncpy(char *dest,char *src, int Len)
{//不主动添加结束符
    for(int i=0;i<Len;++i)
    {
        dest[i]=src[i];
    }
}
int  Utility_Strncmp(const char * src,const char *dest ,int len)
{//不主动添加结束符
    for(int i=0;i<len;++i)
    {
        if(src[i]!=dest[i])
            return -1;
    }
    return 0;
} 
//  0 ->00   255->FF
int  Utility_CharToHex(char _src , char * _dest)
{
    char _temp=0x00;
    _temp = _src>>4;
    if(_temp >= 10 )
        _dest[0] = 'A' + _temp - 10 ;
    else
        _dest[0] = '0' + _temp ;
    _temp = _src & 0x0F;
    if( _temp >= 10 )
        _dest[1] = 'A' + _temp - 10 ;
    else
        _dest[1] = '0' + _temp ;
    
    return 0;
} 


//
//  我们这个应用只会有7位DEC字串,6位HEX字串 和3个字节之间的互换,
//  因此函数也没有设计成通用正确的.
//
int  Utility_Bytes3ToDecStr7(char * _src ,char * _dest)
{//首先需要转换成long这个是必须的.
    unsigned long _tempLong=0;
    _tempLong += (((unsigned long)_src[0])<<16);
    _tempLong += (((unsigned long)_src[1])<<8);
    _tempLong += _src[2];
    //已经产生long了.开始转换 
    _dest[0] = (char)(_tempLong /(1000000L))+'0';
    _tempLong %=1000000L;
    _dest[1] = (char) (_tempLong /(100000L))+'0';
    _tempLong %= 100000L;
    _dest[2] = (char) (_tempLong /(10000L))+'0';
    _tempLong %= 10000L;
    
    //后续就不需要long那么大了. 就转换成int, 
    //因为long非CPU类型,操作缓慢 
    Utility_UintToStr4((unsigned int)_tempLong,&_dest[3]); 
    return 0;//返回字符串长度
}



int Utility_DecStr7ToBytes3(char * _src, char * _dest)
{//
    unsigned long _tempLong=0;
    for(int i=0;i<6;++i)
    {
        _tempLong+= _src[i]-'0';
        _tempLong*=10;
    }
    _tempLong += _src[6]-'0';//个位数
    //产生long了
    _dest[0] = (char)(_tempLong>>16);//取第2个字节
    _dest[1] = (char)(_tempLong>>8);//取第3个字节
    _dest[2] = (char)(_tempLong&0xFF);//取第1个字节
    return 0;
}
int  Utility_UintToStr4(unsigned int _src, char *_dest)
{
    _dest[0]=_src/1000+'0';
    _src %=1000;
    _dest[1]=_src/100+'0';
    _src %=100;
    _dest[2]=_src/10+'0';
    _dest[3]=_src%10+'0';
    return 0;
}
int  Utility_UintToStr3(unsigned int _src, char *_dest)
{
    _dest[0]=_src/100+'0';
    _src %=100;
    _dest[1]=_src/10+'0';
    _dest[2]=_src%10+'0';

    return 0;
}

int  Utility_UintToStr2(unsigned int _src, char *_dest)
{
    _dest[0]=_src/10+'0';
    _dest[1]=_src%10+'0';
    return 0;
}
int  Utility_UintToStr1(unsigned int _src, char *_dest)
{
    _dest[0]=_src%10+'0';

    return 0;
}


void IntTo0xInt(unsigned int num,int count){
	   
	   if(count==1){
	    switcher=0x00;
        switcher+=num;
	   	
	   }
           if(count==2){
             tthex=0x00;
             int a,b;
             a=num/10;
             b=num%10;
             tthex+=a;
             tthex<<=4;
             tthex+=b;
           }
	   if(count==4)
	   {
	   	   anahigh=0x00;
	   	   analow=0x00;
	   	   int a,b,c,d;
	   	   a=num/1000;
	   	   b=num/100%10;
	   	   c=num%100/10;
	   	   d=num%100%10;
	       anahigh+=a;
	       anahigh<<=4;
	       anahigh+=b;
	       analow+=c;
	       analow<<=4;
	       analow+=d;
	   }
	   if(count==6){
	   	  pulsehigh=0x00;
	   	  pulsemedium=0x00;
	   	  pulselow=0x00;
	   	  int a,b,c,d,e,f;
	   	  a=num/100000;
	   	  b=num/10000%10;
	   	  c=num/1000%10;
	   	  d=num/100%10;
	   	  e=num/10%10;
	   	  f=num%10;
		  pulsehigh+=a;
		  pulsehigh<<=4; 
		  pulsehigh+=b;
		   
		  pulsemedium+=c;
		  pulsemedium<<=4; 
		  pulsemedium+=d; 
		  
		  pulselow+=e;
		  pulselow<<=4; 
		  pulselow+=f; 	  
	   }
}
//把ASCII转换为16进制
char ConvertHexChar(char ch)
{
  if((ch>='0')&&(ch <='9'))
    return ch-0x30;
  else if((ch>='A')&&(ch <='F'))
    return ch-'A'+10;
  else if((ch>='a')&&(ch <='f'))
    return ch-'a'+10;
  else 
    return (-1);
}
//把两位ASCII数组转换为一位16进制数组
char ConvertAscIItoHex(char* ascii,char* hex,int asciilen)
{
    int i,j;
//    if((asciilen % 2) == 0)
//    {
//        TraceMsg("The size of AscII array is odd, it must be even. The size of hex array must be even too！",1);
//        return -1;
//    }
    for(i = 0,j = 0; i < asciilen;i+=2,j++)
        hex[j] = ConvertHexChar(ascii[i]) * 16 + ConvertHexChar(ascii[i+1]);
    return 0;
}

// 专门针对天数的判断
static void _addDay(char *dest)// 减少代码大小
{//天数判断复杂
  //1 是否闰年 
  //2 月份数 
  //闰年定义:  4年1闰;100年不闰;400年要闰.  
    if(dest[1]==2)//是否是2月, 28,29
    {//20开头的闰年
    //本系统肯定不会使用到2100年
    //所以就只判断能否被4除  
        if(dest[0]%4==0)//闰年
        {
            if(dest[2]<30)//没超过29天
                return;
            dest[2]-=29; //减去溢出的29
            ++dest[1];  //月份 +1
        }
        else//不是闰年
        {
            if(dest[2] < 29)//没超过28天
                return ;
            dest[2]-=28; //减去溢出的28
            ++dest[1];  //月份 +1
        }
    }
    // 是否是30天的月
    if(dest[1]==4||dest[1]==6||dest[1]==9||dest[1]==11)
    {
        if(dest[2]<31) //没超过30天
            return;
        dest[2]-=30; //减去溢出的30天
        ++dest[1]; //月份+1
    } 
    // 是否是31天的月
    if(dest[1]==1||dest[1]==3||dest[1]==5||dest[1]==7
       ||dest[1]==8||dest[1]==10||dest[1]==12)
    {
        if(dest[2]<32) //没超过31天
            return;
        dest[2]-=31;//减去溢出的31天
        ++dest[1]; //月份 +1
    }
    if(dest[1]<13)//没超过12月
        return ;
    dest[1]-=12; //减去溢出的12个月
    ++dest[0];  //年份 +1 
    return;//搞完了  : )
    
} 

void Utility_Time_AddSecond(char *dest, int second)// second不可以大于60秒
{
  if(second>60)
    return ;
  dest[5]+=second;//加上秒数
  if(dest[5]<60)
    return ; //完成了 
  dest[5]-=60;//分钟加1
  ++dest[3];
  if(dest[4]<60)
    return ; //完成了 
  dest[4]-=60;//小时加1
  ++dest[3];   
  if(dest[3]<24)
    return ;//完成了
  dest[3]-=24;//天数加1;
  ++dest[2];
  _addDay(dest);
}  
//  年月日时分 char[5] 数值 
void Utility_Time_AddMinute(char *dest, int minute)// minute不可以大于60分钟
{
  if(minute>60)
    return ;
  dest[4]+=minute;//加上分钟数
  if(dest[4]<60)
    return ; //完成了 
  dest[4]-=60;//小时加1
  ++dest[3]; 
  if(dest[3]<24)
    return ;//完成了
  dest[3]-=24;//天数加1;
  ++dest[2];
  _addDay(dest);
}  
void Utility_Time_AddHour(char *dest, int hour)//hour不可以大于24
{
  if(hour>24)
    return;
  dest[3]+=hour; 
  if(dest[3]<24)
    return ;//完成了
  
  dest[3]-=24;//天数加1;
  ++dest[2];
  _addDay(dest); 
}
void Utility_Time_AddDay(char *dest,int Day)
{
  if(Day>28)//为了保证1月31日加天数,不会跳月出现错误
    return;
  dest[2]+=Day;
  _addDay(dest);
}
void Utility_Time_AddMonth(char *dest,int Month)
{
    if(Month>12)
        return ;
    dest[1]+=Month;
    if(dest[1]<13)
        return;
    dest[1]-=12;
    ++dest[0];
}
void Utility_CalculateNextReportTimeBytes(char *dest)//根据当前设置,计算出下一次报告时间
{//报告时间 5分钟
    for(int i=0;i<5;++i)
    {
        dest[i]=g_rtc_nowTime[i];
    }
    //读取ReportMode
    char temp[2];
    int mode=0;
    if(Store_ReadReportTimeMode(temp)<0)
    {//没读出模式 就当5分钟一存好了.
        mode=1;
    }
    else
        mode=(temp[0]-'0')*10 + temp[1] - '0';
//  01:  5分钟   02:  10分钟  03: 20分钟   04:  30分钟  
//  05:  1小时   06:   2小时  07:  3小时   08:  6小时  
//  09:  12小时  10:    1天   11:    2天   12:   3天  
//  13:    5天   14:    10天  15:  15天    16:  1个月 
    
    switch(mode)
    {
      case 1: 
        dest[4] -= dest[4]%5;
        Utility_Time_AddMinute(dest,5); //加5分钟   
        break;
        
      case 2:
        dest[4] -= dest[4]%10; 
        Utility_Time_AddMinute(dest,10);//加10分钟 
        break; 
        
      case 3:
        dest[4] -= dest[4]%20;
        Utility_Time_AddMinute(dest,20);//加20分钟
        break;
        
      case 4:
        dest[4] -= dest[4]%30;
        Utility_Time_AddMinute(dest,30);//加30分钟
        break; 
        
      case 5:
        dest[4]=0; //分钟清0 即可 
        Utility_Time_AddHour(dest,1);//加1小时 
        break;
        
      case 6:
        dest[4]=0;//分钟清0
        dest[3] -= dest[3]%2;//
        Utility_Time_AddHour(dest,2);//加2小时 
        break; 
        
      case 7:
        dest[4] =0;
        dest[3] -= dest[3]%3;
        Utility_Time_AddHour(dest,3);//加3小时
        
        break;
      case 8:
        dest[4]=0;
        dest[3] -= dest[3]%6;
        Utility_Time_AddHour(dest,6);//加6小时
        break; 
        
      case 9:
        dest[4]=0;
        dest[3] -= dest[3]%12;
        Utility_Time_AddHour(dest,12);//加12小时
        break;
        
      case 10:
        dest[4]=0;
        dest[3]=8;
        Utility_Time_AddDay(dest,1);//加1天  
        break;
        
//  对于天数,我们不仅要在加模式间隔值前求整,
//  加了之后,仍需要求整.
//  因为天数是变动的, 不是间隔值的整数倍.        
//  天数的求整 比较复杂,因为是以1为起始.
        
        //
        //  还是有问题. 仍然需要修改!!!!
        //
    case 11:
        dest[4]=0;
        dest[3]=8;
        dest[2]-=(dest[2]-1)%2;
        //++dest[2];//天数从1开始
        
        Utility_Time_AddDay(dest,2);//加2天
        
        dest[2]-=(dest[2]-1)%2;
        //++dest[2];//天数从1开始
        break;
      case 12: 
        dest[4]=0;
        dest[3]=8;
        dest[2]-=(dest[2]-1)%3;
        //++dest[2];//天数从1开始
        Utility_Time_AddDay(dest,3);//加3天
        dest[2]-=(dest[2]-1)%3;
        //++dest[2];//天数从1开始
        
        break;
      case 13:
        
        dest[4]=0;
        dest[3]=8;
        dest[2]-=(dest[2]-1)%5;
        //++dest[2];//天数从1开始 
        
        Utility_Time_AddDay(dest,5);//加5天
        dest[2]-=(dest[2]-1)%5;
        //++dest[2];//天数从1开始
        break;
      case 14:
        
        dest[4]=0;
        dest[3]=8;
        dest[2]-=(dest[2]-1)%10;
        //++dest[2];//天数从1开始
        Utility_Time_AddDay(dest,10);//加10天
        dest[2]-=(dest[2]-1)%10;
        //++dest[2];//天数从1开始
        
        break;
      case 15:
        
        dest[4]=0;
        dest[3]=8;
        dest[2]-=(dest[2]-1)%15;
        //++dest[2];
        Utility_Time_AddDay(dest,15);//加15天
        dest[2]-=(dest[2]-1)%15;
        //++dest[2];
        
        break;
      case 16:
        dest[4]=0;
        dest[3]=8;
        dest[2]=1;
        Utility_Time_AddMonth(dest,1);//加1个月
        break;
      default:
        //要是模式有问题.
        //我们就默认为5分钟保存一次
        Utility_Time_AddMinute(dest,5);
    }
}

void Utility_CalculateNextCameraGoTimes(char *dest)//根据当前设置,计算出下一次摄像时间
{//报告时间 5分钟
    for(int i=0;i<5;++i)
    {
        dest[i]=g_rtc_nowTime[i];
    }
    //读取ReportMode
    char temp[2];
    int mode=0;
    if(Store_ReadCameraTimeMode(temp)<0)
    {//没读出模式 就当5分钟一存好了.
        mode=1;
    }
    else
        mode=(temp[0]-'0')*10 + temp[1] - '0';
//  01:  5分钟   02:  10分钟  03: 20分钟   04:  30分钟  
//  05:  1小时   06:   2小时  07:  3小时   08:  6小时  
//  09:  12小时  10:    1天   11:    2天   12:   3天  
//  13:    5天   14:    10天  15:  15天    16:  1个月 
    
    switch(mode)
    {
      case 1: 
        dest[4] -= dest[4]%5;
        Utility_Time_AddMinute(dest,5); //加5分钟   
        break;
        
      case 2:
        dest[4] -= dest[4]%10; 
        Utility_Time_AddMinute(dest,10);//加10分钟 
        break; 
        
      case 3:
        dest[4] -= dest[4]%20;
        Utility_Time_AddMinute(dest,20);//加20分钟
        break;
        
      case 4:
        dest[4] -= dest[4]%30;
        Utility_Time_AddMinute(dest,30);//加30分钟
        break; 
        
      case 5:
        dest[4]=0; //分钟清0 即可 
        Utility_Time_AddHour(dest,1);//加1小时 
        break;
        
      case 6:
        dest[4]=0;//分钟清0
        dest[3] -= dest[3]%2;//
        Utility_Time_AddHour(dest,2);//加2小时 
        break; 
        
      case 7:
        dest[4] =0;
        dest[3] -= dest[3]%3;
        Utility_Time_AddHour(dest,3);//加3小时
        
        break;
      case 8:
        dest[4]=0;
        dest[3] -= dest[3]%6;
        Utility_Time_AddHour(dest,6);//加6小时
        break; 
        
      case 9:
        dest[4]=0;
        dest[3] -= dest[3]%12;
        Utility_Time_AddHour(dest,12);//加12小时
        break;
        
      case 10:
        dest[4]=0;
        dest[3]=8;
        Utility_Time_AddDay(dest,1);//加1天  
        break;
        
//  对于天数,我们不仅要在加模式间隔值前求整,
//  加了之后,仍需要求整.
//  因为天数是变动的, 不是间隔值的整数倍.        
//  天数的求整 比较复杂,因为是以1为起始.
        
        //
        //  还是有问题. 仍然需要修改!!!!
        //
    case 11:
        dest[4]=0;
        dest[3]=8;
        dest[2]-=(dest[2]-1)%2;
        //++dest[2];//天数从1开始
        
        Utility_Time_AddDay(dest,2);//加2天
        
        dest[2]-=(dest[2]-1)%2;
        //++dest[2];//天数从1开始
        break;
      case 12: 
        dest[4]=0;
        dest[3]=8;
        dest[2]-=(dest[2]-1)%3;
        //++dest[2];//天数从1开始
        Utility_Time_AddDay(dest,3);//加3天
        dest[2]-=(dest[2]-1)%3;
        //++dest[2];//天数从1开始
        
        break;
      case 13:
        
        dest[4]=0;
        dest[3]=8;
        dest[2]-=(dest[2]-1)%5;
        //++dest[2];//天数从1开始 
        
        Utility_Time_AddDay(dest,5);//加5天
        dest[2]-=(dest[2]-1)%5;
        //++dest[2];//天数从1开始
        break;
      case 14:
        
        dest[4]=0;
        dest[3]=8;
        dest[2]-=(dest[2]-1)%10;
        //++dest[2];//天数从1开始
        Utility_Time_AddDay(dest,10);//加10天
        dest[2]-=(dest[2]-1)%10;
        //++dest[2];//天数从1开始
        
        break;
      case 15:
        
        dest[4]=0;
        dest[3]=8;
        dest[2]-=(dest[2]-1)%15;
        //++dest[2];
        Utility_Time_AddDay(dest,15);//加15天
        dest[2]-=(dest[2]-1)%15;
        //++dest[2];
        
        break;
      case 16:
        dest[4]=0;
        dest[3]=8;
        dest[2]=1;
        Utility_Time_AddMonth(dest,1);//加1个月
        break;
      default:
        //要是模式有问题.
        //我们就默认为5分钟保存一次
        Utility_Time_AddMinute(dest,5);
    }
}


void Utility_CalculateNextSaveTimeBytes(char *dest)//根据当前设置,计算出下一次保存时间
{//保存时间 5分钟
    for(int i=0;i<5;++i)
    {
        dest[i]=g_rtc_nowTime[i];
    }
    //读取SaveMode
    char temp[2];
    int mode=0;
    if(Store_ReadSaveTimeMode(temp)<0)
    {//没读出模式 就当5分钟一存好了.
        mode=1;
    }
    else
        mode=(temp[0]-'0')*10+ temp[1] - '0'; 
//  01:  5分钟   02:  10分钟  03: 20分钟   04:  30分钟  
//  05:  1小时   06:   2小时  07:  3小时   08:  6小时  
//  09:  12小时  10:    1天   11:    2天   12:   3天  
//  13:    5天   14:    10天  15:  15天    16:  1个月  
     
    // 分钟的整点 就是00 ,小时是 8点?   天数 就是1日.
    
    switch(mode)
    {
      case 1: 
        dest[4] -= dest[4]%5;
        Utility_Time_AddMinute(dest,5); //加5分钟  
        
        break;
      case 2:
        dest[4] -= dest[4]%10; 
        Utility_Time_AddMinute(dest,10);//加10分钟
        
        break; 
      case 3:
        dest[4] -= dest[4]%20;
        Utility_Time_AddMinute(dest,20);//加20分钟
        break;
      case 4:
        dest[4] -= dest[4]%30;
        Utility_Time_AddMinute(dest,30);//加30分钟

        
        break; 
      case 5:  
        dest[4]=0; //分钟清0 即可 
        Utility_Time_AddHour(dest,1);//加1小时
        
        break;
      case 6:
        dest[4]=0;//分钟清0
        dest[3] -= dest[3]%2;  //
        Utility_Time_AddHour(dest,2);//加2小时
        
        
        break; 
      case 7:
        dest[4] =0;
        dest[3] -= dest[3]%3;
        Utility_Time_AddHour(dest,3);//加3小时
        
        
        break;
      case 8:
        dest[4]=0;
        dest[3] -= dest[3]%6;
        Utility_Time_AddHour(dest,6);//加6小时
        
        
        break; 
      case 9:
        dest[4]=0;
        dest[3] -= dest[3]%12;
        Utility_Time_AddHour(dest,12);//加12小时
        
        
        break;
        
      case 10:
        dest[4]=0;
        dest[3]=8;
        Utility_Time_AddDay(dest,1);//加1天  
        break;
        
//  对于天数,我们不仅要在加模式间隔值前求整,
//  加了之后,仍需要求整.
//  因为天数是变动的, 不是间隔值的整数倍.        
//  天数的求整 比较复杂,因为是以1为起始.
    
    case 11:
        dest[4]=0;
        dest[3]=8;
        dest[2]-=(dest[2]-1)%2;
        //++dest[2];//天数从1开始
        Utility_Time_AddDay(dest,2);//加2天    
        dest[2]-=(dest[2]-1)%2;
        //++dest[2];//天数从1开始
        break;
      case 12: 
        dest[4]=0;
        dest[3]=8;
        dest[2]-=(dest[2]-1)%3;
        //++dest[2];//天数从1开始
        Utility_Time_AddDay(dest,3);//加3天
        dest[2]-=(dest[2]-1)%3;
        //++dest[2];//天数从1开始
        
        break;
      case 13:
        
        dest[4]=0;
        dest[3]=8;
        dest[2]-=(dest[2]-1)%5;
        //++dest[2];//天数从1开始 
        
        Utility_Time_AddDay(dest,5);//加5天
        dest[2]-=(dest[2]-1)%5;
        //++dest[2];//天数从1开始
        break;
      case 14:
        
        dest[4]=0;
        dest[3]=8;
        dest[2]-=(dest[2]-1)%10;
        //++dest[2];//天数从1开始
        Utility_Time_AddDay(dest,10);//加10天
        dest[2]-=(dest[2]-1)%10;
        //++dest[2];//天数从1开始
        
        break;
      case 15:
        
        dest[4]=0;
        dest[3]=8;
        dest[2]-=(dest[2]-1)%15;
        //++dest[2];
        Utility_Time_AddDay(dest,15);//加15天
        dest[2]-=(dest[2]-1)%15;
        //++dest[2];
        
        break;
      case 16:
        dest[4]=0;
        dest[3]=8;
        dest[2]=1;
        Utility_Time_AddMonth(dest,1);//加1个月
        break;
      default:
        //要是模式有问题.
        //我们就默认为5分钟保存一次
        Utility_Time_AddMinute(dest,5);
    }
}
void Utility_CalculateNextCheckTimeBytes(char *dest)//根据当前时间,计算下一次检查时间
{//检查时间 5分钟
    for(int i=0;i<5;++i)
    {
        dest[i]=g_rtc_nowTime[i];
    }
    //必须整点整点加5分钟.
    dest[4] -= dest[4]%5;
    Utility_Time_AddMinute(dest,5);
}
int  Utility_Is_A_CheckTime(char *  _time)
{//分钟为5的倍数 就是检查时间
    if(_time[4]%5==0) 
        return 1;
    else
        return 0;
}

int  Utility_Is_A_SaveTime(char *  _time)
{//
    char _temp[2];
    int _mode=0;
    if(Store_ReadSaveTimeMode(_temp)<0)
    {
        return 0;
    }
    else
        _mode= (_temp[0]-'0')*10+ _temp[1] - '0'; 
//  01:  5分钟   02:  10分钟  03: 20分钟   04:  30分钟  
//  05:  1小时   06:   2小时  07:  3小时   08:  6小时  
//  09:  12小时  10:    1天   11:    2天   12:   3天  
//  13:    5天   14:    10天  15:  15天    16:  1个月  
    switch(_mode)
    {
      case 1:  
        if(_time[4]%5==0)
            return 1; 
        break;
      case 2:
        if(_time[4]%10==0)
            return 1;
        break;
      case 3:
        if(_time[4]%20==0)
            return 1;
        break;
      case 4:
        if(_time[4]%30==0)
            return 1;
        break;
      case 5:
        if(_time[4]==0)
            return 1;
        break;
      case 6:
        if(_time[3]%2==0 && _time[4]==0)
            return 1;
        break;
      case 7:
        if(_time[3]%3==0 && _time[4]==0)
            return 1;
        break;
      case 8: 
        if(_time[3]%6==0 && _time[4]==0)
            return 1;
        break; 
      case 9:
        if( (_time[3]%12==0 || _time[3]==0 )&& _time[4]==0)
            return 1;
        break;
      case 10:
        if(_time[3]==8 && _time[4]==0)
            return 1;
        break;  
      case 11:
        if( (_time[2]-1)%2==0 && _time[3]==8 && _time[4]==0 )
            return 1;
        break;
      case 12:
        if( (_time[2]-1)%3==0 && _time[3]==8 && _time[4]==0 )
            return 1;
        break;
      case 13:
        if( (_time[2]-1)%5==0 && _time[3]==8 && _time[4]==0 )
            return 1;
        break;
      case 14:
        if( (_time[2]-1)%10==0 && _time[3]==8 && _time[4]==0 )
            return 1;
        break;
      case 15:
        if( (_time[2]-1)%15==0 && _time[3]==8 && _time[4]==0 )
            return 1;
        break;
      case 16:
        if(_time[2]==1 && _time[3]==8 && _time[4]==0)
            return 1;
        break;
        
      default:
        break;
    }
    return 0;
}

int  Utility_Is_A_ReportTime(char *  _time)
{
    char _temp[2];
    int _mode=0;
    if(Store_ReadReportTimeMode(_temp)<0)
    {//没读出模式 就当5分钟一存好了.
        _mode = 0;
    }
    else
        _mode= (_temp[0]-'0')*10+ _temp[1] - '0'; 
//  01:  5分钟   02:  10分钟  03: 20分钟   04:  30分钟  
//  05:  1小时   06:   2小时  07:  3小时   08:  6小时  
//  09:  12小时  10:    1天   11:    2天   12:   3天  
//  13:    5天   14:    10天  15:  15天    16:  1个月  
    switch(_mode)
    {
      case 1:  
        if(_time[4]%5==0)
            return 1;
        break;
      case 2:
        if(_time[4]%10==0)
            return 1;
        break;
      case 3:
        if(_time[4]%20==0)
            return 1;
        break;
      case 4:
        if(_time[4]%30==0)
            return 1;
        break;
      case 5:
        if(_time[4]==0)
            return 1;
        break;
      case 6:
        if(_time[3]%2==0 && _time[4]==0)
            return 1;
        break;
      case 7:
        if(_time[3]%3==0 && _time[4]==0)
            return 1;
        break;
      case 8: 
        if(_time[3]%6==0 && _time[4]==0)
            return 1;
        break; 
      case 9: 
        if( ( _time[3]%12==0 || _time[3]==0 )&& _time[4]==0)
            return 1;
        break;
      case 10:
        if(_time[3]==8 && _time[4]==0)
            return 1;
        break;  
      case 11:
        if( (_time[2]-1)%2==0 && _time[3]==8 && _time[4]==0 )
            return 1;
        break;
      case 12:
        if( (_time[2]-1)%3==0 && _time[3]==8 && _time[4]==0 )
            return 1;
        break;
      case 13:
        if( (_time[2]-1)%5==0 && _time[3]==8 && _time[4]==0 )
            return 1;
        break;
      case 14:
        if( (_time[2]-1)%10==0 && _time[3]==8 && _time[4]==0 )
            return 1;
        break;
      case 15:
        if( (_time[2]-1)%15==0 && _time[3]==8 && _time[4]==0 )
            return 1;
        break;
      case 16:
        if(_time[2]==1 && _time[3]==8 && _time[4]==0)
            return 1;
        break;
      default:
        break;
    }
    return 0;
}


int  Utility_Is_A_CameraTime(char *  _time)
{
    char _temp[2];
    int _mode=0;
    if(Store_ReadCameraTimeMode(_temp)<0)
    {//没读出模式 就当5分钟一存好了.
        _mode = 0;
    }
    else
        _mode= (_temp[0]-'0')*10+ _temp[1] - '0'; 
//  01:  5分钟   02:  10分钟  03: 20分钟   04:  30分钟  
//  05:  1小时   06:   2小时  07:  3小时   08:  6小时  
//  09:  12小时  10:    1天   11:    2天   12:   3天  
//  13:    5天   14:    10天  15:  15天    16:  1个月  
    switch(_mode)
    {
      case 1:  
        if(_time[4]%5==0)
            return 1;
        break;
      case 2:
        if(_time[4]%10==0)
            return 1;
        break;
      case 3:
        if(_time[4]%20==0)
            return 1;
        break;
      case 4:
        if(_time[4]%30==0)
            return 1;
        break;
      case 5:
        if(_time[4]==0)
            return 1;
        break;
      case 6:
        if(_time[3]%2==0 && _time[4]==0)
            return 1;
        break;
      case 7:
        if(_time[3]%3==0 && _time[4]==0)
            return 1;
        break;
      case 8: 
        if(_time[3]%6==0 && _time[4]==0)
            return 1;
        break; 
      case 9: 
        if( ( _time[3]%12==0 || _time[3]==0 )&& _time[4]==0)
            return 1;
        break;
      case 10:
        if(_time[3]==8 && _time[4]==0)
            return 1;
        break;  
      case 11:
        if( (_time[2]-1)%2==0 && _time[3]==8 && _time[4]==0 )
            return 1;
        break;
      case 12:
        if( (_time[2]-1)%3==0 && _time[3]==8 && _time[4]==0 )
            return 1;
        break;
      case 13:
        if( (_time[2]-1)%5==0 && _time[3]==8 && _time[4]==0 )
            return 1;
        break;
      case 14:
        if( (_time[2]-1)%10==0 && _time[3]==8 && _time[4]==0 )
            return 1;
        break;
      case 15:
        if( (_time[2]-1)%15==0 && _time[3]==8 && _time[4]==0 )
            return 1;
        break;
      case 16:
        if(_time[2]==1 && _time[3]==8 && _time[4]==0)
            return 1;
        break;
      default:
        break;
    }
    return 0;
}
//根据约定,_buffer里已经填写好头部
//0123456789012345678901234567890
//$00011100011<TM*OK*0909060630#
//$00011100011<DL*OK# 

//$00011100011<RQ*TM#
int Utility_PackRuestTimeMsg(char * _buffer)
{ 
    _buffer[13]='R';_buffer[14]='Q';_buffer[15]='*';_buffer[16]='T';_buffer[17]='M';
    return 18;
}
int Utility_PackOKMsg(const char *  _type,   char * _buffer)
{
    _buffer[13]=_type[0];_buffer[14]=_type[1];_buffer[15]='*';
    _buffer[16] ='O';_buffer[17]='K';
    if(_type[0]=='T' && _type[1]=='M')
    {
        _buffer[18]='*';
        RTC_ReadTimeStr6_B(&(_buffer[19]));
        _buffer[31]='#';
        return 32;
    }
    if(_type[0]=='D'&&_type[1]=='L')
    {
        
    }
    _buffer[18]='#';
    return 19;
}
//0123456789012345678901234567890 
//$00011100011<DL*Fail#
int Utility_PackFailMsg(const char * _type,  char * _buffer)
{
    _buffer[13]=_type[0];_buffer[14]=_type[1];_buffer[15]='*';
    _buffer[16]='F';_buffer[17]='a';_buffer[18]='i';_buffer[19]='l';
    _buffer[20]='#';
    return 21;
}
int Utility_PackRejectMsg(const char * _type, char * _buffer)
{
    _buffer[13]=_type[0];_buffer[14]=_type[1];_buffer[15]='*';
    _buffer[16]='R';_buffer[17]='e';_buffer[18]='j';_buffer[19]='e';
    _buffer[20]='c';_buffer[21]='c';_buffer[22]='t';_buffer[23]='#'; 
    return 24;
}
int Utility_PackErrorMsg(const char * _type,  char * _buffer)
{
    _buffer[13]=_type[0];_buffer[14]=_type[1];_buffer[15]='*';
    _buffer[16]='E';_buffer[17]='r';_buffer[18]='r';_buffer[19]='o';
    _buffer[20]='r';_buffer[21]='#'; 
    return 22; 
}
    
int Utility_PackBadMsg(const char * _type,   char * _buffer)
{
    _buffer[13]=_type[0];_buffer[14]=_type[1];_buffer[15]='*';
    _buffer[16]='B';_buffer[17]='a';_buffer[18]='d';_buffer[19]='#';
    return 20;
}
/*2438 P41  5438 P96*/
void Clear_ExternWatchdog()
{
    if (s_reset_pin == 0)
    {
        P9OUT &= ~BIT6;
        s_reset_pin =1;
       // Led3_On();
    }
    else 
    {
        P9OUT |= BIT6;
        s_reset_pin =0;
       // Led3_Off();
    }
}
