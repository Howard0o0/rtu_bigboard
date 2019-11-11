//////////////////////////////////////////////////////
//     �ļ���: rtc.c
//   �ļ��汾: 1.0.0
//   ����ʱ��: 09�� 11��30��
//   ��������:  
//       ����: ����
//       ��ע: ��
//
//////////////////////////////////////////////////////

/*
 * author:  �����
 * time:    2019-08-28
 * changes:  
 *          int  RTC_Open(); 
            char _RTC_Read_OneByte(char regAddr);
            void _RTC_Write_OneByte(char regAddr,char data);
            void _RTC_DisableWrite();
            void _RTC_EnableWrite();
            void _RTC_Go();
            int  _RTC_Check();
            void _RTC_WriteRAM(unsigned char index,const char data);
            char _RTC_ReadRAM(unsigned char index); 
 * abandon:
            void _RTC_DisableWrite();
            void _RTC_EnableWrite();
            void _RTC_EnableCharge();
            void _RTC_DisableCharge(); 
 */
 
#include "msp430common.h"
#include "rtc.h"
#include "common.h"
#include "led.h" 
#include "store.h"
#include "sampler.h"
#include "rom.h"



//�߼�����

extern char g_pulse1_range[3];
extern char g_pulse2_range[3];
extern char g_pulse3_range[3];
extern char g_pulse4_range[3];

//ȫ�ֱ���  ������ʱ��
char g_rtc_nowTime[5]={0,0,0,0,0};




//  �ؼ����ݵı������.�ڴ�һ��,RTCһ��
//  �ڴ��ʼ��ֵȫΪһ����Ȼ�����ֵ.
//  1.д����ʱ,�����ڴ��RTC
//  2.������ʱ,���ڴ� X,  ��RTCֵ Y
//    ��X,Y����ȷ, X!=Y,����XΪ׼,���޸�Y.
//    ��X,Y�Դ���,��������ΪĬ��ֵ
//    ��XΪ����ֵ.Y��ȷ.����YΪ׼,���޸�X.
//  3.RTC��ʼ����ʱ��,����XY��ϵ ����XY.
//

char s_RTC_lastTime[5]={0x00,0x00,0x00,0x00,0x00};
char s_RTC_CheckTime[5]={0x00,0x00,0x00,0x00,0x00};//��ʼ��Ϊ����ֵ
char s_RTC_SaveTime[5]={0x00,0x00,0x00,0x00,0x00};//��ʼ��Ϊ����ֵ
char s_RTC_ReportTime[5]={0x00,0x00,0x00,0x00,0x00};//��ʼ��Ϊ����ֵ
char s_RTC_StartIdx=240;//��ʼ��Ϊ����ֵ  ��ΧΪ1~200
char s_RTC_EndIdx=240;//��ʼ��Ϊ����ֵ    ��ΧΪ1~200

//���������豸����(��Ӧ ��������豸 �����ֵ)
char s_RTC_PulseBytes[4][3]={0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF}; //��һ�ֽ�ΪFF����Ϊ�Ǵ���
//��Ϊ����������ô��, �������֧��7��'9'�Ķ���

//
//  RTC_RetrieveIndex   :    0��ʾδ����   1��ʾ�ѷ���  
//
//   ����         ��һ�����               �ڶ������                ���������      ���������
//    1              0                         1                          0               1  
//    2              0                         1                          0               1
//    3              0                         1                          0               1
//    .              1  <-  _endIdx            0  <- _startIdx            0               1
//    .              1                         0                          0               1
//    .              1                         0                          0               1
//    .              0  <-  _startIdx          1  <- _endIdx              0               1
//   MAX-1           0                         1                          0               1
//   MAX             0                         1                          0               1
//
//  �������Ϳյ��ж�,���� 1�Ƚ� _startIdx �� _endIdx , 2 ���ʱ�ж�_startIdxΪ�ѷ��ͻ���δ����
//  


//
//   RTC���������жϷ����б�����, ���Ҫ���� �жϹر� ���б���
// 

//
//   �ú������ܺܺ�Ӧ�Է���������������δ���͵�����. �����õ�ʵ��.
//  
int RTC_RetrieveIndex()
{
    DownInt();
    char _idx=DATA_MIN_IDX; 
    int _ret=0;
    _ret=Store_CheckDataItemSended(_idx);//ȡ��һ��
    if(_ret==-2)
    {
        UpInt();
        return -1;
    }
    if(_ret==1)
    {//1��ͷ�ĵ�2�����
        while(1)
        {
            _ret=Store_CheckDataItemSended(++ _idx); 
            if(_ret==1)
                continue;
            if( _ret==-2)//���߽���
            {//ȫ1,���� _startIdx = DATA_MIN_IDX   _endIdx = DATA_MIN_IDX
                RTC_SetStartIdx(DATA_MIN_IDX);
                RTC_SetEndIdx(DATA_MIN_IDX);
                UpInt();
                return 0;
            }
            if(_ret==0)
            {//�ҵ�_StartIdx��
                RTC_SetStartIdx(_idx);
                break;
            }
        }
        //��ǰһ��ѭ����������.�������ڿ�ʼѰ��_endIdx ��1����
        while(1)
        {//����1�� ���� _endIdx, 
            _ret=Store_CheckDataItemSended(++ _idx);
            if(_ret == 0)
                continue;
            if(_ret==-2)//���߽���
            {//��_EndIdx ���� DATA_MIN_IDX
                RTC_SetEndIdx(DATA_MIN_IDX);
                UpInt();
                return 0;
            }
            if(_ret==1)
            {
                RTC_SetEndIdx(_idx);
                UpInt();
                return 0;
            }
        }
    }
    else //_ret==0
    {//����ͬ��
        while(1)
        {
            _ret=Store_CheckDataItemSended(++ _idx); 
            if(_ret==0)
                continue;
            if( _ret==-2)//���߽���
            {//ȫ 0 ,���� _startIdx = DATA_MIN_IDX   _endIdx = DATA_MIN_IDX
                RTC_SetStartIdx(DATA_MIN_IDX);
                RTC_SetEndIdx(DATA_MIN_IDX);
                UpInt();
                return 0;
            }
            if(_ret==1)
            {//�ҵ�_endIdx��
                RTC_SetEndIdx(_idx);
                break;
            }
        }
        while(1)
        {//����0��Ϊ  _startIdx
            _ret=Store_CheckDataItemSended(++ _idx); 
            if(_ret==1)
                continue; 
            if(_ret==-2)//���߽���
            {
                RTC_SetStartIdx(DATA_MIN_IDX);
                UpInt();
                return 0;
            }
            if(_ret==0)
            {
                RTC_SetStartIdx(_idx);
                UpInt();
                return 0;
            }
        }
    }
}

