//////////////////////////////////////////////////////
//     文件名: led.h
//   文件版本: 1.0.0  
//   创建时间: 09年11月30日
//   更新内容: 无。 
//       作者: 林智
//       附注: 
// 
//
//////////////////////////////////////////////////////


#pragma once

//
//    LED1   p1.7  //原理图 D7    
//    LED3   p4.7  //原理图 D3
//    LED4   p4.3  //原理图 D4
//

//
//   led1  75ms的快闪 表示一次系统的滴答Ticks   (定时器A的中断 )
//   led1  连续的3下闪烁 表示收到外来数据.
//   ....
//   ... 
//   led4  连续的3下闪烁 表示系统启动
//    
 
void Led_Init();
void Led1_On();
void Led2_On();
void Led3_On();
void Led4_On();
void Led5_On();
void Led6_On();
void Led1_Off();
void Led2_Off();
void Led3_Off();
void Led4_Off(); 
void Led5_Off(); 
void Led6_Off(); 

void Led_Round(); 
void Led_LongOn(); 

void Led_WARN();
void Led1_WARN();
void Led2_WARN();
void Led3_WARN();
void Led4_WARN();
 
void Led_Flash();
 

