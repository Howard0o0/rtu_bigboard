/******************************************/
//      author: zh
//      date：2019.10.18

/******************************************/

#include "msp430common.h"
#include "blueTooth.h"
#include "uart1.h"
// #include "uart2.h"
#include "uart3.h"
#include "console.h"
#include "common.h"
#include <string.h>
#include <stdio.h>
#include "ioDev.h"
#include <stdint.h>

#define BLE_MAX_PROTOCOL_DATA_LEN 500
#define BLE_REPEAT_TIMES 5

int BLEINIT=0;
int sppflag=0;
int isbt=0;

void BLE_buffer_Clear() //清除BUFF
{
	UART1_ClearBuffer();
}



void SPPRX(char * result,int len) //RTU接收蓝牙消息
{   
  printf("BLETX:");
  for(int i=0;i<len;i++)
    printf("%c",result[i]);
  printf("\r\n");
      
  BLE_SendMsg(result,len);
      
}

void SPPTX(char * result,int * len) //蓝牙接收RTU消息
{
  int time=0;
  printf("10s\r\n");

  while(UART1_RecvLineWait(result,512,len)<0)
  {
      if(time>33)//30s 100
        return;
      time++;
  }
  
  printf("BLERX:");
  for(int i=0;i<*len;i++)
    printf("%x",result[i]);
  printf("\r\n");
      
      
}


void BLE_SendAtCmd(char *atCmd,int cmdLen)  //RTU给蓝牙发送AT命令
{
  BLE_buffer_Clear();
  char end[]={0x0D,0x0A};
  UART1_Send(atCmd,cmdLen,0);     //发送一次命令容易丢失回复消息
  UART1_Send(end,sizeof(end),0);
  System_Delayms(100);
  UART1_Send(atCmd,cmdLen,0); 
  UART1_Send(end,sizeof(end),0);
}


void BLE_SendMsg(char *atCmd,int cmdLen)
{
  UART1_Send(atCmd,cmdLen,0); 
}


void BLE_RecAt(char *result,int *num)   //蓝牙接收RTU发送的AT指令
{
  int _repeat = 0;
  *num=0;
  while ( _repeat < BLE_REPEAT_TIMES )
  {
    if(BLE_RecvLineTry ( result,100,num ) == 0) // rcv AT response
    {
      if(strstr(result,"RR")!=0 && strstr(result,"CO")!=0)
        BLE_RecvLineTry(result,100,num);
      break;
    }
    // printf("waiting for rec!time:%d\r\n",_repeat);
    _repeat++;
  }
}

BLERet BLE_ATE()        //ATE0 关闭AT指令回显
{
  // char cmd[] = {0x41,0x54,0x45,0x30 };
  char cmd[] = "ATE0";
  char result[100]=" ";
  int num;
  BLE_SendAtCmd(cmd,sizeof(cmd)-1);
  // printf("AT SEND: %s \r\n",cmd);
  BLE_RecAt(result,&num);
  // printf("REC:%s\r\n",result);
  if(strstr(result,"K") !=0)
  {
    return BLE_SUCCESS;
  }
  else 
    return BLE_ERROR;
  // System_Delayms(500);
}

BLERet ATTEST()  // AT	测试指令
{
      
  // char cmd[] = {0x41 ,0x54  };
  char cmd[] = "AT";
  char result[100] =" ";    
  int num;
  /* test at:AT */
  for(int i=0;i<3;i++)
  {
      BLE_buffer_Clear(); 
      BLE_SendAtCmd(cmd,sizeof(cmd)-1);
      // printf("AT SEND: %s \r\n",cmd);//
      BLE_RecAt(result,&num);
      // printf("REC:%s\r\n",result); //
      if(strstr(result,"K") != 0)
      {
        // printf("ATTEST OK!\r\n");//
        return BLE_SUCCESS;
      }
      System_Delayms(100);
  }
    return BLE_ERROR;
}


BLERet BLE_SetName ( void )//    AT+BLENAME : RTU	设置蓝牙名字为RTU
{

  // char cmd[]={0x41,0x54,0x2B,0x42,0x4C,0x45,0x41,0x44,0x56,0x44,0x41,0x54,0x41,0x3D,0x22,0x30,0x32,0x30,0x31,0x30,0x36,0x30,0x34,0x30,0x39,0x35,0x32,0x35,0x34,0x35,0x35,0x30,0x33,0x30,0x33,0x30,0x32,0x41,0x30,0x22};
  char cmd[]="AT+BLEADVDATA=\"0201060409525455030302A0\"";
  int num;
  char result[100]=" ";

  BLE_SendAtCmd(cmd,sizeof(cmd)-1);
  // printf("AT SEND: %s \r\n",cmd);
  BLE_RecAt(result,&num);
  // printf("REC:%s\r\n",result);
  if(strstr(result,"K") != 0)
  {
    return BLE_SUCCESS;
  }
  else
  {
    return BLE_ERROR;
  }
          
}

