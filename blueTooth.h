/******************************************/
//     锟侥硷拷锟斤拷: bootloader.h
//      时锟戒：2018.2.1
//      锟斤拷锟竭ｏ拷锟脚猴拷
//      锟芥本锟斤拷1.0
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
BLERet BLE_CONNECT();             //判断是否连接
int BLE_MAIN();                 //重启-》初始化-》连接-》配置透传 if 0 connectd , -1 not connected
BLERet BLE_SLEEP();                     //休眠                         
BLERet BLE_RST();                       //重启
BLERet BLE_BLESPP();                    //开启透传
BLERet BLE_BLESPPEND();                   //关闭透传

void SPPRX(char * result,int len);           //手机接受
void SPPTX(char * result,int * len);           //手机发送	末尾加0D0A 回车
int IsInit();

////////////内部

void BLE_buffer_Clear();                                         //清BUFF                

void BLE_SendMsg(char *atCmd,int cmdLen);
BLERet BLE_BLESP();
BLERet ATTEST();
void BLE_RecAt(char *result,int *num);      					//RTU接受                                    
void BLE_SendAtCmd(char *atCmd,int cmdLen);                  //RTU发送
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