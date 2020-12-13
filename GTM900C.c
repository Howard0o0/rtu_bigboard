#include "msp430common.h"
#include "uart0.h"
#include "common.h"
#include "GTM900C.h"
#include "string.h"
#include "GSM.h"
#include "Console.h"
#include "common.h"
#include "store.h"
#include "rtc.h"
#include "hydrologycommand.h"
#include "message.h"
#include "uart0.h"

extern unsigned int sys_errorcode_runcode;

enum AtCmdType
{
    SET_APN,
    ACTIVE_GPRS,
    CHECK_SIM_STATUS,
    QUERY_OPERATER,
    QUERY_IP,
    OPEN_TCP_LINK,
    QUERY_TCP_LINK,
    CLOSE_TCP_LINK,
    QUIT_GPRS_NET,
    SETIO_MODE,
    SEND_DATA,
    QUERY_REMAIN_DATA,
    RECEIVE_DATA,
    OFF_CALL
};

enum RETURN_VALUE
{
    OK_MESSAGE_RECV,
    NO_MESSAGE_RECV_ERROR,
    FAIL_MESSAGE_RECV_ERROR,
    GSM_OPEN_ERROR,
    GSM_OPEN_SUCCESS,
    GPRS_NOT_OPEN,
    GPRS_OPEN_ERROR,
    GPRS_OPEN_SUCCESS,
    GPRS_REBOOT_ERROR,
    GPRS_REBOOT_SUCCESS,
    SET_APN_FAILED,
    SET_APN_SUCCESS,
    QUERY_OPERATER_FAILED,
    QUERY_OPERATER_SUCCESS,
    QUERY_IP_FAILED,
    QUERY_IP_SUCCESS,
    ACTIVE_GPRS_FAILED,
    ACTIVE_GPRS_SUCCESS,
    TCP_CONNECT_SUCCESS,
    TCP_CONNECT_FAILED,
    NO_TCP_CONNECT,
    TCP_CONNECT_OK,
    TCP_CLOSE_FAILED,
    TCP_CLOSE_SUCCESS,
    TCP_Link_CLOSE_FAILED,
    TCP_Link_CLOSE_SUCCESS,
    TCP_Link_ALREADY_EXIST,
    TCP_Link_NOT_EXIST,
    TCP_Link_ERROR,
    GPRS_NET_QUIT_FAILED,
    GPRS_NET_QUIT_SUCCESS,
    TCP_QUERY_FAILED,
    TCP_QUERY_SUCCESS,
    SET_IOMODE_FAILED,
    SET_IOMODE_SUCCESS,
    MSG_SEND_FAILED,
    MSG_SEND_SUCCESS,
    QUERY_REMAIN_DATA_FAILED,
    QUERY_REMAIN_DATA_SUCCESS,
    MSG_RECEIVE_FAILED,
    MSG_RECEIVE_SUCCESS,    
    GPRS_WRITE_SUCCESS,
    GPRS_WRITE_FAILED,
    GPRS_CHECKSIM_SUCCESS,
    GPRS_CHECKSIM_FAILED,
    PHONE_CALL_COME,
    NEW_SMS_COME,
    GPRS_NO_DATA_RECV,
    GPRS_DATA_RECV
};

typedef enum tagTelecomOperater
{
//    ERROR_OPERATER,
    CHINA_MOBILE,
    CHINA_UNICOM,
    CHINA_TELECOM
}Operater;



char centerIP[16];// = "125.220.159.168";
char centerPort[7];// = "6666";
char *psrc = NULL;
int dataLen = 0;

char* _ReceiveData = NULL;
int _ReceiveDataLen = 0;

void GPRS_Print_Error(int errorCode)
{
    switch(errorCode)
    {
        // 1:输入参数错误
        case 1:
        {
            Console_WriteStringln("Wrong AT Command Parameter!");
            break;
        }
        // 2:一个内部的错误产生
        case 2:
        {
            Console_WriteStringln("An internel error in GTM900 Module!");
            
            //通过Timer0_A0中断显示通信模块内部故障 LSHB 20200506
            sys_errorcode_runcode=32;
            Led_OffAll();
            led_flash_init(45);
            Timer0_A0_Enable(); //Timer0_A0开中断
            System_Delayms(3000);
            Timer0_A0_Clear();//Timer0_A0关中断
            Led_OffAll();
            Led1_WorkLightOff();//工作指示灯关,节能
           // end  LSHB LSHB 20200506 
            
            break;
        }
        // 3:IP 地址或者端口号正在被使用
        case 3:
        {
            Console_WriteStringln("IP Address or Port is already used!");
            break;
        }
        // 4:内存申请失败或者 BUFFER 已经被使用完
        case 4:
        {
            Console_WriteStringln("Memory apply failed or buffer run out!");
            break;
        }
        // 5:该 socket 不支持此种操作
        case 5:
        {
            Console_WriteStringln("Unsupported operation of socket!");
            break;
        }
        // 6:不能链接到该主机
        case 6:
        {
            Console_WriteStringln("Connot connect to host, please check host!");
            
         //通过Timer0_A0中断显示通信模块不能链接到该主机 LSHB 20200506
            sys_errorcode_runcode=33;
            Led_OffAll();
            led_flash_init(45);
            Timer0_A0_Enable(); //Timer0_A0开中断
            System_Delayms(1000);
            Timer0_A0_Clear();//Timer0_A0关中断
            Led_OffAll();
            Led1_WorkLightOff();//工作指示灯关,节能
           // end  LSHB 20200506  
            
            break;
        }
        // 7:链接被对方拒绝
        case 7:
        {
            Console_WriteStringln("Connection being refused by host, please check host fireware");
            
            //通过Timer0_A0中断显示通信模块被对方拒绝 LSHB 20200506
            sys_errorcode_runcode=33;
            Led_OffAll();
            led_flash_init(45);
            Timer0_A0_Enable(); //Timer0_A0开中断
            System_Delayms(1000);
            Timer0_A0_Clear();//Timer0_A0关中断
            Led_OffAll();
            Led1_WorkLightOff();//工作指示灯关,节能
           // end  LSHB 20200506      
            
            break;
        }
        // 8:链接超时
        case 8:
        {
            Console_WriteStringln("Connection time out");
            
           //通过Timer0_A0中断显示通信模块链接超时 LSHB 20200506
            sys_errorcode_runcode=34;
            Led_OffAll();
            led_flash_init(45);
            Timer0_A0_Enable(); //Timer0_A0开中断
            System_Delayms(3000);
            Timer0_A0_Clear();//Timer0_A0关中断
            Led_OffAll();
             Led1_WorkLightOff();//工作指示灯关,节能
           // end  LSHB 20200506             
            
            break;
        }
        // 9:该链接已经建立
        case 9:
        {
            Console_WriteStringln("Link already exists");
            break;
        }
        // 10:在 DNSR 服务器中找不到该主机地址
        case 10:
        {
            Console_WriteStringln("Can not find host IP in DNSR");
            break;
        }
        // 11:出现一个暂时的 DNS 错误，稍后可以重新执行
        case 11:
        {
            Console_WriteStringln("Temporary DSN Error");
            break;
        }
        // 12:出现一个永久的 DNS 错误
        case 12:
        {
            Console_WriteStringln("Permanent DSN Error");
            break;
        }
        // 13:在 DNSR 服务器找到该主机地址，但是 IP 地址无效
        case 13:
        {
            Console_WriteStringln("Invalid IP in DNSR");
            break;
        }
        // 14:该 socket 链接还没有建立
        case 14:
        {
            Console_WriteStringln("Socket not exist");
            break;
        }
        // 15:传输的数据长度太大
        case 15:
        {
            Console_WriteStringln("Data out of length");
            break;
        }
        // 16:该链接被对端复位
        case 16:
        {
            Console_WriteStringln("Host reset Link");
            break;
        }
        // 17:该链接有错误产生
        case 17:
        {
            Console_WriteStringln("Error occurs on Link");
            break;
        }
        // 18:没有剩余缓存来保存信息
        case 18:
        {
            Console_WriteStringln("No Cache to save information");
            break;
        }
        // 19:没有网络，或者一个错误产生导致 TCPIP 断开
        case 19:
        {
            Console_WriteStringln("No net exists or TCP/IP function failed");
            
        //通过Timer0_A0中断显示通信模块链接超时 LSHB 20200506
            sys_errorcode_runcode=35;
            Led_OffAll();
            led_flash_init(45);
            Timer0_A0_Enable(); //Timer0_A0开中断
            System_Delayms(3000);
            Timer0_A0_Clear();//Timer0_A0关中断
            Led_OffAll();
             Led1_WorkLightOff();//工作指示灯关,节能
           // end  LSHB 20200506            
            
            
            break;
        }
        // 20:PDP 上下文没有激活
        case 20:
        {
            Console_WriteStringln("PDP not activitied");
            break;
        }  
        // 21:GPRS attach 没有成功
        case 21:
        {
            Console_WriteStringln("GPRS attach failed");
            
           //通过Timer0_A0中断显示通信模块链接超时 LSHB 20200506
            sys_errorcode_runcode=36;
            Led_OffAll();
            led_flash_init(45);
            Timer0_A0_Enable(); //Timer0_A0开中断
            System_Delayms(3000);
            Timer0_A0_Clear();//Timer0_A0关中断
            Led_OffAll();
            Led1_WorkLightOff();//工作指示灯关,节能
           // end  LSHB 20200506     
            
            break;
        }
        // 22:类似的操作正在执行
        case 22:
        {
            Console_WriteStringln("Similar operation already being excuted");
            break;
        }
        // 23:PDP 上下文已经激活
        case 23:
        {
            Console_WriteStringln("PDP already activitied");
            break;
        }
        // 24:当前为多链接模式
        case 24:
        {
            Console_WriteStringln("Tcp is on multi-link mode!");
            break;
        }
        // 25:当前为单链接模式
        case 25:
        {
            Console_WriteStringln("Tcp is on single-link mode!");
            break;
        }
        // 26:输入的 index 号码不存在
        case 26:
        {
            Console_WriteStringln("Index number not exist !");
            break;
        }
        // 27:无效的删除模式
        case 27:
        {
            Console_WriteStringln("Invalid delete mode!");
            break;
        }
        // 28:当前不可以使用该命令
        case 28:
        {
            Console_WriteStringln("Can not use this command at present!");
            break;
        }
        // 29:发送窗口已满
        case 29:
        {
            Console_WriteStringln("Sending window full!");
            break;
        }
        // 30:当前正在通话中，请稍后执行 PDP 激活和 TCPIP 初始化的操作
        case 30:
        {
            Console_WriteStringln("Phone on line, please excuted PDP activite and TCP Initialize later");
            break;
        }
        // 31:其它错误
        case 31:
        {
            Console_WriteStringln("Unexpected error");
            break;
        }
    }
}

