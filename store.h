//////////////////////////////////////////////////////
//     文件名: store.h
//   文件版本: 1.0.0
//   创建时间: 09年 11月30日
//   更新内容:  
//       作者: 林智
//       附注: 无
//
//////////////////////////////////////////////////////

#pragma once

////////////////////////////////////////
//  ROM区域分配  地址为int类型,11位 
//  起始地址: 0x0000  最高地址: 0x07FF 
////////////////////////////////////////

//00 u6
#define ROM1_START 0x00000
//01 u5
#define ROM2_START 0x20000 
//10 u4
#define ROM3_START 0x40000
//11 u3
#define ROM4_START 0x60000

//    0x0000 ~ 0x0035   设备基本配置
//    0x0036 ~ 0x0079   各系统配置
//    0x00A0 ~ 0x07FF   数据区 

#define           BASE_ADDR    0x0000


//设置完整性字节               1字节,  0x0000  
#define         CONFIG_ADDR    BASE_ADDR 

//设备号                       11字节, 0x0001 ~ 0x000B
#define       DEVICENO_ADDR    (BASE_ADDR + 1 )
#define       DEVICENO_LEN     11
//密码                         4字节,  0x000C ~ 0x000F   
#define       PASSWORD_ADDR    (BASE_ADDR + 12)
#define       PASSWORD_LEN     4
//报告 时间间隔模式            2字节,  0x0010 ~ 0x0011
#define REPORTTIME_MODE_ADDR   (BASE_ADDR + 16)
#define REPORTTIME_MODE_LEN    2 
//保存 时间间隔模式            2字节,  0x0012 ~ 0x0013
#define   SAVETIME_MODE_ADDR   (BASE_ADDR + 18)
#define   SAVETIME_MODE_LEN    2 


//数据上下限  数据的基地址  
#define DATA_LIMIT_BASE_ADDR   (BASE_ADDR + 20)
#define DATA_MAX_BASE_ADDR     DATA_LIMIT_BASE_ADDR
//                                    0x0015
#define DATA_MIN_BASE_ADDR     (DATA_LIMIT_BASE_ADDR + 2 )

#define     DATA_LIMIT_LEN     2
//数据1报警上限                2字节,  0x0014~0x0015  
#define     DATA1_MAX_ADDR     DATA_MAX_BASE_ADDR
//数据1报警下限                2字节,  0x0016~0x0017  
#define     DATA1_MIN_ADDR     DATA_MIN_BASE_ADDR
//数据2报警上限                2字节,  0x0018~0x0019  
#define     DATA2_MAX_ADDR     (DATA_MAX_BASE_ADDR+4)
//数据2报警下限                2字节,  0x001A~0x001B  
#define     DATA2_MIN_ADDR     (DATA_MIN_BASE_ADDR+4)
//数据3报警上限                2字节,  0x001C~0x001D  
#define     DATA3_MAX_ADDR     (DATA_MAX_BASE_ADDR+8)
//数据3报警下限                2字节,  0x001E~0x001F  
#define     DATA3_MIN_ADDR     (DATA_MIN_BASE_ADDR+8)
//数据4报警上限                2字节,  0x0020~0x0021  
#define     DATA4_MAX_ADDR     (DATA_MAX_BASE_ADDR+12)
//数据4报警下限                2字节,  0x0022~0x0023  
#define     DATA4_MIN_ADDR     (DATA_MIN_BASE_ADDR+12)
//数据5报警上限                2字节,  0x0024~0x0025  
#define     DATA5_MAX_ADDR     (DATA_MAX_BASE_ADDR+16)
//数据5报警下限                2字节,  0x0026~0x0027  
#define     DATA5_MIN_ADDR     (DATA_MIN_BASE_ADDR+16)
//数据6报警上限                2字节,  0x0028~0x0029  
#define     DATA6_MAX_ADDR     (DATA_MAX_BASE_ADDR+20)
//数据6报警下限                2字节,  0x002A~0x002B  
#define     DATA6_MIN_ADDR     (DATA_MIN_BASE_ADDR+20)
//数据7报警上限                2字节,  0x002C~0x002D  
#define     DATA7_MAX_ADDR     (DATA_MAX_BASE_ADDR+24)
//数据7报警下限                2字节,  0x002E~0x002F  
#define     DATA7_MIN_ADDR     (DATA_MIN_BASE_ADDR+24)
//数据8报警上限                2字节,  0x0030~0x0031  
#define     DATA8_MAX_ADDR     (DATA_MAX_BASE_ADDR+28)
//数据8报警下限                2字节,  0x0032~0x0033  
#define     DATA8_MIN_ADDR     (DATA_MIN_BASE_ADDR+28)

