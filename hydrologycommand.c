
#include "message.h"
#include "hydrologycommand.h"
#include <string.h>
#include "stdint.h"

/* USER CODE BEGIN Includes */
#include "msp430common.h"
#include "uart3.h"
#include "uart1.h"
#include "common.h"
#include "main.h"

//extern const hydrologyElementInfo Element_table[UserElementCount+1]; //++++++++++++++++++++++++++
extern  hydrologyElementInfo Element_table[MAX_ELEMENT+1]; //+++++++++++++++++++++++

extern int UserElementCount; //++++++++++++++++++++++++++++++++
int RS485RegisterCount = 0; //++++++++++
extern char s_picture_flag;
/* USER CODE END Includes */

extern unsigned int sys_errorcode_runcode;

extern bool Is_RS485_1,Is_uart1_RX_INT;


/*以下函数为外部接口函数，需由用户根据硬件设备实现，否则水文协议无法运行*/
void RS485_Dir(char inout)
{
  if(inout)
  {
//    P3DIR |= BIT0+BIT1;
//    P3OUT |= BIT0+BIT1;
//复用脚 对P100要选低
     P10DIR |= BIT4;
     P10OUT |= BIT4;
  }
  else
  {
    P10DIR |= BIT4;
    P10OUT &= ~(BIT4);
  }
}

void RS485_SerilWrite(char *buffer,int len)
{
  UART1_Send(buffer,len,0);
}

void RS485_SerilRead(char *buffer,int len)
{
  int i;
  UART1_RecvLineTry(buffer, len, &i);
}

void RS485_Delayms(int timeout)
{
  System_Delayms(timeout);
}

void RS485_Delayus(int timeout)
{
  System_Delayus(timeout);
}

void RS485_ValueDefine(char *buffer,char *value,int index,int times)
{
  float floatvalue = 0;
  uint32_t intvalue = 0;
  char buffer1[4];
  //++++++++
  char data_order = 0;
//   Hydrology_ReadStoreInfo(HYDROLOGY_RS485_RECV_DATA_ORDER1 + HYDROLOGY_RS485_RECV_DATA_ORDER_LEN * index,&data_order,HYDROLOGY_RS485_RECV_DATA_ORDER_LEN);
    Read_Flash_Segment(HYDROLOGY_RS485_RECV_DATA_ORDER1_BP + HYDROLOGY_RS485_RECV_DATA_ORDER_LEN * index,&data_order,HYDROLOGY_RS485_RECV_DATA_ORDER_LEN);
 
//+++++  
  switch(data_order) //+++++++++++++4.接收数据顺序++++字节顺序+++++++++
  {

    case 1:
    {
      buffer1[3] = buffer[3];
      buffer1[2] = buffer[2];
      buffer1[1] = buffer[1];
      buffer1[0] = buffer[0];
      memcpy(value,buffer1,4);
      break;
    }
    case 2:
    {
      buffer1[3] = buffer[0];
      buffer1[2] = buffer[1];
      buffer1[1] = buffer[2];
      buffer1[0] = buffer[3];
      memcpy(value,buffer1,4);
      break;
    }
   case 3:
    {
      buffer1[3] = buffer[2];
      buffer1[2] = buffer[3];
      buffer1[1] = buffer[0];
      buffer1[0] = buffer[1];
      memcpy(value,buffer1,4);
      break;
    } 
  case 4:
    {
      buffer1[3] = buffer[1];
      buffer1[2] = buffer[0];
      buffer1[1] = buffer[3];
      buffer1[0] = buffer[2];
      memcpy(value,buffer1,4);
      break;
    }
 case 5:
    {
      intvalue = (uint32_t)buffer[3]*16777216 + (uint32_t)buffer[2]*65536 + (uint32_t)buffer[1]*256 + (uint32_t)buffer[0];  
      memcpy(value,(char*)(&intvalue),4);
   
    break;
    }
 case 6:
    {
      intvalue = (uint32_t)buffer[2]*16777216 + (uint32_t)buffer[3]*65536 + (uint32_t)buffer[0]*256 + (uint32_t)buffer[1];  
      memcpy(value,(char*)(&intvalue),4);
   
    break;
    }
 case 7:
    {
      intvalue = (uint32_t)buffer[1]*16777216 + (uint32_t)buffer[0]*65536 + (uint32_t)buffer[3]*256 + (uint32_t)buffer[2];  
      memcpy(value,(char*)(&intvalue),4);
   
    break;
    }
 case 8:
    {
      intvalue = (uint32_t)buffer[0]*16777216 + (uint32_t)buffer[1]*65536 + (uint32_t)buffer[2]*256 + (uint32_t)buffer[3];  
      memcpy(value,(char*)(&intvalue),4);
   
    break;
    }
case 9:
    {
     intvalue =  (uint32_t)buffer[0]*256 + (uint32_t)buffer[1];
     floatvalue = (float)(intvalue)/1000.0;
      
     memcpy(value,(char*)(&floatvalue),4);
    break;
    }
    default:
    {
      break;
    }
  }
}
/*以上函数为外部接口函数，需由用户根据硬件设备实现，否则水文协议无法运行*/

void RS485_Send(char device,char function,char reg_addrH,char reg_addrL,char timeout,int index)
{
  char command[8];
  short crc = 0;
  //+++++++
  char crc_type = 0;
  char _crc_order[2] = {0};
//   Hydrology_ReadStoreInfo(HYDROLOGY_RS485_CRC_TYPE1 + HYDROLOGY_RS485_CRC_TYPE_LEN * index,&crc_type,HYDROLOGY_RS485_CRC_TYPE_LEN);
//   Hydrology_ReadStoreInfo(HYDROLOGY_RS485_CRC_ORDER1 + HYDROLOGY_RS485_CRC_ORDER_LEN * index,_crc_order,HYDROLOGY_RS485_CRC_ORDER_LEN);
   Read_Flash_Segment(HYDROLOGY_RS485_CRC_TYPE1_BP + HYDROLOGY_RS485_CRC_TYPE_LEN * index,&crc_type,HYDROLOGY_RS485_CRC_TYPE_LEN);
   Read_Flash_Segment(HYDROLOGY_RS485_CRC_ORDER1_BP + HYDROLOGY_RS485_CRC_ORDER_LEN * index,_crc_order,HYDROLOGY_RS485_CRC_ORDER_LEN);
   
  //+++++++
  command[0] = device;
  command[1] = function;
  command[2] = reg_addrH ;
  command[3] = reg_addrL;
  command[4] = 0;
  command[5] = 2;
  if(crc_type == 0x01)
  {
  crc = hydrologyCRC16(command,6);   //++++++++++++1.发送crc校验类型和2.大小端顺序++++++++++++++
  
    if(_crc_order[0] == 0x01) //高位在前，低位在后
    {
       command[6] = crc>> 8;
       command[7] = crc% 256;
 
    }
    else
    {
       command[6] = crc% 256;
       command[7] = crc>> 8;
    }
  }
  else if(crc_type == 0x02)
  {
    
   command[6] = _crc_order[0];
   command[7] = _crc_order[1];
  }
 
  //RS485_Dir(1);
  RS485_SerilWrite(command,8);
  TraceHexMsg(command,8);    //++++++++++++++++++++++++++++++++++
  RS485_Delayms(1);
  RS485_Delayus(500);
 // RS485_Dir(0);
}

int RS485_Read(char *buffer,int index)
{
 
  char temp_buffer[9];
  short crc1 = 0;
  short crc2 = 0;
  char recv_crc_order = 0;

  RS485_SerilRead(temp_buffer,9);
   TraceHexMsg(temp_buffer,9);
  
//  Hydrology_ReadStoreInfo(HYDROLOGY_RS485_RECV_CRC_ORDER1 + HYDROLOGY_RS485_RECV_CRC_ORDER_LEN * index,&recv_crc_order,HYDROLOGY_RS485_RECV_CRC_ORDER_LEN);
    Read_Flash_Segment(HYDROLOGY_RS485_RECV_CRC_ORDER1_BP + HYDROLOGY_RS485_RECV_CRC_ORDER_LEN * index,&recv_crc_order,HYDROLOGY_RS485_RECV_CRC_ORDER_LEN);
    
//  memcpy(temp_buffer2,temp_buffer,9);
//  memcpy(&temp_buffer[1],temp_buffer2,8);
//  temp_buffer[0] = 0x01;

  
      if(recv_crc_order == 0x01) ////高位在前，低位在后 
      {
          crc1 = hydrologyCRC16(temp_buffer,7);          //++++++++++++5.接收crc校验大小端顺序++++++++++++++
          crc2 = temp_buffer[7]<<8 | temp_buffer[8];
      }
      else if(recv_crc_order == 0x02)   //低位在前，高位在后
      { 
        crc1 = hydrologyCRC16(temp_buffer,7);
        crc2 = temp_buffer[8]<<8 | temp_buffer[7];   
      }
  
   
  if(crc1 == crc2)
  {
    memcpy(buffer,&temp_buffer[3],4);
    
    //通过Timer0_A0中断显示第一通道485正在接收数据 LSHB 20200506
  sys_errorcode_runcode=67;
  Led_OffAll();
  led_flash_init(15);
  Timer0_A0_Enable(); //Timer0_A0开中断
  System_Delayms(1000);
  Timer0_A0_Clear();//Timer0_A0关中断
  Led_OffAll();
 // end  LSHB 20200506
    
    return 0;
  }
  else
  {
  Hydrology_Printf("RS485 crc check failed!");
  
 //通过Timer0_A0中断显示485未连接，或连接错误 LSHB 20200506
  
  if((Is_RS485_1&Is_uart1_RX_INT)==true)  //RS485-1中断口UART1    
  {
  sys_errorcode_runcode=7;
  //Led1_WorkLightOff();//工作指示灯关
  Led_OffAll();
  led_flash_init(45);
  Timer0_A0_Enable(); //Timer0_A0开中断
  System_Delayms(3000);
  Timer0_A0_Clear();//Timer0_A0关中断
  Led_OffAll();
   Led1_WorkLightOff();//工作指示灯关,节能
  
  Is_uart1_RX_INT=false;
  } 
  // end  LSHB 20200506
  
    return -1;
  }
}