BLERet BLE_SERVER()  // //AT+BLEINIT=2  设置蓝牙模块为服务端
{
  char cmd[]="AT+BLEINIT=2";
  // char cmd[]={0x41,0x54,0x2B,0x42,0x4C,0x45,0x49,0x4E,0x49,0x54,0x3D,0x32 };
  int num;
          
  char result[100]=" ";
  BLE_SendAtCmd(cmd,sizeof(cmd)-1);
  // printf("AT SEND: %s \r\n",cmd);
  BLE_RecAt(result,&num);  
  // printf("REC:%s\r\n",result);
  if(strstr(result,"K") != 0)
  {
    return BLE_SUCCESS;
  }
  else
    return BLE_ERROR;
}



BLERet BLE_GATTSSRVCRE()// AT+BLEGATTSSRVCRE GATTS 设置蓝牙模块的服务
{
  char cmd[]="AT+BLEGATTSSRVCRE";
  // char cmd[]={0x41,0x54,0x2B,0x42,0x4C,0x45,0x47,0x41,0x54,0x54,0x53,0x53,0x52,0x56,0x43,0x52,0x45 };
  int num;
  char result[100]=" ";

  // BLE_SendAtCmd(cmd,sizeof(cmd)-1);
  BLE_buffer_Clear();
  char end[]={0x0D,0x0A};
  UART1_Send("AT",2,0);     
  UART1_Send(end,sizeof(end),0);
  System_Delayms ( 100 );
  UART1_Send(cmd,sizeof(cmd)-1,0);     
  UART1_Send(end,sizeof(end),0);

  // printf("AT SEND: %s \r\n",cmd);
  BLE_RecAt(result,&num);
  // printf("REC:%s\r\n",result);

  if(strstr(result,"K") != 0)
  {
    return BLE_SUCCESS;
  }
  else
    return BLE_ERROR;
}


BLERet BLE_GATTSSRVSTART() //AT+BLEGATTSSRVSTART 开启蓝牙模块的服务
{
  char cmd[]="AT+BLEGATTSSRVSTART";
  // char cmd[]={0x41,0x54,0x2B,0x42,0x4C,0x45,0x47,0x41,0x54,0x54,0x53,0x53,0x52,0x56,0x53,0x54,0x41,0x52,0x54  };
  int num;

  char result[100]=" ";
  BLE_SendAtCmd(cmd,sizeof(cmd)-1);
  // printf("AT SEND: %s \r\n",cmd);
  BLE_RecAt(result,&num);
  // printf("REC:%s\r\n",result);
  if(strstr(result,"K") != 0)
  {
    return BLE_SUCCESS;
  }
  else
    return BLE_ERROR;
}


BLERet BLE_ADVSTART()  //AT+BLEADVSTART 蓝牙模块开启广播  
{
  char cmd[]="AT+BLEADVSTART";
  // char cmd[]={0x41,0x54,0x2B,0x42,0x4C,0x45,0x41,0x44,0x56,0x53,0x54,0x41,0x52,0x54  };
  int num;
  
  char result[100]=" ";
  BLE_SendAtCmd(cmd,sizeof(cmd)-1);
  // printf("AT SEND: %s \r\n",cmd);
  BLE_RecAt(result,&num);
  // printf("REC:%s\r\n",result);
  if(strstr(result,"K") != 0)
  {
    return BLE_SUCCESS;
  }
  else
    return BLE_ERROR;
}


BLERet BLE_BLESPPCFG()//AT+BLESPPCFG=1,1,7,1,5  设置蓝牙模块透传参数
{
  char cmd[]="AT+BLESPPCFG=1,1,7,1,5";
  // char cmd[]={0x41,0x54,0x2B,0x42,0x4C,0x45,0x53,0x50,0x50,0x43,0x46,0x47,0x3D,0x31,0x2C,0x31,0x2C,0x37,0x2C,0x31,0x2C,0x35  };
  int num;

  char result[100]=" ";
  BLE_SendAtCmd(cmd,sizeof(cmd)-1);
  // printf("AT SEND: %s \r\n",cmd);
  BLE_RecAt(result,&num);
  // printf("REC:%s\r\n",result);
  if(strstr(result,"K") != 0)
  {
    return BLE_SUCCESS;
  }
  else
    return BLE_ERROR;
}

