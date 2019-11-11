
#include "message.h"
#include "hydrologycommand.h"
#include "string.h"
#include "stdlib.h"
#include "stdio.h" 

/* USER CODE BEGIN Includes */
#include "rtc.h"
//#include "usart.h"
#include "GTM900C.h"
#include "common.h"
#include "rom.h"
#include "hydrologytask.h"
//#define DEBUG
//#include "ldebug.h"

extern int IsDebug;
extern char IsQuery;
extern char isUARTConfig;


/* USER CODE END Includes */

uint8_t ADCElementCount = 0;
uint8_t ISR_COUNTElementCount = 0;
uint8_t IO_STATUSElementCount = 0;
uint8_t RS485ElementCount = 0;

hydrologyHeader g_HydrologyUpHeader;
hydrologyHeader g_HydrologyDownHeader;
packet g_HydrologyMsg;

hydrologyElement inputPara[MAX_ELEMENT];

/*以下函数为外部接口函数，需由用户根据硬件设备实现，否则水文协议无法运行*/

const hydrologyElementInfo Element_table[UserElementCount+1] = {
//	ELEMENT_REGISTER(0x03,ANALOG,3,1),
//	ELEMENT_REGISTER(0x46,ANALOG,4,2),
//	ELEMENT_REGISTER(0x47,ANALOG,4,1),
//        ELEMENT_REGISTER(0x58,ANALOG,5,2,ADC),          //TEST
      
        
//        ELEMENT_REGISTER(0x58,ANALOG,5,2,ADC),
//        ELEMENT_REGISTER(0x49,ANALOG,5,2,ADC),
//        ELEMENT_REGISTER(0x59,ANALOG,5,2,ADC),
        ELEMENT_REGISTER(0x60,PULSE,11,3,ISR_COUNT),
        ELEMENT_REGISTER(0x61,PULSE,11,3,ISR_COUNT),
	ELEMENT_REGISTER(0x62,PULSE,11,3,ISR_COUNT),
        ELEMENT_REGISTER(0x27,ANALOG,9,3,RS485),
	ELEMENT_REGISTER(0x60,PULSE,11,3,RS485),

   	ELEMENT_REGISTER(NULL,NULL,NULL,NULL,NULL)
	
};

int SinglePacketSize = 0;
int RomElementBeginAddr = 0;
int SinglePacketSendCount = 0;
int PacketSendTimes = 0;

char HYDROLOGY_MODE = M1;

int Hydrology_ReadStoreInfo(long addr,char *data,int len)
{
  int _repeats=0;

  while (len > 16)
  {
    while(ROM_ReadBytes(addr , data , 16)!=0)
    {
      if(_repeats>3)
      { 
        return -1;
      }
      ++_repeats;
    } 

    addr += 16;
    len -= 16;
    data += 16;
  }

  while(ROM_ReadBytes(addr , data , len)!=0)
  {
    if(_repeats>3)
    { 
      return -1;
    }
    ++_repeats;
  }
  return 0;
}

int Hydrology_WriteStoreInfo(long addr,char *data,int len)
{
  int _repeats=0;

  while (len > 16)
  {
    while(ROM_WriteBytes(addr , data , 16)!=0)
    {
      if(_repeats>3)
      { 
          return -1;
      }
      ++_repeats;
    } 

    addr += 16;
    len -= 16;
    data += 16;
    
  }

  while(ROM_WriteBytes(addr , data , len)!=0)
  {
    if(_repeats>3)
    { 
      return -1;
    }
    ++_repeats;
  } 
  return 0;
}

void Hydrology_ReadTime(char* time)
{
  char rtc_nowTime[6];
  
  RTC_ReadTimeBytes6(rtc_nowTime);
  convertSendTimetoHydrology(rtc_nowTime,time);
}

void Hydrology_SetTime(char* time)
{
  char _temp_time[12 + 1];

  hex_2_ascii(time,_temp_time,6);
  
  RTC_SetTimeStr6_B(_temp_time);
}

//int Hydrology_SendData(char* data,int len)
//{
//  int _repeats=0;
//  
//  TraceHexMsg(data,len);
//  if(isUARTConfig)
//  {
//    isUARTConfig = 0;
//    return 0;
//  }
//  
//  if(!IsDebug)
//  {
//    if(!IsQuery)
//    {
//      while(GPRS_Send(data,len,0,HYDROLOGY_CENTER1_IP) == FALSE)
//      {
//        if(GPRS_Close_TCP_Link() != 0)
//            GPRS_Close_GSM();
//        if(_repeats>3)
//        { 
//          break;
//        }
//        ++_repeats;
//      }
//      if(GPRS_Close_TCP_Link() != 0)
//        GPRS_Close_GSM();
//    }
//    
//    _repeats=0;
//    while(GPRS_Send(data, len,0,HYDROLOGY_BACKUP1_IP) == FALSE)
//    {
//      if(GPRS_Close_TCP_Link() != 0)
//        GPRS_Close_GSM();
//      if(_repeats>3)
//      { 
//        break;
//      }
//      ++_repeats;
//    }
//    IsQuery = 0;
//    HydrologyRecord(ERC16);
//  }
//  return 0;
//}
int Hydrology_SendData(char* data,int len)
{
  int _repeats=0;
  
  TraceHexMsg(data,len);
  if(isUARTConfig)
  {
    isUARTConfig = 0;
    return 0;
  }
  
  if(!IsDebug)
  {
    if(!IsQuery)
    {
      while(GPRS_Send(data,len,0,HYDROLOGY_CENTER1_IP) == FALSE)
      {
 //       if(GPRS_Close_TCP_Link() != 0)
//            GPRS_Close_GSM();
        if(_repeats>=0)
        { 
          break;
        }
        ++_repeats;
      }
      if(GPRS_Close_TCP_Link() != 0)
        GPRS_Close_GSM();
       IsQuery = 0;
    }
    
    _repeats=0;
    while(GPRS_Send(data, len,0,HYDROLOGY_BACKUP1_IP) == FALSE)
    {
      if(GPRS_Close_TCP_Link() != 0)
        GPRS_Close_GSM();
      if(_repeats>0)
      { 
        break;
      }
      ++_repeats;
    }
   
    HydrologyRecord(ERC16);
  }
  return 0;
}