int RTC_ReadStartIdx(char *dest)
{
    DownInt();
    if(s_RTC_StartIdx<=DATA_MAX_IDX && s_RTC_StartIdx >= DATA_MIN_IDX)
    {//�ڴ�ֵ��ȷ
        *dest=s_RTC_StartIdx;
        //д��RTC,��֤RTC���ֵ����ȷ
        _RTC_WriteRAM(STARTIDX_ADDR,s_RTC_StartIdx);
        UpInt();
        return 0;
    }
    else
    {//�ڴ�ֵ����
        *dest= _RTC_ReadRAM(STARTIDX_ADDR);
        if((*dest) <= DATA_MAX_IDX && (*dest) >= DATA_MIN_IDX )
        {//RTCֵ������ȷ 
            s_RTC_StartIdx=(*dest); //�����ڴ�ֵ
            UpInt();
            return 0;
        }
        else
        {   
            UpInt();
            return -1;
        }
    }
}

int RTC_SetStartIdx(char src)
{
    DownInt();
    s_RTC_StartIdx=src;//�����ڴ�
    _RTC_WriteRAM(STARTIDX_ADDR,src);//����RTC
    UpInt();
    return 0;
}

int RTC_ReadEndIdx(char *dest)
{
    DownInt();
    if(s_RTC_EndIdx <= DATA_MAX_IDX && s_RTC_EndIdx >= DATA_MIN_IDX )
    {//�ڴ�ֵ��ȷ
        *dest=s_RTC_EndIdx;
        //д��RTC,��֤RTC���ֵ����ȷ
        _RTC_WriteRAM(ENDIDX_ADDR,s_RTC_EndIdx);
        UpInt();
        return 0;
    }
    else
    {//�ڴ�ֵ����
        *dest= _RTC_ReadRAM(ENDIDX_ADDR);
        if((*dest) <= DATA_MAX_IDX && (*dest) >= DATA_MIN_IDX )
        {//RTCֵ������ȷ 
            s_RTC_EndIdx=(*dest); //�����ڴ�ֵ
            UpInt();
            return 0;
        }
        else
        {  
            UpInt();
            return -1;
        }
    } 
}

int RTC_SetEndIdx(const char src)
{
    DownInt();
    s_RTC_EndIdx=src;
    _RTC_WriteRAM(ENDIDX_ADDR,src);
    UpInt();
    return 0;
}
int RTC_ReadCheckTimeBytes(unsigned char index, char *dest, char *lastchecktime)
{
    DownInt();
    if(RTC_IsBadTime(lastchecktime,0)==0)
    {//�ڴ�ֵ����ȷ��.
        dest[0]=lastchecktime[0];
        dest[1]=lastchecktime[1];
        dest[2]=lastchecktime[2];
        dest[3]=lastchecktime[3];
        dest[4]=lastchecktime[4];
        //�����ֵд�뵽RTC��.��֤RTC��Ҳ����ȷ��
        _RTC_WriteRAM(index,lastchecktime[0]);
        _RTC_WriteRAM(index+1,lastchecktime[1]);
        _RTC_WriteRAM(index+2,lastchecktime[2]);
        _RTC_WriteRAM(index+3,lastchecktime[3]);
        _RTC_WriteRAM(index+4,lastchecktime[4]);
        UpInt();
        return 0;
    }
    else
    {//�ڴ�ֵ�Ǵ����.
        dest[0]= _RTC_ReadRAM(index);
        dest[1]=_RTC_ReadRAM(index+1);
        dest[2]=_RTC_ReadRAM(index+2);
        dest[3]=_RTC_ReadRAM(index+3);
        dest[4]=_RTC_ReadRAM(index+4);
        if(RTC_IsBadTime(dest,0)==0)
        {//RTC����ȷ��. 
            lastchecktime[0]=dest[0];
            lastchecktime[1]=dest[1];
            lastchecktime[2]=dest[2];
            lastchecktime[3]=dest[3];
            lastchecktime[4]=dest[4];
            UpInt();
            return 0;
        }
        else
        {//���Ǵ����,�ͷ���-1  
            UpInt();
            return -1;          
        }
    } 
}

int RTC_ReadCheckTimeStr5_A(char * _dest)
{
    DownInt();
    char _temp[5];
    if(RTC_ReadCheckTimeBytes(CHECKTIME_ADDR,_temp,s_RTC_CheckTime))
    {
        UpInt();
        return -1;
    }
    UpInt();
    _dest[0]=_temp[0]/10+'0';
    _dest[1]=_temp[0]%10+'0';
    _dest[2]='/';
    _dest[3]=_temp[1]/10+'0';
    _dest[4]=_temp[1]%10+'0';
    _dest[5]='/';
    _dest[6]=_temp[2]/10+'0';
    _dest[7]=_temp[2]%10+'0';
    _dest[8]='/';
    _dest[9]=_temp[3]/10+'0';
    _dest[10]=_temp[3]%10+'0';
    _dest[11]=':';
    _dest[12]=_temp[4]/10+'0';
    _dest[13]=_temp[4]%10+'0';
    
    return 0;
}

int RTC_SetCheckTimeBytes(unsigned char index, const char *src)
{
    
    s_RTC_CheckTime[0]=src[0];
    s_RTC_CheckTime[1]=src[1];
    s_RTC_CheckTime[2]=src[2];
    s_RTC_CheckTime[3]=src[3];
    s_RTC_CheckTime[4]=src[4];
    DownInt();
    _RTC_WriteRAM(index,src[0]);
    _RTC_WriteRAM(index+1,src[1]);
    _RTC_WriteRAM(index+2,src[2]);
    _RTC_WriteRAM(index+3,src[3]);
    _RTC_WriteRAM(index+4,src[4]);
    UpInt();
    return 0;
}