void Hydrology_ReadRS485(char *value,int index)
{
  char temp_value[5] = {0,0,0,0,0};
  // char temp_value[5] = {0x01,0x04,0x10,0x10,2};
  // char read_value[5]; //debug
 // char test[3]={3,4,5};
  char buffer[4] = {0,0,0,0};
  char single_count1 = 0;
  volatile char single_count2 = 0;
  static char rs485_count = 0;
  int i = 0,j = 0,error = 0;
  
  //+++++++++++
  char delay_time[2] = {0};
  int _delay_time = 0;
  //+++++++++
//  Hydrology_ReadStoreInfo(HYDROLOGY_RS485_COUNT1 + HYDROLOGY_RS485_COUNT_LEN * index,&single_count1,HYDROLOGY_RS485_COUNT_LEN);
   Read_Flash_Segment(HYDROLOGY_RS485_COUNT1_BP + HYDROLOGY_RS485_COUNT_LEN * index,&single_count1,HYDROLOGY_RS485_COUNT_LEN);
  single_count2 = single_count1;
  while(single_count1--)
  {

    //Hydrology_WriteStoreInfo(HYDROLOGY_RS4851,temp_value,HYDROLOGY_RS485_LEN);   //debug
    
    
//    Hydrology_ReadStoreInfo(HYDROLOGY_RS4851 + HYDROLOGY_RS485_LEN * (i+rs485_count),temp_value,HYDROLOGY_RS485_LEN);
//    Hydrology_ReadStoreInfo(HYDROLOGY_RS485_RECV_DELAY1 + HYDROLOGY_RS485_RECV_DELAY_LEN * index,delay_time,HYDROLOGY_RS485_RECV_DELAY_LEN);   
    Read_Flash_Segment(HYDROLOGY_RS4851_BP + HYDROLOGY_RS485_LEN * (i+rs485_count),temp_value,HYDROLOGY_RS485_LEN);
    Read_Flash_Segment(HYDROLOGY_RS485_RECV_DELAY1_BP + HYDROLOGY_RS485_RECV_DELAY_LEN * index,delay_time,HYDROLOGY_RS485_RECV_DELAY_LEN);
    
    delay_time[0] = _BCDtoHEX(delay_time[0]);
    delay_time[1] = _BCDtoHEX(delay_time[1]);
    _delay_time = delay_time[0]*100 + delay_time[1];
    
    for(error = 0;error < 3;error++)
    { 
      
      RS485_Send(temp_value[0],temp_value[1],temp_value[2],temp_value[3],temp_value[4],index);
      RS485_Delayms(_delay_time);  //++++++++++++3.接收延时++++++++5000+++++
      
       //通过Timer0_A0中断显示第一通道485正在发送数据 LSHB 20200506
        sys_errorcode_runcode=66;
        Led_OffAll();
        led_flash_init(30);
        Timer0_A0_Enable(); //Timer0_A0开中断
        System_Delayms(1000);
        Timer0_A0_Clear();//Timer0_A0关中断
        Led_OffAll();
       // end  LSHB 20200506
      
      if(RS485_Read(buffer,index) != 0)
        continue;
      else
      {
        RS485_ValueDefine(buffer,value,index,j++);
        break;
      }
    }
    i++;
  }
  
  if(index + 1 == RS485RegisterCount)
    rs485_count = 0;
  else
    rs485_count += single_count2;
}
/*读取开关量状态值，一个字节表示8个io口*/
/*
void Hydrology_ReadIO_STATUS(char *value,int flag)  //1是对P2开关输入，2是io9-16，3是io17-24，对P8开关输入（分时复用）++++++++++++++++++++++++++++++++++
{
   //开关量 
   char _tempChar1=BIT0;
   char *temp = value;

   if(1 == flag)
   {    
          
      for(int i=0;i<8;++i)
      {
            if(P2IN & _tempChar1)
            {
                *temp |=1<<i;
            }
            else
            {
                *temp &=~(1<<i);
            }
         
         //判断下一个
        _tempChar1= _tempChar1 << 1;
         
    }
   }
   if(2 == flag)
   {
      P11DIR |= BIT0+BIT1+BIT2;             
      P11OUT &=~(BIT0+BIT1+BIT2);
      
    for(int i=0;i<8;i++)
    {
        if(P8IN & _tempChar1)
            {
                *temp |=1<<i;
            }
            else
            {
               *temp &=~(1<<i);
            }
         _tempChar1= _tempChar1 << 1;
    }
   
   }
   
   if(3 == flag)
   {
     P11DIR |= BIT0+BIT1+BIT2;
     P11OUT |=BIT0;
     P11OUT &=~(BIT1+BIT2);
     
    for(int i=0;i<8;i++)
    {
        if(P8IN & _tempChar1)
            {
                *temp |=1<<i;
            }
            else
            {
                *temp &=~(1<<i);
            }
         _tempChar1= _tempChar1 << 1;
    }
   
   }
   return ;


}
*/
unsigned int binary_to_gray(unsigned int x)
{
	return x ^ (x >> 1);
}
unsigned char get_io_status_parse_type(void) 
{
	char switch_parse_type;
	//Hydrology_ReadStoreInfo(HYDROLOGY_SWITCH_PARSE_TYPE, &switch_parse_type,HYDROLOGY_SWITCH_PARSE_TYPE_LEN);
        Read_Flash_Segment(HYDROLOGY_SWITCH_PARSE_TYPE_BP, &switch_parse_type,HYDROLOGY_SWITCH_PARSE_TYPE_LEN);
	return ( unsigned char )switch_parse_type;
}
/*读取开关量状态值，一个字节表示8个io口*/
void Hydrology_ReadIO_STATUS(char *value,int flag)  //1是对P2开关输入，2是io9-16，3是io17-24，对P8开关输入（分时复用）++++++++++++++++++++++++++++++++++
{
   //开关量 
   //char _tempChar0=BIT0;
   char _tempChar1=BIT0;
   char _tempChar2=BIT0;
   char _tempChar3=BIT0;
   char *temp = value;

 /*
      for(int i=0;i<2;++i)
      {
        if(P2IN & _tempChar0)
            {
                *(&temp[0]) |=1<<i;
            }
            else
            {
                *(&temp[0]) &=~(1<<i);
            }
        _tempChar0= _tempChar0 << 1;
      }
      
      if(P2IN & BIT2)
            {
                *(&temp[0]) |=1<<7;
            }
            else
            {
                *(&temp[0]) &=~(1<<7);
            }
       if(P2IN & BIT3)
            {
                *(&temp[1]) |=1<<2;
            }
            else
            {
                *(&temp[1]) &=~(1<<2);
            }
          
      for(int i=4;i<8;++i)
      {
            if(P2IN & _tempChar1)
            {
                *(&temp[1]) |=1<<i;
            }
            else
            {
                *(&temp[1]) &=~(1<<i);
            }
         
         //判断下一个
        _tempChar1= _tempChar1 << 1;
         
      }*/
   
      
      *(&temp[0]) = 0;
      
     for(int i=0;i<8;i++)
        {
        if(P2IN & _tempChar1)
            {
                *(&temp[1]) |=1<<i;
            }
            else
            {
               *(&temp[1]) &=~(1<<i);
            }
         _tempChar1= _tempChar1 << 1;
        }
    
      P11DIR |= BIT0+BIT1;             
      P11OUT &=~(BIT0+BIT1);
      
    for(int i=0;i<8;i++)
    {
        if(P8IN & _tempChar2)
            {
                *(&temp[2]) |=1<<i;
            }
            else
            {
               *(&temp[2]) &=~(1<<i);
            }
         _tempChar2= _tempChar2 << 1;
    }
   
   
   

     P11DIR |= BIT0+BIT1;
     P11OUT |=BIT0;
     P11OUT &=~BIT1;
     
    for(int i=0;i<8;i++)
    {
        if(P8IN & _tempChar3)
            {
                *(&temp[3]) |=1<<i;
            }
            else
            {
                *(&temp[3]) &=~(1<<i);
            }
         _tempChar3= _tempChar3 << 1;
    }
   
    *(&temp[1]) = ~(*(&temp[1]));
     *(&temp[2]) = ~(*(&temp[2]));
      *(&temp[3]) = ~(*(&temp[3]));
      if (get_io_status_parse_type() == HYDROLOGY_SWITCH_PARSE_TYPE_GRAY_CODE) 
      {
		unsigned int io_status_group_1 = (unsigned int) * (&temp[ 1 ]);
		unsigned int io_status_group_2 = (unsigned int) * (&temp[ 2 ]);
		unsigned int io_status_group_3 = (unsigned int) * (&temp[ 3 ]);

		unsigned int io_status_channel_1_gray =
			(io_status_group_1 << 4) | (io_status_group_2 >> 4);
		unsigned int io_status_channel_2_gray =
			((io_status_group_2 & 0xF) << 8) | io_status_group_3;

		io_status_channel_1_gray = binary_to_gray(io_status_channel_1_gray);
		io_status_channel_2_gray = binary_to_gray(io_status_channel_2_gray);

		*(&temp[ 1 ]) = ( char )(io_status_channel_1_gray >> 4);
		*(&temp[ 2 ]) = ( char )((io_status_channel_1_gray & 0xF) << 4
					 | (io_status_channel_2_gray >> 8));
		*(&temp[ 3 ]) = ( char )(io_status_channel_2_gray & 0xFF);
      }
   return ;


}
/*设置开关量,开关量输出*/
void Hydrology_SetIO_STATUS(char *value)          //+++++++++++++++++++++++++++++++++++++++
{
    char _temp = 0xff;
    char _temp2 = *value;
    
    P4DIR |= _temp;   // temp2
    P4OUT |= _temp2;

    //驱动IO口
    //对该位进行设置

    return ;

}
void hydrologyChangeMode(char M)
{
//    HYDROLOGY_MODE = M;
}


int hydrologyConfigSelect(char* guide,char* value)
{
  int ret;

  switch(guide[0])
  {
    case 0x01:
    {
      ret = Hydrology_WriteStoreInfo(HYDROLOGY_CENTER_ADDR,value,HYDROLOGY_CENTER_LEN);
      break;
    }
    case 0x02:
    {
      ret = Hydrology_WriteStoreInfo(HYDROLOGY_REMOTE_ADDR,value,HYDROLOGY_REMOTE_LEN);
      break;
    }
    case 0x03:
    {
      ret = Hydrology_WriteStoreInfo(HYDROLOGY_PASSWORD_ADDR,value,HYDROLOGY_PASSWORD_LEN);
      break;
    }
    case 0x04:
    {
      ret = Hydrology_WriteStoreInfo(HYDROLOGY_CENTER1_IP,value,HYDROLOGY_CENTER_IP_LEN);
      break;
    }
    case 0x05:
    {
      ret = Hydrology_WriteStoreInfo(HYDROLOGY_BACKUP1_IP,value,HYDROLOGY_BACKUP_IP_LEN);
      break;
    }
    case 0x06:
    {
      ret = Hydrology_WriteStoreInfo(HYDROLOGY_CENTER2_IP,value,HYDROLOGY_CENTER_IP_LEN);
      break;
    }
    case 0x07:
    {
      ret = Hydrology_WriteStoreInfo(HYDROLOGY_BACKUP2_IP,value,HYDROLOGY_BACKUP_IP_LEN);
      break;
    }
    case 0x08:
    {
      ret = Hydrology_WriteStoreInfo(HYDROLOGY_CENTER3_IP,value,HYDROLOGY_CENTER_IP_LEN);
      break;
    }
    case 0x09:
    {
      ret = Hydrology_WriteStoreInfo(HYDROLOGY_BACKUP3_IP,value,HYDROLOGY_BACKUP_IP_LEN);
      break;
    }
    case 0x0A:
    {
      ret = Hydrology_WriteStoreInfo(HYDROLOGY_CENTER4_IP,value,HYDROLOGY_CENTER_IP_LEN);
      break;
    }
    case 0x0B:
    {
      ret = Hydrology_WriteStoreInfo(HYDROLOGY_BACKUP4_IP,value,HYDROLOGY_BACKUP_IP_LEN);
      break;
    }
    case 0x0C:
    {
      ret = Hydrology_WriteStoreInfo(HYDROLOGY_WORK_MODE,value,HYDROLOGY_WORK_MODE_LEN);
      break;
    }
    case 0x0D:
    {
      ret = Hydrology_WriteStoreInfo(HYDROLOGY_ELEMENT_SELECT,value,HYDROLOGY_ELEMENT_SELECT_LEN);
      break;
    }
    case 0x0E:
    {
      ret = Hydrology_WriteStoreInfo(HYDROLOGY_REPEATER_STATION,value,HYDROLOGY_REPEATER_STATION_LEN);
      break;
    }
    case 0x0F:
    {
      ret = Hydrology_WriteStoreInfo(HYDROLOGY_DEVICE_ID,value,HYDROLOGY_DEVICE_ID_LEN);
      break;
    }
    default:
    {
      ret = -1;
      break;
    }
  }
  return ret;
}