BLERet BLE_BLESP()//AT+BLESPP  	开启透传
{
  // System_Delayms ( 100 );     
  char cmd[]="AT+BLESPP";
	// char cmd[]={0x41,0x54,0x2B,0x42,0x4C,0x45,0x53,0x50,0x50  };
  int num;
	char result[100]=" ";

  BLE_BLESPPCFG();    //配置透传参数

	// BLE_SendAtCmd(cmd,sizeof(cmd)-1);
  BLE_buffer_Clear();
  char end[]={0x0D,0x0A};

  // UART1_Send("AT",2,0);     
  // UART1_Send(end,sizeof(end),0);
  // System_Delayms ( 100 );
  UART1_Send(cmd,sizeof(cmd)-1,0);     
  UART1_Send(end,sizeof(end),0);
  // printf("AT SEND: %s \r\n",cmd);
	// BLE_RecAt(result,&num);
  // printf("REC:%s\r\n",result);

  // if(strstr(result,"K") != 0)
  // {
  //   return BLE_SUCCESS;
  // }
  // else
  // {
    System_Delayms ( 100 );
    BLE_SendAtCmd(" ",1);
    BLE_RecAt(result,&num);
    // printf("result:%s\r\n",result);
    if(strstr(result,"RR") != 0)
      return BLE_ERROR;
    else 
      return BLE_SUCCESS;
  // }

}

BLERet BLE_BLESPP() //开启透传5次，未成功返回失败
{
  int time=0;
  sppflag=1;
  System_Delayms ( 500 );
  while(BLE_BLESP()!=BLE_SUCCESS)
  {
    time++;
    printf( "SPP...\r\n" );
    System_Delayms ( 100 );
    if(time>5)
    {
      printf("failed,please check system\r\n");
      return BLE_ERROR;
    }
  }
  printf( "SPP!!\r\n" );
  return BLE_SUCCESS;
}

BLERet BLE_BLESPPEND()//+++ 退出透传     
{
    char cmd[]="+++";
    // char cmd[]={0x2B,0x2B,0x2B};
    int num;
    BLE_buffer_Clear();
    char result[100]=" ";

    System_Delayms(1000);   
    UART1_Send(cmd,sizeof(cmd)-1,0);
    BLE_RecAt(result,&num);
    // printf("REC:%s\r\n",result);
    System_Delayms(1000);
    BLERet ret;
    ret=ATTEST();
    if(ret==BLE_SUCCESS)
      printf("SPPEND!\r\n\r\n");
    sppflag=0;
    return ret;
}

BLERet BLE_INIT()       //初始化蓝牙模块
{
     P10DIR |= BIT1;         //MCU-P101=1 
     P10OUT |= BIT1;
    P9DIR |= BIT6;           //P9.6=1
    P9OUT |= BIT6;
    UART1_Open(UART1_BT_TYPE);
    printf("uart open\r\n",9,1);

    BLE_buffer_Clear();
    ATTEST();
    
    printf("wait for 10s ...\r\n");
    System_Delayms(1000);
    
    BLERet ret = BLE_ERROR;
      
    if(BLE_BLESPPEND()==BLE_SUCCESS)
      isbt=1;
    else
    {
      isbt=0;
      printf("can not find bluetooth!!!\r\n");
      return BLE_ERROR;
    }
    // System_Delayms ( 500 );
    
    for(int i=0;i<3;i++)
    {
      ret=BLE_RST(); 
      System_Delayms ( 1000 );
      ret=BLE_ATE();
      System_Delayms ( 1000 );
      ret=ATTEST();
      System_Delayms ( 1000 );
      if(ret==BLE_SUCCESS)
        return BLE_SUCCESS;
    }
    return BLE_ERROR;
}

BLERet BLE_Open()
{
      BLERet ret = BLE_ERROR;
      System_Delayms ( 100 );
      
      ret = BLE_SERVER();
      System_Delayms ( 100 );
      if(ret == BLE_ERROR)
        return ret;
      printf("BLE server\r\n");
      
      ret = BLE_SetName(); 
      System_Delayms ( 100 );
      if(ret == BLE_ERROR)
        return ret;
      printf("BLE set name\r\n");

      ret = BLE_GATTSSRVCRE();
      System_Delayms ( 100 );
      if(ret == BLE_ERROR)
        return ret;
      printf("BLE service\r\n");
      
      ret = BLE_GATTSSRVSTART();
      System_Delayms ( 100 );
      if(ret == BLE_ERROR)
        return ret;
      printf("BLE service start\r\n");

      ret = BLE_ADVSTART();
      System_Delayms ( 100 );
      if(ret == BLE_ERROR)
        return ret;
      printf("BLE adv start\r\n");

      BLEINIT=1;

      return ret;
}