int RTC_ReadSaveTimeBytes(unsigned char index, char *dest, char *lastsavetime)
{
    DownInt();
    if(RTC_IsBadTime(lastsavetime,0)==0)
    {//�ڴ�ֵ����ȷ��. 
        dest[0]=lastsavetime[0];
        dest[1]=lastsavetime[1];
        dest[2]=lastsavetime[2];
        dest[3]=lastsavetime[3];
        dest[4]=lastsavetime[4];
        
        //�����ֵд�뵽RTC��.��֤RTC��Ҳ����ȷ��
        _RTC_WriteRAM(index,lastsavetime[0]);
        _RTC_WriteRAM(index+1,lastsavetime[1]);
        _RTC_WriteRAM(index+2,lastsavetime[2]);
        _RTC_WriteRAM(index+3,lastsavetime[3]);
        _RTC_WriteRAM(index+4,lastsavetime[4]);
        UpInt();
        return 0;
    }
    else
    {//�ڴ�ֵ�Ǵ����.
        dest[0]= _RTC_ReadRAM(index);
        dest[1]=_RTC_ReadRAM(index+1);
        dest[2]=_RTC_ReadRAM(index+2);
        dest[3]=_RTC_ReadRAM(index+3);
        dest[4]=_RTC_ReadRAM(index+4);
        if(RTC_IsBadTime(dest,0)==0)
        {//RTC����ȷ��. 
            lastsavetime[0]=dest[0];
            lastsavetime[1]=dest[1];
            lastsavetime[2]=dest[2];
            lastsavetime[3]=dest[3];
            lastsavetime[4]=dest[4];
            UpInt();
            return 0;
        }
        else
        {//���Ǵ����,�ͷ���-1  
            UpInt();
            return -1;          
        }
    }
}
int RTC_ReadSaveTimeStr5_A(char *dest)
{
    DownInt();
    char temp[5];
    if(RTC_ReadSaveTimeBytes(SAVETIME_ADDR,temp,s_RTC_SaveTime))
    {
        UpInt();
        return -1;
    }
    UpInt();
    dest[0]=temp[0]/10+'0';
    dest[1]=temp[0]%10+'0';
    dest[2]='/';
    dest[3]=temp[1]/10+'0';
    dest[4]=temp[1]%10+'0';
    dest[5]='/';
    dest[6]=temp[2]/10+'0';
    dest[7]=temp[2]%10+'0';
    dest[8]='/';
    dest[9]=temp[3]/10+'0';
    dest[10]=temp[3]%10+'0';
    dest[11]=':';
    dest[12]=temp[4]/10+'0';
    dest[13]=temp[4]%10+'0';
    
    return 0;
}
int RTC_SetSaveTimeBytes(unsigned char index, const char *src)
{
    
    s_RTC_SaveTime[0]=src[0];
    s_RTC_SaveTime[1]=src[1];
    s_RTC_SaveTime[2]=src[2];
    s_RTC_SaveTime[3]=src[3];
    s_RTC_SaveTime[4]=src[4];
    DownInt();
    _RTC_WriteRAM(index,src[0]);
    _RTC_WriteRAM(index+1,src[1]);
    _RTC_WriteRAM(index+2,src[2]);
    _RTC_WriteRAM(index+3,src[3]);
    _RTC_WriteRAM(index+4,src[4]);
    UpInt();
    return 0;
}

int RTC_ReadReportTimeBytes(unsigned char index, char *dest, char *lastreporttime)
{
    DownInt();
    if(RTC_IsBadTime(lastreporttime,0)==0)
    {//�ڴ�ֵ����ȷ��. 
        dest[0]=lastreporttime[0];
        dest[1]=lastreporttime[1];
        dest[2]=lastreporttime[2];
        dest[3]=lastreporttime[3];
        dest[4]=lastreporttime[4];
        
        //�����ֵд�뵽RTC��.��֤RTC��Ҳ����ȷ��
        _RTC_WriteRAM(index,   lastreporttime[0]);
        _RTC_WriteRAM(index+1, lastreporttime[1]);
        _RTC_WriteRAM(index+2, lastreporttime[2]);
        _RTC_WriteRAM(index+3, lastreporttime[3]);
        _RTC_WriteRAM(index+4, lastreporttime[4]);
        UpInt();
        return 0;
    }
    else
    {//�ڴ�ֵ�Ǵ����.
        dest[0]=_RTC_ReadRAM(index);
        dest[1]=_RTC_ReadRAM(index+1);
        dest[2]=_RTC_ReadRAM(index+2);
        dest[3]=_RTC_ReadRAM(index+3);
        dest[4]=_RTC_ReadRAM(index+4);
        if(RTC_IsBadTime(dest,0)==0)
        {//RTC����ȷ��. 
            lastreporttime[0]=dest[0];
            lastreporttime[1]=dest[1];
            lastreporttime[2]=dest[2];
            lastreporttime[3]=dest[3];
            lastreporttime[4]=dest[4];
            UpInt();
            return 0;
        }
        else
        {//���Ǵ����,�ͷ���-1  
            UpInt();
            return -1;          
        }
    } 
}


int RTC_ReadReportTimeStr5_A(char *dest)
{
    DownInt();
    char temp[5];
    if(RTC_ReadReportTimeBytes(REPORTTIME_ADDR,temp,s_RTC_ReportTime))
    {
        UpInt();
        return -1;
    }
    UpInt();
    dest[0]=temp[0]/10+'0';
    dest[1]=temp[0]%10+'0';
    dest[2]='/';
    dest[3]=temp[1]/10+'0';
    dest[4]=temp[1]%10+'0';
    dest[5]='/';
    dest[6]=temp[2]/10+'0';
    dest[7]=temp[2]%10+'0';
    dest[8]='/';
    dest[9]=temp[3]/10+'0';
    dest[10]=temp[3]%10+'0';
    dest[11]=':';
    dest[12]=temp[4]/10+'0';
    dest[13]=temp[4]%10+'0';
    
    return 0;
}

int RTC_SetReportTimeBytes(unsigned char index, const char *src)
{
    
    s_RTC_ReportTime[0]=src[0];
    s_RTC_ReportTime[1]=src[1];
    s_RTC_ReportTime[2]=src[2];
    s_RTC_ReportTime[3]=src[3];
    s_RTC_ReportTime[4]=src[4];
    DownInt();
    _RTC_WriteRAM(index,src[0]);
    _RTC_WriteRAM(index+1,src[1]);
    _RTC_WriteRAM(index+2,src[2]);
    _RTC_WriteRAM(index+3,src[3]);
    _RTC_WriteRAM(index+4,src[4]); 
    UpInt();
    return 0;
}

int RTC_RetrievePulseBytes()
{
    DownInt();
    s_RTC_PulseBytes[0][0]=_RTC_ReadRAM(PULSE1_BYTE1);
    s_RTC_PulseBytes[0][1]=_RTC_ReadRAM(PULSE1_BYTE2);
    s_RTC_PulseBytes[0][2]=_RTC_ReadRAM(PULSE1_BYTE3);
    s_RTC_PulseBytes[1][0]=_RTC_ReadRAM(PULSE2_BYTE1);
    s_RTC_PulseBytes[1][1]=_RTC_ReadRAM(PULSE2_BYTE2);
    s_RTC_PulseBytes[1][2]=_RTC_ReadRAM(PULSE2_BYTE3);
    s_RTC_PulseBytes[2][0]=_RTC_ReadRAM(PULSE3_BYTE1);
    s_RTC_PulseBytes[2][1]=_RTC_ReadRAM(PULSE3_BYTE2);
    s_RTC_PulseBytes[2][2]=_RTC_ReadRAM(PULSE3_BYTE3);
    s_RTC_PulseBytes[3][0]=_RTC_ReadRAM(PULSE4_BYTE1);
    s_RTC_PulseBytes[3][1]=_RTC_ReadRAM(PULSE4_BYTE2);
    s_RTC_PulseBytes[3][2]=_RTC_ReadRAM(PULSE4_BYTE3);
    UpInt();
     
    return 0;
}
//����1
int RTC_ReadPulseBytes(int _index, char * _dest)
{
    
    if( _index < 1 || _index > 4 )
    { 
        return -2;
    }
    DownInt();
    if(s_RTC_PulseBytes[_index-1][0]==0xFF && s_RTC_PulseBytes[_index-1][1]==0xFF && s_RTC_PulseBytes[_index-1][2]==0xFF)
    {//�ڴ�ֵ����
        s_RTC_PulseBytes[_index-1][0]=_RTC_ReadRAM(PULSE1_BYTE1 + (_index-1)*3 );
        s_RTC_PulseBytes[_index-1][1]=_RTC_ReadRAM(PULSE1_BYTE2 + (_index-1)*3 );
        s_RTC_PulseBytes[_index-1][2]=_RTC_ReadRAM(PULSE1_BYTE3 + (_index-1)*3 );
    }
    _dest[0] = s_RTC_PulseBytes[_index-1][0];
    _dest[1] = s_RTC_PulseBytes[_index-1][1];
    _dest[2] = s_RTC_PulseBytes[_index-1][2];
    UpInt();
    return 0;
}