int GPRS_AT_SetAPN_Response(char *_recv)
{
    return TRUE;
}

int GPRS_AT_ActiveGPRSNet_Response(char *_recv)
{
    return TRUE;
}


int GPRS_AT_CheckSimStat_Response(char *_recv,int *_retErrorCode)
{
    int retValue = TRUE;
    if(Utility_Strncmp(_recv, "%TSIM", 5) == 0)
    {
        if(_recv[6] == '0')
        {
            retValue = FALSE;
        }
    }
    else if(Utility_Strncmp(_recv,"ERROR",5)==0)//ERROR: 14  冒号和14中间有空格
    {
        char bufErrorTmp[3] = {0};//调用atoi函数，需要有字符串结束符，2位错误码，1位结束符

        bufErrorTmp[0] = _recv[7];	
        if(_recv[8] != 0)
        {
            bufErrorTmp[1] = _recv[8];
        }
        *_retErrorCode = atoi(bufErrorTmp);	
        GPRS_Print_Error(*_retErrorCode);
        retValue = FALSE;
    }
    return retValue;    
}

Operater TelecomOperater;

int GPRS_AT_QueryOperater_Response(char *_recv,int *_retErrorCode)
{
    int retValue = TRUE;
    if(Utility_Strncmp(_recv, "+COPS: 0,0,\"CHINA  MOBILE\"", 26) == 0)
    {
        TelecomOperater = CHINA_MOBILE;
        retValue = TRUE;
    }
    else if(Utility_Strncmp(_recv,"ERROR",5)==0)//ERROR: 14  冒号和14中间有空格
    {
        char bufErrorTmp[3] = {0};//调用atoi函数，需要有字符串结束符，2位错误码，1位结束符

        bufErrorTmp[0] = _recv[7];	
        if(_recv[8] != 0)
        {
            bufErrorTmp[1] = _recv[8];
        }
        *_retErrorCode = atoi(bufErrorTmp);	
        GPRS_Print_Error(*_retErrorCode);
        retValue = FALSE;
    }
    return retValue;
}

int GPRS_AT_QueryIP_Response(char *_recv,int *_retErrorCode)
{
    int retValue = TRUE;
    if(Utility_Strncmp(_recv, "%ETCPIP:", 8) == 0)
    {
        if(_recv[8] == 0)
        {
            retValue = FALSE;
        }	
    }else if(Utility_Strncmp(_recv,"ERROR",5)==0)//ERROR: 14  冒号和14中间有空格
    {
        char bufErrorTmp[3] = {0};//调用atoi函数，需要有字符串结束符，2位错误码，1位结束符

        bufErrorTmp[0] = _recv[7];	
        if(_recv[8] != 0)
        {
            bufErrorTmp[1] = _recv[8];
        }
        *_retErrorCode = atoi(bufErrorTmp);	
        GPRS_Print_Error(*_retErrorCode);
        retValue = FALSE;
    }
    return retValue;
}

//打开链接和其它命令响应有一定不一致，因为只返回一个connect或者error
int GPRS_AT_OpenTCPLink_Response(char *_recv)
{
    return TRUE;
}

int GPRS_AT_QueryTCPLink_Response(char *_recv,int *_retErrorCode)
{
    int retValue = TRUE;
    if(Utility_Strncmp(_recv, "%IPOPEN:", 8) == 0)
    {
        if(_recv[8] == 0)
        {
            retValue = FALSE;
        }			        
    }
    else if(Utility_Strncmp(_recv,"ERROR",5)==0)//ERROR: 14  冒号和14中间有空格
    {
        char bufErrorTmp[3] = {0};//调用atoi函数，需要有字符串结束符，2位错误码，1位结束符

        bufErrorTmp[0] = _recv[7];	
        if(_recv[8] != 0)
        {
            bufErrorTmp[1] = _recv[8];
        }
        *_retErrorCode = atoi(bufErrorTmp);	
        GPRS_Print_Error(*_retErrorCode);
        retValue = FALSE;
    }
    return retValue;
}