//通道配置                     3字节
#define CHANNEL_CONFIG_ADDR    (DATA_LIMIT_BASE_ADDR + 32 )
//                             0x0034
#define     ANALOG_SEL_ADDR    CHANNEL_CONFIG_ADDR
//                             0x0035  低4位忽略
#define     PULSE_SEL_ADDR     (CHANNEL_CONFIG_ADDR + 1)
//                             0x0036
#define     ONOFF_SEL_ADDR     (CHANNEL_CONFIG_ADDR + 2)
//                             0x0037
#define     WORK_MODE_ADDR     (CHANNEL_CONFIG_ADDR + 3)
//                             0x0038
#define     SYSTEM_TYPE_ADDR   (CHANNEL_CONFIG_ADDR + 4) 
//                             0x0039
#define PULSE_RATE_BASE_ADDR   (CHANNEL_CONFIG_ADDR + 5)
#define     PULSE1_RATE_ADDR   PULSE_RATE_BASE_ADDR
//                             0x003A 
#define     PULSE2_RATE_ADDR   (CHANNEL_CONFIG_ADDR + 6)
//                             0x003B                      
#define     PULSE3_RATE_ADDR   (CHANNEL_CONFIG_ADDR + 7)
//                             0x003C
#define     PULSE4_RATE_ADDR   (CHANNEL_CONFIG_ADDR + 8)


//  以几个9来表示 脉冲表的量程

//                             0x003D 
#define PULSE_RANGE_BASE_ADDR  (CHANNEL_CONFIG_ADDR + 9)
#define   PULSE1_RANGE_ADDR    PULSE_RANGE_BASE_ADDR    
//                             0x003E
#define   PULSE2_RANGE_ADDR    (PULSE_RANGE_BASE_ADDR + 1)
//                             0x003F   
#define   PULSE3_RANGE_ADDR    (PULSE_RANGE_BASE_ADDR + 2)
//                             0x0040   
#define   PULSE4_RANGE_ADDR    (PULSE_RANGE_BASE_ADDR + 3)

//                             0x0041
#define      IO_DIR_CFG_ADDR   (CHANNEL_CONFIG_ADDR + 13)
//                             0x0042
#define    IO_LEVEL_CFG_ADDR   (CHANNEL_CONFIG_ADDR + 14)

#define     Config_END_ADDR    0x004F
//////////////////////////////////////
//
//  GSM 系统的分配
//
///////////////////////////////////////
//                                     0x0050
#define     GSM_START_ADDR             0x0050


#define     GSM_CENTERPHONE_BASE_ADDR  GSM_START_ADDR
#define     GSM_CENTER_PHONE_LEN       11

//                                     0x0050    
//   数据中心1                         11字节,   0x0050 -5A
#define     GSM_CENTER1_PHONE_ADDR     ( GSM_CENTERPHONE_BASE_ADDR )
//   数据中心2                         11字节,   0x005B 
#define     GSM_CENTER2_PHONE_ADDR     ( GSM_CENTERPHONE_BASE_ADDR + 11)
//   数据中心3                         11字节,   0x0066
#define     GSM_CENTER3_PHONE_ADDR     ( GSM_CENTERPHONE_BASE_ADDR + 22)
//   数据中心4                         11字节,   0x0071
#define     GSM_CENTER4_PHONE_ADDR     ( GSM_CENTERPHONE_BASE_ADDR + 33)
//   配置中心                          11字节,   0x007C
#define      GSM_CONFIG_PHONE_ADDR     ( GSM_CENTERPHONE_BASE_ADDR + 44)
//   负责人电话                        11字节,   0x0087
#define      GSM_LEADER_PHONE_ADDR     ( GSM_CENTERPHONE_BASE_ADDR + 55) 
//                                     0x0091  
#define     GSM_CENTERPHONE_END_ADDR   ( GSM_CENTERPHONE_BASE_ADDR + 65 )



///////////////////////////////////////
//
//  DTU 系统的分配 
//
///////////////////////////////////////

//                                     0x0050 
#define     DTU_START_ADDR             0x0050
#define     DTU_CENTERIP_BASE_ADDR     DTU_START_ADDR
#define     CENTER_IP_LEN              6      
//   数据中心1                         6字节,  0x0050 ~  
#define     CENTER1_IP_ADDR           (DTU_CENTERIP_BASE_ADDR)
//   数据中心2                         6字节,  0x0056 ~  
#define     CENTER2_IP_ADDR            (DTU_CENTERIP_BASE_ADDR + 6)
//   数据中心3                         6字节,  0x005B ~  
#define     CENTER3_IP_ADDR            (DTU_CENTERIP_BASE_ADDR + 12)
//   数据中心4                         6字节,  0x0050 ~  
#define     CENTER4_IP_ADDR            (DTU_CENTERIP_BASE_ADDR + 18)
//   数据中心5                         6字节,  0x0055 ~  
#define     CENTER5_IP_ADDR            (DTU_CENTERIP_BASE_ADDR + 24)