int RTC_SetPulseBytes(int _index, char * _src)
{
    
    if( _index < 1 || _index > 4 )
    { 
        return -2;
    }
    s_RTC_PulseBytes[_index-1][0] = _src[0];
    s_RTC_PulseBytes[_index-1][1] = _src[1];
    s_RTC_PulseBytes[_index-1][2] = _src[2];
    
    DownInt();
    //��дRTC RAM
    _RTC_WriteRAM(PULSE1_BYTE1 + (_index-1)*3 ,_src[0]);
    _RTC_WriteRAM(PULSE1_BYTE2 + (_index-1)*3 ,_src[1]);
    _RTC_WriteRAM(PULSE1_BYTE3 + (_index-1)*3 ,_src[2]);
    UpInt();
    return 0;
}

//
//  �˺���ֻ���ж�������, ���жϷ����� �ǹر��жϵ�,��˲�����DownInt();
//
int RTC_IncPulseBytes(int _index)
{// �˺��� ���ж��е���,
   // ( ����ִ��ҪѸ��  , �п����Ż�)
    
    if( _index <=0 || _index > 4 )
    {
        return -2;
    } 
    //һ��Ϊ���ٶ�.
    char  _byte1 = s_RTC_PulseBytes[_index-1][0];
    char  _old_byte1 = _byte1;
    char  _byte2 = s_RTC_PulseBytes[_index-1][1];
    char  _old_byte2 = _byte2;
    char  _byte3 = s_RTC_PulseBytes[_index-1][2];
    char  _old_byte3 = _byte3;
    char  _max_byte1 = g_pulse_range[_index-1][0];
    char  _max_byte2 = g_pulse_range[_index-1][1];
    char  _max_byte3 = g_pulse_range[_index-1][2];
   
    if(_byte3==255)
    {//��3�ֽ�����Ļ�
        _byte3=0;
        if( _byte2==255)
        {//��2�ֽ�����Ļ�
            _byte2=0; 
            if(_byte1==255)//��1�ֽ�����Ļ� 
                 _byte1=0; //ʵ������������ܷ���,���Ͷ����.������Ӱ���ٶ�. 
            else 
                ++ _byte1;  
        }
        else 
            ++ _byte2;
    }
    else
        ++ _byte3; 
    //
    //  Ҫ�ж��Ƿ񳬹�MAX��
    // 
    if( _byte1 < _max_byte1)
        goto Update_And_Return;
    if(_byte1 > _max_byte1)
    {//��0
        _byte1=0;_byte2=0;_byte3=0;
        goto Update_And_Return;
    }
    else
    {// _byte1==_max_byte1
        if( _byte2 < _max_byte2)
            goto Update_And_Return;
        if( _byte2 > _max_byte2)
        {
            _byte1=0;_byte2=0;_byte3=0;
            goto Update_And_Return;
        }
        else
        {// _byte2 == _max_byte2
            if( _byte3 > _max_byte3 )
            {
                _byte1=0;_byte2=0;_byte3=0;
                goto Update_And_Return;
            }
            //����С�ڵ��� ���� ������MAX��
        }
    }
Update_And_Return: 
    // Ȼ����� RTC���ֵ.
    // ע��: ����ǰ����߼�,ֻ���޸��� ��λ���ֽ�,��λ���ֽڲ����޸ĵĿ���
    if(_byte3 != _old_byte3)
    {
        s_RTC_PulseBytes[_index-1][2] = _byte3;
        _RTC_WriteRAM(PULSE1_BYTE3 + (_index-1)*3 ,_byte3 );
        if(_byte2 != _old_byte2)
        {
            s_RTC_PulseBytes[_index-1][1] = _byte2;
            _RTC_WriteRAM(PULSE1_BYTE2 + (_index-1)*3 ,_byte2 );
            if(_byte1 != _old_byte1)
            {
                s_RTC_PulseBytes[_index-1][0] = _byte1;
                _RTC_WriteRAM(PULSE1_BYTE1 + (_index-1)*3 ,_byte1 );
            }
        }
    }
    return 0;
}

 
int  RTC_IsBadTime(char * time, int isTime)
{//�޴�����0,���򷵻�-1.
  //�ж�ԭ��
  //  ������ݱ���Ҫ���ڵ���09��
  //  ��ݱ���С��20��, 
  //  
  //  �·ݱ���Ϊ��1~12��֮�� 
  //  ���ڱ���Ϊ��1~31֮��
  //  ʱ������0~23֮��
  //  �ֱ�����0~59֮��
  //
  //
   if(time[0]<9)
       return -1;
   if(time[0]>30)//30��
       return -1;
   if(time[1]<1)
       return -1;
   if(time[1]>12)
       return -1;
   
   if(time[2]<1)
       return -1;
   if(time[2]>31)
       return -1;
   if(time[3]>23)
       return -1;
   if(time[4]>59)
       return -1; 
   
   
   
   if(isTime==0)
   {
     return 0;
   }
   
   //��ǰһ��ʱ����жԱ�
   //����̫������ʱ�Ͳ��Ƚ���.
   //�������lastTime�Լ�Ҫ����ȷ��
   //��Ҫ������������������޸�.���ʷǳ���С,�����Ƿ�����
   //������Ҫ�ں� ��  �� �� ʱ
   if(s_RTC_lastTime[0] >= 9  && s_RTC_lastTime[1] <= 12 && s_RTC_lastTime[1] >= 1
      && s_RTC_lastTime[3]<=23)
   {
     //��ʼ�Ƚ�
     
     //��
     if( time[0]!=s_RTC_lastTime[0] && time[0]!=s_RTC_lastTime[0]+1)
     {//����Ϊ���� Ҳ��Ϊȥ��.��ô���Ǹ�����
       return -1;
     }
     
     //��
     //����1�µ����
     if( time[1]!=s_RTC_lastTime[1] && time[1]!=s_RTC_lastTime[1]+1)
     {//����Ϊ����,Ҳ��Ϊǰһ����,
       //����Ϊ1�µ����,����ǰһ����Ϊ12��
       //�����²�Ϊ1��, Ϊ1��ʱ,last��Ϊ12.
       if(time[1]!=1)
       {
         return -1;
       }
       else
       {
         if(s_RTC_lastTime[1]!=12)
         {
           return -1;
         }
       }
     }
     
     //ʱ
     //����00ʱ�����
     if( time[3]!=s_RTC_lastTime[3] && time[3]!=s_RTC_lastTime[3]+1)
     {
       if(time[3]!=0)
       {
         return -1;
       }
       else
       {
         if(s_RTC_lastTime[3]!=23)
         {
           return -1;
         }
       }
     }
   }
   //��ʱ������Ϊ��ǰʱ������ȷ��
   //��������
   for(int i=0;i<5;++i)
     s_RTC_lastTime[i]=time[i];
   return 0;
}



