//////////////////////////////////////////////////////
//     文件名: timer.c
//   文件版本: 1.0.0  
//   创建时间: 09年11月30日
//   更新内容: 无。 
//       作者: 林智
//       附注:  
// 
//
//////////////////////////////////////////////////////

#include "msp430common.h"
#include "timer.h"
#include "common.h"
#include "led.h"
#include "hydrologytask.h"

static unsigned int s_ClockTicks=0;   
static unsigned int s_reset_count=0;



unsigned int sys_errorcode_runcode=100; // 定义 sys_errorcode_runcode为全局变量，取值1-69， 初值定为100，防止误点灯   LSHB 20200506



void Timer_Zero()
{
    s_ClockTicks=0;
} 
int Timer_Passed_Mins(int _min)
{
    if(_min <= s_ClockTicks)
        return 1;
    else
        return 0;
}

/* Timer0_A5*/
void Timer0_A0_Init(unsigned int ccr0)  // 系统运行状态和错误指示灯控制，ccr0=45 系统错误闪烁频率，ccr0=15 系统通信发送数据闪烁频率，ccr0=30 系统通信接收数据闪烁频率。闪烁时长一般3秒 LSHB 20200506
{
  /*5438 TA0CTL*/
    TA0CTL |=  TACLR;      //清除计数
    TA0CTL |= TASSEL0;    //选择 ACLK时钟
    TA0CTL |= ID1+ID0;    // 1/8分频   
    TA0CTL |= MC0 ;// 增计数 模式   
    /*5438 TA1CCR0*/
    TA0CCR0 = ccr0;  
     /*2418 TACCTL0 5438 TA0CCTL0*/
//    TA0CCTL0 = CCIE;	 // 通过Timer0_A0中断专用函数打开和关闭Timer0_A0中断，此处不能打开中断
    
} 


/*time1 a3*/
//注意 ACLK 使用了 8分频,
void Timer1_A0_Init(unsigned int ccr0)
{
  /*2418 TACTL 5438 TA1CTL*/
    TA1CTL |=  TACLR;      //清除计数
    TA1CTL |= TASSEL0;    //选择 ACLK时钟
    TA1CTL |= ID1+ID0;    // 1/8分频 
    TA1CTL |= MC0 ;// 增计数 模式 
    //1分钟 中断一次
    /*2418 TACCR0 5438 TA1CCR0*/
    TA1CCR0 = ccr0;  //  (32768HZ / (8 * 8) ) * 60s =30720
     /*2418 TACCTL0 5438 TA1CCTL0*/
    TA1CCTL0 |= CCIE;	 // CCR0 中断允许
    /*2418 TACTL 5438 TA1CTL*/
     
} 
 
void WatchDog_Init()
{ 
    //开成最大情况的.
    //使用ACLK 32K/8 分频
  /*2418 WDTSSEL 5438 WDTSSEL0*/
    WDTCTL = WDTPW + WDTSSEL0; 
}  

void WatchDog_Clear()
{
  /*2418 WDTSSEL 5438 WDTSSEL0*/
    WDTCTL = WDTPW  + WDTSSEL0 + WDTCNTCL;
} 

/* Timer0_B7*/
void TimerB_Init(unsigned int ccr0)
{
  /*2418 TBCTL 5438 TB0CTL*/
    TB0CTL = TBCLR;
    TB0CTL |= TBSSEL0;    //选择ACLK时钟
    TB0CTL |= ID0;  //1/8分频
    TB0CTL |= MC1  ;   //   连续计数模式          
    //TBCTL |= TBIE;       //使能TBIFG中断
    //4分钟 中断一次 ,(跑2趟)
    //TBCCR0=ccr0; // (32768HZ /(8*8) ) * 240 / 2 = 61440
    
    TBCCR0=61450;//用TIMERB0中断指示工作状态和系统错误状态61450是个初始值，没有物理含义  LSHB 20200506
    
    TBCCR1=61440; 
//    TBCCR2=10240;
//    TBCCR2=4096;//用来清狗的 中断. 2s中断1次
//    TBCCR2=2048;
    TBCCR2=1024;//用来清狗的 中断.
    /*2418 TBCCTL1 TBCCTL2 5438 TB0CCTL1 TB0CCTL2*/
    
//   TB0CCTL0=CCIE;//用来启动TIMERB0中断  LSHB 20191004
    
    TB0CCTL1 |= CCIE;
    TB0CCTL2 |= CCIE;
    
    /*2418 P41  5438 P96 systemdebug*/
    P9DIR |= BIT6;
    P9OUT |= BIT6;
}


void TimerB_Clear()
{
    s_reset_count=0;
}

void Timer0_A0_Enable()  // 启动Timer0_A0中断 LSHB 20200506
{
    TA0CCTL0 |= CCIE;
}

void Timer0_A0_Clear()  // 清除Timer0_A0中断 LSHB 20200506
{
     TA0CCTL0 &= ~CCIE;
}

void TimerB0_Enable()  // 启动TIMERB0中断 LSHB 20200506
{
    TB0CCTL0=CCIE;
}

void TimerB0_Clear()  // 清除TIMERB0中断 LSHB 20200506
{
    TB0CCTL0=~CCIE;
}

#pragma vector = TIMER0_A0_VECTOR
__interrupt void TIMER0_A0_ISR (void)
{    
  
  sys_error_status(sys_errorcode_runcode);  //用于系统错误和工作状态指示灯控制 LSHB   20200506
}


/*   TIMERA0_VECTOR   *   TIMER1_A0_VECTOR*/
#pragma vector=TIMER1_A0_VECTOR 
__interrupt void TIMERA0_ISR(void)
{
    //唤醒CPU,如果CPU本来就是醒的,不会导致混乱
    //LPM3_EXIT;
    //LPM2_EXIT;
   // Led1_On();
   // System_Delayms(75);
   // Led1_Off();
    ++s_ClockTicks;//增加一次滴答 
    HydrologyTimeBase();
    TimerB_Clear();
    WatchDog_Clear();
}

//30分钟的看门狗
//#pragma vector = TIMERB0_VECTOR
#pragma vector = TIMERB0_VECTOR
__interrupt void TIMERB0_ISR (void)
{    
// 保留，以后用
 
}


//#pragma vector=TIMERB1_VECTOR
#pragma vector=TIMERB1_VECTOR
__interrupt void TIMERB1_ISR(void)
{ 
    //必须要读一下TBIV来清中断
    unsigned int _tbiv = TBIV ; 
    switch(_tbiv)
    {
      case 2:
        //半分钟来一次
        //计数如果超过1000
        ++s_reset_count;
        if(s_reset_count>60)
        {//超过30分钟就重启
            System_Reset();
        }
        TBCCR1 += 61440;
        break;
      case 4:
        WatchDog_Clear(); 
        TBCCR2 += 1024;
       Clear_ExternWatchdog();
        break;
      case 10:
        break;
      default:
        break;
    } 
}