void Hydrology_Printf(char *buff)
{
  TraceMsg(buff,1);
}

/*以上函数为外部接口函数，需由用户根据硬件设备实现，否则水文协议无法运行*/

void Hydrology_SetRTUType(char* type)
{
  Hydrology_WriteStoreInfo(HYDROLOGY_RTUTYPE,type,HYDROLOGY_RTUTYPE_LEN);
}

void Hydrology_ReadObservationTime(char id,char* observationtime,int index)
{
  long addr = HYDROLOGY_ANALOG1_OBSERVATION_TIME + index * HYDROLOGY_OBSERVATION_TIME_LEN;
  int i = 0;
  
  while(Element_table[i].ID != 0)
  {
    if(Element_table[i].ID == id)
    {
      switch(Element_table[i].type)
      {
        case ANALOG:
        {
          addr = HYDROLOGY_ANALOG1_OBSERVATION_TIME + index * HYDROLOGY_OBSERVATION_TIME_LEN;
          break;
        }
        case PULSE:
        {
          addr = HYDROLOGY_PULSE1_OBSERVATION_TIME + index * HYDROLOGY_OBSERVATION_TIME_LEN;
          break;
        }
        case SWITCH:
        {
          addr = HYDROLOGY_SWITCH1_OBSERVATION_TIME + index * HYDROLOGY_OBSERVATION_TIME_LEN;
          break;
        }
        default:
          break;
      }
    }
    i++;
  }
  Hydrology_ReadStoreInfo(addr,observationtime,HYDROLOGY_OBSERVATION_TIME_LEN);
}

void Hydrology_SetObservationTime(char id,char* observationtime,int index)
{
  long addr = HYDROLOGY_ANALOG1_OBSERVATION_TIME + index * HYDROLOGY_OBSERVATION_TIME_LEN;
  int i = 0;
  
  while(Element_table[i].ID != 0)
  {
    if(Element_table[i].ID == id)
    {
      switch(Element_table[i].type)
      {
        case ANALOG:
        {
          addr = HYDROLOGY_ANALOG1_OBSERVATION_TIME + index * HYDROLOGY_OBSERVATION_TIME_LEN;
          break;
        }
        case PULSE:
        {
          addr = HYDROLOGY_PULSE1_OBSERVATION_TIME + index * HYDROLOGY_OBSERVATION_TIME_LEN;
          break;
        }
        case SWITCH:
        {
          addr = HYDROLOGY_SWITCH1_OBSERVATION_TIME + index * HYDROLOGY_OBSERVATION_TIME_LEN;
          break;
        }
        default:
          break;
      }
    }
    i++;
  }
  Hydrology_WriteStoreInfo(addr,observationtime,HYDROLOGY_OBSERVATION_TIME_LEN);
}

void getBCDnums(double num, int * intergerpart, int* decimerpart, int d, char* pout_intergerValue, char * pout_decimerValue)
{
  char strfloat[20];
  int i = 0;
  int len = 0;
  int j = 0;
  int k = 0;

  for ( i = 0; i < 20 ; i++)
  {
    strfloat[i] = 'X';
  }
  sprintf(strfloat,"%f",num);
  for ( i = 0; i < 20; i++)
  {
    if ( 'X' != strfloat[i] )
                    len++;
  }
  for ( i = 0 ; i < len ; i++)
  {
    if ( '0' != strfloat[i])
    {
      break;
    }
  }
  len = len - i - 1;

  for ( i = 0; i < len ; i++)
  {
    if ('.' == strfloat[i])
    {
      j = i;
      break;
    }
  }
  if ( i < len ) //
  {
    *decimerpart = len - j - 1;
    if(*decimerpart > d)
    {
      *decimerpart = d;
    }

    *intergerpart = j;

    for ( i = 0; i < j ; i++)
    {
      pout_intergerValue[i] = strfloat[i];
    }
    pout_intergerValue[j] = 0;

    for ( i = j+1; (i < len) && (k < (*decimerpart)); i++)
    {
      pout_decimerValue[k++] = strfloat[i];
    }
    pout_decimerValue[k] = 0;
  }
  else 
  {
    *decimerpart = 0;
    *intergerpart = len;
    sprintf(pout_intergerValue,"%d",(int)num);
  }
}

int getEvenNum(int num)
{
  if(num%2 == 0)
    return num;
  else 
    return num+1;
}

/*字节高5位，D表示数据字节数，字节低3位，d表示小数点后位数*/
void getguideid(char* value,char D, char d)
{
  char high5 = 0;
  char low3 = 0;
  char evenD = 0;

  evenD = getEvenNum(D);

  high5 = evenD/2;
  high5 = high5 << 3;
  low3 = d; //d的范围是0-7之间，才能用3位表示
  *value = high5 | low3;
}


