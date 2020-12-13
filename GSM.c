//////////////////////////////////////////////////////
//     文件名: gsm.c
//   文件版本: 1.0.0
//   创建时间: 09年11月30日
//   更新内容: 
//       作者: 林智
//       附注: 无
// 
//////////////////////////////////////////////////////

//////////////////////////////////////////////////////
//     文件名: gsm.c
//   文件版本: 3.0.0
//   创建时间: 19年8月28日
//   更新内容: 更换充电引脚和GPRS的IGT引脚
//       
//////////////////////////////////////////////////////
//#include <msp430x26x.h>
#include "GSM.h"
#include "Store.h"
#include "common.h"
#include "Timer.h"
#include "uart0.h"
#include "rtc.h"
#include "led.h"
#include "Sampler.h"
#include "Console.h"
#include "message.h"
#include "hydrologycommand.h"
#include "string.h"
// 短信缓存数组里最多放几条
#define ARRAY_MAX 15 
#define MSG_MAX_LEN  120



 int g_work_mode;
 int main_time_error;
extern int s_alert_flag[8];

//  调试统计计数  
int  s_TimeOut=0;
int  s_RING=0;
int  s_OOPS=0;
int  s_MsgFail=0;
int  s_MsgOK=0;
int  s_MsgDel=0;

int  s_RecvIdx=0;    //注意是指向最后索引的下一个
int  s_ProcIdx=0;    //注意是指向最后处理的下一个. 
int  s_MsgNum=0;     //收到短信的总数 
int  s_MsgArray[ARRAY_MAX];

//函数间交互的变量
int  s_MsgLeft=0;
char s_NetState='0';

/*
 *@ author : howard
 *@ return : 0：success  
            -1:获取时间失败  
            -2:全局数据区空间不足，malloc失败
*/
int  GSM_AT_QueryTime(char *year,char *month,char *date,char *hour,char *min,char *second)
{
    int rcvFlag = 0;
    int _repeat = 0;
    int _retNum;
    char _sendAT[30]="AT+CCLK?";
    char *pcAtResp = NULL;
    char *pcSeperate = "/";
    static char *pcRcvAtBuff = NULL;  //定义静态局部变量，分配在全局数据区
    
    
    pcRcvAtBuff = (char *)malloc(200 * sizeof(char));
    if( pcRcvAtBuff == NULL )
    {
      printf("pcRcvAtBuff malloc failed! \r\n");
      return -2;
    }
    
    UART0_Send(_sendAT,8,1);
    
    
    while (_repeat < 10)
    {
        if ( (UART0_RecvLineTry(pcRcvAtBuff, 200, &_retNum) == 0) && ((pcAtResp = strstr(pcRcvAtBuff,pcSeperate)) != NULL) ) //有数据
        {
            rcvFlag = 1;
            break ;
        }
        _repeat++;
        System_Delayms(50);
    }
    
    
    /* got AT response contained time */
    if( rcvFlag == 1 )
    {
//        printf("%s \r\n ",pcRcvAtBuff);
//        printf("%s \r\n ",pcAtResp);
        pcAtResp -= 2;
        
        int offset = 0;
        char year1,year2,month1,month2,date1,date2,hour1,hour2,min1,min2,sec1,sec2;
        year1 = *(pcAtResp+offset);
        offset++;
        year2 = *(pcAtResp+offset);
        offset++;
        offset++;
        month1 = *(pcAtResp+offset);
        offset++;
        month2 = *(pcAtResp+offset);
        offset++;
        offset++;
        date1 = *(pcAtResp+offset);
        offset++;
        date2 = *(pcAtResp+offset);
        offset++;
        offset++;
        hour1 = *(pcAtResp+offset);
        offset++;
        hour2 = *(pcAtResp+offset);
        offset++;
        offset++;
        min1 = *(pcAtResp+offset);
        offset++;
        min2 = *(pcAtResp+offset);
        offset++;
        offset++;
        sec1 = *(pcAtResp+offset);
        offset++;
        sec2 = *(pcAtResp+offset);

  /* char *year,char *month,char *date,char *hour,char *min,char *second */
        *year = (year1-48)*10 + (year2-48);
        *month = (month1-48)*10 + (month2-48);
        *date = (date1-48)*10 + (date2-48);
        *hour =( (hour1-48)*10 + (hour2-48));
        *min = (min1-48)*10 + (min2-48);
        *second = (sec1-48)*10 + (sec2-48);
    }
    
    free(pcRcvAtBuff);
    pcRcvAtBuff = NULL;
    
    if( rcvFlag == 1 )
    {
      return 0;
    }
    else
    {
      return -1;
    }
    
    
}

int GSM_Open()
{
//    int _retNum;
//    char _recv[30]="AT%TSIM";
    TraceMsg("GSM Open !",1);
    //初始化串口0,
    UART0_Open(UART0_GTM900_TYPE);  //lmj0814,GTM900 AT指令最后需要附加的是\r\n,和mc35i有所不同，因此，在串口发送函数里面，最后附加\r\n
    
    // 为SIM模块上电 
    P10DIR |= BIT7; // 2438 P4.2 为 BATT,给高电平  5438为P10.7
    P10OUT &= ~BIT7; 
     P10OUT |= BIT7; 
    System_Delayms(100);
    
    P3DIR |= BIT0; //  2438 p1.0 为 /IGT  5438为P3.0
    P3OUT |= BIT0; //  开始为高阻态   
    System_Delayms(10);//  最少需要10ms 
    
    
    P3OUT &=~BIT0; // 给/IGT一个下降,保持100ms
//    System_Delayms(50);  // 需要延时 
    
//    P3DIR &= ~BIT3; 
    System_Delayms(50); // GTM900需要50ms
    //System_Delayms(50); // MC35i需要150ms
    P3OUT |= BIT0; // 再把/IGT 拉高

    System_Delayms(1200);// 等待串口 1秒的无效状态
                 // 串口正常通信
    //再等12秒 等SIM连上网络,至少也得7秒
    // 貌似10秒还不太够///
    System_Delayms(30000);
  
    //初始化相关变量
    s_RecvIdx=0;     
    s_ProcIdx=0;
    s_MsgNum=0;
    s_MsgLeft=0;
    for(int i=0;i<ARRAY_MAX;++i)//短信数组初始值为0(0是不正确的短信索引)
        s_MsgArray[i]=0;
    return 0;
}

int GSM_HardReboot()
{
	P3DIR |= BIT0; //  p1.0 为 /IGT 
    //P1OUT |= BIT0; //  开始为高阻态   
    //System_Delayms(10);//  最少需要10ms 
    P3OUT &=~BIT0; // 给/IGT一个下降,保持2500ms
    System_Delayms(2500); // 关机需要拉低2-3秒
    P3OUT |= BIT0; // 再把/IGT 拉高
    System_Delayms(15000);// 等待15s关机
                 // 串口正常通信
   	P3OUT &=~BIT0; // 给/IGT一个下降,保持50ms
    System_Delayms(50); // 最少需要50ms 再重新开机
    P3OUT |= BIT0; // 再把/IGT 拉高
    System_Delayms(30000);
    //初始化相关变量
    s_RecvIdx=0;     
    s_ProcIdx=0;
    s_MsgNum=0;
    s_MsgLeft=0;
    for(int i=0;i<ARRAY_MAX;++i)//短信数组初始值为0(0是不正确的短信索引)
        s_MsgArray[i]=0;
	
    return 0;
}

int GSM_Close(int _type)
{
    TraceMsg("GSM Close !",1);
    if(!_type)
        GSM_AT_ShutDown();//发送关闭AT指令
    System_Delayms(100);
    //清空这些计数
    s_RecvIdx=0;     
    s_ProcIdx=0;
    s_MsgNum=0;   
    s_MsgLeft=0;
    
    P10DIR |= BIT7;
    P10OUT &= ~BIT7;  //断开SIM电压
    
    UART0_Close();   //关闭串口0
    return 0; 
}
int GSM_CheckOK()
{
    int times=0;
    int ret=-1;
    while(ret!=0)
    {
        ret=GSM_Init();
        if(times>1)
        {
            Led_WARN();
            TraceMsg("GSM doesn't work !",1);
            return -1;//无法使用返回 -1; 
             
        }
        ++times;
    }
    return 0;//正常返回   0 ; 
}

int GSM_Waiting_Process(int _type)
{
    int _waits=100;//30秒
    switch(_type)
    { 
      case 1:
        _waits=100;
        break;
      case 2:
        _waits=200;//1分钟
        break;
      case 3:
        _waits=300;//1.5分钟
        break;
      case 4:
        _waits=400;//2分钟
        break;
      default:
        _waits=100;//30s
        break;
    }
    int _retNum;
    char _recv[UART0_MAXBUFFLEN];//接收缓存
    int _repeats=0;  //
    int _repeats1=0; //
    if(GSM_DealAllData()==2)//有短信直接返回
    {
        return 2;
    }
    while(1)
    {
        if(GSM_CheckLive()<0)
        {
            GSM_Open();
            if(GSM_CheckOK()<0)
            {
                return -2;
            }
        }
        //等待数据, 等待1分钟
        while(UART0_RecvLineWait(_recv,UART0_MAXBUFFLEN,&_retNum)<0)
        {
            //UART0_RecvLineWait是300ms
            //300ms * 300 =  90000ms = 90s
            if(_repeats > _waits)    
            {
                return 0;
            }
            ++ _repeats;
        }
        //如果是短信,就速度返回进行处理
        if(GSM_DealData(_recv,_retNum)==2)
            return 2;
        
        ++ _repeats1;//接收到数据的一次循环
        if( _repeats1>2)// 
            return -1;
    } 
}

