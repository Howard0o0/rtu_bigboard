//////////////////////////////////////////////////////
//     文件名: sampler.h
//   文件版本: 1.0.0
//   创建时间: 09年 11月30日
//   更新内容:  
//       作者: 林智
//       附注: 无
//
//////////////////////////////////////////////////////

#pragma once 


extern char g_pulse_rate[4]; //  满计数

extern char g_pulse_range[4][3]; 

//  管理模拟量  和开关量 
int  Sampler_Init();

int  Sampler_Open();

int  Sampler_Close();

int  Sampler_Sample();

int  Sampler_CheckNormal(); 
int  Sampler_GSM_ReadAlertString(char * _dest);
int  Sampler_DTU_ReadAlertString(char * _dest);
int  Sampler_SaveData(char * _saveTime);
int  Sampler_IO_Level(int _ioIdx, int _level);

