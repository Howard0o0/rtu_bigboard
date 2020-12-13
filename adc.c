//////////////////////////////////////////////////////
//     文件名: adc.c
//   文件版本: 2.0.0
//   创建时间: 2010年 3月4日
//   更新内容: 精简
//       作者: 林智
//       附注: 无
// 
//////////////////////////////////////////////////////

#include "msp430common.h"
#include "adc.h"
#include "led.h"
#include "common.h"

extern unsigned int sys_errorcode_runcode;

#define AD_NUM 16

unsigned int A[16]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}; //  A0为 电源的 1/2 分压

void ADC_Open()
{ 
    P6SEL = 0xFF;            //设置P6.0~P6.7 为模拟输入A0~A7 没变
    
    ADC12CTL0 &= ~(ADC12ENC);     //设置ENC为0，从而修改ADC12寄存器·的值
    ADC12CTL1 |= ADC12CSTARTADD_0;//转换的起始地址为：ADCMEM0	 
    ADC12CTL1 |= ADC12CONSEQ_1;   //转换模式为：序列通道,单次转换 
    //设置各个通道
//    ADC12MCTL0 = ADC12INCH_0 + ADC12SREF0 + ADC12SREF1;
//    ADC12MCTL1 = ADC12INCH_1 + ADC12SREF0;
//    ADC12MCTL2 = ADC12INCH_2 + ADC12SREF0;
//    ADC12MCTL3 = ADC12INCH_3 + ADC12SREF0;
//    ADC12MCTL4 = ADC12INCH_4 + ADC12SREF0;
//    ADC12MCTL5 = ADC12INCH_5 + ADC12SREF0;
//    ADC12MCTL6 = ADC12INCH_6 + ADC12SREF0;
    ADC12MCTL0 = ADC12INCH_0;
    ADC12MCTL1 = ADC12INCH_1;
    ADC12MCTL2 = ADC12INCH_2;
    ADC12MCTL3 = ADC12INCH_3;
    ADC12MCTL4 = ADC12INCH_4;
    ADC12MCTL5 = ADC12INCH_5;
    ADC12MCTL6 = ADC12INCH_6;
    ADC12MCTL7 = ADC12INCH_7;
    ADC12MCTL8 = ADC12INCH_8;
    ADC12MCTL9 = ADC12INCH_9;
    ADC12MCTL10 = ADC12INCH_10;
    ADC12MCTL11 = ADC12INCH_11;
    ADC12MCTL12 = ADC12INCH_12;
    ADC12MCTL13 = ADC12INCH_13;
    ADC12MCTL14 = ADC12INCH_14;
    ADC12MCTL15 = ADC12INCH_15+ ADC12EOS;
// ADC12MCTL7 = ADC12INCH_7 + ADC12SREF0 + ADC12EOS;
  
    ADC12CTL0 |= ADC12ON + ADC12SHT00;    //打开转换核心
    ADC12CTL0 |= ADC12REFON;      //打开参考电压
    ADC12CTL0 |= ADC12REF2_5V;    //使用2.5V的参考电压
    ADC12CTL0 |= ADC12MSC;        ///* ADC12 Multiple SampleConversion 
  
    ADC12CTL1 |= ADC12SSEL_1; //用的是ACLK, 越慢越好
    ADC12CTL1 |= ADC12DIV_7;  // 8分频
    ADC12CTL1 |= (ADC12SHP);      //转换时钟来自于转换定时器
    
    //设置完毕,使能AD转换
    ADC12CTL0 |= ADC12ENC;
    System_Delayms(100); //待AD模块启动
    return;
}
int ADC_ReadAnalogStr(int _index, char *_dest)
{//转换数字, _index从1起
    if(_index <1 || _index>8)
        return -2;
    int _tempInt = A[--_index];
    Utility_UintToStr4(_tempInt,_dest);
    return 0;
}  
void ADC_Sample()
{  
    P5SEL |= BIT0;
    P5SEL |= BIT1;
    P5OUT |= BIT0;
    P5OUT |= BIT1;
    int _repeats=0;   //失败尝试次数
    A[0]=0;A[1]=0;A[2]=0;A[3]=0;A[4]=0;A[5]=0;A[6]=0;A[7]=0;//每次采样前要清空A[I]
    A[8]=0;A[9]=0;A[10]=0;A[11]=0;A[12]=0;A[13]=0;A[14]=0;A[15]=0;
 
    unsigned int max00=0; 
    unsigned int max10=0; 
    unsigned int max20=0;   
    unsigned int max30=0; 
    unsigned int max40=0; 
    unsigned int max50=0; 
    unsigned int max60=0; 
    unsigned int max70=0;
    unsigned int max80=0; 
    unsigned int max90=0; 
    unsigned int max100=0;   
    unsigned int max110=0; 
    unsigned int max120=0; 
    unsigned int max130=0; 
    unsigned int max140=0; 
    unsigned int max150=0; 
    
    unsigned int min00=4096; 
    unsigned int min10=4096; 
    unsigned int min20=4096; 
    unsigned int min30=4096; 
    unsigned int min40=4096; 
    unsigned int min50=4096; 
    unsigned int min60=4096; 
    unsigned int min70=4096; 
    unsigned int min80=4096; 
    unsigned int min90=4096; 
    unsigned int min100=4096; 
    unsigned int min110=4096; 
    unsigned int min120=4096; 
    unsigned int min130=4096; 
    unsigned int min140=4096; 
    unsigned int min150=4096; 
    
    unsigned int temp0=0;
    unsigned int temp1=0;
    unsigned int temp2=0;
    unsigned int temp3=0;
    unsigned int temp4=0;
    unsigned int temp5=0;
    unsigned int temp6=0;
    unsigned int temp7=0;
    unsigned int temp8=0;
    unsigned int temp9=0;
    unsigned int temp10=0;
    unsigned int temp11=0;
    unsigned int temp12=0;
    unsigned int temp13=0;
    unsigned int temp14=0;
    unsigned int temp15=0;
    
    //总共10次, 去掉1个最大值,1个最小值,然后除以8.
    //实际只删除2个极值 仍不是很稳 
    for(int j=0;j<10;++j)
    {
        for(int i=0;i<AD_NUM;++i)
        {
            ADC12CTL0 |= ADC12SC;
            ADC12CTL0 &= ~ADC12SC;
            for(int j=2;j>1;--j)
            {
             //通过Timer0_A0中断表示正在ADC采样 LSHB 20200506
            sys_errorcode_runcode=1; //ADC采样指示灯
            Led_OffAll();
            led_flash_init(15);
            Timer0_A0_Enable(); //Timer0_A0开中断
            System_Delayms(50);
            Timer0_A0_Clear();//Timer0_A0关中断
            Led_OffAll();
           // end  LSHB 20200506        
            
            }
        }
        _repeats=0;
        //等待转换完成 
        while( (ADC12CTL1 & ADC12BUSY)==1)
        {//最多只等待1秒
            System_Delayms(100);  
            ++ _repeats;
            if(_repeats>10)
                break;
        }
        temp0=ADC12MEM0; //操作数据寄存器 有副作用,
        temp1=ADC12MEM1; // 先缓存 操作
        temp2=ADC12MEM2;
        temp3=ADC12MEM3;
        temp4=ADC12MEM4;
        temp5=ADC12MEM5;
        temp6=ADC12MEM6;
        temp7=ADC12MEM7; 
        temp8=ADC12MEM8; 
        temp9=ADC12MEM9; 
        temp10=ADC12MEM10;
        temp11=ADC12MEM11;
        temp12=ADC12MEM12;
        temp13=ADC12MEM13;
        temp14=ADC12MEM14;
        temp15=ADC12MEM15;
        
        //更新最大最小值
        
        if(max00<temp0)  
            max00=temp0; 
        if(min00>temp0) 
            min00=temp0; 
        if(max10<temp1) 
            max10=temp1; 
        if(min10>temp1) 
            min10=temp1; 
        if(max20<temp2) 
            max20=temp2; 
        if(min20>temp2)  
            min20=temp2;  
        if(max30<temp3) 
            max30=temp3; 
        if(min30>temp3) 
            min30=temp3;
        if(max40<temp4)  
            max40=temp4; 
        if(min40>temp4) 
            min40=temp4; 
        if(max50<temp5)  
            max50=temp5; 
        if(min50>temp5) 
            min50=temp5; 
        if(max60<temp6) 
            max60=temp6; 
        if(min60>temp6)  
            min60=temp6; 
        if(max70<temp7)  
            max70=temp7; 
        if(min70>temp7)
            min70=temp7;
         if(max80<temp8)  
            max80=temp8; 
        if(min80>temp8)
            min80=temp8;       
         if(max90<temp9)  
            max90=temp9; 
        if(min90>temp9)
            min90=temp9;      
          if(max100<temp10)  
            max100=temp10; 
        if(min100>temp10)
            min100=temp10;
          if(max110<temp11)  
            max110=temp11; 
        if(min110>temp11)
            min110=temp11;        
          if(max120<temp12)  
            max120=temp12; 
        if(min120>temp12)
            min120=temp12;        
          if(max130<temp13)  
            max130=temp13; 
        if(min130>temp13)
            min130=temp13;        
          if(max140<temp14)  
            max140=temp14; 
        if(min140>temp14)
            min140=temp14;        
          if(max150<temp15)  
            max150=temp15; 
        if(min150>temp15)
            min150=temp15;
        
        //累加
        A[0]+=temp0;A[1]+=temp1;A[2]+=temp2;A[3]+=temp3;
        A[4]+=temp4;A[5]+=temp5;A[6]+=temp6;A[7]+=temp7;
         A[8]+=temp8;A[9]+=temp9;A[10]+=temp10;A[11]+=temp11;
        A[12]+=temp12;A[13]+=temp13;A[14]+=temp14;A[15]+=temp15;
    }
    //去掉最大值 最小值
    A[0]-=(max00 + min00); A[1]-=(max10 + min10);
    A[2]-=(max20 + min20); A[3]-=(max30 + min30);
    A[4]-=(max40 + min40); A[5]-=(max50 + min50);
    A[6]-=(max60 + min60); A[7]-=(max70 + min70);
    A[8]-=(max80 + min80); A[9]-=(max90 + min90);
    A[10]-=(max100 + min100); A[11]-=(max110 + min110);
    A[12]-=(max120 + min120); A[13]-=(max130 + min130);
    A[14]-=(max140 + min140); A[15]-=(max150 + min150);
    //求平均数
    A[0]>>=3;A[1]>>=3;A[2]>>=3;A[3]>>=3;A[4]>>=3;A[5]>>=3;A[6]>>=3;A[7]>>=3; 
    A[8]>>=3;A[9]>>=3;A[10]>>=3;A[11]>>=3;A[12]>>=3;A[13]>>=3;A[14]>>=3;A[15]>>=3; 
   
//#ifdef __CONSOLE__DEBUG__
  
//    TraceMsg("////////////////////////////////////////",1);
//    TraceMsg("A0: ",1);
//    TraceInt4(A[0],1);
//    TraceMsg("      ",1);
//    TraceMsg("A1: ",1);
//    TraceInt4(A[1],1);
//     
//    TraceMsg("A2: ",1);
//    TraceInt4(A[2],1);
//    TraceMsg("      ",1);
//    TraceMsg("A3: ",1);
//    TraceInt4(A[3],1);
//    
//    TraceMsg("A4: ",1);
//    TraceInt4(A[4],1);
//    TraceMsg("      ",1);
//    TraceMsg("A5: ",1);
//    TraceInt4(A[5],1);
//    
//    TraceMsg("A6: ",1);
//    TraceInt4(A[6],1);
//    TraceMsg("      ",1);
//    TraceMsg("A7: ",1);
//    TraceInt4(A[7],1);
  
//    TraceMsg("      ",1);
//    TraceMsg("A8: ",1);
//    TraceInt4(A[8],1);
//    TraceMsg("      ",1);
//    TraceMsg("A9: ",1);
//    TraceInt4(A[9],1);
//    TraceMsg("      ",1);
//    TraceMsg("A10: ",1);
//    TraceInt4(A[10],1);
//     System_Delayms(20);
//    TraceMsg("      ",1);
//    TraceMsg("A11: ",1);
//    TraceInt4(A[11],1);
//    TraceMsg("      ",1);
//    TraceMsg("A12: ",1);
//    TraceInt4(A[12],1);
//    TraceMsg("      ",1);
//    TraceMsg("A13: ",1);
//    TraceInt4(A[13],1);
//    TraceMsg("      ",1);
//    TraceMsg("A14: ",1);
//    TraceInt4(A[14],1);
//    TraceMsg("      ",1);
//    TraceMsg("A15: ",1);
//    TraceInt4(A[15],1);
//    TraceMsg("////////////////////////////////////////",1);    
//#endif
}

void ADC_Close()
{
    ADC12CTL0 &=~(ADC12ENC);               //禁止转换
    ADC12CTL0 &=~ ( ADC12ON + ADC12REFON); //关闭转换核心 和 内部参考电压
}
