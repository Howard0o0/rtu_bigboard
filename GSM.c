//////////////////////////////////////////////////////
//     �ļ���: gsm.c
//   �ļ��汾: 1.0.0
//   ����ʱ��: 09��11��30��
//   ��������: 
//       ����: ����
//       ��ע: ��
// 
//////////////////////////////////////////////////////
//   �ļ��汾: 3.0.0
//   ����ʱ��: 19��8��28��
//   ��������: ����������ź�GPRS��IGT����      
//////////////////////////////////////////////////////
//#include <msp430x26x.h>
#include "GSM.h"
#include "store.h"
#include "common.h"
#include "timer.h"
#include "uart0.h"
#include "rtc.h"
#include "led.h"
#include "sampler.h"
#include "Console.h"

// ���Ż������������ż���
#define ARRAY_MAX 15 
#define MSG_MAX_LEN  120



 int g_work_mode;
 int main_time_error;
extern int s_alert_flag[8];

//  ����ͳ�Ƽ���  
int  s_TimeOut=0;
int  s_RING=0;
int  s_OOPS=0;
int  s_MsgFail=0;
int  s_MsgOK=0;
int  s_MsgDel=0;

int  s_RecvIdx=0;    //ע����ָ�������������һ��
int  s_ProcIdx=0;    //ע����ָ�����������һ��. 
int  s_MsgNum=0;     //�յ����ŵ����� 
int  s_MsgArray[ARRAY_MAX];

//�����佻���ı���
int  s_MsgLeft=0;
char s_NetState='0';

int GSM_Open()
{
    TraceMsg("GSM Open !",1);
                                    //��ʼ������0,
    UART0_Open(UART0_GTM900_TYPE);  //lmj0814,GTM900 ATָ�������Ҫ���ӵ���\r\n,��mc35i������ͬ����ˣ��ڴ��ڷ��ͺ������棬��󸽼�\r\n
    
                                  // ΪSIMģ���ϵ� 
    P10DIR |= BIT7;              // 2438 P4.2 Ϊ BATT,���ߵ�ƽ  5438ΪP10.7
    P10OUT &= ~BIT7; 
     P10OUT |= BIT7; 
    System_Delayms(100);
    
    P3DIR |= BIT0;              //  2438 p1.0 Ϊ /IGT  5438ΪP3.0
    P3OUT |= BIT0;              //  ��ʼΪ����̬   
    System_Delayms(10);         //  ������Ҫ10ms 
    
    
    P3OUT &=~BIT0;              // ��/IGTһ���½�,����100ms
                                // ��Ҫ��ʱ     

    System_Delayms(50);         // GTM900��Ҫ50ms
                                // MC35i��Ҫ150ms
    P3OUT |= BIT0;              // �ٰ�/IGT ����

    System_Delayms(1200);       // �ȴ����� 1�����Ч״̬
                                // ��������ͨ��
                                //�ٵ�12�� ��SIM��������,����Ҳ��7��
                                // ò��10�뻹��̫��///
    System_Delayms(30000);
  
                                //��ʼ����ر���
    s_RecvIdx=0;     
    s_ProcIdx=0;
    s_MsgNum=0;
    s_MsgLeft=0;
    for(int i=0;i<ARRAY_MAX;++i)//���������ʼֵΪ0(0�ǲ���ȷ�Ķ�������)
        s_MsgArray[i]=0;
    return 0;
}

int GSM_HardReboot()
{
	P3DIR |= BIT0; //  p1.0 Ϊ /IGT 
    //P1OUT |= BIT0; //  ��ʼΪ����̬   
    //System_Delayms(10);//  ������Ҫ10ms 
    P3OUT &=~BIT0; // ��/IGTһ���½�,����2500ms
    System_Delayms(2500); // �ػ���Ҫ����2-3��
    P3OUT |= BIT0; // �ٰ�/IGT ����
    System_Delayms(15000);// �ȴ�15s�ػ�
                 // ��������ͨ��
   	P3OUT &=~BIT0; // ��/IGTһ���½�,����50ms
    System_Delayms(50); // ������Ҫ50ms �����¿���
    P3OUT |= BIT0; // �ٰ�/IGT ����
    System_Delayms(30000);
    //��ʼ����ر���
    s_RecvIdx=0;     
    s_ProcIdx=0;
    s_MsgNum=0;
    s_MsgLeft=0;
    for(int i=0;i<ARRAY_MAX;++i)//���������ʼֵΪ0(0�ǲ���ȷ�Ķ�������)
        s_MsgArray[i]=0;
	
    return 0;
}

int GSM_Close(int _type)
{
    TraceMsg("GSM Close !",1);
    if(!_type)
        GSM_AT_ShutDown();//���͹ر�ATָ��
    System_Delayms(100);
    //�����Щ����
    s_RecvIdx=0;     
    s_ProcIdx=0;
    s_MsgNum=0;   
    s_MsgLeft=0;
    
    P10DIR |= BIT7;
    P10OUT &= ~BIT7;  //�Ͽ�SIM��ѹ
    
    UART0_Close();   //�رմ���0
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
            return -1;//�޷�ʹ�÷��� -1; 
             
        }
        ++times;
    }
    return 0;//��������   0 ; 
}

