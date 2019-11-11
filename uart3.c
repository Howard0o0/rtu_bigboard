//////////////////////////////////////////////////////
//     �ļ���: uart3.c
//   �ļ��汾: 1.0.0
//   ����ʱ��: 09��11��30��
//   ��������:  
//       ����: ����
//       ��ע: ��
//
//////////////////////////////////////////////////////

#include "msp430common.h"
#include "common.h"
#include "uart3.h"
#include "uart0.h"
#include "led.h"
#include "stdio.h"
#include "ioDev.h"
/************A1********/
#define TXD3 BIT4
#define RXD3 BIT5

char testbuf[2];
int testcount=0;

char * UART3_Tx_Buf=NULL; 
char UART3_Rx_Buffer[UART3_MAXIndex][UART3_MAXBUFFLEN]; //  ���ݴ洢�� 
int  UART3_Rx_BufLen[UART3_MAXIndex];                   //  ÿ�н��յ������ݳ���  
int  UART3_Rx_INTIndex=0;                               //  �жϸ�д����λ��
int  UART3_Rx_INTLen=0;                                 //  �жϸ�д���еĵڼ����ַ� 
int  UART3_Rx_RecvIndex=0;                              //  ��ǰ�ö�����λ�� 

unsigned int UART3_Tx_Flag=0;
unsigned int UART3_Tx_Len=0;
extern int WIFI_Inited_Flag;


//ָʾ��ǰ���͵�
static int s_uart3_type=0;  

int UART3_Open(int  _type)
{
    s_uart3_type = _type;
    
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
     
 //   UART3_ClearBuffer();
        
 //   ME2 |= UTXE1+URXE1;   //ʹ��UART3��TXD��RXD  
 //   IE2 |= URXIE1+UTXIE1; //ʹ��UART3��RX��TX�ж�  
    
 //   P3SEL |= BIT6;//����P3.6ΪUART3��TXD 
 //   P3SEL |= BIT7;//����P3.7ΪUART3��RXD
 //   P3DIR |= BIT6;//P3.6Ϊ����ܽ�   
//     if(s_uart3_type==1)
////    {
//        P10DIR |= BIT0;             //ly P100���ߣ�uart3���ڵ��ԣ��͵Ļ�P104��,105����485��
//        P10OUT |= BIT0;  
////    }
////    else
////    {
//        P10DIR |= BIT0;             //ly P100���ߣ�uart3���ڵ��ԣ��͵Ļ�P104��,105����485��
//        P10OUT &=~ BIT0;
////    }
   
 
  UCA3CTL1 |= UCSWRST;
  UCA3CTL1 |= UCSSEL1;   //smclk 1M 
  
  UCA3BR0 = 8;
  UCA3BR1 = 0;
  UCA3MCTL |= UCBRF_0+UCBRS_6;
  UCA3CTL1 &= ~UCSWRST;
  
  UART3_ClearBuffer();
  /********************/
  P10DIR |= TXD3;
  P10SEL |= TXD3 + RXD3;
  
  /*2418 UC1IE UCA1RXIE 5438 UCA1IE UCRXIE*/
  UCA3IE |= UCRXIE;//���ܴ��ڽ����ж�          

  return 0;
}
void UART3_Open_9600(int _type)
{
  s_uart3_type = _type;
    
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
     
  //  UART3_ClearBuffer();
        
  //  ME2 |= UTXE1+URXE1;   //ʹ��UART3��TXD��RXD  
 //   IE2 |= URXIE1+UTXIE1; //ʹ��UART3��RX��TX�ж�  
    
 //   P3SEL |= BIT6;//����P3.6ΪUART3��TXD 
 //   P3SEL |= BIT7;//����P3.7ΪUART3��RXD
 //   P3DIR |= BIT6;//P3.6Ϊ����ܽ�    
  UCA3CTL1 |= UCSWRST;
  UCA3CTL1 |= UCSSEL1;   //smclk 1M 
  
  //104��Ӧ9600�����ʣ�52��Ӧ19200? 8��Ӧ115200
  UCA3BR0 = 104;
  UCA3BR1 = 0;
  UCA3MCTL |= UCBRF_0+UCBRS_6;//
  //UCA1MCTL = 10;
  UCA3CTL1 &= ~UCSWRST;
  
  UART3_ClearBuffer();
  /********************/
  P10DIR |= TXD3;
  P10SEL |= TXD3 + RXD3;
  
  /*2418 UC1IE UCA1RXIE 5438 UCA1IE UCRXIE*/
  UCA3IE |= UCRXIE;
}
void UART3_Close()
{ 
   //�ر�RS232��ƽת����·
  //P4DIR |= BIT0;
  //P4OUT &= ~BIT0; 
   
   UART3_ClearBuffer(); 
   //�رմ���1
   
/*2418 UC1IE UCA1RXIE 5438 UCA1IE UCRXIE*/
   UCA3IE &= ~UCRXIE;	 

}

