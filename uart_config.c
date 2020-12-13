
#include "msp430common.h"
#include "adc.h"
#include "led.h"
#include "common.h"
#include "uart3.h"
#include "string.h"
#include "ioDev.h"
#include "message.h"
#include "uart1.h"
#include "main.h"

char isUARTConfig = 0;
extern bool sys_errorcode_runcode,Is_uart3_RX_INT; // LSHB 20200506

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
     // uart3 有接收中断，led8快闪0.5s。
  if(Is_uart3_RX_INT==true)
  {
       //通过Timer0_A0中断显示调试信息接收 LSHB 20200506
        sys_errorcode_runcode=69;
        Led_OffAll();
        led_flash_init(75);
        Timer0_A0_Enable(); //Timer0_A0开中断
        System_Delayms(500);           
        Timer0_A0_Clear();//Timer0_A0关中断
        Led_OffAll();
        Is_uart3_RX_INT=false;
       // end  LSHB 20200506  
  } 
    
  }

  return 0;
}

void Hydrology_InitWaitConfig()
{
  int trycount = 15000;
  
  TraceMsg("Device is waiting for configing within 10s",1);
  while(trycount--)
  {
   
    Hydrology_ProcessUARTReceieve();
    System_Delayms(1);
    
  }
  
 
  if(BLE_ISFINDBT())
  {
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
 
}