//////////////////////////////////////////////////////
//     �ļ���: uart1.c
//   �ļ��汾: 1.0.0
//   ����ʱ��: 09��11��30��
//   ��������:  
//       ����: ����
//       ��ע: ��
//
//////////////////////////////////////////////////////

#include "msp430common.h"
#include "common.h"
#include "uart1.h"
#include "uart0.h"
#include "uart3.h"
#include "led.h"
#include "stdio.h"
/************A1********/
#define TXD3 BIT6
#define RXD3 BIT7

//char testbuf[2];
//int testcount=0;

char * UART1_Tx_Buf=NULL; 
char UART1_Rx_Buffer[UART1_MAXIndex][UART1_MAXBUFFLEN]; //  ���ݴ洢�� 
int  UART1_Rx_BufLen[UART1_MAXIndex];                   //  ÿ�н��յ������ݳ���  
int  UART1_Rx_INTIndex=0;                               //  �жϸ�д����λ��
int  UART1_Rx_INTLen=0;                                 //  �жϸ�д���еĵڼ����ַ� 
int  UART1_Rx_RecvIndex=0;                              //  ��ǰ�ö�����λ�� 

unsigned int UART1_Tx_Flag=0;
unsigned int UART1_Tx_Len=0;
extern int WIFI_Inited_Flag;


//ָʾ��ǰ���͵�
static int s_uart1_type=0;  

int UART1_Open(int  _type)
{
    s_uart1_type = _type;
    
    //����rs232  ��ƽת����·
 //   P4DIR |= BIT0;
 //   P4OUT |= BIT0;
    
 //   UCTL1 = 0x00;
 //   UCTL1 &=~ SWRST; 
 //   UCTL1 |= CHAR;
 //   UTCTL1 = 0X00;	
    //115200��  XT2=8000000   SMCLK
    //UTCTL1=SSEL1; UBR0_1 = 0x45; UBR1_1 = 0x00; UMCTL1 = 0x4A;
    
    //  9600,    XT2=8000000   SMCLK
 //   UTCTL1=SSEL1;UBR0_1 = 0x41;UBR1_1 = 0x03;UMCTL1 = 0x00;
     
 //   UART1_ClearBuffer();
        
 //   ME2 |= UTXE1+URXE1;   //ʹ��UART1��TXD��RXD  
 //   IE2 |= URXIE1+UTXIE1; //ʹ��UART1��RX��TX�ж�  
    
 //   P3SEL |= BIT6;//����P3.6ΪUART1��TXD 
 //   P3SEL |= BIT7;//����P3.7ΪUART1��RXD
 //   P3DIR |= BIT6;//P3.6Ϊ����ܽ�   
//     if(s_uart1_type==1)
////    {
//        P10DIR |= BIT0;             //ly P100���ߣ�uart1���ڵ��ԣ��͵Ļ�P104��,105����485��
//        P10OUT |= BIT0;  
////    }
////    else
////    {
//        P10DIR |= BIT0;             //ly P100���ߣ�uart1���ڵ��ԣ��͵Ļ�P104��,105����485��
//        P10OUT &=~ BIT0;
////    }
   
 
  UCA1CTL1 |= UCSWRST;
  UCA1CTL1 |= UCSSEL1;   //smclk 1M 
  
  UCA1BR0 = 8;
  UCA1BR1 = 0;
  UCA1MCTL |= UCBRF_0+UCBRS_6;
  UCA1CTL1 &= ~UCSWRST;
  
  UART1_ClearBuffer();
  /********************/
  P5DIR |= TXD3;
  P5SEL |= TXD3 + RXD3;
  
  /*2418 UC1IE UCA1RXIE 5438 UCA1IE UCRXIE*/
  UCA1IE |= UCRXIE;//���ܴ��ڽ����ж�          

  return 0;
}
void UART1_Open_9600(int _type)
{
  s_uart1_type = _type;
    
    //����rs232  ��ƽת����·
 //   P4DIR |= BIT0;
 //   P4OUT |= BIT0;
    
 //   UCTL1 = 0x00;
 //   UCTL1 &=~ SWRST; 
 //   UCTL1 |= CHAR;
 //   UTCTL1 = 0X00;	
    //115200��  XT2=8000000   SMCLK
    //UTCTL1=SSEL1; UBR0_1 = 0x45; UBR1_1 = 0x00; UMCTL1 = 0x4A;
    
    //  9600,    XT2=8000000   SMCLK
 //   UTCTL1=SSEL1;UBR0_1 = 0x41;UBR1_1 = 0x03;UMCTL1 = 0x00;
     
  //  UART1_ClearBuffer();
        
  //  ME2 |= UTXE1+URXE1;   //ʹ��UART1��TXD��RXD  
 //   IE2 |= URXIE1+UTXIE1; //ʹ��UART1��RX��TX�ж�  
    
 //   P3SEL |= BIT6;//����P3.6ΪUART1��TXD 
 //   P3SEL |= BIT7;//����P3.7ΪUART1��RXD
 //   P3DIR |= BIT6;//P3.6Ϊ����ܽ�    
  UCA1CTL1 |= UCSWRST;
  UCA1CTL1 |= UCSSEL1;   //smclk 1M 
  
  //104��Ӧ9600�����ʣ�52��Ӧ19200? 8��Ӧ115200
  UCA1BR0 = 104;
  UCA1BR1 = 0;
  UCA1MCTL |= UCBRF_0+UCBRS_6;//
  //UCA1MCTL = 10;
  UCA1CTL1 &= ~UCSWRST;
  
  UART1_ClearBuffer();
  /********************/
  P5DIR |= TXD3;
  P5SEL |= TXD3 + RXD3;
  
  /*2418 UC1IE UCA1RXIE 5438 UCA1IE UCRXIE*/
  UCA1IE |= UCRXIE;
}
void UART1_Close()
{ 
   //�ر�RS232��ƽת����·
  //P4DIR |= BIT0;
  //P4OUT &= ~BIT0; 
   
   UART1_ClearBuffer(); 
   //�رմ���1
   
/*2418 UC1IE UCA1RXIE 5438 UCA1IE UCRXIE*/
   UCA1IE &= ~UCRXIE;	 

}