int hydrologyReadSelect(char* guide,char* value)
{
  int ret;

  switch(guide[0])
  {
    case 0x01:
    {
      ret = Hydrology_ReadStoreInfo(HYDROLOGY_CENTER_ADDR,value,HYDROLOGY_CENTER_LEN);
      break;
    }
    case 0x02:
    {
      ret = Hydrology_ReadStoreInfo(HYDROLOGY_REMOTE_ADDR,value,HYDROLOGY_REMOTE_LEN);
      break;
    }
    case 0x03:
    {
      ret = Hydrology_ReadStoreInfo(HYDROLOGY_PASSWORD_ADDR,value,HYDROLOGY_PASSWORD_LEN);
      break;
    }
    case 0x04:
    {
      ret = Hydrology_ReadStoreInfo(HYDROLOGY_CENTER1_IP,value,HYDROLOGY_CENTER_IP_LEN);
      break;
    }
    case 0x05:
    {
      ret = Hydrology_ReadStoreInfo(HYDROLOGY_BACKUP1_IP,value,HYDROLOGY_BACKUP_IP_LEN);
      break;
    }
    case 0x06:
    {
      ret = Hydrology_ReadStoreInfo(HYDROLOGY_CENTER2_IP,value,HYDROLOGY_CENTER_IP_LEN);
      break;
    }
    case 0x07:
    {
      ret = Hydrology_ReadStoreInfo(HYDROLOGY_BACKUP2_IP,value,HYDROLOGY_BACKUP_IP_LEN);
      break;
    }
    case 0x08:
    {
      ret = Hydrology_ReadStoreInfo(HYDROLOGY_CENTER3_IP,value,HYDROLOGY_CENTER_IP_LEN);
      break;
    }
    case 0x09:
    {
      ret = Hydrology_ReadStoreInfo(HYDROLOGY_BACKUP3_IP,value,HYDROLOGY_BACKUP_IP_LEN);
      break;
    }
    case 0x0A:
    {
      ret = Hydrology_ReadStoreInfo(HYDROLOGY_CENTER4_IP,value,HYDROLOGY_CENTER_IP_LEN);
      break;
    }
    case 0x0B:
    {
      ret = Hydrology_ReadStoreInfo(HYDROLOGY_BACKUP4_IP,value,HYDROLOGY_BACKUP_IP_LEN);
      break;
    }
    case 0x0C:
    {
      ret = Hydrology_ReadStoreInfo(HYDROLOGY_WORK_MODE,value,HYDROLOGY_WORK_MODE_LEN);
      break;
    }
    case 0x0D:
    {
      ret = Hydrology_ReadStoreInfo(HYDROLOGY_ELEMENT_SELECT,value,HYDROLOGY_ELEMENT_SELECT_LEN);
      break;
    }
    case 0x0E:
    {
      ret = Hydrology_ReadStoreInfo(HYDROLOGY_REPEATER_STATION,value,HYDROLOGY_REPEATER_STATION_LEN);
      break;
    }
    case 0x0F:
    {
      ret = Hydrology_ReadStoreInfo(HYDROLOGY_DEVICE_ID,value,HYDROLOGY_DEVICE_ID_LEN);
      break;
    }
    default:
    {
      ret = -1;
      break;
    }
  }
  return ret;
}

int hydrologyBasicInfoConfig(void)
{
  int i = 0;
  int ret;

  hydrologyDownBody* pbody = (hydrologyDownBody*)(g_HydrologyMsg.downbody);

  for(i=0; i< pbody->count; i++)
  {
    ret = hydrologyConfigSelect((pbody->element)[i].guide, (pbody->element)[i].value);
    if(ret == -1)
      return i+1;
  }
  return 0;
}

int hydrologyBasicInfoRead(void)
{
  int i = 0;
  int ret;

  hydrologyDownBody* pbody = (hydrologyDownBody*)(g_HydrologyMsg.downbody);

  for(i=0; i< pbody->count; i++)
  {
    ret = hydrologyReadSelect((pbody->element)[i].guide, (pbody->element)[i].value);
    if(ret == -1)
      return i+1;
  }
  return 0;
}

int hydrologySetParaSelect(char* guide,char* value)
{
  int ret;

  switch(guide[0])
  {
    case 0x20:
    {
      ret = Hydrology_WriteStoreInfo(HYDROLOGY_TIMER_INTERVAL,value,HYDROLOGY_TIMER_INTERVAL_LEN);
      break;
    }
    case 0x21:
    {
       
        /*发送图片的flag置1，存flag和发送图片的间隔*/
      /*
      if(!value)
      {  
        s_picture_flag = 0x00;
      }
      else
      {
      s_picture_flag = 0x01;
      }
      Hydrology_WriteStoreInfo(PICTURE_FLAG,&s_picture_flag,PICTURE_FLAG_LEN);
     
     ret= Hydrology_WriteStoreInfo(PICTURE_INTERVAL,value,PICTURE_INTERVAL_LEN);*/

      ret = Hydrology_WriteStoreInfo(HYDROLOGY_ADD_INTERVAL,value,HYDROLOGY_ADD_INTERVAL_LEN);
      break;
    }
    case 0x22:
    {
      ret = Hydrology_WriteStoreInfo(HYDROLOGY_RAINFALL_BEGIN_TIME,value,HYDROLOGY_RAINFALL_BEGIN_TIME_LEN);
      break;
    }
    case 0x23:
    {
      ret = Hydrology_WriteStoreInfo(HYDROLOGY_SAMPLE_INTERVAL,value,HYDROLOGY_SAMPLE_INTERVAL_LEN);
      break;
    }
    case 0x24:
    {
      ret = Hydrology_WriteStoreInfo(HYDROLOGY_WATERLEVEL_STORE_INTERVAL,value,HYDROLOGY_WATERLEVEL_STORE_INTERVAL_LEN);
      break;
    }
    case 0x25:
    {
      ret = Hydrology_WriteStoreInfo(HYDROLOGY_RAINFALL_RESOLUTION,value,HYDROLOGY_RAINFALL_RESOLUTION_LEN);
      break;
    }
    case 0x26:
    {
      ret = Hydrology_WriteStoreInfo(HYDROLOGY_WATERLEVEL_RESOLUTION,value,HYDROLOGY_WATERLEVEL_RESOLUTION_LEN);
      break;
    }
    case 0x27:
    {
      ret = Hydrology_WriteStoreInfo(HYDROLOGY_RAINFALL_ADD_THRESHOLD,value,HYDROLOGY_RAINFALL_ADD_THRESHOLD_LEN);
      break;
    }
    case 0x28:
    {
      ret = Hydrology_WriteStoreInfo(HYDROLOGY_WATERLEVEL_BASICVALUE1,value,HYDROLOGY_WATERLEVEL_BASICVALUE1_LEN);
      break;
    }
    case 0x29:
    {
      ret = Hydrology_WriteStoreInfo(HYDROLOGY_WATERLEVEL_BASICVALUE2,value,HYDROLOGY_WATERLEVEL_BASICVALUE2_LEN);
      break;
    }
    case 0x2A:
    {
      ret = Hydrology_WriteStoreInfo(HYDROLOGY_WATERLEVEL_BASICVALUE3,value,HYDROLOGY_WATERLEVEL_BASICVALUE3_LEN);
      break;
    }
    case 0x2B:
    {
      ret = Hydrology_WriteStoreInfo(HYDROLOGY_WATERLEVEL_BASICVALUE4,value,HYDROLOGY_WATERLEVEL_BASICVALUE4_LEN);
      break;
    }
    case 0x2C:
    {
      ret = Hydrology_WriteStoreInfo(HYDROLOGY_WATERLEVEL_BASICVALUE5,value,HYDROLOGY_WATERLEVEL_BASICVALUE5_LEN);
      break;
    }
    case 0x2D:
    {
      ret = Hydrology_WriteStoreInfo(HYDROLOGY_WATERLEVEL_BASICVALUE6,value,HYDROLOGY_WATERLEVEL_BASICVALUE6_LEN);
      break;
    }
    case 0x2E:
    {
      ret = Hydrology_WriteStoreInfo(HYDROLOGY_WATERLEVEL_BASICVALUE7,value,HYDROLOGY_WATERLEVEL_BASICVALUE7_LEN);
      break;
    }
    case 0x2F:
    {
      ret = Hydrology_WriteStoreInfo(HYDROLOGY_WATERLEVEL_BASICVALUE8,value,HYDROLOGY_WATERLEVEL_BASICVALUE8_LEN);
      break;
    }
    case 0x30:
    {
      ret = Hydrology_WriteStoreInfo(HYDROLOGY_WATERLEVEL_CORRECTVALUE1,value,HYDROLOGY_WATERLEVEL_CORRECTVALUE1_LEN);
      break;
    }
    case 0x31:
    {
      ret = Hydrology_WriteStoreInfo(HYDROLOGY_WATERLEVEL_CORRECTVALUE2,value,HYDROLOGY_WATERLEVEL_CORRECTVALUE2_LEN);
      break;
    }
    case 0x32:
    {
      ret = Hydrology_WriteStoreInfo(HYDROLOGY_WATERLEVEL_CORRECTVALUE3,value,HYDROLOGY_WATERLEVEL_CORRECTVALUE3_LEN);
      break;
    }
    case 0x33:
    {
      ret = Hydrology_WriteStoreInfo(HYDROLOGY_WATERLEVEL_CORRECTVALUE4,value,HYDROLOGY_WATERLEVEL_CORRECTVALUE4_LEN);
      break;
    }
    case 0x34:
    {
      ret = Hydrology_WriteStoreInfo(HYDROLOGY_WATERLEVEL_CORRECTVALUE5,value,HYDROLOGY_WATERLEVEL_CORRECTVALUE5_LEN);
      break;
    }
    case 0x35:
    {
      ret = Hydrology_WriteStoreInfo(HYDROLOGY_WATERLEVEL_CORRECTVALUE6,value,HYDROLOGY_WATERLEVEL_CORRECTVALUE6_LEN);
      break;
    }
    case 0x36:
    {
      ret = Hydrology_WriteStoreInfo(HYDROLOGY_WATERLEVEL_CORRECTVALUE7,value,HYDROLOGY_WATERLEVEL_CORRECTVALUE7_LEN);
      break;
    }
    case 0x37:
    {
      ret = Hydrology_WriteStoreInfo(HYDROLOGY_WATERLEVEL_CORRECTVALUE8,value,HYDROLOGY_WATERLEVEL_CORRECTVALUE8_LEN);
      break;
    }
    case 0x38:
    {
      ret = Hydrology_WriteStoreInfo(HYDROLOGY_ADD_WATERLEVEL1,value,HYDROLOGY_ADD_WATERLEVEL1_LEN);
      break;
    }
    case 0x39:
    {
      ret = Hydrology_WriteStoreInfo(HYDROLOGY_ADD_WATERLEVEL2,value,HYDROLOGY_ADD_WATERLEVEL2_LEN);
      break;
    }
    case 0x3A:
    {
      ret = Hydrology_WriteStoreInfo(HYDROLOGY_ADD_WATERLEVEL3,value,HYDROLOGY_ADD_WATERLEVEL3_LEN);
      break;
    }
    case 0x3B:
    {
      ret = Hydrology_WriteStoreInfo(HYDROLOGY_ADD_WATERLEVEL4,value,HYDROLOGY_ADD_WATERLEVEL4_LEN);
      break;
    }
    case 0x3C:
    {
      ret = Hydrology_WriteStoreInfo(HYDROLOGY_ADD_WATERLEVEL5,value,HYDROLOGY_ADD_WATERLEVEL5_LEN);
      break;
    }
    case 0x3D:
    {
      ret = Hydrology_WriteStoreInfo(HYDROLOGY_ADD_WATERLEVEL6,value,HYDROLOGY_ADD_WATERLEVEL6_LEN);
      break;
    }
    case 0x3E:
    {
      ret = Hydrology_WriteStoreInfo(HYDROLOGY_ADD_WATERLEVEL7,value,HYDROLOGY_ADD_WATERLEVEL7_LEN);
      break;
    }
    case 0x3F:
    {
      ret = Hydrology_WriteStoreInfo(HYDROLOGY_ADD_WATERLEVEL8,value,HYDROLOGY_ADD_WATERLEVEL8_LEN);
      break;
    }
    case 0x40:
    {
      ret = Hydrology_WriteStoreInfo(HYDROLOGY_ADD_ABOVE_THRESHOLD,value,HYDROLOGY_ADD_ABOVE_THRESHOLD_LEN);
      break;
    }
    case 0x41:
    {
      ret = Hydrology_WriteStoreInfo(HYDROLOGY_ADD_BELOW_THRESHOLD,value,HYDROLOGY_ADD_BELOW_THRESHOLD_LEN);
      break;
    }
    case 0xA9:
    {
      ret = Hydrology_WriteStoreInfo(HYDROLOGY_RS485_COUNT1,value,HYDROLOGY_RS485_COUNT_LEN);
      break;
    }
    case 0xAA:
    {
      ret = Hydrology_WriteStoreInfo(HYDROLOGY_RS485_COUNT2,value,HYDROLOGY_RS485_COUNT_LEN);
      break;
    }
    case 0xAB:
    {
      ret = Hydrology_WriteStoreInfo(HYDROLOGY_RS485_COUNT3,value,HYDROLOGY_RS485_COUNT_LEN);
      break;
    }
    case 0xAC:
    {
      ret = Hydrology_WriteStoreInfo(HYDROLOGY_RS485_COUNT4,value,HYDROLOGY_RS485_COUNT_LEN);
      break;
    }
    case 0xAD:
    {
      ret = Hydrology_WriteStoreInfo(HYDROLOGY_RS485_COUNT5,value,HYDROLOGY_RS485_COUNT_LEN);
      break;
    }
    case 0xAE:
    {
      ret = Hydrology_WriteStoreInfo(HYDROLOGY_RS485_COUNT6,value,HYDROLOGY_RS485_COUNT_LEN);
      break;
    }
    case 0xAF:
    {
      ret = Hydrology_WriteStoreInfo(HYDROLOGY_RS485_COUNT7,value,HYDROLOGY_RS485_COUNT_LEN);
      break;
    }
    case 0xB0:
    {
      ret = Hydrology_WriteStoreInfo(HYDROLOGY_RS485_COUNT8,value,HYDROLOGY_RS485_COUNT_LEN);
      break;
    }
    case 0xB1:
    {
      ret = Hydrology_WriteStoreInfo(HYDROLOGY_RS485_COUNT9,value,HYDROLOGY_RS485_COUNT_LEN);
      break;
    }
    case 0xB2:
    {
      ret = Hydrology_WriteStoreInfo(HYDROLOGY_RS485_COUNT10,value,HYDROLOGY_RS485_COUNT_LEN);
      break;
    }
    case 0xB3:
    {
      ret = Hydrology_WriteStoreInfo(HYDROLOGY_RS4851,value,HYDROLOGY_RS485_LEN);
      break;
    }
    case 0xB4:
    {
      ret = Hydrology_WriteStoreInfo(HYDROLOGY_RS4852,value,HYDROLOGY_RS485_LEN);
      break;
    }
    case 0xB5:
    {
      ret = Hydrology_WriteStoreInfo(HYDROLOGY_RS4853,value,HYDROLOGY_RS485_LEN);
      break;
    }
    case 0xB6:
    {
      ret = Hydrology_WriteStoreInfo(HYDROLOGY_RS4854,value,HYDROLOGY_RS485_LEN);
      break;
    }
    case 0xB7:
    {
      ret = Hydrology_WriteStoreInfo(HYDROLOGY_RS4855,value,HYDROLOGY_RS485_LEN);
      break;
    }
    case 0xB8:
    {
      ret = Hydrology_WriteStoreInfo(HYDROLOGY_RS4856,value,HYDROLOGY_RS485_LEN);
      break;
    }
    case 0xB9:
    {
      ret = Hydrology_WriteStoreInfo(HYDROLOGY_RS4857,value,HYDROLOGY_RS485_LEN);
      break;
    }
    case 0xBA:
    {
      ret = Hydrology_WriteStoreInfo(HYDROLOGY_RS4858,value,HYDROLOGY_RS485_LEN);
      break;
    }
    case 0xBB:
    {
      ret = Hydrology_WriteStoreInfo(HYDROLOGY_RS4859,value,HYDROLOGY_RS485_LEN);
      break;
    }
    case 0xBC:
    {
      ret = Hydrology_WriteStoreInfo(HYDROLOGY_RS48510,value,HYDROLOGY_RS485_LEN);
      break;
    }
    case 0xBD:
    {
      ret = Hydrology_WriteStoreInfo(HYDROLOGY_RS48511,value,HYDROLOGY_RS485_LEN);
      break;
    }
    case 0xBE:
    {
      ret = Hydrology_WriteStoreInfo(HYDROLOGY_RS48512,value,HYDROLOGY_RS485_LEN);
      break;
    }
    case 0xBF:
    {
      ret = Hydrology_WriteStoreInfo(HYDROLOGY_RS48513,value,HYDROLOGY_RS485_LEN);
      break;
    }
    case 0xC0:
    {
      ret = Hydrology_WriteStoreInfo(HYDROLOGY_RS48514,value,HYDROLOGY_RS485_LEN);
      break;
    }
    case 0xC1:
    {
      ret = Hydrology_WriteStoreInfo(HYDROLOGY_RS48515,value,HYDROLOGY_RS485_LEN);
      break;
    }
    case 0xC2:
    {
      ret = Hydrology_WriteStoreInfo(HYDROLOGY_RS48516,value,HYDROLOGY_RS485_LEN);
      break;
    }
    case 0xC3:
    {
      ret = Hydrology_WriteStoreInfo(HYDROLOGY_RS48517,value,HYDROLOGY_RS485_LEN);
      break;
    }
    case 0xC4:
    {
      ret = Hydrology_WriteStoreInfo(HYDROLOGY_RS48518,value,HYDROLOGY_RS485_LEN);
      break;
    }
    case 0xC5:
    {
      ret = Hydrology_WriteStoreInfo(HYDROLOGY_RS48519,value,HYDROLOGY_RS485_LEN);
      break;
    }
    case 0xC6:
    {
      ret = Hydrology_WriteStoreInfo(HYDROLOGY_RS48520,value,HYDROLOGY_RS485_LEN);
      break;
    }
    case 0xFF:
    {
      //ret = Hydrology_WriteElementTable();  //遇到0xff，就是一个element table里的元素  
      break;
    }
    default:
    {
      ret = -1;
      break;
    }
  }
  return ret;
}

