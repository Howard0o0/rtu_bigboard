//////////////////////////////////////////////////////
//     �ļ���: Sampler.c
//   �ļ��汾: 1.0.0
//   ����ʱ��: 09�� 11��30��
//   ��������:  
//       ����: ����
//       ��ע: ��
//
//////////////////////////////////////////////////////

#include "msp430common.h"

#include "sampler.h"
#include "adc.h" 
#include "rtc.h"
#include "store.h"
#include "common.h" 
#include "stdint.h" 
#include "hydrologycommand.h"
#include "message.h"

//static int s_pulse1_flag=0;
//static int s_pulse1_flag1=0;
//static int s_pulse1_flag2=0;

static int s_pulse2_flag=0;
static int s_pulse3_flag=0;
static int s_pulse4_flag=0;

//static unsigned int  s_pulse1_num=0; //���� 
static unsigned int  s_pulse2_num=0; 
static unsigned int  s_pulse3_num=0; 
static unsigned int  s_pulse4_num=0;


char g_pulse_rate[4]={0,0,0,0}; //  ������
char g_pulse_range[4][3];

//�������� ,  �����ظ�����.�Լ���������ĳ��� 
int s_alert_flag[8];
                  
char alert_str[60];//A,0000,B,0000,C,0000,D,0000,E,0000,F,0000,G,0000,H,0000
int alert_str_idx=0;

int  Sampler_Init()
{//������Ϳ������Ķ˿ڽ��г�ʼ��.
    
//����ΪP2 -->P1    ly 
//����ΪP5 -->P4        
    P1SEL = 0x00;   
    P1DIR = 0x00;
    P4SEL = 0x00;
    
    char _temp = 0x00; 
    
//  ��ȡIO��������
    if(Store_ReadIoDirConfig(&_temp)>=0 ) 
        P4DIR = _temp; 
    printf("");
//  ��ȡIO��ƽ����
    if(Store_ReadIoLevelConfig(&_temp)>=0) 
        P4OUT = _temp; 

    //������ʹֵ��ʧ,�������︴�Ƴ���. 
    
//  ����Ƶ��
    Store_ReadPulseRate(1,&(g_pulse_rate[0]));
    Store_ReadPulseRate(2,&(g_pulse_rate[1]));
    Store_ReadPulseRate(3,&(g_pulse_rate[2]));
    Store_ReadPulseRate(4,&(g_pulse_rate[3]));
    
//  �������ֵ
    Store_ReadPulseRangeBytes(1,g_pulse_range[0]);
    Store_ReadPulseRangeBytes(2,g_pulse_range[1]);
    Store_ReadPulseRangeBytes(3,g_pulse_range[2]);
    Store_ReadPulseRangeBytes(4,g_pulse_range[3]);
    RTC_RetrievePulseBytes();//��ȡ����ֵ
    
    
// 2418 P2����,P2.3 �����ж� ����RTC׼������֮�� . P2.0-2.4�ж�flag high-to-low transition
//5438 P1����  P1.3                                P1.0-1.4
    P1IE = 0xff;
    P1IES = 0xff;
    
    return 0;
}

//����8��IO�ڵĵ�ƽ,��������Ϊ���. ��ʼΪ1
int Sampler_IO_Level(int _ioIdx, int _level)
{
    if( _ioIdx < 1 || _ioIdx>8)
        return -3;
    //��ȡIO����
    char _temp1=0x00; char _temp2=0x00;char _temp3=0x00;
    if(Store_ReadIoDirConfig(&_temp1)<0)
    {//����޷������Ͷ�Ĭ��Ϊ�������.
        //�����ɹ�.
        _temp1=0xFF;
    }
    -- _ioIdx;
    _temp2 = ( 0x01 << _ioIdx );
    if(_temp2 & _temp1==0x00)
    {//��λ��Ϊ���
        return -2;
    }
    
    //������Ҫ��������
    if(Store_ReadIoLevelConfig(&_temp3)<0) 
        return -1; 
    if(_level) 
        _temp3 |= _temp2;
    else
        _temp3 &= ~ _temp2;
    if(Store_SetIoLevelConfig(_temp3)<0)
        return -1;
    
    P4DIR |= _temp2;   //���뻹�����
    //����IO��
    if(_level)         //�Ը�λ��������
        P4OUT |= _temp2;
    else
        P4OUT &= ~ _temp2;
    return 0;
}

