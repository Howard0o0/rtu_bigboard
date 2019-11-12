#include "reportbinding.h"
//#include "hydrology.h"
#include "common.h"
#include "store.h"
#include "wifi_config.h"
#include "console.h"
#include "stdlib.h"

int g_bindnum = 0;

ReportBindinfo g_reportbindinfo[RTU_SAMPLERPORT_MAXNUM]={0};
ImportParaInfo g_importparainfo = {0};

int  Report_CheckPortType(char port, SampleDataPortType* porttype)
{
    if ('A' <= port && port <= 'H')
    {
        *porttype = PORT_Analogy;
        return 0;
    }

    if ('I' <= port && port <= 'L')
    {
        *porttype = PORT_PULSE;
        return 0;
    }

    if ('M' <= port && port <= 'T')
    {
        *porttype = PORT_SWITCH;
        return 0;
    }

    Console_WriteStringln("Report_CheckPortType check error");
    return -1;
    
}

int Report_FindBindingInfo(char port,ReportBindinfo* pbindinfo,int eleindex)//根据端口寻找绑定关系
{
    int _ret = 0;

    if (port == g_reportbindinfo[eleindex].SamplePort)
    {

        _ret = Report_CheckPortType(port,&(g_reportbindinfo[eleindex].porttype));

        if ( 0 != _ret)
        {
            return -1;
        }

        if (port == 'A')
        {
            pbindinfo->volflag = 1;  
        }
        else
        {
            pbindinfo->volflag = 0;
        }

        Utility_Strncpy(pbindinfo->rtuAddr,g_reportbindinfo[eleindex].rtuAddr,RTU_ADDRESS);
        pbindinfo->d = g_reportbindinfo[eleindex].d;
        pbindinfo->D = g_reportbindinfo[eleindex].D;
        pbindinfo->porttype = g_reportbindinfo[eleindex].porttype;
        pbindinfo->SamplePort = g_reportbindinfo[eleindex].SamplePort;
        pbindinfo->element = g_reportbindinfo[eleindex].element;
        Utility_Strncpy(pbindinfo->maxRange, g_reportbindinfo[eleindex].maxRange,CHANNEL_RANGE_MAX_ADDR_LEN);
        Utility_Strncpy(pbindinfo->minRange,g_reportbindinfo[eleindex].minRange,CHANNEL_RANGE_MIN_ADDR_LEN);

        return 0;         
    }

    Console_WriteStringln("Report_FindBindingInfo error");
    return -1;
  
}

int Report_AddBindingInfo()
{
//    hydrologyConfigPara configpara = {0};
//    hydrologyConfigData* pdata = 0;
//    
//    int i = 0;
//    int j = 0;
//    int _ret = 0;
//    
//    Console_WriteStringln("Report_AddBindingInfo");
//
//    _ret = Store_ReadHydrologyConfigData((char*)&configpara);
//    
//    if (_ret != 0)
//    {
//        Console_WriteStringln("Store_ReadHydrologyConfigData error");
//        return -1;
//    }
//
//    WIFI_Console_Hydrology(&configpara);
//
//    g_importparainfo.funcode = ConvertHexChar(configpara.funcode[0]) * CONVERT_HEX + ConvertHexChar(configpara.funcode[1]);
//    Console_WriteStringln("funcode:");
//    Console_WriteHexCharln(&g_importparainfo.funcode, 1);
//    
//    g_importparainfo.centeraddr = ConvertHexChar(configpara.controladdr[0]) * CONVERT_HEX * CONVERT_HEX
//        + ConvertHexChar(configpara.controladdr[1]) * CONVERT_HEX + ConvertHexChar(configpara.controladdr[2]);//必须小于255 
//    Console_WriteStringln("g_centeraddr:");
//    Console_WriteHexCharln(&(g_importparainfo.centeraddr),1);
//    
//    Console_WriteStringln("bindnum:");
//    g_bindnum = configpara.addrnum[0] - '0';
//
//    Console_WriteInt(g_bindnum);
//
//    for (i = 0; i < g_bindnum; i++)
//    {   
//        Console_WriteStringln("addr index=");
//        Console_WriteInt(i);
//        
//        pdata = &(configpara.configdata[i]);
//        
//        for ( j = 0 ; j < RTU_ADDRESS; j++)
//        {
//           g_reportbindinfo[i].rtuAddr[j] = ConvertHexChar(pdata->remoteaddr[j*2]) * CONVERT_HEX + ConvertHexChar(pdata->remoteaddr[j*2+1]);
//        }
//
//        Console_WriteStringln("rtuaddr:");
//        Console_WriteHexCharln(g_reportbindinfo[i].rtuAddr, RTU_ADDRESS);
//        
//        Utility_Strncpy(g_reportbindinfo[i].minRange, pdata->rangemin, CHANNEL_RANGE_MIN_ADDR_LEN);
//        Console_WriteStringln("RangMin");
//        Console_WriteBytesln(g_reportbindinfo[i].minRange,1);
//        
//
//        Utility_Strncpy(g_reportbindinfo[i].maxRange, pdata->rangemax, CHANNEL_RANGE_MAX_ADDR_LEN);
//        Console_WriteStringln("RangMax");
//        Console_WriteBytesln(g_reportbindinfo[i].maxRange,9);
//
//        g_reportbindinfo[i].SamplePort = pdata->channelid[0];
//        Console_WriteStringln("samport:");
//        Console_WriteBytesln(&(g_reportbindinfo[i].SamplePort),1);
//
//        g_reportbindinfo[i].element = ConvertHexChar(pdata->channelguide[0]) * CONVERT_HEX +  ConvertHexChar(pdata->channelguide[1]);
//        Console_WriteStringln("element");
//        Console_WriteHexCharln(&(g_reportbindinfo[i].element), 1);
//
//        g_reportbindinfo[i].D = atoi(pdata->D);
//        Console_WriteStringln("D:");
//        Console_WriteInt(g_reportbindinfo[i].D);
//        
//        g_reportbindinfo[i].d = atoi(pdata->d) ;
//        Console_WriteStringln("d:");
//        Console_WriteInt( g_reportbindinfo[i].d);
//        
//    }

    return 0;    
}