//这里没考虑出错，后面要处理
int GPRS_AT_CloseTCPLink_Response(char *_recv,int *_retErrorCode)
{
    int retValue = FALSE;
    //IPCLOSE如果有IP链接，会先返回%IPCLOSE,然后再返回OK,如果没有链接，只会返回一个命令响应成功 OK
    if((Utility_Strncmp(_recv,"%IPCLOSE",8) ==0) || (Utility_Strncmp(_recv,"OK",2) ==0))
    {
        retValue =  TRUE;
    }
    else if(Utility_Strncmp(_recv,"ERROR",5)==0)//ERROR: 14  冒号和14中间有空格
    {
        char bufErrorTmp[3] = {0};//调用atoi函数，需要有字符串结束符，2位错误码，1位结束符

        bufErrorTmp[0] = _recv[7];	
        if(_recv[8] != 0)
        {
            bufErrorTmp[1] = _recv[8];
        }
        *_retErrorCode = atoi(bufErrorTmp);	
        GPRS_Print_Error(*_retErrorCode);
        retValue = FALSE;
    }
    return retValue;
}

//这里没考虑出错，后面要处理
int GPRS_AT_QuitGPRSNet_Response(char *_recv,int *_retErrorCode)
{
    int retValue = FALSE;
    //IPCLOSE如果有IP链接，会先返回%IPCLOSE,然后再返回OK,如果没有链接，只会返回一个命令响应成功 OK
    if((Utility_Strncmp(_recv,"%IPCLOSE",8) ==0) || (Utility_Strncmp(_recv,"OK",2) ==0))
    {
        retValue =  TRUE;
    }
    else if(Utility_Strncmp(_recv,"ERROR",5)==0)//ERROR: 14  冒号和14中间有空格
    {
        char bufErrorTmp[3] = {0};//调用atoi函数，需要有字符串结束符，2位错误码，1位结束符

        bufErrorTmp[0] = _recv[7];	
        if(_recv[8] != 0)
        {
            bufErrorTmp[1] = _recv[8];
        }
        *_retErrorCode = atoi(bufErrorTmp);	
        GPRS_Print_Error(*_retErrorCode);
        retValue = FALSE;
    }
    return retValue;
}

//使用ASCii码还是Hex码发送消息，0,1,1---ASCii， 1,1,0---Hex
int GPRS_AT_SetIOMode_Response(char *_recv)
{
    int retValue = TRUE;
    //SetIOMode如果有IP链接，会先返回OK,没有IP，就会返回 EXT: I
    if(Utility_Strncmp(_recv, "EXT:", 4) == 0)
    {
        retValue = FALSE;	
    }
    return retValue;
}

int _GTM900SendBufLeft = 16;
int GRPS_AT_Send_Response(char *_recv,int *_retErrorCode)
{
    int retValue = FALSE;
    if(Utility_Strncmp(_recv, "%IPSEND:", 8) == 0)
    {
        char bufLeftTmp[3] = {0};//调用atoi函数，需要有字符串结束符
        bufLeftTmp[0] = _recv[8];
        if(_recv[9] != 0)
        {
            bufLeftTmp[1] = _recv[9];
        }
        _GTM900SendBufLeft = atoi(bufLeftTmp);
        retValue = TRUE;
    }
    else if(Utility_Strncmp(_recv,"ERROR",5)==0)//ERROR: 14  冒号和14中间有空格
    {
        char bufErrorTmp[3] = {0};//调用atoi函数，需要有字符串结束符，2位错误码，1位结束符

        bufErrorTmp[0] = _recv[7];	
        if(_recv[8] != 0)
        {
            bufErrorTmp[1] = _recv[8];
        }
        *_retErrorCode = atoi(bufErrorTmp);	
        GPRS_Print_Error(*_retErrorCode);
        retValue = FALSE;
    }

    return retValue;
}

int RemainingData = 0;
int GPRS_AT_QueryRemainData_Response(char *_recv,int *_retErrorCode)
{
    int retValue = TRUE;
    
    if(Utility_Strncmp(_recv, "%IPDQ:", 6) == 0)
    {
        RemainingData = atoi(&_recv[7]);
    }
    else if(Utility_Strncmp(_recv,"ERROR",5)==0)//ERROR: 14  冒号和14中间有空格
    {
        char bufErrorTmp[3] = {0};//调用atoi函数，需要有字符串结束符，2位错误码，1位结束符

        bufErrorTmp[0] = _recv[7];	
        if(_recv[8] != 0)
        {
            bufErrorTmp[1] = _recv[8];
        }
        *_retErrorCode = atoi(bufErrorTmp);	
        GPRS_Print_Error(*_retErrorCode);
        retValue = FALSE;
    }
    return retValue;
}

int GRPS_AT_Receive_Response(char *_recv,int *_retErrorCode)
{
    int retValue = FALSE;
    char* pdata = NULL;
    char* psrc = _recv + 10;
    int _datalensize = 0;
    
    if(Utility_Strncmp(_recv, "%IPDR:", 6) == 0)
    {
        while((*psrc) != ',')
        {
            _datalensize++;
            psrc++;
        }
        psrc = _recv + 10;
        pdata = (char*)malloc(_datalensize);
        Utility_Strncpy(pdata, _recv + 10, _datalensize);
        _ReceiveDataLen = Utility_atoi(pdata, _datalensize);
        free(pdata);
        pdata = NULL;//
        _ReceiveData = (char*)malloc(_ReceiveDataLen*2);
        if(_ReceiveData == NULL)
        {
            Console_WriteStringln("Malloc in GRPS_AT_Receive_Response failed");
            return MSG_SEND_FAILED;
        }
        
        while((*psrc) != '\"')
        {
            psrc++;
        }
        Utility_Strncpy(_ReceiveData, psrc+1, _ReceiveDataLen*2);
        //free(_ReceiveData);  //+++
        //_ReceiveData = NULL;
        retValue = TRUE;
    }
    else if(Utility_Strncmp(_recv,"ERROR",5)==0)//ERROR: 14  冒号和14中间有空格
    {
        char bufErrorTmp[3] = {0};//调用atoi函数，需要有字符串结束符，2位错误码，1位结束符

        bufErrorTmp[0] = _recv[7];	
        if(_recv[8] != 0)
        {
            bufErrorTmp[1] = _recv[8];
        }
        *_retErrorCode = atoi(bufErrorTmp);	
        GPRS_Print_Error(*_retErrorCode);
        retValue = FALSE;
    }

    return retValue;
}

int GPRS_AT_OFF_CALL_Response(char *_recv)
{
    return TRUE;
}