int converToHexElement(double input, int D,int d,char* out)
{
  char strInterValue[20] = {0}; //strInterValue表示整数值
  char strDeciValue[20] = {0}; 	//strDeciValue表示小数值
  int integer =0; 							//interger表示整数位数
  int decimer =0; 							//decimer表示小数位数
  //int intergerValue = 0 ;			// 表示整数值
  //int decimerValue = 0 ; 			//表示小数值
  int total = 0; 								//total 表示input总的位数（除去小数点）
  int evenD = 0; 								//偶数D
  int difftotal = 0; 						//表示evenD与total差值
  int diffInterger = 0;					//表示整数位需要补齐
  int diffDecimer = 0;					//表示小数位需要补齐
  //int delDecimer = 0 ;				//表示小数位需要删除的位数
  int i = 0;
  int j = 0;
  int m = 0;

  char tmp[30];
  for(m = 0 ; m < 30 ; m++)
  {
    tmp[m] = '0';
  }
  getBCDnums(input,&integer,&decimer, d,strInterValue,strDeciValue);
  evenD = getEvenNum(D);
  total = integer + decimer;

  if ( evenD >= total ) 				//有输入配置参数保证
  {
    difftotal = evenD - total;
    if( d >= decimer ) 					//这个是肯定发生的，getBCDnums保证
    {
      diffDecimer = d - decimer; //小数位需要补0的位数，，往后补0
      diffInterger = difftotal - diffDecimer;//整数位需要补0 的位数,假设difftotal总是大于diffDecimer，往前补0
    }
    /*将当前小数位和整数位都整合到tmp数组中，分为下面几个部分 0--->diffInterger-1 整数补0的个数，diffInterger---> diffInterger+ interger是整数位个数，
    diffInter+ interge ---> evenD是小数位个数*/
    memcpy(&tmp[diffInterger],strInterValue, integer);
    memcpy(&tmp[diffInterger+integer],strDeciValue, decimer);

    tmp[evenD] = 0 ;

    for ( i = 0; i < evenD ; i=i+2)
    {
      out[j++] = (tmp[i] - '0') * 16 + (tmp[i+1]-'0');
    }
  }
  else //当前这种情况不会发生
  {
    return -1;
  }
  return 0;
}
int mallocElement(char element,char D,char d,hydrologyElement* ele)
{
  ele->guide[0] = element;
  getguideid(&(ele->guide[1]),D,d);

  if ( D%2 == 0 )
  {
    ele->value = (char*)malloc(D/2);
    
    if (ele->value == 0)
    {
      return -1;
    }

    memset(ele->value,0,D/2);
    
    ele->num = D/2;
  }
  else
  {
    ele->value = (char*)malloc((D+1)/2);
    
    if (ele->value == 0)
    {
      return -1;
    }

    memset(ele->value,0,(D+1)/2);
    ele->num = (D+1)/2;
  }

  return 0;
}

int hydrologyJudgeType(char funcode)
{
  int type;
  switch(funcode)
  {
    case LinkMaintenance:
    {
      type = 2;
      break;
    }
    case Test:
    {
      type = 1;
      break;
    }
    case EvenPeriodInformation:
    {
      type = 1;
      break;
    }
    case TimerReport:
    {
      type = 1;
      break;
    }
    case AddReport:
    {
      type = 1;
      break;
    }
    case Hour:
    {
      type = 1;
      break;
    }
    case ArtificialNumber:
    {
      type = 1;
      break;
    }
    case Picture:
    {
      type = 1;
      break;
    }
    case Realtime:
    {
      type = 1;
      break;
    }
    case Period:
    {
      type = 1;
      break;
    }
    case InquireArtificialNumber:
    {
      type = 1;
      break;
    }
    case SpecifiedElement:
    {
      type = 1;
      break;
    }
    case ConfigurationModification:
    {
      type = 3;
      break;
    }
    case ConfigurationRead:
    {
      type = 3;
      break;
    }
    case ParameterModification:
    {
      type = 3;
      break;
    }
    case ParameterRead:
    {
      type = 3;
      break;
    }
    case WaterPumpMotor:
    {
      type = 1;
      break;
    }
    case SoftwareVersion:
    {
      type = 1;
      break;
    }
    case Status:
    {
      type = 1;
      break;
    }
    case InitializeSolidStorage:
    {
      type = 1;
      break;
    }
    case Reset:
    {
      type = 3;
      break;
    }        
    case ChangePassword:
    {
      type = 1;
      break;
    }        
    case SetClock:
    {
      type = 1;
      break;
    }
    case SetICCard:
    {
      type = 1;
      break;
    }
    case Pump:
    {
      type = 1;
      break;
    }
    case Valve:
    {
      type = 1;
      break;
    }        
    case Gate:
    {
      type = 1;
      break;
    }
    case WaterSetting:
    {
      type = 1;
      break;
    }
    case Record:
    {
      type = 1;
      break;
    }       
    case Time:
    {
      type = 1;
      break;
    }
    default:
      break;
  }    
  return type;
	
}

void Hydrology_ReadAnalog(float *value,int index)
{
  long addr = HYDROLOGY_ANALOG1 + index * 4;
  char temp_value[4];
  
  Hydrology_ReadStoreInfo(addr,temp_value,HYDROLOGY_ANALOG_LEN);
  *value = *((float*)temp_value);
}

void Hydrology_ReadPulse(long *value,int index)
{
  long addr = HYDROLOGY_PULSE1 + index * 4;
  char temp_value[4];
  
  Hydrology_ReadStoreInfo(addr,temp_value,HYDROLOGY_PULSE_LEN);
  *value = *((long*)temp_value);
}

