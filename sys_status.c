//////////////////////////////////////////////////////
//     文件名: sys_status.c
//   文件版本: 1.0.0  
//   创建时间: 19年10月4日
//   更新内容: 无。 
//       作者: LSHB
//       附注:  
//           1、定义正常工作时指示灯的状态
//             2、定义系统错误时指示灯的状态
//////////////////////////////////////////////////////

#include "msp430common.h"
#include "timer.h"
#include "common.h"
#include "led.h"
#include "sys_status.h"




unsigned short int LED_OnOff_Flag=0;



////////////////////////////////////
//    函数名：sys_error_status()，led_flash_init(),RS485_1_sendrecieve_led_flash(),RS485_2_sendrecieve_led_flash(),RS232_1_sendrecieve_led_flash(),RS232_2_sendrecieve_led_flash()，pulse_input_led_flash(),switch_input_led_flash()
//   功能: 系统工作状态指示和错误状态指示
//    参数: 无
//  返回值: 无
//   作者：LSHB
//   时间：2019年10月4日
//    附注: >>  
////////////////////////////////////


void led_flash_init(unsigned int timer0A0ccr0) //控制指示灯闪烁的时间间隔

{
 
  TA0CCR0 = timer0A0ccr0; //给timer0 A0 计数初始值
  
}

void RS485_1_sendrecieve_led_flash()
{
  if (LED_OnOff_Flag == 0)
    {
        Led3_On();
        LED_OnOff_Flag =1;
    }
    else 
    {
        Led3_Off();
        LED_OnOff_Flag =0;       
    }
}


void RS485_2_sendrecieve_led_flash()
{
  if (LED_OnOff_Flag == 0)
    {
        Led4_On();
        LED_OnOff_Flag =1;
    }
    else 
    {
        Led4_Off();
        LED_OnOff_Flag =0;       
    }
}


void RS232_1_sendrecieve_led_flash()
{
  if (LED_OnOff_Flag == 0)
    {
        Led5_On();
        LED_OnOff_Flag =1;
    }
    else 
    {
        Led5_Off();
        LED_OnOff_Flag =0;       
    }
}

void RS232_2_sendrecieve_led_flash()
{
  if (LED_OnOff_Flag == 0)
    {
        Led6_On();
        LED_OnOff_Flag =1;
    }
    else 
    {
        Led6_Off();
        LED_OnOff_Flag =0;       
    }
}



void pulse_input_led_flash()
{
  if (LED_OnOff_Flag == 0)
    {
        Led7_On();
        LED_OnOff_Flag =1;
    }
    else 
    {
        Led7_Off();
        LED_OnOff_Flag =0;       
    }
}

void switch_input_led_flash()
{
  if (LED_OnOff_Flag == 0)
    {
        Led8_On();
        LED_OnOff_Flag =1;
    }
    else 
    {
        Led8_Off();
        LED_OnOff_Flag =0;       
    }
}





