/*
 * wifi_config.c
 *
 *  Created on: 2017��6��29��
 *      Author: lmj
 */

#include "msp430common.h"
#include "common.h"
#include "uart3.h"
#include "uart0.h"
#include "store.h"
#include "wifi_config.h"
#include "main.h"
#include "led.h"
#include "string.h"
#include <stdio.h>
#include "debug.h"
#include "Console.h"

#define WIFI_CONFIGED_FLAG             0xA2

#define A_STEP_LEN                     57
#define C_STEP_LEN                     87

#define WIFI_DELAY_TIMEMS               100


#define WIFI_AP_MODE                   "AT+CWMODE=2"

//WIFI����ATָ��
#define WIFI_RST                       "AT+RST"

//����SSID������
#define WIFI_SSID_PASSWORD             "AT+CWSAP=\"%s\",\"%s\",1,3"

//����AP����
#define WIFI_OPEN_LISTENNING           "AT+CIPMUX=1"

//����TCP�˿ڣ��ȴ�����
#define WIFI_OPEN_TCP_PORT             "AT+CIPSERVER=1,8080"

//��ѯ��ǰAP���Ӹ���

#define WIFI_OPEN_CONNECON_COUNT      "AT+CIPSTATUS"
#define WIFI_OPEN_CONNECON_COUNT_LEN  12


int WIFI_Inited_Flag = 0;
int g_wifi_connetflag = 0;
int g_wifi_index = 0;
char g_ssidname[10] = {"RTU5"};
char g_wifipwd[10] ={"12345678"};

extern int trace_open;


char* getParaMemberandLen(hydrologyConfigPara* pconfigPara,int index,int* memlen)
{
  int len[30] = {CENTER_ADDR_LEN,
                 FUNCTION_CODE_LEN,
                 REMOTE_STATION_CLASSIFY_CODE_LEN,
                 REMOTE_STATION_LONGITUDE_LEN,
                 REMOTE_STATION_LATITUDE_LEN,
                 REMOTE_GPRS_SERVER_IP_ADDR_LEN,
                 REMOTE_GRPS_SERVER_PORT_ADDR_LEN,
                 REMOTE_GPRS_SERVER_APN_ADDR_LEN,
                 REMOTE_STATION_NUM_LEN,
                 REMOTE_STATION_ADDR_LEN,
                 CHANNEL_RANGE_MIN_ADDR_LEN,
                 CHANNEL_RANGE_MAX_ADDR_LEN,
                 CHANNEL_CODE_ID_ADDR_LEN,
                 CHANNEL_CODE_GUIDING_LEN,
                 CHANNEL_CODE_DATALEN_LEN,
                 CHANNEL_CODE_DECIMAL_LEN,
                 REMOTE_STATION_ADDR_LEN,
                 CHANNEL_RANGE_MIN_ADDR_LEN,
                 CHANNEL_RANGE_MAX_ADDR_LEN,
                 CHANNEL_CODE_ID_ADDR_LEN,
                 CHANNEL_CODE_GUIDING_LEN,
                 CHANNEL_CODE_DATALEN_LEN,
                 CHANNEL_CODE_DECIMAL_LEN,
                 REMOTE_STATION_ADDR_LEN,
                 CHANNEL_RANGE_MIN_ADDR_LEN,
                 CHANNEL_RANGE_MAX_ADDR_LEN,
                 CHANNEL_CODE_ID_ADDR_LEN,
                 CHANNEL_CODE_GUIDING_LEN,
                 CHANNEL_CODE_DATALEN_LEN,
                 CHANNEL_CODE_DECIMAL_LEN                   
                 };
  
  char* mempoint[30] = {pconfigPara->controladdr,
                        pconfigPara->funcode,
                        pconfigPara->classifycode,
                        pconfigPara->longitude,
                        pconfigPara->latitude,
                        pconfigPara->serverip,
                        pconfigPara->serverport,
                        pconfigPara->serverapn,
                        pconfigPara->addrnum,
                        pconfigPara->configdata[0].remoteaddr,
                        pconfigPara->configdata[0].rangemin,
                        pconfigPara->configdata[0].rangemax,
                        pconfigPara->configdata[0].channelid,
                        pconfigPara->configdata[0].channelguide,
                        pconfigPara->configdata[0].D,
                        pconfigPara->configdata[0].d,
                        pconfigPara->configdata[1].remoteaddr,
                        pconfigPara->configdata[1].rangemin,
                        pconfigPara->configdata[1].rangemax,
                        pconfigPara->configdata[1].channelid,
                        pconfigPara->configdata[1].channelguide,
                        pconfigPara->configdata[1].D,
                        pconfigPara->configdata[1].d,
                        pconfigPara->configdata[2].remoteaddr,
                        pconfigPara->configdata[2].rangemin,
                        pconfigPara->configdata[2].rangemax,
                        pconfigPara->configdata[2].channelid,
                        pconfigPara->configdata[2].channelguide,
                        pconfigPara->configdata[2].D,
                        pconfigPara->configdata[2].d}; 

    if (index >= 30)
    {
        return NULL;
    }

    *memlen = len[index];

    return mempoint[index];

   
}