int hydrologyReadParaSelect(char* guide,char* value)
{
  int ret;

  switch(guide[0])
  {
    case 0x20:
    {
      ret = Hydrology_ReadStoreInfo(HYDROLOGY_TIMER_INTERVAL,value,HYDROLOGY_TIMER_INTERVAL_LEN);
      break;
    }
    case 0x21:
    {
      ret = Hydrology_ReadStoreInfo(HYDROLOGY_ADD_INTERVAL,value,HYDROLOGY_ADD_INTERVAL_LEN);
      break;
    }
    case 0x22:
    {
      ret = Hydrology_ReadStoreInfo(HYDROLOGY_RAINFALL_BEGIN_TIME,value,HYDROLOGY_RAINFALL_BEGIN_TIME_LEN);
      break;
    }
    case 0x23:
    {
      ret = Hydrology_ReadStoreInfo(HYDROLOGY_SAMPLE_INTERVAL,value,HYDROLOGY_SAMPLE_INTERVAL_LEN);
      break;
    }
    case 0x24:
    {
      ret = Hydrology_ReadStoreInfo(HYDROLOGY_WATERLEVEL_STORE_INTERVAL,value,HYDROLOGY_WATERLEVEL_STORE_INTERVAL_LEN);
      break;
    }
    case 0x25:
    {
      ret = Hydrology_ReadStoreInfo(HYDROLOGY_RAINFALL_RESOLUTION,value,HYDROLOGY_RAINFALL_RESOLUTION_LEN);
      break;
    }
    case 0x26:
    {
      ret = Hydrology_ReadStoreInfo(HYDROLOGY_WATERLEVEL_RESOLUTION,value,HYDROLOGY_WATERLEVEL_RESOLUTION_LEN);
      break;
    }
    case 0x27:
    {
      ret = Hydrology_ReadStoreInfo(HYDROLOGY_RAINFALL_ADD_THRESHOLD,value,HYDROLOGY_RAINFALL_ADD_THRESHOLD_LEN);
      break;
    }
    case 0x28:
    {
      ret = Hydrology_ReadStoreInfo(HYDROLOGY_WATERLEVEL_BASICVALUE1,value,HYDROLOGY_WATERLEVEL_BASICVALUE1_LEN);
      break;
    }
    case 0x29:
    {
      ret = Hydrology_ReadStoreInfo(HYDROLOGY_WATERLEVEL_BASICVALUE2,value,HYDROLOGY_WATERLEVEL_BASICVALUE2_LEN);
      break;
    }
    case 0x2A:
    {
      ret = Hydrology_ReadStoreInfo(HYDROLOGY_WATERLEVEL_BASICVALUE3,value,HYDROLOGY_WATERLEVEL_BASICVALUE3_LEN);
      break;
    }
    case 0x2B:
    {
      ret = Hydrology_ReadStoreInfo(HYDROLOGY_WATERLEVEL_BASICVALUE4,value,HYDROLOGY_WATERLEVEL_BASICVALUE4_LEN);
      break;
    }
    case 0x2C:
    {
      ret = Hydrology_ReadStoreInfo(HYDROLOGY_WATERLEVEL_BASICVALUE5,value,HYDROLOGY_WATERLEVEL_BASICVALUE5_LEN);
      break;
    }
    case 0x2D:
    {
      ret = Hydrology_ReadStoreInfo(HYDROLOGY_WATERLEVEL_BASICVALUE6,value,HYDROLOGY_WATERLEVEL_BASICVALUE6_LEN);
      break;
    }
    case 0x2E:
    {
      ret = Hydrology_ReadStoreInfo(HYDROLOGY_WATERLEVEL_BASICVALUE7,value,HYDROLOGY_WATERLEVEL_BASICVALUE7_LEN);
      break;
    }
    case 0x2F:
    {
      ret = Hydrology_ReadStoreInfo(HYDROLOGY_WATERLEVEL_BASICVALUE8,value,HYDROLOGY_WATERLEVEL_BASICVALUE8_LEN);
      break;
    }
    case 0x30:
    {
      ret = Hydrology_ReadStoreInfo(HYDROLOGY_WATERLEVEL_CORRECTVALUE1,value,HYDROLOGY_WATERLEVEL_CORRECTVALUE1_LEN);
      break;
    }
    case 0x31:
    {
      ret = Hydrology_ReadStoreInfo(HYDROLOGY_WATERLEVEL_CORRECTVALUE2,value,HYDROLOGY_WATERLEVEL_CORRECTVALUE2_LEN);
      break;
    }
    case 0x32:
    {
      ret = Hydrology_ReadStoreInfo(HYDROLOGY_WATERLEVEL_CORRECTVALUE3,value,HYDROLOGY_WATERLEVEL_CORRECTVALUE3_LEN);
      break;
    }
    case 0x33:
    {
      ret = Hydrology_ReadStoreInfo(HYDROLOGY_WATERLEVEL_CORRECTVALUE4,value,HYDROLOGY_WATERLEVEL_CORRECTVALUE4_LEN);
      break;
    }
    case 0x34:
    {
      ret = Hydrology_ReadStoreInfo(HYDROLOGY_WATERLEVEL_CORRECTVALUE5,value,HYDROLOGY_WATERLEVEL_CORRECTVALUE5_LEN);
      break;
    }
    case 0x35:
    {
      ret = Hydrology_ReadStoreInfo(HYDROLOGY_WATERLEVEL_CORRECTVALUE6,value,HYDROLOGY_WATERLEVEL_CORRECTVALUE6_LEN);
      break;
    }
    case 0x36:
    {
      ret = Hydrology_ReadStoreInfo(HYDROLOGY_WATERLEVEL_CORRECTVALUE7,value,HYDROLOGY_WATERLEVEL_CORRECTVALUE7_LEN);
      break;
    }
    case 0x37:
    {
      ret = Hydrology_ReadStoreInfo(HYDROLOGY_WATERLEVEL_CORRECTVALUE8,value,HYDROLOGY_WATERLEVEL_CORRECTVALUE8_LEN);
      break;
    }
    case 0x38:
    {
      ret = Hydrology_ReadStoreInfo(HYDROLOGY_ADD_WATERLEVEL1,value,HYDROLOGY_ADD_WATERLEVEL1_LEN);
      break;
    }
    case 0x39:
    {
      ret = Hydrology_ReadStoreInfo(HYDROLOGY_ADD_WATERLEVEL2,value,HYDROLOGY_ADD_WATERLEVEL2_LEN);
      break;
    }
    case 0x3A:
    {
      ret = Hydrology_ReadStoreInfo(HYDROLOGY_ADD_WATERLEVEL3,value,HYDROLOGY_ADD_WATERLEVEL3_LEN);
      break;
    }
    case 0x3B:
    {
      ret = Hydrology_ReadStoreInfo(HYDROLOGY_ADD_WATERLEVEL4,value,HYDROLOGY_ADD_WATERLEVEL4_LEN);
      break;
    }
    case 0x3C:
    {
      ret = Hydrology_ReadStoreInfo(HYDROLOGY_ADD_WATERLEVEL5,value,HYDROLOGY_ADD_WATERLEVEL5_LEN);
      break;
    }
    case 0x3D:
    {
      ret = Hydrology_ReadStoreInfo(HYDROLOGY_ADD_WATERLEVEL6,value,HYDROLOGY_ADD_WATERLEVEL6_LEN);
      break;
    }
    case 0x3E:
    {
      ret = Hydrology_ReadStoreInfo(HYDROLOGY_ADD_WATERLEVEL7,value,HYDROLOGY_ADD_WATERLEVEL7_LEN);
      break;
    }
    case 0x3F:
    {
      ret = Hydrology_ReadStoreInfo(HYDROLOGY_ADD_WATERLEVEL8,value,HYDROLOGY_ADD_WATERLEVEL8_LEN);
      break;
    }
    case 0x40:
    {
      ret = Hydrology_ReadStoreInfo(HYDROLOGY_ADD_ABOVE_THRESHOLD,value,HYDROLOGY_ADD_ABOVE_THRESHOLD_LEN);
      break;
    }
    case 0x41:
    {
      ret = Hydrology_ReadStoreInfo(HYDROLOGY_ADD_BELOW_THRESHOLD,value,HYDROLOGY_ADD_BELOW_THRESHOLD_LEN);
      break;
    }
    case 0xA9:
    {
      //ret = Hydrology_ReadStoreInfo(HYDROLOGY_RS485_COUNT1,value,HYDROLOGY_RS485_COUNT_LEN);
       Read_Flash_Segment(HYDROLOGY_RS485_COUNT1_BP,value,HYDROLOGY_RS485_COUNT_LEN);
      
      break;
    }
    case 0xAA:
    {
      //ret = Hydrology_ReadStoreInfo(HYDROLOGY_RS485_COUNT2,value,HYDROLOGY_RS485_COUNT_LEN);
      Read_Flash_Segment(HYDROLOGY_RS485_COUNT2_BP,value,HYDROLOGY_RS485_COUNT_LEN);
      break;
    }
    case 0xAB:
    {
//      ret = Hydrology_ReadStoreInfo(HYDROLOGY_RS485_COUNT3,value,HYDROLOGY_RS485_COUNT_LEN);
      Read_Flash_Segment(HYDROLOGY_RS485_COUNT3_BP,value,HYDROLOGY_RS485_COUNT_LEN);
      break;
    }
    case 0xAC:
    {
      //ret = Hydrology_ReadStoreInfo(HYDROLOGY_RS485_COUNT4,value,HYDROLOGY_RS485_COUNT_LEN);
      Read_Flash_Segment(HYDROLOGY_RS485_COUNT4_BP,value,HYDROLOGY_RS485_COUNT_LEN);
      break;
    }
    case 0xAD:
    {
      //ret = Hydrology_ReadStoreInfo(HYDROLOGY_RS485_COUNT5,value,HYDROLOGY_RS485_COUNT_LEN);
      Read_Flash_Segment(HYDROLOGY_RS485_COUNT5_BP,value,HYDROLOGY_RS485_COUNT_LEN);
      break;
    }
    case 0xAE:
    {
      //ret = Hydrology_ReadStoreInfo(HYDROLOGY_RS485_COUNT6,value,HYDROLOGY_RS485_COUNT_LEN);
      Read_Flash_Segment(HYDROLOGY_RS485_COUNT6_BP,value,HYDROLOGY_RS485_COUNT_LEN);
      break;
    }
    case 0xAF:
    {
      //ret = Hydrology_ReadStoreInfo(HYDROLOGY_RS485_COUNT7,value,HYDROLOGY_RS485_COUNT_LEN);
      Read_Flash_Segment(HYDROLOGY_RS485_COUNT7_BP,value,HYDROLOGY_RS485_COUNT_LEN);
      break;
    }
    case 0xB0:
    {
      //ret = Hydrology_ReadStoreInfo(HYDROLOGY_RS485_COUNT8,value,HYDROLOGY_RS485_COUNT_LEN);
      Read_Flash_Segment(HYDROLOGY_RS485_COUNT8_BP,value,HYDROLOGY_RS485_COUNT_LEN);
      break;
    }
    case 0xB1:
    {
     // ret = Hydrology_ReadStoreInfo(HYDROLOGY_RS485_COUNT9,value,HYDROLOGY_RS485_COUNT_LEN);
      Read_Flash_Segment(HYDROLOGY_RS485_COUNT9_BP,value,HYDROLOGY_RS485_COUNT_LEN);
      break;
    }
    case 0xB2:
    {
      //ret = Hydrology_ReadStoreInfo(HYDROLOGY_RS485_COUNT10,value,HYDROLOGY_RS485_COUNT_LEN);
      Read_Flash_Segment(HYDROLOGY_RS485_COUNT10_BP,value,HYDROLOGY_RS485_COUNT_LEN);
      break;
    }
    case 0xB3:
    {
      //ret = Hydrology_ReadStoreInfo(HYDROLOGY_RS4851,value,HYDROLOGY_RS485_LEN);
      Read_Flash_Segment(HYDROLOGY_RS4851_BP,value,HYDROLOGY_RS485_LEN);
      break;
    }
    case 0xB4:
    {
//      ret = Hydrology_ReadStoreInfo(HYDROLOGY_RS4852,value,HYDROLOGY_RS485_LEN);
       Read_Flash_Segment(HYDROLOGY_RS4852_BP,value,HYDROLOGY_RS485_LEN);
      break;
    }
    case 0xB5:
    {
//      ret = Hydrology_ReadStoreInfo(HYDROLOGY_RS4853,value,HYDROLOGY_RS485_LEN);
       Read_Flash_Segment(HYDROLOGY_RS4853_BP,value,HYDROLOGY_RS485_LEN);
      break;
    }
    case 0xB6:
    {
//      ret = Hydrology_ReadStoreInfo(HYDROLOGY_RS4854,value,HYDROLOGY_RS485_LEN);
       Read_Flash_Segment(HYDROLOGY_RS4854_BP,value,HYDROLOGY_RS485_LEN);
      break;
    }
    case 0xB7:
    {
//      ret = Hydrology_ReadStoreInfo(HYDROLOGY_RS4855,value,HYDROLOGY_RS485_LEN);
       Read_Flash_Segment(HYDROLOGY_RS4855_BP,value,HYDROLOGY_RS485_LEN);
      break;
    }
    case 0xB8:
    {
//      ret = Hydrology_ReadStoreInfo(HYDROLOGY_RS4856,value,HYDROLOGY_RS485_LEN);
       Read_Flash_Segment(HYDROLOGY_RS4856_BP,value,HYDROLOGY_RS485_LEN);
      break;
    }
    case 0xB9:
    {
//      ret = Hydrology_ReadStoreInfo(HYDROLOGY_RS4857,value,HYDROLOGY_RS485_LEN);
       Read_Flash_Segment(HYDROLOGY_RS4857_BP,value,HYDROLOGY_RS485_LEN);
      break;
    }
    case 0xBA:
    {
//      ret = Hydrology_ReadStoreInfo(HYDROLOGY_RS4858,value,HYDROLOGY_RS485_LEN);
       Read_Flash_Segment(HYDROLOGY_RS4858_BP,value,HYDROLOGY_RS485_LEN);
      break;
    }
    case 0xBB:
    {
//      ret = Hydrology_ReadStoreInfo(HYDROLOGY_RS4859,value,HYDROLOGY_RS485_LEN);
       Read_Flash_Segment(HYDROLOGY_RS4859_BP,value,HYDROLOGY_RS485_LEN);
      break;
    }
    case 0xBC:
    {
//      ret = Hydrology_ReadStoreInfo(HYDROLOGY_RS48510,value,HYDROLOGY_RS485_LEN);
       Read_Flash_Segment(HYDROLOGY_RS48510_BP,value,HYDROLOGY_RS485_LEN);
      break;
    }
    case 0xBD:
    {
//      ret = Hydrology_ReadStoreInfo(HYDROLOGY_RS48511,value,HYDROLOGY_RS485_LEN);
       Read_Flash_Segment(HYDROLOGY_RS48511_BP,value,HYDROLOGY_RS485_LEN);
      break;
    }
    case 0xBE:
    {
//      ret = Hydrology_ReadStoreInfo(HYDROLOGY_RS48512,value,HYDROLOGY_RS485_LEN);
       Read_Flash_Segment(HYDROLOGY_RS48512_BP,value,HYDROLOGY_RS485_LEN);
      break;
    }
    case 0xBF:
    {
//      ret = Hydrology_ReadStoreInfo(HYDROLOGY_RS48513,value,HYDROLOGY_RS485_LEN);
       Read_Flash_Segment(HYDROLOGY_RS48513_BP,value,HYDROLOGY_RS485_LEN);
      break;
    }
    case 0xC0:
    {
//      ret = Hydrology_ReadStoreInfo(HYDROLOGY_RS48514,value,HYDROLOGY_RS485_LEN);
       Read_Flash_Segment(HYDROLOGY_RS48514_BP,value,HYDROLOGY_RS485_LEN);
      break;
    }
    case 0xC1:
    {
//      ret = Hydrology_ReadStoreInfo(HYDROLOGY_RS48515,value,HYDROLOGY_RS485_LEN);
       Read_Flash_Segment(HYDROLOGY_RS48515_BP,value,HYDROLOGY_RS485_LEN);
      break;
    }
    case 0xC2:
    {
//      ret = Hydrology_ReadStoreInfo(HYDROLOGY_RS48516,value,HYDROLOGY_RS485_LEN);
       Read_Flash_Segment(HYDROLOGY_RS48516_BP,value,HYDROLOGY_RS485_LEN);
      break;
    }
    case 0xC3:
    {
//      ret = Hydrology_ReadStoreInfo(HYDROLOGY_RS48517,value,HYDROLOGY_RS485_LEN);
       Read_Flash_Segment(HYDROLOGY_RS48517_BP,value,HYDROLOGY_RS485_LEN);
      break;
    }
    case 0xC4:
    {
//      ret = Hydrology_ReadStoreInfo(HYDROLOGY_RS48518,value,HYDROLOGY_RS485_LEN);
       Read_Flash_Segment(HYDROLOGY_RS48518_BP,value,HYDROLOGY_RS485_LEN);
      break;
    }
    case 0xC5:
    {
//      ret = Hydrology_ReadStoreInfo(HYDROLOGY_RS48519,value,HYDROLOGY_RS485_LEN);
       Read_Flash_Segment(HYDROLOGY_RS48519_BP,value,HYDROLOGY_RS485_LEN);
      break;
    }
    case 0xC6:
    {
//      ret = Hydrology_ReadStoreInfo(HYDROLOGY_RS48520,value,HYDROLOGY_RS485_LEN);
       Read_Flash_Segment(HYDROLOGY_RS48520_BP,value,HYDROLOGY_RS485_LEN);
      break;
    }
    default:
    {
      ret = -1;
      break;
    }
  }
  return ret;
}