void Hydrology_ReadSwitch(int *value,int index)
{
  long addr = HYDROLOGY_SWITCH1 + index * 4;
  char temp_value[4]; 
  Hydrology_ReadStoreInfo(addr,temp_value,HYDROLOGY_SWITCH_LEN);
  *value = *((int*)temp_value);
}

void Hydrology_ReadRom(long beginaddr,char *value,int index)
{
  long addr = beginaddr + index * SinglePacketSize;
  
  Hydrology_ReadStoreInfo(addr,value,SinglePacketSize);
}

void Hydrology_CalElementInfo(int *count,char funcode)
{
  int i = 0,acount = 0,pocunt = 0,scount = 0;
  float floatvalue = 0;
  long intvalue1 = 0;
  int intvalue2 = 0;
  int type = 0;
  hydrologyDownBody* downpbody = (hydrologyDownBody*) (g_HydrologyMsg.downbody);

  type = hydrologyJudgeType(funcode);
  
  if(type == 1)
  {
    while(Element_table[i].ID != 0)
    {
      switch(Element_table[i].type)
      {
        case ANALOG:
        {
          Hydrology_ReadAnalog(&floatvalue,acount++);
          mallocElement(Element_table[i].ID,Element_table[i].D,Element_table[i].d,&inputPara[i]);
          converToHexElement((double)floatvalue,Element_table[i].D,Element_table[i].d,inputPara[i].value);
          break;
        }
        case PULSE:
        {
          Hydrology_ReadPulse(&intvalue1,pocunt++);
          mallocElement(Element_table[i].ID,Element_table[i].D,Element_table[i].d,&inputPara[i]);
          converToHexElement((double)intvalue1,Element_table[i].D,Element_table[i].d,inputPara[i].value);
          break;
        }
        case SWITCH:
        {
          Hydrology_ReadSwitch(&intvalue2,scount++);
          mallocElement(Element_table[i].ID,Element_table[i].D,Element_table[i].d,&inputPara[i]);
          converToHexElement((double)intvalue2,Element_table[i].D,Element_table[i].d,inputPara[i].value);
          break;
        }
        case STORE:
        {
          inputPara[i].guide[0] = Element_table[i].ID;
          inputPara[i].guide[1] = Element_table[i].ID;
          inputPara[i].value = (char*)malloc(SinglePacketSize);
          if (NULL != inputPara[i].value)
          {
            inputPara[i].num = SinglePacketSize;
            Hydrology_ReadRom(RomElementBeginAddr,inputPara[i].value,SinglePacketSendCount++);
          }
          break;
        }
        default:
          break;
      }
      i++;
      (*count)++;
    }
  }
  else if(type == 2)
  {

  }
  else if(type == 3)
  {
    for(i = 0;i < downpbody->count;i++)
    {
      memcpy(inputPara[i].guide,(downpbody->element)[i].guide,2);
      
      inputPara[i].num = ((downpbody->element)[i].guide[1] >> 3);
      inputPara[i].value = (char*) malloc((downpbody->element)[i].num);
      if (NULL == inputPara[i].value)
        continue;
      HydrologyReadSuiteElement(funcode,inputPara[i].guide,inputPara[i].value);
      
      (*count)++;
    }    
  }
}

void hydrologyMakeUpHeader(char funcode)
{
//    char _temp_picturesize[2] = {0x01,0x5E};
//    int picturesize;
    hydrologyHeader* pheader = (hydrologyHeader*) (g_HydrologyMsg.header);

    Hydrology_ReadStoreInfo(HYDROLOGY_PASSWORD_ADDR,pheader->password,HYDROLOGY_PASSWORD_LEN);
    
    pheader->framestart[0] = SOH1;
    pheader->framestart[1] = SOH2;

    Hydrology_ReadStoreInfo(HYDROLOGY_CENTER_ADDR,&(pheader->centeraddr),HYDROLOGY_CENTER_LEN-3);
    
    Hydrology_ReadStoreInfo(HYDROLOGY_REMOTE_ADDR,pheader->remoteaddr,HYDROLOGY_REMOTE_LEN);
    
    pheader->funcode = funcode;
    pheader->dir_len[0] = 0 << 4;
    
    if(HYDROLOGY_MODE == M3)
    {
      SinglePacketSendCount++;
      pheader->paketstart = SYN;
      
      pheader->count_seq[0] = PacketSendTimes >> 4;
      pheader->count_seq[1] = ((PacketSendTimes & 0x000F) << 4) + (SinglePacketSendCount >> 8);
      pheader->count_seq[2] = SinglePacketSendCount & 0x00FF;
    }
    else
      pheader->paketstart = STX;
    
}

void getstreamid(char streamid[2])
{
    static unsigned short id = 0 ;
    id++;
    id = id % 65536;
    if(id==0)
      id=1;
    streamid[0]=(id>>8)&0xff;
    streamid[1]=id&0xff;
}