//
// 上报数据格式如下:
//                                                                                                      1 
//  0         1         2         3         4         5         6         7         8         9         0    
//  0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678
//  $    站号   > 时间     * 数据条目(模拟值为 类型码+4位10进制   脉冲为类型码加 6位16进制 开关为类型码加一位2进制)
//  $00011100011>0908161230*A1000B1000C1000D2000E2000F3000G3000H4000I000000J000000K000000L000000M1N1O1P1Q1R1S1T1#
//
//
//   逻辑很混乱, 没办法了.
//
int GSM_Report_Process()
{//发给多中心.
    //检查数据存储上标和 下标 
    char _startIdx=0x00;
    char _seek_num=0; //确保读数据的总数 不会超过总条数,避免出现死循环
    char _endIdx=0x00;
    char _data[220];
    char _phone[12];
    int  _ret=0;
    int  _send_ret=0;
    int  _idx = 13; 
    int  _exit=0;  
    //记录哪些短信包含 哪些_start的数组
    int  _array[10]={0,0,0,0,0,0,0,0,0,0};
    int  _array_idx=0;
    if(RTC_ReadStartIdx(&_startIdx)<0 || RTC_ReadEndIdx(&_endIdx)<0 )
    {//如果读出数据索引标记失败
        TraceMsg("read idx error ,reGenerate .",1);
        if(Hydrology_RetrieveIndex()<0)
        {
            TraceMsg("reGen failed .",1);
            RTC_SetStartIdx(DATA_MIN_IDX);
            RTC_SetEndIdx(DATA_MIN_IDX);
        }
        TraceMsg("StartIdx:",0); 
        TraceInt4(_startIdx,1);
        TraceMsg("EndIdx:",0);
        TraceInt4(_endIdx,1);
        RTC_ReadStartIdx(&_startIdx);//重新读出
        RTC_ReadEndIdx(&_endIdx);//重新读出
    }
    //头部
    _data[0] = '$';
    Store_ReadDeviceNO(&(_data[1]));
    _data[12] = '>';
    if( _endIdx<DATA_MIN_IDX || _endIdx >DATA_MAX_IDX ||
       _startIdx<DATA_MIN_IDX || _startIdx >DATA_MAX_IDX  )
    {
        return -1;
    } 
    //
    //   1. 组装 数据
    // 
    //   2. 发送数据
    //
    //   3. 是否达到_endIdx 退出
    //  
    
    while(1)
    {//发送循环
        //_idx开始为13; _ret为0
        _idx=13;_ret=0;
        //组装数据循环
        while(1)
        {//怎样能绝对避免死循环呢?  
            _ret = Store_ReadDataItem(_startIdx,&(_data[_idx]),0);
            if(_ret<0)
            {
                RTC_SetStartIdx(_startIdx);//更新_startIdx
                return -1; //无法读取数据 就直接退了.
            }
            if(_ret==1)
            {//这个是一个已经发送过的数据,那么就不发了 
                if(_startIdx == _endIdx)
                {//检查是否到了  _endIdx, 如果是就不继续循环了.
                    _exit=1;
                 //完成组装.
                    _data[_idx-1]='#';//把最后一个;号改为#
                    break;
                }
                //
                //
                //
                if(_seek_num > DATA_MAX_IDX - DATA_MIN_IDX)
                {//这里判断一下,避免出现死循环.
                    _exit=1;
                    _data[_idx-1]='#';//把最后一个;号改为#
                    break;
                }
                
                if(_startIdx >= DATA_MAX_IDX)  _startIdx=DATA_MIN_IDX;
                else   ++ _startIdx;//下一数据
                ++_seek_num;//递增一个数据就加1.
                
                continue;
            }
            else
            {//是正常的发送数据,就增加_idx,并添加;号 
                _array[_array_idx++] = _startIdx;
                _idx += _ret;
                _data[_idx++]=';';//加上一个分隔符
                if( _idx + _ret < MSG_MAX_LEN )//长度够再装一条.
                {
                    if(_startIdx >= DATA_MAX_IDX ) _startIdx = DATA_MIN_IDX;
                    else ++ _startIdx;
                    ++_seek_num;//递增一个数据就加1.
                    
                    continue;
                }
            }
            _data[_idx-1]='#';//把最后一个;号改为#
            
            //增加下一次的 _startIdx
            if(_startIdx>=DATA_MAX_IDX) _startIdx=DATA_MIN_IDX;
            else   ++ _startIdx;//下一数据
            ++_seek_num;//递增一个数据就加1.
            
            //完成了数据的组装,长度为_idx;
            break;
        }
        //发送循环
        if(_array_idx!=0)//若内容缓存中有东西就发送
        {
            for(int i =1; i<=4;++i)
            {
                if(Store_GSM_ReadCenterPhone(i,_phone)<0)  continue;
                if(_phone[0]=='0'||_phone[1]=='0'||_phone[2]=='0') continue;
                _send_ret += GSM_SendMsgTxt(_phone,_data,_idx);
            }
            if(_send_ret==0)//将内容中包含到的数据条目都设置成发送过.
            {
                for(int i=0;i<_array_idx;++i)
                {//设置为已经发送
                    Store_MarkDataItemSended(_array[i]);
                }
                //然后清除数组
                _array_idx=0;
                //发送完后,要更新_startIdx.
                RTC_SetStartIdx(_startIdx);
            }
            else
                _exit=1;//某条短信发送失败,就不发了.
            
            
            //然后清除数组
            _array_idx=0;
        }
        //判断是否 达到 _endIdx
        if(_exit)
            break;
    }
    return 0;
}

int GSM_Process(int _type, int _needReport)
{
    if(_needReport)
        GSM_Report_Process();
    
    if(GSM_CheckLive()<0)
    {
        GSM_Open();
        if(GSM_CheckOK()<0)
        {
            return -1;
        }
    }
    
    GSM_ProcMsgArray();
    
    
    
    GSM_StartCheckSIM();//检查并处理SIM卡内剩余短信
    if(GSM_CheckLive()<0)
    {
        GSM_Open();
        if(GSM_CheckOK()<0)
        {
            return -1;
        }
    }
    
    Timer_Zero();
    //等待过程 我们想有以下效果. 
    // 先处理完先前的短信,然后开始等待
    // 2次短信间隔最大时间为1分钟,如果超过1分钟就退出.
    // 如果完全无数据,1分钟就退出.
    // 要是 电话则 SIM_Deal直接就挂断, 如果是其他数据,也不管.
    // 要是短信到来. 则会立即开始处理.
    // 处理完后再继续等待这个1分钟.
    
    //第一部分等待. 等待时间是 _type
    if( s_ProcIdx != s_RecvIdx) //有短信需要进行处理
    {
        //开始处理这些数组里的短信索引
        //处理过程中,增加处理下标,
        //直到处理下标达到了接收下标,就返回.  
        GSM_ProcMsgArray();
    }
    if(GSM_Waiting_Process(_type)!=2)//进行等待  有短信或是电话 就继续一轮等待
    {//不是因为短信 而返回的. 也即无数据到来.那就等完了.可以返回了.
        goto BREAK;
    }
    //如果有第2部分的等待. 间隔时间是30秒
    while(1)
    {
        //1. 处理短信
        GSM_ProcMsgArray(); 
        if(Timer_Passed_Mins(5)) //时间不得超过5分钟,
        {//Console_WriteStringln("SIM_LOOP: too long time, force to quit !");
            goto BREAK;
        }
        //2. 然后等待
        if(GSM_Waiting_Process(1)!=2)//进行等待  有短信或是电话 就继续一轮等待
        {//间隔时间是30秒
            break;
        }
    }
BREAK:
    if(GSM_CheckLive()<0)
    {
        GSM_Open();
        if(GSM_CheckOK()<0)
        {
            return -1; 
        }
    }
    GSM_EndCheckSIM();// 关机前检查SIM中短信容量  确保为0;  
    return 0;// 
}
//  0         1         2         3         4         5         6         7         8         9         0    
//  0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678
//  $    站号   > 时间     * 数据条目(模拟值为 类型码+4位10进制   脉冲为类型码加 6位16进制 开关为类型码加一位2进制)
//  $00011100011>0908161230*A1000B1000C1000D2000E2000F3000G3000H4000I000000J000000K000000L000000M1N1O1P1Q1R1S1T1# 

int GSM_Alert_Process()
{//
    TraceMsg("Alert !",1);
    if(GSM_CheckLive()<0)
        return -1;
    char _phone[11];
    char _send[150];
    int _ret=0;
    
    _send[0]='$';
    if(Store_ReadDeviceNO(&_send[1])<0)
        return -1;
    _send[12]='>';
    RTC_ReadTimeStr5_B(&_send[12]);
    _send[23]='*';
    _ret = Sampler_GSM_ReadAlertString(&(_send[24])); 
    if(_ret<0)
        return -1;
    _ret += 24;
    _send[_ret++]='#';
    //给配置中心发报警短信
    if(Store_GSM_ReadCenterPhone(1,_phone)>=0)
    {
        if(_phone[0]!='0' ||_phone[1]!='0' ||_phone[2]!='0')
        {
            GSM_SendMsgTxt(_phone,_send,_ret);
        }
    }
    //负责人
    if(Store_GSM_ReadCenterPhone(6,_phone)<0)//读取负责人电话 
            return -1; 
    if(_phone[0]!='0' ||_phone[1]!='0' ||_phone[2]!='0')
    {
        GSM_SendMsgTxt(_phone,_send,_ret); 
        GSM_CallLeader(_phone);//给负责人打电话
    }
    return 0; 
}