int WIFI_Write(char* str,int len)
{
    UART3_ClearBuffer();
    UART3_Send(str,len,1);
    return 0;
}

int WIFI_InitCmd()
{
    char datacmd[50] ={0};
    
    WIFI_Write(WIFI_AP_MODE,Utility_Strlen(WIFI_AP_MODE));
    System_Delayms(WIFI_DELAY_TIMEMS*2);

    memset(datacmd,0,50);
    sprintf(datacmd,WIFI_SSID_PASSWORD,g_ssidname,g_wifipwd);
    WIFI_Write(datacmd,Utility_Strlen(datacmd));
    System_Delayms(WIFI_DELAY_TIMEMS*2);

    WIFI_Write(WIFI_RST,Utility_Strlen(WIFI_RST));
    System_Delayms(WIFI_DELAY_TIMEMS*10); 
    
    return XG_SUCCESS;
    
}    

int WIFI_Server_Start()
{

    //�رջ���
    WIFI_Write("ATE0",Utility_Strlen("ATE0"));
    System_Delayms(WIFI_DELAY_TIMEMS);
    
    WIFI_Write(WIFI_OPEN_LISTENNING,Utility_Strlen(WIFI_OPEN_LISTENNING));
    System_Delayms(WIFI_DELAY_TIMEMS*2);

    WIFI_Write(WIFI_OPEN_TCP_PORT,Utility_Strlen(WIFI_OPEN_TCP_PORT));
    System_Delayms(WIFI_DELAY_TIMEMS*2);

    return XG_SUCCESS;

}

int WIFI_ParseData(char* _data,int len,int* startIndex)
{

    char* connectIndex = NULL;
    int wifiindex = 0;

    if(0 == Utility_Strncmp(_data, "+IPD", 4))
    {
        //WIFIģ���յ�������ͨѶ�����й̶�ǰ׺��+IPD,0,2:    0��ʾ�豸��ʶ��2��ʶ�յ����ݳ���
        *startIndex = indexOfColon(_data,len)+1;//ȥ����ͷ����
        
        return 0; 
    }

    if(NULL != strstr(_data,"CONNECT"))
    {
        connectIndex = strtok(_data,",");//��һ�ε���strtok
        if (connectIndex != NULL)
        {
            g_wifi_connetflag = 1;
            TraceMsg("open connect",1);
            g_wifi_index = atoi(connectIndex);

            Console_WriteStringln("connect success,begin to config");
            Console_WriteInt(g_wifi_index);        
            
        }           
        
    }
    
    if(NULL != strstr(_data,"CLOSED"))
    {
        connectIndex = strtok(_data,",");//��һ�ε���strtok
        if (connectIndex != NULL)
        {
            wifiindex = atoi(connectIndex);
            if (wifiindex == g_wifi_index)
            {
               g_wifi_connetflag = 0;//�Ͽ���wifi����

            }
            
        }
        
    }

    if(NULL != strstr(_data,"link is not valid"))
    {
        g_wifi_connetflag = 0;//�Ͽ���wifi����
    }

    return -1;  
}


int WIFI_WaitSendResult()
{
    int  _dataLen=0;
    int  _repeat = 0;
    char _data[UART3_MAXBUFFLEN] = {0};

    while(_repeat < 10)
    {
        while(WIFI_RecvLineTry(_data,UART3_MAXBUFFLEN,&_dataLen) == 0)
        {            
            if(TRUE == Utility_Strncmp(_data, "OK", 2))
            {
                return TRUE;
            }

            if(TRUE == Utility_Strncmp(_data, "ERROR", 5))
            {
                return FALSE;
            }

            
            
            memset(_data,0,UART3_MAXBUFFLEN);
        }
         _repeat ++;
        System_Delayms(WIFI_DELAY_TIMEMS);
    }
    
    return FALSE; //������Ҳ����0
    
}