int GPRS_SubProc_AT_ResponseOne(char *_recv, int *_retMsgNum, int cmdType, int *_retErrorCode)
{
    int retValue=FALSE;

    if(Utility_Strncmp(_recv,"RING",4)==0)//如果是电话来了，直接挂掉，重新等待接受。
    {
        GPRS_AT_OFF_CALL();
        retValue = FALSE;//出现电话
        return retValue;
    }
    
    switch(cmdType)
    {
        case SET_APN:
        {
            retValue = GPRS_AT_SetAPN_Response(_recv);
            break;
        }
        case ACTIVE_GPRS:
        {
            retValue = GPRS_AT_ActiveGPRSNet_Response(_recv);
            break;
        }
        case CHECK_SIM_STATUS:
        {
            retValue = GPRS_AT_CheckSimStat_Response(_recv, _retErrorCode);
            break;
        }
        case QUERY_OPERATER:
        {
            retValue = GPRS_AT_QueryOperater_Response(_recv, _retErrorCode);
            break;
        }
        case QUERY_IP:
        {
            retValue = GPRS_AT_QueryIP_Response(_recv, _retErrorCode);//如果第一步的"%ETCPIP:0"标识值为0，直接返回，后面不用进行了
            break;
        }
        case OPEN_TCP_LINK:
        {
            retValue = GPRS_AT_OpenTCPLink_Response(_recv);
            break;
        }
        case QUERY_TCP_LINK:
        {
            retValue = GPRS_AT_QueryTCPLink_Response(_recv, _retErrorCode);//如果第一步的"%ETCPIP:0"标识值为0，直接返回，后面不用进行了
            break;
        }
        case CLOSE_TCP_LINK:
        {
            retValue = GPRS_AT_CloseTCPLink_Response(_recv, _retErrorCode);
            break;
        }
        case QUIT_GPRS_NET:
        {
            retValue = GPRS_AT_QuitGPRSNet_Response(_recv, _retErrorCode);
            break;
        }
        case SETIO_MODE:
        {
            retValue = GPRS_AT_SetIOMode_Response(_recv);
            break;
        }
        case SEND_DATA:
        {
            retValue = GRPS_AT_Send_Response(_recv, _retErrorCode);
            break;
        }
        case QUERY_REMAIN_DATA:
        {
            retValue = GPRS_AT_QueryRemainData_Response(_recv, _retErrorCode);
            break;
        }
        case RECEIVE_DATA:
        {
            retValue = GRPS_AT_Receive_Response(_recv, _retErrorCode);
            break;
        }
        case OFF_CALL:
        {
            retValue = GPRS_AT_OFF_CALL_Response(_recv);
            break;
        }
        default:
        {
            retValue = TRUE;
            break;
        }
    }
    return retValue;
}


//只要不是ERROR或者OK,其它响应执行完了，还要继续处理。这里有问题，明天继续处理!!!!!!!!!!! 17-08-21
int GPRS_SubProc_AT_ResponseTwo(char *_recv, int *_retErrorCode)
{
    int retValue = FALSE;
    if(Utility_Strncmp(_recv,"OK",2)==0)
    {
        retValue = TRUE;
    }
    else if(Utility_Strncmp(_recv,"CONNECT",7) ==0 )
    {
        retValue = TRUE;
    }else if(Utility_Strncmp(_recv,"ERROR",5)==0)//ERROR: 14  冒号和14中间有空格
    {
        char bufErrorTmp[3] = {0};//调用atoi函数，需要有字符串结束符，2位错误码，1位结束符

        bufErrorTmp[0] = _recv[7];	
        if(_recv[8] != 0)
        {
            bufErrorTmp[1] = _recv[8];
        }
        *_retErrorCode = atoi(bufErrorTmp);	
        GPRS_Print_Error(*_retErrorCode);
        retValue = FALSE;
    }
    else
    {
        retValue = FALSE;
    }
    return retValue;
}

int GPRS_Proc_AT_Response(char *_recv, int *_retMsgNum, int cmdType, int  *errorCode)
{
    int _recvTryTimes1 = 0;
    int _retValue = FALSE;
    int _okFlag;//设定用来判断需不需要收2行响应

    //只返回一个OK的，okFlag初始为TRUE,返回2行的，初始为FALSE
    switch(cmdType)
    {
        case SET_APN:
        {
            _okFlag = TRUE;
            break;
        }
        case ACTIVE_GPRS:
        {
            _okFlag = TRUE;
            break;
        }
        case CHECK_SIM_STATUS:
        {  
            _okFlag = FALSE;
            break;
        }
        case QUERY_OPERATER:
        {  
            _okFlag = FALSE;
            break;
        }
        case QUERY_IP:
        {  
            _okFlag = FALSE;
            break;
        }
        case OPEN_TCP_LINK:
        {
            _okFlag = TRUE;
            break;
        }
        case QUERY_TCP_LINK:
        {
            _okFlag = FALSE;
            break;
        }
        case CLOSE_TCP_LINK:
        {
            _okFlag = FALSE;
            break;
        }
        case QUIT_GPRS_NET:
        {
            _okFlag = FALSE;
            break;
        }
        case SETIO_MODE:
        {
            _okFlag = TRUE;
            break;
        }
        case SEND_DATA:
        {  
            _okFlag = FALSE;
            break;
        }
        case QUERY_REMAIN_DATA:
        {  
            _okFlag = FALSE;
            break;
        }
        case RECEIVE_DATA:
        {  
            _okFlag = FALSE;
            break;
        }
        case OFF_CALL:
        {
            _okFlag = TRUE;
            break;
        }
        default:
        {
            _okFlag = TRUE;
            break;
        }
    } 

    
    while(_recvTryTimes1 < 100)
    {
        if(UART0_RecvLineTry(_recv,UART0_MAXBUFFLEN,_retMsgNum)==-1)
        {
            _recvTryTimes1 += 1;
            if(cmdType == ACTIVE_GPRS)
                System_Delayms(700);//原本打算最大延时30s，后来发现，如果发网络指令，刚好电话在前面进来，模块会卡住，也不会有ring进来，直到拨号结束，联通是45s自动挂断，移动是65s自动挂断
            else if(cmdType == QUERY_REMAIN_DATA)
                System_Delayms(100);
            else 
                System_Delayms(500);//只有激活GPRS会卡长时间，其它命令卡顿时间不超过30s
            continue;
        }
        else
        {
            if(_okFlag == FALSE)//如果返回的第一行不是OK,则需要处理第一行；否则直接处理第二行
            {
                int respRet ;
                respRet = GPRS_SubProc_AT_ResponseOne(_recv,_retMsgNum,cmdType, errorCode);
                if(respRet == TRUE)//如果第一步处理有返回结果，就继续处理，不正确也会继续收到一个OK,只要第一步不出现错误标志，收到消息就往下一步走，由下一步处理
                {
                    _okFlag = TRUE;
                }
                else//如果第一步返回结果不为TRUE，后面不需要处理了。
                {
                    _retValue = FALSE;
                    break;
                }
            }
            else
            {
                _retValue = GPRS_SubProc_AT_ResponseTwo(_recv, errorCode);
                if(_retValue == TRUE){
                    break;
                }
                else
                {
                    if(errorCode!=0)//只有出现错误才退出，否则不返回
                        break;
                }
            }
        }
    }
    return _retValue;
}

int GPRS_WriteBytes(char* psrc, int len)
{
    UART0_Send(psrc, len ,1);
    return 0;
}