int GSM_CheckLive()
{//发送AT检查是否还有响应,无响应则认为GSM模块坏掉了
    int _repeats1=0;
    //int _repeats2=0;

  AT:    
    GSM_DealAllData();//清除缓冲区数据
    UART0_Send("AT",2,1);
    
    char _recv[20];
    int _retNum=0;
    if(UART0_RecvLineLongWait(_recv,20,&_retNum)<0)
    {
        ++ _repeats1;
        if(_repeats1<3)
        {
            goto AT;
        }
        Led_WARN();
        TraceMsg("GSM doesn't work !",1);
        return -1;
    }
    GSM_DealAllData();//清除缓冲区数据
    //再检查下连网状态
    s_NetState='0';
    GSM_AT_QueryNet(); //查询网络   
    if(s_NetState=='1'|| s_NetState=='5')
        return 0;
    else
    {
        Led_WARN();
        TraceMsg("GSM doesn't work !",1);
        return -2;
    }
}
int GSM_Init()//SIM初始设置
{
    int _repeats=0;  
    //int _repeats1=0;
    int _repeats2=0;
   // int _repeats3=0;
   s_NetState='0';
    while(GSM_AT_CloseFeedback()!=0)
    {//关闭回显,一定要成功, 不然后续代码都是错误的.
        if(_repeats>2)
        {//3次都是错误的.
            TraceMsg("Failed to close feedback !",1);
            return -1; 
        }
        ++_repeats;
    }

    while(GSM_AT_QuerySim()!=0)
    {//查询SIM卡是否正常.
        if(_repeats>2)
        {//3次都是错误的.
            TraceMsg("Sim card is not normal !",1);
            return -1;
        }
        ++_repeats;
    }

 #if 0
    while( GSM_AT_SetMsgmode(1)!=0)
    {//设置短信txt模式,一定要成功, 不然后续代码都是错误的.
        if(_repeats1>2)
        {//3次都是错误的. 
            return -2;
        }
        ++_repeats1;
    }
    while( GSM_AT_SetMsgNotify()!=0)
    {//设置短信要 串口提醒.
        if(_repeats3>2)
        {//3次都是错误的. 
            return -4;
        }
        ++_repeats3;
    }
#endif

    while( !(s_NetState=='1' || s_NetState=='5') )
    {
        TraceMsg("Wait for signal ready!", 1);
        System_Delayms(5000);//等待一秒
        //SIM_InitOK函数最多会被重复3次多.所以我们这里不必太多.
        if(_repeats2>2)
        {
            return -3;
        }
        GSM_AT_QueryNet(); //查询网络
        ++_repeats2;
    }
    return 0;
}
//
//  对于短信已读过(REC Readed)的分析,.首先我们要处理的短信的索引来源于
//  1. CNMI消息被记录到了msgArray里,读取msgArray获得的索引
//  2. checkSIM函数,按顺序递增的 索引
//  注意: 一条短信是不会有两次CNMI消息的.短信也不会被新来的短信覆盖 
//    1. 处理过程中 读短信在中途失败了.
//    比如说读到短信头,但是没读到短信内容,就会重新再次读这条短信,
//     此时可能这条短信就是已读状态了.
//    2. 删除短信失败了.虽然返回OK.但实际上并没删除
//       (手册上指出 删除短信不管成功没成功,都返回OK,
//         返回ERROR只不过是识别AT指令出错)
//    3. 其他情况 ????
//
//    1,2的矛盾就是 
//      要么删除一条实际并未处理的短信.
//      要么重复处理一条短信.重复次数依赖于删除这条短信失败的次数.
//    
//    我们选择重复处理.  所以我们仍旧处理REC Readed的短信.  
//0表示正常
//1表示无短信
//-1表示超时
int  GSM_ProcMessageTxt(int _index)//带失败重试,最多3次
{
    int _retNum;
    int _dataLen=0;//先做AT指令长度,后做短信内容长度 
    char _recv[UART0_MAXBUFFLEN];//接收缓冲
    char _rePhone[14];//回复电话
    char _send[50]; //发送缓冲,会"客串"一下接收缓存
    int _repeats=0;  //失败次数
    int _repeats1=0;
    int _repeats2=0;
    int _repeats3=0;
ProcMessage:
    //1. 生成AT指令串,
    Utility_Strncpy(_send,"AT+CMGR=",8);
    if(_index>=10)
    {
        _send[8]=_index/10+'0';
        _send[9]=_index%10+'0';
        _dataLen=10;
    }
    else
    {
        _send[8]=_index+'0';
        _dataLen=9;
    }
    //2.把缓冲区清除一下
    GSM_DealAllData();
    GSM_AT_QuitInput();
    
    
    /*
    //////////////////////////////////
    char _recv1[60];
    char _delete_msg[11]="AT+CMGD=1,4";
    //该指令只会返回:
    //OK
    UART0_Send(_delete_msg,strlen(_delete_msg),1); 
    System_Delayms(10000); //需要2秒的删除时间
    if(UART0_RecvLineWait(_recv1,60,&_retNum)==-1);
    if(Utility_Strncmp(_recv1,"ERROR",5)==0);
    if(Utility_Strncmp(_recv1,"OK",2)==0);
    ////////////////////////////////////////////////////
    */
    
    //3. 发AT读短信,并分析处理 
    UART0_Send(_send,_dataLen,1);
    System_Delayms(50);
    if(UART0_RecvLineWait(_recv,UART0_MAXBUFFLEN,&_retNum)==-1)
    { //未接收到短信头
        TraceMsg("can't get msg head.",1);
        ++s_TimeOut;
        if(_repeats > 1 )
        {
            //Console_WriteStringln("SIM_ProcMessage:Read Msg TimeOut:");
            //Console_WriteIntln(index);
            return -1;
        }
        ++_repeats;
        goto ProcMessage;
    }
    if(Utility_Strncmp(_recv,"+CMGR: ",7)==0)
    {//获得到了短信头,我们先判断下这个位置是不是没有短信
    //无短信. "+CMGR: 0,,0"
    // +CMGR: "REC UNREAD","+8613607193119",,"09/02/27,16:36:16+32"格式 
        if(Utility_Strncmp(_recv,"+CMGR: 0,,0",11)==0)
        {
            TraceMsg(" no msg find ",1); 
            return 1;//无短信
        }
        //这里需要注意的是?GTM900返回的字符串，以\"也即双引号引出。所以，这里索引坐标应该是从第8个开始
        if(Utility_Strncmp(&(_recv[8]),"REC",3)!=0)
        {
            System_Delayms(1000);
            //先把后续的短信内容收了.
            if(UART0_RecvLineLongWait(_recv,UART0_MAXBUFFLEN,&_retNum)<0)//最多等待可达5秒
            {//收不到也无所谓.反正我们不需要.
               ;
            }
            //再接个OK
            if(UART0_RecvLineWait(_recv,UART0_MAXBUFFLEN,&_retNum)==0)
            {
                if(Utility_Strncmp(_recv,"OK",2)==0)
                {
                    return 0;
                }
                else
                    GSM_DealData(_recv,_retNum);
            }
            return 0;
        }
        //然后我们获得短信的回复电话
        GSM_GetRePhone(_recv,_rePhone);
        System_Delayms(1000);//这里延时 需要很多 很多..
        //如果是一条未读的短信.它的数据末尾换行来得非常的晚..
        //尤其是受到干扰的情况下,  
        if(UART0_RecvLineLongWait(_recv,UART0_MAXBUFFLEN,&_dataLen)<0)//最多等待可达5秒
        {//短信内容数据一直没读完.
            TraceMsg("can't read msg data ",1);
            if(_repeats1>2)
                return -1;
            ++_repeats1;
            goto ProcMessage;
        }
        //判断是否是空短信
        if(Utility_Strncmp(_recv,"OK",2)==0)
        {
            TraceMsg("msg is a empty msg ",1);
            return 0;
        }
        if(UART0_RecvLineWait(_send,50,&_retNum)==-1)//都读到数据了, 不应返回 
            ++s_TimeOut;
        else
            GSM_DealData(_send,_retNum);//接受一个OK
        
        //如果是错误电话号码 就直接返回0 比如10086 网络电话等等
        if(Utility_CheckDigital(_rePhone,0,10))
        {
            TraceMsg("phone num is not right ",1);
            return 0;
        }
        char message[10]="SMS:";
        message[4]=(char)(_index);
        TraceStr(message,5,1);
        return GSM_ProcMsgData(_recv,_rePhone,_dataLen); //然后开始处理这条短信    
    }
    //
    //  如果是 ERROR说明这个 index可能有问题.
    //
    if(Utility_Strncmp(_recv,"ERROR",5)==0)//没读到短信头, 就重新再来.
    {
        if(_index>40)//40以上的index 很可能是过界了.
            return 2;//下标过界
        if(_repeats2>2)
            return -1;
        ++_repeats2; 
        goto ProcMessage; 
    }
    //既不是ERROR,也不是短信头;
    //再来一次;
    GSM_DealData(_recv,_retNum);
    if(_repeats3>2)
        return -1;
    ++_repeats3;
    goto ProcMessage;
}

int GSM_ProcMsgArray()
{//直到处理下标 达到短信数组总数
    int _ret; 
    while(s_ProcIdx != s_RecvIdx)
    { //下面这个函数的返回值 就是 它内部调用的发送短信函数的返回值
    //无短信 会返回 1
        _ret=GSM_ProcMessageTxt(s_MsgArray[s_ProcIdx]);
        if( _ret < 0)
        {//如果它发送短信没成功,我们再处理一次这条短信
         //没处理成功的短信,我们在这里都不删除.
            if(GSM_ProcMessageTxt(s_MsgArray[s_ProcIdx])== 0)
                GSM_AT_DeleteMsg(s_MsgArray[s_ProcIdx]);
        }
        else
        {
            if(_ret==0)
                GSM_AT_DeleteMsg(s_MsgArray[s_ProcIdx]);
        }
        if(s_ProcIdx < ARRAY_MAX-1)
            ++s_ProcIdx;
        else
            s_ProcIdx=0;
    }
    return 0;
}

int GSM_StartCheckSIM()  //开机检查一次SIM卡
{
    int _index=1;
    int _MsgNum=0;
    int _ret=0;
    s_MsgLeft= 0;
    if(GSM_AT_QueryStore()<0)//该函数会填写 SIM_MsgLeftNum;  
    {
        if(GSM_AT_QueryStore()<0)
            return -1;
    }
    if(s_MsgLeft == 0)//没短信
        return 0; 
   _MsgNum = s_MsgLeft;//有多少条短信 
    while( _MsgNum !=0 )//开机发现SIM卡里还有短信
    {
        _ret=GSM_ProcMessageTxt(_index);
        //这个函数可能返回 -1 0 1
        if(_ret==1)//如果这个位置没有短信,则下一条
        {
            ++ _index;
            if( _index > 80)//一直都处理到索引80了.. 可能是运行出错了.不管了.
                break;
            continue;//使这个循环多处理一条短信
        }
        if( _ret==2)
        {//该短信索引过界了
            break;
        }
        if( _ret==0)
        {//处理成功 就删除它 
            GSM_AT_DeleteMsg(_index);  
        }
        //失败了 就多处理一次
        if(_ret<0)
        {
            //成功了就删除
            if(GSM_ProcMessageTxt(_index)==0) 
                GSM_AT_DeleteMsg(_index);
            //再失败,我们就不管了,但不删除
        }
        //如果是返回-1 超时的话, 我们就直接不管了,
        // 但是也不删除. 留以后处理.
        //下一条短信
        ++ _index;
        -- _MsgNum;
    }
    //以上这个过程中来的短信 会被放在MsgArray数组中,会被正确处理的. 
    return 0;
}

