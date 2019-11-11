#include "msp430common.h"
#include "common.h"
#include "hydrologytask.h"
#include "stdint.h"
#include "rtc.h"
#include "main.h"
#include "common.h"
#include "math.h"
#include "store.h"
#include "sampler.h"
#include "adc.h"
#include "packet.h"
//#include "convertsampledata.h"
//#include "hydrology.h"
//#include "hydrologmakebody.h"
#include "hydrologycommand.h"
#include "GTM900C.h"
#include "message.h"
#include "uart3.h"
#include "uart_config.h"
#include "timer.h"
#include "string.h"

//char LinkMaintenance_Flag = 0;
//char Test_Flag = 0;
//char EvenPeriodInformation_Flag = 0;
//char TimerReport_Flag = 0;
//char AddReport_Flag = 0;
//char Hour_Flag = 1;
//char ArtificialNumber_Flag = 0;
//char Picture_Flag = 0;
//char Realtime_Flag = 0;
//char Period_Flag = 0;
//char InquireArtificialNumber_Flag = 0;
//char SpecifiedElement_Flag = 0;
//char ConfigurationModification_Flag = 0;
//char ConfigurationRead_Flag = 0;
//char ParameterModification_Flag = 0;
//char ParameterRead_Flag = 0;
//char WaterPumpMotor_Flag = 0;
//char SoftwareVersion_Flag = 0;
//char Status_Flag = 0;
//char InitializeSolidStorage_Flag = 0;
//char Reset_Flag = 0;
//char ChangePassword_Flag = 0;
//char SetClock_Flag = 0;
//char SetICCard_Flag = 0;
//char Pump_Flag = 0;
//char Valve_Flag = 0;
//char Gate_Flag = 0;
//char WaterSetting_Flag = 0;
//char Record_Flag = 0;
//char Time_Flag = 0;

extern int IsDebug;

uint16_t time_10min = 0 ,time_5min = 0 ,time_1min = 1 ,time_1s = 0;

void HydrologyTimeBase()
{
  time_1min++;
  time_5min++;
  time_10min++;

}

void convertSampleTimetoHydrology(char* src,char* dst)
{
  dst[0] = _DECtoBCD(src[0]);
  dst[1] = _DECtoBCD(src[1]);
  dst[2] = _DECtoBCD(src[2]);
  dst[3] = _DECtoBCD(src[3]);
  dst[4] = _DECtoBCD(src[4]);
}

void convertSendTimetoHydrology(char* src,char* dst)
{
  dst[0] = _DECtoBCD(src[0]);
  dst[1] = _DECtoBCD(src[1]);
  dst[2] = _DECtoBCD(src[2]);
  dst[3] = _DECtoBCD(src[3]);
  dst[4] = _DECtoBCD(src[4]);
  dst[5] = _DECtoBCD(src[5]);
}

float ConvertAnalog(int v,int range)
{
  float tmp;
  
  tmp = (v -4096.0) / (4096.0) * range;
  
  
  return tmp;
}

void ADC_Element(char *value,int index)
{
//int range[5] = {1,20,100,5000,4000};     //ģ������Χ
  int range[16] = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
  float floatvalue = 0;
  
  floatvalue = ConvertAnalog(A[index+1],range[index]);
  memcpy(value,(char*)(&floatvalue),4);
}

char value[4] = {0,0,0,0};