void WIFI_Open()
{
	//�˴�Flag�ǹ�WIFIģ���Լ����ã������ж�֮ǰ�Ƿ����ù�WIFI��������ù����Ͳ�������
    char wifiFlag;

    //wifi EN�Ÿߵ�ƽ
    P4DIR |= BIT1;
    P4OUT |= BIT1;

    System_Delayms(1000);
    
    TraceMsg("start wifi",1);

    Store_ReadWIFIConfigFlag(&wifiFlag);
    if(WIFI_CONFIGED_FLAG != wifiFlag)
    {
      WIFI_InitCmd();
      TraceMsg("set wifi config end",1);

      //�洢WiFI���ñ�ʶ
      wifiFlag = WIFI_CONFIGED_FLAG;
      Store_SetWIFIConfigFlag(&wifiFlag);
    
    }

    WIFI_Server_Start();

    TraceMsg("set wifi server end",1);

    WIFI_ClearBuffer();
    
    //��������ģ��Wifi�Ѿ��ɹ�����
    WIFI_Inited_Flag = 1;
    trace_open = 0;//wifi����trace������log����

}

int WIFI_SendData(char* _sendData, int dataLength)
{
    int _ret = 0;  
    int _repeat = 0;
    char datacmd[20] ={0};
    
    sprintf(datacmd,"AT+CIPSEND=%d,%d",g_wifi_index,dataLength);

    WIFI_ClearBuffer();//��������֮ǰ����ջ��棬����Ӱ�������յ�"OK"
    
    WIFI_Write(datacmd,Utility_Strlen(datacmd));

    while(_repeat < 3)
    {
        _ret = WIFI_WaitSendResult();
        if(TRUE == _ret)
        {
            WIFI_Write(_sendData, dataLength);
            System_Delayms(WIFI_DELAY_TIMEMS);//��ʱ100ms���ȴ��������
            WIFI_ClearBuffer();
            return TRUE;
        }
        _repeat ++;  
        WIFI_Write(datacmd,Utility_Strlen(datacmd));
        System_Delayms(WIFI_DELAY_TIMEMS);
    }
    return FALSE;
} 

int WIFI_GetCIPStatus()
{
    int _ret;
    
    _ret =  WIFI_SendData(WIFI_OPEN_CONNECON_COUNT,WIFI_OPEN_CONNECON_COUNT_LEN);

    return _ret;
}


//ע�⣬ԭʼ�ĵײ��UART_Receive����ÿ�����ֻ�ܽ���130���ַ����޸ĺ󣬿��Խ���400�ֽ�
int WIFI_ReceiveDataProcess()
{
    int  _dataLen=0;
    int startIdx = 0;
    int _ret= 0;
    char _data[UART3_MAXBUFFLEN] = {0};
    char* pdata = _data;

    while(WIFI_RecvLineTry(pdata,UART3_MAXBUFFLEN,&_dataLen)==0)
    {//������������
        TraceMsg("data:",1);
        TraceMsg(pdata,1);

        if (WIFI_Inited_Flag == 1)
        {
            _ret = WIFI_ParseData(pdata,_dataLen,&startIdx);
        }

        if(_ret == 0)
        {
            _dataLen = _dataLen - startIdx;
            pdata = pdata + startIdx;

            TraceMsg("parse result",1);
            TraceMsg(pdata,1);
            Console_WriteStringln("enter config");
            TraceInt4(_dataLen,1);
            _ret = WIFI_ReceiveConfigDataProcess(pdata,_dataLen);
            if(_ret == 0)
            {
                Console_WriteStringln("config success!!");
                return 0;
            }
            else  //wifi��������ʧ����Ϣ
            {
                Console_WriteStringln("config error!!");
                return -1;
            }
        }
    }

	return -1;
}

void WIFI_Close()
{ 
    //UART3_Close();
    
    //wifi EN�ŵ͵�ƽ
    P4DIR |= BIT1;
    P4OUT &= ~BIT1;
//    System_Delayms(1000);

    WIFI_Inited_Flag = 0;

}

void WIFI_ClearBuffer() //���wifi����
{
    UART3_ClearBuffer();
}

