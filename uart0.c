//////////////////////////////////////////////////////
//     文件名: uart0.c
//   文件版本: 1.0.0
//   创建时间: 09年11月30日
//   更新内容:  
//       作者: 林智
//       附注: 无
//
//////////////////////////////////////////////////////
 
#include "msp430common.h"
#include "uart0.h"
#include "uart3.h"
#include "led.h"
#include "common.h"
#include "dtu.h"
//#include "hydrology.h"
/************A0********/
#define TXD0 BIT4
#define RXD0 BIT5


//接收缓冲区 队列，每行数据最多MAXBUFFLEN个字符
//共可以接收 5条
// 中断处理程序 
//  负责写的变量:  1.缓冲区 2.每行长度（获取一整行了后,把0改写为长度)
//                 2.当前该填写的位置(获得一行后,就把位置定到下一行)
//  负责读的变量:  1.每行长度(如果不是0,表示数据未处理,就表明出现了溢出)        
//                 2.当前该填写的位置,向该行写数据.
//
// 接收程序
//  负责写的变量:  1.每行的长度,读取完的时候把它设置为0,
//                                中断程序就知道这行可以覆盖 
//                 2.读的位置, 读完一行,定位到下一行,下次可以继续读 
//  负责读的变量:  1.该读的位置,  2.该行的长度 
//   
//短信输入状态变量
int IsGetInput=0;
//发送缓冲区由调用者提供
char * UART0_Tx_Buf=NULL; 
char UART0_Rx_Buffer[UART0_MAXIndex][UART0_MAXBUFFLEN];//  数据存储区 
int  UART0_Rx_BufLen[UART0_MAXIndex];            //  每行接收到的数据长度  
int UART0_Rx_INTIndex=0;                //  中断该写的行位置
int UART0_Rx_INTLen=0;                  //  中断该写该行的第几个字符 
int UART0_Rx_RecvIndex=0;               //  当前该读的行位置 
unsigned int UART0_Tx_Flag=0;
unsigned int UART0_Tx_Len=0;
int Take_Photo_Count=0;
extern char _receiveUart0[];
int uart0_count=0;
int real_count=0;
int flag_message=0;
int order_length=0;
//指示当前类型的
static int s_uart0_type=0; 
unsigned char test[3];


char uart0_test[2];
int ucount=0;