int GPRS_AT_SetAPN()
{
    int errorCode = 0;
    int _retMsgNum = 0;
    int _cmdLen = 0;
    int _retValue = FALSE;
    char _send[30] = "at+cgdcont=1,\"ip\",\"";
    char _recv[30] = {0};
    
    _cmdLen += 19;
    
    if(TelecomOperater == CHINA_MOBILE)
    {
        Utility_Strncpy(&_send[_cmdLen], "cmnet", Utility_Strlen("cmnet"));
    }
    else
        return SET_APN_FAILED;
    
    _cmdLen += Utility_Strlen("cmnet");
    _send[_cmdLen] = '\"';
    _cmdLen += 1;
  
  //该AT指令会返回:
  //OK
  //注意，需要先关闭回显，以免造成麻烦
    GPRS_PrepareForSend();
    GPRS_WriteBytes(_send,_cmdLen);
    _retValue = GPRS_Proc_AT_Response(_recv, &_retMsgNum, SET_APN, &errorCode);

    if(_retValue==TRUE)
    {
        TraceMsg("Set APN Success",1);
        return SET_APN_SUCCESS;
    }
    else
    {
        Console_WriteStringln("Set APN Failed");
    }

    return SET_APN_FAILED;
}

int GPRS_AT_ActiveGPRSNet()
{
    int errorCode=0;
    int _retMsgNum = 0;//
    int _cmdLen = 0;
    int _retValue = 0;
    char _send[10] = "at%etcpip";
    char _recv[30] = { 0 };
    _cmdLen += 9;


    //该AT指令会返回:
    //OK
    //注意，需要先关闭回显，以免造成麻烦
    GPRS_PrepareForSend();
    GPRS_WriteBytes(_send,_cmdLen);
    _retValue = GPRS_Proc_AT_Response(_recv, &_retMsgNum, ACTIVE_GPRS, &errorCode);

    if(_retValue == TRUE)
    {
        TraceMsg("Active GPRS success",1);
        return ACTIVE_GPRS_SUCCESS;
    }
    else
    {
        Console_WriteStringln("Active GPRS failed");
    }
    	
    return ACTIVE_GPRS_FAILED;
}


int GPRS_AT_CheckSimStat()
{
    int * errorCode = NULL;
    int _retMsgNum = 0;//
    int _cmdLen = 0;
    int _retValue = 0;
    char _send[10] = "AT%TSIM";
    char _recv[30] = { 0 };
    _cmdLen += 7;


    //该AT指令会返回:
    //OK
    //注意，需要先关闭回显，以免造成麻烦
    GPRS_PrepareForSend();
    GPRS_WriteBytes(_send,_cmdLen);
    _retValue = GPRS_Proc_AT_Response(_recv, &_retMsgNum, CHECK_SIM_STATUS, errorCode);

    if(_retValue == TRUE)
    {
        TraceMsg("CheckSimStat Success",1);
        return GPRS_CHECKSIM_SUCCESS;
    }else
    {
        Console_WriteStringln("CheckSimStat failed");
        
      //通过Timer0_A0中断显示通信模块被对方拒绝 LSHB 20200506
            sys_errorcode_runcode=37;
            Led_OffAll();
            led_flash_init(45);
            Timer0_A0_Enable(); //Timer0_A0开中断
            System_Delayms(5000);
            Timer0_A0_Clear();//Timer0_A0关中断
            Led_OffAll();
            Led1_WorkLightOff();//工作指示灯关,节能
           // end  LSHB 20200506             
        
    }

    return GPRS_CHECKSIM_FAILED;
}

int GPRS_AT_QueryOperater()
{
    int errorCode=0;
    int _retMsgNum = 0;//
    int _cmdLen = 0;
    int _retValue = 0;
    char _send[8] = "AT+COPS?";
    char _recv[100] = {0};
    _cmdLen += 8;

    //该AT指令会返回:
    //OK
    //注意，需要先关闭回显，以免造成麻烦
    GPRS_PrepareForSend();
    GPRS_WriteBytes(_send,_cmdLen);
    _retValue = GPRS_Proc_AT_Response(_recv, &_retMsgNum, QUERY_OPERATER, &errorCode);

    if(_retValue == TRUE)
    {
        if(TelecomOperater == CHINA_MOBILE)
        {
            TraceMsg("Operater is China Mobile",1);
            return QUERY_OPERATER_SUCCESS;
        }
    }
    else
    {
        Console_WriteStringln("QUERY Operater Failed");
    }


    return QUERY_OPERATER_FAILED;
}

int GPRS_AT_QueryIP()
{
    int errorCode=0;
    int _retMsgNum = 0;//
    int _cmdLen = 0;
    int _retValue = 0;
    char _send[10] = "at%etcpip?";
    char _recv[100] = {0};
    _cmdLen += 10;

    //该AT指令会返回:
    //OK
    //注意，需要先关闭回显，以免造成麻烦
    GPRS_PrepareForSend();
    GPRS_WriteBytes(_send,_cmdLen);
    _retValue = GPRS_Proc_AT_Response(_recv, &_retMsgNum, QUERY_IP, &errorCode);
    // 查询指令会先返回一行%ETCPIP:1,"10.101.53.141",,"114.114.114.114","211.138.156.146", 
    //注意，只返回本地IP还不行，还要有网关和DNS，否则后面打开连链接会出问题
    // 再返回一行 OK

    if(_retValue == TRUE)
    {
        TraceMsg("QUERY IP Success",1);
        return QUERY_IP_SUCCESS;
    }
    else
    {
        Console_WriteStringln("QUERY IP Failed");
    }


    return QUERY_IP_FAILED;
}

void judgeintlen(int *ip,char *iplen,char len)
{
    char i;
    for(i = 0;i < len;i++)
    {
        if(ip[i] >= 100)
            iplen[i] = 3;
        else if(ip[i] >= 10)
            iplen[i] = 2;
        else
            iplen[i] = 1;
    }
}

char judgeportlen(char *port)
{
    char portlen[3] = {0,0,0};
    char i;
    
    for(i = 0;i < 3;i++)
    {
        if(port[i] >= 10)
            portlen[i] = 2;
        else if(port[i] > 0)
            portlen[i] = 1;
        else
            portlen[i] = 0;
    }
    return portlen[0]+portlen[1]+portlen[2];
}