int GSM_EndCheckSIM()     //关机检查一次SIM卡
{
    int _index; 
    int _MsgNum;
    int _repeats=0;
    int _ret=0;
EndCheckSIM: 
    _index=1;
    _MsgNum=0;
    s_MsgLeft = 0;
    if(GSM_AT_QueryStore()<0)//该函数会填写SIM_MsgLeftNum; 
    {
        if(GSM_AT_QueryStore()<0)
            return -1;
    }
    if(s_MsgLeft == 0)//没短信
        return 0; 
    _MsgNum = s_MsgLeft;//有多少条短信 
    while(_MsgNum!=0)
    {
        _ret=GSM_ProcMessageTxt(_index);
        if(_ret==1)//如果这个位置是空短信,则找下一条
        {
            ++ _index;
            continue;//使这个循环多处理一条短信
        }
        if(_ret==2)
        {
            break;
        }
        //不管是否处理成功 都删除它  
        //这里和startcheck 以及procMsgArray不一样
        GSM_AT_DeleteMsg(_index); 
        ++ _index;
        -- _MsgNum;
    }
    
    if(_repeats>2)
        return -2;
    ++ _repeats;
    goto EndCheckSIM; //再重新检查SIM卡里的短信是否为0 
} 
int GSM_SendMsgTxt(char * _phoneNum, char * _data,int _dataLen)
{ 
    TraceMsg("send a msg !",1);
    if(_dataLen<=0)
        return -2;
    if(Utility_CheckDigital(_phoneNum,0,10))
        return -3;
    int _retNum;
    int _index=0;
    int _repeats=0;
    int _repeats1=0;
    int _repeats2=0;
    char _send[60];//13607193119,129";
SendMsgTxt:
    Utility_Strncpy(_send,"AT+CMGS=",8);
    //Console_WriteStringln("SIM_SendMsgTxt:Send a Msg");
    //_phoneNum[11]='\0';
    //Console_WriteStringln(phoneNum);
    _index=8;
    _phoneNum="\"18071045881\"";//lmj170814
    /*
    Utility_Strncpy(&_send[_index],_phoneNum,11);//AT指令电话号码需要加双引号，这里没有加入，怎么不会出错?
    _index+=11;
    */
    Utility_Strncpy(&_send[_index],_phoneNum,13);
    _index+=13;

    /*  下面这一段代码不知道是什么意思?在GTM900指令中，没有后面这个129
    _send[_index++]=',';
    _send[_index++]='1';
    _send[_index++]='2';
    _send[_index++]='9'; 
  */
        
    GSM_AT_QuitInput();//避免前一个失败的 > 符号带来的混乱
    GSM_DealAllData();// 发送前,我们需要 清空接收 缓冲区 ,
    UART0_ClearInput();//清 > 符号标记为0
  
    UART0_Send(_send,_index,1);
    //等待 > 提示符号
    if(UART0_WaitInput()==-1)
    {
        ++s_TimeOut;
        if(_repeats>1)//次数超过2次
        {
            TraceMsg("send msg fail 1 !",1);
            return -1;
        }
        ++_repeats;
        //  没等到的话,可能是因为返回其它数据
        GSM_DealAllData();
        //Console_WriteStringln("SIM_SendMsgTxt:WaitInput Fail !");
        //SIM可能傻了.
        //我们让GSM休息休息
        System_Delayms(500);
        goto SendMsgTxt;
    } 
    //Console_WriteStringln("SIM_SendMsgTxt:Got > :");
    //等到了 > 提示符号
    Led3_WARN();
    //发送数据
    _data[_dataLen]=0x1A;//CTRL+Z  
    UART0_Send(_data,_dataLen+1,0);//多发一个0x1A 
    //Console_WriteStringln("SIM_SendMsgTxt:Send data:");
    //Console_WriteStringln(data);
    //   这里我们延时10秒等待短信发送成功.
    //   等待收到+CMGS   以及一个OK. 
    //开始延时.
    //Console_WriteStringln("SIM_SendMsgTxt:Waiting !");
    System_Delayms(10000);
    if(UART0_RecvLineWait(_send,60,& _retNum)==-1)
    {//短信发送 未成功
        ++s_MsgFail;
        GSM_AT_QuitInput();
        if(_repeats1>2)
      {
      //Console_WriteStringln("SIM_SendMsgTxt:Send fail finally !");
          TraceMsg("send msg fail  2 !",1);
      return -1;
      }
      ++_repeats1;
    //Console_WriteStringln("SIM_SendMsgTxt:Send fail,try again !");
      goto SendMsgTxt;
  }
  if(Utility_Strncmp(_send,"+CMGS: ",7)==0)
  {//成功
    //Console_WriteStringln("SIM_SendMsgTxt:Send OK !");
    Led4_WARN();
    if(UART0_RecvLineWait(_send,60,& _retNum)==0)
      GSM_DealData(_send,_retNum);
    //收没收到OK,都无所谓.
    ++s_MsgOK;
    TraceMsg("send msg ok !",1);
    return 0;
  }
  //Console_WriteStringln("SIM_SendMsgTxt:Others data,send fail !");
  //Console_WriteStringln(send);
  GSM_DealData(_send,_retNum);
  ++s_MsgFail;
  if(_repeats2 >2)
  {//避免错误 祸害 后续AT指令
    GSM_AT_QuitInput();
    TraceMsg("send msg fail  3 !",1);
    return -2;
  }
  ++_repeats2;
  goto SendMsgTxt;
} 
 