//串口0的初始化
void UART0_Open(int _type)
{
//    TraceMsg("uart0 open",1);
    s_uart0_type = _type;
 //   UCTL0 = 0x00;
 //   UCTL0 &=~ SWRST;	
    //数据位为8bit
 //   UCTL0 |= CHAR;	
    //将寄存器的内容清零
 //   UTCTL0 = 0X00;	  
    //  9600,    MCLK=32768,  ACLK, 错误率太高
    //UTCTL0=SSEL0; UBR0_0 = 0x03;  UBR1_0 = 0X00;  UMCTL0 = 0x4A; 
    //  2400,    MCLK=32768,  ACLK, 数据出现丢失, 2400太慢了    
    // UTCTL0=SSEL0; UBR0_0 = 0x0D; UBR1_0 = 0X00; UMCTL0 = 0x6D;
    //  4800,    MCLK=32768,  ACLK, 数据不稳定  ,
    //UTCTL0=SSEL0; UBR0_0 = 0x06;  UBR1_0 = 0X00;  UMCTL0 = 0xFB; 
   
    // 57600,    XT2=8000000   SMCLK
    //UTCTL0=SSEL1;UBR0_0 = 0x8A;UBR1_0 = 0x00;UMCTL0 = 0xDE;  
    //  9600,    XT2=8000000   SMCLK
    //UTCTL0=SSEL1;UBR0_0 = 0x41;UBR1_0 = 0x03;UMCTL0 = 0x00;  原程序
    // 19200，   XT2=8000000   SMCLK
    //UTCTL0=SSEL1;UBR0_0 = 0xA0;UBR1_0 = 0x01;UMCTL0 = 0xC0;
    //115200，   XT2=8000000   SMCLK
 //   UTCTL0=SSEL1;UBR0_0 = 0x45;UBR1_0 = 0x00;UMCTL0 = 0x4A;
   
    
 //   UART0_ClearBuffer();//一些参数 要归0
        
 //   ME1 |= UTXE0 + URXE0; //使能UART0的TXD和RXD
 //   IE1 |= URXIE0;//使能UART0的RX中断
 //   IE1 |= UTXIE0;//使能UART0的TX中断 
//    P3SEL |= BIT4;//设置P3.4为UART0的TXD	 
//    P3SEL |= BIT5;//设置P3.5为UART0的RXD	 
//    P3DIR |= BIT4;//P3.4为输出管脚   
//    return ;

  //GSM/GPRS模块通信时候，是TTL电平，直接有CPU发送给模块TTL电平信号，无须转换电平 
      
  UCA0CTL1 |= UCSWRST;       //置位UCSWRST位，使UART模块的寄存器处于初始状态
  UCA0CTL1 |= UCSSEL1;       //UCLK=SMCLK=1MHz;
/*
  通过设置三个与波特率有关的寄存器，即波特率整数  UxBR1、UxBR0、和波特率修整寄存器UxMCTL，
  使用以下公式计算波特率控制参数：
  波特率参数（UxBR0，UxBR1）=波特率时钟源频率/所需波特率 。（结果取整数部分）
  例如：波特率时钟源频率=8MHz，所需波特率=115200，那么波特率参数UxBR0，UxBR1就是8000KHz/115.2KHz=69.444444444，取整后为69，小数部分0.444444444作为修整寄存器中的参数，
  使用如下公式计算：
  波特率修整参数（UxMCTL）=8 x 小数部分（结果取整数部分）
  那么，应该是8 x 0.444444444=3.5，那么这里可以取3或4，这里取4 。
  因此，需要4个脉冲插补位，在UxMCTL寄存器中，任意选择4位后，UxMCTL=0x55。注意，尽量将插补脉冲分散放置到UxMCTL8个位中。
  至此，串口波特率参数设置完成。

  TI MSP430系列单片机，usart模块的波特率值设定是通过以下三个寄存器决定的：UxBR0,UxBR1,UxMCTL

  波特率＝BRCLK/N ,主要是计算出N。

  BRCLK:时钟源，可以通过寄存器设定何为时钟源; 通过寄存器UCAxCTL1的SSEL两位选择，01:ACLK,02:SMCLK

  N:波特率产生的分频因子。N=UxBR1+UxBR0+UxMCTL,其中UxBR1+UxBR0为整数部分，UxMCTL为设定小数部分，
  其中 UxBR1为高位，UxBR0为低位，两者结合起来为一个16位的字。

  举例说明：波特率＝115200，时钟源＝8MHz ，为外部晶体振荡器

  N=8000000/115200＝69.44 。很明显：UxBR1+UxBR0＝13，即UxBR1＝0，UxBR0＝0x45，首先把小数部分0.44 ×8，
  即3.52，取整后为3。这个3表示在UxMCTL中的8位里去掉最后一位的前7位值为3，并且，UxMCTL分为First Stage Modulation和
  Second Stage Modulation，也就是前者为高4位，取值范围0-F，后者为低4位，3对应00000110,也即0x06=UCBRS0+USBRS1，。

  对于本程序而言，使用子时钟SMLK 1M，也即1000000
  GTM900有波特率，300,600,1200,2400,4800,9600,14400,19200,38400,57600,115200;
  1000000/115200=8.7, 0.7*8=5.6, 取整为5，UxBRS为 101, UCA0MCTL总的为00001010 = 0x0A;
  1000000/57600=17.36111, 0.36111*8=2.888，取整为2，UxBRS为010,总的为00000100 = 0x08;
  1000000/38400=26.0417, 0.0417*8=0.333，取整为0，UxBRS为000,总的为00000000 = 0x00;
  1000000/19200=52.08, 0.08*8=0.64，取整为0，UxBRS为000,总的为00000000 = 0x00;
  1000000/14400=69.44, 0.44*8=3.5，取整为3，UxBRS为011,总的为00000110 = 0x06;
  1000000/9600=104.166, 0.166*8=1.3，取整为1，UxBRS为001,总的为00000010 = 0x02;
  1000000/4800=208.333, 0.333*8=2.6，取整为2，UxBRS为010,总的为00000100 = 0x04;

  */  
  
  UCA0BR0 = 8;
  UCA0BR1 = 0;
//  UCA0MCTL |= UCBRF_0+UCBRS_6; //龚恩照
  UCA0MCTL = 0x0A;
  UCA0CTL1 &= ~UCSWRST;
  
  UART0_ClearBuffer();
  /********************/
  P3DIR |= TXD0;
  P3SEL |= TXD0 + RXD0;         //发送和接收引脚为第2功能
  /*2418 IE2 UCA0RXIE 5438 UCA0IE UCRXIE*/
  UCA0IE |= UCRXIE;           //接收中断使能

  
}