void getElementDd(char ID,char *D,char *d)         //++++++++++++++++编码要素配置表+++++++++++++++++++++++
{       
  char x = ID ;
  char *y = D ;
  char *z = d ;
 if(x==0x0B){
    *y = 1; *z = 0;
  }else if(x==0x19 ||x==0x33 ||x==0x34)
  {
    *y = 2; *z = 0;
  }else if(x==0x04 ||x==0x0A ||x==0x0C ||x==0x49)
  {
    *y = 3; *z = 0;
  }else if(x==0x02 ||x==0x03 ||x==0x0D )
  {
    *y = 3; *z = 1;
  }else if(x==0x45)  //4字节hex
  {
    *y = 8; *z = 0;
  }else if((x>=0x10 && x<=0x18)||x==0x35 ||x==0x47 ||x==0x4A ||(x>=0x70 && x<=0x75))
  {
    *y = 4;*z = 1;
  }else if(x==0x38 ||x==0x46||x==0x4F ||x==0x57)
  {
    *y = 4; *z = 2;
  }else if(x==0x08 || x==0x48  )
  {
    *y = 5; *z = 0;
  }else if(x==0x06 || x==0x07|| (x>=0x1A && x<=0x25)|| x==0x4B )
  {
    *y = 5; *z = 1;
  }else if(x==0x09 || x==0x0F|| x==0x4E || (x>=0x58 && x<=0x5F)  )
  {
    *y = 5; *z = 2;
  }else if(x==0x36||x==0x37||x==0x4D )
  {
    *y = 5; *z = 3;
  }else if(x==0x26  )
  {
    *y = 6; *z = 1;
  }else if(x==0x4C ||x==0x0E)
  {
    *y = 6; *z = 2;
  }else if(x==0x51)
  {
    *y = 6; *z = 4;
  }else if((x>=0x39 && x<=0x43) )
  {
    *y = 7; *z = 3;
  } else if(x==0x50  )
  {
    *y = 7; *z = 4;
  }else if((x>=0x52 && x<=0x56))
  {
    *y = 7; *z = 5;
  }else if(x==0x01)
  {
    *y = 8; *z = 2;
  }else if((x>=0x27 && x<=0x31)||x==0x44)
  {
    *y = 9; *z = 3;
  }else if((x>=0x68 && x<=0x6F))
  {
    *y = 10; *z = 2;
  } else if((x>=0x60 && x<=0x67)||x==0x32)
  {
    *y = 11; *z = 3;
  }else
  {
    *y = 0; *z = 0;
  }
   
  
}
int hydrologySetPara(void)
{
  int i,j = 0;
  int adc=0;
  int ret;
  char user = 0,rs485 = 0;
  char temp[8] = {0,0,0,0,0,0,0,0};
  char end = 0;
  char isr_count_flag =0;  
  char pictureinterval=0;
  char switch_parse_type = 0;
  int count = 0;
  hydrologyDownBody* pbody = (hydrologyDownBody*)(g_HydrologyMsg.downbody);
  
  if(Extend_Flag == 1)                                                            //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  {
     
    UserElementCount = pbody->count;
    count = UserElementCount;
    /*
    for(i=0;i<count;i++)
    {
        if((pbody->element)[i].guide[0]==0xF3)  //图片报
     {
        //发送图片的flag置1，存flag和发送图片的间隔
       pictureinterval = (pbody->element)[i].value[0];
       Hydrology_WriteStoreInfo(PICTURE_INTERVAL,&pictureinterval,PICTURE_INTERVAL_LEN);
      UserElementCount--;
     continue;
     }
    }*/
    user = (char)UserElementCount;
    Hydrology_WriteStoreInfo(HYDROLOGY_USER_ElEMENT_COUNT ,&user,HYDROLOGY_USER_ElEMENT_COUNT_LEN);
    Write_Flash_Segment(HYDROLOGY_USER_ElEMENT_COUNT_BP,&user,HYDROLOGY_USER_ElEMENT_COUNT_LEN);
    
    TraceInt4(UserElementCount,1);
    for(i=0; i< UserElementCount; i++)     
    {

     

     Hydrology_WriteStoreInfo(HYDROLOGY_ELEMENT1_ID+i*HYDROLOGY_ELEMENT_ID_LEN,&(pbody->element)[i].guide[0],HYDROLOGY_ELEMENT_ID_LEN);
     Hydrology_WriteStoreInfo(HYDROLOGY_ELEMENT1_TYPE+i*HYDROLOGY_ELEMENT_TYPE_LEN,&(pbody->element)[i].value[0],HYDROLOGY_ELEMENT_TYPE_LEN);
     Hydrology_WriteStoreInfo(HYDROLOGY_ELEMENT1_MODE+i*HYDROLOGY_ELEMENT_MODE_LEN,&(pbody->element)[i].value[1],HYDROLOGY_ELEMENT_MODE_LEN);
     Hydrology_WriteStoreInfo(HYDROLOGY_ELEMENT1_CHANNEL+i*HYDROLOGY_ELEMENT_CHANNEL_LEN,&(pbody->element)[i].value[2],HYDROLOGY_ELEMENT_CHANNEL_LEN);
     
     Write_Flash_Segment(HYDROLOGY_ELEMENT1_ID_BP+i*HYDROLOGY_ELEMENT_ID_LEN,&(pbody->element)[i].guide[0],HYDROLOGY_ELEMENT_ID_LEN);
     Write_Flash_Segment(HYDROLOGY_ELEMENT1_TYPE_BP+i*HYDROLOGY_ELEMENT_TYPE_LEN,&(pbody->element)[i].value[0],HYDROLOGY_ELEMENT_TYPE_LEN);
     Write_Flash_Segment(HYDROLOGY_ELEMENT1_MODE_BP+i*HYDROLOGY_ELEMENT_MODE_LEN,&(pbody->element)[i].value[1],HYDROLOGY_ELEMENT_MODE_LEN);
     Write_Flash_Segment(HYDROLOGY_ELEMENT1_CHANNEL_BP+i*HYDROLOGY_ELEMENT_CHANNEL_LEN,&(pbody->element)[i].value[2],HYDROLOGY_ELEMENT_CHANNEL_LEN);
      /*根据ID获取D,d*/     
     getElementDd(Element_table[i].ID,&Element_table[i].D,&Element_table[i].d);   //未备份，是否需要提前到外部去执行
     
      if((pbody->element)[i].value[1] == RS485)
      {  
         
         temp[0] =  (pbody->element)[i].value[3];     //寄存器个数
         Hydrology_WriteStoreInfo(HYDROLOGY_RS485_COUNT1 +j*HYDROLOGY_RS485_COUNT_LEN,temp,HYDROLOGY_RS485_COUNT_LEN);  
         Write_Flash_Segment(HYDROLOGY_RS485_COUNT1_BP +j*HYDROLOGY_RS485_COUNT_LEN,temp,HYDROLOGY_RS485_COUNT_LEN);
         memcpy(temp,&(pbody->element)[i].value[5],(pbody->element)[i].value[4]);            //(pbody->element)[i].value[4] =HYDROLOGY_RS485_LEN   485命令
         Hydrology_WriteStoreInfo(HYDROLOGY_RS4851+j*HYDROLOGY_RS485_LEN,temp,HYDROLOGY_RS485_LEN);
         Write_Flash_Segment(HYDROLOGY_RS4851_BP+j*HYDROLOGY_RS485_LEN,temp,HYDROLOGY_RS485_LEN);  
         
         //+++++++++++++++++++++++++++++扩展部分++++++++++++++++++++++++++++++++++++++++++++
         int offt = 10;
         temp[0] =  (pbody->element)[i].value[offt]; 
         Hydrology_WriteStoreInfo(HYDROLOGY_RS485_CRC_TYPE1+j*HYDROLOGY_RS485_CRC_TYPE_LEN,temp,HYDROLOGY_RS485_CRC_TYPE_LEN);
         Write_Flash_Segment(HYDROLOGY_RS485_CRC_TYPE1_BP+j*HYDROLOGY_RS485_CRC_TYPE_LEN,temp,HYDROLOGY_RS485_CRC_TYPE_LEN);
         offt+=1;
         memcpy(temp,&(pbody->element)[i].value[offt],2);
         Hydrology_WriteStoreInfo(HYDROLOGY_RS485_CRC_ORDER1+j*HYDROLOGY_RS485_CRC_ORDER_LEN,temp,HYDROLOGY_RS485_CRC_ORDER_LEN);
         Write_Flash_Segment(HYDROLOGY_RS485_CRC_ORDER1_BP+j*HYDROLOGY_RS485_CRC_ORDER_LEN,temp,HYDROLOGY_RS485_CRC_ORDER_LEN);
         offt+=2;
          memcpy(temp,&(pbody->element)[i].value[offt],2);
         Hydrology_WriteStoreInfo(HYDROLOGY_RS485_RECV_DELAY1+j*HYDROLOGY_RS485_RECV_DELAY_LEN,temp,HYDROLOGY_RS485_RECV_DELAY_LEN);
         Write_Flash_Segment(HYDROLOGY_RS485_RECV_DELAY1_BP+j*HYDROLOGY_RS485_RECV_DELAY_LEN,temp,HYDROLOGY_RS485_RECV_DELAY_LEN);
         offt+=2;
          temp[0] =  (pbody->element)[i].value[offt]; 
         Hydrology_WriteStoreInfo(HYDROLOGY_RS485_RECV_DATA_ORDER1+j*HYDROLOGY_RS485_RECV_DATA_ORDER_LEN,temp,HYDROLOGY_RS485_RECV_DATA_ORDER_LEN);
         Write_Flash_Segment(HYDROLOGY_RS485_RECV_DATA_ORDER1_BP+j*HYDROLOGY_RS485_RECV_DATA_ORDER_LEN,temp,HYDROLOGY_RS485_RECV_DATA_ORDER_LEN);
         offt+=1;
          temp[0] =  (pbody->element)[i].value[offt]; 
         Hydrology_WriteStoreInfo(HYDROLOGY_RS485_RECV_CRC_ORDER1+j*HYDROLOGY_RS485_RECV_CRC_ORDER_LEN,temp,HYDROLOGY_RS485_RECV_CRC_ORDER_LEN);
         Write_Flash_Segment(HYDROLOGY_RS485_RECV_CRC_ORDER1_BP+j*HYDROLOGY_RS485_RECV_CRC_ORDER_LEN,temp,HYDROLOGY_RS485_RECV_CRC_ORDER_LEN);
         offt+=1;
         //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
         j++; //485要素的数量
      }
      if((pbody->element)[i].value[1] == ISR_COUNT)//脉冲量，判断是单路计数的还是联合计数的，雨量是联合计数的flag=1
        {
            if((pbody->element)[i].guide[0]==0x20||(pbody->element)[i].guide[0]==0x26)
            {
                isr_count_flag = 1;
               Hydrology_WriteStoreInfo(HYDROLOGY_ISR_COUNT_FLAG,&isr_count_flag,HYDROLOGY_ISR_COUNT_FLAG_LEN);  //初始化这块内存是0
               Write_Flash_Segment(HYDROLOGY_ISR_COUNT_FLAG_BP,&isr_count_flag,HYDROLOGY_ISR_COUNT_FLAG_LEN);
            }
            else
            {
               Hydrology_WriteStoreInfo(HYDROLOGY_ISR_COUNT_FLAG,&isr_count_flag,HYDROLOGY_ISR_COUNT_FLAG_LEN);
               Write_Flash_Segment(HYDROLOGY_ISR_COUNT_FLAG_BP,&isr_count_flag,HYDROLOGY_ISR_COUNT_FLAG_LEN);
            }
        }
      /* -------------------switch parse type------------------ */
	if ((pbody->element)[ i ].value[ 1 ] == IO_STATUS) 
        {
	  switch_parse_type = (pbody->element)[ i ].value[ 3 ];
	Hydrology_WriteStoreInfo(HYDROLOGY_SWITCH_PARSE_TYPE,&switch_parse_type,HYDROLOGY_SWITCH_PARSE_TYPE_LEN);
        Write_Flash_Segment(HYDROLOGY_SWITCH_PARSE_TYPE_BP,&switch_parse_type,HYDROLOGY_SWITCH_PARSE_TYPE_LEN);
	//printf("HYDROLOGY_SWITCH_PARSE_TYPE : %x \n\n", switch_parse_type);
	}
       if((pbody->element)[i].value[1] == ADC){
                int offset = 3;

                memcpy(temp,&(pbody->element)[i].value[offset],HYDROLOGY_ADC_RANGE_LEN);
                Hydrology_WriteStoreInfo(HYDROLOGY_ADC_RANGE1+adc*HYDROLOGY_ADC_RANGE_LEN,temp,HYDROLOGY_ADC_RANGE_LEN);
                Write_Flash_Segment(HYDROLOGY_ADC_RANGE1_BP+adc*HYDROLOGY_ADC_RANGE_LEN,temp,HYDROLOGY_ADC_RANGE_LEN);
                offset+=HYDROLOGY_ADC_RANGE_LEN;

                memcpy(temp,&(pbody->element)[i].value[offset],HYDROLOGY_ADC_COEFFICIENT_LEN);
                Hydrology_WriteStoreInfo(HYDROLOGY_ADC_COEFFICIENT1+adc*HYDROLOGY_ADC_COEFFICIENT_LEN,temp,HYDROLOGY_ADC_COEFFICIENT_LEN);
                Write_Flash_Segment(HYDROLOGY_ADC_COEFFICIENT1_BP+adc*HYDROLOGY_ADC_COEFFICIENT_LEN,temp,HYDROLOGY_ADC_COEFFICIENT_LEN);
                offset+=HYDROLOGY_ADC_COEFFICIENT_LEN;

                memcpy(temp,&(pbody->element)[i].value[offset],HYDROLOGY_ADC_OFFSET_LEN);
                Hydrology_WriteStoreInfo(HYDROLOGY_ADC_OFFSET1+adc*HYDROLOGY_ADC_OFFSET_LEN,temp,HYDROLOGY_ADC_OFFSET_LEN);
                Write_Flash_Segment(HYDROLOGY_ADC_OFFSET1_BP+adc*HYDROLOGY_ADC_OFFSET_LEN,temp,HYDROLOGY_ADC_OFFSET_LEN);
                offset+=HYDROLOGY_ADC_OFFSET_LEN;

                memcpy(temp,&(pbody->element)[i].value[offset],HYDROLOGY_ADC_BASE_LEN);
                Hydrology_WriteStoreInfo(HYDROLOGY_ADC_BASE1+adc*HYDROLOGY_ADC_BASE_LEN,temp,HYDROLOGY_ADC_BASE_LEN);
                Write_Flash_Segment(HYDROLOGY_ADC_BASE1_BP+adc*HYDROLOGY_ADC_BASE_LEN,temp,HYDROLOGY_ADC_BASE_LEN);
                offset+=HYDROLOGY_ADC_BASE_LEN;

                memcpy(temp,&(pbody->element)[i].value[offset],HYDROLOGY_ADC_UPPER_LEN);
                Hydrology_WriteStoreInfo(HYDROLOGY_ADC_UPPER1+adc*HYDROLOGY_ADC_UPPER_LEN,temp,HYDROLOGY_ADC_UPPER_LEN);
                Write_Flash_Segment(HYDROLOGY_ADC_UPPER1_BP+adc*HYDROLOGY_ADC_UPPER_LEN,temp,HYDROLOGY_ADC_UPPER_LEN);
                offset+=HYDROLOGY_ADC_UPPER_LEN;

                memcpy(temp,&(pbody->element)[i].value[offset],HYDROLOGY_ADC_LOWER_LEN);
                Hydrology_WriteStoreInfo(HYDROLOGY_ADC_LOWER1+adc*HYDROLOGY_ADC_LOWER_LEN,temp,HYDROLOGY_ADC_LOWER_LEN);
                Write_Flash_Segment(HYDROLOGY_ADC_LOWER1_BP+adc*HYDROLOGY_ADC_LOWER_LEN,temp,HYDROLOGY_ADC_LOWER_LEN);
                offset+=HYDROLOGY_ADC_LOWER_LEN;

                memcpy(temp,&(pbody->element)[i].value[offset],HYDROLOGY_ADC_WARNNING_LEN);
                Hydrology_WriteStoreInfo(HYDROLOGY_ADC_WARNNING1_1+adc*HYDROLOGY_ADC_WARNNING_LEN,temp,HYDROLOGY_ADC_WARNNING_LEN);
                Write_Flash_Segment(HYDROLOGY_ADC_WARNNING1_1_BP+adc*HYDROLOGY_ADC_WARNNING_LEN,temp,HYDROLOGY_ADC_WARNNING_LEN);
                offset+=HYDROLOGY_ADC_WARNNING_LEN;

                memcpy(temp,&(pbody->element)[i].value[offset],HYDROLOGY_ADC_WARNNING_LEN);
                Hydrology_WriteStoreInfo(HYDROLOGY_ADC_WARNNING2_1+adc*HYDROLOGY_ADC_WARNNING_LEN,temp,HYDROLOGY_ADC_WARNNING_LEN);
                Write_Flash_Segment(HYDROLOGY_ADC_WARNNING2_1_BP+adc*HYDROLOGY_ADC_WARNNING_LEN,temp,HYDROLOGY_ADC_WARNNING_LEN);
                offset+=HYDROLOGY_ADC_WARNNING_LEN;

                adc++;
        }
   
    }  
    
      RS485RegisterCount = j;
      rs485 = (char)RS485RegisterCount;
      Hydrology_WriteStoreInfo(HYDROLOGY_RS485_REGISTER_COUNT ,&rs485,HYDROLOGY_RS485_REGISTER_COUNT_LEN);
      Write_Flash_Segment(HYDROLOGY_RS485_REGISTER_COUNT_BP ,&rs485,HYDROLOGY_RS485_REGISTER_COUNT_LEN);
      TraceInt4(RS485RegisterCount,1);
   
     Hydrology_WriteStoreInfo(HYDROLOGY_ELEMENT1_ID+i*HYDROLOGY_ELEMENT_ID_LEN,&end,HYDROLOGY_ELEMENT_ID_LEN);
     Hydrology_WriteStoreInfo(HYDROLOGY_ELEMENT1_TYPE+i*HYDROLOGY_ELEMENT_TYPE_LEN,&end,HYDROLOGY_ELEMENT_TYPE_LEN);
     Hydrology_WriteStoreInfo(HYDROLOGY_ELEMENT1_MODE+i*HYDROLOGY_ELEMENT_MODE_LEN,&end,HYDROLOGY_ELEMENT_MODE_LEN);
     Hydrology_WriteStoreInfo(HYDROLOGY_ELEMENT1_CHANNEL+i*HYDROLOGY_ELEMENT_CHANNEL_LEN,&end,HYDROLOGY_ELEMENT_CHANNEL_LEN);
     
     Write_Flash_Segment(HYDROLOGY_ELEMENT1_ID_BP+i*HYDROLOGY_ELEMENT_ID_LEN,&end,HYDROLOGY_ELEMENT_ID_LEN);
     Write_Flash_Segment(HYDROLOGY_ELEMENT1_TYPE_BP+i*HYDROLOGY_ELEMENT_TYPE_LEN,&end,HYDROLOGY_ELEMENT_TYPE_LEN);
     Write_Flash_Segment(HYDROLOGY_ELEMENT1_MODE_BP+i*HYDROLOGY_ELEMENT_MODE_LEN,&end,HYDROLOGY_ELEMENT_MODE_LEN);
     Write_Flash_Segment(HYDROLOGY_ELEMENT1_CHANNEL_BP+i*HYDROLOGY_ELEMENT_CHANNEL_LEN,&end,HYDROLOGY_ELEMENT_CHANNEL_LEN);
  }                                                                                    
  else{
    
            for(i=0; i< pbody->count; i++)
          {
            ret = hydrologySetParaSelect((pbody->element)[i].guide, (pbody->element)[i].value);
            if(ret == -1)
             return i+1;
            }
        }   
  return 0;
}