void sys_error_status( unsigned  int sys_errorcode_runcode)
{
    
    switch(sys_errorcode_runcode)
    {
      case 1:
        {
        if (LED_OnOff_Flag == 0)
    {
        Led3_On();
        LED_OnOff_Flag =1;
    }
    else 
    {
        Led3_Off();
        LED_OnOff_Flag =0;       
    }
        break;
        }
        
   case 2:
     {
        if (LED_OnOff_Flag == 0)
    {
        Led4_On();
        LED_OnOff_Flag =1;
    }
    else 
    {
        Led4_Off();
        LED_OnOff_Flag =0;       
    }
        break;     
     } 
     
  case 3:
    {
        if (LED_OnOff_Flag == 0)
    {
      Led3_On(); 
      Led4_On();
        LED_OnOff_Flag =1;
    }
    else 
    {
        Led3_Off();
        Led4_Off();
        LED_OnOff_Flag =0;       
    }
        break; 
    }
        
  case 4:
    {
        if (LED_OnOff_Flag == 0)
    {
      Led5_On();
        LED_OnOff_Flag =1;
    }
    else 
    {
        Led5_Off();
        LED_OnOff_Flag =0;       
    }
        break;
    }
        
   case 5:
     {
        if (LED_OnOff_Flag == 0)
    {
      Led3_On(); 
      Led5_On();
        LED_OnOff_Flag =1;
    }
    else 
    {
        Led3_Off();
        Led5_Off();
        LED_OnOff_Flag =0;       
    }
        break;  
     }
        
   case 6:
     {
        if (LED_OnOff_Flag == 0)
    {
      Led4_On(); 
      Led5_On();
        LED_OnOff_Flag =1;
    }
    else 
    {
        Led4_Off();
        Led5_Off();
        LED_OnOff_Flag =0;       
    }
        break;    
     }
        
   case 7:
     {
        if (LED_OnOff_Flag == 0)
    {
        Led3_On();
        Led4_On(); 
        Led5_On();
        LED_OnOff_Flag =1;
    }
    else 
    {
        Led3_Off();
        Led4_Off();
        Led5_Off();
        LED_OnOff_Flag =0;       
    }
        break; 
     }
        
  case 8:
    {
        if (LED_OnOff_Flag == 0)
    {
        Led6_On();
        LED_OnOff_Flag =1;
    }
    else 
    {
        Led6_Off();
        LED_OnOff_Flag =0;       
    }
        break;
    }
        
    case 9:
      {
        if (LED_OnOff_Flag == 0)
    {
      Led3_On(); 
      Led6_On();
        LED_OnOff_Flag =1;
    }
    else 
    {
        Led3_Off();
        Led6_Off();
        LED_OnOff_Flag =0;       
    }
        break;  
      }
        
      case 10:
      {
        if (LED_OnOff_Flag == 0)
    {
      Led4_On(); 
      Led6_On();
      LED_OnOff_Flag =1;
    }
    else 
    {
        Led4_Off();
        Led6_Off();
        LED_OnOff_Flag =0;       
    }
        
        break;  
      }
        
  case 11:
      {
        if (LED_OnOff_Flag == 0)
    {
        Led3_On(); 
        Led4_On(); 
        Led6_On();
        LED_OnOff_Flag =1;
    }
    else 
    {
        Led3_Off();
        Led4_Off();
        Led6_Off();
        LED_OnOff_Flag =0;       
    }
        
        break;  
      }  
      
 case 12:
      {
        if (LED_OnOff_Flag == 0)
    {
      Led5_On(); 
      Led6_On();
      LED_OnOff_Flag =1;
    }
    else 
    {
        Led5_Off();
        Led6_Off();
        LED_OnOff_Flag =0;       
    }
        
        break;  
      }           
      
 case 13:
      {
        if (LED_OnOff_Flag == 0)
    {
        Led3_On(); 
        Led5_On(); 
        Led6_On();
        LED_OnOff_Flag =1;
    }
    else 
    {
        Led3_Off();
        Led5_Off();
        Led6_Off();
        LED_OnOff_Flag =0;       
    }
        
        break;  
      }       
      
    case 14:
      {
        if (LED_OnOff_Flag == 0)
    {
        Led4_On(); 
        Led5_On(); 
        Led6_On();
        LED_OnOff_Flag =1;
    }
    else 
    {
        Led4_Off();
        Led5_Off();
        Led6_Off();
        LED_OnOff_Flag =0;       
    }
        
        break;  
      }    
      
  case 15:
      {
        if (LED_OnOff_Flag == 0)
    {
        Led3_On(); 
        Led4_On();
        Led5_On();
        Led6_On();
        LED_OnOff_Flag =1;
    }
    else 
    {
        Led3_Off();
        Led4_Off();
        Led5_Off();
        Led6_Off();
        LED_OnOff_Flag =0;       
    }
        
        break;  
      }      

   case 16:
      {
        if (LED_OnOff_Flag == 0)
    {
        Led7_On(); 
        LED_OnOff_Flag =1;
    }
    else 
    {
        Led7_Off();
        LED_OnOff_Flag =0;       
    }
        
        break;  
      }          
      
  case 17:
      {
        if (LED_OnOff_Flag == 0)
    {
        Led3_On(); 
        Led7_On();
        LED_OnOff_Flag =1;
    }
    else 
    {
        Led3_Off();
        Led7_Off();
        LED_OnOff_Flag =0;       
    }
        break;  
      }       
      
    case 18:
      {
        if (LED_OnOff_Flag == 0)
    {
        Led4_On();
        Led7_On();
        LED_OnOff_Flag =1;
    }
    else 
    {
        Led4_Off();
        Led7_Off();
        LED_OnOff_Flag =0;       
    }
        
        break;  
      }         
 
       case 19:
      {
        if (LED_OnOff_Flag == 0)
    {
        Led3_On(); 
        Led4_On();
        Led7_On();
        LED_OnOff_Flag =1;
    }
    else 
    {
        Led3_Off();
        Led4_Off();
        Led7_Off();
        LED_OnOff_Flag =0;       
    }
        
        break;  
      }      
  case 20:
      {
        if (LED_OnOff_Flag == 0)
    {
        Led5_On();
        Led7_On();
        LED_OnOff_Flag =1;
    }
    else 
    {
        Led5_Off();
        Led7_Off();
        LED_OnOff_Flag =0;       
    }
        
        break;  
      }           
      
  case 21:
      {
        if (LED_OnOff_Flag == 0)
    {
        Led3_On(); 
        Led5_On();
        Led7_On();
        LED_OnOff_Flag =1;
    }
    else 
    {
        Led3_Off();
        Led5_Off();
        Led7_Off();
        LED_OnOff_Flag =0;       
    }
        
        break;  
      }           
  
   case 22:
      {
        if (LED_OnOff_Flag == 0)
    {
        Led4_On();
        Led5_On();
        Led7_On();
        LED_OnOff_Flag =1;
    }
    else 
    {
        Led4_Off();
        Led5_Off();
        Led7_Off();
        LED_OnOff_Flag =0;       
    }
        
        break;  
      }          
      
  case 23:
      {
        if (LED_OnOff_Flag == 0)
    {
        Led3_On(); 
        Led4_On();
        Led5_On();
        Led7_On();
        LED_OnOff_Flag =1;
    }
    else 
    {
        Led3_Off();
        Led4_Off();
        Led5_Off();
        Led7_Off();
        LED_OnOff_Flag =0;       
    }
        
        break;  
      }           
 case 24:
      {
        if (LED_OnOff_Flag == 0)
    {
        Led6_On();
        Led7_On();
        LED_OnOff_Flag =1;
    }
    else 
    {
        Led6_Off();
        Led7_Off();
        LED_OnOff_Flag =0;       
    }
        
        break;  
      }            
 
 case 25:
      {
        if (LED_OnOff_Flag == 0)
    {
        Led3_On(); 
        Led6_On();
        Led7_On();
        LED_OnOff_Flag =1;
    }
    else 
    {
        Led3_Off();
        Led6_Off();
        Led7_Off();
        LED_OnOff_Flag =0;       
    }
        
        break;  
      }            
      
 case 26:
      {
        if (LED_OnOff_Flag == 0)
    {
        Led4_On();
        Led6_On();
        Led7_On();
        LED_OnOff_Flag =1;
    }
    else 
    {
        Led4_Off();
        Led6_Off();
        Led7_Off();
        LED_OnOff_Flag =0;       
    }
        
        break;  
      }            
      
  case 27:
      {
        if (LED_OnOff_Flag == 0)
    {
        Led3_On(); 
        Led4_On();
        Led6_On();
        Led7_On();
        LED_OnOff_Flag =1;
    }
    else 
    {
        Led3_Off();
        Led4_Off();
        Led6_Off();
        Led7_Off();
        LED_OnOff_Flag =0;       
    }
        
        break;  
      }      
      
  case 28:
      {
        if (LED_OnOff_Flag == 0)
    {
        Led5_On();
        Led6_On();
        Led7_On();
        LED_OnOff_Flag =1;
    }
    else 
    {
        Led5_Off();
        Led6_Off();
        Led7_Off();
        LED_OnOff_Flag =0;       
    }
        
        break;  
      }           
      
  case 29:
      {
        if (LED_OnOff_Flag == 0)
    {
        Led3_On(); 
        Led5_On();
        Led6_On();
        Led7_On();
        LED_OnOff_Flag =1;
    }
    else 
    {
        Led3_Off();
        Led5_Off();
        Led6_Off();
        Led7_Off();
        LED_OnOff_Flag =0;       
    }
        
        break;  
      }           
 
  case 30:
      {
        if (LED_OnOff_Flag == 0)
    {
        Led4_On(); 
        Led5_On();
        Led6_On();
        Led7_On();
        LED_OnOff_Flag =1;
    }
    else 
    {
        Led4_Off();
        Led5_Off();
        Led6_Off();
        Led7_Off();
        LED_OnOff_Flag =0;       
    }
        
        break;  
      }                
      
 case 31:
      {
        if (LED_OnOff_Flag == 0)
    {
        Led3_On(); 
        Led4_On();
        Led5_On();
        Led6_On();
        Led7_On();
        LED_OnOff_Flag =1;
    }
    else 
    {
        Led3_Off();
        Led4_Off();
        Led5_Off();
        Led6_Off();
        Led7_Off();
        LED_OnOff_Flag =0;       
    }
        
        break;  
      }                 
      
   case 32:
      {
        if (LED_OnOff_Flag == 0)
    {
        Led8_On();
        LED_OnOff_Flag =1;
    }
    else 
    {
        Led8_Off();
        LED_OnOff_Flag =0;       
    }
        
        break;  
      }                     
      
      case 33:
      {
        if (LED_OnOff_Flag == 0)
    {
        Led3_On(); 
        Led8_On();
        LED_OnOff_Flag =1;
    }
    else 
    {
        Led3_Off();
        Led8_Off();
        LED_OnOff_Flag =0;       
    }
        
        break;  
      }                 
      
     case 34:
      {
        if (LED_OnOff_Flag == 0)
    {
        Led4_On();
        Led8_On();
        LED_OnOff_Flag =1;
    }
    else 
    {
        Led4_Off();
        Led8_Off();
        LED_OnOff_Flag =0;       
    }
        
        break;  
      }                  
      
    case 35:
      {
        if (LED_OnOff_Flag == 0)
    {
        Led3_On(); 
        Led4_On();
        Led8_On();
        LED_OnOff_Flag =1;
    }
    else 
    {
        Led3_Off();
        Led4_Off();
        Led8_Off();
        LED_OnOff_Flag =0;       
    }
        
        break;  
      }                   
      
      
      case 36:
      {
        if (LED_OnOff_Flag == 0)
    {
        Led5_On();
        Led8_On();
        LED_OnOff_Flag =1;
    }
    else 
    {
        Led5_Off();
        Led8_Off();
        LED_OnOff_Flag =0;       
    }
        
        break;  
      }                 
      
    case 37:
      {
        if (LED_OnOff_Flag == 0)
    {
        Led3_On(); 
        Led5_On();
        Led8_On();
        LED_OnOff_Flag =1;
    }
    else 
    {
        Led3_Off();
        Led5_Off();
        Led8_Off();
        LED_OnOff_Flag =0;       
    }
        
        break;  
      }              
      
     case 38:
      {
        if (LED_OnOff_Flag == 0)
    {
        Led4_On();
        Led5_On();
        Led8_On();
        LED_OnOff_Flag =1;
    }
    else 
    {
        Led4_Off();
        Led5_Off();
        Led8_Off();
        LED_OnOff_Flag =0;       
    }
        
        break;  
      }                  
      
    case 39:
      {
        if (LED_OnOff_Flag == 0)
    {
        Led3_On(); 
        Led4_On();
        Led5_On();
        Led8_On();
        LED_OnOff_Flag =1;
    }
    else 
    {
        Led3_Off();
        Led4_Off();
        Led5_Off();
        Led8_Off();
        LED_OnOff_Flag =0;       
    }
        
        break;  
      }                   
      
      
    case 40:
      {
        if (LED_OnOff_Flag == 0)
    {
        Led6_On();
        Led8_On();
        LED_OnOff_Flag =1;
    }
    else 
    {
        Led6_Off();
        Led8_Off();
        LED_OnOff_Flag =0;       
    }
        
        break;  
      }                   
      
   case 41:
      {
        if (LED_OnOff_Flag == 0)
    {
        Led3_On(); 
        Led6_On();
        Led8_On();
        LED_OnOff_Flag =1;
    }
    else 
    {
        Led3_Off();
        Led6_Off();
        Led8_Off();
        LED_OnOff_Flag =0;       
    }
        
        break;  
      }                    
      
    case 42:
      {
        if (LED_OnOff_Flag == 0)
    {
        Led4_On();
        Led6_On();
        Led8_On();
        LED_OnOff_Flag =1;
    }
    else 
    {
        Led4_Off();
        Led6_Off();
        Led8_Off();
        LED_OnOff_Flag =0;       
    }
        
        break;  
      }              
      
    case 43:
      {
        if (LED_OnOff_Flag == 0)
    {
        Led3_On(); 
        Led4_On();
        Led6_On();
        Led8_On();
        LED_OnOff_Flag =1;
    }
    else 
    {
        Led3_Off();
        Led4_Off();
        Led6_Off();
        Led8_Off();
        LED_OnOff_Flag =0;       
    }
        
        break;  
      }                   
      
     case 44:
      {
        if (LED_OnOff_Flag == 0)
    {
        Led5_On();
        Led6_On();
        Led8_On();
        LED_OnOff_Flag =1;
    }
    else 
    {
        Led5_Off();
        Led6_Off();
        Led8_Off();
        LED_OnOff_Flag =0;       
    }
        
        break;  
      }                  
      
      case 45:
      {
        if (LED_OnOff_Flag == 0)
    {
        Led3_On(); 
        Led5_On();
        Led6_On();
        Led8_On();
        LED_OnOff_Flag =1;
    }
    else 
    {
        Led3_Off();
        Led5_Off();
        Led6_Off();
        Led8_Off();
        LED_OnOff_Flag =0;       
    }
        
        break;  
      }                 
      
   case 46:
      {
        if (LED_OnOff_Flag == 0)
    {
        Led4_On();
        Led5_On();
        Led6_On();
        Led8_On();
        LED_OnOff_Flag =1;
    }
    else 
    {
        Led4_Off();
        Led5_Off();
        Led6_Off();
        Led8_Off();
        LED_OnOff_Flag =0;       
    }
        
        break;  
      }                    
      
    case 47:
      {
        if (LED_OnOff_Flag == 0)
    {
        Led3_On(); 
        Led4_On();
        Led5_On();
        Led6_On();
        Led8_On();
        LED_OnOff_Flag =1;
    }
    else 
    {
        Led3_Off();
        Led4_Off();
        Led5_Off();
        Led6_Off();
        Led8_Off();
        LED_OnOff_Flag =0;       
    }
        
        break;  
      }              
      
     case 48:
      {
        if (LED_OnOff_Flag == 0)
    {
        Led7_On();
        Led8_On();
        LED_OnOff_Flag =1;
    }
    else 
    {
        Led7_Off();
        Led8_Off();
        LED_OnOff_Flag =0;       
    }
        
        break;  
      }                  
      
      
     case 49:
      {
        if (LED_OnOff_Flag == 0)
    {
        Led3_On(); 
        Led7_On();
        Led8_On();
        LED_OnOff_Flag =1;
    }
    else 
    {
        Led3_Off();
        Led7_Off();
        Led8_Off();
        LED_OnOff_Flag =0;       
    }
        
        break;  
      }                  
      
    case 50:
      {
        if (LED_OnOff_Flag == 0)
    {
        Led4_On();
        Led7_On();
        Led8_On();
        LED_OnOff_Flag =1;
    }
    else 
    {
        Led4_Off();
        Led7_Off();
        Led8_Off();
        LED_OnOff_Flag =0;       
    }
        
        break;  
      }                   
  
    case 51:
      {
        if (LED_OnOff_Flag == 0)
    {
        Led3_On(); 
        Led4_On();
        Led7_On();
        Led8_On();
        LED_OnOff_Flag =1;
    }
    else 
    {
        Led3_Off();
        Led4_Off();
        Led7_Off();
        Led8_Off();
        LED_OnOff_Flag =0;       
    }
        
        break;  
      }                   
      
    case 52:
      {
        if (LED_OnOff_Flag == 0)
    {
        Led5_On();
        Led7_On();
        Led8_On();
        LED_OnOff_Flag =1;
    }
    else 
    {
        Led5_Off();
        Led7_Off();
        Led8_Off();
        LED_OnOff_Flag =0;       
    }
        
        break;  
      }                   
      
    case 53:
      {
        if (LED_OnOff_Flag == 0)
    {
        Led3_On(); 
        Led5_On();
        Led7_On();
        Led8_On();
        LED_OnOff_Flag =1;
    }
    else 
    {
        Led3_Off();
        Led5_Off();
        Led7_Off();
        Led8_Off();
        LED_OnOff_Flag =0;       
    }
        
        break;  
      }                   
      
   case 54:
      {
        if (LED_OnOff_Flag == 0)
    {
        Led4_On();
        Led5_On();
        Led7_On();
        Led8_On();
        LED_OnOff_Flag =1;
    }
    else 
    {
        Led4_Off();
        Led5_Off();
        Led7_Off();
        Led8_Off();
        LED_OnOff_Flag =0;       
    }
        
        break;  
      }              

   case 55:
      {
        if (LED_OnOff_Flag == 0)
    {
        Led3_On(); 
        Led4_On();
        Led5_On();
        Led7_On();
        Led8_On();
        LED_OnOff_Flag =1;
    }
    else 
    {
        Led3_Off();
        Led4_Off();
        Led5_Off();
        Led7_Off();
        Led8_Off();
        LED_OnOff_Flag =0;       
    }
        
        break;  
      }              

   case 56:
      {
        if (LED_OnOff_Flag == 0)
    {
        Led6_On();
        Led7_On();
        Led8_On();
        LED_OnOff_Flag =1;
    }
    else 
    {
        Led6_Off();
        Led7_Off();
        Led8_Off();
        LED_OnOff_Flag =0;       
    }
        
        break;  
      }              

   case 57:
      {
        if (LED_OnOff_Flag == 0)
    {
        Led3_On(); 
        Led6_On();
        Led7_On();
        Led8_On();
        LED_OnOff_Flag =1;
    }
    else 
    {
        Led3_Off();
        Led6_Off();
        Led7_Off();
        Led8_Off();
        LED_OnOff_Flag =0;       
    }
        
        break;  
      }              
      
     case 58:
      {
        if (LED_OnOff_Flag == 0)
    {
        Led4_On();
        Led6_On();
        Led7_On();
        Led8_On();
        LED_OnOff_Flag =1;
    }
    else 
    {
        Led4_Off();
        Led6_Off();
        Led7_Off();
        Led8_Off();
        LED_OnOff_Flag =0;       
    }
        
        break;  
      }                  
      
    case 59:
      {
        if (LED_OnOff_Flag == 0)
    {
        Led3_On(); 
        Led4_On();
        Led6_On();
        Led7_On();
        Led8_On();
        LED_OnOff_Flag =1;
    }
    else 
    {
        Led3_Off();
        Led4_Off();
        Led6_Off();
        Led7_Off();
        Led8_Off();
        LED_OnOff_Flag =0;       
    }
        
        break;  
      }                   
      
      case 60:
      {
        if (LED_OnOff_Flag == 0)
    {
        Led5_On();
        Led6_On();
        Led7_On();
        Led8_On();
        LED_OnOff_Flag =1;
    }
    else 
    {
        Led5_Off();
        Led6_Off();
        Led7_Off();
        Led8_Off();
        LED_OnOff_Flag =0;       
    }
        
        break;  
      }                 
      
   case 61:
      {
        if (LED_OnOff_Flag == 0)
    {
        Led3_On(); 
        Led5_On();
        Led6_On();
        Led7_On();
        Led8_On();
        LED_OnOff_Flag =1;
    }
    else 
    {
        Led3_Off();
        Led5_Off();
        Led6_Off();
        Led7_Off();
        Led8_Off();
        LED_OnOff_Flag =0;       
    }
        
        break;  
      }              
      
    case 62:
      {
        if (LED_OnOff_Flag == 0)
    {
        Led4_On();
        Led5_On();
        Led6_On();
        Led7_On();
        Led8_On();
        LED_OnOff_Flag =1;
    }
    else 
    {
        Led4_Off();
        Led5_Off();
        Led6_Off();
        Led7_Off();
        Led8_Off();
        LED_OnOff_Flag =0;       
    }
        
        break;  
      }              
      
     case 63:
      {
        if (LED_OnOff_Flag == 0)
    {
        Led3_On(); 
        Led4_On();
        Led5_On();
        Led6_On();
        Led7_On();
        Led8_On();
        LED_OnOff_Flag =1;
    }
    else 
    {
        Led3_Off();
        Led4_Off();
        Led5_Off();
        Led6_Off();
        Led7_Off();
        Led8_Off();
        LED_OnOff_Flag =0;       
    }
        
        break;  
      }                  
      
  case 64:
      {
        if (LED_OnOff_Flag == 0)
    {
        Led3_On(); 
        LED_OnOff_Flag =1;
    }
    else 
    {
        Led3_Off();
        LED_OnOff_Flag =0;       
    }
        
        break;  
      }                 
      
 case 65:
      {
        if (LED_OnOff_Flag == 0)
    {
        Led4_On(); 
        LED_OnOff_Flag =1;
    }
    else 
    {
        Led4_Off();
        LED_OnOff_Flag =0;       
    }
        
        break;  
      }              
      
  case 66:
      {
        if (LED_OnOff_Flag == 0)
    {
        Led5_On(); 
        LED_OnOff_Flag =1;
    }
    else 
    {
        Led5_Off();
        LED_OnOff_Flag =0;       
    }
        
        break;  
      }                  
      
      
  case 67:
      {
        if (LED_OnOff_Flag == 0)
    {
        Led6_On(); 
        LED_OnOff_Flag =1;
    }
    else 
    {
        Led6_Off();
        LED_OnOff_Flag =0;       
    }
        
        break;  
      }          
      
      
  case 68:
      {
        if (LED_OnOff_Flag == 0)
    {
        Led7_On(); 
        LED_OnOff_Flag =1;
    }
    else 
    {
        Led7_Off();
        LED_OnOff_Flag =0;       
    }
        
        break;  
      }                  
      
      
 case 69:
      {
        if (LED_OnOff_Flag == 0)
    {
        Led8_On(); 
        LED_OnOff_Flag =1;
    }
    else 
    {
        Led8_Off();
        LED_OnOff_Flag =0;       
    }
        
        break;  
      }                   
     
      
      default:
        break;
    }
}