int GSM_Waiting_Process(int _type)
{
    int _waits=100;//30��
    switch(_type)
    { 
      case 1:
        _waits=100;
        break;
      case 2:
        _waits=200;//1����
        break;
      case 3:
        _waits=300;//1.5����
        break;
      case 4:
        _waits=400;//2����
        break;
      default:
        _waits=100;//30s
        break;
    }
    int _retNum;
    char _recv[UART0_MAXBUFFLEN];//���ջ���
    int _repeats=0;  //
    int _repeats1=0; //
    if(GSM_DealAllData()==2)//�ж���ֱ�ӷ���
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
        //�ȴ�����, �ȴ�1����
        while(UART0_RecvLineWait(_recv,UART0_MAXBUFFLEN,&_retNum)<0)
        {
            //UART0_RecvLineWait��300ms
            //300ms * 300 =  90000ms = 90s
            if(_repeats > _waits)    
            {
                return 0;
            }
            ++ _repeats;
        }
        //����Ƕ���,���ٶȷ��ؽ��д���
        if(GSM_DealData(_recv,_retNum)==2)
            return 2;
        
        ++ _repeats1;//���յ����ݵ�һ��ѭ��
        if( _repeats1>2)// 
            return -1;
    } 
}

//
// �ϱ����ݸ�ʽ����:
//                                                                                                      1 
//  0         1         2         3         4         5         6         7         8         9         0    
//  0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678
//  $    վ��   > ʱ��     * ������Ŀ(ģ��ֵΪ ������+4λ10����   ����Ϊ������� 6λ16���� ����Ϊ�������һλ2����)
//  $00011100011>0908161230*A1000B1000C1000D2000E2000F3000G3000H4000I000000J000000K000000L000000M1N1O1P1Q1R1S1T1#
//
//
//   �߼��ܻ���, û�취��.
//
int GSM_Report_Process()
{//����������.
    //������ݴ洢�ϱ�� �±� 
    char _startIdx=0x00;
    char _seek_num=0; //ȷ�������ݵ����� ���ᳬ��������,���������ѭ��
    char _endIdx=0x00;
    char _data[220];
    char _phone[12];
    int  _ret=0;
    int  _send_ret=0;
    int  _idx = 13; 
    int  _exit=0;  
    //��¼��Щ���Ű��� ��Щ_start������
    int  _array[10]={0,0,0,0,0,0,0,0,0,0};
    int  _array_idx=0;
    if(RTC_ReadStartIdx(&_startIdx)<0 || RTC_ReadEndIdx(&_endIdx)<0 )
    {//������������������ʧ��
        TraceMsg("read idx error ,reGenerate .",1);
        if(RTC_RetrieveIndex()<0)
        {
            TraceMsg("reGen failed .",1);
            RTC_SetStartIdx(DATA_MIN_IDX);
            RTC_SetEndIdx(DATA_MIN_IDX);
        }
        TraceMsg("StartIdx:",0); 
        TraceInt4(_startIdx,1);
        TraceMsg("EndIdx:",0);
        TraceInt4(_endIdx,1);
        RTC_ReadStartIdx(&_startIdx);//���¶���
        RTC_ReadEndIdx(&_endIdx);//���¶���
    }
    //ͷ��
    _data[0] = '$';
    Store_ReadDeviceNO(&(_data[1]));
    _data[12] = '>';
    if( _endIdx<DATA_MIN_IDX || _endIdx >DATA_MAX_IDX ||
       _startIdx<DATA_MIN_IDX || _startIdx >DATA_MAX_IDX  )
    {
        return -1;
    } 
    //
    //   1. ��װ ����
    // 
    //   2. ��������
    //
    //   3. �Ƿ�ﵽ_endIdx �˳�
    //  
    
    while(1)
    {//����ѭ��
        //_idx��ʼΪ13; _retΪ0
        _idx=13;_ret=0;
        //��װ����ѭ��
        while(1)
        {//�����ܾ��Ա�����ѭ����?  
            _ret = Store_ReadDataItem(_startIdx,&(_data[_idx]),0);
            if(_ret<0)
            {
                RTC_SetStartIdx(_startIdx);//����_startIdx
                return -1; //�޷���ȡ���� ��ֱ������.
            }
            if(_ret==1)
            {//�����һ���Ѿ����͹�������,��ô�Ͳ����� 
                if(_startIdx == _endIdx)
                {//����Ƿ���  _endIdx, ����ǾͲ�����ѭ����.
                    _exit=1;
                 //�����װ.
                    _data[_idx-1]='#';//�����һ��;�Ÿ�Ϊ#
                    break;
                }
                //
                //
                //
                if(_seek_num > DATA_MAX_IDX - DATA_MIN_IDX)
                {//�����ж�һ��,���������ѭ��.
                    _exit=1;
                    _data[_idx-1]='#';//�����һ��;�Ÿ�Ϊ#
                    break;
                }
                
                if(_startIdx >= DATA_MAX_IDX)  _startIdx=DATA_MIN_IDX;
                else   ++ _startIdx;//��һ����
                ++_seek_num;//����һ�����ݾͼ�1.
                
                continue;
            }
            else
            {//�������ķ�������,������_idx,������;�� 
                _array[_array_idx++] = _startIdx;
                _idx += _ret;
                _data[_idx++]=';';//����һ���ָ���
                if( _idx + _ret < MSG_MAX_LEN )//���ȹ���װһ��.
                {
                    if(_startIdx >= DATA_MAX_IDX ) _startIdx = DATA_MIN_IDX;
                    else ++ _startIdx;
                    ++_seek_num;//����һ�����ݾͼ�1.
                    
                    continue;
                }
            }
            _data[_idx-1]='#';//�����һ��;�Ÿ�Ϊ#
            
            //������һ�ε� _startIdx
            if(_startIdx>=DATA_MAX_IDX) _startIdx=DATA_MIN_IDX;
            else   ++ _startIdx;//��һ����
            ++_seek_num;//����һ�����ݾͼ�1.
            
            //��������ݵ���װ,����Ϊ_idx;
            break;
        }
        //����ѭ��
        if(_array_idx!=0)//�����ݻ������ж����ͷ���
        {
            for(int i =1; i<=4;++i)
            {
                if(Store_GSM_ReadCenterPhone(i,_phone)<0)  continue;
                if(_phone[0]=='0'||_phone[1]=='0'||_phone[2]=='0') continue;
                _send_ret += GSM_SendMsgTxt(_phone,_data,_idx);
            }
            if(_send_ret==0)//�������а�������������Ŀ�����óɷ��͹�.
            {
                for(int i=0;i<_array_idx;++i)
                {//����Ϊ�Ѿ�����
                    Store_MarkDataItemSended(_array[i]);
                }
                //Ȼ���������
                _array_idx=0;
                //�������,Ҫ����_startIdx.
                RTC_SetStartIdx(_startIdx);
            }
            else
                _exit=1;//ĳ�����ŷ���ʧ��,�Ͳ�����.
            
            
            //Ȼ���������
            _array_idx=0;
        }
        //�ж��Ƿ� �ﵽ _endIdx
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
    
    
    
    GSM_StartCheckSIM();//��鲢����SIM����ʣ�����
    if(GSM_CheckLive()<0)
    {
        GSM_Open();
        if(GSM_CheckOK()<0)
        {
            return -1;
        }
    }
    
    Timer_Zero();
    //�ȴ����� ������������Ч��. 
    // �ȴ�������ǰ�Ķ���,Ȼ��ʼ�ȴ�
    // 2�ζ��ż�����ʱ��Ϊ1����,�������1���Ӿ��˳�.
    // �����ȫ������,1���Ӿ��˳�.
    // Ҫ�� �绰�� SIM_Dealֱ�Ӿ͹Ҷ�, �������������,Ҳ����.
    // Ҫ�Ƕ��ŵ���. ���������ʼ����.
    // ��������ټ����ȴ����1����.
    
    //��һ���ֵȴ�. �ȴ�ʱ���� _type
    if( s_ProcIdx != s_RecvIdx) //�ж�����Ҫ���д���
    {
        //��ʼ������Щ������Ķ�������
        //����������,���Ӵ����±�,
        //ֱ�������±�ﵽ�˽����±�,�ͷ���.  
        GSM_ProcMsgArray();
    }
    if(GSM_Waiting_Process(_type)!=2)//���еȴ�  �ж��Ż��ǵ绰 �ͼ���һ�ֵȴ�
    {//������Ϊ���� �����ص�. Ҳ�������ݵ���.�Ǿ͵�����.���Է�����.
        goto BREAK;
    }
    //����е�2���ֵĵȴ�. ���ʱ����30��
    while(1)
    {
        //1. ��������
        GSM_ProcMsgArray(); 
        if(Timer_Passed_Mins(5)) //ʱ�䲻�ó���5����,
        {//Console_WriteStringln("SIM_LOOP: too long time, force to quit !");
            goto BREAK;
        }
        //2. Ȼ��ȴ�
        if(GSM_Waiting_Process(1)!=2)//���еȴ�  �ж��Ż��ǵ绰 �ͼ���һ�ֵȴ�
        {//���ʱ����30��
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
    GSM_EndCheckSIM();// �ػ�ǰ���SIM�ж�������  ȷ��Ϊ0;  
    return 0;// 
}
//  0         1         2         3         4         5         6         7         8         9         0    
//  0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678
//  $    վ��   > ʱ��     * ������Ŀ(ģ��ֵΪ ������+4λ10����   ����Ϊ������� 6λ16���� ����Ϊ�������һλ2����)
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
    //���������ķ���������
    if(Store_GSM_ReadCenterPhone(1,_phone)>=0)
    {
        if(_phone[0]!='0' ||_phone[1]!='0' ||_phone[2]!='0')
        {
            GSM_SendMsgTxt(_phone,_send,_ret);
        }
    }
    //������
    if(Store_GSM_ReadCenterPhone(6,_phone)<0)//��ȡ�����˵绰 
            return -1; 
    if(_phone[0]!='0' ||_phone[1]!='0' ||_phone[2]!='0')
    {
        GSM_SendMsgTxt(_phone,_send,_ret); 
        GSM_CallLeader(_phone);//�������˴�绰
    }
    return 0; 
}

int GSM_CheckLive()
{//����AT����Ƿ�����Ӧ,����Ӧ����ΪGSMģ�黵����
    int _repeats1=0;
    //int _repeats2=0;

  AT:    
    GSM_DealAllData();//�������������
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
    GSM_DealAllData();//�������������
    //�ټ��������״̬
    s_NetState='0';
    GSM_AT_QueryNet(); //��ѯ����   
    if(s_NetState=='1'|| s_NetState=='5')
        return 0;
    else
    {
        Led_WARN();
        TraceMsg("GSM doesn't work !",1);
        return -2;
    }
}
int GSM_Init()//SIM��ʼ����
{
    int _repeats=0;  
    //int _repeats1=0;
    int _repeats2=0;
   // int _repeats3=0;
   s_NetState='0';
    while(GSM_AT_CloseFeedback()!=0)
    {//�رջ���,һ��Ҫ�ɹ�, ��Ȼ�������붼�Ǵ����.
        if(_repeats>2)
        {//3�ζ��Ǵ����.
            TraceMsg("Failed to close feedback !",1);
            return -1; 
        }
        ++_repeats;
    }

    while(GSM_AT_QuerySim()!=0)
    {//��ѯSIM���Ƿ�����.
        if(_repeats>2)
        {//3�ζ��Ǵ����.
            TraceMsg("Sim card is not normal !",1);
            return -1;
        }
        ++_repeats;
    }

 #if 0
    while( GSM_AT_SetMsgmode(1)!=0)
    {//���ö���txtģʽ,һ��Ҫ�ɹ�, ��Ȼ�������붼�Ǵ����.
        if(_repeats1>2)
        {//3�ζ��Ǵ����. 
            return -2;
        }
        ++_repeats1;
    }
    while( GSM_AT_SetMsgNotify()!=0)
    {//���ö���Ҫ ��������.
        if(_repeats3>2)
        {//3�ζ��Ǵ����. 
            return -4;
        }
        ++_repeats3;
    }
#endif

    while( !(s_NetState=='1' || s_NetState=='5') )
    {
        TraceMsg("Wait for signal ready!", 1);
        System_Delayms(5000);//�ȴ�һ��
        //SIM_InitOK�������ᱻ�ظ�3�ζ�.�����������ﲻ��̫��.
        if(_repeats2>2)
        {
            return -3;
        }
        GSM_AT_QueryNet(); //��ѯ����
        ++_repeats2;
    }
    return 0;
}
//
//  ���ڶ����Ѷ���(REC Readed)�ķ���,.��������Ҫ�����Ķ��ŵ�������Դ��
//  1. CNMI��Ϣ����¼����msgArray��,��ȡmsgArray��õ�����
//  2. checkSIM����,��˳������� ����
//  ע��: һ�������ǲ���������CNMI��Ϣ��.����Ҳ���ᱻ�����Ķ��Ÿ��� 
//    1. ���������� ����������;ʧ����.
//    ����˵��������ͷ,����û������������,�ͻ������ٴζ���������,
//     ��ʱ�����������ž����Ѷ�״̬��.
//    2. ɾ������ʧ����.��Ȼ����OK.��ʵ���ϲ�ûɾ��
//       (�ֲ���ָ�� ɾ�����Ų��ܳɹ�û�ɹ�,������OK,
//         ����ERRORֻ������ʶ��ATָ�����)
//    3. ������� ????
//
//    1,2��ì�ܾ��� 
//      Ҫôɾ��һ��ʵ�ʲ�δ�����Ķ���.
//      Ҫô�ظ�����һ������.�ظ�����������ɾ����������ʧ�ܵĴ���.
//    
//    ����ѡ���ظ�����.  ���������Ծɴ���REC Readed�Ķ���.  
//0��ʾ����
//1��ʾ�޶���
//-1��ʾ��ʱ
int  GSM_ProcMessageTxt(int _index)//��ʧ������,���3��
{
    int _retNum;
    int _dataLen=0;//����ATָ���,�����������ݳ��� 
    char _recv[UART0_MAXBUFFLEN];//���ջ���
    char _rePhone[14];//�ظ��绰
    char _send[50]; //���ͻ���,��"�ʹ�"һ�½��ջ���
    int _repeats=0;  //ʧ�ܴ���
    int _repeats1=0;
    int _repeats2=0;
    int _repeats3=0;
ProcMessage:
    //1. ����ATָ�,
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
    //2.�ѻ��������һ��
    GSM_DealAllData();
    GSM_AT_QuitInput();
    
    
    /*
    //////////////////////////////////
    char _recv1[60];
    char _delete_msg[11]="AT+CMGD=1,4";
    //��ָ��ֻ�᷵��:
    //OK
    UART0_Send(_delete_msg,strlen(_delete_msg),1); 
    System_Delayms(10000); //��Ҫ2���ɾ��ʱ��
    if(UART0_RecvLineWait(_recv1,60,&_retNum)==-1);
    if(Utility_Strncmp(_recv1,"ERROR",5)==0);
    if(Utility_Strncmp(_recv1,"OK",2)==0);
    ////////////////////////////////////////////////////
    */
    
    //3. ��AT������,���������� 
    UART0_Send(_send,_dataLen,1);
    System_Delayms(50);
    if(UART0_RecvLineWait(_recv,UART0_MAXBUFFLEN,&_retNum)==-1)
    { //δ���յ�����ͷ
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
    {//��õ��˶���ͷ,�������ж������λ���ǲ���û�ж���
    //�޶���. "+CMGR: 0,,0"
    // +CMGR: "REC UNREAD","+8613607193119",,"09/02/27,16:36:16+32"��ʽ 
        if(Utility_Strncmp(_recv,"+CMGR: 0,,0",11)==0)
        {
            TraceMsg(" no msg find ",1); 
            return 1;//�޶���
        }
        //������Ҫע�����?GTM900���ص��ַ�������\"Ҳ��˫�������������ԣ�������������Ӧ���Ǵӵ�8����ʼ
        if(Utility_Strncmp(&(_recv[8]),"REC",3)!=0)
        {
            System_Delayms(1000);
            //�ȰѺ����Ķ�����������.
            if(UART0_RecvLineLongWait(_recv,UART0_MAXBUFFLEN,&_retNum)<0)//���ȴ��ɴ�5��
            {//�ղ���Ҳ����ν.�������ǲ���Ҫ.
               ;
            }
            //�ٽӸ�OK
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
        //Ȼ�����ǻ�ö��ŵĻظ��绰
        GSM_GetRePhone(_recv,_rePhone);
        System_Delayms(1000);//������ʱ ��Ҫ�ܶ� �ܶ�..
        //�����һ��δ���Ķ���.��������ĩβ�������÷ǳ�����..
        //�������ܵ����ŵ������,  
        if(UART0_RecvLineLongWait(_recv,UART0_MAXBUFFLEN,&_dataLen)<0)//���ȴ��ɴ�5��
        {//������������һֱû����.
            TraceMsg("can't read msg data ",1);
            if(_repeats1>2)
                return -1;
            ++_repeats1;
            goto ProcMessage;
        }
        //�ж��Ƿ��ǿն���
        if(Utility_Strncmp(_recv,"OK",2)==0)
        {
            TraceMsg("msg is a empty msg ",1);
            return 0;
        }
        if(UART0_RecvLineWait(_send,50,&_retNum)==-1)//������������, ��Ӧ���� 
            ++s_TimeOut;
        else
            GSM_DealData(_send,_retNum);//����һ��OK
        
        //����Ǵ���绰���� ��ֱ�ӷ���0 ����10086 ����绰�ȵ�
        if(Utility_CheckDigital(_rePhone,0,10))
        {
            TraceMsg("phone num is not right ",1);
            return 0;
        }
        char message[10]="SMS:";
        message[4]=(char)(_index);
        TraceStr(message,5,1);
        return GSM_ProcMsgData(_recv,_rePhone,_dataLen); //Ȼ��ʼ������������    
    }
    //
    //  ����� ERROR˵����� index����������.
    //
    if(Utility_Strncmp(_recv,"ERROR",5)==0)//û��������ͷ, ����������.
    {
        if(_index>40)//40���ϵ�index �ܿ����ǹ�����.
            return 2;//�±����
        if(_repeats2>2)
            return -1;
        ++_repeats2; 
        goto ProcMessage; 
    }
    //�Ȳ���ERROR,Ҳ���Ƕ���ͷ;
    //����һ��;
    GSM_DealData(_recv,_retNum);
    if(_repeats3>2)
        return -1;
    ++_repeats3;
    goto ProcMessage;
}

int GSM_ProcMsgArray()
{//ֱ�������±� �ﵽ������������
    int _ret; 
    while(s_ProcIdx != s_RecvIdx)
    { //������������ķ���ֵ ���� ���ڲ����õķ��Ͷ��ź����ķ���ֵ
    //�޶��� �᷵�� 1
        _ret=GSM_ProcMessageTxt(s_MsgArray[s_ProcIdx]);
        if( _ret < 0)
        {//��������Ͷ���û�ɹ�,�����ٴ���һ����������
         //û�����ɹ��Ķ���,���������ﶼ��ɾ��.
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

int GSM_StartCheckSIM()  //�������һ��SIM��
{
    int _index=1;
    int _MsgNum=0;
    int _ret=0;
    s_MsgLeft= 0;
    if(GSM_AT_QueryStore()<0)//�ú�������д SIM_MsgLeftNum;  
    {
        if(GSM_AT_QueryStore()<0)
            return -1;
    }
    if(s_MsgLeft == 0)//û����
        return 0; 
   _MsgNum = s_MsgLeft;//�ж��������� 
    while( _MsgNum !=0 )//��������SIM���ﻹ�ж���
    {
        _ret=GSM_ProcMessageTxt(_index);
        //����������ܷ��� -1 0 1
        if(_ret==1)//������λ��û�ж���,����һ��
        {
            ++ _index;
            if( _index > 80)//һֱ������������80��.. ���������г�����.������.
                break;
            continue;//ʹ���ѭ���ദ��һ������
        }
        if( _ret==2)
        {//�ö�������������
            break;
        }
        if( _ret==0)
        {//�����ɹ� ��ɾ���� 
            GSM_AT_DeleteMsg(_index);  
        }
        //ʧ���� �Ͷദ��һ��
        if(_ret<0)
        {
            //�ɹ��˾�ɾ��
            if(GSM_ProcMessageTxt(_index)==0) 
                GSM_AT_DeleteMsg(_index);
            //��ʧ��,���ǾͲ�����,����ɾ��
        }
        //����Ƿ���-1 ��ʱ�Ļ�, ���Ǿ�ֱ�Ӳ�����,
        // ����Ҳ��ɾ��. ���Ժ���.
        //��һ������
        ++ _index;
        -- _MsgNum;
    }
    //����������������Ķ��� �ᱻ����MsgArray������,�ᱻ��ȷ������. 
    return 0;
}

int GSM_EndCheckSIM()     //�ػ����һ��SIM��
{
    int _index; 
    int _MsgNum;
    int _repeats=0;
    int _ret=0;
EndCheckSIM: 
    _index=1;
    _MsgNum=0;
    s_MsgLeft = 0;
    if(GSM_AT_QueryStore()<0)//�ú�������дSIM_MsgLeftNum; 
    {
        if(GSM_AT_QueryStore()<0)
            return -1;
    }
    if(s_MsgLeft == 0)//û����
        return 0; 
    _MsgNum = s_MsgLeft;//�ж��������� 
    while(_MsgNum!=0)
    {
        _ret=GSM_ProcMessageTxt(_index);
        if(_ret==1)//������λ���ǿն���,������һ��
        {
            ++ _index;
            continue;//ʹ���ѭ���ദ��һ������
        }
        if(_ret==2)
        {
            break;
        }
        //�����Ƿ����ɹ� ��ɾ����  
        //�����startcheck �Լ�procMsgArray��һ��
        GSM_AT_DeleteMsg(_index); 
        ++ _index;
        -- _MsgNum;
    }
    
    if(_repeats>2)
        return -2;
    ++ _repeats;
    goto EndCheckSIM; //�����¼��SIM����Ķ����Ƿ�Ϊ0 
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
    Utility_Strncpy(&_send[_index],_phoneNum,11);//ATָ��绰������Ҫ��˫���ţ�����û�м��룬��ô�������?
    _index+=11;
    */
    Utility_Strncpy(&_send[_index],_phoneNum,13);
    _index+=13;

    /*  ������һ�δ��벻֪����ʲô��˼?��GTM900ָ���У�û�к������129
    _send[_index++]=',';
    _send[_index++]='1';
    _send[_index++]='2';
    _send[_index++]='9'; 
  */
        
    GSM_AT_QuitInput();//����ǰһ��ʧ�ܵ� > ���Ŵ����Ļ���
    GSM_DealAllData();// ����ǰ,������Ҫ ��ս��� ������ ,
    UART0_ClearInput();//�� > ���ű��Ϊ0
  
    UART0_Send(_send,_index,1);
    //�ȴ� > ��ʾ����
    if(UART0_WaitInput()==-1)
    {
        ++s_TimeOut;
        if(_repeats>1)//��������2��
        {
            TraceMsg("send msg fail 1 !",1);
            return -1;
        }
        ++_repeats;
        //  û�ȵ��Ļ�,��������Ϊ������������
        GSM_DealAllData();
        //Console_WriteStringln("SIM_SendMsgTxt:WaitInput Fail !");
        //SIM����ɵ��.
        //������GSM��Ϣ��Ϣ
        System_Delayms(500);
        goto SendMsgTxt;
    } 
    //Console_WriteStringln("SIM_SendMsgTxt:Got > :");
    //�ȵ��� > ��ʾ����
    Led3_WARN();
    //��������
    _data[_dataLen]=0x1A;//CTRL+Z  
    UART0_Send(_data,_dataLen+1,0);//�෢һ��0x1A 
    //Console_WriteStringln("SIM_SendMsgTxt:Send data:");
    //Console_WriteStringln(data);
    //   ����������ʱ10��ȴ����ŷ��ͳɹ�.
    //   �ȴ��յ�+CMGS   �Լ�һ��OK. 
    //��ʼ��ʱ.
    //Console_WriteStringln("SIM_SendMsgTxt:Waiting !");
    System_Delayms(10000);
    if(UART0_RecvLineWait(_send,60,& _retNum)==-1)
    {//���ŷ��� δ�ɹ�
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
  {//�ɹ�
    //Console_WriteStringln("SIM_SendMsgTxt:Send OK !");
    Led4_WARN();
    if(UART0_RecvLineWait(_send,60,& _retNum)==0)
      GSM_DealData(_send,_retNum);
    //��û�յ�OK,������ν.
    ++s_MsgOK;
    TraceMsg("send msg ok !",1);
    return 0;
  }
  //Console_WriteStringln("SIM_SendMsgTxt:Others data,send fail !");
  //Console_WriteStringln(send);
  GSM_DealData(_send,_retNum);
  ++s_MsgFail;
  if(_repeats2 >2)
  {//������� ���� ����ATָ��
    GSM_AT_QuitInput();
    TraceMsg("send msg fail  3 !",1);
    return -2;
  }
  ++_repeats2;
  goto SendMsgTxt;
} 
 
//  
//  ����ֵΪ0,��ʾ�ö��ŷ�������.
//  01234567890123456789012345678901
//  $1234*NO*00011100011,0000,00,00#
//  ���ǻظ������ݶ���
//  01234567890123456
//  $00011100011<*****************    $00011100011<UN*FAIL#
//  BS,DL,CP ,TM,PD,PL,CS,PR,WM
int GSM_ProcMsgData(char * _data,char* _rePhone,int _dataLen)
{
    TraceMsg(" start to proc msg data ",1);
    Led2_WARN();
    char _send[120];//��120����
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
    if(_data[0]!='$' || _data[_dataLen-1]!='#')//����$��ͷ#��β��ֱ�Ӻ���
    {
        TraceMsg("wrong format ",1);
        return 0;
    }
    //ƴ��ͷ��
    _send[0]='$';
    if(Store_ReadDeviceNO(&(_send[1]))<0)
        return -1;
    _send[12]='<';
    //���*��
    if(_data[5]!='*' || _data[8]!='*')
    {//����Ͳ�������.��ֹ��Ϊ�Լ����Լ���,������ѭ��.
        //_ret=Utility_PackErrorMsg("UN",_send);
        //return GSM_SendMsgTxt(_rePhone,_send,_ret);
        TraceMsg("wrong format ",1);
        return 0;
    }
    //���ж������Ƿ���ȷ
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
            //ͬʱ������һ�α���ʱ�� 
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
            //ͬʱ������һ�α���ʱ��
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
        //$1234*CP*1,13607193119# //���õ绰
        
        //$1234*CS*A,00000011#��  //����ͨ��ѡ��
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
            {//ģ���� ,��λ��ǰ
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
            {//������,ǰ4λ,��λ��ǰ
                
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
            {//������,��λ��ǰ
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
        //�������� �Ǿ��Ǵ�����
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
        
        //��Ҫ���max�Ƿ����min
        _min=0; _max=0; _idx1=_data[9]-'A'+1;
        if(_data[11]!='9' ||_data[12]!='9' ||_data[13]!='9' ||_data[14]!='9')
        {
            _min += (_data[11]-'0')*1000;
            _min += (_data[12]-'0')*100;
            _min += (_data[13]-'0')*10;
            _min += (_data[14]-'0');
        }
        else
        {//����������ֵ����,���Ǿ�ȡ����ֵ
            if(Store_ReadDataMinInt(_idx1,&_min)<0) 
                _min=0;//��������������.�����max>min��
        }
        
        if(_data[16]!='9' ||_data[17]!='9' ||_data[18]!='9' ||_data[19]!='9')
        {
            _max += (_data[16]-'0')*1000;
            _max += (_data[17]-'0')*100;
            _max += (_data[18]-'0')*10;
            _max += (_data[19]-'0');
        }
        else
        {//����������ֵ����,���Ǿ�ȡ����ֵ
            if(Store_ReadDataMaxInt(_idx1,&_max)<0) 
                _max=4096;//��������������.�����max>min��
        }
        if(_min > _max)
        {
            _ret = Utility_PackBadMsg("DL",_send);
            return GSM_SendMsgTxt(_rePhone,_send,_ret);
        }
        //д��rom
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
            
            s_alert_flag[_idx1-1]=0;//������ʶҪȥ��
            
            
            //���±�13�������뿪ʼ��д.
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
            _send[30]='#';//����Ϊ31,���Է�����.
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
                    _tempChar1 &= 0xEF;//���λ��0
                }
                else
                {
                    _tempChar1 |= 0x80;//���λ��1
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
            //  _tempChar1 Ϊ��ֵ
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
                //�޸��ڴ�ֵ
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
                //���޸��ڴ�ֵ
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
            
            //���ֵ����������ֵ,������BAD����.
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
        RTC_ReadTimeBytes5(g_rtc_nowTime); //���¶���ʱ��
        
        //�������ü��ʱ��
        Utility_CalculateNextCheckTimeBytes(_buffer);
        RTC_SetCheckTimeBytes(CHECKTIME_ADDR,_buffer);
        //�������ñ���ʱ��
        Utility_CalculateNextSaveTimeBytes(_buffer);
        RTC_SetSaveTimeBytes(SAVETIME_ADDR,_buffer);
        //�������ñ���ʱ��
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
        _ret += Store_SetWorkMode(_data[9]);//WorkMode��ʱ���ַ�.�Ժ�����ϸ����Ϊ��ֵ.
        if(_ret<0)
        {
            _ret =Utility_PackFailMsg("WM",_send);
            return GSM_SendMsgTxt(_rePhone,_send,_ret);
        }
        else
        {//�ɹ��л�ģʽ�� ������
            _ret =Utility_PackOKMsg("WM",_send);
            GSM_SendMsgTxt(_rePhone,_send,_ret);
            //�������� û��ɾ�� ��ֱ��������. ���Իᱻ�ظ�2��.�����ʱ���ø�.
            if( g_work_mode != (_data[9]-'0') )
            {//���ģʽ��һ��, ��ô��������.
                // �������ֱ������ ����� WM���ŵ���һֱ����������.
                //���Ըĳ�ģʽ��һ��������.
                GSM_Close(0);//�ر�GSM
                System_Delayms(1000);
                System_Reset();
            }
            return 0;
        }
        //
        //  �����Ǳ���ģʽ
        //BS,DL,CP,TM,PL,CS,PR,PM,WM
        //$����*RP*��������#
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
                _send[_idx1-1]='#';//���һ����һ��','��
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
                
                P5OUT = _tempChar1; //�ٸ���һ��,���ⲻһ��.
                
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
                
                P5DIR = _tempChar1; //�ٸ���һ��,���ⲻһ��.
                
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
                
                //��д,�������ַ�
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
                    //���õ���ROM��,��Ӧ�����ڴ�.
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
  //+CPMS: "MT",*,**,"MT",*,**,"MT",*,**//����ֻ����һ��
  if(_recv[13]!=',')
  {//������Ŀ��2 λ
    s_MsgLeft=(_recv[12]-'0')*10+(_recv[13]-'0');
  }
  else
    s_MsgLeft=(_recv[12]-'0');
  TraceMsg("Find Msg :",0);
  TraceInt4(s_MsgLeft,1);
  return s_MsgLeft;
}

int GSM_GetRePhone(char *src, char *dest)
{// +CMGR: "REC UNREAD","+8613607193119",,"09/02/27,16:36:16+32"��ʽ 
    int index=0;
    while(src[index++]!=',');//��һ��,��֮����ǻظ��绰 
    ++index;//indexָ��"��
    if(src[index]=='+')
    {//��+86,��Ҫ������3���ַ�
        index+=3;
    }
    else//û��+,GTM900��ʽ��
    {
        index+=2;
    }
    //����ָ�������,��ʼ����,����11λ
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

//�Է��������,���Ǿ������Ҷ�.
//��һ���,����Ҳ�Ҷ�
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
    // ����ATָ�ʼ����
    // ʵ�ʲ��� Լ6~8�� �Է��ֻ����з�Ӧ. 
    UART0_Send(_call,15,1);
    System_Delayms(5000);
    
    //�������ﲻ�����κ���������,
    //һֱ��ʱ,ֱ��ʱ���㹻
    //����ʱ 15��
    //  ����ʱ 3��
    System_Delayms(30000);
    for(int i=0;i<5;++i)
    {
        System_Delayms(1000);
        if(UART0_RecvLineWait(_recv,UART0_MAXBUFFLEN,&_retNum)!=-1)//��ǰ�汾��ʱΪ100ms
        {
            if(Utility_Strncmp(_recv,"OK",2)==0)
            {//�����˽���绰��. Ӧ�ùҶ�
                GSM_AT_OffCall();
            }
            //������������ڴ�绰 ���� No Carrier
            //��������˹Ҷ�,  ģ�龹Ȼ û�з�Ӧ.. ����..������.�������15������.
            //����û�źŵĻ�, ����Ҳ���������... 
            if(Utility_Strncmp(_recv,"NO CARRIER",10)==0)
            {//���ǾͲ�����.������,�������������ǰ,�Ѿ��ж��ŷ��ͳɹ�.
             //���Բ�̫������,SIMģ��û׼���� �� NO CARRIER
             //������������ �Ƚϰ�ȫ��
                GSM_AT_OffCall();
                return 0;
            }
            //�������������,������,�����Ƕ��ŵ���
            GSM_DealData(_recv,_retNum);
        }
        System_Delayms(1000);
    }
    //��ʱ����.���Լ��ҵ�.
    GSM_AT_OffCall(); 
    return 0;
} 
//   ���� �绰 ���������� 1
//   ��ʾ �ж���.  ����2 
//   ����  OK      , ����  0
//   ����  ERROR   , ����  -1 
//   ����  ��������,  ���� -2
//   ����δԤ�ϵ�������,���� -3
//   δ��������Ӧ�����ݽ�������
int GSM_DealData(char *_recv, int _dataLen)
{   
  //
  //  ����������
  //
    if(Utility_Strncmp(_recv,"RING",4)==0)
    {//�绰  
        ++s_RING;
        TraceMsg(" RING !",1);
        //Console_WriteStringln("SIM_Deal:RING !");
        GSM_AT_OffCall();
        return 2;
    }
    if(Utility_Strncmp(_recv,"+CMTI: ",7)==0)
    {  
        Led1_WARN(); 
        TraceMsg("Got A Msg !",1);
        ++s_MsgNum;//��� 
        if(_recv[13]=='\0')//recvLine������ĩβ����Ϊ'\0'
            s_MsgArray[s_RecvIdx]=(_recv[12]-'0');
        else
            s_MsgArray[s_RecvIdx]=(_recv[12]-'0')*10 + _recv[13]-'0'; 
        
        if(s_RecvIdx<ARRAY_MAX-1) 
            ++s_RecvIdx;//��������,ArrayIdxָ����ǵ�һ����Чλ
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
     //  ��Ӧ������
     //
     //��������յ� �Լ� �������ȼ��� ˳������ 
     if(Utility_Strncmp(_recv,"OK",2)==0)
     { 
         return 0;
     }
    //�����Ķ�������.
     return -1;
}
// ר��������ջ�����;
// ��ÿ�����ݵ���SIM_Deal
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
  //����ָ����յ�:
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
      //�ո�OK
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


//lmj 20170904���ӣ���ѯ��״̬�����Ƿ񻵵�
int  GSM_AT_QuerySim()
{
    int _retNum;
    char _recv[30]="AT%TSIM";
    //��ATָ��᷵��:
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
  //��ATָ��᷵��:
  //+CREG: 0,1
  //OK
  UART0_Send(_recv,8,1);
  if(UART0_RecvLineWait(_recv,60,&_retNum)==-1)
  { 
    ++s_TimeOut;
    return -1;
  }
  if(Utility_Strncmp(_recv,"+CREG: ",7)==0)
  {//�������״̬
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
    //��ָ���:
    //OK 
    if(_mode==0)
        _recv[8]='0';
    else
        _recv[8]='1';  
    UART0_Send(_recv,9,1);
    if(UART0_RecvLineWait(_recv,30,&_retNum)==-1)
    {  
        System_Delayms(100);//��ָ����Ҫ�϶���ӳ�
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
    System_Delayms(200);//��ָ����Ҫ�϶���ӳ�
    //++SIM_BadNum; 
    GSM_DealData(_recv,_retNum);
    return -2;
}

int  GSM_AT_CloseFeedback()
{  //����ָ����յ�:
  //ATE0 
  //OK
    int _retNum;
    int _repeats=0;
    char _recv[30]="ATE0";  
CloseFeedback:  
    //UART0_Send(_recv,4,1); lmj20170814
    UART0_Send(_recv,4,1);//lmj0814����UART0��������Ĳ���س����У�ֱ�ӽ��س����з���ָ���ַ�������
//    System_Delayms(50);
    if(UART0_RecvLineWait(_recv,30,&_retNum)==-1)
    {  
        ++s_TimeOut; 
        return -1; 
    } 
      //�յ�ERROR ,
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
    //�����ATE0,���ٻ��һ��OK,�Ϳ��Է�����
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
    //�����OK,˵���Ѿ��ر�
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
    //��ָ��ֻ�᷵��:
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
    System_Delayms(100);//����ǰ����Ϣ��.
    GSM_DealAllData();
    UART0_Send(_delete_msg,_len,1); 
    System_Delayms(3000); //��Ҫ2���ɾ��ʱ��
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
    //�������ķ���,��return��
    //���ﲻӦ�ٽ���ɾ��
    //��Ϊ�п��ܵ�һ��ɾ���ɹ���.
    //ǡ��������������,��������.
    //������ɾ��,����ɶ�����ɾ��.
    GSM_DealData(_recv,_retNum); 
    return 1;
}

int GSM_AT_ShutDown()
{//�ػ�ǰŪ����ò��û����. 
    GSM_DealAllData();//����Ҷϸ��绰~~
    int _retNum;
    GSM_AT_QuitInput();//��ֹǰ�����ʧ�ܻ�������.
    int _repeats=0;
    int _repeats1=0;
    char _recv[50]="AT%MSO";
    //AT^SMSO ��ָ���
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
    { //�ٽӸ�OK
        //Console_WriteStringln("SIM_AT_ShutDown:Done !");
        UART0_RecvLineWait(_recv,50,&_retNum);
        //���ػ���,������ʲô��
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
    //��ָ������Ҷ��˵绰,��ô��û�з���,�޵绰״̬ʱ,����OK
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
    //�ú�������һ�� ESC���� ���˳�
    //��������״̬
    char _temp=0x1B;
    //UART0_Send(&_temp,1,1);//��������״̬�����ַ�������Ҫ���س�����
    UART0_Send(&_temp,1,0);
    return 0;
}

 