//�ж�ʱ���Ƿ��ǹ�ȥ ,��ʽΪ ������ʱ�� ��char[5]���� 

// 1   Ϊ��ȥʱ��
// 0   Ϊ����ʱ��
// -1  Ϊδ��ʱ��  

int  RTC_IsPassed(char * time)
{   
    if( g_rtc_nowTime[0] != time[0])
    {
       if(g_rtc_nowTime[0]>time[0])
          return 1;
       else
          return -1;
    } 
    if( g_rtc_nowTime[1] != time[1])
    { 
       if(g_rtc_nowTime[1] > time[1])
          return 1;
       else
          return -1;
    } 
    if( g_rtc_nowTime[2] != time[2])
    { 
       if(g_rtc_nowTime[2] > time[2])
          return 1;
       else
          return -1;
    }  
    if( g_rtc_nowTime[3] != time[3])
    { 
       if( g_rtc_nowTime[3] > time[3])
          return 1;
       else
          return -1;
    } 
    if( g_rtc_nowTime[4] != time[4])
    {
       if( g_rtc_nowTime[4] > time[4])
          return 1;
       else
          return -1;
    }
    else
       return 0; //������һ�� 
}   

//����  09/03/05/23:00:00 ��ʽ���ַ���
void RTC_ReadTimeStr6_A(char * dest)
{ 
    char year;
    char day;
    char month;
    char date;
    char hour;
    char minute;
    char second;
    char control;
    DownInt();
    _RTC_ReadTime(&second,&minute,&hour,&date,&month,&day,&year,&control); 
    UpInt();
    dest[0]=year/10+'0';
    dest[1]=year%10+'0';
    dest[2]='/'; 
    dest[3]=month/10+'0';
    dest[4]=month%10+'0';
    dest[5]='/'; 
    dest[6]=date/10+'0';
    dest[7]=date%10+'0';
    dest[8]='/'; 
    dest[9]=hour/10+'0';
    dest[10]=hour%10+'0';
    dest[11]=':';
    dest[12]=minute/10+'0';
    dest[13]=minute%10+'0'; 
    dest[14]=':';
    dest[15]=second/10+'0';
    dest[16]=second%10+'0';
    
} 
//      01234567 8 9 10  11 12 13  
//����  09/03/05   2  3  :  0  0 ��ʽ���ַ���
void RTC_ReadTimeStr5_A(char * dest)
{ 
    
    char year;
    char day;
    char month;
    char date;
    char hour;
    char minute;
    char second;
    char control;
    DownInt();
    _RTC_ReadTime(&second,&minute,&hour,&date,&month,&day,&year,&control);
    UpInt();
    dest[0]=year/10+'0';
    dest[1]=year%10+'0';
    dest[2]='/';
    dest[3]=month/10+'0';
    dest[4]=month%10+'0';
    dest[5]='/';
    dest[6]=date/10+'0';
    dest[7]=date%10+'0';
    dest[8]='/';
    dest[9]=hour/10+'0';
    dest[10]=hour%10+'0';
    dest[11]=':';
    dest[12]=minute/10+'0';
    dest[13]=minute%10+'0';
    
}


//12���ַ�.
void RTC_ReadTimeStr6_B(char * dest)
{   
    char year;
    char day;
    char month;
    char date;
    char hour;
    char minute;
    char second;
    char control;
    DownInt();
    _RTC_ReadTime(&second,&minute,&hour,&date,&month,&day,&year,&control);
    UpInt();
    dest[0]=year/10+'0';
    dest[1]=year%10+'0'; 
    dest[2]=month/10+'0';
    dest[3]=month%10+'0'; 
    dest[4]=date/10+'0';
    dest[5]=date%10+'0'; 
    dest[6]=hour/10+'0';
    dest[7]=hour%10+'0'; 
    dest[8]=minute/10+'0';
    dest[9]=minute%10+'0';  
    dest[10]=second/10+'0';
    dest[11]=second%10+'0';
    
}
void RTC_ReadTimeStr5_B(char * dest)
{  
    
    char year;
    char day;
    char month;
    char date;
    char hour;
    char minute;
    char second;
    char control;
    DownInt();
    _RTC_ReadTime(&second,&minute,&hour,&date,&month,&day,&year,&control); 
    UpInt();
    dest[0]=year/10+'0';
    dest[1]=year%10+'0'; 
    dest[2]=month/10+'0';
    dest[3]=month%10+'0'; 
    dest[4]=date/10+'0';
    dest[5]=date%10+'0'; 
    dest[6]=hour/10+'0';
    dest[7]=hour%10+'0'; 
    dest[8]=minute/10+'0';
    dest[9]=minute%10+'0';
    
}
//01234567890123456
//09/10/14/10:00:00
void RTC_SetTimeStr6_A(const char * src)
{
    DownInt();
    _RTC_SetTime( (src[15]-'0')*10 + src[16]-'0',(src[12]-'0')*10 + src[13]-'0',
                 (src[9]-'0')*10 + src[10]-'0',(src[6]-'0')*10+src[7]-'0',
                 (src[3]-'0')*10+src[4]-'0',1,(src[0]-'0')*10 +src[1]-'0',0);
    UpInt();
}

void RTC_SetTimeStr6_B(const char *src)
{
    DownInt();
    _RTC_SetTime((src[10]-'0')*10+src[11]-'0', (src[8]-'0')*10+src[9]-'0',
              (src[6]-'0')*10+src[7]-'0' ,(src[4]-'0')*10+src[5]-'0' ,
              (src[2]-'0')*10+src[3]-'0' ,1,(src[0]-'0')*10+src[1]-'0',0);
    UpInt();
}
//�� ������������ʱʱ�ַ� �ĸ�ʽ������ʱ��
//   0 1 2 3 4 5 6 7 8 9
//  090509101123
// ���� ȫ������Ϊ1,����1
void RTC_SetTimeStr5_B(const char *src)
{ 
    DownInt();
    char second=_RTC_ReadSecond();
    _RTC_SetTime(second , (src[8]-'0')*10+src[9]-'0',
              (src[6]-'0')*10+src[7]-'0' ,(src[4]-'0')*10+src[5]-'0' ,
              (src[2]-'0')*10+src[3]-'0' ,1,(src[0]-'0')*10+src[1]-'0',0);
    UpInt();
}
void RTC_ReadTimeBytes5(char * dest)
{
    
    char second;
    char control;
    char day;
    DownInt();
    _RTC_ReadTime(&second,&(dest[4]),&(dest[3]),&(dest[2]),&(dest[1]),&day,&(dest[0]),&control);
    UpInt();
}
int RTC_SetTimeBytes5(const char * src)
{
    DownInt();
    char second = _RTC_ReadSecond();
    int ret=_RTC_SetTime(second,src[4],src[3],src[2],src[1],1,src[0],0); 
    UpInt();
    return ret;
}
void RTC_ReadTimeBytes6(char * dest )
{
#if 0

    char year;
    char day;
    char month;
    char date;
    char hour;
    char minute;
    char second;
    char control;
    DownInt();
    _RTC_ReadTime(&second,&minute,&hour,&date,&month,&day,&year,&control); 
    UpInt();
    dest[0]=year/10+'0';
    dest[1]=year%10+'0';

    dest[2]=month/10+'0';
    dest[3]=month%10+'0';

    dest[4]=date/10+'0';
    dest[5]=date%10+'0';

    dest[6]=hour/10+'0';
    dest[7]=hour%10+'0';

    dest[8]=minute/10+'0';
    dest[9]=minute%10+'0'; 

    dest[10]=second/10+'0';
    dest[11]=second%10+'0';
#else
    char control;
    char day;
    DownInt();

    _RTC_ReadTime(&(dest[5]),&(dest[4]),&(dest[3]),&(dest[2]),&(dest[1]),&day,&(dest[0]),&control);

    UpInt();
#endif
}
int RTC_SetTimeBytes6(const char * src)
{ 
    DownInt();
    int ret= _RTC_SetTime(src[5],src[4],src[3],src[2],src[1],1,src[0],0); 
    UpInt();
    return ret;
}
 