void UART3_ClearBuffer()
{
    DownInt();//���ж�
    
    UART3_Tx_Buf=0;
    UART3_Rx_INTIndex=0;
    UART3_Rx_INTLen=0;
    UART3_Rx_RecvIndex=0;
  
    UART3_Tx_Flag=0;
    UART3_Tx_Len=0; 
    
    for(int i=0;i<UART3_MAXIndex;++i){
      UART3_Rx_BufLen[i]=0;
    }
    
    UpInt();//���ж�
}
int  UART3_Send(char * _data ,int _len, int _CR)
{

    if(UART3_Tx_Flag!=0)//�ȴ���һ�η��ͽ���
    {//�͵�500ms 
        System_Delayms(500);
   
        UART3_Tx_Flag=0;//ǿ������Ϊ0;
    }
    if(_len>1)
    {
        //��ȫ�ֱ�����ֵ
        UART3_Tx_Buf=_data; //�����һ�������ж� ����Ϊ0
        UART3_Tx_Len=_len; //
        UART3_Tx_Flag=1; //
       for(int i=0;i<UART3_Tx_Len;i++)
       {
         
         /*2418 UC1IFG UCA1TXIFG 5438 UCA1IFG UCTXIFG*/
        while (!(UCA3IFG&UCTXIFG));
            UCA3TXBUF =_data[i];
       }
        UART3_Tx_Flag=0;
    }
    if(_len==1) //
    {
      /*2418 UC1IFG UCA1TXIFG 5438 UCA1IFG UCTXIFG*/
        while (!(UCA3IFG&UCTXIFG));
        UCA3TXBUF = _data[0];
        
    }
    if(_CR)
    {//����һ������
      /*2418 UC1IFG UCA1TXIFG 5438 UCA1IFG UCTXIFG*/
        while (!(UCA3IFG&UCTXIFG));
        UCA3TXBUF=13;
        /*2418 UC1IFG UCA1TXIFG 5438 UCA1IFG UCTXIFG*/
        while (!(UCA3IFG&UCTXIFG));
        UCA3TXBUF=10;
        
    }
    if(_CR)
    {//����һ������
      /*2418 UC1IFG UCA1TXIFG 5438 UCA1IFG UCTXIFG*/
        while (!(UCA3IFG&UCTXIFG));
        UCA3TXBUF=13;
        /*2418 UC1IFG UCA1TXIFG 5438 UCA1IFG UCTXIFG*/
        while (!(UCA3IFG&UCTXIFG));
        UCA3TXBUF=10;
        
    }

    // && (ptDevBle->isenable())
    /* ������ӡ */
    PT_IODev  ptDevBle =  getIODev();
    if( ptDevBle->isspp() == 1 )    //if BLE is already in SPP mode, send debug message to BLE receiver
    {
        ptDevBle->sendMsg(_data,_len);
        if(_CR)
        {
            System_Delayms(1000);    //ptDevBle->sendMsg�ļ������̫�̣�esp32�ᷴӦ������
            ptDevBle->sendMsg("\r\n",2);
        }
        ptDevBle->close();
    }


    return 0;
}

//����
int  UART3_SendtoInt(int num)
{
/*2418 UC1IFG UCA1TXIFG 5438 UCA1IFG UCTXIFG*/
        while (!(UCA3IFG&UCTXIFG));
            UCA3TXBUF =num;
          System_Delayms(2000);
    return 0;
}

int  UART3_RecvLine(char * _dest ,int _max, int * _pNum)
{
    int i=0;
    //�ö���λ�ó���Ϊ0, ��ѭ���ȴ� 
    while(UART3_Rx_BufLen[UART3_Rx_RecvIndex]==0);
    //�������ˣ��Ͱ����ݸ��Ƴ���, 
    for(i=0; ( i< _max) && ( i<UART3_Rx_BufLen[UART3_Rx_RecvIndex]); ++i)
    {
        _dest[i]=UART3_Rx_Buffer[UART3_Rx_RecvIndex][i];
    }
    *_pNum = UART3_Rx_BufLen[UART3_Rx_RecvIndex];
    //������Ϻ�,�ͰѸ�λ�õĳ�������Ϊ0,�����жϿ��Ը�����.
    UART3_Rx_BufLen[UART3_Rx_RecvIndex]=0;
    //��λ����һ��
    // ��� ����9 �ͼ�ȥ9 ,�ӵ�һ�п�ʼ,����ͼ�������.
    if( UART3_Rx_RecvIndex >= UART3_MAXIndex -1)
        UART3_Rx_RecvIndex=0;
    else
        ++UART3_Rx_RecvIndex;
    return 0;
}