int Sampler_Open()
{ 
    //�򿪴�������Դ 
    P9DIR |= BIT3;
    P9OUT |= BIT3; 
    
    
    //��ѹ����
    System_Delayms(500);
    ADC_Open(); //
    return 0;
}

int Sampler_Close()
{
    ADC_Close();
    P9DIR |= BIT3;
    P9OUT &= ~BIT3;
    return 0;
}

int Sampler_Sample()
{
    ADC_Sample();
    return 0;
}
 
int Sampler_GSM_ReadAlertString(char * _dest)
{//�����ͷ���-1
    int  _idx=0; 
    char _tempChar1 = 0x00;
    char _tempChar2 = 0x00;
    char _buffer[3];
    if(Store_ReadAnalogSelect(&_tempChar1)<0)
            return -1;
    //����ģ����
    for(int i=0;i<8;++i)
    {
        if(_tempChar1 & 0x01)
        {//�����λ��
            //�����������������
            _dest[_idx] = 'A' + i ;
            ADC_ReadAnalogStr(i+1, &(_dest[++ _idx]));
            _idx += 4;
        }
        //Ȼ������һ��
        _tempChar1 >>= 1 ;//����һλ
    } 
    //����������
    if(Store_ReadPulseSelect(&_tempChar1)<0)
            return -1;
    for(int i=0;i<4;++i)
    {
        if( _tempChar1 & 0x80)
        {//�����λ�в����Ӹ�λ
            _dest[_idx++] = 'I' + i ; 
            RTC_ReadPulseBytes(i+1,_buffer);
            Utility_CharToHex( _buffer[0],&(_dest[_idx]));
            _idx+=2;
            Utility_CharToHex( _buffer[1],&(_dest[_idx])); 
            _idx+=2;
            Utility_CharToHex( _buffer[2],&(_dest[_idx]));
            _idx+=2;
         }
         _tempChar1 <<=1;
    } 
    //������ 
    _tempChar1=0x01;
    //_tempChar2װ������������
    if(Store_ReadIoSelect(&_tempChar2)<0)
        return -1;
    for(int i=0;i<8;++i)
    {//����8��λ
        if(_tempChar2&0x01)
        {//Ϊ1��λҪ��¼0��1
            _dest[_idx++] = 'M' + i;
            if(P4IN & _tempChar1)
            {
                _dest[_idx++]='1';
            }
            else
            {
                _dest[_idx++]='0';
            }
         }
         //�ж���һ��
         _tempChar1 >>=1;
         _tempChar2 >>=1;
    }
    //����  ����#��,���ͳ����Լ���
    return _idx;
}

int Sampler_DTU_ReadAlertString( char * _dest)
{
    Utility_Strncpy(_dest,alert_str,alert_str_idx);
    return alert_str_idx;
}