//  
//  返回值为0,表示该短信分析过了.
//  01234567890123456789012345678901
//  $1234*NO*00011100011,0000,00,00#
//  我们回复的内容都是
//  01234567890123456
//  $00011100011<*****************    $00011100011<UN*FAIL#
//  BS,DL,CP ,TM,PD,PL,CS,PR,WM
int GSM_ProcMsgData(char * _data,char* _rePhone,int _dataLen)
{
    TraceMsg(" start to proc msg data ",1);
    Led2_WARN();
    char _send[120];//给120够了
    char _buffer[20];
    int _ret=0;
    char _tempChar1=0x00;
    char _tempChar2=0x00;
    char _tempChar3=0x00;
    char _tempChar4=0x00;
    int _tempInt=0;
    int _max=0;
    int _min=0;
    int _idx1=0;
    int _idx2=0;
    if(_data[0]!='$' || _data[_dataLen-1]!='#')//不以$开头#结尾就直接忽略
    {
        TraceMsg("wrong format ",1);
        return 0;
    }
    //拼好头部
    _send[0]='$';
    if(Store_ReadDeviceNO(&(_send[1]))<0)
        return -1;
    _send[12]='<';
    //检查*号
    if(_data[5]!='*' || _data[8]!='*')
    {//这个就不处理了.防止因为自己给自己发,导致死循环.
        //_ret=Utility_PackErrorMsg("UN",_send);
        //return GSM_SendMsgTxt(_rePhone,_send,_ret);
        TraceMsg("wrong format ",1);
        return 0;
    }
    //先判断密码是否正确
    if(Store_ReadPassword(_buffer)>=0)
    {
        if(Utility_Strncmp(_buffer,&(_data[1]),4)!=0)
        {
            TraceMsg("wrong password ",1);
            _ret=Utility_PackRejectMsg("UN",_send);
            return GSM_SendMsgTxt(_rePhone,_send,_ret);
        }
    }
    
    //BS,DL,CP ,TM,PD,PL,CS,PR,WM
    switch(_data[6])
    {
      case 'B':
        //          1         2         3 
        //01234567890123456789012345678901
        //$1234*BS*00011100011,0000,00,00#
        if(_data[7]!='S' || _dataLen!=32 || _data[20]!=',' || _data[25]!=',' ||_data[28]!=','
           ||Utility_CheckDigital(_data,9,19) || Utility_CheckDigital(_data,21,24)
               ||Utility_CheckDigital(_data,26,27)|| Utility_CheckDigital(_data,29,30))
        {
            _ret=Utility_PackErrorMsg("UN",_send);
            return GSM_SendMsgTxt(_rePhone,_send,_ret);
        }
        TraceMsg("BS msg ",1);
        if( _data[17] != '0' || _data[18] != '0' || _data[19] != '0' )
        {
            _ret += Store_SetDeviceNO(&(_data[9]));
        }
        if( _data[21] !='0' || _data[21] !='0' || _data[21] !='0' || _data[21] !='0')
        {
            _ret += Store_SetPassword(&(_data[21]));
        }
        if( _data[26] !='0' || _data[27] != '0' )
        {
            _tempInt= (_data[26]-'0')*10 + _data[27]-'0';
            if( _tempInt < 1 || _tempInt >16)
            {
                _ret = Utility_PackBadMsg("BS",_send);
                return GSM_SendMsgTxt(_rePhone,_send,_ret);
            }
            _ret +=Store_SetSaveTimeMode(&(_data[26]));
            //同时修正下一次保存时间 
            Utility_CalculateNextSaveTimeBytes(_buffer);
            RTC_SetSaveTimeBytes(SAVETIME_ADDR,_buffer);
        }
        if( _data[29] != '0' || _data[30] != '0')
        {
            _tempInt = (_data[29]-'0')*10 + _data[30]-'0';
            if( _tempInt <=0 || _tempInt >16)
            {
                _ret = Utility_PackBadMsg("BS",_send);
                return GSM_SendMsgTxt(_rePhone,_send,_ret);
            }
            _ret += Store_SetReportTimeMode(&(_data[29]));
            //同时修正下一次报告时间
            Utility_CalculateNextReportTimeBytes(_buffer);
            RTC_SetReportTimeBytes(REPORTTIME_ADDR,_buffer);
        }
        if(_ret<0)
        {
            _ret = Utility_PackFailMsg("BS",_send);
            return GSM_SendMsgTxt(_rePhone,_send,_ret);
        }
        else
        {
            _ret = Utility_PackOKMsg("BS",_send);
            return GSM_SendMsgTxt(_rePhone,_send,_ret);
        }
      case 'C':
        //012345678901234567890123456789012345678901
        //$1234*CP*1,13607193119# //设置电话
        
        //$1234*CS*A,00000011#或  //设置通道选择
        //$1234*CS*P,11000000#
        if( _data[7]=='P' && _data[10]==',' && (!Utility_CheckDigital(_data,11,21)) )
        {
            TraceMsg("CP msg ",1);
            if(_data[9]< '1' || _data[9] > '6' )
            {
                _ret = Utility_PackBadMsg("CP",_send);
                return GSM_SendMsgTxt(_rePhone, _send, _ret);
            }
            _ret= Store_GSM_SetCenterPhone(_data[9]-'0', &(_data[11]));
            if(_ret<0)
            {
                _ret= Utility_PackFailMsg("CP",_send);
            }
            else
            {
                _ret= Utility_PackOKMsg("CP",_send);
            }
            return GSM_SendMsgTxt(_rePhone,_send,_ret);
        }
        if( _data[7]=='S' && _data[10]==',' && (! Utility_CheckBinary(_data,11,18) ) )
        {
            TraceMsg("CS msg ",1);
            _tempChar1=0x00;
            if( _data[9]!='A' &&_data[9]!='P' &&_data[9]!='I')
            {
                _ret = Utility_PackBadMsg("CS",_send);
                return GSM_SendMsgTxt(_rePhone, _send,_ret);
            }
            if( _data[9]=='A')
            {//模拟量 ,低位在前
                for(int i=11;i<19;++i)
                {//$1234*CS*P,11000000#
                    _tempChar1 >>=1;
                    if(_data[i]!='0')
                        _tempChar1 |= BIT7;
                    else
                        _tempChar1 &= ~BIT7;
                }
                _ret = Store_SetAnalogSelect(_tempChar1);
            }
            if( _data[9]=='P')
            {//脉冲量,前4位,低位在前
                
                for(int i=11; i<15;++i)
                {
                    _tempChar1 <<=1;
                    if(_data[i]!='0')
                        _tempChar1 |= BIT4;
                    else
                        _tempChar1 &= ~BIT4;
                }
                _ret =Store_SetPulseSelect(_tempChar1);
            }
            if( _data[9]=='I')
            {//开关量,低位在前
                for(int i=11; i<19;++i)
                {
                    _tempChar1 >>=1;
                    if(_data[i]!='0')
                        _tempChar1 |= BIT7;
                    else
                        _tempChar1 &= ~BIT7;
                }
                _ret =Store_SetIoSelect(_tempChar1);
            }
            if(_ret <0)
            {
                _ret = Utility_PackFailMsg("CS",_send);
                return GSM_SendMsgTxt(_rePhone,_send,_ret);
            }
            else
            {
                _ret=Utility_PackOKMsg("CS",_send);
                return GSM_SendMsgTxt(_rePhone,_send,_ret);
            }
        }
        TraceMsg("error msg ",1);
        //都不符合 那就是错误了
        _ret =Utility_PackErrorMsg("UN",_send);
        return GSM_SendMsgTxt(_rePhone, _send,_ret);
      case 'D':
        //012345678901234567890
        //$1234*DL*A,0000,4096#
        if( _data[7]!='L' || _data[10]!=',' || _data[15]!=',' || Utility_CheckDigital(_data,11,14)|| Utility_CheckDigital(_data,16,19))
        {
            _ret=Utility_PackErrorMsg("UN",_send);
            return GSM_SendMsgTxt(_rePhone,_send,_ret);
        }
        TraceMsg("DL msg ",1);
        if( _data[9] < 'A' || _data[9] > 'H'  )
        {
            _ret = Utility_PackBadMsg("DL",_send);
            return GSM_SendMsgTxt(_rePhone,_send,_ret);
        }
        
        //需要检查max是否大于min
        _min=0; _max=0; _idx1=_data[9]-'A'+1;
        if(_data[11]!='9' ||_data[12]!='9' ||_data[13]!='9' ||_data[14]!='9')
        {
            _min += (_data[11]-'0')*1000;
            _min += (_data[12]-'0')*100;
            _min += (_data[13]-'0')*10;
            _min += (_data[14]-'0');
        }
        else
        {//如果不对这个值设置,我们就取保存值
            if(Store_ReadDataMinInt(_idx1,&_min)<0) 
                _min=0;//读不出来就算了.不检查max>min了
        }
        
        if(_data[16]!='9' ||_data[17]!='9' ||_data[18]!='9' ||_data[19]!='9')
        {
            _max += (_data[16]-'0')*1000;
            _max += (_data[17]-'0')*100;
            _max += (_data[18]-'0')*10;
            _max += (_data[19]-'0');
        }
        else
        {//如果不对这个值设置,我们就取保存值
            if(Store_ReadDataMaxInt(_idx1,&_max)<0) 
                _max=4096;//读不出来就算了.不检查max>min了
        }
        if(_min > _max)
        {
            _ret = Utility_PackBadMsg("DL",_send);
            return GSM_SendMsgTxt(_rePhone,_send,_ret);
        }
        //写回rom
        _ret += Store_SetDataMaxInt(_idx1,_max);
        _ret += Store_SetDataMinInt(_idx1,_min);
        if(_ret<0)
        {
            _ret = Utility_PackFailMsg("DL",_send);
            return GSM_SendMsgTxt(_rePhone,_send,_ret);
        }
        else
        {   //          1         2         3  
            //0123456789012345678901234567890
            //$00011100011<DL*OK*1,0000,4096#
            
            s_alert_flag[_idx1-1]=0;//报警标识要去掉
            
            
            //从下标13的类型码开始填写.
            _send[13]='D';_send[14]='L';_send[15]='*';
            _send[16]='O';_send[17]='K';_send[18]='*';
            _send[19]='A'+_idx1-1;_send[20]=',';
            if(Store_ReadDataMinStr(_idx1,&_send[21])<0)
            {
                _ret = Utility_PackFailMsg("DL",_send);
                return GSM_SendMsgTxt(_rePhone,_send,_ret);
            }
            _send[25]=',';
            if(Store_ReadDataMaxStr(_idx1,&_send[26])<0)
            {
                _ret = Utility_PackFailMsg("DL",_send);
                return GSM_SendMsgTxt(_rePhone,_send,_ret);
            }
            _send[30]='#';//长度为31,可以发送了.
            return GSM_SendMsgTxt(_rePhone,_send,31);
        }
      case 'P':
        //01234567890123456789
        //$1234*PD*00001111#
        //$1234*PL*M,0#
        //$1234*PR*J,100#
        //$1234*PM*K,6#
        //$1234*PV*L,1234567#
        if( (_data[7]!='L'&&_data[7]!='R'&& _data[7]!='M' && _data[7]!='V' && _data[7]!='D') || _data[10]!=',')
        {
            _ret = Utility_PackErrorMsg("UN",_send);
            return GSM_SendMsgTxt(_rePhone,_send,_ret);
        }
        if(_data[7]=='D')
        {
            TraceMsg("PD msg " ,1);
            if(Utility_CheckBinary(_data,9,16))
            {
                _ret = Utility_PackBadMsg("PD",_send);
                return GSM_SendMsgTxt(_rePhone,_send,_ret);
            }
            _tempChar1=0x00;
            for(int i=0; i<8;++i)
            {
                _tempChar1>>=1;
                if(_data[9+i]=='0')
                {
                    _tempChar1 &= 0xEF;//最高位清0
                }
                else
                {
                    _tempChar1 |= 0x80;//最高位置1
                }
            }
            _ret = Store_SetIoDirConfig(_tempChar1);
            if(_ret<0)
            {
                _ret=Utility_PackFailMsg("PD",_send);
                return GSM_SendMsgTxt(_rePhone,_send,_ret);
            }
            else
            {
                _ret=Utility_PackOKMsg("PD",_send);
                return GSM_SendMsgTxt(_rePhone,_send,_ret);
            }
        }
        if(_data[7]=='L')
        {
            TraceMsg("PL msg ",1);
            if( _data[9]< 'M' || _data[9]>'T' || (_data[11]!='1' && _data[11]!='0') )
            {
                _ret = Utility_PackBadMsg("PL",_send);
                return GSM_SendMsgTxt(_rePhone,_send,_ret);
            }
            _idx1 = _data[9]-'M'+1;
            _tempChar1= _data[11]-'0';
            _ret = Sampler_IO_Level(_idx1,_tempChar1);
            if( _ret < 0 )
            {
                if(_ret== -2)
                    _ret = Utility_PackBadMsg("PL",_send);
                else
                    _ret = Utility_PackFailMsg("PL",_send);
                return GSM_SendMsgTxt(_rePhone,_send,_ret);
            }
            else
            {
                _ret = Utility_PackOKMsg("PL",_send);
                return GSM_SendMsgTxt(_rePhone,_send,_ret);
            }
        }
        //0123456789012345
        //$1234*PR*I,100#
        if(_data[7]=='R')
        {
            if( _data[9] < 'I' || _data[9] >'L'|| Utility_CheckDigital(_data,11,13) )
            {
                _ret = Utility_PackBadMsg("PR",_send);
                return GSM_SendMsgTxt(_rePhone,_send,_ret);
            }
            TraceMsg("PR msg ",1);
            _idx1=_data[9]-'I'+1;
            //  _tempChar1 为数值
            _tempInt = (_data[11]-'0')*100;
            _tempInt += (_data[12]-'0')*10;
            _tempInt += (_data[13]-'0');
            if(_tempInt>255 || _tempInt==0)
            {
                _ret= Utility_PackBadMsg("PR",_send);
                return GSM_SendMsgTxt(_rePhone,_send,_ret);
            } 
            
            _ret += Store_SetPulseRate(_idx1 , _tempInt);
            
            if(_ret<0)
            {
                _ret= Utility_PackFailMsg("PR",_send);
                return GSM_SendMsgTxt(_rePhone,_send,_ret);
            }
            else
            {
                //修改内存值
                g_pulse_rate[_idx1-1] = _tempInt;
            
                _ret= Utility_PackOKMsg("PR",_send);
                return GSM_SendMsgTxt(_rePhone,_send,_ret);
            }
        }
        //01234567890123
        //$1234*PM*I,6#
        if(_data[7]=='M') 
        {
            if( _data[9] <  'I' || _data[9] >'L'|| _data[11] < '1' ||_data[11] > '7' )
            {
                _ret = Utility_PackBadMsg("PR",_send);
                return GSM_SendMsgTxt(_rePhone,_send,_ret);
            }
            TraceMsg("PM msg ",1);
            _idx1= _data[9]-'I'+1;
            _ret += Store_SetPulseRange(_idx1,_data[11]-'0'); 
            
            if(_ret<0)
            {
                _ret = Utility_PackFailMsg("PM",_send);
                return GSM_SendMsgTxt(_rePhone,_send,_ret);
            }
            else
            {
                //再修改内存值
                _ret += Store_ReadPulseRangeBytes(_idx1,g_pulse_range[_idx1-1]);
                _ret =Utility_PackOKMsg("PM",_send);
                return GSM_SendMsgTxt(_rePhone,_send,_ret);
            }
        }
        //01234567890123456789
        //$1234*PV*I,1234567#
        //  PV*OK#
        if(_data[7]=='V')
        {
            if(_data[9] <'I' || _data[9] >'L' ||  Utility_CheckDigital(_data,11,17))
            {
                _ret = Utility_PackBadMsg("PV",_send);
                return GSM_SendMsgTxt(_rePhone,_send,_ret);
            }
            TraceMsg("PV msg ",1);
            _idx1 = _data[9]-'I'+1; 
            //1234567 ->  3 byte
            Utility_DecStr7ToBytes3(&_data[11],_buffer);
            
            //设的值如果超过最大值,将返回BAD短信.
            if(Utility_BytesCompare3(_buffer,g_pulse_range[_idx1-1])==1)
            {
                _ret =Utility_PackBadMsg("PV",_send);
                return GSM_SendMsgTxt(_rePhone,_send,_ret);
            }
            RTC_SetPulseBytes(_idx1,_buffer);
            _ret = Utility_PackOKMsg("PV",_send);
            return GSM_SendMsgTxt(_rePhone,_send,_ret); 
        }
        break;
        //012345678901234567890
        //$1234*TM*0908201030#
      case 'T':
        if(_data[7]!='M')
        { 
            _ret= Utility_PackErrorMsg("UN",_send);
            return GSM_SendMsgTxt(_rePhone,_send,_ret);
        }
        TraceMsg("TM msg ",1);
        if(Utility_CheckDigital(_data,9,18) < 0 )
        {
            _ret = Utility_PackBadMsg("TM",_send);
            return GSM_SendMsgTxt(_rePhone,_send,_ret);
        }
        RTC_SetTimeStr5_B(&(_data[9]));
        RTC_ReadTimeBytes5(g_rtc_nowTime); //重新读出时间
        
        //重新设置检查时间
        Utility_CalculateNextCheckTimeBytes(_buffer);
        RTC_SetCheckTimeBytes(CHECKTIME_ADDR,_buffer);
        //重新设置保存时间
        Utility_CalculateNextSaveTimeBytes(_buffer);
        RTC_SetSaveTimeBytes(SAVETIME_ADDR,_buffer);
        //重新设置报告时间
        Utility_CalculateNextReportTimeBytes(_buffer);
        RTC_SetReportTimeBytes(REPORTTIME_ADDR,_buffer);
        
        _ret=Utility_PackOKMsg("TM",_send);
        main_time_error =0;
        return GSM_SendMsgTxt(_rePhone,_send,_ret);
        //01234567890
        //$1234*WM*0#
      case 'W':
        if(_data[7]!='M')
        {
            _ret= Utility_PackErrorMsg("UN",_send);
            return GSM_SendMsgTxt(_rePhone,_send,_ret);
        }
        TraceMsg("WM msg ",1);
        if(_data[9]!= '0' &&_data[9]!='1')
        {
            _ret= Utility_PackBadMsg("WM",_send);
            return GSM_SendMsgTxt(_rePhone,_send,_ret);
        }
        _ret += Store_SetWorkMode(_data[9]);//WorkMode暂时是字符.以后再仔细修正为数值.
        if(_ret<0)
        {
            _ret =Utility_PackFailMsg("WM",_send);
            return GSM_SendMsgTxt(_rePhone,_send,_ret);
        }
        else
        {//成功切换模式了 就重启
            _ret =Utility_PackOKMsg("WM",_send);
            GSM_SendMsgTxt(_rePhone,_send,_ret);
            //这条短信 没被删除 就直接重启了. 所以会被回复2次.这个暂时不好改.
            if( g_work_mode != (_data[9]-'0') )
            {//如果模式不一样, 那么就重启了.
                // 这里如果直接重启 会出现 WM短信导致一直重启的问题.
                //所以改成模式不一样才重启.
                GSM_Close(0);//关闭GSM
                System_Delayms(1000);
                System_Reset();
            }
            return 0;
        }
        //
        //  下面是报告模式
        //BS,DL,CP,TM,PL,CS,PR,PM,WM
        //$密码*RP*次类型码#
        //
        //012345678901
        //$1234*RP*TM#
        //
      case 'R':
        if(_data[7]=='P' && _data[8] == '*')
        {
            if(_data[9]=='B' && _data[10]=='S')
            {   //012345678901234567890123456789
                //$00011100011<BS*0000,00,00#
                TraceMsg("RP_BS msg ",1);
                _send[13]='B';_send[14]='S';_send[15]='*';
                _ret += Store_ReadPassword( & (_send[16]) );
                _send[20]=',';
                _ret += Store_ReadSaveTimeMode(& (_send[21]));
                _send[23]=',';
                _ret += Store_ReadReportTimeMode( & (_send[24]) );
                _send[26]='#';
                if(_ret<0)
                {
                    _ret=Utility_PackFailMsg("RP",_send);
                    return GSM_SendMsgTxt(_rePhone,_send,_ret);
                }
                else
                {
                    return GSM_SendMsgTxt(_rePhone,_send, 27);
                }
            }
            if(_data[9]=='D' && _data[10]=='L')
            {   //0123456789012345678901234567890123
                //$00011100011<DL*0000,4096,0000,4096,.....
                TraceMsg("RP_DL msg ",1);
                _send[13]='D';_send[14]='L';_send[15]='*';
                _idx1 = 16;
                for(int i=1; i<= 8;++i)
                {
                    _ret += Store_ReadDataMinStr(i ,& (_send[_idx1]) );
                    _idx1 +=4;
                    _send[_idx1++]=',';
                    _ret += Store_ReadDataMaxStr(i ,& (_send[_idx1]) );
                    _idx1 +=4;
                    _send[_idx1++]=',';
                } 
                _send[_idx1-1]='#';//最后一个多一个','号
                if(_ret<0)
                {
                    _ret= Utility_PackFailMsg("RP",_send);
                    return GSM_SendMsgTxt(_rePhone,_send,_ret);
                }
                else
                {
                    return GSM_SendMsgTxt(_rePhone,_send,_idx1);
                }
            }
            if(_data[9]=='C' && _data[10]=='P')
            {   //012345678901234567890123456789
                //$00011100011<CP*13607193119,13607193119,136....
                TraceMsg("RP_CP msg ",1);
                _send[13]='C';_send[14]='P';_send[15]='*';
                _idx1=16;
                for(int i=1;i<=6; ++i)
                {
                    _ret += Store_GSM_ReadCenterPhone(i, &(_send[_idx1]));
                    _idx1 +=11;
                    _send[_idx1 ++]=','; 
                }
                _send[_idx1-1]='#';
                if(_ret<0)
                {
                    _ret =Utility_PackFailMsg("RP",_send);
                    return GSM_SendMsgTxt(_rePhone,_send,_ret);
                }
                else
                {
                    return GSM_SendMsgTxt(_rePhone,_send,_idx1);
                }
            }
            if(_data[9]=='T' && _data[10]=='M')
            {   //01234567890123456789012345678
                //$00011100011<TM*090604120032#
                TraceMsg("RP_TM msg ",1);
                _send[13]='T';_send[14]='M';_send[15]='*';
                _idx1=16;
                RTC_ReadTimeStr6_B( &(_send[_idx1]) );
                _idx1 += 12;
                _send[_idx1++ ]='#';
                return GSM_SendMsgTxt(_rePhone,_send,_idx1);
            }
            if(_data[9]=='P' && _data[10]=='L')
            {   //0123456789012345678901234
                //$00011100011<PL*00110011#
                TraceMsg("RP_PL msg ",1);
                _send[13]='P';_send[14]='L';_send[15]='*';
                _idx1=16;
                _ret=Store_ReadIoLevelConfig(&_tempChar1);
                
                if(_ret<0)
                {
                    _ret=Utility_PackFailMsg("RP",_send);
                    return GSM_SendMsgTxt(_rePhone,_send,_ret);
                }
                
                P5OUT = _tempChar1; //再更新一下,避免不一致.
                
                if(_tempChar1 & BIT0)
                    _send[_idx1++]='1';
                else
                    _send[_idx1++]='0';
                if(_tempChar1 & BIT1)
                    _send[_idx1++]='1';
                else
                    _send[_idx1++]='0';
                if(_tempChar1 & BIT2)
                    _send[_idx1++]='1';
                else
                    _send[_idx1++]='0';
                if(_tempChar1 & BIT3)
                    _send[_idx1++]='1';
                else
                    _send[_idx1++]='0';
                if(_tempChar1 & BIT4)
                    _send[_idx1++]='1';
                else
                    _send[_idx1++]='0';
                if(_tempChar1 & BIT5)
                    _send[_idx1++]='1';
                else
                    _send[_idx1++]='0';
                if(_tempChar1 & BIT6)
                    _send[_idx1++]='1';
                else
                    _send[_idx1++]='0';
                if(_tempChar1 & BIT7)
                    _send[_idx1++]='1';
                else
                    _send[_idx1++]='0';
                //
                _send[_idx1++]='#';
                return GSM_SendMsgTxt(_rePhone,_send,_idx1); 
            }
            if(_data[9]=='P' && _data[10]=='D')
            {
                TraceMsg("RP_PD msg ",1);
                _send[13]='P';_send[14]='D';_send[15]='*';
                _idx1=16;
                _ret=Store_ReadIoDirConfig(&_tempChar1);
                
                if(_ret<0)
                {
                    _ret=Utility_PackFailMsg("RP",_send);
                    return GSM_SendMsgTxt(_rePhone,_send,_ret);
                }
                
                P5DIR = _tempChar1; //再更新一下,避免不一致.
                
                if(_tempChar1 & BIT0)
                    _send[_idx1++]='1';
                else
                    _send[_idx1++]='0';
                if(_tempChar1 & BIT1)
                    _send[_idx1++]='1';
                else
                    _send[_idx1++]='0';
                if(_tempChar1 & BIT2)
                    _send[_idx1++]='1';
                else
                    _send[_idx1++]='0';
                if(_tempChar1 & BIT3)
                    _send[_idx1++]='1';
                else
                    _send[_idx1++]='0';
                if(_tempChar1 & BIT4)
                    _send[_idx1++]='1';
                else
                    _send[_idx1++]='0';
                if(_tempChar1 & BIT5)
                    _send[_idx1++]='1';
                else
                    _send[_idx1++]='0';
                if(_tempChar1 & BIT6)
                    _send[_idx1++]='1';
                else
                    _send[_idx1++]='0';
                if(_tempChar1 & BIT7)
                    _send[_idx1++]='1';
                else
                    _send[_idx1++]='0';
                //
                _send[_idx1++]='#';
                return GSM_SendMsgTxt(_rePhone,_send,_idx1); 
            }
            if(_data[9]=='C' && _data[10]=='S')
            {   //          1         2         3         4   
                //0123456789012345678901234567890123456789012
                //$00011100011<CS*00110011,01010000,00110011#
                TraceMsg("RP_CS msg ",1);
                _send[13]='C';_send[14]='S';_send[15]='*';
                _idx1=16;
                _idx2=34;
                _ret+=Store_ReadAnalogSelect(&_tempChar1);
                _ret+=Store_ReadPulseSelect(&_tempChar2);
                _ret+=Store_ReadIoSelect(&_tempChar3);
                if(_ret<0)
                {
                    _ret =Utility_PackFailMsg("RP",_send);
                    return GSM_SendMsgTxt(_rePhone,_send,_ret);
                }
                _tempChar4=0x01;
                for(int i=0;i<8;++i)
                {
                    if(_tempChar1 & _tempChar4) 
                        _send[_idx1++]='1';
                    else
                        _send[_idx1++]='0';
                    
                    if(_tempChar3&_tempChar4)
                        _send[_idx2++]='1';
                    else
                        _send[_idx2++]='0';
                    _tempChar4 <<=1;
                }
                if(_tempChar2 & BIT7)
                    _send[25]='1';
                else
                    _send[25]='0'; 
                if(_tempChar2 & BIT6)
                    _send[26]='1';
                else
                    _send[26]='0'; 
                if(_tempChar2 & BIT5)
                    _send[27]='1';
                else
                    _send[27]='0';
                if(_tempChar2 & BIT4)
                    _send[28]='1';
                else
                    _send[28]='0';
                _send[29]='0';_send[30]='0';_send[31]='0';_send[32]='0';
                
                //填写,和其他字符
                _send[24]=',';_send[33]=',';  
                _send[42]='#';
                return GSM_SendMsgTxt(_rePhone,_send,43);
            }
            
            if(_data[9]=='P' && _data[10]=='R')
            {   //012345678901234567890123456789012
                //$00011100011<PR*100,010,200,100#
                TraceMsg("RP_PR msg ",1);
                _send[13]='P';_send[14]='R';_send[15]='*';
                _idx1=16;
                for(int i=1;i<=4;++i)
                {
                    _ret += Store_ReadPulseRate(i,& _tempChar1);
                    //设置到了ROM中,还应设置内存.
                    Store_ReadPulseRate(i,&(g_pulse_rate[i-1]));
                    _send[_idx1++]=_tempChar1/100+'0';
                    _tempChar1 %=100;
                    _send[_idx1++]=_tempChar1/10+'0';
                    _send[_idx1++]=_tempChar1%10+'0';
                    _send[_idx1++]=',';
                }
                _send[_idx1-1]='#';
                if(_ret<0)
                {
                    _ret=Utility_PackFailMsg("RP",_send);
                    return GSM_SendMsgTxt(_rePhone,_send,_ret);
                }
                else
                    return GSM_SendMsgTxt(_rePhone,_send,_idx1);
            }
            if(_data[9]=='P' && _data[10]=='M')
            {   //0123456789012345678901234
                //$00011100011<PM*5,5,6,7#
                TraceMsg("RP_PM msg ",1);
                _send[13]='P';_send[14]='M';_send[15]='*';
                _idx1 = 16;
                for(int i=1;i<=4;++i)
                {
                    _ret +=Store_ReadPulseRange(i,&_tempChar1);
                    _send[_idx1++] = _tempChar1 + '0';
                    _send[_idx1++] =',';
                }
                _send[_idx1-1]='#';
                if(_ret<0)
                {
                    _ret=Utility_PackFailMsg("RP",_send);
                    return GSM_SendMsgTxt(_rePhone,_send,_ret);
                }
                else
                    return GSM_SendMsgTxt(_rePhone,_send,_idx1);
                
            }
            if(_data[9]=='W' && _data[10]=='M')
            {   //012345678901234567890
                //$00011100011<WM*0#
                TraceMsg("RP_WM msg ",1);
                _send[13]='W';_send[14]='M';_send[15]='*';
                _ret = Store_ReadWorkMode(&_tempChar1);
                if(_ret < 0 ) 
                {
                    _ret=Utility_PackFailMsg("RP",_send); 
                    return GSM_SendMsgTxt(_rePhone,_send,_ret);
                }
                _send[16]=_tempChar1;
                _send[17]='#';
                return GSM_SendMsgTxt(_rePhone,_send, 18);
            }
            if(_data[9]=='N' && _data[10]=='T')
            {   //0         1         2         3         4         5         6 
                //0123456789012345678901234567890123456789012345678901234567890
                //$00011100011<NT*09/06/04/12:00,09/06/04/12:00,09/06/04/12:00#
                TraceMsg("RP_NT msg ",1);
                _send[13]='N';_send[14]='T';_send[15]='*';
                _ret += RTC_ReadCheckTimeStr5_A(&(_send[16]));
                _send[30]=',';
                _ret += RTC_ReadSaveTimeStr5_A(&(_send[31]));
                _send[45]=',';
                _ret += RTC_ReadReportTimeStr5_A(&(_send[46]));
                if(_ret <0)
                {
                    _ret = Utility_PackFailMsg("RP",_send);
                    return GSM_SendMsgTxt(_rePhone,_send,_ret);
                }
                
                _send[60]='#';
                return GSM_SendMsgTxt(_rePhone,_send,61);
            }
            if(_data[9]=='P' &&_data[10]=='V')
            {   //0         1         2         3         4      
                //01234567890123456789012345678901234567890123456789
                //$00011100011<PV*1234567,1234567,1234567,1234567#
                TraceMsg("RP_PV msg ",1);
                _send[13]='P';_send[14]='V';_send[15]='*';
                _ret += RTC_ReadPulseBytes(1,_buffer);
                Utility_Bytes3ToDecStr7(_buffer,&_send[16]);
                _send[23]=',';
                _ret += RTC_ReadPulseBytes(2,_buffer);
                Utility_Bytes3ToDecStr7(_buffer,&_send[24]);
                _ret += RTC_ReadPulseBytes(3,_buffer);
                _send[31]=',';
                Utility_Bytes3ToDecStr7(_buffer,&_send[30]);
                _ret += RTC_ReadPulseBytes(4,_buffer);
                _send[39]=',';
                Utility_Bytes3ToDecStr7(_buffer,&_send[40]);
                _send[47]='#';
                if(_ret<0)
                {
                    _ret = Utility_PackFailMsg("PV",_send);
                    return GSM_SendMsgTxt(_rePhone,_send,_ret);
                }
                else
                {
                    return GSM_SendMsgTxt(_rePhone,_send,48);
                }
            }
        }
        break;
      default:
        _ret = Utility_PackErrorMsg("UN",_send);
        return GSM_SendMsgTxt(_rePhone,_send,_ret);
    }
    return 0;
}