int WIFI_ReceiveConfigDataProcess(char * dest, int recvLength)
{
    //���ĳ���
    char stepA[A_STEP_LEN] = {0};
    char stepC[C_STEP_LEN] = {0};
    int _step=0;//��ǰ��չ�Ĳ���
    int mallocsize = 0;
    int _ret  = 0;

   /*���������鷳���Ȳ��жϳ���*/
   // Utility_Strncpy(lenStr,&dest[3],3);
   //int datagramLen = atoi(lenStr);
    
    if(!('$' == dest[0] && 'S' == dest[1] && '$' == dest[2] && '$' ==  dest[recvLength-1] ))
    {
        Console_WriteErrorStringln("data start id and end id is wrong");
        return -1;
    }

//#if 0
//    if(recvLength != datagramLen) 
//    {
//        Console_WriteStringln("data len is wrong");
//        Console_WriteInt(recvLength);
//        Console_WriteInt(datagramLen);       
//        return -1;
//    }
//#endif

    TraceMsg("handle config data",1);

    //ִ��A����  #A#00000000111,1234,D,0,01,01,11110000,11110000,11110000#
    //        #A#�豸��,����,ϵͳģʽ,����ģʽ,����ģʽ,����ģʽ,ģ��ͨ��ѡ��,����ͨ��ѡ��,IO��ͨ��ѡ��#
    Utility_Strncpy(stepA, &dest[3], A_STEP_LEN);

    _ret = Main_ProcInitConfig(stepA,A_STEP_LEN,&_step);
    if (_ret != 0)
    {
        Console_WriteErrorStringln("A step failed");
        return -1;
    }
    
    //char hydrologyPara[datagramLen-A_STEP_LEN-C_STEP_LEN]={0};//������ʱ�򣬿��ܻ������⣬C�������С����ʹ�ó���������ʹ�ñ���ʽ
    // #B#001,32,4B,1141956,303434,3,0060501111,0060502222,0060503333,27,28,29,9,3,9,3,9,3#
    // ����վ��ַ,������,ң��վ������,ң��վ����(3λ����,2λ��,2λ��, BCD��),ң��վγ��(2λ����,2λ��,2λ��, BCD��),ң��վ��ַ����, ң��վ��ַ1(ǰ6λBCD��,��4λHex��),ң��վ��ַ2,ң��վ��ַ3,ͨ��1����Ҫ��, ͨ��2����Ҫ��,ͨ��3����Ҫ��,Ҫ��1����λ��,Ҫ��1С��λ��,Ҫ��2����λ��,Ҫ��2С��λ��,Ҫ��3����λ��,Ҫ��3С��λ��
    mallocsize = (recvLength - A_STEP_LEN - C_STEP_LEN - 4) * sizeof(char);
    TraceMsg("malloc size:",1);
    TraceInt4(mallocsize,1);
    
    char *hydrologyPara = (char *) malloc(mallocsize);//6�ǰ�ͷ�����ݳ�����ռ�ֽ�,1���ֽڽ���������ȥ7���ֽ�
    if(hydrologyPara == 0)
    {
        Console_WriteErrorStringln("0 memory malloced");
        return -1;
    }
    
    Utility_Strncpy(hydrologyPara, &dest[3 + A_STEP_LEN],mallocsize);

    _ret = Main_ProcInitConfig(hydrologyPara, mallocsize, &_step);
    
    free(hydrologyPara);

    if (_ret != 0)
    {                    
        Console_WriteErrorStringln("B step failed");
        return -1;
    }

    //ִ��C����  #C#00000000,00000000,250,250,250,250,5555,0000000,0000000,0000000,0000000,170509181130#
    //        #C#IO�ڷ�������IO�ڸߵ͵�ƽ,����1Ƶ��, ����2Ƶ��, ����3Ƶ��, ����4Ƶ��,��������(9�ĸ���,1~7),����1����(intֵ) ,����2����,����3����,����4����,ʱ�䴮(��)#
    Utility_Strncpy(stepC, &dest[recvLength - C_STEP_LEN - 1], C_STEP_LEN);//1�ǰ�β

    _ret = Main_ProcInitConfig(stepC,C_STEP_LEN,&_step);
    if (_ret != 0)
    {                    
        Console_WriteErrorStringln("C step failed");
        return -1;
    }
    
    return _ret;
}


int indexOfColon(char * _data, int _dataLen)
{
  for(int i=0;i<_dataLen;i++)
  {
    if(_data[i] == ':')
      return i;
  }
  return -1;
}