//////////////////////////////////////
//
//  485 系统的分配 
//
///////////////////////////////////////


//数据区
//                                    0x00A0
#define     DATA_START_ADDR           0x00A0
#define     DATA_ITEM_LEN             40     //多加一条cameraaddress
#define     DATA_ITEM_MAX             DATA_MAX_IDX
#define     DATA_END_ADDR             (DATA_START_ADDR + DATA_ITEM_MAX * DATA_ITEM_LEN)


#define     DATA_MAX_IDX              20
#define     DATA_MIN_IDX              1  


//WiFi配置标识
#define       WIFI_CONFIG_FLAG_ADDR                     0x0400
#define       WIFI_CONFIG_FLAG_LEN                      1

//水文规约协议
#define       HYDROLOGYCONFIGDATA_ADDR                  0x0402
#define       HYDROLOGYCONFIGDATA_ADDR_LEN              sizeof(hydrologyConfigPara)

//#define       HYDROLOGY_CENTER_LENGTH                   4
//
//#define       HYDROLOGY_REMOTE_LENGTH                   5
//
//#define       HYDROLOGY_CENTER_TYPE_LENGTH              1
//#define       HYDROLOGY_CENTER_ADDR2_LENGTH             9
//#define       HYDROLOGY_SERVERIP_LENGTH                 6
//#define       HYDROLOGY_SERVERPORT_LENGTH               3
//#define       HYDROLOGY_BACKUP_TYPE_LENGTH              1
//#define       HYDROLOGY_BACKUP_ADDR_LENGTH              6
//
//#define       HYDROLOGY_CENTER1                         1
//#define       HYDROLOGY_CENTER2                         2
//#define       HYDROLOGY_CENTER3                         3
//#define       HYDROLOGY_CENTER4                         4
//
//#define       HYDROLOGY_WORK_MODE_LENGTH                1
//
//#define       HYDROLOGY_ELEMENT_SELECT_LENGTH           8
//#define       HYDROLOGY_SERVERAPN_LENGTH                1
//#define       HYDROLOGY_MOBILEPHONE_LENGTH              11
//
//#define       HYDROLOGY_TIMER_INTERVAL_LEN              1
//#define       HYDROLOGY_ADD_INTERVAL_LEN                1
//#define       HYDROLOGY_RAINFALL_BEGIN_TIME_LEN         1
//#define       HYDROLOGY_SAMPLE_INTERVAL_LEN             2
//#define       HYDROLOGY_WATERLEVEL_STORE_INTERVAL_LEN   1
//#define       HYDROLOGY_RAINFALL_RESOLUTION_LEN         1
//#define       HYDROLOGY_WATERLEVEL_RESOLUTION_LEN       1
//#define       HYDROLOGY_RAINFALL_ADD_THRESHOLD_LEN      1
//#define       HYDROLOGY_WATERLEVEL_BASIC_VALUE_LEN      4
//#define       HYDROLOGY_WATERLEVEL_CORRECT_VALUE_LEN    3
//#define       HYDROLOGY_ADD_WATERLEVEL_LEN              2
//#define       HYDROLOGY_ADD_ABOVE_THRESHOLD_LEN         2
//#define       HYDROLOGY_ADD_BELOW_THRESHOLD_LEN         2
//
//#define       HYDROLOGY_SOFTWARE_VERSION_LEN            17
//#define       HYDROLOGY_STATUS_ALARM_INFO_LEN           4
//
//#define       SAMPLETIME_LEN                            5
//#define       NOWPRECIPITATION_LEN                      3
//#define       TOTALPRECIPITATION_LEN                    3
//#define       INSTANTWATERLEVEL_LEN                     4
//#define       FIVEMINPRECIPITATION_LEN                  12
//#define       FIVEMINRELATIVEWATERLEVEL_LEN             24
//#define       VOLTAGE_LEN                               2
//
//#define       TIMESTEP_LEN                              3
//#define       EVENPERIODELEMENT_LEN                     4
//
//#define       ARTIFICIAL_NUMNER_LEN                     51
//#define       ARTIFICIAL_NUMNER_SIZE_LEN                1
//#define       ARTIFICIAL_REMOTE_ADDR_ID_LEN             2
//#define       ARTIFICIAL_REMOTE_ADDR_LEN                10
//#define       ARTIFICIAL_RTUTYPE_LEN                    1
//#define       ARTIFICIAL_SAMPLETIME_ID_LEN              2
//#define       ARTIFICIAL_SAMPLETIME_LEN                 14
//#define       ARTIFICIAL_ELEMENT_ID_LEN                 2
//#define       ARTIFICIAL_ELEMENT_LEN                    4
//
//#define       HYDROLOGY_PUMP_LEN                        2
//#define       HYDROLOGY_VALVE_LEN                       2
//#define       HYDROLOGY_GATE_LEN                        18
//#define       HYDROLOGY_PUMP_SIZE_LEN                   1
//#define       HYDROLOGY_VALVE_SIZE_LEN                  1
//#define       HYDROLOGY_GATE_SIZE_LEN                   1
//#define       HYDROLOGY_WATERSETTING_LEN                1
//#define       HYDROLOGY_RECORD_LEN                      2
//
//#define       HYDROLOGY_PICTURE_SIZE_LEN                2
//#define       HYDROLOGY_PICTURE_SINGLE_LEN              100
//#define       HYDROLOGY_PICTURE_LEN                     350
//
//#define       HYDROLOGY_CONFIG_ADDR                     0x0480
//
//#define       HYDROLOGY_CENTER_ADDR                     HYDROLOGY_CONFIG_ADDR + 0
//
//#define       HYDROLOGY_REMOTE_ADDR                     HYDROLOGY_CENTER_ADDR + 4
//
//#define       HYDROLOGY_CENTER_TYPE1_ADDR               HYDROLOGY_REMOTE_ADDR + 5
//#define       HYDROLOGY_CENTER_ADDR21_ADDR              HYDROLOGY_CENTER_TYPE1_ADDR + 1
//#define       HYDROLOGY_SERVERIP1_ADDR                  HYDROLOGY_CENTER_ADDR21_ADDR + 9
//#define       HYDROLOGY_SERVERPORT1_ADDR                HYDROLOGY_SERVERIP1_ADDR + 6
//#define       HYDROLOGY_CENTER_BACKUP1_TYPE             HYDROLOGY_SERVERPORT1_ADDR + 3
//#define       HYDROLOGY_CENTER_BACKUP1_ADDR             HYDROLOGY_CENTER_BACKUP1_TYPE + 1
//
//#define       HYDROLOGY_CENTER_TYPE2_ADDR               HYDROLOGY_CENTER_BACKUP1_ADDR + 6
//#define       HYDROLOGY_CENTER_ADDR22_ADDR              HYDROLOGY_CENTER_TYPE2_ADDR + 1
//#define       HYDROLOGY_SERVERIP2_ADDR                  HYDROLOGY_CENTER_ADDR22_ADDR + 9
//#define       HYDROLOGY_SERVERPORT2_ADDR                HYDROLOGY_SERVERIP2_ADDR + 6
//#define       HYDROLOGY_CENTER_BACKUP2_TYPE             HYDROLOGY_SERVERPORT2_ADDR + 3
//#define       HYDROLOGY_CENTER_BACKUP2_ADDR             HYDROLOGY_CENTER_BACKUP2_TYPE + 1
//
//#define       HYDROLOGY_CENTER_TYPE3_ADDR               HYDROLOGY_CENTER_BACKUP2_ADDR + 6
//#define       HYDROLOGY_CENTER_ADDR23_ADDR              HYDROLOGY_CENTER_TYPE3_ADDR + 1
//#define       HYDROLOGY_SERVERIP3_ADDR                  HYDROLOGY_CENTER_ADDR23_ADDR + 9
//#define       HYDROLOGY_SERVERPORT3_ADDR                HYDROLOGY_SERVERIP3_ADDR + 6
//#define       HYDROLOGY_CENTER_BACKUP3_TYPE             HYDROLOGY_SERVERPORT3_ADDR + 3
//#define       HYDROLOGY_CENTER_BACKUP3_ADDR             HYDROLOGY_CENTER_BACKUP3_TYPE + 1
//
//#define       HYDROLOGY_CENTER_TYPE4_ADDR               HYDROLOGY_CENTER_BACKUP3_ADDR + 6
//#define       HYDROLOGY_CENTER_ADDR24_ADDR              HYDROLOGY_CENTER_TYPE4_ADDR + 1
//#define       HYDROLOGY_SERVERIP4_ADDR                  HYDROLOGY_CENTER_ADDR24_ADDR + 9
//#define       HYDROLOGY_SERVERPORT4_ADDR                HYDROLOGY_SERVERIP4_ADDR + 6
//#define       HYDROLOGY_CENTER_BACKUP4_TYPE             HYDROLOGY_SERVERPORT4_ADDR + 3
//#define       HYDROLOGY_CENTER_BACKUP4_ADDR             HYDROLOGY_CENTER_BACKUP4_TYPE + 1
//
//#define       HYDROLOGY_WORK_MODE                       HYDROLOGY_CENTER_BACKUP4_ADDR + 6
//
//#define       HYDROLOGY_ELEMENT_SELECT                  HYDROLOGY_CONFIG_ADDR + 90
//
//#define       HYDROLOGY_SERVERAPN                       HYDROLOGY_CONFIG_ADDR + 98
//#define       HYDROLOGY_MOBILEPHONE                     HYDROLOGY_CONFIG_ADDR + 99
//
//#define       HYDROLOGY_BASIC_CONFIG_END                HYDROLOGY_CONFIG_ADDR + 110
//
//#define       HYDROLOGY_RUN_PARA                        HYDROLOGY_BASIC_CONFIG_END
//
//#define       HYDROLOGY_TIMER_INTERVAL                  HYDROLOGY_RUN_PARA + 0
//#define       HYDROLOGY_ADD_INTERVAL                    HYDROLOGY_RUN_PARA + 1
//#define       HYDROLOGY_RAINFALL_BEGIN_TIME             HYDROLOGY_RUN_PARA + 2
//#define       HYDROLOGY_SAMPLE_INTERVAL                 HYDROLOGY_RUN_PARA + 3
//#define       HYDROLOGY_WATERLEVEL_STORE_INTERVAL       HYDROLOGY_RUN_PARA + 5
//#define       HYDROLOGY_RAINFALL_RESOLUTION             HYDROLOGY_RUN_PARA + 6
//#define       HYDROLOGY_WATERLEVEL_RESOLUTION           HYDROLOGY_RUN_PARA + 7
//#define       HYDROLOGY_RAINFALL_ADD_THRESHOLD          HYDROLOGY_RUN_PARA + 8
//#define       HYDROLOGY_WATERLEVEL_BASIC_VALUE          HYDROLOGY_RUN_PARA + 9
//#define       HYDROLOGY_WATERLEVEL_CORRECT_VALUE        HYDROLOGY_RUN_PARA + 13
//#define       HYDROLOGY_ADD_WATERLEVEL                  HYDROLOGY_RUN_PARA + 16
//#define       HYDROLOGY_ADD_ABOVE_THRESHOLD             HYDROLOGY_RUN_PARA + 18
//#define       HYDROLOGY_ADD_BELOW_THRESHOLD             HYDROLOGY_RUN_PARA + 20
//
//#define       HYDROLOGY_RUN_PARA_END                    HYDROLOGY_RUN_PARA + 22
//
//#define       HYDROLOGY_SOFTWARE_VERSION                HYDROLOGY_RUN_PARA_END
//
//#define       HYDROLOGY_STATUS_ALARM_INFO               HYDROLOGY_SOFTWARE_VERSION + 17
//
//#define       HYDROLOGY_CONFIG_END                      HYDROLOGY_STATUS_ALARM_INFO + 4
//
//#define       NOWPRECIPITATION_SAMPLETIME               HYDROLOGY_CONFIG_END + 0
//#define       NOWPRECIPITATION                          NOWPRECIPITATION_SAMPLETIME + 5
//#define       TOTALPRECIPITATION_SAMPLETIME             NOWPRECIPITATION + 3
//#define       TOTALPRECIPITATION                        TOTALPRECIPITATION_SAMPLETIME + 5
//#define       INSTANTWATERLEVEL_SAMPLETIME              TOTALPRECIPITATION + 3
//#define       INSTANTWATERLEVEL                         INSTANTWATERLEVEL_SAMPLETIME + 5
//#define       FIVEMINPRECIPITATION_SAMPLETIME           INSTANTWATERLEVEL + 4
//#define       FIVEMINPRECIPITATION                      FIVEMINPRECIPITATION_SAMPLETIME + 5
//#define       FIVEMINRELATIVEWATERLEVEL_SAMPLETIME      FIVEMINPRECIPITATION + 12
//#define       FIVEMINRELATIVEWATERLEVEL                 FIVEMINRELATIVEWATERLEVEL_SAMPLETIME + 5
//#define       VOLTAGE                                   FIVEMINRELATIVEWATERLEVEL + 24
//
//#define       EVENPERIODELEMENT_SAMPLETIME              VOLTAGE + 2
//#define       TIMESTEP                                  EVENPERIODELEMENT_SAMPLETIME + 5
//#define       EVENPERIODELEMENT                         TIMESTEP + 3
//
//#define       ARTIFICIAL_NUMNER                         EVENPERIODELEMENT + 4 * 12
//#define       ARTIFICIAL_NUMNER_SIZE                    EVENPERIODELEMENT + 4 * 12
//#define       ARTIFICIAL_REMOTE_ADDR_ID                 ARTIFICIAL_NUMNER_SIZE + 1
//#define       ARTIFICIAL_REMOTE_ADDR                    ARTIFICIAL_REMOTE_ADDR_ID + 2
//#define       ARTIFICIAL_RTUTYPE                        ARTIFICIAL_REMOTE_ADDR + 10
//#define       ARTIFICIAL_SAMPLETIME_ID                  ARTIFICIAL_RTUTYPE + 1
//#define       ARTIFICIAL_SAMPLETIME                     ARTIFICIAL_SAMPLETIME_ID + 2
//#define       ARTIFICIAL_ELEMENT_ID                     ARTIFICIAL_SAMPLETIME + 14
//#define       ARTIFICIAL_ELEMENT                        ARTIFICIAL_ELEMENT_ID + 2
//
//#define       HYDROLOGY_PUMP                            ARTIFICIAL_NUMNER + 51
//#define       HYDROLOGY_VALVE                           HYDROLOGY_PUMP + 2
//#define       HYDROLOGY_GATE                            HYDROLOGY_VALVE + 2
//#define       HYDROLOGY_WATERSETTING                    HYDROLOGY_GATE + 2 + 2 * 8
//#define       HYDROLOGY_RECORD                          HYDROLOGY_WATERSETTING + 1
//
//#define       HYDROLOGY_PICTURE_SIZE                    HYDROLOGY_RECORD + 2 * 32
//#define       HYDROLOGY_PICTURE                         HYDROLOGY_PICTURE_SIZE + 2
//
//#define       HOUR_SAMPLETIME                           HYDROLOGY_PICTURE + 350
//#define       HOURPRECIPITATION                         HOUR_SAMPLETIME + 5
//#define       LASTHOURPRECIPITATION                     HOURPRECIPITATION + 5
//
//#define       HYDROLOGY_END                             LASTHOURPRECIPITATION + 3