void hydrologyMakeUpBodyBasicInfo(char funcode)
{
  hydrologyUpBody* pbody = (hydrologyUpBody*) (g_HydrologyMsg.upbody);
  int type = 0;

  type = hydrologyJudgeType(funcode);
  pbody->len = 0;
  if(type == 1)
  {
    getstreamid(pbody->streamid);
    pbody->len += 2;
    Hydrology_ReadTime(pbody->sendtime);
    pbody->len += 6;
    pbody->rtuaddrid[0] = 0xF1;
    pbody->rtuaddrid[1] = 0xF1;
    pbody->len += 2;    
    Hydrology_ReadStoreInfo(HYDROLOGY_REMOTE_ADDR,pbody->rtuaddr,HYDROLOGY_REMOTE_LEN);
    pbody->len += 5;
    Hydrology_ReadStoreInfo(HYDROLOGY_RTUTYPE,&(pbody->rtutype),HYDROLOGY_RTUTYPE_LEN);
    pbody->len += 1;
    pbody->observationtimeid[0]=0xF0;
    pbody->observationtimeid[1]=0xF0;
    pbody->len += 2;
    Hydrology_ReadObservationTime(Element_table[0].ID,pbody->observationtime,0);
    pbody->len += 5;
    if(SinglePacketSendCount > 1)
      pbody->len = 0;
  }
  else if(type == 2)
  {
    getstreamid(pbody->streamid);
    pbody->len += 2;
    Hydrology_ReadTime(pbody->sendtime);
    pbody->len += 6;
  }
  else if(type == 3)
  {
    getstreamid(pbody->streamid);
    pbody->len += 2;
    Hydrology_ReadTime(pbody->sendtime);
    pbody->len += 6;
    pbody->rtuaddrid[0] = 0xF1;
    pbody->rtuaddrid[1] = 0xF1;
    pbody->len += 2;    
    Hydrology_ReadStoreInfo(HYDROLOGY_REMOTE_ADDR,pbody->rtuaddr,HYDROLOGY_REMOTE_LEN);
    pbody->len += 5;
  }
}

int hydrologyMakeUpBody(int count)
{
  hydrologyUpBody* pbody = (hydrologyUpBody*) (g_HydrologyMsg.upbody);
  int i;

  pbody->count = count;
  for(i = 0;i < pbody->count;i++)
  {
    memcpy((pbody->element)[i].guide, inputPara[i].guide,2);
    (pbody->element)[i].value = (char*) malloc(inputPara[i].num);
    if(NULL == (pbody->element)[i].value)
       return -1;
    memcpy((pbody->element)[i].value,inputPara[i].value, inputPara[i].num);
    (pbody->element)[i].num = inputPara[i].num;     
    pbody->len += 2 + inputPara[i].num;
  }
  return 0;
}

// ---------------- POPULAR POLYNOMIALS ----------------
// CCITT:      x^16 + x^12 + x^5 + x^0                 (0x1021)
// CRC-16:     x^16 + x^15 + x^2 + x^0                 (0x8005)
#define         CRC_16_POLYNOMIALS      0x8005

const char chCRCHTalbe[] =
{
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40};
const char chCRCLTalbe[] =
{
0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06, 0x07, 0xC7,
0x05, 0xC5, 0xC4, 0x04, 0xCC, 0x0C, 0x0D, 0xCD, 0x0F, 0xCF, 0xCE, 0x0E,
0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09, 0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9,
0x1B, 0xDB, 0xDA, 0x1A, 0x1E, 0xDE, 0xDF, 0x1F, 0xDD, 0x1D, 0x1C, 0xDC,
0x14, 0xD4, 0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3,
0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3, 0xF2, 0x32,
0x36, 0xF6, 0xF7, 0x37, 0xF5, 0x35, 0x34, 0xF4, 0x3C, 0xFC, 0xFD, 0x3D,
0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A, 0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38,
0x28, 0xE8, 0xE9, 0x29, 0xEB, 0x2B, 0x2A, 0xEA, 0xEE, 0x2E, 0x2F, 0xEF,
0x2D, 0xED, 0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,
0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60, 0x61, 0xA1,
0x63, 0xA3, 0xA2, 0x62, 0x66, 0xA6, 0xA7, 0x67, 0xA5, 0x65, 0x64, 0xA4,
0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F, 0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB,
0x69, 0xA9, 0xA8, 0x68, 0x78, 0xB8, 0xB9, 0x79, 0xBB, 0x7B, 0x7A, 0xBA,
0xBE, 0x7E, 0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5,
0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71, 0x70, 0xB0,
0x50, 0x90, 0x91, 0x51, 0x93, 0x53, 0x52, 0x92, 0x96, 0x56, 0x57, 0x97,
0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C, 0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E,
0x5A, 0x9A, 0x9B, 0x5B, 0x99, 0x59, 0x58, 0x98, 0x88, 0x48, 0x49, 0x89,
0x4B, 0x8B, 0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42, 0x43, 0x83,
0x41, 0x81, 0x80, 0x40};

short hydrologyCRC16(char* pchMsg, int wDataLen)
{
  char chCRCHi = 0xFF;
  char chCRCLo = 0xFF;
  short wIndex;
  while (wDataLen--)
  {
    wIndex = chCRCLo ^ *pchMsg++ ;
    chCRCLo = chCRCHi ^ chCRCHTalbe[wIndex];
    chCRCHi = chCRCLTalbe[wIndex] ;
  }
  return ((chCRCHi << 8) | chCRCLo) ;
}

