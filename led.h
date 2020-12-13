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


 
void Led_Init();
void Led3_On();
void Led4_On();
void Led5_On();
void Led6_On();
void Led7_On();
void Led8_On();
void Led3_Off();
void Led4_Off();
void Led5_Off();


void Led6_Off(); 
void Led7_Off(); 
void Led8_Off(); 

void Led_Round(); 
void Led_LongOn(); 

void Led_WARN1();
void Led_WARN2();
void Led_WARN3();
void Led_WARN4();
void Led_WARN5();
 
void Led_Flash();
 
void Led1_WorkLightOn(); // 工作指示灯闪烁  LSHB 20200506
void Led1_WorkLightOff(); // 工作指示灯灭  LSHB 20200506