int hydrologyReadPara(void)
{
  int i = 0;
  int ret;

  hydrologyDownBody* pbody = (hydrologyDownBody*)(g_HydrologyMsg.downbody);

  for(i=0; i< pbody->count; i++)
  {
    ret = hydrologyReadParaSelect((pbody->element)[i].guide, (pbody->element)[i].value);
    if(ret == -1)
      return i+1;
  }
  return 0;
}

int hydrologyInitializeSolidStorage(void)
{
    char _src[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    int writelen = HYDROLOGY_DATAMAX_ADDR - HYDROLOGY_CONFIG_ADDR;  //HYDROLOGY_END
    int writeaddr = HYDROLOGY_CONFIG_ADDR;
    
    if(writelen >= 0x07FF)
      writelen = 0x07FF;
    
    while (writelen > 16)
    {
      Hydrology_WriteStoreInfo(writeaddr, _src, 16);

      writeaddr = writeaddr + 16;
      writelen = writelen - 16;
    }

    Hydrology_WriteStoreInfo(writeaddr, _src, writelen);
      
    return 0;
}

int Hydrology_Reset(void)
{
  char temp[14] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0};
  int i = 0;
  temp[0] = 0x50;
  Hydrology_WriteStoreInfo(HYDROLOGY_RTUTYPE,temp,HYDROLOGY_RTUTYPE_LEN);
  temp[0] = 0x01;temp[1] = 0x02;temp[2] = 0x03;temp[3] = 0x04;
  Hydrology_WriteStoreInfo(HYDROLOGY_CENTER_ADDR,temp,HYDROLOGY_CENTER_LEN);
  temp[0] = 0x00;temp[1] = 0x12;temp[2] = 0x34;temp[3] = 0x56;temp[4] = 0x78;
  Hydrology_WriteStoreInfo(HYDROLOGY_REMOTE_ADDR,temp,HYDROLOGY_REMOTE_LEN);
  temp[0] = 0x12;temp[1] = 0x34;
  Hydrology_WriteStoreInfo(HYDROLOGY_PASSWORD_ADDR,temp,HYDROLOGY_PASSWORD_LEN);
  temp[0] = 0x02;temp[1] = 0x05;temp[2] = 0x80;temp[3] = 0x49;temp[4] = 0x14;
  temp[5] = 0x02;temp[6] = 0x02;temp[7] = 0x00;temp[8] = 0x89;temp[9] = 0x86;
  Hydrology_WriteStoreInfo(HYDROLOGY_CENTER1_IP,temp,HYDROLOGY_CENTER_IP_LEN);
  temp[0] = 0x02;temp[1] = 0x18;temp[2] = 0x30;temp[3] = 0x92;temp[4] = 0x03;
  temp[5] = 0x30;temp[6] = 0x30;temp[7] = 0x00;temp[8] = 0x66;temp[9] = 0x66;
  Hydrology_WriteStoreInfo(HYDROLOGY_BACKUP1_IP,temp,HYDROLOGY_BACKUP_IP_LEN);
  temp[0] = 0x02;temp[1] = 0x22;temp[2] = 0x21;temp[3] = 0x60;temp[4] = 0x24;
  temp[5] = 0x52;temp[6] = 0x06;temp[7] = 0x00;temp[8] = 0x66;temp[9] = 0x66;
  Hydrology_WriteStoreInfo(HYDROLOGY_CENTER2_IP,temp,HYDROLOGY_CENTER_IP_LEN);
  temp[0] = 0x02;temp[1] = 0x12;temp[2] = 0x00;temp[3] = 0x78;temp[4] = 0x13;
  temp[5] = 0x91;temp[6] = 0x49;temp[7] = 0x00;temp[8] = 0x99;temp[9] = 0x99;
  Hydrology_WriteStoreInfo(HYDROLOGY_BACKUP2_IP,temp,HYDROLOGY_BACKUP_IP_LEN);
  temp[0] = 0x02;temp[1] = 0x12;temp[2] = 0x00;temp[3] = 0x78;temp[4] = 0x13;
  temp[5] = 0x91;temp[6] = 0x49;temp[7] = 0x00;temp[8] = 0x99;temp[9] = 0x99;
  Hydrology_WriteStoreInfo(HYDROLOGY_CENTER3_IP,temp,HYDROLOGY_CENTER_IP_LEN);
  temp[0] = 0x02;temp[1] = 0x12;temp[2] = 0x00;temp[3] = 0x78;temp[4] = 0x13;
  temp[5] = 0x91;temp[6] = 0x49;temp[7] = 0x00;temp[8] = 0x99;temp[9] = 0x99;
  Hydrology_WriteStoreInfo(HYDROLOGY_BACKUP3_IP,temp,HYDROLOGY_BACKUP_IP_LEN);
  temp[0] = 0x02;temp[1] = 0x12;temp[2] = 0x00;temp[3] = 0x78;temp[4] = 0x13;
  temp[5] = 0x91;temp[6] = 0x49;temp[7] = 0x00;temp[8] = 0x99;temp[9] = 0x99;
  Hydrology_WriteStoreInfo(HYDROLOGY_CENTER4_IP,temp,HYDROLOGY_CENTER_IP_LEN);
  temp[0] = 0x02;temp[1] = 0x12;temp[2] = 0x00;temp[3] = 0x78;temp[4] = 0x13;
  temp[5] = 0x91;temp[6] = 0x49;temp[7] = 0x00;temp[8] = 0x99;temp[9] = 0x99;
  Hydrology_WriteStoreInfo(HYDROLOGY_BACKUP4_IP,temp,HYDROLOGY_BACKUP_IP_LEN);
  temp[0] = 0x02;
  Hydrology_WriteStoreInfo(HYDROLOGY_WORK_MODE,temp,HYDROLOGY_WORK_MODE_LEN);
  temp[0] = 0x80;temp[1] = 0x01;temp[2] = 0x06;temp[3] = 0x01;temp[4] = 0x00;
  temp[5] = 0x00;temp[6] = 0x00;temp[7] = 0x00;
  Hydrology_WriteStoreInfo(HYDROLOGY_ELEMENT_SELECT,temp,HYDROLOGY_ELEMENT_SELECT_LEN);
  temp[0] = 0x00;temp[1] = 0x00;temp[2] = 0x00;temp[3] = 0x00;temp[4] = 0x00;temp[5] = 0x00;
  temp[6] = 0x00;temp[7] = 0x00;temp[8] = 0x00;temp[9] = 0x00;temp[10] = 0x00;temp[11] = 0x00;
  Hydrology_WriteStoreInfo(HYDROLOGY_REPEATER_STATION,temp,HYDROLOGY_REPEATER_STATION_LEN);
  temp[0] = '1';temp[1] = '1';temp[2] = '2';temp[3] = '3';temp[4] = '4';temp[5] = '5';
  temp[6] = '6';temp[7] = '7';temp[8] = '8';temp[9] = '9';temp[10] = '0';temp[11] = '1';
  Hydrology_WriteStoreInfo(HYDROLOGY_DEVICE_ID,temp,HYDROLOGY_DEVICE_ID_LEN);
  temp[0] = 0x01;
  Hydrology_WriteStoreInfo(HYDROLOGY_TIMER_INTERVAL,temp,HYDROLOGY_TIMER_INTERVAL_LEN);
  temp[0] = 0x05;
  Hydrology_WriteStoreInfo(HYDROLOGY_ADD_INTERVAL,temp,HYDROLOGY_ADD_INTERVAL_LEN);
  temp[0] = 0x08;
  Hydrology_WriteStoreInfo(HYDROLOGY_RAINFALL_BEGIN_TIME,temp,HYDROLOGY_RAINFALL_BEGIN_TIME_LEN);
  temp[0] = 0x01;//temp[1] = 0x00;
  Hydrology_WriteStoreInfo(HYDROLOGY_SAMPLE_INTERVAL,temp,HYDROLOGY_SAMPLE_INTERVAL_LEN);
  temp[0] = 0x01;//temp[0] = 0x05;
  Hydrology_WriteStoreInfo(HYDROLOGY_WATERLEVEL_STORE_INTERVAL,temp,HYDROLOGY_WATERLEVEL_STORE_INTERVAL_LEN);
  temp[0] = 0x05;
  Hydrology_WriteStoreInfo(HYDROLOGY_RAINFALL_RESOLUTION,temp,HYDROLOGY_RAINFALL_RESOLUTION_LEN);
  temp[0] = 0x01;
  Hydrology_WriteStoreInfo(HYDROLOGY_WATERLEVEL_RESOLUTION,temp,HYDROLOGY_WATERLEVEL_RESOLUTION_LEN);
  temp[0] = 0x01;
  Hydrology_WriteStoreInfo(HYDROLOGY_RAINFALL_ADD_THRESHOLD,temp,HYDROLOGY_RAINFALL_ADD_THRESHOLD_LEN);
  temp[0] = 0x01;temp[1] = 0x00;temp[2] = 0x00;temp[3] = 0x00;
  Hydrology_WriteStoreInfo(HYDROLOGY_WATERLEVEL_BASICVALUE1,temp,HYDROLOGY_WATERLEVEL_BASICVALUE1_LEN);
  temp[0] = 0x01;temp[1] = 0x00;temp[2] = 0x00;
  Hydrology_WriteStoreInfo(HYDROLOGY_WATERLEVEL_CORRECTVALUE1,temp,HYDROLOGY_WATERLEVEL_CORRECTVALUE1_LEN);
  temp[0] = 0x25;temp[1] = 0x01;
  Hydrology_WriteStoreInfo(HYDROLOGY_ADD_WATERLEVEL1,temp,HYDROLOGY_ADD_WATERLEVEL1_LEN);
  temp[0] = 0x01;temp[1] = 0x00;
  Hydrology_WriteStoreInfo(HYDROLOGY_ADD_ABOVE_THRESHOLD,temp,HYDROLOGY_ADD_ABOVE_THRESHOLD_LEN);
  temp[0] = 0x03;temp[1] = 0x00;
  Hydrology_WriteStoreInfo(HYDROLOGY_ADD_BELOW_THRESHOLD,temp,HYDROLOGY_ADD_BELOW_THRESHOLD_LEN);
  memcpy(temp,"*WHU-2018-V2.0",14);
  Hydrology_WriteStoreInfo(HYDROLOGY_SOFTWARE_VERSION,temp,HYDROLOGY_SOFTWARE_VERSION_LEN);
  
    temp[0] = 0x00;temp[1] = 0x00;temp[2] = 0x00;temp[3] = 0x00;temp[4] = 0x00;temp[5] = 0x00;
    for(i=0;i<8;i++){
   Hydrology_WriteStoreInfo(HYDROLOGY_ISR_COUNT1+i*HYDROLOGY_ISR_COUNT_LEN,temp,HYDROLOGY_ISR_COUNT_LEN);
    }
    temp[0] = 0x00;
   Hydrology_WriteStoreInfo(HYDROLOGY_ISR_COUNT_FLAG,temp,HYDROLOGY_ISR_COUNT_FLAG_LEN); //脉冲计数方式，0为单路，1为联合
  
    Hydrology_SetStartIdx(1);  //0,0可测试retrive函数
    Hydrology_SetEndIdx(1);
    Hydrology_SetDataPacketCount(0); //有效数据包数1
    //Store_ClearWork();  //数据全部标记为已发送
//  temp[0] = 1;
//  Hydrology_WriteStoreInfo(HYDROLOGY_RS485_COUNT1,temp,HYDROLOGY_RS485_COUNT_LEN);
//  temp[0] = 0x01;temp[1] = 0x03;temp[2] = 0x00;temp[3] = 0x03;temp[4] = 5;
//  Hydrology_WriteStoreInfo(HYDROLOGY_RS4851,temp,HYDROLOGY_RS485_LEN);
  /*
  temp[0] = 1;
  Hydrology_WriteStoreInfo(HYDROLOGY_RS485_COUNT1,temp,HYDROLOGY_RS485_COUNT_LEN);
  
  temp[0] = 0x01;temp[1] = 0x04;temp[2] = 0x10;temp[3] = 0x10;temp[4] = 2;
  Hydrology_WriteStoreInfo(HYDROLOGY_RS4851,temp,HYDROLOGY_RS485_LEN);
  
  temp[0] = 1;
  Hydrology_WriteStoreInfo(HYDROLOGY_RS485_COUNT2,temp,HYDROLOGY_RS485_COUNT_LEN);
  
  temp[0] = 0x01;temp[1] = 0x04;temp[2] = 0x10;temp[3] = 0x14;temp[4] = 2;
  Hydrology_WriteStoreInfo(HYDROLOGY_RS4852,temp,HYDROLOGY_RS485_LEN);

  */
  return 0;
}

