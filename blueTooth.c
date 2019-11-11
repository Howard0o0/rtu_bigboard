/******************************************/
//      author: zh
//      date��2019.10.18

/******************************************/

#include "msp430common.h"
#include "blueTooth.h"
#include "uart1.h"
// #include "uart2.h"
#include "uart3.h"
#include "Console.h"
#include "common.h"
#include <string.h>
#include <stdio.h>
#include "ioDev.h"
#include <stdint.h>

#define BLE_MAX_PROTOCOL_DATA_LEN 500
#define BLE_REPEAT_TIMES 5

int BLEINIT=0;
int sppflag=0;

void BLE_buffer_Clear() //��BUFF
{
	UART1_ClearBuffer();
}



void SPPRX(char * result,int len) //�ֻ�����
{   
  printf("BLETX:");
  for(int i=0;i<len;i++)
    printf("%c",result[i]);
  printf("\r\n");
      
  BLE_SendMsg(result,len);
      
}

void SPPTX(char * result,int * len) //�ֻ�����
{
  int time=0;
  printf("30s\r\n");

  while(UART1_RecvLineWait(result,100,len)<0)
  {
      if(time>10)//30s 100
        return;
      time++;
  }
  
  printf("BLERX:");
  for(int i=0;i<*len;i++)
    printf("%c",result[i]);
  printf("\r\n");
      
      
}


void BLE_SendAtCmd(char *atCmd,int cmdLen)  //RTU����
{
  BLE_buffer_Clear();
  char end[]={0x0D,0x0A};
  UART1_Send(atCmd,cmdLen,0);     //��һ��OK�ղ���
  UART1_Send(end,sizeof(end),0);
  System_Delayms(100);
  UART1_Send(atCmd,cmdLen,0); 
  UART1_Send(end,sizeof(end),0);
}


void BLE_SendMsg(char *atCmd,int cmdLen)
{
  UART1_Send(atCmd,cmdLen,0); 
}


void BLE_RecAt(char *result,int *num)   //RTU����
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
    printf("waiting for rec!time:%d\r\n",_repeat);
    _repeat++;
  }
}

BLERet BLE_ATE()        //ATE0����
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

BLERet ATTEST()  // AT
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
      printf("AT SEND: %s \r\n",cmd);//
      BLE_RecAt(result,&num);
      printf("REC:%s\r\n",result); //
      if(strstr(result,"K") != 0)
      {
        // printf("ATTEST OK!\r\n");//
        return BLE_SUCCESS;
      }
      System_Delayms(100);
  }
    return BLE_ERROR;
}


BLERet BLE_SetName ( void )//    AT+BLENAME : RTU
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

BLERet BLE_SERVER()  // //AT+BLEINIT=2  ��ʼ��Ϊ�����??
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