BLERet BLE_CONNECT()    //检查蓝牙是否被连接
{
  System_Delayms ( 100 );
  BLE_buffer_Clear();
  char cmd[]="AT+BLECONN?";
  // char cmd[]={0x41,0x54,0x2B,0x42,0x4C,0x45,0x43,0x4F,0x4E,0x4E,0x3F };
  int num;
  char result[100]={0};

  BLE_SendAtCmd(cmd,sizeof(cmd)-1);
  // printf("AT SEND: %s \r\n",cmd);
  BLE_RecAt(result,&num);
  // printf("REC:%s\r\n",result);
  if(strstr(result,"N:0") != 0)
  {
    System_Delayms ( 500 );
    return BLE_SUCCESS;
  }
  else 
    return BLE_ERROR;
}

BLERet BLE_ADVSTOP()    //关闭广播
{
  char cmd[]="AT+BLEADVSTOP";
  // char cmd[]={0x41,0x54,0x2B,0x42,0x4C,0x45,0x41,0x44,0x56,0x53,0x54,0x4F,0x50 };
  int num;

  char result[100]=" ";
  BLE_SendAtCmd(cmd,sizeof(cmd)-1);
  // printf("AT SEND: %s \r\n",cmd);
  BLE_RecAt(result,&num);
  // printf("REC:%s\r\n",result);
  if(strstr(result,"K") != 0)
  {
    return BLE_SUCCESS;
  }
  else
    return BLE_ERROR;
}


BLERet BLE_RST()    //重启蓝牙
{
  BLEINIT=0;

  char cmd[]="AT+RST";
  // char cmd[]={0x41,0x54,0x2B,0x52,0x53,0x54  };
  int num;
  
  char result[100]=" ";
  BLE_SendAtCmd(cmd,sizeof(cmd)-1);
  // printf("AT SEND: %s \r\n",cmd);
  BLE_RecAt(result,&num);
  // printf("REC:%s\r\n",result);
  if(strstr(result,"ea") != 0)
  {
    return BLE_SUCCESS;
  }
  else
    return BLE_ERROR;

}

BLERet BLE_SLEEP()      //蓝牙模块进入休眠状态
{
  char cmd[]="AT+CWMODE=0";
  // char cmd[]={0x41,0x54,0x2B,0x43,0x57,0x4D,0x4F,0x44,0x45,0x3D,0x30  };
  int num;

  char result[100]=" ";
  BLE_SendAtCmd(cmd,sizeof(cmd)-1);
  // printf("AT SEND: %s \r\n",cmd);
  BLE_RecAt(result,&num);
  // printf("REC:%s\r\n",result);
  if(strstr(result,"K") != 0)
  {
  }
  else
    return BLE_ERROR;

  char cmd2[]="AT+BLEINIT=0";
  // char cmd2[]={0x41,0x54,0x2B,0x42,0x4C,0x45,0x49,0x4E,0x49,0x54,0x3D,0x30 };

  BLE_SendAtCmd(cmd2,sizeof(cmd2)-1);
  // printf("AT SEND: %s \r\n",cmd2);
  BLE_RecAt(result,&num);
  // printf("REC:%s\r\n",result);
  if(strstr(result,"OK") != 0)
  {
    return BLE_SUCCESS;
  }
  else
    return BLE_ERROR;

}


void BLE_Close()
{
	UART1_Close();
}


int BLE_RecvLineTry ( char* _dest,const int _max, int* _pNum )  ////10.16
{
	if ( 0 == UART1_RecvLineWait ( _dest, _max,  _pNum ) )
	{
                _dest[*_pNum] = 0;  // end symbol
		return 0;
	}
	else
	{
		return -1;
	}
}


