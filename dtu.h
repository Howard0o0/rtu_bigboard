//////////////////////////////////////////////////////
//     文件名: dtu.h
//   文件版本: 1.0.0
//   创建时间: 09年11月30日
//   更新内容:  
//       作者: 林智
//       附注: 无
//
//////////////////////////////////////////////////////

#pragma once
int  DTU_Open(int _wait); 
int  DTU_CheckOK();
void DTU_Close();
int  DTU_Report_Process();
int  DTU_Waiting_Process(int type);
int  DTU_Alert_Process();
int  DTU_SendData(char * _data, char _len);
int  DTU_ProcRecvData(char * _data, int _dataLen); 
int  DTU_ProcAllData();
  