BLERet BLE_GATTSSRVCRE()// AT+BLEGATTSSRVCRE GATTS ��������
{
  char cmd[]="AT+BLEGATTSSRVCRE";
  // char cmd[]={0x41,0x54,0x2B,0x42,0x4C,0x45,0x47,0x41,0x54,0x54,0x53,0x53,0x52,0x56,0x43,0x52,0x45 };
  int num;
  char result[100]=" ";

  // BLE_SendAtCmd(cmd,sizeof(cmd)-1);
  BLE_buffer_Clear();
  char end[]={0x0D,0x0A};
  UART1_Send("AT",2,0);     //��һ��OK�ղ���
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


BLERet BLE_GATTSSRVSTART() //AT+BLEGATTSSRVSTART��GATTS ��������
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


BLERet BLE_ADVSTART()  //AT+BLEADVSTART �����㲥  
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


BLERet BLE_BLESPPCFG()//AT+BLESPPCFG=1,1,7,1,5  ����͸��
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

BLERet BLE_BLESP()//AT+BLESPP  
{
  System_Delayms ( 100 );     //����
  char cmd[]="AT+BLESPP";
	// char cmd[]={0x41,0x54,0x2B,0x42,0x4C,0x45,0x53,0x50,0x50  };
  int num;
	char result[100]=" ";

  BLE_BLESPPCFG();    //����

	// BLE_SendAtCmd(cmd,sizeof(cmd)-1);
  BLE_buffer_Clear();
  char end[]={0x0D,0x0A};

  // UART1_Send("AT",2,0);     //��һ��OK�ղ���
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

BLERet BLE_BLESPP() //����͸��
{
  int time=0;
  sppflag=1;

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
  printf( "SPP!\r\n" );
  return BLE_SUCCESS;
}

BLERet BLE_BLESPPEND()//+++ �ر�͸��     
{
    char cmd[]="+++";
    // char cmd[]={0x2B,0x2B,0x2B};
    int num;
    BLE_buffer_Clear();
    char result[100]=" ";

    System_Delayms(1000);   //����
    UART1_Send(cmd,sizeof(cmd)-1,0);
    BLE_RecAt(result,&num);
    // printf("REC:%s\r\n",result);
    System_Delayms(1000);
    BLERet ret;
    ret=ATTEST();
    if(ret==BLE_SUCCESS)
      printf("SPPEND!\r\n");
    sppflag=0;
    return ret;
}

BLERet BLE_INIT()       //��ʼ������
{
    P10OUT |= BIT1;
    P10DIR |= BIT1;         //MCU-P101=1
    P9DIR |= BIT6;           //P9.6=1
    P9OUT |= BIT6;
    UART1_Open(1);
    printf("uart open\r\n",9,1);

    BLE_buffer_Clear();
    ATTEST();
    
    printf("wait for 10s ...\r\n");
    System_Delayms(1000);
    
    BLERet ret = BLE_ERROR;
      
    BLE_BLESPPEND();
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

BLERet BLE_CONNECT()    //�ж��Ƿ�����
{
  System_Delayms ( 100 );
  BLE_buffer_Clear();
  char cmd[]="AT+BLECONN?";
  // char cmd[]={0x41,0x54,0x2B,0x42,0x4C,0x45,0x43,0x4F,0x4E,0x4E,0x3F };
  int num;
  char result[100]=" ";

  BLE_SendAtCmd(cmd,sizeof(cmd)-1);
  printf("AT SEND: %s \r\n",cmd);
  BLE_RecAt(result,&num);
  printf("REC:%s\r\n",result);
  if(strstr(result,"N:0") != 0)
  {
    return BLE_SUCCESS;
  }
  else 
    return BLE_ERROR;
}

BLERet BLE_ADVSTOP()    //�رչ㲥
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


BLERet BLE_RST()    //����
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

BLERet BLE_SLEEP()      //����
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


int BLE_MAIN()  //����-����ʼ��-������ -������͸�� if 0 connectd , -1 not connected
{
        if(BLE_INIT()!=BLE_SUCCESS)
          return -1;
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
          // if(time>30)
          if(0)
          {
            printf("failed to connect\r\n");
            return -1;
          }
        }
        printf( "CONNECTED!\r\n\r\n" );
        
        
        
        
        time=0;
        while(BLE_BLESPPCFG() != BLE_SUCCESS)
        {
          time++;
          printf( "CONFIG...\r\n" );
          System_Delayms ( 100 );
          if(time>5)
          {
            printf("failed,please check system\r\n");
            return -1;
          }
        }
        printf( "CONFIG!\r\n\r\n" );
        
        BLE_buffer_Clear();
        printf("waiting CCCD\r\n");
        char result[100]=" ";
        int num;
        int flag1=0,flag2=0;
        for(int i=0;i<2;i++)
        {
          time=0;
          while(1)
          {
            BLE_RecAt(result,&num);
            if(flag1==0 && strstr(result,"6") != 0 && strstr(result,"ITE") != 0)
            {
              printf("success6  ...\r\n");
              flag1=1;
              break;
            }
            else if(flag2==0 && strstr(result,"7") != 0 && strstr(result,"ITE") != 0)
            {
              printf("success7  ...\r\n");
              flag2=1;
              break;
            }
            else if(time>20)
            {
              printf("failed to enable CCCD\r\n");
              return -1;
            }
            time++;
            printf("waiting CCCD\r\n");
            System_Delayms(100);
          }
        }
        printf("CCCD!\r\n\r\n");


        

        // printf("waiting CCCD\r\n");
        // char result[100];
        // BLE_RecAt(result);
        // time=0;
        // while(strstr(result,"+W") == 0)
        // {
        //   time++;
        //   //printf("REC:%s\r\n",result);
        //   printf("waiting CCCD\r\n");
        //   BLE_RecAt(result);
        //   if(time>25)
        //   {
        //     printf("failed to enable CCCD\r\n");
        //     return -1;
        //   }
        // }
        // //printf("REC:%s\r\n",result);
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
void ble_connect();
void ble_adv();

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
    .adv   = ble_adv,
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