//��������õ�ʱ�䣺�롢�֡�ʱ���ա��¡����ڡ��ꡢ������
int _RTC_SetTime(const char second,const char minute,
                 const char hour,const char date,const char month,
                 const char day,const char year,const char control)   //����99��12��31�� ����7 23��59��00��
{
  if(second >59 || minute >59 || hour >23|| day>7 || day ==0 || date>31 
     || date==0 || month >12 || month ==0 || year >99)
    return -1;
  char temp[8];
  temp[0]=_DECtoBCD(second);
  temp[1]=_DECtoBCD(minute);
  temp[2]=_DECtoBCD(hour);
  temp[3]=_DECtoBCD(date);
  temp[4]=_DECtoBCD(month);
  temp[5]=_DECtoBCD(day);
  temp[6]=_DECtoBCD(year);
  temp[7]=_DECtoBCD(control);
  
//  _RTC_EnableWrite();
//  _RTC_MultiWrite(CMD_WRITE_BATCH,temp,8);
//  _RTC_DisableWrite();
  _RTC_Write_OneByte(RegAddr_Sec,temp[0]);
  _RTC_Write_OneByte(RegAddr_Min,temp[1]);
  _RTC_Write_OneByte(RegAddr_Hour,temp[2]);
  _RTC_Write_OneByte(RegAddr_Day,temp[5]);
  _RTC_Write_OneByte(RegAddr_Date,temp[3]);
  _RTC_Write_OneByte(RegAddr_CMon,temp[4]);
  _RTC_Write_OneByte(RegAddr_Year,temp[6]);
  
  
  
  //ʱ�������ڷŵ�s_RTC_lastTime�ﱣ��һ��
  
  s_RTC_lastTime[0]=year;
  s_RTC_lastTime[1]=month;
  s_RTC_lastTime[2]=date;
  s_RTC_lastTime[3]=hour;
  s_RTC_lastTime[4]=minute;
  return 0;
} 


char _RTC_ReadSecond()
{
  char temp;
  //_RTC_MultiRead(CMD_READ_SECOND,&temp,1);
  temp = _RTC_Read_OneByte(RegAddr_Sec);
  return  _BCDtoDEC(temp);
}



void _RTC_ReadTime(char *second,char *minute,
                 char *hour,char *date,char *month,
                 char *day, char *year,char *control)
{
  char temp[8];
//  _RTC_MultiRead(CMD_READ_BATCH,temp,8);
  
  temp[0] = _RTC_Read_OneByte(RegAddr_Sec);
  temp[1] = _RTC_Read_OneByte(RegAddr_Min);
  temp[2] = _RTC_Read_OneByte(RegAddr_Hour);
  temp[5] = _RTC_Read_OneByte(RegAddr_Day);
  temp[3] = _RTC_Read_OneByte(RegAddr_Date);
  temp[4] = _RTC_Read_OneByte(RegAddr_CMon);
  temp[6] = _RTC_Read_OneByte(RegAddr_Year);
  
  temp[0] &= ~BIT7;
  *second =_BCDtoDEC(temp[0]);
  *minute =_BCDtoDEC(temp[1]);
  *hour =_BCDtoDEC(temp[2]);
  *date =_BCDtoDEC(temp[3]);
  *month =_BCDtoDEC(temp[4]);
  *day =_BCDtoDEC(temp[5]);
  *year =_BCDtoDEC(temp[6]);
  *control =_BCDtoDEC(temp[7]);
}
/*
void RTC_ReadRTCTimeBytes(char * dest)
{
  char temp[8];
  _RTC_MultiRead(CMD_READ_BATCH,temp,8);
  temp[0] &= ~BIT7;
  dest[0]=_BCDtoDEC(temp[0]);
  dest[1]=_BCDtoDEC(temp[1]);
  dest[2]=_BCDtoDEC(temp[2]);
  dest[3]=_BCDtoDEC(temp[3]);
  dest[4]=_BCDtoDEC(temp[4]);
  dest[5]=_BCDtoDEC(temp[5]);
  dest[6]=_BCDtoDEC(temp[6]);
  dest[7]=_BCDtoDEC(temp[7]);
} 
*/
int _RTC_SendByte(char data)
{
    _RTC_Set_IO_OUT(); //io������Ϊ�����
      
    for(int i=0;i<8;i++)
    {  
       //�ӵ͵��� ���͵� IO��
        if(data & 0x01)
        {  
          _RTC_Set_IO_high(); 
        }
        else
        {
          _RTC_Set_IO_low();
        }
        // IO���ϵĵ�ƽȷ���ú�
        //��ʱ������һ�����������壬DS1302�ͻ�ȥ���ո�����,
        //ʱ���½��أ�����������Ч
        
        _RTC_Set_SCLK_low();
        _RTC_Set_SCLK_low();
        _RTC_Set_SCLK_high();
        _RTC_Set_SCLK_high();
        
        //�ȴ�DS1302 �����������
         _RTC_delay(40); 
         
        //��λ
        data>>=1;
    } 
    return 0;
}