int  GSM_GetMsgStore(char *_recv)
{
  //+CPMS: "MT",*,**,"MT",*,**,"MT",*,**//我们只读第一个
  if(_recv[13]!=',')
  {//短信数目有2 位
    s_MsgLeft=(_recv[12]-'0')*10+(_recv[13]-'0');
  }
  else
    s_MsgLeft=(_recv[12]-'0');
  TraceMsg("Find Msg :",0);
  TraceInt4(s_MsgLeft,1);
  return s_MsgLeft;
}

int GSM_GetRePhone(char *src, char *dest)
{// +CMGR: "REC UNREAD","+8613607193119",,"09/02/27,16:36:16+32"格式 
    int index=0;
    while(src[index++]!=',');//第一个,号之后就是回复电话 
    ++index;//index指向"号
    if(src[index]=='+')
    {//有+86,需要再跳过3个字符
        index+=3;
    }
    else//没有+,GTM900格式，
    {
        index+=2;
    }
    //现在指向号码了,开始复制,复制11位
    dest[0]=src[index++];
    dest[1]=src[index++];
    dest[2]=src[index++];
    dest[3]=src[index++];
    dest[4]=src[index++];
    dest[5]=src[index++];
    dest[6]=src[index++];
    dest[7]=src[index++];
    dest[8]=src[index++];
    dest[9]=src[index++];
    dest[10]=src[index];
    dest[11]='\0';
    return 0;
}