int GPRS_AT_OpenTCPLink(int center)
{
    int errorCode=0;
    int _retMsgNum = 0;//
    int _cmdLen = 0;
    int _retValue = 0;
    int IPStrLen = 0;
    int PortStrLen = 0;
    char _recv[30] = {0};
    char centerip[6] = {0,0,0,0,0,0};
    int ip[4] = {0,0,0,0};
    char iplen[4] = {0,0,0,0};
    char i,j;
    char centerport[3] = {0,0,0};
    char centerportlen = 0;
    
    char ipvalue[10];

    char _send[50] = "AT%IPOPEN=\"TCP\",\""; 	
    _cmdLen += 17;
    
    memset(ipvalue,0,sizeof(ipvalue));
    Hydrology_ReadStoreInfo(center,ipvalue,HYDROLOGY_CENTER_IP_LEN);
    memcpy(centerip,&ipvalue[1],6);
    memcpy(centerport,&ipvalue[7],3);
    
//    Store_ReadHydrologyServerIP(centerip,center);
    centerip[0] = _BCDtoHEX(centerip[0]);
    centerip[1] = _BCDtoHEX(centerip[1]);
    centerip[2] = _BCDtoHEX(centerip[2]);
    centerip[3] = _BCDtoHEX(centerip[3]);
    centerip[4] = _BCDtoHEX(centerip[4]);
    centerip[5] = _BCDtoHEX(centerip[5]);
    
    ip[0] = centerip[0]*10 + centerip[1]/10;
    ip[1] = (centerip[1]%10)*100 + centerip[2];
    ip[2] = centerip[3]*10 + centerip[4]/10;
    ip[3] = (centerip[4]%10)*100 + centerip[5];
    
    judgeintlen(ip,iplen,4);
    
    for(i = 0,j = 0;i < 4;i++)
    {
        if(iplen[i] == 3)
        {
            centerIP[j] = ip[i]/100+'0';
            centerIP[j+1] = ((ip[i]/10)%10)+'0';
            centerIP[j+2] = ip[i]%10+'0';
            centerIP[j+3] = '.';
            j += 4;
        }
        else if(iplen[i] == 2)
        {
            centerIP[j] = ip[i]/10+'0';
            centerIP[j+1] = ip[i]%10+'0';
            centerIP[j+2] = '.';
            j += 3;
        }
        else
        {
            centerIP[j] = ip[i]+'0';
            centerIP[j+1] = '.';
            j += 2;
        }
    }
    centerIP[j-1] = '\0';
    TraceMsg(centerIP,1);
    
    IPStrLen = Utility_Strlen(centerIP);
    Utility_Strncpy(&_send[_cmdLen], centerIP, IPStrLen);
    _cmdLen += IPStrLen;

    _send[_cmdLen] = '\"';
    _cmdLen += 1;

    _send[_cmdLen] = ',';
    _cmdLen += 1;
    
//    Store_ReadHydrologyServerPort(centerport,center);
    centerport[0] = _BCDtoHEX(centerport[0]);
    centerport[1] = _BCDtoHEX(centerport[1]);
    centerport[2] = _BCDtoHEX(centerport[2]);
    
    centerportlen = judgeportlen(centerport);
    
    switch(centerportlen)
    {
        case 6:
        {
            centerPort[0] = centerport[0]/10+'0';
            centerPort[1] = centerport[0]%10+'0';
            centerPort[2] = centerport[1]/10+'0';
            centerPort[3] = centerport[1]%10+'0';
            centerPort[4] = centerport[2]/10+'0';
            centerPort[5] = centerport[2]%10+'0';
            centerPort[6] = '\0';
            break;
        }
        case 5:
        {
            centerPort[0] = centerport[0]%10+'0';
            centerPort[1] = centerport[1]/10+'0';
            centerPort[2] = centerport[1]%10+'0';
            centerPort[3] = centerport[2]/10+'0';
            centerPort[4] = centerport[2]%10+'0';
            centerPort[5] = '\0';
            break;
        }
        case 4:
        {
            centerPort[0] = centerport[1]/10+'0';
            centerPort[1] = centerport[1]%10+'0';
            centerPort[2] = centerport[2]/10+'0';
            centerPort[3] = centerport[2]%10+'0';
            centerPort[4] = '\0';
            break;
        }
        case 3:
        {
            centerPort[0] = centerport[1]%10+'0';
            centerPort[1] = centerport[2]/10+'0';
            centerPort[2] = centerport[2]%10+'0';
            centerPort[3] = '\0';
            break;
        }
        case 2:
        {
            centerPort[0] = centerport[2]/10+'0';
            centerPort[1] = centerport[2]%10+'0';
            centerPort[2] = '\0';
            break;
        }
        case 1:
        {
            centerPort[0] = centerport[2]%10+'0';
            centerPort[1] = '\0';
            break;
        }
        
    }
    if(Utility_Strncmp("59.48.98.130",centerIP,12) == 0)
        Utility_Strncpy(centerPort,"5005",4);
    PortStrLen = Utility_Strlen(centerPort);
    Utility_Strncpy(&_send[_cmdLen], centerPort, PortStrLen);
    _cmdLen += PortStrLen;
    
    TraceMsg(centerPort,1);

    //该AT指令会返回:
    //OK
    //注意，需要先关闭回显，以免造成麻烦

    GPRS_PrepareForSend();
    GPRS_WriteBytes(_send,_cmdLen);
    _retValue = GPRS_Proc_AT_Response(_recv, &_retMsgNum, OPEN_TCP_LINK, &errorCode);
    //开启成功会返回"CONNECT"

    if(_retValue == TRUE)
    {
        TraceMsg("TCP Connection success",1);
        //Console_WriteStringln("TCP Connection success");
        return TCP_CONNECT_SUCCESS;
    }
    else
    {
        Console_WriteStringln("TCP Connection failed-1223");
        //++SIM_ErrorNum;
    }

    return TCP_CONNECT_FAILED;
}


//此函数暂时未用到
int GPRS_AT_QueryTCPLink(int * errorCode)
{
    int _retMsgNum = 0;//
    int _cmdLen = 0;
    int _retValue = 0;
    char _send[10] = "AT%IPOPEN?";
    char _recv[100] = {0};
    _cmdLen += 10;

    //该AT指令会返回:
    //OK
    //注意，需要先关闭回显，以免造成麻烦
    GPRS_PrepareForSend();
    GPRS_WriteBytes(_send,_cmdLen);
    _retValue = GPRS_Proc_AT_Response(_recv, &_retMsgNum, QUERY_TCP_LINK, errorCode);
    // 查询指令会先返回一行%ETCPIP:1,"10.101.53.141",,"114.114.114.114","211.138.156.146", 
    //注意，只返回本地IP还不行，还要有网关和DNS，否则后面打开连链接会出问题
    // 再返回一行 OK

    if(_retValue == FALSE)//0，证明没有激活GPRS网络
    {
        Console_WriteStringln("No Tcp Connection error");
        return NO_TCP_CONNECT;
    }
    else
    {
        TraceMsg("Tcp Connection OK",1);
        Console_WriteStringln("Tcp Connection OK");
        return TCP_CONNECT_OK;
    }

}

int GPRS_AT_CloseTCPLink(int * errorCode)
{
    int _retMsgNum = 0;//
    int _cmdLen = 0;
    int _retValue = 0;
    char _recv[30] = {0};


    char _send[50] = " AT%IPCLOSE=1";
    _cmdLen += 12;

    //该AT指令会返回:
    //%IPCLOSE:
    //OK
    //注意，需要先关闭回显，以免造成麻烦
    GPRS_PrepareForSend();
    GPRS_WriteBytes(_send,_cmdLen);
    _retValue = GPRS_Proc_AT_Response(_recv, &_retMsgNum, CLOSE_TCP_LINK, errorCode);

    if(_retValue == FALSE)
    {
        Console_WriteStringln("Close TCP link failed-1286");
        return TCP_Link_CLOSE_FAILED;
    }else
    {
        TraceMsg("Close TCP link Success",1);
        return TCP_Link_CLOSE_SUCCESS;
    }

}


int GPRS_AT_QuitGPRSNet(int * errorCode)
{
    int _retMsgNum = 0;//
    int _cmdLen = 0;
    int _retValue = 0;
    char _recv[30] = {0};
    char _send[50] = "AT%IPCLOSE=5";
    _cmdLen += 12;

    //该AT指令会返回:
    //%IPCLOSE:
    //OK
    //注意，需要先关闭回显，以免造成麻烦
    GPRS_PrepareForSend();
    GPRS_WriteBytes(_send,_cmdLen);
    _retValue = GPRS_Proc_AT_Response(_recv, &_retMsgNum, QUIT_GPRS_NET, errorCode);
    //开启成功会返回"CONNECT"

    if(_retValue == TRUE)
    {
        TraceMsg("Quit GPRS Net Success",1);
        //Console_WriteStringln("Quit GPRS Net Success");
        return GPRS_NET_QUIT_SUCCESS;
    }else
    {
        Console_WriteStringln("Quit GPRS Net failed");
    }

    return GPRS_NET_QUIT_FAILED;
}

