//////////////////////////////////////////////////////
//     �ļ���: common.c
//   �ļ��汾: 1.0.0
//   ����ʱ��: 09�� 11��30��
//   ��������:   
//       ����: ����
//       ��ע: ��
////////////////////////////////////////////////////////
//   �ļ��汾: 3.0.0
//   ����ʱ��: 19�� 9��4��
//   ��������: ����msp4305438A��ʱ�ӳ�ʼ������  
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
static int s_clock=0;   //����ָʾ��ǰƵ�� 
static unsigned int _int = 0; //�жϽ���DownInt() �Ĳ���static unsigned int s_reset_pin =0;
static unsigned int s_reset_pin =0;

void TraceOpen()
{//���Դ�
  if(trace_open){
       Console_Open(); 
       
  }
   else
   {
       Console_Close();//����Ϊ�˱���,���ڿ��Ż��˷Ѻܶ��
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
{//����һ���ַ���
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
{//����ָ�����ȵ��ַ���
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
{//�������ֵ
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
{//���һ��int 4λ
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
{//���������ǼĴ���,�����ٶȹ���,�������ǲ���nValue
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
{//���������ǼĴ���,�����ٶȹ���,�������ǲ���nValue
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


//  ���ж� ������Ч,
//  ���ж�,��Ҫ��ǰ��ر��˶��ٴ��ж�
//  ����Ҫ���ܵ� �Գ�ʹ��.
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
//2438 ��P2��Ӧ�Ķ˿ڵ��жϹ��ܽ��û�ʹ��
//�����ж���ʹ�� 
//5438 P1
void DsP2Int(int i)//0~7
{//����λ��0 
    P1IFG &= ~(BIT0<<i);
    P1IE &= ~(BIT0<<i);
}
void EnP2Int(int i)
{//����λ��1
    //ʹ��ǰ���ԭ���е��жϱ�� 
    P1IFG &= ~(BIT0<<i);
    P1IE  |=  (BIT0<<i);
}

void DsInt()//���������
{
    _DINT();
}
void EnInt()//���������
{
    _EINT();
}

void Max3222_Open()
{
//    //����rs232  ��ƽת����·  output������
//     P4DIR |= BIT0;
//     P4OUT |= BIT0; 
       P9DIR |= BIT7;
       P9OUT |= BIT7; 
}

void Max3222_Close()
{
    //�ر�rs232ת����·

    P9DIR |= BIT7;
    P9OUT &= ~BIT7;
}

//1��ʾwifi,0��ʾ����
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
    //����ϵͳ
    WDTCTL =WDTCNTCL;
}
/*2418��Ƶ����*/
/*
void Clock_SMCLK_DIV(int i)
{
    switch(i)
    {
      case 1:
        BCSCTL2 &= ~ DIVS1;
        BCSCTL2 &= ~ DIVS0; //ACLK ����Ƶ
        break;
      case 2:
        BCSCTL2 &= ~ DIVS1;   //ACLK 2��Ƶ
        BCSCTL2 |=  DIVS0;  
        break;
      case 4:
        BCSCTL2 |=  DIVS1;   //ACLK 4��Ƶ
        BCSCTL2 &= ~ DIVS0;
        break;        
      case 8:
        BCSCTL2 |= DIVS1 +DIVS0; //ACLK 8��Ƶ
        break;
      default:
        BCSCTL2 &= ~ DIVS1;
        BCSCTL2 &= ~ DIVS0; //ACLK ����Ƶ
    }
    //�ȴ�ʱ������
    do
    {
        //�ȴ�ʱ���ȶ�
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
        BCSCTL1 &= ~ DIVA0; //ACLK ����Ƶ
        break;
      case 2:
        BCSCTL1 &= ~ DIVA1;   //ACLK 2��Ƶ
        BCSCTL1 |=  DIVA0;  
        break;
      case 4:
        BCSCTL1 |=  DIVA1;   //ACLK 4��Ƶ
        BCSCTL1 &= ~ DIVA0;
        break;        
      case 8:
        BCSCTL1 |= DIVA1 +DIVA0; //ACLK 8��Ƶ
        break;
      default:
        BCSCTL1 &= ~ DIVA1;
        BCSCTL1 &= ~ DIVA0; //ACLK ����Ƶ
    }
    //�ȴ�ʱ������
    do
    {
        //�ȴ�ʱ���ȶ�
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
//    BCSCTL1= 0x00; //����XT2
//    do
//    {
//     //�ȴ�ʱ���ȶ�
//        IFG1 &= ~OFIFG ;
//        for(i=0x20; i >0; i--) ;
//    }while( (IFG1 & OFIFG)==OFIFG );
//  
//    BCSCTL2= 0x00;
//    BCSCTL2 |= SELM1;       // MCLK  ʹ��XT2   8M
//    BCSCTL2 |= SELS;        // SMCLK ʹ��XT2   8M
//                            // ACLK  ʹ��XT1   32K
//}

/*2418��Ƶ����*/
void Clock_SMCLK_DIV(int i)
{
    switch(i)
    {
      case 1:
        UCSCTL5 &= ~ DIVS1;
        UCSCTL5 &= ~ DIVS0; //ACLK ����Ƶ
        break;
      case 2:
        UCSCTL5 &= ~ DIVS1;   //ACLK 2��Ƶ
        UCSCTL5 |=  DIVS0;  
        break;
      case 4:
        UCSCTL5 |=  DIVS1;   //ACLK 4��Ƶ
        UCSCTL5 &= ~ DIVS0;
        break;        
      case 8:
        UCSCTL5 |= DIVS1 +DIVS0; //ACLK 8��Ƶ
        break;
      default:
        UCSCTL5 &= ~ DIVS1;
        UCSCTL5 &= ~ DIVS0; //ACLK ����Ƶ
    }
    //�ȴ�ʱ������
    do
    {
        //�ȴ�ʱ���ȶ�
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
        UCSCTL5 &= ~ DIVA0; //ACLK ����Ƶ
        break;
      case 2:
        UCSCTL5 &= ~ DIVA1;   //ACLK 2��Ƶ
        UCSCTL5 |=  DIVA0;  
        break;
      case 4:
        UCSCTL5 |=  DIVA1;   //ACLK 4��Ƶ
        UCSCTL5 &= ~ DIVA0;
        break;        
      case 8:
        UCSCTL5 |= DIVA1 +DIVA0; //ACLK 8��Ƶ
        break;
      default:
        UCSCTL5 &= ~ DIVA1;
        UCSCTL5 &= ~ DIVA0; //ACLK ����Ƶ
    }
    //�ȴ�ʱ������
    do
    {
        //�ȴ�ʱ���ȶ�
        SFRIFG1 &= ~OFIFG ;
        for(int i=0x20; i >0; i--);
    }while( (SFRIFG1 & OFIFG)==OFIFG ); 
} 
/******************************************************************************
���ܣ������ں˵�ѹ
��ϸ��Power Management Module (PMM).The PMM manages all functions related to the power supply and its supervision for the device. Its primary
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
 * MCLK,SMCLKʱ��Դѡ��XT2����(8MHz)�� ACLKʱ��Դѡ��XT1����(32k)
 * MCLK,SMCLK = 1MHz, ACLK = 4k
 */

void Clock_Init(){
    unsigned char i;
    
    WDTCTL=WDTPW+WDTHOLD;                 //�رտ��Ź���ʱ
    
    P5SEL |= BIT2 + BIT3;                   //P5.2��P5.3ѡ��Ϊ����XT2����  
    P7SEL |= BIT0 + BIT1;                   //P7.0��P7.1ѡ��Ϊ����XT1����  
    Set_Vcore(PMMCOREV_3);                  // Set frequency up to 25MHz
    UCSCTL6 &= ~(XT1OFF + XT2OFF);          // Set XT1 & XT2 On 
    
    UCSCTL6 |= XCAP_3;                      // Internal load XT1 cap 12pF��MSP430F5438A V4.0��СϵͳXT1δ���ⲿ����
    
    UCSCTL6 |= XT2BYPASS;                   //ѡ���ⲿ������
    UCSCTL6 |= XT1BYPASS;
    UCSCTL4 |= SELA__XT1CLK + SELS__XT2CLK + SELM__XT2CLK;    //ѡ��MCLK��SMCLKΪXT2,
    do                                      // Loop until XT1,XT2 & DCO stabilizes
    {
      UCSCTL7 &= ~(XT2OFFG + XT1LFOFFG + XT1HFOFFG + DCOFFG);
      SFRIFG1 &= ~OFIFG;                    // �������ʧЧ��־
//      for (i = 0xFF; i > 0; i--);           // ��ʱ���ȴ�XT2����
    } while (SFRIFG1 & OFIFG);              // �ж�XT2�Ƿ�����
    //Delay_ms(50);
     System_Delayms(50);
    Clock_ACLK_DIV(8);  
    Clock_SMCLK_DIV(8);
        
}

/* for msp430f2418 */

//void Clock_Init()
//{//��ʼ��ʱ��
//#if 0
//    Clock_Use8MHZ();
//    Clock_ACLK_DIV(8);
//    Clock_SMCLK_DIV(8);
//  
//#else
//
//   /*��������һ���ʺ�2418��2169 CPUʱ�ӳ�ʼ���ĵĴ���*/
//    unsigned long i=0xffff;
//    _BIC_SR(0xFFFF);                      //��SR�еĸ�λ���㣬����������ͨ�������Ļ��������֪����һ��ֱ�Ӻ����GIE����λ�����еĿ������жϹضϡ�
//
//    WDTCTL=WDTPW+WDTHOLD;                 //�رտ��Ź���ʱ
//
//    BCSCTL1&=~XT2OFF;                     //��XT2  
//
//    BCSCTL2= 0x00;
//    BCSCTL2 |= SELM1;       // MCLK  ʹ��XT2   8M
//    BCSCTL2 |= SELS;        // SMCLK ʹ��XT2   8M
//                    // ACLK  ʹ��XT1   32K
//
//
//    // BCSCTL2 |= (SELM_2+DIVM_2+SELS);        //MCLK��SMCLKѡ��xt2��5MHz������MCLK=1.25MHz��SMCLK=5MHz
//   // BCSCTL3 |=  (XT2S_2+LFXT1S_2+XCAP_1);                  //6pF���� 
//    IFG1 &= ~OFIFG;
//
//    IFG1&=~(WDTIFG+OFIFG+NMIIFG+PORIFG+RSTIFG);  //�жϱ�־����
//    FCTL3=FWKEY;                          //ACCVIFG����
//    //IE1|=OFIE+NMIIE+ACCVIE;               //�жϳ�ʼ������
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

//���check ip�������ƣ�ĿǰУ�鷶Χ��[0-9].[0-9].[0-9].[0-9]
int Utility_CheckIp(const char* _str,int _start,int _end)
{
    int section = 0;  //ÿһ�ڵ�ʮ����ֵ 
    int dot = 0;       //������ָ��� 

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
{// �ж�3�ֽ����ݵĴ�С,�����ж���ʹ�� 
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
{//���������ӽ�����
    for(int i=0;i<Len;++i)
    {
        dest[i]=src[i];
    }
}
int  Utility_Strncmp(const char * src,const char *dest ,int len)
{//���������ӽ�����
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
//  �������Ӧ��ֻ����7λDEC�ִ�,6λHEX�ִ� ��3���ֽ�֮��Ļ���,
//  ��˺���Ҳû����Ƴ�ͨ����ȷ��.
//
int  Utility_Bytes3ToDecStr7(char * _src ,char * _dest)
{//������Ҫת����long����Ǳ����.
    unsigned long _tempLong=0;
    _tempLong += (((unsigned long)_src[0])<<16);
    _tempLong += (((unsigned long)_src[1])<<8);
    _tempLong += _src[2];
    //�Ѿ�����long��.��ʼת�� 
    _dest[0] = (char)(_tempLong /(1000000L))+'0';
    _tempLong %=1000000L;
    _dest[1] = (char) (_tempLong /(100000L))+'0';
    _tempLong %= 100000L;
    _dest[2] = (char) (_tempLong /(10000L))+'0';
    _tempLong %= 10000L;
    
    //�����Ͳ���Ҫlong��ô����. ��ת����int, 
    //��Ϊlong��CPU����,�������� 
    Utility_UintToStr4((unsigned int)_tempLong,&_dest[3]); 
    return 0;//�����ַ�������
}



int Utility_DecStr7ToBytes3(char * _src, char * _dest)
{//
    unsigned long _tempLong=0;
    for(int i=0;i<6;++i)
    {
        _tempLong+= _src[i]-'0';
        _tempLong*=10;
    }
    _tempLong += _src[6]-'0';//��λ��
    //����long��
    _dest[0] = (char)(_tempLong>>16);//ȡ��2���ֽ�
    _dest[1] = (char)(_tempLong>>8);//ȡ��3���ֽ�
    _dest[2] = (char)(_tempLong&0xFF);//ȡ��1���ֽ�
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
//��ASCIIת��Ϊ16����
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
//����λASCII����ת��Ϊһλ16��������
char ConvertAscIItoHex(char* ascii,char* hex,int asciilen)
{
    int i,j;
//    if((asciilen % 2) == 0)
//    {
//        TraceMsg("The size of AscII array is odd, it must be even. The size of hex array must be even too��",1);
//        return -1;
//    }
    for(i = 0,j = 0; i < asciilen;i+=2,j++)
        hex[j] = ConvertHexChar(ascii[i]) * 16 + ConvertHexChar(ascii[i+1]);
    return 0;
}

// ר������������ж�
static void _addDay(char *dest)// ���ٴ����С
{
    if(dest[1]==2)//�Ƿ���2��, 28,29
    {
        if(dest[0]%4==0)//����
        {
            if(dest[2]<30)//û����29��
                return;
            dest[2]-=29; //��ȥ�����29
            ++dest[1];  //�·� +1
        }
        else//��������
        {
            if(dest[2] < 29)//û����28��
                return ;
            dest[2]-=28; //��ȥ�����28
            ++dest[1];  //�·� +1
        }
    }
    // �Ƿ���30�����
    if(dest[1]==4||dest[1]==6||dest[1]==9||dest[1]==11)
    {
        if(dest[2]<31) //û����30��
            return;
        dest[2]-=30; //��ȥ�����30��
        ++dest[1]; //�·�+1
    } 
    // �Ƿ���31�����
    if(dest[1]==1||dest[1]==3||dest[1]==5||dest[1]==7
       ||dest[1]==8||dest[1]==10||dest[1]==12)
    {
        if(dest[2]<32) //û����31��
            return;
        dest[2]-=31;//��ȥ�����31��
        ++dest[1]; //�·� +1
    }
    if(dest[1]<13)//û����12��
        return ;
    dest[1]-=12; //��ȥ�����12����
    ++dest[0];  //��� +1 
    return;//������  : )
    
} 

void Utility_Time_AddSecond(char *dest, int second)// second�����Դ���60��
{
  if(second>60)
    return ;
  dest[5]+=second;//��������
  if(dest[5]<60)
    return ; //����� 
  dest[5]-=60;//���Ӽ�1
  ++dest[3];
  if(dest[4]<60)
    return ; //����� 
  dest[4]-=60;//Сʱ��1
  ++dest[3];   
  if(dest[3]<24)
    return ;//�����
  dest[3]-=24;//������1;
  ++dest[2];
  _addDay(dest);
}  
//  ������ʱ�� char[5] ��ֵ 
void Utility_Time_AddMinute(char *dest, int minute)// minute�����Դ���60����
{
  if(minute>60)
    return ;
  dest[4]+=minute;//���Ϸ�����
  if(dest[4]<60)
    return ; //����� 
  dest[4]-=60;//Сʱ��1
  ++dest[3]; 
  if(dest[3]<24)
    return ;//�����
  dest[3]-=24;//������1;
  ++dest[2];
  _addDay(dest);
}  
void Utility_Time_AddHour(char *dest, int hour)//hour�����Դ���24
{
  if(hour>24)
    return;
  dest[3]+=hour; 
  if(dest[3]<24)
    return ;//�����
  
  dest[3]-=24;//������1;
  ++dest[2];
  _addDay(dest); 
}
void Utility_Time_AddDay(char *dest,int Day)
{
  if(Day>28)//Ϊ�˱�֤1��31�ռ�����,�������³��ִ���
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
void Utility_CalculateNextReportTimeBytes(char *dest)//���ݵ�ǰ����,�������һ�α���ʱ��
{//����ʱ�� 5����
    for(int i=0;i<5;++i)
    {
        dest[i]=g_rtc_nowTime[i];
    }
    //��ȡReportMode
    char temp[2];
    int mode=0;
    if(Store_ReadReportTimeMode(temp)<0)
    {//û����ģʽ �͵�5����һ�����.
        mode=1;
    }
    else
        mode=(temp[0]-'0')*10 + temp[1] - '0';
//  01:  5����   02:  10����  03: 20����   04:  30����  
//  05:  1Сʱ   06:   2Сʱ  07:  3Сʱ   08:  6Сʱ  
//  09:  12Сʱ  10:    1��   11:    2��   12:   3��  
//  13:    5��   14:    10��  15:  15��    16:  1���� 
    
    switch(mode)
    {
      case 1: 
        dest[4] -= dest[4]%5;
        Utility_Time_AddMinute(dest,5); //��5����   
        break;
        
      case 2:
        dest[4] -= dest[4]%10; 
        Utility_Time_AddMinute(dest,10);//��10���� 
        break; 
        
      case 3:
        dest[4] -= dest[4]%20;
        Utility_Time_AddMinute(dest,20);//��20����
        break;
        
      case 4:
        dest[4] -= dest[4]%30;
        Utility_Time_AddMinute(dest,30);//��30����
        break; 
        
      case 5:
        dest[4]=0; //������0 ���� 
        Utility_Time_AddHour(dest,1);//��1Сʱ 
        break;
        
      case 6:
        dest[4]=0;//������0
        dest[3] -= dest[3]%2;//
        Utility_Time_AddHour(dest,2);//��2Сʱ 
        break; 
        
      case 7:
        dest[4] =0;
        dest[3] -= dest[3]%3;
        Utility_Time_AddHour(dest,3);//��3Сʱ
        
        break;
      case 8:
        dest[4]=0;
        dest[3] -= dest[3]%6;
        Utility_Time_AddHour(dest,6);//��6Сʱ
        break; 
        
      case 9:
        dest[4]=0;
        dest[3] -= dest[3]%12;
        Utility_Time_AddHour(dest,12);//��12Сʱ
        break;
        
      case 10:
        dest[4]=0;
        dest[3]=8;
        Utility_Time_AddDay(dest,1);//��1��  
        break;
        
//  ��������,���ǲ���Ҫ�ڼ�ģʽ���ֵǰ����,
//  ����֮��,����Ҫ����.
//  ��Ϊ�����Ǳ䶯��, ���Ǽ��ֵ��������.        
//  ���������� �Ƚϸ���,��Ϊ����1Ϊ��ʼ.
        
        //
        //  ����������. ��Ȼ��Ҫ�޸�!!!!
        //
    case 11:
        dest[4]=0;
        dest[3]=8;
        dest[2]-=(dest[2]-1)%2;
        //++dest[2];//������1��ʼ
        
        Utility_Time_AddDay(dest,2);//��2��
        
        dest[2]-=(dest[2]-1)%2;
        //++dest[2];//������1��ʼ
        break;
      case 12: 
        dest[4]=0;
        dest[3]=8;
        dest[2]-=(dest[2]-1)%3;
        //++dest[2];//������1��ʼ
        Utility_Time_AddDay(dest,3);//��3��
        dest[2]-=(dest[2]-1)%3;
        //++dest[2];//������1��ʼ
        
        break;
      case 13:
        
        dest[4]=0;
        dest[3]=8;
        dest[2]-=(dest[2]-1)%5;
        //++dest[2];//������1��ʼ 
        
        Utility_Time_AddDay(dest,5);//��5��
        dest[2]-=(dest[2]-1)%5;
        //++dest[2];//������1��ʼ
        break;
      case 14:
        
        dest[4]=0;
        dest[3]=8;
        dest[2]-=(dest[2]-1)%10;
        //++dest[2];//������1��ʼ
        Utility_Time_AddDay(dest,10);//��10��
        dest[2]-=(dest[2]-1)%10;
        //++dest[2];//������1��ʼ
        
        break;
      case 15:
        
        dest[4]=0;
        dest[3]=8;
        dest[2]-=(dest[2]-1)%15;
        //++dest[2];
        Utility_Time_AddDay(dest,15);//��15��
        dest[2]-=(dest[2]-1)%15;
        //++dest[2];
        
        break;
      case 16:
        dest[4]=0;
        dest[3]=8;
        dest[2]=1;
        Utility_Time_AddMonth(dest,1);//��1����
        break;
      default:
        //Ҫ��ģʽ������.
        //���Ǿ�Ĭ��Ϊ5���ӱ���һ��
        Utility_Time_AddMinute(dest,5);
    }
}

void Utility_CalculateNextCameraGoTimes(char *dest)//���ݵ�ǰ����,�������һ������ʱ��
{//����ʱ�� 5����
    for(int i=0;i<5;++i)
    {
        dest[i]=g_rtc_nowTime[i];
    }
    //��ȡReportMode
    char temp[2];
    int mode=0;
    if(Store_ReadCameraTimeMode(temp)<0)
    {//û����ģʽ �͵�5����һ�����.
        mode=1;
    }
    else
        mode=(temp[0]-'0')*10 + temp[1] - '0';
//  01:  5����   02:  10����  03: 20����   04:  30����  
//  05:  1Сʱ   06:   2Сʱ  07:  3Сʱ   08:  6Сʱ  
//  09:  12Сʱ  10:    1��   11:    2��   12:   3��  
//  13:    5��   14:    10��  15:  15��    16:  1���� 
    
    switch(mode)
    {
      case 1: 
        dest[4] -= dest[4]%5;
        Utility_Time_AddMinute(dest,5); //��5����   
        break;
        
      case 2:
        dest[4] -= dest[4]%10; 
        Utility_Time_AddMinute(dest,10);//��10���� 
        break; 
        
      case 3:
        dest[4] -= dest[4]%20;
        Utility_Time_AddMinute(dest,20);//��20����
        break;
        
      case 4:
        dest[4] -= dest[4]%30;
        Utility_Time_AddMinute(dest,30);//��30����
        break; 
        
      case 5:
        dest[4]=0; //������0 ���� 
        Utility_Time_AddHour(dest,1);//��1Сʱ 
        break;
        
      case 6:
        dest[4]=0;//������0
        dest[3] -= dest[3]%2;//
        Utility_Time_AddHour(dest,2);//��2Сʱ 
        break; 
        
      case 7:
        dest[4] =0;
        dest[3] -= dest[3]%3;
        Utility_Time_AddHour(dest,3);//��3Сʱ
        
        break;
      case 8:
        dest[4]=0;
        dest[3] -= dest[3]%6;
        Utility_Time_AddHour(dest,6);//��6Сʱ
        break; 
        
      case 9:
        dest[4]=0;
        dest[3] -= dest[3]%12;
        Utility_Time_AddHour(dest,12);//��12Сʱ
        break;
        
      case 10:
        dest[4]=0;
        dest[3]=8;
        Utility_Time_AddDay(dest,1);//��1��  
        break;
        
//  ��������,���ǲ���Ҫ�ڼ�ģʽ���ֵǰ����,
//  ����֮��,����Ҫ����.
//  ��Ϊ�����Ǳ䶯��, ���Ǽ��ֵ��������.        
//  ���������� �Ƚϸ���,��Ϊ����1Ϊ��ʼ.
        
        //
        //  ����������. ��Ȼ��Ҫ�޸�!!!!
        //
    case 11:
        dest[4]=0;
        dest[3]=8;
        dest[2]-=(dest[2]-1)%2;
        //++dest[2];//������1��ʼ
        
        Utility_Time_AddDay(dest,2);//��2��
        
        dest[2]-=(dest[2]-1)%2;
        //++dest[2];//������1��ʼ
        break;
      case 12: 
        dest[4]=0;
        dest[3]=8;
        dest[2]-=(dest[2]-1)%3;
        //++dest[2];//������1��ʼ
        Utility_Time_AddDay(dest,3);//��3��
        dest[2]-=(dest[2]-1)%3;
        //++dest[2];//������1��ʼ
        
        break;
      case 13:
        
        dest[4]=0;
        dest[3]=8;
        dest[2]-=(dest[2]-1)%5;
        //++dest[2];//������1��ʼ 
        
        Utility_Time_AddDay(dest,5);//��5��
        dest[2]-=(dest[2]-1)%5;
        //++dest[2];//������1��ʼ
        break;
      case 14:
        
        dest[4]=0;
        dest[3]=8;
        dest[2]-=(dest[2]-1)%10;
        //++dest[2];//������1��ʼ
        Utility_Time_AddDay(dest,10);//��10��
        dest[2]-=(dest[2]-1)%10;
        //++dest[2];//������1��ʼ
        
        break;
      case 15:
        
        dest[4]=0;
        dest[3]=8;
        dest[2]-=(dest[2]-1)%15;
        //++dest[2];
        Utility_Time_AddDay(dest,15);//��15��
        dest[2]-=(dest[2]-1)%15;
        //++dest[2];
        
        break;
      case 16:
        dest[4]=0;
        dest[3]=8;
        dest[2]=1;
        Utility_Time_AddMonth(dest,1);//��1����
        break;
      default:
        //Ҫ��ģʽ������.
        //���Ǿ�Ĭ��Ϊ5���ӱ���һ��
        Utility_Time_AddMinute(dest,5);
    }
}


void Utility_CalculateNextSaveTimeBytes(char *dest)//���ݵ�ǰ����,�������һ�α���ʱ��
{//����ʱ�� 5����
    for(int i=0;i<5;++i)
    {
        dest[i]=g_rtc_nowTime[i];
    }
    //��ȡSaveMode
    char temp[2];
    int mode=0;
    if(Store_ReadSaveTimeMode(temp)<0)
    {//û����ģʽ �͵�5����һ�����.
        mode=1;
    }
    else
        mode=(temp[0]-'0')*10+ temp[1] - '0'; 
//  01:  5����   02:  10����  03: 20����   04:  30����  
//  05:  1Сʱ   06:   2Сʱ  07:  3Сʱ   08:  6Сʱ  
//  09:  12Сʱ  10:    1��   11:    2��   12:   3��  
//  13:    5��   14:    10��  15:  15��    16:  1����  
     
    // ���ӵ����� ����00 ,Сʱ�� 8��?   ���� ����1��.
    
    switch(mode)
    {
      case 1: 
        dest[4] -= dest[4]%5;
        Utility_Time_AddMinute(dest,5); //��5����  
        
        break;
      case 2:
        dest[4] -= dest[4]%10; 
        Utility_Time_AddMinute(dest,10);//��10����
        
        break; 
      case 3:
        dest[4] -= dest[4]%20;
        Utility_Time_AddMinute(dest,20);//��20����
        break;
      case 4:
        dest[4] -= dest[4]%30;
        Utility_Time_AddMinute(dest,30);//��30����

        
        break; 
      case 5:  
        dest[4]=0; //������0 ���� 
        Utility_Time_AddHour(dest,1);//��1Сʱ
        
        break;
      case 6:
        dest[4]=0;//������0
        dest[3] -= dest[3]%2;  //
        Utility_Time_AddHour(dest,2);//��2Сʱ
        
        
        break; 
      case 7:
        dest[4] =0;
        dest[3] -= dest[3]%3;
        Utility_Time_AddHour(dest,3);//��3Сʱ
        
        
        break;
      case 8:
        dest[4]=0;
        dest[3] -= dest[3]%6;
        Utility_Time_AddHour(dest,6);//��6Сʱ
        
        
        break; 
      case 9:
        dest[4]=0;
        dest[3] -= dest[3]%12;
        Utility_Time_AddHour(dest,12);//��12Сʱ
        
        
        break;
        
      case 10:
        dest[4]=0;
        dest[3]=8;
        Utility_Time_AddDay(dest,1);//��1��  
        break;
        
//  ��������,���ǲ���Ҫ�ڼ�ģʽ���ֵǰ����,
//  ����֮��,����Ҫ����.
//  ��Ϊ�����Ǳ䶯��, ���Ǽ��ֵ��������.        
//  ���������� �Ƚϸ���,��Ϊ����1Ϊ��ʼ.
    
    case 11:
        dest[4]=0;
        dest[3]=8;
        dest[2]-=(dest[2]-1)%2;
        //++dest[2];//������1��ʼ
        Utility_Time_AddDay(dest,2);//��2��    
        dest[2]-=(dest[2]-1)%2;
        //++dest[2];//������1��ʼ
        break;
      case 12: 
        dest[4]=0;
        dest[3]=8;
        dest[2]-=(dest[2]-1)%3;
        //++dest[2];//������1��ʼ
        Utility_Time_AddDay(dest,3);//��3��
        dest[2]-=(dest[2]-1)%3;
        //++dest[2];//������1��ʼ
        
        break;
      case 13:
        
        dest[4]=0;
        dest[3]=8;
        dest[2]-=(dest[2]-1)%5;
        //++dest[2];//������1��ʼ 
        
        Utility_Time_AddDay(dest,5);//��5��
        dest[2]-=(dest[2]-1)%5;
        //++dest[2];//������1��ʼ
        break;
      case 14:
        
        dest[4]=0;
        dest[3]=8;
        dest[2]-=(dest[2]-1)%10;
        //++dest[2];//������1��ʼ
        Utility_Time_AddDay(dest,10);//��10��
        dest[2]-=(dest[2]-1)%10;
        //++dest[2];//������1��ʼ
        
        break;
      case 15:
        
        dest[4]=0;
        dest[3]=8;
        dest[2]-=(dest[2]-1)%15;
        //++dest[2];
        Utility_Time_AddDay(dest,15);//��15��
        dest[2]-=(dest[2]-1)%15;
        //++dest[2];
        
        break;
      case 16:
        dest[4]=0;
        dest[3]=8;
        dest[2]=1;
        Utility_Time_AddMonth(dest,1);//��1����
        break;
      default:
        //Ҫ��ģʽ������.
        //���Ǿ�Ĭ��Ϊ5���ӱ���һ��
        Utility_Time_AddMinute(dest,5);
    }
}
void Utility_CalculateNextCheckTimeBytes(char *dest)//���ݵ�ǰʱ��,������һ�μ��ʱ��
{//���ʱ�� 5����
    for(int i=0;i<5;++i)
    {
        dest[i]=g_rtc_nowTime[i];
    }
    //�������������5����.
    dest[4] -= dest[4]%5;
    Utility_Time_AddMinute(dest,5);
}
int  Utility_Is_A_CheckTime(char *  _time)
{//����Ϊ5�ı��� ���Ǽ��ʱ��
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
//  01:  5����   02:  10����  03: 20����   04:  30����  
//  05:  1Сʱ   06:   2Сʱ  07:  3Сʱ   08:  6Сʱ  
//  09:  12Сʱ  10:    1��   11:    2��   12:   3��  
//  13:    5��   14:    10��  15:  15��    16:  1����  
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
    {//û����ģʽ �͵�5����һ�����.
        _mode = 0;
    }
    else
        _mode= (_temp[0]-'0')*10+ _temp[1] - '0'; 
//  01:  5����   02:  10����  03: 20����   04:  30����  
//  05:  1Сʱ   06:   2Сʱ  07:  3Сʱ   08:  6Сʱ  
//  09:  12Сʱ  10:    1��   11:    2��   12:   3��  
//  13:    5��   14:    10��  15:  15��    16:  1����  
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
    {//û����ģʽ �͵�5����һ�����.
        _mode = 0;
    }
    else
        _mode= (_temp[0]-'0')*10+ _temp[1] - '0'; 
//  01:  5����   02:  10����  03: 20����   04:  30����  
//  05:  1Сʱ   06:   2Сʱ  07:  3Сʱ   08:  6Сʱ  
//  09:  12Сʱ  10:    1��   11:    2��   12:   3��  
//  13:    5��   14:    10��  15:  15��    16:  1����  
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
//����Լ��,_buffer���Ѿ���д��ͷ��
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