void UART0_Open_9600(int _type)
{
    TraceMsg("uart0 open",1);
    s_uart0_type = _type;

  //GSM/GPRS模块通信时候，是TTL电平，直接有CPU发送给模块TTL电平信号，无须转换电平 
      
  UCA0CTL1 |= UCSWRST;       //置位UCSWRST位，使UART模块的寄存器处于初始状态
  UCA0CTL1 |= UCSSEL1;       //UCLK=SMCLK=1MHz;
  
  UCA0BR0 = 104;
  UCA0BR1 = 0;
  //UCA0MCTL |= UCBRF_0+UCBRS_0; 龚恩照
  UCA0MCTL = 2;
  UCA0CTL1 &= ~UCSWRST;
  UART0_ClearBuffer();
  /********************/
  P3DIR |= TXD0;
  P3SEL |= TXD0 + RXD0;         //发送和接收引脚为第2功能
  /*2418 IE2 UCA0RXIE 5438 UCA0IE UCRXIE*/
  UCA0IE |= UCRXIE;           //接收中断使能

  
}

/*未使用*/
void Wifi_Uart0_Open()//给wifi调试用
{
  //开电平转换电路, WIFI测试时候需要向外发送数据，需要RS232电平，而于GSM/GPRS模块通信时候，是TTL电平
   
  /****************/
   P4DIR |= BIT0;   //
   P4OUT |= BIT0;  
    
  UCA0CTL1 |= UCSWRST;       //置位UCSWRST位，使UART模块的寄存器处于初始状态
  UCA0CTL1 |= UCSSEL1;       //UCLK=SMCLK=1MHz;
//  UCA0BR0 = 52;
//  UCA0BR1 = 0;
//  UCA0MCTL |= UCBRF_0+UCBRS_0;
  
    
  UCA0BR0 = 8;
  UCA0BR1 = 0;
  UCA0MCTL |= UCBRF_0+UCBRS_6;
  
  
  UCA0CTL1 &= ~UCSWRST;
  UART0_ClearBuffer();
  P3DIR |= TXD0;
  P3SEL |= TXD0 + RXD0;         //发送和接收引脚为第2功能
  
  //IE2 |= UCA0RXIE;           //接收中断使能
  
  /*2418 UC0IE UCA0RXIE 5438 UCA0IE UCRXIE*/
  UCA0IE |= UCRXIE;
  
}


void UART0_Close()
{//关闭UART0的接收和 发送功能

//   TraceMsg("uart0 close",1);
  /*2418 UC0IE UCA0RXIE 5438 UCA0IE UCRXIE*/
   UCA0IE &= ~UCRXIE;	 
 //  IE2 &= ~UCA0TXIE;
   
   //一些参数 要归0 
   UART0_ClearBuffer();
} 


void UART0_ClearBuffer()
{
    DownInt();//关中断
    
    UART0_Tx_Buf=0;
    UART0_Rx_INTIndex=0;
    UART0_Rx_INTLen=0;
    UART0_Rx_RecvIndex=0;
  
    UART0_Tx_Flag=0;
    UART0_Tx_Len=0;
    IsGetInput=0;
    for(int i=0;i<UART0_MAXIndex;++i)
        UART0_Rx_BufLen[i]=0;
    
    UpInt();//开中断
}


