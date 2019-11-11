//////////////////////////////////////////////////////
//     文件名: rom.h
//   文件版本: 1.0.0
//   创建时间: 09年 11月30日
//   更新内容:  
//       作者: 林智
//       附注: 无
//
//////////////////////////////////////////////////////

#pragma once  
 
#define SCL BIT2
#define SDA BIT1
 
//高级函数

//   中级函数 ROM函数 
void ROM_WP_OFF();
void ROM_WP_ON();
void ROM_Init();
int  ROM_WriteByte_RTC(long addr, char data);
int  ROM_WriteByte(long addr,char data); 
int  ROM_WriteBytes(long addr,const char * src,int length);
int  ROM_ReadByte(long addr,char *dest); 
int  ROM_ReadBytes(long addr, char *dest ,int length);
   
//  低级函数 I2C函数 
void I2C_Initial(void);
void I2C_Set_sda_high(void);
void I2C_Set_sda_low (void);
void I2C_Set_sck_high(void);
void I2C_Set_sck_low (void);
int  I2C_GetACK(void);
void I2C_SetACK(void);
void I2C_SetNAK(void);
void I2C_START(void);
void I2C_STOP(void);
void I2C_TxByte(char);
int  I2C_RxByte(void); 

