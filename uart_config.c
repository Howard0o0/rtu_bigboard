
#include "msp430common.h"
#include "adc.h"
#include "led.h"
#include "common.h"
#include "uart3.h"
#include "string.h"

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
  int count = 10000;
  
  TraceMsg("Device is waiting for configing within 10s",1);
  while(count--)
  {
    Hydrology_ProcessUARTReceieve();
    System_Delayms(1);
  }
}