char _RTC_RecvByte(void)
{ 
    char temp=0;
    //��IO�ߵ��������Ϊ�ߣ�
    //  ���԰�������DS1302����IO�ߵĵ�ƽ
    _RTC_Set_IO_high();
    //io������Ϊ���룬��������
    _RTC_Set_IO_IN();
   
    for(int i=0; i<8;++i)
    { 
        //   ��ʱ������һ���½������壬���������Ѿ�����,
       
        //      DS1302���������һλ���ݵ� IO�� ��
        
        //�����½�����ʱһ��  �Ϳ�ʼ���� ������
        _RTC_delay(40);
        _RTC_Set_SCLK_high();   
        _RTC_Set_SCLK_high();
        _RTC_Set_SCLK_low();
        _RTC_Set_SCLK_low();
     
        //�ܹ�ֻ����7λ��������8�����Է��ڶ�ȡ֮ǰ
        temp >>=1;
        // ���Ͷ������ֽں�IO���Ͼͻ�������
        // ��IO��һ������ȡ���ӵ͵��ߣ����һ��д�����λ��
        if((P1IN & RTC_IO)==RTC_IO)
        {
            temp |=0x80;
        }
        // IO���ϵĵ�ƽ��ȡ���֮��
    }
    return temp;
}

/*
 * function: read one byte from RTC
 * params: regAddr=>RTC reg address
 * return: char 
*/
char _RTC_Read_OneByte(char regAddr)
{
    char read_data;
    
    //start 
    IIC_Start(); 
    //send write cmd
    IIC_Send_Byte(WRITE_CMD);   
    //wait ack
    IIC_Wait_Ack();
    //send reg addr
    IIC_Send_Byte(regAddr);   
    //wait ack
    IIC_Wait_Ack();	
    //iic_start
    IIC_Start(); 
    //send read cmd
    IIC_Send_Byte(READ_CMD);          
    //wait ack
    IIC_Wait_Ack();
    //receive data byte
    read_data=IIC_Read_Byte(0);
    //iic stop
    IIC_Stop();
    
    return read_data;
 
}

/*
 * func: write one byte into RTC 
*/
void _RTC_Write_OneByte(char regAddr,char data)
{
     //start 
    IIC_Start(); 
    //send write cmd
    IIC_Send_Byte(WRITE_CMD);   //����������ַ0XA0,д���� 
    //wait ack
    IIC_Wait_Ack();
    //send reg addr
    IIC_Send_Byte(regAddr);   //���͵͵�ַ
    //wait ack
    IIC_Wait_Ack();	
    //send write_data
    IIC_Send_Byte(data);
    //wait ack
    IIC_Wait_Ack();
    //iic stop
    IIC_Stop();
  
}


int _RTC_MultiRead(char cmd, char * dest, int length)
{
    //��DS1302,֮ǰ�� ʱ��������Ϊ��
    //_RTC_Set_RST_low(); 
    _RTC_Set_SCLK_low();
    _RTC_Set_SCLK_low();
    //////////_RTC_Set_RST_high();
    //��������
    _RTC_SendByte(cmd);
   //��ʼ��������
    for(int i=0;i<length;++i)
    {
       dest[i]=_RTC_RecvByte();
     }
    //����֮��ر�DS1302 
    //_RTC_Set_RST_low();
    return 0;
}

int _RTC_MultiWrite(char cmd ,const char * src, int length)
{
    //��DS1302,֮ǰ�� ʱ��������Ϊ��
    
    
    //_RTC_Set_RST_low(); 
    _RTC_Set_SCLK_low();
    _RTC_Set_SCLK_low();
    //////////_RTC_Set_RST_high();
    //����cmd
    _RTC_SendByte(cmd);
    //������������
    for(int i=0;i<length;++i)
    {
       _RTC_SendByte(src[i]);
    }
    //д��֮��ر�DS1302
    //RTC_Set_RST_low(); 
    
    //_RTC_Set_RST_low();
    
     
   // _ENT();
    return 0;
}

/* not used in RTC DS3231 */
void _RTC_DisableWrite()
{
//    char temp=0x80;
//    _RTC_MultiWrite(CMD_WRITE_CONTROL,&temp,1);
}
/* not used in RTC DS3231 */
void _RTC_EnableWrite()
{
//    char temp=0x00;
//    _RTC_MultiWrite(CMD_WRITE_CONTROL,&temp,1);
}
void _RTC_Go()
{  
    //Ӧ���ȶ������ӣ������λ����д��ȥ���������Ӳ����
    char temp;
    //_RTC_MultiRead(CMD_READ_SECOND,&temp,1);
    temp = _RTC_Read_OneByte(RegAddr_Sec);
    temp &= ~BIT7;  
    _RTC_EnableWrite();
    //_RTC_MultiWrite(CMD_WRITE_SECOND,&temp,1);
    _RTC_Write_OneByte(RegAddr_Sec,temp);
    _RTC_DisableWrite();
}
void _RTC_Pause()
{
    //Ӧ���ȶ������ӣ����ø�λ����д��ȥ���������Ӳ����
    char temp;
    //_RTC_MultiRead(CMD_READ_SECOND,&temp,1);
    temp = _RTC_Read_OneByte(RegAddr_Sec);
    temp |= BIT7;
    _RTC_EnableWrite();
    //_RTC_MultiWrite(CMD_WRITE_SECOND,&temp,1);
    _RTC_Write_OneByte(RegAddr_Sec,temp);
    _RTC_DisableWrite();
}

/*
 * func: check if Oscillator stop
 * return: Oscillator stop=>1/2  Normal=>0
*/
int _RTC_Check()
{
    //RegAddr_Control,// Control
    //RegAddr_CtlStat,// Control/Status
    char temp;
    
    if(_RTC_Read_OneByte(RegAddr_Control) & 0x80) //����ֹͣ������
        return 1;
    else if(_RTC_Read_OneByte(RegAddr_CtlStat) & 0x80) //���� EOSC����ֹ��
        return 2;
    else                //normal
        return 0;
}
// void _RTC_DisableCharge()
// {
//     char temp=0x50;
//     _RTC_EnableWrite();
//     _RTC_MultiWrite(CMD_WRITE_CHARGE,&temp,1);
//     _RTC_DisableWrite();
// }
// void _RTC_EnableCharge()
// {
//   char temp=0xA5;
//   _RTC_EnableWrite();
//   _RTC_MultiWrite(CMD_WRITE_CHARGE,&temp,1);
//   _RTC_DisableWrite();
// }

/*
 * �浽RTC_RAM�е�ֵ:
 * StartIdx,EndIdx   ==>  ����״̬���еĶ��׶�β
 * LastCheckTime  ==>   ȫ�ֱ������ڴ�����һ�ݣ�RTC��RAM��Ҳ��һ��
 * NextCheckTime   ==>  ��һ�μ��ʱ��
 * 
*/

/*
 * instruction: there is no RAM in DS3231,assign 31 bytes from EEPROM as RTC_RAM
 * param: index=>(RAM offset,range from 0 to 30)
*/
void _RTC_WriteRAM(unsigned char index,const char data)
{
    if(index > 30)
        return;
//   _RTC_EnableWrite();
//   _RTC_MultiWrite(RTC_RAM_BASE+index,&data,1);
//   _RTC_DisableWrite();
    
    ROM_WriteByte_RTC(RTC_RAM_BASE+(long)index,data); 

}

