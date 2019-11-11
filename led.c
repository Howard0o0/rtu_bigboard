//////////////////////////////////////////////////////
//     文件名: led.c
//   文件版本: 1.0.0  
//   创建时间: 09年11月30日
//   更新内容: 无。 
//       作者: 林智
//       附注:  
//  
//////////////////////////////////////////////////////

#include "msp430common.h"
#include "common.h"
#include "led.h"

void Led_Init()
{  //都是输出 
 
    P5DIR |= BIT3;
    P7DIR |= BIT1;
    P7DIR |= BIT2;
    P7DIR |= BIT3;
    P9DIR |= BIT0;
    P10DIR |= BIT6; 
    
    P5SEL &= ~BIT3;
    P7SEL &= ~BIT1;
    
    P5OUT |= BIT3;
    P7OUT |= BIT1;
    P7OUT |= BIT2;
    P7OUT |= BIT3;
    P7OUT |= BIT3;
    P10OUT |= BIT6;
} 

void Led_Round()
{
    Led1_On();
    System_Delayms(200);
    Led1_Off(); 
    
    Led2_On();
    System_Delayms(200);
    Led2_Off(); 
    
    Led3_On();
    System_Delayms(200);
    Led3_Off(); 
    
    Led4_On();
    System_Delayms(200);
    Led4_Off();
    
    Led5_On();
    System_Delayms(200);
    Led5_Off();
    Led6_On();
    System_Delayms(200);
    Led6_Off();
    Led1_On();
    System_Delayms(200);
    Led1_Off();  
    Led2_On();
    System_Delayms(200);
    Led2_Off();  
    Led3_On();
    System_Delayms(200);
    Led3_Off();  
    Led4_On();
    System_Delayms(200);
    Led4_Off(); 
    Led5_On();
    System_Delayms(200);
    Led5_Off();
    Led6_On();
    System_Delayms(200);
    Led6_Off();
}


void Led1_On()//D6
{
   P5OUT|=BIT3; 
}
void Led2_On()
{ //输出低电平 
   P7OUT |=BIT1;
}
void Led3_On()//D4
{
   P7OUT|=BIT2;
}
void Led4_On()//D5
{
   P7OUT |=BIT3;
}
void Led5_On()//D5
{
   P9OUT |=BIT0;
}
void Led6_On()//D5
{
   P10OUT |=BIT6;
}

void Led1_Off()
{
   P5OUT &= ~ BIT3;
}
void Led2_Off()
{ 
    P7OUT &= ~ BIT1;
}
void Led3_Off()
{
   P7OUT &= ~ BIT2;
}
void Led4_Off()
{
   P7OUT &= ~ BIT3;
}
void Led5_Off()
{ 
   P9OUT &= ~ BIT0;
}
void Led6_Off()
{ 
   P10OUT &= ~ BIT6;
}


void Led_OffAll()
{
    P5OUT |= BIT3;
    P7OUT |= BIT1;
    P7OUT |= BIT2;
    P7OUT |= BIT3;
    P7OUT |= BIT3;
    P10OUT |= BIT6; 
} 
void Led_OnAll()
{
  P5OUT &= ~BIT3;
  P7OUT &= ~BIT1;
  P7OUT &= ~BIT2;
  P7OUT &= ~BIT3;
  P9OUT &= ~BIT0;
  P10OUT &= ~BIT6;
}

void Led_LongOn()
{
    Led_OnAll();
    System_Delayms(5000);
    Led_OffAll();
}
 
void Led1_WARN()
{
    Led1_On();
    System_Delayms(200);
    Led1_Off();
    System_Delayms(200);
    Led1_On();
    System_Delayms(200);
    Led1_Off();
    System_Delayms(200);
    Led1_On();
    System_Delayms(200);
    Led1_Off();
    System_Delayms(200);
}
void Led2_WARN()
{ 
}
void Led3_WARN()
{
  Led3_On();
  System_Delayms(200);
  Led3_Off();
  System_Delayms(200);
  Led3_On();
  System_Delayms(200);
  Led3_Off();
  System_Delayms(200);
  Led3_On();
  System_Delayms(200);
  Led3_Off();
  System_Delayms(200);
}
void Led4_WARN()
{
  Led4_On();
  System_Delayms(200);
  Led4_Off();
  System_Delayms(200);
  Led4_On();
  System_Delayms(200);
  Led4_Off();
  System_Delayms(200);
  Led4_On();
  System_Delayms(200);
  Led4_Off();
  System_Delayms(200);
}
void Led_WARN()
{
  Led_OnAll();
  System_Delayms(200);
  Led_OffAll();
  System_Delayms(200);
  Led_OnAll();
  System_Delayms(200);
  Led_OffAll();
  System_Delayms(200);
  Led_OnAll();
  System_Delayms(200);
  Led_OffAll();
  System_Delayms(200);
} 
void Led_Flash()
{
    Led_OnAll();
    System_Delayms(75);
    Led_OffAll(); 
    System_Delayms(1000);
}


