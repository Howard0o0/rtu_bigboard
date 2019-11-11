//////////////////////////////////////////////////////
//     �ļ���: main.h
//   �ļ��汾: 1.0.0
//   ����ʱ��: 09��11��30��
//   ��������:  
//       ����: ����
//       ��ע: ��
//
//////////////////////////////////////////////////////

#pragma once  

#define MAIN_TYPE_GSM  1
#define MAIN_TYPE_DTU  2  
#define MAIN_TYPE_GTM900 3
//#define MAIN_TYPE_485  3

#define WORK_TYPE_POWERSAVE 0
#define WORK_TYPE_LASTON    1

#define  CONFIG_FLAG  0xA2

extern int g_main_type;
extern int g_work_mode;
extern int main_time_error;

void Restart_Init();
int  Restart_Proc();

void Main_Init();

int  Main_Process_PowerSave(); 
int  Main_Process_Always();

int  Main_GSM_BadTime_Process_PowerSave(int type); //
int  Main_GSM_Report_Process_PowerSave(); //
int  Main_GSM_Alert_Process_PowerSave(); // 
int  Main_GSM_BadTime_Process_Always(int type); //
int  Main_GSM_Report_Process_Always(); //
int  Main_GSM_Alert_Process_Always(); // 
int  Main_GSM_Idle_Process();

int  Main_DTU_BadTime_Process_PowerSave(int type); //
int  Main_DTU_Report_Process_PowerSave(); //
int  Main_DTU_Alert_Process_PowerSave(); //
int  Main_DTU_BadTime_Process_Always(int type); //
int  Main_DTU_Report_Process_Always(); //
int  Main_DTU_Alert_Process_Always(); //
int  Main_DTU_Idle_Process();

/*
int  Main_485_BadTime_Process_PowerSave(int type); //
int  Main_485_Report_Process_PowerSave(); //
int  Main_485_Alert_Process_PowerSave(); //
int  Main_485_BadTime_Process_Always(int type); //
int  Main_485_Report_Process_Always(); //
int  Main_485_Alert_Process_Always(); //
int  Main_485_Idle_Process();
*/
 
//void Hydrology_ReadIO_STATUS(char *value);
//void Hydrology_SetIO_STATUS(char *value);
void Main_GotoSleep();
int  Main_Init_Process();
int  Main_ProcCommand(char * _data, int _dataLen,int * _step);
int Main_ProcInitConfig(char* _data,int _dataLen,int* _step);