int Hydrology_SetPassword(void)
{
  hydrologyDownBody* pbody = (hydrologyDownBody*)(g_HydrologyMsg.downbody);

  Hydrology_WriteStoreInfo(HYDROLOGY_PASSWORD_ADDR,(pbody->element)[0].value,HYDROLOGY_PASSWORD_LEN);
          
  return 0;
}

int hydrologySetClock(void)
{
  hydrologyDownBody* pbody = (hydrologyDownBody*)(g_HydrologyMsg.downbody);

  Hydrology_SetTime(pbody->sendtime);
          
  return 0;
}

int hydrologySetICCard(void)
{
  hydrologyDownBody* pbody = (hydrologyDownBody*)(g_HydrologyMsg.downbody);

  Hydrology_WriteStoreInfo(HYDROLOGY_STATUS_ALARM_INFO,(pbody->element)[0].value,HYDROLOGY_STATUS_ALARM_INFO_LEN);
          
  return 0;
}

int hydrologyPump(void)
{
  hydrologyDownBody* pbody = (hydrologyDownBody*)(g_HydrologyMsg.downbody);

  Hydrology_WriteStoreInfo(HYDROLOGY_PUMP,(pbody->element)[0].value,HYDROLOGY_PUMP_LEN);
  Hydrology_SetIO_STATUS((pbody->element)[0].value); //+++++
          
  return 0;
}