//0123456798901234567890123456789012345678901234567890123456789
//3:A=1024;C=1022;H=3333;
//����1��ʾ Ҫ����
//����δѡ���ͨ�� �򲻹���
int Sampler_CheckNormal()
{
    int _max=4096; //���ޱ���
    int _min=0;    //���ޱ���
    char _tempChar1=0x00; //��ʱ����
    int  _tempInt = 0;    //��ʱ����
    
    int _need_alert=0; //�Ƿ���Ҫ����.
    
    alert_str_idx=1;   //������дalert_str;
    alert_str[alert_str_idx++]=':';
    char _alert_num=0;//����ָʾ��������.
    
    
    if(Store_ReadAnalogSelect(&_tempChar1)<0)
    {//�޷�����ѡ��,�͵���ѡ����.
        _tempChar1 = 0xFF;
    }
    //�������μ�����ͨ��
    for(int i=0; i< 8; ++i)
    {
        _tempInt= _tempChar1 & (0x01<<i);
        if(_tempInt==0)
        {//���ͨ��δ��ʹ��
            continue;
        }
        if(Store_ReadDataMinInt( i+1 , &_min)<0 )
            _min=0;
        if(Store_ReadDataMaxInt( i+1 , &_max)<0 )
            _max=4096;
        
        if( A[i] < _min || A[i] > _max)
        {//ֻҪ������Χ�����ڱ�����Ϣ�����¼�¼
            //���ɸ�ͨ���ϵı�����Ŀ,
            ++_alert_num;
            alert_str[alert_str_idx++]='A'+i;
            alert_str[alert_str_idx++]='=';
            _tempInt=A[i];
            Utility_UintToStr4(_tempInt,&alert_str[alert_str_idx]);
            alert_str_idx+=4;
            alert_str[alert_str_idx++]= ';';//��һ��,�Ž��зָ�
            //����ⲿ�ֵ���д
        }
        //������ڼǺź;���
        //�����һ· �������.
        //�ж��Ƿ���Ҫ��� �ظ�������
        if(s_alert_flag[i]>0)
        {
            //���������� ��ʽ����ת�����鷳. , ��Ϊһ������!
            //( A[i] < _max-100)  �� _max-100<0��ʱ�� ,�޷��ŵ�A[i]�������鷳
            if(   (    _max-100>0   &&   A[i]< _max-100 )   ||   (  _max-100<=0 && A[i]<_max ) )
            {
                TraceMsg("A[i] = ",0);TraceInt4(A[i],1);
                TraceMsg("max = ",0);TraceInt4(_max,1);
                TraceMsg("remove alert flag for max-limited.",1);
                s_alert_flag[i]=0;
            }
            continue;
        }
        
        if(s_alert_flag[i]<0)
        {
            if( ( _min+100<4096 &&  A[i] >_min+100) || (_min+100>=4096 && A[i]> _min) )
            {
                TraceMsg("A[i] = ",0);TraceInt4(A[i],1);
                TraceMsg("min = ",0);TraceInt4(_min,1);
                TraceMsg("remove alert flag for min-limited.",1);
                s_alert_flag[i]=0; 
            } 
            continue;
        }
        
        if( A[i] < _min)
        {
            s_alert_flag[i]=-1;//���ñ��,1��ʾ������Ϊ̫С��������
            _need_alert=1;
        }
        if( A[i] > _max)
        {
            s_alert_flag[i]=1;//���ñ��
            _need_alert=1;
        }
    }
    //Ȼ����±�����Ŀ�ַ�.
    alert_str[0] = '0' + _alert_num;
    if(_need_alert)
        return 1;
    else
        return 0;
}

//  ��������Ϊ
//  У���ֽ� 0909011230�ֽ�A1�ֽ�A2�ֽ�B1�ֽ�B2�ֽ�..�ֽ�I1�ֽ�I2...�����ֽ�
//  ����:
//  0           1          2          3     
//  0  1234567890 1234567890123456 789012345678  9
//  У 0909011230 AABBCCDDEEFFGGHH IIIJJJKKKLLL ����
// 
int Sampler_SaveData(char * _saveTime)
{
    char _data[40];  //������Ϊ40
    _data[0] = 0x00; // �ѷ��ͱ�� ��Ϊ0x00 ,
    _data[1] = _saveTime[0]/10 + '0';
    _data[2] = _saveTime[0]%10 + '0';
    _data[3] = _saveTime[1]/10 + '0';
    _data[4] = _saveTime[1]%10 + '0';
    _data[5] = _saveTime[2]/10 + '0';
    _data[6] = _saveTime[2]%10 + '0';
    _data[7] = _saveTime[3]/10 + '0';
    _data[8] = _saveTime[3]%10 + '0';
    _data[9] = _saveTime[4]/10 + '0';
    _data[10] = _saveTime[4]%10 + '0';
    
    _data[11] = A[0] >> 8 ;
    _data[12] = A[0] & 0x00FF ;
    _data[13] = A[1] >> 8 ;
    _data[14] = A[1] & 0x00FF ;
    _data[15] = A[2] >> 8 ;
    _data[16] = A[2] & 0x00FF ;
    _data[17] = A[3] >> 8 ;
    _data[18] = A[3] & 0x00FF ;
    _data[19] = A[4] >> 8 ;
    _data[20] = A[4] & 0x00FF ;
    _data[21] = A[5] >> 8 ;
    _data[22] = A[5] & 0x00FF ;
    _data[23] = A[6] >> 8 ;
    _data[24] = A[6] & 0x00FF ;
    _data[25] = A[7] >> 8 ;
    _data[26] = A[7] & 0x00FF ;
    
    RTC_ReadPulseBytes(1,&(_data[27]));
    RTC_ReadPulseBytes(2,&(_data[30]));
    RTC_ReadPulseBytes(3,&(_data[33]));
    RTC_ReadPulseBytes(4,&(_data[36]));
    char _tempIO=0x00;
    //
    //  �жϿ��ؿ�
    //
    //  ���ڿ��صı�������,
    //  �������ȶ�ȡ ����,
    //  ���������,���Ǳ���  P5IN�ĵ�ƽֵ
    //  ���������,���Ǳ��� ��P5OUT������
    char _dir;
    Store_ReadIoDirConfig(&_dir);
    P4DIR = _dir;//�ٸ���һ��,������ROM����ͬ
    char _level;
    Store_ReadIoLevelConfig(&_level);
    P4OUT = _level;//�ٸ���һ��,������ROM����ͬ
    
    char _bit = BIT0;
    for(int i=0;i<8;++i)
    {
        if(_dir & _bit)
        {//��ʾ��λΪ���
            if(_level & _bit) //����Ϊ�ߵ�ƽ
                _tempIO |=_bit; 
            else
                _tempIO &= ~_bit;
        }
        else
        {//��ʾ��λΪ����
            if(P4IN & _bit)
                _tempIO |= _bit;
            else
                _tempIO &= ~_bit;
        }
        _bit<<=1;
    }
    //Ȼ����д
    _data[39] = _tempIO; 
    //д��_data��
    if(Store_WriteDataItemAuto(_data)<0)
    {
        return -1;
    }
    return 0;
}