void Store_Init();   
 
int  Store_ReadDataMaxInt(int index, int *destInt); //int数值
int  Store_ReadDataMaxStr(int index, char *dest);   //4位数字
int  Store_SetDataMaxInt(int index , int temp);
int  Store_ReadDataMinInt(int index, int *destInt);
int  Store_ReadDataMinStr(int index, char *dest);
int  Store_SetDataMinInt(int index,  int temp);

//保存的数据条
int  Store_ClearWork();
int  Store_EraseData();

int  Store_CheckDataItemSended(int _index); //检查发送标记
int  Store_MarkDataItemSended(int _index);  //设置发送标记
int  Store_ReadDataItem(int _index , char * _dest, int _flag);
//int  Store_ReadDataItemAuto(char * _dest);

int  New_Store_ReadDataItem(char _index , char * _dest, int _flag);  //用作新水利测试
int  Store_WriteDataItem( int  _index ,  char * _src);
int  Store_WriteDataItemAuto( char * _src);

//设置参数
int  Store_ReadConfig(char *dest); 
int  Store_SetConfig(char config);
int  Store_ReadDeviceNO(char *dest);
int  Store_SetDeviceNO(const char *src);
int  Store_ReadPassword(char *dest);
int  Store_SetPassword(const char *src);
int  Store_ReadReportTimeMode(char *dest); //2个字符
int  Store_SetReportTimeMode(const char * src);
int  Store_ReadSaveTimeMode(char *dest);
int  Store_SetSaveTimeMode(const char * src);
int  Store_ReadWorkMode(char * _dest);
int  Store_SetWorkMode(char _src);
int  Store_ReadSystemType(char * _dest);
int  Store_SetSystemType(char _src);