int  UART3_RecvLineTry(char * _dest,const int _max, int * _pNum)
{
    int i=0;
    //�ö���λ�ó���Ϊ0, ��ѭ���ȴ� 

    if(UART3_Rx_BufLen[UART3_Rx_RecvIndex]==0)
    {
        return -1;
    }

    TraceInt4(UART3_Rx_RecvIndex,1);
    TraceInt4(UART3_Rx_BufLen[UART3_Rx_RecvIndex],1);
    //�������ˣ��Ͱ����ݸ��Ƴ���, 
    for(i=0; ( i< _max) && ( i<UART3_Rx_BufLen[UART3_Rx_RecvIndex]); ++i)
    {
        _dest[i]=UART3_Rx_Buffer[UART3_Rx_RecvIndex][i];
    }
    *_pNum =UART3_Rx_BufLen[UART3_Rx_RecvIndex];
    //������Ϻ�,�ͰѸ�λ�õĳ�������Ϊ0,�����жϿ��Ը�����.
    UART3_Rx_BufLen[UART3_Rx_RecvIndex]=0;
    //��λ����һ��
    // ��� ����9 �ͼ�ȥ9 ,�ӵ�һ�п�ʼ,����ͼ�������.
    if( UART3_Rx_RecvIndex >= UART3_MAXIndex -1)
        UART3_Rx_RecvIndex=0;
    else
        ++UART3_Rx_RecvIndex;
    return 0;
}
int  UART3_RecvLineWait(char *_dest ,const int _max, int * _pNum)
{
    int i=0; 
    //�ö���λ�ó���Ϊ0, ��ѭ���ȴ� 
    while(UART3_Rx_BufLen[UART3_Rx_RecvIndex]==0)
    {
        System_Delayms(30);
        ++i;
        if(i>10) 
            return -1;
    }
    //�������ˣ��Ͱ����ݸ��Ƴ���, 
    for(i=0; ( i< _max) && ( i<UART3_Rx_BufLen[UART3_Rx_RecvIndex]); ++i)
    {
        _dest[i]=UART3_Rx_Buffer[UART3_Rx_RecvIndex][i];
    }
    *_pNum = UART3_Rx_BufLen[UART3_Rx_RecvIndex];
    //������Ϻ�,�ͰѸ�λ�õĳ�������Ϊ0,�����жϿ��Ը�����.
    UART3_Rx_BufLen[UART3_Rx_RecvIndex]=0;
    //��λ����һ��
    // ��� ����9 �ͼ�ȥ9 ,�ӵ�һ�п�ʼ,����ͼ�������.
    if( UART3_Rx_RecvIndex >= UART3_MAXIndex -1)
        UART3_Rx_RecvIndex=0;
    else
        ++UART3_Rx_RecvIndex;
    return 0;
}
int  UART3_RecvLineLongWait(char *_dest,int _max, int * _pNum)
{
    int i=0;
    //�ö���λ�ó���Ϊ0, ��ѭ���ȴ� 
    while(UART3_Rx_BufLen[UART3_Rx_RecvIndex]==0)
    {//�ȴ�5��.
        System_Delayms(50);
        ++i;
        if(i>100)
            return -1;        
    }
    //�������ˣ��Ͱ����ݸ��Ƴ���, 
    for(i=0; ( i< _max) && ( i<UART3_Rx_BufLen[UART3_Rx_RecvIndex]); ++i)
    {
        _dest[i]=UART3_Rx_Buffer[UART3_Rx_RecvIndex][i];
    }
    *_pNum = UART3_Rx_BufLen[UART3_Rx_RecvIndex];
    //������Ϻ�,�ͰѸ�λ�õĳ�������Ϊ0,�����жϿ��Ը�����.
    UART3_Rx_BufLen[UART3_Rx_RecvIndex]=0;
    //��λ����һ��
    // ��� ����9 �ͼ�ȥ9 ,�ӵ�һ�п�ʼ,����ͼ�������.
    if( UART3_Rx_RecvIndex >= UART3_MAXIndex -1)
        UART3_Rx_RecvIndex=0;
    else
        ++UART3_Rx_RecvIndex;
    return 0;
}