void UART1_ClearBuffer()
{
    DownInt();//���ж�
    
    UART1_Tx_Buf=0;
    UART1_Rx_INTIndex=0;
    UART1_Rx_INTLen=0;
    UART1_Rx_RecvIndex=0;
  
    UART1_Tx_Flag=0;
    UART1_Tx_Len=0; 
    
    for(int i=0;i<UART1_MAXIndex;++i){
      UART1_Rx_BufLen[i]=0;
    }
    
    UpInt();//���ж�
}
int  UART1_Send(char * _data ,int _len, int _CR)
{

    if(UART1_Tx_Flag!=0)//�ȴ���һ�η��ͽ���
    {//�͵�500ms 
        System_Delayms(500);
   
        UART1_Tx_Flag=0;//ǿ������Ϊ0;
    }
    if(_len>1)
    {
        //��ȫ�ֱ�����ֵ
        UART1_Tx_Buf=_data; //�����һ�������ж� ����Ϊ0
        UART1_Tx_Len=_len; //�����һ�������ж� ����Ϊ0,�෢���һ����������
        UART1_Tx_Flag=1; //���������������һ�����ݵ��ж���������Ϊ0��  
       for(int i=0;i<UART1_Tx_Len;i++)
       {
         
         /*2418 UC1IFG UCA1TXIFG 5438 UCA1IFG UCTXIFG*/
        while (!(UCA1IFG&UCTXIFG));
            UCA1TXBUF =_data[i];
       }
        UART1_Tx_Flag=0;
    }
    if(_len==1)//1���ַ���ʱ�� ���жϷ��� �޷��ɹ�.��ʱ�Ȼ��ɲ�ѯ����.�Ժ���о�
    {
      /*2418 UC1IFG UCA1TXIFG 5438 UCA1IFG UCTXIFG*/
        while (!(UCA1IFG&UCTXIFG));
        UCA1TXBUF = _data[0];
        
    }
    if(_CR)
    {//����һ������
      /*2418 UC1IFG UCA1TXIFG 5438 UCA1IFG UCTXIFG*/
        while (!(UCA1IFG&UCTXIFG));
        UCA1TXBUF=13;
        /*2418 UC1IFG UCA1TXIFG 5438 UCA1IFG UCTXIFG*/
        while (!(UCA1IFG&UCTXIFG));
        UCA1TXBUF=10;
        
    }
    if(_CR)
    {//����һ������
      /*2418 UC1IFG UCA1TXIFG 5438 UCA1IFG UCTXIFG*/
        while (!(UCA1IFG&UCTXIFG));
        UCA1TXBUF=13;
        /*2418 UC1IFG UCA1TXIFG 5438 UCA1IFG UCTXIFG*/
        while (!(UCA1IFG&UCTXIFG));
        UCA1TXBUF=10;
        
    }
    return 0;
}

//����
int  UART1_SendtoInt(int num)
{
/*2418 UC1IFG UCA1TXIFG 5438 UCA1IFG UCTXIFG*/
        while (!(UCA1IFG&UCTXIFG));
            UCA1TXBUF =num;
          System_Delayms(2000);
    return 0;
}