int UART0_WaitInput()
{
    int temp=0;
    while(IsGetInput!=1)
    {
        System_Delayms(150);// 10 * 100 ms( 1.5s内未收到)
        if(temp++ >10)
            return -1;
    } 
    IsGetInput=0;// 等到了 return 0
    return 0;
} 
int UART0_ClearInput()
{
    IsGetInput=0;
    return 0;
}
int UART0_Send(char * data ,int len, int CR)
{
    
    if(UART0_Tx_Flag!=0)//等待上一次发送结束
    {//就等500ms 
        System_Delayms(500);
        UART0_Tx_Flag=0;//强制设置为0;
    }
    //给全局变量赋值
    UART0_Tx_Buf=data; //由最后一次数据中断 设置为0
    UART0_Tx_Len=len; //由最后一次数据中断 设置为0,多发最后一个结束符号
    UART0_Tx_Flag=1;  
    for(int i=0;i<UART0_Tx_Len;i++){     
      while (!(UCA0IFG&UCTXIFG));
        UCA0TXBUF = UART0_Tx_Buf[i];
    }
    UART0_Tx_Flag=0;
    if(!CR)
    {
        return 0;
    }    
    //AT指令补发一个换行
    if(s_uart0_type==UART0_GSM_TYPE)
    {//AT指令认为13就为换行
        char _cr1=13;
        UART0_Tx_Buf=&_cr1;
        UART0_Tx_Len=1;
    }
    else
    {//DTU认为 13 10为换行
        char _cr2[2];
        _cr2[0]=13;
        _cr2[1]=10;
        UART0_Tx_Buf=_cr2;
        UART0_Tx_Len=2;
    } 
    UART0_Tx_Flag=1; 
    for(int i=0;i<UART0_Tx_Len;i++){
      /*2418 UC0IFG UCA0TXIFG 5438 UCA0IFG UCTXIFG*/
        while (!(UCA0IFG&UCTXIFG));
        UCA0TXBUF = UART0_Tx_Buf[i];
    }
    UART0_Tx_Flag=0;
    return 0;
}

int GPRS_RecvLineTry(char *dest, int maxNum, int * pNum)
{
    int i=0;
    if(UART0_Rx_BufLen[UART0_Rx_RecvIndex]==0)
        return -1;
    //有数据了，就把数据复制出来, 
    for(i=0;(i<maxNum)&&( i<UART0_Rx_BufLen[UART0_Rx_RecvIndex]);++i)
    {
        dest[i]=UART0_Rx_Buffer[UART0_Rx_RecvIndex][i];
    }
    //i个字符，最后一个下标是i-1
    dest[i-1]='\0';
    *pNum=i-1;
    return 0;
}