long count_pulse1 = 0;
long count_pulse2 = 0;
long count_pulse3 = 0;

char char_1[3] = {0,0,0};
long long_1[3] = {0,0,0};
char char_2[3] = {0,0,0};
long long_2[3] = {0,0,0};
char char_3[3] = {0,0,0};
long long_3[3] = {0,0,0};

int i;

void chartolong(char *arr1, long *arr2)
{
    arr2[0] = (long)arr1[0];
    arr2[1] = (long)arr1[1];
    arr2[2] = (long)arr1[2];
}

void longtochar(char *arr1, long *arr2)
{
    arr1[0] = (char)arr2[0];
    arr1[1] = (char)arr2[1];
    arr1[2] = (char)arr2[2];
}


char ISR_Count_Temp[5] = {0,0,0,0,0};

void ISR_Count_Cal(char* ISR_Count_Arr)
{
  long ISR_Count = 0;
  
  ISR_Count = (ISR_Count_Arr[4] * 0x100000000) + (ISR_Count_Arr[3] * 0x1000000) + (ISR_Count_Arr[2] * 0x10000) + (ISR_Count_Arr[1] * 0x100) + (ISR_Count_Arr[0]);
//  ISR_Count = 255;
  ISR_Count++;
  
  ISR_Count_Arr[4] = (ISR_Count & 0xFF00000000) >> 32;
  ISR_Count_Arr[3] = (ISR_Count & 0x00FF000000) >> 24;
  ISR_Count_Arr[2] = (ISR_Count & 0x0000FF0000) >> 16;
  ISR_Count_Arr[1] = (ISR_Count & 0x000000FF00) >> 8;
  ISR_Count_Arr[0] = (ISR_Count & 0x00000000FF) >> 0;
}