int BLE_MAIN()  //初始化蓝牙 配置蓝牙 广播 等待连接
{
  if(BLE_INIT()!=BLE_SUCCESS){
    return -1;
  }
        printf( "BLE Init success!\r\n\r\n" );
        
        
        int time=0;
        while(BLE_Open() != BLE_SUCCESS)
        {
          printf( "open ble failed ,reset system\r\n" );
          time++;
          System_Delayms ( 100 );
          if(time>5)
          {
            printf("failed,please check system\r\n");
            return -1;
          }
        }         
        printf( "BLE open success!\r\n\r\n" );
        
        
        
        time=0;
        while(BLE_CONNECT() != BLE_SUCCESS)
        {
          time++;
          printf( "CONNECT...\r\n" );
          System_Delayms ( 1000 );
          if(time>10)
          {
            printf("failed to connect\r\n");
            return -1;
          }
        }
        printf( "CONNECTED!\r\n\r\n" );
        
        
        // BLE_buffer_Clear();
        // printf("waiting CCCD\r\n");
        // char result[100]=" ";
        // int num;
        // int flag1=0,flag2=0;
        // for(int i=0;i<2;i++)
        // {
        //   time=0;
        //   while(1)
        //   {
        //     BLE_RecAt(result,&num);
        //     if(flag1==0 && strstr(result,"6") != 0 && strstr(result,"ITE") != 0)
        //     {
        //       printf("success6  ...\r\n");
        //       flag1=1;
        //       break;
        //     }
        //     else if(flag2==0 && strstr(result,"7") != 0 && strstr(result,"ITE") != 0)
        //     {
        //       printf("success7  ...\r\n");
        //       flag2=1;
        //       break;
        //     }
        //     else if(time>10)
        //     {
        //       printf("failed to enable CCCD\r\n");
        //       return -1;
        //     }
        //     time++;
        //     printf("waiting CCCD\r\n");
        //     System_Delayms(100);
        //   }
        // }
        // printf("CCCD!\r\n\r\n");


        

       
        
        // time=0;
        // while(BLE_BLESPP()!=BLE_SUCCESS)
        // {
        //   time++;
        //   printf( "SPP...\r\n" );
        //   System_Delayms ( 1000 );
        //   if(time>5)
        //   {
        //     printf("failed,please check system\r\n");
        //     return -1;
        //   }
        // }
        // printf( "SPP!\r\n" );
        
        // System_Delayms(2000);
        return 0;
	//System_Reset();
}
int BLE_ISFINDBT()
{
  return isbt;
}

/*
 * author   :   Howard
 * date     :   2019/10/09
 * Desc     :   BLE driver
*/
int ble_init();
int ble_isCanUse();
int ble_open();
void ble_getMsg(char *msgRecv,int *len);
int ble_sendMsg(char *msgRecv,int len);
int ble_close();

int ble_isinit();
int ble_sppflag();
void ble_rst();
void ble_adv();
void ble_task_start();
void ble_task_end();


T_IODev T_CommuteDevBLE = 
{
    .name = "BLE",
    .isCanUseFlag = 0,
    .isCanUse = ble_isCanUse,
    .open = ble_open,
    .getMsg = ble_getMsg,
    .sendMsg = ble_sendMsg,
    .close = ble_close,
    .init = ble_init,

    .restart=ble_rst,
    .isinit = ble_isinit,
    .isspp = ble_sppflag,
    .adv = ble_adv,
    .bletaskstart=ble_task_start,
    .bletaskend=ble_task_end,
};


void ble_rst()
{
  BLE_RST();
}
int ble_sppflag()
{
  return sppflag;
}
int ble_isinit()
{
  return BLEINIT;
}
void ble_adv()
{
  BLE_ADVSTART();
}
void ble_task_start()
{
  if(BLE_ISFINDBT())
  {
    if(ble_isCanUse())
    {
      if(ble_open()!=0)
      {
        ble_rst();
        ble_init();
        if(ble_open()!=0)
          ble_close();
      }
    } 
    else
    {
      ble_adv();
    }
  }
}
void ble_task_end()
{
  if(BLE_ISFINDBT())
  {
    if(ble_sppflag())
      ble_close();
  }
}


// 0 success, -1 failed
int ble_init()
{
  int iRet = BLE_MAIN();
  if(iRet == 0)
  {
    T_CommuteDevBLE.isCanUseFlag = 1;
  }
  else
  {
    T_CommuteDevBLE.isCanUseFlag = 0;
  }
  

  return iRet;
}

// 1 available, -1 not available
int ble_isCanUse()
{
  if( BLE_CONNECT() == BLE_SUCCESS )
  {
    T_CommuteDevBLE.isCanUseFlag = 1;
    return 1;
  }
  else
  {
    sppflag = 0;
    T_CommuteDevBLE.isCanUseFlag = 0;
    return 0;
  }

}

// 0 success , otherwise fail
int ble_open()
{
  int iRet;
  iRet = BLE_BLESPP();


  return iRet;
}

void ble_getMsg(char *msgRecv,int *len)
{

  SPPTX(msgRecv,len);
}

//0 success
int ble_sendMsg(char *msgRecv,int len)
{
  SPPRX(msgRecv,len);

  return 0;
}

int ble_close()
{
  return BLE_BLESPPEND();
}





void BleDriverInstall()
{
  RegisterIODev(&T_CommuteDevBLE);
}