int UART0_RecvLine(char *dest ,int maxNum, int * pNum)
{
    int i;
    //该读的位置长度为0, 则循环等待 
    while(UART0_Rx_BufLen[UART0_Rx_RecvIndex]==0);
    //有数据了，就把数据复制出来, 
    for(i=0;(i<maxNum)&&( i<UART0_Rx_BufLen[UART0_Rx_RecvIndex]);++i)
    {
        dest[i]=UART0_Rx_Buffer[UART0_Rx_RecvIndex][i];
    }
    //i个字符，最后一个下标是i-1
    dest[i-1]='\0';
    *pNum=i;
    //复制完毕后,就把该位置的长度设置为0,告诉中断可以覆盖了.
    UART0_Rx_BufLen[UART0_Rx_RecvIndex]=0;
    //定位到下一行
    // 如果 等于9 就减去9 ,从第一行开始,否则就继续自增.
    UART0_Rx_RecvIndex += UART0_Rx_RecvIndex <  (UART0_MAXIndex-1) ? 1 : 1-UART0_MAXIndex; 
    return 0;
}  
int UART0_RecvLineTry(char *dest, int maxNum, int * pNum)
{
    int i=0;
    if(UART0_Rx_BufLen[UART0_Rx_RecvIndex]==0)
        return -1;
    //有数据了，就把数据复制出来, 
    for(i=0;(i<maxNum)&&( i<UART0_Rx_BufLen[UART0_Rx_RecvIndex]);++i)
    {
        dest[i]=UART0_Rx_Buffer[UART0_Rx_RecvIndex][i];
    }
    //i个字符，最后一个下标是i-1
    dest[i-1]='\0';
    *pNum=i-1;
    //复制完毕后,就把该位置的长度设置为0,告诉中断可以覆盖了.
    UART0_Rx_BufLen[UART0_Rx_RecvIndex]=0;
    //定位到下一行
    // 如果 等于MAX-1 就减去MAX-1 ,从第一行开始,否则就继续自增.
    UART0_Rx_RecvIndex += UART0_Rx_RecvIndex <  (UART0_MAXIndex-1) ? 1 : 1-UART0_MAXIndex;
    return 0;
}
int UART0_RecvLineWait(char *dest ,int maxNum, int * pNum)
{//最多300ms
    int i=0;
    int _waits;
    if(s_uart0_type==UART0_GSM_TYPE) 
        _waits=10;  //300ms
    else
        _waits=100; //3秒
    
    while(UART0_Rx_BufLen[UART0_Rx_RecvIndex]==0)//该读的位置长度为0, 则循环等待
    {
        System_Delayms(30);
        ++i;
        if( i > _waits)
        { //无数据
            return -1;
        }
    }
    //有数据了，就把数据复制出来,
    for(i=0;(i<maxNum)&&( i<UART0_Rx_BufLen[UART0_Rx_RecvIndex]);++i)
    {
        dest[i]=UART0_Rx_Buffer[UART0_Rx_RecvIndex][i];
    }
    //i个字符，最后一个下标是i-1
    dest[i-1]='\0';
    *pNum=i-1;
    //复制完毕后,就把该位置的长度设置为0,告诉中断可以覆盖了.
    UART0_Rx_BufLen[UART0_Rx_RecvIndex]=0;
    //定位到下一行
    // 如果 等于MAX-1 就减去MAX-1 ,从第一行开始,否则就继续自增.
    UART0_Rx_RecvIndex += UART0_Rx_RecvIndex <  (UART0_MAXIndex-1) ? 1 : 1-UART0_MAXIndex;
    return 0;
}

int New_UART0_RecvLineWait()
{//最多300ms
    int i=0;
    int _waits;
    if(s_uart0_type==UART0_GSM_TYPE) 
        _waits=10;  //300ms
    else
        _waits=100; //3秒
    
    while(uart0_count==0)//该读的位置长度为0, 则循环等待
    {
        System_Delayms(30);
        ++i;
        if( i > _waits)
        { //无数据
            return -1;
        }
    }
    return 0;
}


int UART0_RecvLineLongWait(char *dest, int maxNum, int * pNum)
{
    int i=0;
    int times1=0;
    //不同的模式的waits不一样.
    int _waits;
    if(s_uart0_type==UART0_GSM_TYPE) 
        _waits=60;  //3秒
    else
        _waits=200; //10秒
     
    //该读的位置长度为0, 则循环等待
    while(UART0_Rx_BufLen[UART0_Rx_RecvIndex]==0)
    { 
        System_Delayms(50);
        ++times1;
        if( times1 > _waits)
        {// 无数据
            return -1; 
        }
    }
    //有数据了，就把数据复制出来, 
    for(i=0;(i<maxNum)&&( i<UART0_Rx_BufLen[UART0_Rx_RecvIndex]);++i)
    {
        dest[i]=UART0_Rx_Buffer[UART0_Rx_RecvIndex][i];
    }
    //i个字符，最后一个下标是i-1
    dest[i-1]='\0';
    *pNum=i-1;
    //复制完毕后,就把该位置的长度设置为0,告诉中断可以覆盖了.
    UART0_Rx_BufLen[UART0_Rx_RecvIndex]=0;
    //定位到下一行
    // 如果 等于MAX-1 就减去MAX-1 ,从第一行开始,否则就继续自增.
    UART0_Rx_RecvIndex += UART0_Rx_RecvIndex <  (UART0_MAXIndex-1) ? 1 : 1-UART0_MAXIndex;
    return 0;
} 



