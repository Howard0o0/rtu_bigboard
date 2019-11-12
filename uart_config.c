
#include "msp430common.h"
#include "adc.h"
#include "led.h"
#include "common.h"
#include "uart3.h"
#include "string.h"
#include "ioDev.h"
#include "message.h"

char isUARTConfig = 0;

int Hydrology_ProcessUARTReceieve()
{
  char buffer[300];
  int count = 0;
  
  memset(buffer,0,300);
  
  UART3_RecvLineTry(buffer,UART3_MAXBUFFLEN,&count);

  if(count != 0)
  {
    TraceHexMsg(buffer,count);
    isUARTConfig = 1;
    hydrologyProcessReceieve(buffer, count);
  }

 

  return 0;
}

void Hydrology_InitWaitConfig()
{
  int trycount = 10000;
  
  TraceMsg("Device is waiting for configing within 10s",1);
  while(trycount--)
  {
    Hydrology_ProcessUARTReceieve();
    System_Delayms(1);
  }
  
 
   /* 蓝牙接收 */
  char buffer[300];
  int count = 0;
  PT_IODev  ptDevBle =  getIODev();
  // if(ptDevBle->isCanUse() && (ptDevBle->open() == 0))
  if(ptDevBle->isspp())
  {
      
      char acBleRcvBuf[200] = {0};
      int iLen = 0;
      ptDevBle->sendMsg("waiting config ... ",sizeof("waiting config ... "));
      printf("waing msg from phone \r\n");
      ptDevBle->getMsg(buffer,&count);
      // ptDevBle->close();
  }

  if(count != 0)
    {
      TraceHexMsg(buffer,count);
      isUARTConfig = 1;
      hydrologyProcessReceieve(buffer, count);
    }
}