/*
 * instruction: there is no RAM in DS3231,assign 31 bytes from EEPROM as RTC_RAM
 * param: index=>(RAM offset,range from 0 to 30)
*/
char _RTC_ReadRAM(unsigned char index)
{
  if(index >30)
    return 0;
  char temp;
//   _RTC_MultiRead(CMD_READ_RAM_BASE+(index<<1),&temp,1);
  ROM_ReadByte(RTC_RAM_BASE+(long)index,&temp);
  return temp;
}
  
// �ͼ����� ��  ��������
char _BCDtoDEC(char val)
{
    val = (val >> 4) *10+(val &0x0f); ///��BCD��ת��Ϊ10������
    return val; ///����10������
}
char _DECtoBCD(char val)
{
  return ((val / 10) *16+val % 10);
}
void _RTC_delay(int j)
{ //ʱ�򳤶̺���Ҫ
  for(int i=0 ;i <j;i++);//��ǰΪ20
}

/*
 * function: init pins,enable RTC inside Oscillator
 * pins: P9.2=>RTC_SCLK  P9.1=>RTC_SDA
*/ 
int RTC_Open()
{  
  
    /* set pins */
    DownInt();
    System_Delayms(20);
    P9DIR |= RTC_SCLK; // set p9.2 output
    P9DIR |= RTC_IO;   // set p9.1 output
    IIC_SCL_HIGH();
    IIC_SDA_HIGH();
    System_Delayms(500);  //delay to wait RTC ready
    UpInt();

    /* enable Oscillator and clear flags*/
    _RTC_Write_OneByte(RegAddr_Control,0);
    _RTC_Write_OneByte(RegAddr_CtlStat,0);
    
    return 0;
}


int RTC_Close()
{  
    return 0;
}

void _RTC_Set_RST_low()
{ 
    P4OUT &= ~(RTC_RST);   
    _RTC_delay(20); 
    return ;
}
void _RTC_Set_RST_high()
{   
    P4OUT |= RTC_RST;  
    _RTC_delay(20);
    return ;
}
void _RTC_Set_SCLK_low()
{
    
    P1OUT &= ~(RTC_SCLK);
    _RTC_delay(10);
    return ;
}
void _RTC_Set_SCLK_high()
{
    P1OUT |= RTC_SCLK;
    _RTC_delay(10);
    return ;
}
void _RTC_Set_IO_low()
{
    P1OUT &= ~(RTC_IO);
    _RTC_delay(10);
    return ;
}
void _RTC_Set_IO_high()
{
    P1OUT |= RTC_IO;
    _RTC_delay(10);
    return ;
} 
void _RTC_Set_IO_IN()
{
    P1DIR &= ~RTC_IO ;
    _RTC_delay(10);
}
void _RTC_Set_IO_OUT()
{
    P1DIR |= RTC_IO ; 
    _RTC_delay(10);
}

/* IIC ���� */

void IIC_SCL_HIGH(void)
{
    P9OUT |= 1<<2;
}

void IIC_SCL_LOW(void)
{
    P9OUT &= ~(1<<2);
}

void IIC_SDA_HIGH(void)
{
    P9OUT |= 1<<1;
}

void IIC_SDA_LOW(void)
{
    P9OUT &= ~(1<<1);
}

int READ_SDA(void)
{
  
    if(P9IN & (1<<1))
        return 1;
    else
        return 0;
}

//��ʼ��IIC
void IIC_Init(void)
{					     
//	RCC->AHB1ENR|=1<<7;    //ʹ��PORTHʱ��	   	  
//	GPIO_Set(GPIOH,PIN4|PIN5,GPIO_MODE_OUT,GPIO_OTYPE_PP,GPIO_SPEED_50M,GPIO_PUPD_PU);//PH4/PH5���� 
//	IIC_SCL_HIGH();
//	IIC_SDA_HIGH();
}
//����IIC��ʼ�ź�
void IIC_Start(void)
{
	SDA_OUT();     //sda�����
	IIC_SDA_HIGH();	  	  
	IIC_SCL_HIGH();
	Delay_us(4);
 	IIC_SDA_LOW();//START:when CLK is high,DATA change form high to low 
	Delay_us(4);
	IIC_SCL_LOW();//ǯסI2C���ߣ�׼�����ͻ�������� 
}	  
//����IICֹͣ�ź�
void IIC_Stop(void)
{
	SDA_OUT();//sda�����
	IIC_SCL_LOW();
	IIC_SDA_LOW();//STOP:when CLK is high DATA change form low to high
 	Delay_us(4);
	IIC_SCL_HIGH(); 
	IIC_SDA_HIGH();//����I2C���߽����ź�
	Delay_us(4);							   	
}
//�ȴ�Ӧ���źŵ���
//����ֵ��1������Ӧ��ʧ��
//        0������Ӧ��ɹ�
u8 IIC_Wait_Ack(void)
{
	u8 ucErrTime=0;
	SDA_IN();      //SDA����Ϊ����  
	IIC_SDA_HIGH();Delay_us(1);	   
	IIC_SCL_HIGH();Delay_us(1);	 
	while(READ_SDA())
	{
		ucErrTime++;
		if(ucErrTime>250)
		{
			IIC_Stop();
			return 1;
		}
	}
	IIC_SCL_LOW();//ʱ�����0 	   
	return 0;  
} 
//����ACKӦ��
void IIC_Ack(void)
{
	IIC_SCL_LOW();
	SDA_OUT();
	IIC_SDA_LOW();
	Delay_us(2);
	IIC_SCL_HIGH();
	Delay_us(2);
	IIC_SCL_LOW();
}
//������ACKӦ��		    
void IIC_NAck(void)
{
	IIC_SCL_LOW();
	SDA_OUT();
	IIC_SDA_HIGH();
	Delay_us(2);
	IIC_SCL_HIGH();
	Delay_us(2);
	IIC_SCL_LOW();
}					 				     
//IIC����һ���ֽ�
//���شӻ�����Ӧ��
//1����Ӧ��
//0����Ӧ��			  
void IIC_Send_Byte(u8 txd)
{                        
    u8 t;   
    SDA_OUT(); 	    
    IIC_SCL_LOW();//����ʱ�ӿ�ʼ���ݴ���
    for(t=0;t<8;t++)
    {   
        if((txd&0x80)>>7)
            IIC_SDA_HIGH();
        else
            IIC_SDA_LOW();
        txd<<=1; 	  
        Delay_us(2);   //��TEA5767��������ʱ���Ǳ����
        IIC_SCL_HIGH();
        Delay_us(2); 
        IIC_SCL_LOW();	
        Delay_us(2);
    }	 
} 	    
//��1���ֽڣ�ack=1ʱ������ACK��ack=0������nACK   
u8 IIC_Read_Byte(unsigned char ack)
{
	unsigned char i,receive=0;
	SDA_IN();//SDA����Ϊ����
    for(i=0;i<8;i++ )
	{
        IIC_SCL_LOW(); 
        Delay_us(2);
        IIC_SCL_HIGH();
        receive<<=1;
        if(READ_SDA())receive++;   
        Delay_us(1); 
    }					 
    if (!ack)
        IIC_NAck();//����nACK
    else
        IIC_Ack(); //����ACK   
    return receive;
}