int gprsConfigSuccess = FALSE;
//使用ASCii码还是Hex码发送消息，0,1,1---ASCii， 1,1,0---Hex
int GPRS_AT_SetIOMode()
{
    int errorCode=0;
    int _retMsgNum = 0;//
    int _cmdLen = 0;
    int _retValue = 0;
    char _send[20] = "at%iomode=1,1,0";
    char _recv[30] = { 0 };
    _cmdLen += 15;

    //该AT指令会返回:
    //OK
    //注意，需要先关闭回显，以免造成麻烦
    GPRS_PrepareForSend();
    GPRS_WriteBytes(_send,_cmdLen);
    _retValue = GPRS_Proc_AT_Response(_recv, &_retMsgNum, SETIO_MODE, &errorCode);

    if(_retValue == TRUE)
    {
        TraceMsg("Set IO Mode Success",1);
        return SET_IOMODE_SUCCESS;
    }else
    {
        Console_WriteStringln("Set IO Mode Failed");
    }

    return SET_IOMODE_FAILED;
}

//int GRPS_AT_Send(char* psrc, int dataLen, int * errorCode)
int GRPS_AT_Send()
{
#if 0
    if(_GTM900SendBufLeft == 0 )//如果发送缓存=0，则不发送。
    {
        return MSG_SEND_FAILED;
    }
#endif

    int errorCode=0;
    int _retMsgNum = 0;//
    int _cmdLen = 0;
    int _retValue = 0;
    char _recv[30] = {0};

    char *_send = (char *)malloc(dataLen+12);
    if(_send == NULL)
    {
        Console_WriteStringln("Malloc in GPRS_AT_Send failed");
        return MSG_SEND_FAILED;
    }
    Utility_Strncpy(_send,"AT%IPSEND=\"",11);
    _cmdLen += 11;

    Utility_Strncpy(&_send[_cmdLen], psrc, dataLen);
    _cmdLen += dataLen;

    _send[_cmdLen] = '\"';
    _cmdLen += 1;

    //该AT指令会返回:
    //OK
    //注意，需要先关闭回显，以免造成后续数据接收干扰
    GPRS_PrepareForSend();
    GPRS_WriteBytes(_send,_cmdLen);
    _retValue = GPRS_Proc_AT_Response(_recv, &_retMsgNum, SEND_DATA, &errorCode);
    // 发送指令会先返回一行 %IPSEND:1,15
    // 再返回一行 OK
    free(_send);
    _send = NULL;

    if(_retValue == TRUE)
    {
        TraceMsg("AT Send Data Success",1);
        return MSG_SEND_SUCCESS;
    }
    else
    {
        Console_WriteStringln("AT Send Data failed-sub1408");
        
     //通过Timer0_A0中断显示通信模块不存在或内部电路故障 LSHB 20200506
            sys_errorcode_runcode=38;
            Led_OffAll();
            led_flash_init(45);
            Timer0_A0_Enable(); //Timer0_A0开中断
            System_Delayms(1000);
            Timer0_A0_Clear();//Timer0_A0关中断
            Led_OffAll();
            Led1_WorkLightOff();//工作指示灯关,节能
           // end  LSHB 20200506                       
        
       
    }
    
    return MSG_SEND_FAILED;
}

int GRPS_AT_QueryRemainData()
{
    int errorCode=0;
    int _retMsgNum = 0;
    int _cmdLen = 0;
    int _retValue = 0;
    char _send[8] = "AT%IPDQ";
    char _recv[200] = {0};
    
    _cmdLen += 7;
    //该AT指令会返回:
    //OK
    //注意，需要先关闭回显，以免造成后续数据接收干扰
    GPRS_PrepareForSend();
    GPRS_WriteBytes(_send,_cmdLen);
    _retValue = GPRS_Proc_AT_Response(_recv, &_retMsgNum, QUERY_REMAIN_DATA, &errorCode);
    // 发送指令会先返回一行 %IPDQ:
    // 再返回一行 OK

    if(_retValue == TRUE)
    {
        return QUERY_REMAIN_DATA_SUCCESS;
    }
    else
    {
        Console_WriteStringln("AT Query remaining data failed");
    }
    
    return QUERY_REMAIN_DATA_FAILED;
}

int GRPS_AT_Receive()
{
    int errorCode=0;
    int _retMsgNum = 0;
    int _cmdLen = 0;
    int _retValue = 0;
    char _send[8] = "AT%IPDR";
    char _recv[200] = {0};
    
    _cmdLen += 7;
    //该AT指令会返回:
    //OK
    //注意，需要先关闭回显，以免造成后续数据接收干扰
    GPRS_PrepareForSend();
    GPRS_WriteBytes(_send,_cmdLen);
    _retValue = GPRS_Proc_AT_Response(_recv, &_retMsgNum, RECEIVE_DATA, &errorCode);
    // 发送指令会先返回一行 %IPDR:1,1,87,"7E7E"
    // 再返回一行 OK

    if(_retValue == TRUE)
    {
        TraceMsg("AT Receive Data Success",1);
        return MSG_RECEIVE_SUCCESS;
    }
    else
    {
        Console_WriteStringln("AT Receive Data failed");
    }
    
    return MSG_RECEIVE_FAILED;
}

int GPRS_OFF_CALL()
{
    int _retNum;
    char _ath[]="ATH";
    char _recv[50];
    //该指令如果挂断了电话,返回OK,无电话状态时,也返回OK,OK只表示模块收到了指令
    //OK  
    UART0_Send(_ath,3,1);//两个ring之间时间差大于ring和OK出现时间差，因此，OK肯定会先出来?    if(UART0_RecvLineWait(_recv,50,&_retNum)==-1)
    if(UART0_RecvLineWait(_recv,50,&_retNum)==-1)
    {
        return 0;
    }
    if(Utility_Strncmp(_recv,"OK",2)==0)
    {
        return -3; 
    }
    else
    {
        return 0;
    }
}


int GPRS_AT_OFF_CALL()
{
    int _retNum = 0;
    char _ath[]="ATH";
    char _recv[50];
    //该指令如果挂断了电话,返回OK,无电话状态时,也返回OK,OK只表示模块收到了指令
    //OK  
    UART0_Send(_ath,3,1);//两个ring之间时间差大于ring和OK出现时间差，因此，OK肯定会先出来?    
    if(UART0_RecvLineWait(_recv,50,&_retNum)==-1)
    {
        return 0;
    }
    if(Utility_Strncmp(_recv,"OK",2)==0)
    {
        return -3; 
    }
    else
    {
        return 0;
    }
}

int  GPRS_ClearRecvBuf()
{
    UART0_ClearBuffer();
    return 0;
}

void GPRS_PrepareForSend()
{
    char _recvBeforeSend[100];
    int _retLenghtBeforeSend;

    if(UART0_RecvLineTry(_recvBeforeSend,UART0_MAXBUFFLEN,&_retLenghtBeforeSend)==0)
    {
        if(Utility_Strncmp(_recvBeforeSend,"RING",4)==0)//如果是电话来了，立刻关闭电话，其它响应一概不处理
        {
            GPRS_AT_OFF_CALL();
        }
    }
    GPRS_ClearRecvBuf();//清空缓存
}