int WIFI_GetHydrologyConfigData(char* src,int datalen,hydrologyConfigPara* pconfigPara)
{
    int addrnum = 0;
    int i = 0;
    int memberlen = 0;
    char* delim = ",";//�ָ���
    char* psrc = NULL;
    char* pdata = NULL;
    char* para = NULL;
    

// #if 0
//    if (datalen != sizeof(hydrologyConfigPara) + 4)
//    {
//        Console_WriteErrorStringln("datalen is error ");
//        TraceInt4(datalen, 1);
//        TraceInt4(sizeof(hydrologyConfigPara),1);
//        return FALSE;
//    }
// #endif

    if(!(src[0]=='#' && src[2]=='#' && src[datalen-1]=='#'))
    {
        Console_WriteStringln("param valid 1");
        return FALSE;
    }

    src[datalen-1] = 0;
    psrc = src + 3;
    

    pdata = strtok(psrc,delim);//��һ�ε���strtok
    while(pdata != NULL)//������ֵ��ΪNULLʱ������ѭ��
    {
        TraceMsg(pdata,1);//����ֽ���ַ���

        para = getParaMemberandLen(pconfigPara,i,&memberlen);

        if (para == NULL)
        {
            Console_WriteErrorStringln("get para mem failed!! ");
            return FALSE;
        }

        if ( i == ADDR_NUM_OFFSETCONFIG ) //ң��վ��ַ����
        {
            addrnum = atoi(pdata);
            TraceMsg("rtu addr num:",1);
            TraceInt4(addrnum, 1);
            if (addrnum > REMOTE_STATION_ADDR_MAXNUM)
            {
                Console_WriteErrorStringln("rtu addr num is out of limit! ");
                return FALSE;
            }
        }
        i++; //��ȡ��һ���ṹ���Ա
        
        if(Utility_Strlen(pdata) > memberlen)
        {
            Console_WriteErrorStringln("param len out of limit ");
            Console_WriteInt(Utility_Strlen(pdata));
            Console_WriteInt(memberlen);
            return FALSE;
        }

        Utility_Strncpy(para, pdata, Utility_Strlen(pdata));
        
        pdata =strtok(NULL,delim);//��������strtok���ֽ�ʣ�µ��ַ���
    }
    
    return TRUE;
            
}

int WIFI_HydrologyConfigProcess(char * src, int dataLength, int *_step)
{
    int _ret = 0;
    hydrologyConfigPara configPara;
    
    memset(&configPara,0, sizeof(configPara));

    TraceMsg("WIFI_HydrologyConfigProcess",1);
    TraceStr(src,dataLength,1);
    
    if (FALSE == WIFI_GetHydrologyConfigData(src, dataLength,&configPara))
    {
        Console_WriteErrorStringln("hydrology protocol parameter invalid");
        return -1;
    }

    WIFI_Console_Hydrology(&configPara);

    _ret = Store_SetHydrologyConfigData((char*)&configPara);

    if (_ret != 0)
    {
        Console_WriteErrorStringln("Store_SetHydrologyConfigData error");
        return -1;
        
    }

    if(_step != 0)
    {
        Console_WriteStringln("hydrology protocol parameter successfully configured");
        *_step=2;
    }
    else
    {
        Console_WriteStringln("hydrology protocol parameter successfully configured");
    }
    return 0;
}

int  WIFI_RecvLineTry(char * _dest,const int _max, int * _pNum)
{
    return UART3_RecvLineTry(_dest,_max,_pNum);
}

void WIFI_Console_Hydrology(hydrologyConfigPara* pconfigData)
{

    int i = 0;
    int addrsize = 0;
    
    TraceMsg("controladdr:",1);
    TraceMsg(pconfigData->controladdr,1);

    TraceMsg("funcode:",1);
    TraceMsg(pconfigData->funcode,1);

    TraceMsg("classifycode:",1);
    TraceMsg(pconfigData->classifycode,1);
    
    TraceMsg("longitude:",1);
    TraceMsg(pconfigData->longitude,1);

       
    TraceMsg("latitude:",1);
    TraceMsg(pconfigData->latitude,1);

    TraceMsg("serverip:",1);
    TraceMsg(pconfigData->serverip,1);
    
    TraceMsg("serverport:",1);
    TraceMsg(pconfigData->serverport,1);

    TraceMsg("serverapn:",1);
    TraceMsg(pconfigData->serverapn,1);
    
    TraceMsg("addrnum:",1);
    TraceMsg(pconfigData->addrnum,1);

    addrsize = (pconfigData->addrnum[0]) - '0';
    

    for (i=0; i < addrsize; i++)
    {
        TraceMsg("addr info:",1);
        TraceMsg(pconfigData->configdata[i].remoteaddr,1); 
        TraceMsg(pconfigData->configdata[i].channelid,1); 
        TraceMsg(pconfigData->configdata[i].channelguide,1);
        TraceMsg(pconfigData->configdata[i].D,1); 
        TraceMsg(pconfigData->configdata[i].d,1); 
    }
    
}