int hydrologyCheck(char* input,int inputlen)
{
  short crcRet = 0;
  short inputCrc = 0;
  int bodylen = 0;

  crcRet = hydrologyCRC16(input,inputlen - 2);

  inputCrc = (input[inputlen - 2] << 8) | input[inputlen - 1];

  if(crcRet == inputCrc)
  {
    TraceMsg("CRC check success !",1);
  }
  else
  {
    Hydrology_Printf("CRC check fail !");
    return -1;
  }

  if((input[0] == SOH1) && (input[1] == SOH2))
  {
    ;//TraceMsg("Frame head check success !",1);
  }
  else
  {
    Hydrology_Printf("Frame head check fail !");
    return -2;
  }

  bodylen = (input[11] & 0x0F) * 256 + input[12];

  if(bodylen == (inputlen - 17))
  {
    ;//TraceMsg("Hydrolog length check success !",1);
  }
  else
  {
    Hydrology_Printf("Hydrolog length check fail !");
    return -3;
  }
  return 0;
}

int hydrologyMakeDownHeader(char* input,int inputlen,int *position,int *bodylen)
{
  hydrologyHeader* pheader = &g_HydrologyDownHeader;

  if(hydrologyCheck(input, inputlen) == 0)
  {
    ;//TraceMsg("Hydrolog check success !",1);
  }
  else
  {
    Hydrology_Printf("Hydrology check fail !");
    return -1;
  }
    
  memcpy(pheader->framestart,&input[*position],2);
  *position += 2;

  memcpy(pheader->remoteaddr,&input[*position],5);
  *position += 5;    

  memcpy(&(pheader->centeraddr),&input[*position],1);
  *position += 1;

  memcpy(pheader->password,&input[*position],2);
  *position += 2;

  memcpy(&(pheader->funcode),&input[*position],1);
  *position += 1;

  memcpy(pheader->dir_len,&input[*position],1);
  pheader->dir_len[0] >>= 4;

  *bodylen = (input[*position] & 0x0F) * 256 + input[*position+1];
  *position += 2;

  memcpy(&(pheader->paketstart),&input[*position],1);
  *position += 1;

  if(pheader->paketstart == SYN)
  {
    memcpy(pheader->count_seq,&input[*position],3);
    *position += 3;
  }
  return 0;
}

int hydrologyNeedEditElement(char funcode)
{
  int trueorfalse = 0;
  switch(funcode)
  {
    case LinkMaintenance:
    {
      trueorfalse = 1;
      break;
    }
    case Test:
    {
      trueorfalse = 1;
      break;
    }
    case EvenPeriodInformation:
    {
      trueorfalse = 1;
      break;
    }
    case TimerReport:
    {
      trueorfalse = 1;
      break;
    }
    case AddReport:
    {
      trueorfalse = 1;
      break;
    }
    case Hour:
    {
      trueorfalse = 1;
      break;
    }
    case ArtificialNumber:
    {
      trueorfalse = 1;
      break;
    }
    case Picture:
    {
      trueorfalse = 1;
      break;
    }
    case Realtime:
    {
      trueorfalse = 1;
      break;
    }
    case Period:
    {
      trueorfalse = 1;
      break;
    }
    case InquireArtificialNumber:
    {
      trueorfalse = 1;
      break;
    }
    case SpecifiedElement:
    {
      trueorfalse = 1;
      break;
    }
    case ConfigurationModification:
    {
      trueorfalse = 1;
      break;
    }
    case ConfigurationRead:
    {
      trueorfalse = 1;
      break;
    }
    case ParameterModification:
    {
      trueorfalse = 1;
      break;
    }
    case ParameterRead:
    {
      trueorfalse = 1;
      break;
    }
    case WaterPumpMotor:
    {
      trueorfalse = 1;
      break;
    }
    case SoftwareVersion:
    {
      trueorfalse = 1;
      break;
    }
    case Status:
    {
      trueorfalse = 1;
      break;
    }
    case InitializeSolidStorage:
    {
      trueorfalse = 1;
      break;
    }
    case Reset:
    {
      trueorfalse = 0;
      break;
    }        
    case ChangePassword:
    {
      trueorfalse = 1;
      break;
    }        
    case SetClock:
    {
      trueorfalse = 1;
      break;
    }
    case SetICCard:
    {
      trueorfalse = 1;
      break;
    }
    case Pump:
    {
      trueorfalse = 1;
      break;
    }
    case Valve:
    {
      trueorfalse = 1;
      break;
    }        
    case Gate:
    {
      trueorfalse = 1;
      break;
    }
    case WaterSetting:
    {
      trueorfalse = 1;
      break;
    }
    case Record:
    {
      trueorfalse = 1;
      break;
    }       
    case Time:
    {
      trueorfalse = 1;
      break;
    }
    default:
      break;
  }    
  return trueorfalse;
}

int hydrologyMakeDownBody(char* input,int len,int position)
{
  hydrologyDownBody* pbody = (hydrologyDownBody*) (g_HydrologyMsg.downbody);
  int trueorfalse = 0;

  trueorfalse = hydrologyNeedEditElement(input[10]);

  memcpy(pbody->streamid,&input[position],2);
  position += 2;
  len -= 2;

  memcpy(pbody->sendtime,&input[position],6);
  position += 6;
  len -= 6;

  pbody->count = 0;
  if(!trueorfalse)
    return 0;
  while(len != 0)
  {
    memcpy((pbody->element)[pbody->count].guide, &input[position],2);
    position += 2;
    len -= 2;
    
    (pbody->element)[pbody->count].num = ((pbody->element)[pbody->count].guide[1] >> 3);
    (pbody->element)[pbody->count].value = (char*) malloc((pbody->element)[pbody->count].num);
    if (NULL == (pbody->element)[pbody->count].value)
      return -1;
    memcpy((pbody->element)[pbody->count].value,&input[position],(pbody->element)[pbody->count].num);
    position += (pbody->element)[pbody->count].num;
    len -= (pbody->element)[pbody->count].num;
    
    pbody->count ++;
    if(len < 0)
      return -2;
  }
  return 0;
}

