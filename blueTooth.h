/******************************************/
//      author: zh
//      date：2019.10.18

/******************************************/
#ifndef	_BLE_
#define _BLE_

#pragma once

#define BLESTARTSTRING "AT:SBM14580-Start\r\n"
#define BLESTARTSTRLEN  19
#define BLECONNECTSTRING "AT:BLE-CONN-OK\r\n"
#define BLECONNECTSTRINGLEN 16


#define BLEBUFOF  "AT:BUF-OF\r\n"
#define BLEBUFON  "AT:BUF-ON\r\n"


typedef enum tagBLEResult
{
	BLE_SUCCESS = 0,
	BLE_ERROR

} BLERet;

typedef enum tagBLEErrorCode
{
	BLE_OK = 0,
	BLE_VALIDCHECKError = 1,
	BLE_UsrTimeError,
	BLE_DeviceErrror,
	BLE_LockIdError,
	BLE_FunCodeError,
	BLE_UserIdError,
	BLE_DataValidateError,
	BLE_DataLenCheckError,
	BLE_LockPwdError,
	BLE_READKEYError,
	BLE_CRCCHECKError
} BLEErrorCode;

typedef enum tagBLE_STATE
{
	// Connectable state
	BLE_CONNECTABLE,

	// Connected state
	BLE_CONNECTED,

	// Disabled State
	BLE_DISABLED
} BLE_STATE;

///////////外部接口
/*  BLERet:BLE_SUCCESS   BLE_ERROR */
void BleDriverInstall();
BLERet BLE_ADVSTART();                  //开启广播  
BLERet BLE_ADVSTOP();                   //关闭广播                                         
BLERet BLE_CONNECT();             //检查蓝牙是否被链接
int BLE_MAIN();                 //初始化蓝牙 配置蓝牙 广播 等待连接 if 0 connectd , -1 not connected
BLERet BLE_SLEEP();                     //蓝牙进入休眠模式                         
BLERet BLE_RST();                       //重启蓝牙
BLERet BLE_BLESPP();                    //开启透传
BLERet BLE_BLESPPEND();                   //关闭透传
int BLE_ISFINDBT();             //是否检测到蓝牙                    

void SPPRX(char * result,int len);           //RTU接收蓝牙消息
void SPPTX(char * result,int * len);           //蓝牙接收RTU消息
int IsInit();

////////////内部函数

void BLE_buffer_Clear();                                         //清除BUFF                

void BLE_SendMsg(char *atCmd,int cmdLen);
BLERet BLE_BLESP();
BLERet ATTEST();
void BLE_RecAt(char *result,int *num);      			//蓝牙接收AT指令                                   
void BLE_SendAtCmd(char *atCmd,int cmdLen);                  //RTU发送AT指令
BLERet BLE_SetName ( void );                                    //                              
BLERet BLE_SERVER();
BLERet BLE_GATTSSRVCRE();
BLERet BLE_GATTSSRVSTART();
BLERet BLE_BLESPPCFG();                                  //                              
BLERet BLE_INIT();
BLERet BLE_ATE();
BLERet BLE_Open();
void BLE_Close();
int BLE_RecvLineTry ( char* _dest,const int _max, int* _pNum );




#endif