void HydrologyOneMinute()
{
  char _temp_sampertime[6] = {0,0,0,0,0,0};
  int i = 0;
  int adc_i = 0,isr_i = 0,io_i = 0,rs485_i = 0;
  int a = 0,b = 0,c = 0;
  long isr_count = 0;
  char isr_count_temp[5] = {0,0,0,0,0};
  
  ADC_Sample();
  System_Delayms(20);
  UART1_Open_9600(UART3_U485_TYPE);
  while(Element_table[i].ID != 0)
  {
    memset(value,0,sizeof(value));
    switch(Element_table[i].Mode)
    {
      case ADC:
      {
        ADC_Element(value,adc_i);
        adc_i++;
        break;
      }
      case ISR_COUNT:
      {
        Hydrology_ReadStoreInfo(HYDROLOGY_ISR_COUNT1 + isr_i*HYDROLOGY_ISR_COUNT_LEN,isr_count_temp,HYDROLOGY_ISR_COUNT_LEN);
        isr_count = (isr_count_temp[4] * 0x100000000) + (isr_count_temp[3] * 0x1000000) + (isr_count_temp[2] * 0x10000) + (isr_count_temp[1] * 0x100) + (isr_count_temp[0]);
        memcpy(value,(char*)(&isr_count),4);
        isr_i++;
        break;
      }
      case IO_STATUS:
      {
        //Hydrology_ReadIO_STATUS(value,io_i);
        io_i++;
        break;
      }
      case RS485:
      {
        Hydrology_ReadRS485(value,rs485_i);
        rs485_i++;
        break;
      }
    }
    
    switch(Element_table[i].type)
    {
      case ANALOG:
      {
        convertSampleTimetoHydrology(g_rtc_nowTime,_temp_sampertime);
        Hydrology_SetObservationTime(Element_table[i].ID,_temp_sampertime,i);
        Hydrology_WriteStoreInfo(HYDROLOGY_ANALOG1 + a*HYDROLOGY_ANALOG_LEN,value,HYDROLOGY_ANALOG_LEN);
        a++;
        break;
      }
      case PULSE:
      {
        convertSampleTimetoHydrology(g_rtc_nowTime,_temp_sampertime);
        Hydrology_SetObservationTime(Element_table[i].ID,_temp_sampertime,i);
        Hydrology_WriteStoreInfo(HYDROLOGY_PULSE1 + b*HYDROLOGY_PULSE_LEN,value,HYDROLOGY_PULSE_LEN);
        b++;
        break;
      }
      case SWITCH:
      {
        convertSampleTimetoHydrology(g_rtc_nowTime,_temp_sampertime);
        Hydrology_SetObservationTime(Element_table[i].ID,_temp_sampertime,i);
        Hydrology_WriteStoreInfo(HYDROLOGY_SWITCH1 + c*HYDROLOGY_SWITCH_LEN,value,HYDROLOGY_SWITCH_LEN);
        c++;
        break;
      }
    }
    i++;
  }
  UART3_Open(UART3_CONSOLE_TYPE);
}

int HydrologyOnline()
{
  if(time_10min >= 1)
    hydrologyProcessSend(LinkMaintenance);
        
    return 0;
}

int HydrologyOffline()
{
    GPRS_Close_TCP_Link();
    GPRS_Close_GSM();
        
    return 0;
}


int HydrologyInstantWaterLevel(char* _saveTime)
{
//    char _temp_instantwaterlevel[4];
      char waterlevelstoreinterval;
      static char endtime[6] = {0,0,0,0,0,0};
    
//    if(endtime[0] == 0 && endtime[1] == 0 && endtime[2] == 0 && endtime[3] == 0 && endtime[4] == 0 && endtime[5] == 0)
//    {
      Hydrology_ReadStoreInfo(HYDROLOGY_WATERLEVEL_STORE_INTERVAL,&waterlevelstoreinterval,HYDROLOGY_WATERLEVEL_STORE_INTERVAL_LEN);
      waterlevelstoreinterval = _BCDtoDEC(waterlevelstoreinterval);
      Utility_Strncpy(endtime,_saveTime,6);
//      Utility_Time_AddMinute(endtime,waterlevelstoreinterval);
//      endtime[5] = 0;
//    }RTC_IsPassed(endtime) < 0 || 
      
//ly Ϊ�˷�����Թرռ�飬��ʽ����ʱ��ָ��ж�
   if(endtime[4]%waterlevelstoreinterval != 0 && !IsDebug)
        return -1;
    
   hydrologyProcessSend(TimerReport);
    
    if(!IsDebug)
    {
      System_Delayms(5000);
      JudgeServerDataArrived();
      Hydrology_ProcessGPRSReceieve();
      JudgeServerDataArrived();
      Hydrology_ProcessGPRSReceieve();
      JudgeServerDataArrived();
      Hydrology_ProcessGPRSReceieve();
      if(GPRS_Close_TCP_Link() != 0)
            GPRS_Close_GSM();
    }
    
    time_10min = 0;
    
    endtime[0] = 0;
    endtime[1] = 0;
    endtime[2] = 0;
    endtime[3] = 0;
    endtime[4] = 0;
    endtime[5] = 0;

    return 0;
}

int HydrologyVoltage()
{
//    char _temp_voltage[4];
//
//    _temp_voltage[0] = A[0] >> 8;
//    _temp_voltage[1] = A[0] & 0x00FF;
//
//    Store_SetHydrologyVoltage(_temp_voltage);
//
    return 0;
}


int HydrologyTask()
{
  
    TimerB_Clear();
    WatchDog_Clear();
//    JudgeServerDataArrived();
//    Hydrology_ProcessGPRSReceieve();
    Hydrology_ProcessUARTReceieve();
    
    if(!IsDebug)
    {
      if(time_1min)
          time_1min = 0;
      else
          return -1;
    }
    
    HydrologyOneMinute();  //ȡҪ��
    
//    if(time_10min == 10)
//    {
//        HydrologyOffline();
//    }
    
    RTC_ReadTimeBytes5(g_rtc_nowTime);
    
    char rtc_nowTime[6];
    RTC_ReadTimeBytes6(rtc_nowTime);
    
    HydrologyInstantWaterLevel(rtc_nowTime);
    
//    HydrologyOnline();
      
    return 0;
}


















































