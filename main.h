//////////////////////////////////////////////////////
//     文件名: main.h
//   文件版本: 1.0.0
//   创建时间: 09年11月30日
//   更新内容:
//       作者: 林智
//       附注: 无
//
//////////////////////////////////////////////////////

#pragma once

#define MAIN_TYPE_GSM 1
#define MAIN_TYPE_DTU 2
#define MAIN_TYPE_GTM900 3
//#define MAIN_TYPE_485  3

#define WORK_TYPE_POWERSAVE 0
#define WORK_TYPE_LASTON 1

#define CONFIG_FLAG 0xA2

#define true 1
#define false 0

#define bool int

extern int g_main_type;
extern int g_work_mode;
extern int main_time_error;

void Restart_Init();
int  Restart_Proc();

void Main_Init();

int Main_Process_PowerSave();
int Main_Process_Always();

int Main_GSM_BadTime_Process_PowerSave(int type);  //
int Main_GSM_Report_Process_PowerSave();	   //
int Main_GSM_Alert_Process_PowerSave();		   //
int Main_GSM_BadTime_Process_Always(int type);	   //
int Main_GSM_Report_Process_Always();		   //
int Main_GSM_Alert_Process_Always();		   //
int Main_GSM_Idle_Process();

int Main_DTU_BadTime_Process_PowerSave(int type);  //
int Main_DTU_Report_Process_PowerSave();	   //
int Main_DTU_Alert_Process_PowerSave();		   //
int Main_DTU_BadTime_Process_Always(int type);	   //
int Main_DTU_Report_Process_Always();		   //
int Main_DTU_Alert_Process_Always();		   //
int Main_DTU_Idle_Process();

/*
int  Main_485_BadTime_Process_PowerSave(int type); //
int  Main_485_Report_Process_PowerSave(); //
int  Main_485_Alert_Process_PowerSave(); //
int  Main_485_BadTime_Process_Always(int type); //
int  Main_485_Report_Process_Always(); //
int  Main_485_Alert_Process_Always(); //
int  Main_485_Idle_Process();
*/

void Main_GotoSleep();
int  Main_Init_Process();  //串口设置命令的响应处理
int  Main_ProcCommand(char* _data, int _dataLen, int* _step);
int  Main_ProcInitConfig(char* _data, int _dataLen, int* _step);