int hydrologyValve(void)
{
  hydrologyDownBody* pbody = (hydrologyDownBody*)(g_HydrologyMsg.downbody);

  Hydrology_WriteStoreInfo(HYDROLOGY_VALVE,(pbody->element)[0].value,HYDROLOGY_VALVE_LEN);
  Hydrology_SetIO_STATUS((pbody->element)[0].value); //++++++
          
  return 0;
}

int hydrologyGate(void)
{
  char gatesize;
  hydrologyDownBody* pbody = (hydrologyDownBody*)(g_HydrologyMsg.downbody);
  
  gatesize = (pbody->element)[0].guide[0];
  gatesize = ((gatesize - 1)/8 + 1) + 2*gatesize + 1;
  Hydrology_WriteStoreInfo(HYDROLOGY_GATE,(pbody->element)[0].value,HYDROLOGY_GATE_LEN);
  Hydrology_SetIO_STATUS((pbody->element)[0].value); //++++
          
  return 0;
}

int hydrologyWaterSetting(void)
{
  hydrologyDownBody* pbody = (hydrologyDownBody*)(g_HydrologyMsg.downbody);

  Hydrology_WriteStoreInfo(HYDROLOGY_WATERSETTING,(pbody->element)[0].value,HYDROLOGY_WATERSETTING);
          
  return 0;
}

int HydrologyRecord(int index)     // 
{
  uint16_t ERC_Couter = 0;
  int addr = (index - 1) * 2;
  char _temp_ERC_Couter[2];

  Hydrology_ReadStoreInfo(HYDROLOGY_RECORD + addr,_temp_ERC_Couter,2);
  ERC_Couter = (_temp_ERC_Couter[0] << 8) + _temp_ERC_Couter[1];
  ERC_Couter++;
  _temp_ERC_Couter[0] = ERC_Couter >> 8;
  _temp_ERC_Couter[1] = ERC_Couter & 0x00FF;
  Hydrology_WriteStoreInfo(HYDROLOGY_RECORD + addr,_temp_ERC_Couter,2);
                  
  return 0;
}

char IsQuery = 0;
int hydrologyCommand(int type)
{
  switch(type)
  {
    case Picture:
    {
//     Picture_Flag = 1;
      hydrologyChangeMode(M3);
      break;
    }        
    case ConfigurationModification:
    {
      hydrologyBasicInfoConfig();
      break;
    }
    case ConfigurationRead:
    {
      hydrologyBasicInfoRead();
      IsQuery = 1;
      break;
    }
    case ParameterModification:
    {
      hydrologySetPara();
      HydrologyRecord(ERC2);
      break;
    }
    case ParameterRead:
    {
      hydrologyReadPara();
      IsQuery = 1;
      break;
    }
    case InitializeSolidStorage:
    {
      hydrologyInitializeSolidStorage();
      HydrologyRecord(ERC1);
      break;
    }
    case Reset:
    {
      Hydrology_Reset();
      break;
    }        
    case ChangePassword:
    {
      Hydrology_SetPassword();
      HydrologyRecord(ERC5);
      break;
    }        
    case SetClock:
    {
      hydrologySetClock();
      break;
    }
    case SetICCard:
    {
      hydrologySetICCard();
      break;
    }
    case Pump:
    {
      hydrologyPump();
      break;
    }
    case Valve:
    {
      hydrologyValve();
      break;
    }        
    case Gate:
    {
      hydrologyGate();
      break;
    }
    case WaterSetting:
    {
      hydrologyWaterSetting();
      break;
    }        
  }
  return 0;
}


int HydrologyReadSuiteElement(int type,char* guide,char* value)
{
  switch(type)
  {
    case Picture:
    {
//     Picture_Flag = 1;
      hydrologyChangeMode(M3);
      break;
    }        
    case ConfigurationModification:
    {
      hydrologyReadSelect(guide,value);
      break;
    }
    case ConfigurationRead:
    {
      hydrologyReadSelect(guide,value);
      break;
    }
    case ParameterModification:
    {
      hydrologyReadParaSelect(guide,value);
      break;
    }
    case ParameterRead:
    {
      hydrologyReadParaSelect(guide,value);
      break;
    }
    case InitializeSolidStorage:
    {
      hydrologyInitializeSolidStorage();
      HydrologyRecord(ERC1);
      break;
    }
    case Reset:
    {
      ;
      break;
    }        
    case ChangePassword:
    {
      Hydrology_SetPassword();
      HydrologyRecord(ERC5);
      break;
    }        
    case SetClock:
    {
      hydrologySetClock();
      break;
    }
    case SetICCard:
    {
      hydrologySetICCard();
      break;
    }
    case Pump:
    {
      hydrologyPump();
      break;
    }
    case Valve:
    {
      hydrologyValve();
      break;
    }        
    case Gate:
    {
      hydrologyGate();
      break;
    }
    case WaterSetting:
    {
      hydrologyWaterSetting();
      break;
    }        
  }                
  return 0;
}




