//对方如果接听,我们就立即挂断.
//响一会后,我们也挂断
int GSM_CallLeader(char * _phone)
{
    //Console_WriteStringln("Call Leader !");
    if(GSM_CheckLive()<0)
    {
        GSM_Open();
        if(GSM_CheckOK()<0)
        {
            return -1;
        }
    }
    int _retNum;
    char _recv[UART0_MAXBUFFLEN];
    char _call[18]="ATD";
    Utility_Strncpy(&_call[3],_phone,11);
    _call[14]=';';
    // 发送AT指令开始拨打
    // 实际测试 约6~8秒 对方手机才有反应. 
    UART0_Send(_call,15,1);
    System_Delayms(5000);
    
    //我们这里不进行任何其他事情,
    //一直延时,直到时间足够
    //共延时 15秒
    //  先延时 3秒
    System_Delayms(30000);
    for(int i=0;i<5;++i)
    {
        System_Delayms(1000);
        if(UART0_RecvLineWait(_recv,UART0_MAXBUFFLEN,&_retNum)!=-1)//当前版本超时为100ms
        {
            if(Utility_Strncmp(_recv,"OK",2)==0)
            {//负责人接起电话了. 应该挂断
                GSM_AT_OffCall();
            }
            //如果负责人正在打电话 会有 No Carrier
            //如果负责人挂断,  模块竟然 没有反应.. 郁闷..不管了.反正最多15秒左右.
            //但是没信号的话, 可能也是这个返回... 
            if(Utility_Strncmp(_recv,"NO CARRIER",10)==0)
            {//我们就不管了.按道理,在这个函数运行前,已经有短信发送成功.
             //所以不太可能是,SIM模块没准备好 的 NO CARRIER
             //这样处理还是 比较安全的
                GSM_AT_OffCall();
                return 0;
            }
            //如果是其他数据,接收下,可能是短信到来
            GSM_DealData(_recv,_retNum);
        }
        System_Delayms(1000);
    }
    //延时够了.我自己挂掉.
    GSM_AT_OffCall(); 
    return 0;
} 
//   对于 电话 处理并返回 1
//   表示 有短信.  返回2 
//   对于  OK      , 返回  0
//   对于  ERROR   , 返回  -1 
//   对于  其它数据,  返回 -2
//   对于未预料到的数据,返回 -3
//   未处理的响应型数据将被计数
int GSM_DealData(char *_recv, int _dataLen)
{   
  //
  //  主动型数据
  //
    if(Utility_Strncmp(_recv,"RING",4)==0)
    {//电话  
        ++s_RING;
        TraceMsg(" RING !",1);
        //Console_WriteStringln("SIM_Deal:RING !");
        GSM_AT_OffCall();
        return 2;
    }
    if(Utility_Strncmp(_recv,"+CMTI: ",7)==0)
    {  
         Led_WARN1(); 
        TraceMsg("Got A Msg !",1);
        ++s_MsgNum;//存放 
        if(_recv[13]=='\0')//recvLine函数将末尾处理为'\0'
            s_MsgArray[s_RecvIdx]=(_recv[12]-'0');
        else
            s_MsgArray[s_RecvIdx]=(_recv[12]-'0')*10 + _recv[13]-'0'; 
        
        if(s_RecvIdx<ARRAY_MAX-1) 
            ++s_RecvIdx;//增加索引,ArrayIdx指向的是第一个无效位
        else
            s_RecvIdx=0;
        //++Deal_Msg;
        return 2;
     }
     if(Utility_Strncmp(_recv,"+CMGS: ",7)==0)
     { 
         return 1; 
     }
     //
     //  响应型数据
     //
     //按最可能收到 以及 处理优先级的 顺序排列 
     if(Utility_Strncmp(_recv,"OK",2)==0)
     { 
         return 0;
     }
    //其他的都不管了.
     return -1;
}
// 专门用来清空缓冲区;
// 对每行数据调用SIM_Deal
int GSM_DealAllData()
{
    int _retNum; 
    char _recv[UART0_MAXBUFFLEN];
    while(UART0_RecvLineTry(_recv,UART0_MAXBUFFLEN,&_retNum)==0)
    {
        if(GSM_DealData(_recv,_retNum)==2)
        {
            return 2;
        }
    }
    return 0;
}
int  GSM_AT_QueryStore()
{  
  GSM_DealAllData();
  int _retNum;
  int _repeats=0;
  int _ret;
  char _recv[60]="AT+CPMS?";
  //这条指令会收到:
  //+CPMS: "MT",*,**,"MT",*,**,"MT",*,**
  //OK
QueryStore: 
  UART0_Send(_recv,8,1); 
  _ret=UART0_RecvLineLongWait(_recv,60,&_retNum);
  if(_ret<0)
  {
      ++s_TimeOut;
      //Console_WriteStringln("SIM_AT_QueryStore: timeOut");  
      return -1;
  }
  if(Utility_Strncmp(_recv,"+CPMS: ",7)==0)
  {
      GSM_GetMsgStore(_recv);
      //收个OK
      if(UART0_RecvLineWait(_recv,60,&_retNum)==-1)
      { ++s_TimeOut; return 1;}
      if(Utility_Strncmp(_recv,"OK",2)==0)
        return 0;
      else
      {
         GSM_DealData(_recv,_retNum);
         return 0;
      }
  } 
  GSM_DealData(_recv,_retNum);
  if(_repeats>2)
  {
    return -2;
  }
  ++ _repeats;
  goto QueryStore;
} 