int  UART1_RecvLine(char * _dest ,int _max, int * _pNum)
{
    int i=0;
    //�ö���λ�ó���Ϊ0, ��ѭ���ȴ� 
    while(UART1_Rx_BufLen[UART1_Rx_RecvIndex]==0);
    //�������ˣ��Ͱ����ݸ��Ƴ���, 
    for(i=0; ( i< _max) && ( i<UART1_Rx_BufLen[UART1_Rx_RecvIndex]); ++i)
    {
        _dest[i]=UART1_Rx_Buffer[UART1_Rx_RecvIndex][i];
    }
    *_pNum = UART1_Rx_BufLen[UART1_Rx_RecvIndex];
    //������Ϻ�,�ͰѸ�λ�õĳ�������Ϊ0,�����жϿ��Ը�����.
    UART1_Rx_BufLen[UART1_Rx_RecvIndex]=0;
    //��λ����һ��
    // ��� ����9 �ͼ�ȥ9 ,�ӵ�һ�п�ʼ,����ͼ�������.
    if( UART1_Rx_RecvIndex >= UART1_MAXIndex -1)
        UART1_Rx_RecvIndex=0;
    else
        ++UART1_Rx_RecvIndex;
    return 0;
}

int  UART1_RecvLineTry(char * _dest,const int _max, int * _pNum)
{
    int i=0;
    //�ö���λ�ó���Ϊ0, ��ѭ���ȴ� 

    if(UART1_Rx_BufLen[UART1_Rx_RecvIndex]==0)
    {
        return -1;
    }

    TraceInt4(UART1_Rx_RecvIndex,1);
    TraceInt4(UART1_Rx_BufLen[UART1_Rx_RecvIndex],1);
    //�������ˣ��Ͱ����ݸ��Ƴ���, 
    for(i=0; ( i< _max) && ( i<UART1_Rx_BufLen[UART1_Rx_RecvIndex]); ++i)
    {
        _dest[i]=UART1_Rx_Buffer[UART1_Rx_RecvIndex][i];
    }
    *_pNum =UART1_Rx_BufLen[UART1_Rx_RecvIndex];
    //������Ϻ�,�ͰѸ�λ�õĳ�������Ϊ0,�����жϿ��Ը�����.
    UART1_Rx_BufLen[UART1_Rx_RecvIndex]=0;
    //��λ����һ��
    // ��� ����9 �ͼ�ȥ9 ,�ӵ�һ�п�ʼ,����ͼ�������.
    if( UART1_Rx_RecvIndex >= UART1_MAXIndex -1)
        UART1_Rx_RecvIndex=0;
    else
        ++UART1_Rx_RecvIndex;
    return 0;
}
int  UART1_RecvLineWait(char *_dest ,const int _max, int * _pNum)
{
    int i=0; 
    //�ö���λ�ó���Ϊ0, ��ѭ���ȴ� 
    while(UART1_Rx_BufLen[UART1_Rx_RecvIndex]==0)
    {
        System_Delayms(30);
        ++i;
        if(i>10) 
            return -1;
    }
    //�������ˣ��Ͱ����ݸ��Ƴ���, 
    for(i=0; ( i< _max) && ( i<UART1_Rx_BufLen[UART1_Rx_RecvIndex]); ++i)
    {
        _dest[i]=UART1_Rx_Buffer[UART1_Rx_RecvIndex][i];
    }
    *_pNum = UART1_Rx_BufLen[UART1_Rx_RecvIndex];
    //������Ϻ�,�ͰѸ�λ�õĳ�������Ϊ0,�����жϿ��Ը�����.
    UART1_Rx_BufLen[UART1_Rx_RecvIndex]=0;
    //��λ����һ��
    // ��� ����9 �ͼ�ȥ9 ,�ӵ�һ�п�ʼ,����ͼ�������.
    if( UART1_Rx_RecvIndex >= UART1_MAXIndex -1)
        UART1_Rx_RecvIndex=0;
    else
        ++UART1_Rx_RecvIndex;
    return 0;
}
int  UART1_RecvLineLongWait(char *_dest,int _max, int * _pNum)
{
    int i=0;
    //�ö���λ�ó���Ϊ0, ��ѭ���ȴ� 
    while(UART1_Rx_BufLen[UART1_Rx_RecvIndex]==0)
    {//�ȴ�5��.
        System_Delayms(50);
        ++i;
        if(i>100)
            return -1;        
    }
    //�������ˣ��Ͱ����ݸ��Ƴ���, 
    for(i=0; ( i< _max) && ( i<UART1_Rx_BufLen[UART1_Rx_RecvIndex]); ++i)
    {
        _dest[i]=UART1_Rx_Buffer[UART1_Rx_RecvIndex][i];
    }
    *_pNum = UART1_Rx_BufLen[UART1_Rx_RecvIndex];
    //������Ϻ�,�ͰѸ�λ�õĳ�������Ϊ0,�����жϿ��Ը�����.
    UART1_Rx_BufLen[UART1_Rx_RecvIndex]=0;
    //��λ����һ��
    // ��� ����9 �ͼ�ȥ9 ,�ӵ�һ�п�ʼ,����ͼ�������.
    if( UART1_Rx_RecvIndex >= UART1_MAXIndex -1)
        UART1_Rx_RecvIndex=0;
    else
        ++UART1_Rx_RecvIndex;
    return 0;
}