//设置WIFI配置标识  by 刘梦君 2017-7-16
int Store_SetWIFIConfigFlag(char * _src);

//读WIFI配置标识  by 刘梦君 2017-7-16
int Store_ReadWIFIConfigFlag(char * _dest);



int Store_ReadCameraTimeMode(char * _dest);
int Store_SetCameraTimeMode(const char * _src);

int  Store_ReadAnalogSelect(char * _dest);
int  Store_SetAnalogSelect(char _src);
int  Store_SetAnalogSelectStr(const char * _src);


int  Store_ReadPulseSelect(char * _dest);
int  Store_SetPulseSelect(char _src);
int  Store_SetPulseSelectStr(const char * _src);



int  Store_ReadIoSelect(char * _dest);
int  Store_SetIoSelect(char _src);
int  Store_SetIoSelectStr(const char*  _src);


int  Store_ReadPulseRate(int _index, char * _dest);
int  Store_SetPulseRate(int _index, char _src);


int  Store_ReadPulseRangeBytes(int _index, char *_dest);
int  Store_ReadPulseRange(int _index, char *_dest);
int  Store_SetPulseRange(int _index, char _src);

int  Store_ReadIoDirConfigStr(char * _dest);
int  Store_ReadIoDirConfig(char * _dest);
int  Store_SetIoDirConfig(char _src);
int  Store_SetIoDirConfigStr(char * _src);