/*��������ж�*/
#pragma vector = PORT1_VECTOR
__interrupt void PORT1_ISR(void) 
{       
  static int a[8] = {0};
    //TraceInt4(i++,1);
    _DINT();
    //����1
    if(P1IFG & BIT0)
    { 
        P1IFG &= ~(BIT0); 
        //���ñ��
        Hydrology_ReadStoreInfo(HYDROLOGY_ISR_COUNT1,ISR_Count_Temp,HYDROLOGY_ISR_COUNT_LEN);
        ISR_Count_Cal(ISR_Count_Temp);
        Hydrology_WriteStoreInfo(HYDROLOGY_ISR_COUNT1,ISR_Count_Temp,HYDROLOGY_ISR_COUNT_LEN);
        TraceMsg("PULS1:",1);
        TraceInt4(a[0]++,1);
    
    }
    //����2 
    if(P1IFG & BIT1)
    {
        P1IFG &= ~(BIT1);
        //�����
        Hydrology_ReadStoreInfo(HYDROLOGY_ISR_COUNT2 + HYDROLOGY_ISR_COUNT_LEN,ISR_Count_Temp,HYDROLOGY_ISR_COUNT_LEN);
        ISR_Count_Cal(ISR_Count_Temp);
        Hydrology_WriteStoreInfo(HYDROLOGY_ISR_COUNT2 + HYDROLOGY_ISR_COUNT_LEN,ISR_Count_Temp,HYDROLOGY_ISR_COUNT_LEN);
        TraceMsg("PULS2:",1);  
        TraceInt4(a[1]++,1);
    }
    //����3
  if(P1IFG & BIT2)
  {
      P1IFG &= ~(BIT2);
        Hydrology_ReadStoreInfo(HYDROLOGY_ISR_COUNT3 + HYDROLOGY_ISR_COUNT_LEN,ISR_Count_Temp,HYDROLOGY_ISR_COUNT_LEN);
        ISR_Count_Cal(ISR_Count_Temp);
        Hydrology_WriteStoreInfo(HYDROLOGY_ISR_COUNT3 + HYDROLOGY_ISR_COUNT_LEN,ISR_Count_Temp,HYDROLOGY_ISR_COUNT_LEN);
        TraceMsg("PULS3:",1); 
        TraceInt4(a[2]++,1);
   }
    //����4 
   if(P1IFG & BIT3)
    {   P1IFG &= ~(BIT3);
        Hydrology_ReadStoreInfo(HYDROLOGY_ISR_COUNT4 + HYDROLOGY_ISR_COUNT_LEN,ISR_Count_Temp,HYDROLOGY_ISR_COUNT_LEN);
        ISR_Count_Cal(ISR_Count_Temp);
        Hydrology_WriteStoreInfo(HYDROLOGY_ISR_COUNT4 + HYDROLOGY_ISR_COUNT_LEN,ISR_Count_Temp,HYDROLOGY_ISR_COUNT_LEN);
        TraceMsg("PULS4:",1); 
        TraceInt4(a[3]++,1);
   }
    //����5
 if(P1IFG & BIT4)
   {
      P1IFG &= ~(BIT4);
        Hydrology_ReadStoreInfo(HYDROLOGY_ISR_COUNT5 + HYDROLOGY_ISR_COUNT_LEN,ISR_Count_Temp,HYDROLOGY_ISR_COUNT_LEN);
        ISR_Count_Cal(ISR_Count_Temp);
        Hydrology_WriteStoreInfo(HYDROLOGY_ISR_COUNT5 + HYDROLOGY_ISR_COUNT_LEN,ISR_Count_Temp,HYDROLOGY_ISR_COUNT_LEN);
        TraceMsg("PULS5:",1);
        TraceInt4(a[4]++,1);
   }
     //����6
    if(P1IFG & BIT5)
   {
       P1IFG &= ~(BIT5);
         Hydrology_ReadStoreInfo(HYDROLOGY_ISR_COUNT6 + HYDROLOGY_ISR_COUNT_LEN,ISR_Count_Temp,HYDROLOGY_ISR_COUNT_LEN);
        ISR_Count_Cal(ISR_Count_Temp);
        Hydrology_WriteStoreInfo(HYDROLOGY_ISR_COUNT6 + HYDROLOGY_ISR_COUNT_LEN,ISR_Count_Temp,HYDROLOGY_ISR_COUNT_LEN);
        TraceMsg("PULS6:",1);
        TraceInt4(a[5]++,1);
   }
    //����7
   if(P1IFG & BIT6)
   {    P1IFG &= ~(BIT6);
         Hydrology_ReadStoreInfo(HYDROLOGY_ISR_COUNT7 + HYDROLOGY_ISR_COUNT_LEN,ISR_Count_Temp,HYDROLOGY_ISR_COUNT_LEN);
        ISR_Count_Cal(ISR_Count_Temp);
        Hydrology_WriteStoreInfo(HYDROLOGY_ISR_COUNT7 + HYDROLOGY_ISR_COUNT_LEN,ISR_Count_Temp,HYDROLOGY_ISR_COUNT_LEN);
        TraceMsg("PULS7:",1);
        TraceInt4(a[6]++,1);
   }
     //����8
    if(P1IFG & BIT7)
   {    P1IFG &= ~(BIT7);
         Hydrology_ReadStoreInfo(HYDROLOGY_ISR_COUNT8 + HYDROLOGY_ISR_COUNT_LEN,ISR_Count_Temp,HYDROLOGY_ISR_COUNT_LEN);
        ISR_Count_Cal(ISR_Count_Temp);
        Hydrology_WriteStoreInfo(HYDROLOGY_ISR_COUNT8 + HYDROLOGY_ISR_COUNT_LEN,ISR_Count_Temp,HYDROLOGY_ISR_COUNT_LEN);
       TraceMsg("PULS8:",1);
        TraceInt4(a[7]++,1);    
    } 
    
    _EINT();
    return ;
}