//lmj 20170904添加，查询卡状态，卡是否坏掉
int  GSM_AT_QuerySim()
{
    int _retNum;
    char _recv[30]="AT%TSIM";
    //该AT指令会返回:
    //+CREG: 0,1
    //OK
    UART0_Send(_recv,7,1);
    if(UART0_RecvLineWait(_recv,60,&_retNum)==-1)
    { 
        ++s_TimeOut;
        return -1;
    }
    if(Utility_Strncmp(_recv,"%TSIM 1",7)==0)
    {
        if(UART0_RecvLineWait(_recv,30,&_retNum)==-1)
        {
            ++s_TimeOut; 
            return -1;
        }
        GSM_DealData(_recv,_retNum);
        TraceMsg("SIM Card OK",1);
        return 0;  
    }else
    {
        if(UART0_RecvLineWait(_recv,30,&_retNum)==-1)
        {
            ++s_TimeOut; 
            return -1;
        }
        GSM_DealData(_recv,_retNum);
        Console_WriteStringln("SIM Card Error, Please Insert or replace a New Sim Car");
        return -1;
    }
    
  //  return -2;
}


int  GSM_AT_QueryNet()
{ 
  int _retNum;
  char _recv[30]="AT+CREG?";
  //该AT指令会返回:
  //+CREG: 0,1
  //OK
  UART0_Send(_recv,8,1);
  if(UART0_RecvLineWait(_recv,60,&_retNum)==-1)
  { 
    ++s_TimeOut;
    return -1;
  }
  if(Utility_Strncmp(_recv,"+CREG: ",7)==0)
  {//获得联网状态
    s_NetState=_recv[9];
    if(UART0_RecvLineWait(_recv,30,&_retNum)==-1)
    {
      ++s_TimeOut; 
      return -1;
    }
    if(Utility_Strncmp(_recv,"OK",2)==0)
    {
      	return 0;  
    }
  }
  GSM_DealData(_recv,_retNum);
  return -2; 
} 
int  GSM_AT_SetMsgmode(int _mode)
{ 
    GSM_DealAllData();
    int _retNum; 
    char _recv[30]="AT+CMGF=1";
    //该指令返回:
    //OK 
    if(_mode==0)
        _recv[8]='0';
    else
        _recv[8]='1';  
    UART0_Send(_recv,9,1);
    if(UART0_RecvLineWait(_recv,30,&_retNum)==-1)
    {  
        System_Delayms(100);//该指令需要较多的延迟
        ++s_TimeOut;  
        return -1;
    }
    if(Utility_Strncmp(_recv,"OK",2)==0)
    {
    	Console_WriteStringln("SetMsgmode success");
        return 0;
    }
    if(Utility_Strncmp(_recv,"ERROR",5)==0)
    {
       Console_WriteStringln("SetMsgmode failed");
        //++SIM_ErrorNum;
        return -2;
    }
    System_Delayms(200);//该指令需要较多的延迟
    //++SIM_BadNum; 
    GSM_DealData(_recv,_retNum);
    return -2;
}

int  GSM_AT_CloseFeedback()
{  //这条指令会收到:
  //ATE0 
  //OK
    int _retNum;
    int _repeats=0;
    char _recv[30]="ATE0";  
CloseFeedback:  
    //UART0_Send(_recv,4,1); lmj20170814
    UART0_Send(_recv,4,1);//lmj0814不用UART0发送里面的补充回车换行，直接将回车换行放在指令字符串当中
//    System_Delayms(50);
    if(UART0_RecvLineWait(_recv,30,&_retNum)==-1)
    {  
        ++s_TimeOut; 
        return -1; 
    } 
      //收到ERROR ,
    if(Utility_Strncmp(_recv,"ERROR",5)==0)
    {
        if(_repeats>2)
        {
        	Console_WriteStringln("Close feedback failed");
            return -2;
        }
        //++SIM_ErrorNum;
        ++ _repeats;
        goto CloseFeedback;
    } 
    //如果是ATE0,则再获得一个OK,就可以返回了
    if(Utility_Strncmp(_recv,"ATE0",4)==0)
    {
        if(UART0_RecvLineWait(_recv,30,&_retNum)==-1)
        { ++s_TimeOut; return -1; }
        if(Utility_Strncmp(_recv,"OK",2)==0)
        {
        	TraceMsg("Close feedback success", 1);
            return 0;  
        }
    //++SIM_ErrorNum;
        return -2;
    }
    //如果是OK,说明已经关闭
    if(Utility_Strncmp(_recv,"OK",2)==0)
        return 0;
    GSM_DealData(_recv,_retNum);
    //++SIM_BadNum;
    return -2;
}

int GSM_AT_SetMsgNotify()
{
    int _retNum;
    char _recv[100]="AT+CNMI=2,1,0,0,1";
    //char _recv[100]="AT+CNMI=2,1";    
    int _repeats=0;
  CNMI:
    UART0_Send(_recv,17,1); 
    if(UART0_RecvLineWait(_recv,60,&_retNum)==-1)
    { 
        ++s_TimeOut;
        return -1;
    }
    if(Utility_Strncmp(_recv,"OK",2)==0)
    {	
    	Console_WriteStringln("SetMsgNotify success");
        return 0;
    }
    else
    {
        if(_repeats>3)
        {
        	Console_WriteStringln("SetMsgNotify failed");
            return -1;
        }
        ++_repeats;
        goto CNMI;
    }
} 
int  GSM_AT_DeleteMsg(int _index)
{ 
    GSM_DealAllData();
    int _retNum;
    int _len=0;
    //Console_WriteStringln("SIM_AT_DeleteMsg:Delete:");
    //Console_WriteIntln(index); 
    int repeats=0;
    int repeats1=0;
    if(_index > 90 || _index < 1)
        return -3;
    char _recv[60];
    char _delete_msg[11]="AT+CMGD=";
    //该指令只会返回:
    //OK
    if(_index >=10)
    {
        _delete_msg[8]=(_index/10)+'0';
        _delete_msg[9]=(_index%10)+'0'; 
        _len=10;
    }
    else
    {
        _delete_msg[8]=_index+'0';
        _len=9;
    }
Delete:
    System_Delayms(100);//发送前先休息下.
    GSM_DealAllData();
    UART0_Send(_delete_msg,_len,1); 
    System_Delayms(3000); //需要2秒的删除时间
    if(UART0_RecvLineWait(_recv,60,&_retNum)==-1)
    {
        ++s_TimeOut;
        if(repeats>2)
        {
            return -1;
        }
        ++repeats;
        goto Delete;
    }
    if(Utility_Strncmp(_recv,"ERROR",5)==0)
    {  
        if(repeats1>2)
        { 
            return -2;
        }
        ++repeats1;
        goto Delete;
    }
    if(Utility_Strncmp(_recv,"OK",2)==0)
    { 
        ++s_MsgDel;
        return 0;
    }
    //是其他的返回,就return了
    //这里不应再进行删除
    //因为有可能第一次删除成功了.
    //恰好了来了条短信,放在这里.
    //这里再删除,就造成短信误删了.
    GSM_DealData(_recv,_retNum); 
    return 1;
}

int GSM_AT_ShutDown()
{//关机前弄个这貌似没意义. 
    GSM_DealAllData();//顶多挂断个电话~~
    int _retNum;
    GSM_AT_QuitInput();//防止前面短信失败祸害这里.
    int _repeats=0;
    int _repeats1=0;
    char _recv[50]="AT%MSO";
    //AT^SMSO 该指令返回
    //^SMSO: MS OFF
    //OK
SHUTDOWN:
    UART0_Send(_recv,7,1);
    if(UART0_RecvLineWait(_recv,50,&_retNum)==-1)
    {
        if(_repeats>1)
        {
            //Console_WriteStringln("SIM_AT_ShutDown:TimeOut !");
            return -1;
        }
        ++_repeats;
        goto SHUTDOWN;
    }
    if(Utility_Strncmp(_recv,"^SMSO: MS OFF",13)==0)
    { //再接个OK
        //Console_WriteStringln("SIM_AT_ShutDown:Done !");
        UART0_RecvLineWait(_recv,50,&_retNum);
        //都关机了,不管是什么了
        return 0;
    }
    if(_repeats1>2)
        return -1;
    ++_repeats1;
    goto SHUTDOWN;
}
int GSM_AT_OffCall()
{
    GSM_DealAllData();
    int _retNum;
    char _ath[]="ATH";
    char _recv[50];
    //该指令如果挂断了电话,那么就没有返回,无电话状态时,返回OK
    //OK  
    UART0_Send(_ath,3,1);
    if(UART0_RecvLineWait(_recv,50,&_retNum)==-1)
        return 0;
    if(Utility_Strncmp(_recv,"OK",2)==0)
        return -3; 
    else
    {
        GSM_DealData(_recv,_retNum); 
        return 0;
    }
} 
int GSM_AT_QuitInput()
{
    //该函数发送一个 ESC符号 来退出
    //短信输入状态
    char _temp=0x1B;
    //UART0_Send(&_temp,1,1);//发送输入状态结束字符，不需要带回车换行
    UART0_Send(&_temp,1,0);
    return 0;
}

 