int  Store_ReadIoLevelConfigStr(char * _dest);
int  Store_ReadIoLevelConfig(char * _dest);
int  Store_SetIoLevelConfig(char _src);
int  Store_SetIoLevelConfigStr(char * _src);

//GSM
int  Store_GSM_ReadCenterPhone(int index, char *dest);  
int  Store_GSM_SetCenterPhone(int index, char *src); 



//hydrology
//int Store_SetHydrologyCenterAddr(char* _src);
//int Store_ReadHydrologyCenterAddr(char* _src);
//int Store_SetHydrologyRemoteAddr(char* _src);
//int Store_ReadHydrologyRemoteAddr(char* _src);
//int Store_SetHydrologyCenterType(char* _src,int center);
//int Store_ReadHydrologyCenterType(char* _src,int center);
//int Store_SetHydrologyServerIP(char* _src,int center);
//int Store_ReadHydrologyServerIP(char* _src,int center);
//int Store_SetHydrologyServerPort(char* _src,int center);
//int Store_ReadHydrologyServerPort(char* _src,int center);
//int Store_SetHydrologyBackUpType(char* _src,int center);
//int Store_ReadHydrologyBackUpType(char* _src,int center);
//int Store_SetHydrologyBackUpAddr(char* _src,int center);
//int Store_ReadHydrologyBackUpAddr(char* _src,int center);
//int Store_SetHydrologyCenter(char* _src,int center);
//int Store_ReadHydrologyCenter(char* _src,int center);
//int Store_SetHydrologyBackUp(char* _src,int center);
//int Store_ReadHydrologyBackUp(char* _src,int center);
//int Store_SetHydrologyWorkMode(char* _src);
//int Store_ReadHydrologyWorkMode(char* _src);
//int Store_SetHydrologyElementSelect(char* _src);
//int Store_ReadHydrologyElementSelect(char* _src);
//int Store_SetHydrologyServerAPN(char* _src);
//int Store_ReadHydrologyServerAPN(char* _src);
//int Store_SetHydrologyMobilephone(char* _src);
//int Store_ReadHydrologyMobilephone(char* _src);
//int Store_SetHydrologyDeviceID(char* _src);
//int Store_ReadHydrologyDeviceID(char* _src);
//
//int Store_SetHydrologyTimerInterval(char* _src);
//int Store_ReadHydrologyTimerInterval(char* _src);
//int Store_SetHydrologyAddInterval(char* _src);
//int Store_ReadHydrologyAddInterval(char* _src);
//int Store_SetHydrologyRainfallBeginTime(char* _src);
//int Store_ReadHydrologyRainfallBeginTime(char* _src);
//int Store_SetHydrologySampleInterval(char* _src);
//int Store_ReadHydrologySampleInterval(char* _src);
//int Store_SetHydrologyWaterlevelStoreInterval(char* _src);
//int Store_ReadHydrologyWaterlevelStoreInterval(char* _src);
//int Store_SetHydrologyRainfallResolution(char* _src);
//int Store_ReadHydrologyRainfallResolution(char* _src);
//int Store_SetHydrologyWaterlevelResolution(char* _src);
//int Store_ReadHydrologyWaterlevelResolution(char* _src);
//int Store_SetHydrologyRainfallAddThreshold(char* _src);
//int Store_ReadHydrologyRainfallAddThreshold(char* _src);
//int Store_SetHydrologyWaterlevelBasicValue(char* _src);
//int Store_ReadHydrologyWaterlevelBasicValue(char* _src);
//int Store_SetHydrologyWaterlevelCorrectValue(char* _src);
//int Store_ReadHydrologyWaterlevelCorrectValue(char* _src);
//int Store_SetHydrologyAddWaterlevel(char* _src);
//int Store_ReadHydrologyAddWaterlevel(char* _src);
//int Store_SetHydrologyAddAboveThreshold(char* _src);
//int Store_ReadHydrologyAddAboveThreshold(char* _src);
//int Store_SetHydrologyAddBelowThreshold(char* _src);
//int Store_ReadHydrologyAddBelowThreshold(char* _src);
//
//int Store_SetHydrologySoftwareVersion(char* _src,int writelen);
//int Store_ReadHydrologySoftwareVersion(char* _dest,int readlen);
//int Store_ReadHydrologySoftwareVersionSize(char* _dest);
//int Store_SetHydrologyStatusAlarmInfo(char* _src);
//int Store_ReadHydrologyStatusAlarmInfo(char* _src);
//
//int Store_SetNowPrecipitationSampleTime(char* _src);
//int Store_ReadNowPrecipitationSampleTime(char* _src);
//int Store_SetHydrologyNowPrecipitation(char* _src);
//int Store_ReadHydrologyNowPrecipitation(char* _src);
//int Store_SetTotalPrecipitationSampleTime(char* _src);
//int Store_ReadTotalPrecipitationSampleTime(char* _src);
//int Store_SetHydrologyTotalPrecipitation(char* _src);
//int Store_ReadHydrologyTotalPrecipitation(char* _src);
//int Store_SetInstantWaterLevelSampleTime(char* _src);
//int Store_ReadInstantWaterLevelSampleTime(char* _src);
//int Store_SetHydrologyInstantWaterLevel(char* _src);
//int Store_ReadHydrologyInstantWaterLevel(char* _src);
//int Store_SetFiveMinPrecipitationSampleTime(char* _src);
//int Store_ReadFiveMinPrecipitationSampleTime(char* _src);
//int Store_SetHydrologyFiveMinPrecipitation(char* _src);
//int Store_ReadHydrologyFiveMinPrecipitation(char* _src);
//int Store_SetSingleFiveMinPrecipitation(char* _src,int index);
//int Store_ReadSingleFiveMinPrecipitation(char* _src,int index);
//int Store_SetFiveMinRelativeWaterLevelSampleTime(char* _src);
//int Store_ReadFiveMinRelativeWaterLevelSampleTime(char* _src);
//int Store_SetHydrologyFiveMinRelativeWaterLevel(char* _src);
//int Store_ReadHydrologyFiveMinRelativeWaterLevel(char* _dest);
//int Store_SetSingleFiveMinRelativeWaterLevel(char* _src,int index);
//int Store_ReadSingleFiveMinRelativeWaterLevel(char* _src,int index);
//int Store_SetHydrologyVoltage(char* _src);
//int Store_ReadHydrologyVoltage(char* _src);
//
//int Store_SetEvenPeriodElementSampleTime(char* _src);
//int Store_ReadEvenPeriodElementSampleTime(char* _src);
//int Store_SetHydrologyTimeStep(char* _src);
//int Store_ReadHydrologyTimeStep(char* _src);
//int Store_SetHydrologyEvenPeriodElement(char* _src,int index);
//int Store_ReadHydrologyEvenPeriodElement(char* _src,int index);
//
//int Store_SetArtificialNumber(char* _src,int writelen);
//int Store_ReadArtificialNumber(char* _dest,int readlen);
//int Store_ReadArtificialNumberSize(char* _dest);
//int Store_SetArtificialRemoteAddrId(char* _src);
//int Store_ReadArtificialRemoteAddrId(char* _src);
//int Store_SetArtificialRemoteAddr(char* _src);
//int Store_ReadArtificialRemoteAddr(char* _src);
//int Store_SetArtificialRtuType(char* _src);
//int Store_ReadArtificialRtuType(char* _src);
//int Store_SetArtificialSampleTimeId(char* _src);
//int Store_ReadArtificialSampleTimeId(char* _src);
//int Store_SetArtificialSampleTime(char* _src);
//int Store_ReadArtificialSampleTime(char* _src);
//int Store_SetArtificialElementId(char* _src);
//int Store_ReadArtificialElementId(char* _src);
//int Store_SetArtificialElement(char* _src);
//int Store_ReadArtificialElement(char* _src);

