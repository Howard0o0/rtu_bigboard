#ifndef REMPORTBINDING_H
#define REMPORTBINDING_H
#include "wifi_config.h"
//遥测端，即RTU地址
#define RTU_ADDRESS 5 

#define RTU_SAMPLERPORT_MAXNUM 8

typedef enum _SampleDataPortType
{
   //开关量M-T
  PORT_SWITCH,
  
  //脉冲量I-L
  PORT_PULSE,
  
  //模拟量A-H
  PORT_Analogy,
  
  PORT_MAX
}SampleDataPortType;

typedef struct _ReportBindingInfo
{
   char SamplePort; //采集端口 
   SampleDataPortType porttype;//采集端口类型
   char element;//采集端口对应的编码要素
   char rtuAddr[RTU_ADDRESS];
   char volflag; //1 //1表示电压，0表示其他编码要素
   char minRange[CHANNEL_RANGE_MIN_ADDR_LEN];
   char maxRange[CHANNEL_RANGE_MAX_ADDR_LEN];
   int D;
   int d;
 }ReportBindinfo;


typedef struct _ImportParaInfo
{
    char funcode;
    char centeraddr;
}ImportParaInfo;

int Report_AddBindingInfo();
int Report_DelBindingInfo(char port);
int Report_FindBindingInfo(char port,ReportBindinfo* pbindinfo,int eleindex);//根据端口寻找绑定关系

#endif