void hydrologyMakeUpTail(char* buffer,int *pbuffer,char funcode)
{
  hydrologyHeader* pheader = (hydrologyHeader*) (g_HydrologyMsg.header);
  hydrologyUpBody* uppbody = (hydrologyUpBody*) (g_HydrologyMsg.upbody);
  hydrologyDownBody* downpbody = (hydrologyDownBody*) (g_HydrologyMsg.downbody);
  int i,bodylen;
  int type = 0;

  type = hydrologyJudgeType(funcode);

  memcpy(buffer,g_HydrologyMsg.header,sizeof(hydrologyHeader));
  *pbuffer = sizeof(hydrologyHeader) - 3;

  if(HYDROLOGY_MODE == M3)
  {
    g_HydrologyMsg.end = ETB;
    if(PacketSendTimes == SinglePacketSendCount)
      g_HydrologyMsg.end = ETX;
  }
  else
    g_HydrologyMsg.end = ETX;

  if(type == 1)
  {
    memcpy(&buffer[*pbuffer],g_HydrologyMsg.upbody,23);
    *pbuffer += 23;
    for(i = 0;i < uppbody->count;i++)
    {
      memcpy(&buffer[*pbuffer],(uppbody->element)[i].guide,2);
      *pbuffer += 2;
      memcpy(&buffer[*pbuffer],(uppbody->element)[i].value, (uppbody->element)[i].num);
      *pbuffer += (uppbody->element)[i].num;
    }
    buffer[*pbuffer] = g_HydrologyMsg.end;
    *pbuffer += 1;
    bodylen = *pbuffer - 15;
    pheader->dir_len[0] |= bodylen>>8;
    pheader->dir_len[1] |= bodylen&0xFF;
    memcpy(&buffer[11],pheader->dir_len,2);
    
    g_HydrologyMsg.crc16 = hydrologyCRC16(buffer,*pbuffer);
    buffer[*pbuffer] = g_HydrologyMsg.crc16 >> 8;
    *pbuffer += 1;
    buffer[*pbuffer] = g_HydrologyMsg.crc16 & 0xFF;
    *pbuffer += 1;
  }
  else if(type == 2)
  {
    memcpy(&buffer[*pbuffer],g_HydrologyMsg.upbody,8);
    *pbuffer += 8;
    buffer[*pbuffer] = g_HydrologyMsg.end;
    *pbuffer += 1;
    bodylen = *pbuffer - 15;
    pheader->dir_len[0] |= bodylen>>8;
    pheader->dir_len[1] |= bodylen&0xFF;
    memcpy(&buffer[11],pheader->dir_len,2);
    
    g_HydrologyMsg.crc16 = hydrologyCRC16(buffer,*pbuffer);
    buffer[*pbuffer] = g_HydrologyMsg.crc16 >> 8;
    *pbuffer += 1;
    buffer[*pbuffer] = g_HydrologyMsg.crc16 & 0xFF;
    *pbuffer += 1;
  }
  else if(type == 3)
  {
    memcpy(&buffer[*pbuffer],g_HydrologyMsg.upbody,15);
    *pbuffer += 15;
    for(i = 0;i < downpbody->count;i++)
    {
      memcpy(&buffer[*pbuffer],(downpbody->element)[i].guide,2);
      *pbuffer += 2;
      memcpy(&buffer[*pbuffer],(downpbody->element)[i].value, (downpbody->element)[i].num);
      *pbuffer += (downpbody->element)[i].num;
    }
    buffer[*pbuffer] = g_HydrologyMsg.end;
    *pbuffer += 1;
    bodylen = *pbuffer - 15;
    pheader->dir_len[0] |= bodylen>>8;
    pheader->dir_len[1] |= bodylen&0xFF;
    memcpy(&buffer[11],pheader->dir_len,2);
    
    g_HydrologyMsg.crc16 = hydrologyCRC16(buffer,*pbuffer);
    buffer[*pbuffer] = g_HydrologyMsg.crc16 >> 8;
    *pbuffer += 1;
    buffer[*pbuffer] = g_HydrologyMsg.crc16 & 0xFF;
    *pbuffer += 1;
  }
}
	

void hydrologyInitSend()
{
   memset(&g_HydrologyUpHeader,0,sizeof(hydrologyHeader));
   
   g_HydrologyMsg.header = (void*)&g_HydrologyUpHeader;

   g_HydrologyMsg.upbody = (void*)malloc(sizeof(hydrologyUpBody));

   hydrologyUpBody* uppbody = (hydrologyUpBody*) (g_HydrologyMsg.upbody);
   uppbody->count = 0;
   
   if (0 == g_HydrologyMsg.upbody)
      Hydrology_Printf("g_HydrologyMsg.upbody malloc failed");
   
   for(int i = 0; i < UserElementCount; ++i)
   {
      switch(Element_table[i].Mode)
      {
        case ADC:
        {
          ADCElementCount++;
          break;
        }
        case ISR_COUNT:
        {
          ISR_COUNTElementCount++;
          break;
        }
        case IO_STATUS:
        {
          IO_STATUSElementCount++;
          break;
        }
        case RS485:
        {
          RS485ElementCount++;
          break;
        }
      }
   }
}