//int Store_SetInitializeSolidStorage();
//
//int Store_SetReset();
//
//int Store_SetHydrologyPump(char* _src);
//int Store_ReadHydrologyPump(char* _src);
//int Store_ReadHydrologyPumpSize(char* _src);
//int Store_SetHydrologyValve(char* _src);
//int Store_ReadHydrologyValve(char* _src);
//int Store_ReadHydrologyValveSize(char* _src);
//int Store_SetHydrologyGate(char* _src,int writelen);
//int Store_ReadHydrologyGate(char* _dest,int readlen);
//int Store_ReadHydrologyGateSize(char* _src);
//int Store_SetHydrologyWaterSetting(char* _src);
//int Store_ReadHydrologyWaterSetting(char* _src);
//int Store_SetHydrologyRecord(char* _src,int index);
//int Store_ReadHydrologyRecord(char* _src,int index);
//int Store_ReadHydrologyOverallRecord(char* _dest);
//
//int Store_SetHydrologyPictureSize(char* _src);
//int Store_ReadHydrologyPictureSize(char* _src);
//int Store_SetHydrologyPicture(char* _src,int writelen,int writeaddr);
//int Store_ReadHydrologyPicture(char* _dest,int readlen,int readaddr);
//
//int Store_SetHourSampleTime(char* _src);
//int Store_ReadHourSampleTime(char* _src);
//int Store_SetHydrologyHourPrecipitation(char* _src);
//int Store_ReadHydrologyHourPrecipitation(char* _src);
//int Store_SetHydrologyLastHourPrecipitation(char* _src);
//int Store_ReadHydrologyLastHourPrecipitation(char* _src);

int Store_SetHydrologyConfigData(char* _src);
int Store_ReadHydrologyConfigData(char* _dest);