void Judge_Watermeter()
{
    if(UART1_Rx_INTLen == 9)
    {
        UART1_Rx_BufLen[UART1_Rx_INTIndex] = UART1_Rx_INTLen;
        UART1_Rx_INTLen=0;
        if(UART1_Rx_INTIndex >= UART1_MAXIndex-1)
            UART1_Rx_INTIndex=0;
        else
            ++UART1_Rx_INTIndex;
    }
}


//
//    ���ܵ�һ���ַ�.
//    
//    װ�ص�UART1_Rx_Buffer[UART1_Rx_INTIndex][UART1_Rx_INTLen]��.
//    ������UART1_Rx_INTLen ;���UART1_Rx_INTLen�Ѿ������һ�������ַ�Ϊ����
//    ��д���л���ĳ���Ϊ UART1_Rx_INTLen+1;
//    ����UART1_Rx_INTIndex,ָ����һ��������. �����һ����������������δ������.
//    ��ô�Ͳ�����,������д��ǰ�Ļ�����.
//    
//    
//    
/*************VECTOR*/
#pragma vector=USCI_A1_VECTOR 
__interrupt void UART1_RX_ISR(void)   //�����յ����ַ���ʾ���������
{
   //_DINT();
   char _temp; 
      //char *tbuffer;  
   _temp = UCA1RXBUF;
#if 1

    UART1_Rx_Buffer[UART1_Rx_INTIndex][UART1_Rx_INTLen]=_temp;
    ++UART1_Rx_INTLen;
    
    if(s_uart1_type == UART3_CONSOLE_TYPE)
    {   
      
        if(((_temp==0x0A) && (UART1_Rx_INTLen!=0) && (UART1_Rx_Buffer[UART1_Rx_INTIndex][UART1_Rx_INTLen-2]==0x0D)) || (_temp == ')'))
        {
            //�����ͷ���յ���������з���,ֱ������
            if(UART1_Rx_INTLen==1)
            {
                UART1_Rx_INTLen=0; //���¿�ʼ���� 
                return ;
            }
            else
            {
                //   ��λ����һ�� 
                //UART1_Rx_Buffer[UART1_Rx_INTIndex][UART1_Rx_INTLen-1]=13; //������ø�13
                UART1_Rx_BufLen[UART1_Rx_INTIndex] = UART1_Rx_INTLen - 2;//���������з�
                UART1_Rx_INTLen=0;
                if(UART1_Rx_INTIndex >= UART1_MAXIndex-1)
                    UART1_Rx_INTIndex=0;
                else
                    ++UART1_Rx_INTIndex;
                //UART1_Rx_INTIndex += UART1_Rx_INTIndex < (UART1_MAXIndex - 1) ? 1 : 1-UART1_MAXIndex;  
                return ;
            }
        }
    }
    else
    {
        Judge_Watermeter();
        return ;
    }
    
//    if(_temp==0x0A)
//        return ;
//
//    if((_temp==0x0D) && (UART1_Rx_INTLen!=0) && UART1_Rx_Buffer[UART1_Rx_INTIndex][UART1_Rx_INTLen-1]==0x0D)
//    {
//        return;
//    }
    
    if(UART1_Rx_INTLen >= UART1_MAXBUFFLEN-1)
    {//�г���������, ����ֱ�ӽضϳ�һ��.
        UART1_Rx_BufLen[UART1_Rx_INTIndex] = UART1_Rx_INTLen + 1;
        UART1_Rx_INTLen=0;
        if(UART1_Rx_INTIndex >= UART1_MAXIndex-1)
            UART1_Rx_INTIndex=0;
        else
            ++UART1_Rx_INTIndex;
    }
    
    //�жϻ������Ƿ�����:UART1_Rx_INTIndex��¼�´α�����������´������Ѿ��д洢��˵������������
    if( UART1_Rx_BufLen[UART1_Rx_INTIndex]!=0)
    {
        //��һ�л�δ���������Ǿ͸������������һ��
        if(UART1_Rx_INTIndex <= 0)
            UART1_Rx_INTIndex = UART1_MAXIndex-1;
        else
            --UART1_Rx_INTIndex;
        //�Ѹ��г�������Ϊ0,���ж�ռ��
        UART1_Rx_BufLen[UART1_Rx_INTIndex]=0;
    }

 #endif
}

//
//int putchar(int c)
//{
//    while (!(UCA1IFG&UCTXIFG));             // USCI_A0 TX buffer ready?
//    UCA1TXBUF = (unsigned char)c; 
//  
//    return c;
//}