void hydrologyExitSend()
{
  int i = 0;
  hydrologyUpBody* uppbody = (hydrologyUpBody*) (g_HydrologyMsg.upbody);

  for ( i = 0; i < uppbody->count;i++)
  {
    if((uppbody->element)[i].value != NULL)
    {
      free((uppbody->element)[i].value);
      (uppbody->element)[i].value = NULL;
    }
    if(inputPara[i].value != NULL)
    {
      free(inputPara[i].value);
      inputPara[i].value = NULL;
    }
  }
  free(uppbody);
}

int hydrologyProcessSend(char funcode)
{
  int elecount = 0;
  int bufferlen = 0;
  char buffer[300];

  memset(buffer,0,sizeof(buffer));

  hydrologyInitSend();

  Hydrology_CalElementInfo(&elecount,funcode);

  hydrologyMakeUpHeader(funcode);

  hydrologyMakeUpBodyBasicInfo(funcode);

  hydrologyMakeUpBody(elecount);

  hydrologyMakeUpTail(buffer,&bufferlen,funcode);

  Hydrology_SendData(buffer,bufferlen);

  hydrologyExitSend();

  return 0;
}

int hydrologyNeedRespond(char funcode)
{
  int trueorfalse = 0;
  switch(funcode)
  {
    case LinkMaintenance:
    {
      trueorfalse = 0;
      break;
    }
    case Test:
    {
      trueorfalse = 0;
      break;
    }
    case EvenPeriodInformation:
    {
      trueorfalse = 1;
      break;
    }
    case TimerReport:
    {
      trueorfalse = 0;
      break;
    }
    case AddReport:
    {
      trueorfalse = 1;
      break;
    }
    case Hour:
    {
      trueorfalse = 0;
      break;
    }
    case ArtificialNumber:
    {
      trueorfalse = 0;
      break;
    }
    case Picture:
    {
      trueorfalse = 1;
      break;
    }
    case Realtime:
    {
      trueorfalse = 1;
      break;
    }
    case Period:
    {
      trueorfalse = 1;
      break;
    }
    case InquireArtificialNumber:
    {
      trueorfalse = 1;
      break;
    }
    case SpecifiedElement:
    {
      trueorfalse = 1;
      break;
    }
    case ConfigurationModification:
    {
      trueorfalse = 1;
      break;
    }
    case ConfigurationRead:
    {
      trueorfalse = 1;
      break;
    }
    case ParameterModification:
    {
      trueorfalse = 1;
      break;
    }
    case ParameterRead:
    {
      trueorfalse = 1;
      break;
    }
    case WaterPumpMotor:
    {
      trueorfalse = 1;
      break;
    }
    case SoftwareVersion:
    {
      trueorfalse = 1;
      break;
    }
    case Status:
    {
      trueorfalse = 1;
      break;
    }
    case InitializeSolidStorage:
    {
      trueorfalse = 1;
      break;
    }
    case Reset:
    {
      trueorfalse = 1;
      break;
    }        
    case ChangePassword:
    {
      trueorfalse = 1;
      break;
    }        
    case SetClock:
    {
      trueorfalse = 1;
      break;
    }
    case SetICCard:
    {
      trueorfalse = 1;
      break;
    }
    case Pump:
    {
      trueorfalse = 1;
      break;
    }
    case Valve:
    {
      trueorfalse = 1;
      break;
    }        
    case Gate:
    {
      trueorfalse = 1;
      break;
    }
    case WaterSetting:
    {
      trueorfalse = 1;
      break;
    }
    case Record:
    {
      trueorfalse = 1;
      break;
    }       
    case Time:
    {
      trueorfalse = 1;
      break;
    }
    default:
      break;
  }    
  return trueorfalse;
}

void hydrologyQueryRespond(char funcode)
{
  int type = 0;
  int trueorfalse = 0;

  trueorfalse = hydrologyNeedRespond(funcode);
  
  if(trueorfalse)
  {
    type = hydrologyJudgeType(funcode);
    if(type == 1)
    {
      hydrologyProcessSend(funcode);
    }
    else if(type == 2)
    {
      hydrologyProcessSend(funcode);
    }
    else if(type == 3)
    {
      hydrologyProcessSend(funcode);
    }
  }
  
}

void hydrologyInitReceieve()
{
   memset(&g_HydrologyDownHeader,0,sizeof(hydrologyHeader));

   g_HydrologyMsg.downbody = (void*)malloc(sizeof(hydrologyDownBody));

   hydrologyDownBody* downpbody = (hydrologyDownBody*) (g_HydrologyMsg.downbody);
   downpbody->count = 0;
   
   if (0 == g_HydrologyMsg.downbody)
      Hydrology_Printf("g_HydrologyMsg.downbody malloc failed");
}

void hydrologyExitReceieve()
{
  int i = 0;

  hydrologyDownBody* downpbody = (hydrologyDownBody*) (g_HydrologyMsg.downbody);
  for ( i = 0; i < downpbody->count;i++)
  {
    if((downpbody->element)[i].value != NULL)
    {
      free((downpbody->element)[i].value);
      (downpbody->element)[i].value = NULL;
    }
  }
  free(downpbody);
}

int hydrologyProcessReceieve(char* input,int inputlen)
{
  int i = 0,bodylen = 0;
    
  hydrologyInitReceieve();
  
  if(hydrologyMakeDownHeader(input,inputlen,&i,&bodylen) != 0)
    return -1;
  
  hydrologyMakeDownBody(input,bodylen,i);
  
  hydrologyCommand(input[10]);
  
  hydrologyQueryRespond(input[10]);
  
  hydrologyCommand(SetClock);

  hydrologyExitReceieve();
  
  return 0;
}
	
	
	