//
//  接收中断 基本通过测试
//
//  特殊处理的:  
//     行头的 13 10 将被直接抛弃. 
// 
//UART0_Rx_Buffer[UART0_MAXIndex][UART0_MAXBUFFLEN];//  数据存储区 
//UART0_Rx_BufLen[UART0_MAXIndex];            //  每行接收到的数据长度  
//UART0_Rx_INTIndex=0;                //  中断该写的行位置
//UART0_Rx_INTLen=0;                  //  中断该写该行的第几个字符 
//UART0_Rx_RecvIndex=0;               //  当前该读的行位置 
/******************VECTOR****/
#pragma vector=USCI_A0_VECTOR

__interrupt void UART0_RX_ISR(void)   //将接收到的字符显示到串口输出
{
      
    char _temp;//%IPDATA:1,1,87
    
    _temp=UCA0RXBUF;
//     UCA1TXBUF=_temp;
//    UART3_Send(&_temp,1,0);//测试用的代码
#if 1
    //如果开头收到 > 符号,说明短信提示符的到来,
    //抛弃这个>,并设置全局标记
    // 由于受干扰时的 混乱, 我们不在检查是否是行头.
    // 由其它机制来尽量保证不会被数据中的 > 影响到. 
    if(_temp=='>')// && UART0_Rx_INTLen==0)
    {
        IsGetInput=1;
        return ;
    }
    //开头收到空格, 抛弃掉,认为是 > 后附带的 空格
    if(_temp==' ' && UART0_Rx_INTLen==0)
    {
        return ;
    } 
    //一般情况
    //判断是否是DTU状态.这个状态不接受0x1F以下的,除了换行 回车
    //if(s_uart0_type==UART0_DTU_TYPE&& _temp<=0x1F && _temp!=0x0A && _temp != 0x0D && _temp != 0x00)
    //{
    //    return ;
    //}
    if(UART0_Rx_INTLen >= UART0_MAXBUFFLEN-1)
    {//行长度溢出了, 我们直接截断成一行.
        UART0_Rx_Buffer[UART0_Rx_INTIndex][UART0_Rx_INTLen-1]=13; //随便设置个13
        UART0_Rx_BufLen[UART0_Rx_INTIndex]=UART0_Rx_INTLen; //不包括换行符
        UART0_Rx_INTLen=0;
        UART0_Rx_INTIndex += UART0_Rx_INTIndex < (UART0_MAXIndex - 1) ? 1 : 1-UART0_MAXIndex;
    }
    // 只认为13 10(0x0D 0x0A)是换行符号. 行长度不包括换行符号
    // (因此需要关闭回显,因为回显以13结尾)
    //
    if((_temp==0x0A) && (UART0_Rx_INTLen!=0) && (UART0_Rx_Buffer[UART0_Rx_INTIndex][UART0_Rx_INTLen-1]==0x0D))
    {
        //如果是头部收到的这个换行符号
        // 直接抛弃
        if(UART0_Rx_INTLen==1)
        {
            UART0_Rx_INTLen=0; //重新开始接收 
            return ;
        }
        else
        { 
            //换行处理
            //收到完整一行后的处理
            //   该行的长度设置为数据长度
            //   定位到下一行
            //   该写的字符位 设置为0
            //   换行符号处理为13
            UART0_Rx_Buffer[UART0_Rx_INTIndex][UART0_Rx_INTLen-1]=13; //随便设置个13
            UART0_Rx_BufLen[UART0_Rx_INTIndex]=UART0_Rx_INTLen; //不包括换行符
            UART0_Rx_INTLen=0;
            UART0_Rx_INTIndex += UART0_Rx_INTIndex < (UART0_MAXIndex - 1) ? 1 : 1-UART0_MAXIndex;  
//            JudgeServerDataArrived();
            return ;
        } 
    } 
    
    
    //判断是否会溢出
    if( UART0_Rx_BufLen[UART0_Rx_INTIndex]!=0)
    { 
        //下一行还未被处理，那就覆盖最后处理的这一行
        UART0_Rx_INTIndex -= UART0_Rx_INTIndex > 0 ? 1 : 1-UART0_MAXIndex; 
        //把该行长度设置为0,由中断占用
        UART0_Rx_BufLen[UART0_Rx_INTIndex]=0;
    }
    //一般情况 
    UART0_Rx_Buffer[UART0_Rx_INTIndex][UART0_Rx_INTLen]=_temp;
    ++UART0_Rx_INTLen;
#endif  

}


