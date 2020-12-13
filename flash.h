//////////////////////////////////////////////////////
//     文件名: flash.h
//   文件版本: 0.0.0
//   创建时间: 09年 8月10日
//   更新内容:  
//       作者: 林智
//       附注: 无
//
//////////////////////////////////////////////////////
#pragma once 

#define BANK_D_START_ADDR       0x30000
#define BANK_D_END_ADDR         0x3FFFF
#define BANK_D_SIZE             0x10000
#define FLASH_SEGMENT_SIZE      512


void FLASH_EraseOneSEG ( unsigned long int Address ); //擦除flash,每次擦删除一个段

// *****************************************************
//把数组从片内FLASH读出
//参数waddr:数组的位置
//参数value:读出的数组的指针
//参数size:读出的数组的大小
// *****************************************************

void FLASH_EraseOneBank ( unsigned long int Address );//擦除flash,每次擦删除一个BANK

void FLASH_Writew ( unsigned long int Address,unsigned int nValue ); //写入一个字节

// *****************************************************
//把数组写入数据到片内FLASH
//参数Address:保存数组的位置
//参数value:数组的指针
//参数size:数组的大小
//先擦除再写
// *****************************************************
void Write_flash_Segment ( unsigned long int Address,  char* value,  int size );
int Read_Flashw ( unsigned long int waddr ); //读取一个字节


void Write_Flash_Erase_Segment ( unsigned long int Address,  char* value,  int size );
void Write_Flash_Segment ( unsigned long int Address,  char* value,  int size );
void Read_Flash_Segment(unsigned long int Address, char* value, int size);