int GPRS_Create_TCP_Link(int center)
{    
    int _repeats=0;
    
    if(gprsConfigSuccess == FALSE)
    {
        GSM_Open();
        
        while(GSM_AT_CloseFeedback() != 0)
        {
            if(_repeats>0)
                return -1;
            ++_repeats;
        } 
        _repeats = 0;
        
        while(GPRS_AT_CheckSimStat() != GPRS_CHECKSIM_SUCCESS)
        {
            if(_repeats>0)
                return -1;
            ++_repeats;
        } 
        _repeats = 0;
        
        while(GPRS_AT_QueryOperater() != QUERY_OPERATER_SUCCESS)
        {
            if(_repeats>0)
                return -1;
            ++_repeats;
        } 
        _repeats = 0;
        
        while(GPRS_AT_SetAPN() != SET_APN_SUCCESS)
        {
            if(_repeats>0)
                return -1;
            ++_repeats;
        } 
        _repeats = 0;
        
        while(GPRS_AT_ActiveGPRSNet() != ACTIVE_GPRS_SUCCESS)
        {
            if(_repeats>0)
                return -1;
            ++_repeats;
        } 
        _repeats = 0;
        
        while(GPRS_AT_QueryIP() != QUERY_IP_SUCCESS)
        {
            if(_repeats>0)
                return -1;
            ++_repeats;
        } 
        _repeats = 0;
        
        while(GPRS_AT_OpenTCPLink(center) != TCP_CONNECT_SUCCESS)
        {
            if(_repeats>0)
                return -1;
            ++_repeats;
        } 
        _repeats = 0;
        
        while(GPRS_AT_SetIOMode() != SET_IOMODE_SUCCESS)
        {
            if(_repeats>0)
                return -1;
            ++_repeats;
        } 
        _repeats = 0;
    
        gprsConfigSuccess = TRUE;
    }
    
    return 0;
}

int GPRS_Close_TCP_Link()
{
    int _repeats=0;
    int _retErrorCode;
    
    gprsConfigSuccess = FALSE;
    
    while(GPRS_AT_CloseTCPLink(&_retErrorCode) != TCP_Link_CLOSE_SUCCESS)
    {
        if(_repeats>0)
            return -1;
        ++_repeats;
    } 
    _repeats = 0;
    
    while(GPRS_AT_QuitGPRSNet(&_retErrorCode) != GPRS_NET_QUIT_SUCCESS)
    {
        if(_repeats>0)
            return -1;
        ++_repeats;
    } 
    _repeats = 0;
    
    return 0;
}

void GPRS_Close_GSM()
{
    gprsConfigSuccess = FALSE;
    GSM_Close(0);
    System_Delayms(500);//关机延时0.5s
}

int GPRS_Send(char* pSend, int sendDataLen, int isLastPacket, int center)
{
    int _retvalue;
    //首先查看TCP连接建立没有，没有建立，需要重新建立连接?
    if(gprsConfigSuccess == FALSE)
    {
        GPRS_Close_GSM();
        GPRS_Create_TCP_Link(center);
    }

    if( sendDataLen > 1024)
    {
        Console_WriteStringln("Error, Packet exceed 1024, Please Decrease Sending Packet Size!");
    }

    psrc = (char *)malloc(sendDataLen*2+1);//+1是因为转换函数hex_2_ascii最后有个\0,否则最后会出越界错误
    if(psrc == NULL)
    {
        Console_WriteStringln("Malloc in GPRS_Send Failed");
        return FALSE;
    }
    hex_2_ascii(pSend, psrc, sendDataLen);
    dataLen = sendDataLen*2;
    
    _retvalue = GRPS_AT_Send();

    free(psrc);
    psrc = NULL;
    
    if(_retvalue == MSG_SEND_SUCCESS)
    {
        TraceMsg("GPRS Send Data Success",1);
        return TRUE;
    }
    else
    {
        TraceMsg("GPRS Send Data Failed-1690",1);
        
       //通过Timer0_A0中断显示通信模块不存在或内部电路故障 LSHB 20200506
            sys_errorcode_runcode=38;
            Led_OffAll();
            led_flash_init(45);
            Timer0_A0_Enable(); //Timer0_A0开中断
            System_Delayms(1000);
            Timer0_A0_Clear();//Timer0_A0关中断
            Led_OffAll();
            Led1_WorkLightOff();//工作指示灯关,节能
           // end  LSHB 20200506  
        
        
        
    }
    
    return FALSE;
}

int GPRS_QueryRemainData()
{
    if(GRPS_AT_QueryRemainData() == QUERY_REMAIN_DATA_SUCCESS)
    {
        return RemainingData;
    }
    return -1;
}

char* GPRS_Receive()
{
    char* hexdata = NULL;//+++
    
    if(GRPS_AT_Receive() == MSG_RECEIVE_SUCCESS)
    {
        hexdata = (char*)malloc(_ReceiveDataLen);
        ConvertAscIItoHex(_ReceiveData, hexdata, _ReceiveDataLen*2);
        free(_ReceiveData);
        _ReceiveData = NULL;
    }
    else if(GRPS_AT_Receive() == MSG_RECEIVE_SUCCESS)
    {
        hexdata = (char*)malloc(_ReceiveDataLen);
        ConvertAscIItoHex(_ReceiveData, hexdata, _ReceiveDataLen*2);
        free(_ReceiveData);
        _ReceiveData = NULL;
    }
    else if(GRPS_AT_Receive() == MSG_RECEIVE_SUCCESS)
    {
        hexdata = (char*)malloc(_ReceiveDataLen);
        ConvertAscIItoHex(_ReceiveData, hexdata, _ReceiveDataLen*2);
        free(_ReceiveData);
        _ReceiveData = NULL;
    }
      
    return hexdata;
}

char unprocessedGPRSdata = 0;
int GPRSDataArrived = FALSE;
void JudgeServerDataArrived(void)
{
    if(Utility_Strncmp(UART0_Rx_Buffer[0], "%IPDATA:",8) == 0)
    {
      memset(UART0_Rx_Buffer[0],0,UART0_MAXBUFFLEN);
      GPRSDataArrived = TRUE;
    }
    if(Utility_Strncmp(UART0_Rx_Buffer[1], "%IPDATA:",8) == 0)
    {
      memset(UART0_Rx_Buffer[1],0,UART0_MAXBUFFLEN);
      GPRSDataArrived = TRUE;
    }
    if(Utility_Strncmp(UART0_Rx_Buffer[2], "%IPDATA:",8) == 0)
    {
      memset(UART0_Rx_Buffer[2],0,UART0_MAXBUFFLEN);
      GPRSDataArrived = TRUE;
    }
}

int Hydrology_ProcessGPRSReceieve()
{
  char *dowmhydrologydata = NULL;
  
  if(GPRSDataArrived != TRUE)
    return -1;
//   GPRS_QueryRemainData();
   unprocessedGPRSdata = 1;
  while(unprocessedGPRSdata--)
  {
    dowmhydrologydata = GPRS_Receive();
    
    TraceHexMsg(dowmhydrologydata,_ReceiveDataLen);
     if(hydrologyProcessReceieve(dowmhydrologydata, _ReceiveDataLen) != 0)
        continue;
    HydrologyRecord(ERC17);
  }
  if(dowmhydrologydata != NULL)
    free(dowmhydrologydata);
    dowmhydrologydata = NULL;//+++
  
  GPRSDataArrived = FALSE;
  return 0;
}









