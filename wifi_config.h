/*
 * wifi_config.h
 *
 *  Created on: 2017年6月29日
 *      Author: lmj
 */

#ifndef WIFI_CONFIG_H_
#define WIFI_CONFIG_H_


#define RTU_ADDRESS 5
#define RTU_ADDRESS_MAXNUM 8

#define ADDR_NUM_OFFSETCONFIG 8

#define REMOTE_STATION_ADDR_MAXNUM 3

#define REMOTE_STATION_ADDR_LEN 10
#define CHANNEL_CODE_ID_ADDR_LEN 1
#define CHANNEL_CODE_GUIDING_LEN 2
#define CHANNEL_CODE_DATALEN_LEN 2
#define CHANNEL_CODE_DECIMAL_LEN 2
#define CHANNEL_RANGE_MIN_ADDR_LEN 1
#define CHANNEL_RANGE_MAX_ADDR_LEN 9

#define CENTER_ADDR_LEN 3
#define FUNCTION_CODE_LEN 2
#define REMOTE_STATION_CLASSIFY_CODE_LEN 2
#define REMOTE_STATION_LONGITUDE_LEN 7
#define REMOTE_STATION_LATITUDE_LEN 6 
#define REMOTE_GPRS_SERVER_IP_ADDR_LEN 15
#define REMOTE_GRPS_SERVER_PORT_ADDR_LEN 5
#define REMOTE_GPRS_SERVER_APN_ADDR_LEN 6
#define REMOTE_STATION_NUM_LEN 1

//这个结构体顺序很重要，与配置顺序一对一,不要打乱
typedef struct taghydrologyConfigData
{
  char remoteaddr[REMOTE_STATION_ADDR_LEN+1];
  char rangemin[CHANNEL_RANGE_MIN_ADDR_LEN+1];
  char rangemax[CHANNEL_RANGE_MAX_ADDR_LEN+1];
  char channelid[CHANNEL_CODE_ID_ADDR_LEN+1];
  char channelguide[CHANNEL_CODE_GUIDING_LEN+1];
  char D[CHANNEL_CODE_DATALEN_LEN+1];
  char d[CHANNEL_CODE_DECIMAL_LEN+1];
}hydrologyConfigData;

typedef struct taghydrologyConfigPara
{
    char controladdr[CENTER_ADDR_LEN+1];
    char funcode[FUNCTION_CODE_LEN+1];
    char classifycode[REMOTE_STATION_CLASSIFY_CODE_LEN+1];
    char longitude[REMOTE_STATION_LONGITUDE_LEN+1];
    char latitude[REMOTE_STATION_LATITUDE_LEN+1];
    char serverip[REMOTE_GPRS_SERVER_IP_ADDR_LEN+1];
    char serverport[REMOTE_GRPS_SERVER_PORT_ADDR_LEN+1];
    char serverapn[REMOTE_GPRS_SERVER_APN_ADDR_LEN+1];
    char addrnum[REMOTE_STATION_NUM_LEN+1];
    hydrologyConfigData configdata[REMOTE_STATION_ADDR_MAXNUM];    
}hydrologyConfigPara;


int indexOfColon(char * _data,int _dataLen);//寻找第一个冒号
   
void WIFI_Open();
void WIFI_Close();
int WIFI_Send(char * src, int len);

int WIFI_ReceiveDataProcess();
int WIFI_ReceiveConfigDataProcess(char * dest, int recvLength);
int WIFI_HydrologyConfigProcess(char * src, int dataLength, int *_step);

int WIFI_SendData(char _sendData[], int dataLength);
void WIFI_ClearBuffer(); //清除wifi缓存


int  WIFI_RecvLineTry(char * _dest,const int _max, int * _pNum);
void WIFI_Console_Hydrology(hydrologyConfigPara* pconfigData);


#endif /* WIFI_CONFIG_H_ */