//void Judge_Watermeter()
//{
//    if(UART3_Rx_INTLen == 9)
//    {
//        UART3_Rx_BufLen[UART3_Rx_INTIndex] = UART3_Rx_INTLen;
//        UART3_Rx_INTLen=0;
//        if(UART3_Rx_INTIndex >= UART3_MAXIndex-1)
//            UART3_Rx_INTIndex=0;
//        else
//            ++UART3_Rx_INTIndex;
//    }
//}


//
//    ���ܵ�һ���ַ�.
//    
//    װ�ص�UART3_Rx_Buffer[UART3_Rx_INTIndex][UART3_Rx_INTLen]��.
//    ������UART3_Rx_INTLen ;���UART3_Rx_INTLen�Ѿ������һ�������ַ�Ϊ����
//    ��д���л���ĳ���Ϊ UART3_Rx_INTLen+1;
//    ����UART3_Rx_INTIndex,ָ����һ��������. �����һ����������������δ������.
//    ��ô�Ͳ�����,������д��ǰ�Ļ�����.
//    
//    
//    
/*************VECTOR*/
#pragma vector=USCI_A3_VECTOR 
__interrupt void UART3_RX_ISR(void)   //�����յ����ַ���ʾ���������
{
   //_DINT();
   char _temp; 
      //char *tbuffer;  
   _temp = UCA3RXBUF;
#if 1

    UART3_Rx_Buffer[UART3_Rx_INTIndex][UART3_Rx_INTLen]=_temp;
    ++UART3_Rx_INTLen;
    
    if(s_uart3_type == UART3_CONSOLE_TYPE)
    {   
      
        if(((_temp==0x0A) && (UART3_Rx_INTLen!=0) && (UART3_Rx_Buffer[UART3_Rx_INTIndex][UART3_Rx_INTLen-2]==0x0D)) || (_temp == ')'))
        {
            //�����ͷ���յ���������з���,ֱ������
            if(UART3_Rx_INTLen==1)
            {
                UART3_Rx_INTLen=0; //���¿�ʼ���� 
                return ;
            }
            else
            {
                //   ��λ����һ�� 
                //UART3_Rx_Buffer[UART3_Rx_INTIndex][UART3_Rx_INTLen-1]=13; //������ø�13
                UART3_Rx_BufLen[UART3_Rx_INTIndex] = UART3_Rx_INTLen - 2;//���������з�
                UART3_Rx_INTLen=0;
                if(UART3_Rx_INTIndex >= UART3_MAXIndex-1)
                    UART3_Rx_INTIndex=0;
                else
                    ++UART3_Rx_INTIndex;
                //UART3_Rx_INTIndex += UART3_Rx_INTIndex < (UART3_MAXIndex - 1) ? 1 : 1-UART3_MAXIndex;  
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
//    if((_temp==0x0D) && (UART3_Rx_INTLen!=0) && UART3_Rx_Buffer[UART3_Rx_INTIndex][UART3_Rx_INTLen-1]==0x0D)
//    {
//        return;
//    }
    
    if(UART3_Rx_INTLen >= UART3_MAXBUFFLEN-1)
    {//�г���������, ����ֱ�ӽضϳ�һ��.
        UART3_Rx_BufLen[UART3_Rx_INTIndex] = UART3_Rx_INTLen + 1;
        UART3_Rx_INTLen=0;
        if(UART3_Rx_INTIndex >= UART3_MAXIndex-1)
            UART3_Rx_INTIndex=0;
        else
            ++UART3_Rx_INTIndex;
    }
    
    //�жϻ������Ƿ�����:UART3_Rx_INTIndex��¼�´α�����������´������Ѿ��д洢��˵������������
    if( UART3_Rx_BufLen[UART3_Rx_INTIndex]!=0)
    {
        //��һ�л�δ���������Ǿ͸������������һ��
        if(UART3_Rx_INTIndex <= 0)
            UART3_Rx_INTIndex = UART3_MAXIndex-1;
        else
            --UART3_Rx_INTIndex;
        //�Ѹ��г�������Ϊ0,���ж�ռ��
        UART3_Rx_BufLen[UART3_Rx_INTIndex]=0;
    }

 #endif
}


int putchar(int c)
{
    while (!(UCA3IFG&UCTXIFG));             // USCI_A0 TX buffer ready?
    UCA3TXBUF = (unsigned char)c; 
  
    return c;
}