//////////////////////////////////////////////////////
//     文件名: sys_status.h
//   文件版本: 1.0.0  
//   创建时间: 19年10月4日
//   更新内容: 无。 
//       作者: LSHB
//       附注:  
//           1、定义正常工作时指示灯的状态
//             2、定义系统错误时指示灯的状态
//////////////////////////////////////////////////////

#pragma once

void sys_error_status( unsigned int sys_errorcode_runcode);

void led_flash_init(unsigned int timer0A0ccr0);

void RS485_1_sendrecieve_led_flash();

void RS485_2_sendrecieve_led_flash();

void RS232_1_sendrecieve_led_flash();

void RS232_2_sendrecieve_led_flash();

void pulse_input_led_flash();

void switch_input_led_flash();