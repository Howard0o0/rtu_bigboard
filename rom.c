//////////////////////////////////////////////////////
//     文件名: rom.c
//   文件版本: 1.0.0
//   创建时间: 09年11月30日
//   更新内容:  
//       作者: 林智
//       附注: 无
//
//////////////////////////////////////////////////////
 
#include "msp430common.h"
#include "rom.h"
#include "common.h" 
#include "rtc.h" 

//   ROM  中级函数 

void ROM_WP_OFF()       //关闭写保护
{
    P5DIR |= BIT5;              //u3
    P5OUT &= ~(BIT5);
    
    P5DIR |= BIT4;              //u4
    P5OUT &= ~(BIT4);
    
    P3DIR |= BIT7;              //u5
    P3OUT &= ~(BIT7);
    
    P3DIR |= BIT6;         //u6     
    P3OUT &= ~(BIT6);
}
void ROM_WP_ON()            //开启写保护
{
    P5DIR |= BIT5;              //u3
    P5OUT |= BIT5;
    
    P5DIR |= BIT4;              //u4
    P5OUT |= BIT4;
    
    P3DIR |= BIT7;              //u5
    P3OUT |= BIT7;
    
    P3DIR |= BIT6;         //u6     
    P3OUT |= BIT6;
}
void ROM_Init()
{  
    DownInt();
    I2C_Initial(); 
    UpInt();
}
//供RTC使用的WriteByte
int ROM_WriteByte_RTC(long addr, char data)                     //修改了addr的数据类型以满足addr长度要求
{                                                               //修改为at24c1024b的时序/lshb 2019/08/27
    DownInt();
    
    if( addr <524287-31 || addr >524287)//地址空间 0x0007ffe0 --0x0007ffff
    {
        UpInt();
        return -1;
    }
    ROM_WP_OFF();
    //构造地址
    //A2 A1 P0 高8位 低8位 共19位地址
    unsigned char hi= (addr >> 8) & 0xFF ;
    unsigned char lo= addr & 0xFF ;
    unsigned char nTemp = 0xA0 + ((addr / 0x10000) << 1)  ; //写命令
    // 启动数据总线
    I2C_START();
    // 发送控制字节
    I2C_TxByte(nTemp);
    // 等待 ACK
    nTemp = I2C_GetACK();
    if(nTemp & BIT3) 
    {
        UpInt();
        return -1;
    }
    //高位地址字节
    I2C_TxByte(hi);
    // 等待 ACK
    nTemp = I2C_GetACK();
    if(nTemp & BIT3) 
    {
        UpInt();
        return -1;
    }
    // 发送低位地址字节
    I2C_TxByte(lo);
    // 等待 ACK
    nTemp = I2C_GetACK(); 
    if(nTemp & BIT3)
    {
        UpInt();
        return -2;
    }
    // 发送数据字节
    I2C_TxByte(data);
    // 等待 ACK
    nTemp = I2C_GetACK();
    if(nTemp & BIT3) 
    {
        UpInt();
        return -3;
    }
    
    // 停止总线
    I2C_STOP();
    ROM_WP_ON();
    _NOP();
    UpInt();
    return (nTemp & SDA);
}

int ROM_WriteByte(long addr, char data)                 //修改了addr的数据类型以满足addr长度要求
{                                                        //修改为at24c1024b的时序/lshb 2019/08/27
    DownInt();
    
    if( addr >524287-32)//地址空间 0x0000  -- 0x0007ffdf
    {
        UpInt();
        return -1;
    }
    ROM_WP_OFF();
    //构造地址
    //A2 A1 P0 高8位 低8位 共19位地址
    unsigned char hi= (addr >> 8) & 0xFF ;
    unsigned char lo= addr & 0xFF ;
    unsigned char nTemp = 0xA0 + ((addr / 0x10000) << 1)  ; //写命令
    // 启动数据总线
    I2C_START();
    // 发送控制字节
    I2C_TxByte(nTemp);
    // 等待 ACK
    nTemp = I2C_GetACK();
    if(nTemp & BIT3) 
    {
        UpInt();
        return -1;
    }
    //高位地址字节
    I2C_TxByte(hi);
    // 等待 ACK
    nTemp = I2C_GetACK();
    if(nTemp & BIT3) 
    {
        UpInt();
        return -1;
    }
    // 发送低位地址字节
    I2C_TxByte(lo);
    // 等待 ACK
    nTemp = I2C_GetACK(); 
    if(nTemp & BIT3)
    {
        UpInt();
        return -2;
    }
    // 发送数据字节
    I2C_TxByte(data);
    // 等待 ACK
    nTemp = I2C_GetACK();
    if(nTemp & BIT3) 
    {
        UpInt();
        return -3;
    }
    
    // 停止总线
    I2C_STOP();
    ROM_WP_ON();
    _NOP();
    UpInt();
    return (nTemp & SDA);
}
//最终使用的写入多字节函数,该函数不考虑边界
static int _ROM_WriteBytes(long addr,const char * src,int length)               //修改了addr的数据类型以满足addr长度要求
{//该函数 不会被外部函数调用, 内部函数已经考虑了中断问题,所以此处不必考虑中断/     //修改为at24c1024b的时序/lshb 2019/08/27
    if(length >256)
    { 
        return -1;
    }
    if(length <1)
    { 
        return -1;
    }
    ROM_WP_OFF();

    //构造地址
    //A2 A1 P0 高8位 低8位 共19位地址
    unsigned char hi= (addr >> 8) & 0xFF ;
    unsigned char lo= addr & 0xFF ;
    unsigned char nTemp = 0xA0 + ((addr / 0x10000) << 1)  ; //写命令
    // 启动数据总线
    I2C_START();
    // 发送控制字节
    I2C_TxByte(nTemp);
    // 等待 ACK
    nTemp = I2C_GetACK();
    if(nTemp & BIT3)
    { 
        return -1; 
    }
    //高位地址字节
    I2C_TxByte(hi);
    // 等待 ACK
    nTemp = I2C_GetACK();
    if(nTemp & BIT3)
    { 
        return -1; 
    }
    // 发送低位地址字节
    I2C_TxByte(lo);
    // 等待 ACK
    nTemp = I2C_GetACK();
    if(nTemp & BIT3) 
    { 
        return -2; 
    }
    // 发送数据字节
    for(int i = 0; i < length;i++)
    {
        I2C_TxByte(src[i]);
        // 等待 ACK
        nTemp = I2C_GetACK();
        if(nTemp & BIT3)
        { 
            return -3;
        }
    }
    // 停止总线
    I2C_STOP();	
    ROM_WP_ON();
    return (nTemp & SDA); 
}
//
//  考虑换页
//
int ROM_WriteBytes(long addr,const char * src,int length)       //修改了addr的数据类型以满足addr长度要求                                                                  
{                                                                 //修改了数据的长度要求
    DownInt();                                                   //修改为at24c1024b的时序/lshb 2019/08/27
    if( addr >524287-32)//地址空间 0x0000  -- 0x0007ffe0
    {
        UpInt();
        return -1;
    }
    if(length >524287-32)
    { 
        UpInt();
        printf("length=%d>256",length);
        return -1;
    }
    if(length <1)
    { 
        UpInt();
        printf("length=%d<1",length);
        return -1;
    }
 
    //
    //  判断地址和边界的关系
    //
    int bytes = 256 - addr%256; 
    if(length<=bytes)//一次就能发完.
    {
        if(_ROM_WriteBytes(addr,src,length)<0)
        {
            UpInt();
            return -3;
        } 
        UpInt();
        return 0;
    }
    else
    {
      if(_ROM_WriteBytes(addr,src,bytes)<0)
      { 
          UpInt();
          return -3;
      }
    }
    
    int leftBytes=length-bytes;//判断剩余多少字节
    long nextAddr=addr+bytes; //补足剩余的字节数,就为下一页的起始地址.
    while(leftBytes>0)
    {
      
      if(leftBytes<=256)
      {
        
        if(_ROM_WriteBytes(nextAddr,&(src[bytes]),leftBytes)<0)
        {
            UpInt();
            return -3;
        } 
        UpInt();
        return 0;
      }
      else 
      {
        if(_ROM_WriteBytes(nextAddr,&(src[bytes]),256)<0)
        {
            UpInt();
            return -3;
        }
        bytes+=256;
        leftBytes=leftBytes-256;//判断剩余多少字节
        nextAddr=nextAddr+256; //补足剩余的字节数,就为下一页的起始地址.
      }
    }

//    
//    if(leftBytes<=0)//说明发送完毕了
//    { 
//        UpInt();
//        return 0;
//    }
//    long nextAddr=addr+bytes; //补足剩余的字节数,就为下一页的起始地址.
//    //新的地址一次性写完
//    if(_ROM_WriteBytes(nextAddr,&(src[bytes]),leftBytes)<0)
//    { 
//        UpInt();
//        return -3;
//    } 
//    UpInt();
//    return 0;
}

int ROM_ReadByte(long addr,char *dest)  //修改了addr的数据类型以满足addr长度要求
{                                        //修改为at24c1024b的时序/lshb 2019/08/27
    DownInt();
    
    if(addr>524287)     
    { 
        UpInt();
        return -1;
    }
    // 读 会 多发送一个伪字节 用来传送地址.

    //构造地址
    //A2 A1 P0 高8位 低8位 共19位地址
    unsigned char hi= (addr >> 8) & 0xFF ;
    unsigned char lo= addr & 0xFF ;
    unsigned char nTemp = 0xA0 + ((addr / 0x10000) << 1)  ; //写命令
    // 启动数据总线
    I2C_START();
    // 发送控制字节
    I2C_TxByte(nTemp);
    // 等待 ACK
    nTemp = I2C_GetACK();
    if(nTemp & BIT3) 
    {
        UpInt();
        return -1;
    }
    //高位地址字节
    I2C_TxByte(hi);
    // 等待 ACK
    nTemp = I2C_GetACK();
    if(nTemp & BIT3) 
    {
        UpInt();
        return -1;
    }
    // 发送低位地址字节
    I2C_TxByte(lo);
    // 等待 ACK
    nTemp = I2C_GetACK();
    if(nTemp & BIT3)
    {
        UpInt();
        return -1;
    }
     
    // 启动数据总线
    I2C_START();
    // 发送控制字节 
    nTemp = 0xA1 + ((addr / 0x10000) << 1);
    I2C_TxByte(nTemp);
    // 等待 ACK
    nTemp = I2C_GetACK();
    if(nTemp & BIT3)
    {
        UpInt();
        return -1;
    }
    //读取数据
    *dest = I2C_RxByte();
    // 停止总线
    I2C_STOP();
    _NOP();
    //成功返回
    UpInt();
    return 0;
}

int ROM_ReadBytes(long addr, char *dest ,int length)    //修改了addr的数据类型以满足addr长度要求
{                                                        //修改发送数据长度要求
    DownInt();                                           //修改为at24c1024b的时序/lshb 2019/08/27
    if(addr>524287)     
    { 
        UpInt();
        return -1;
    }
    if(length >524287)
    {
        UpInt();
        return -1;
    }
    if(length <1)
    {
        UpInt();
        return -1;
    }
    // 读 会 多发送一个伪字节 用来传送地址. 
    //构造地址
    //A2 A1 P0 高8位 低8位 共19位地址
    unsigned char hi= (addr >> 8) & 0xFF ;
    unsigned char lo= addr & 0xFF ;
    unsigned char nTemp = 0xA0 + ((addr / 0x10000) << 1)  ; //写命令 
  
    int i;
    
    // 启动数据总线
    I2C_START();
    // 发送控制字节
    I2C_TxByte(nTemp);
    // 等待 ACK
    nTemp = I2C_GetACK();
    if(nTemp & BIT3) 
    {
        UpInt();
        return -1;
    }
    //高位地址字节
    I2C_TxByte(hi);
    // 等待 ACK
    nTemp = I2C_GetACK();
    if(nTemp & BIT3) 
    {
        UpInt();
        return -1;
    }
    // 发送低地址字节
    I2C_TxByte(lo);
    // 等待 ACK
    nTemp = I2C_GetACK();
    if(nTemp & BIT3) 
    {
        UpInt();
        return -1;
    }
    
    // 启动数据总线
    I2C_START();
    // 发送控制字节
    nTemp = 0xA1 + ((addr / 0x10000) << 1);
    I2C_TxByte(nTemp);
    // 等待 ACK
    nTemp = I2C_GetACK();
    if(nTemp & BIT3) 
    {
        UpInt();
        return -1;
    }
    //读取数据
    for(i = 0; i < length-1; i++)
    {
        //读一个字节数据
        dest[i] = I2C_RxByte();
        //发送ACK
        I2C_SetACK();
    }
    dest[i] = I2C_RxByte(); 
  
    I2C_SetNAK();   
  
    // 停止总线
    I2C_STOP();
    //成功返回
    UpInt();
    return 0;
}




//   I2C 低级函数 
void I2C_Initial( void )        //修改函数使它和V3.0板的管脚相匹配/lshb 2019/08/27
{
    P3DIR |= SCL;		//将SCL管脚（P3.2)设置为输出管脚    
    I2C_Set_sck_low();
    I2C_STOP();     
    return;
}
void I2C_Set_sda_high( void )   //修改函数使它和V3.0板的管脚相匹配/lshb 2019/08/27
{
    P3DIR |= SDA;		//将SDA设置为输出模式
    P3OUT |= SDA;		//SDA管脚输出为高电平   
    _NOP();
    _NOP();
    return;
}
void I2C_Set_sda_low ( void )   //修改函数使它和V3.0板的管脚相匹配/lshb 2019/08/27
{
    P3DIR |= SDA;		//将SDA设置为输出模式
    P3OUT &= ~(SDA);		//SDA管脚输出为低电平
    _NOP();
    _NOP();
    return;
}
void I2C_Set_sck_high( void )   //修改函数使它和V3.0板的管脚相匹配/lshb 2019/08/27
{
    P3DIR |= SCL;		//将SCL设置为输出模式
    P3OUT |= SCL;		//SCL管脚输出为高电平
    _NOP();
    _NOP();
    return;
}
void I2C_Set_sck_low ( void )   //修改函数使它和V3.0板的管脚相匹配/lshb 2019/08/27
{
    P3DIR |= SCL;		//将SCL设置为输出模式
    P3OUT &= ~(SCL);		//SCL管脚输出为低电平
    _NOP();
    _NOP();
    return;
}
int  I2C_GetACK(void)   //修改函数使它和V3.0板的管脚相匹配/lshb 2019/08/27
{
    int nTemp = 0;
    int j; 
    _NOP();
    _NOP();
    I2C_Set_sck_low();
    for(j = 50;j > 0;j--);
    P3DIR &= ~(SDA);		//将SDA设置为输入方向
    //I2C_Set_sda_high();
    I2C_Set_sck_high();  
    for(j = 50;j > 0;j--);
    nTemp = (int)(P3IN & SDA);	//获得数据
    I2C_Set_sck_low();
    return (nTemp & SDA);
}
void I2C_SetACK(void)
{   
    I2C_Set_sck_low();
    I2C_Set_sda_low();
    I2C_Set_sck_high();
    I2C_Set_sck_low();
    return;
}

void I2C_SetNAK(void)
{
    I2C_Set_sck_low();
    I2C_Set_sda_high();
    I2C_Set_sck_high();
    I2C_Set_sck_low();
    return;
}
void I2C_START(void)
{
    int i;
    I2C_Set_sda_high();
    for(i = 15;i > 0;i--);
    I2C_Set_sck_high();
    for(i = 15;i > 0;i--);
    I2C_Set_sda_low();
    for(i = 15;i > 0;i--);
    I2C_Set_sck_low();
    return;
}
void I2C_STOP(void)
{
    int i;
    I2C_Set_sda_low();
    for(i = 5;i > 0;i--);
    ///I2C_Set_sck_low();
    ///for(i = 5;i > 0;i--);
    I2C_Set_sck_high();
    for(i = 5;i > 0;i--);
    I2C_Set_sda_high();
    for(i = 5;i > 0;i--);
    I2C_Set_sck_low();
    System_Delayms(10); //延迟一点时间
    return;
}
void I2C_TxByte(char nValue)
{
    //先发高字节
    int i;
    int j;
//    I2C_Set_sck_low
    for(i = 0;i < 8;i++)
    {
    	if(nValue & 0x80)
    	    I2C_Set_sda_high();
    	else
    	    I2C_Set_sda_low();
    	for(j = 30;j > 0;j--);
    	I2C_Set_sck_high();
    	nValue <<= 1;
    	for(j = 30;j > 0;j--);
    	I2C_Set_sck_low();
    }
    return;
}
/////////////////////////////////////////////
// 接收是从 LSB 到 MSB 的顺序
//
//
//    最后一个是 最低位
//
int  I2C_RxByte(void)           //修改函数使它和V3.0板的管脚相匹配/lshb 2019/08/27
{
  int nTemp = 0;
  int i;
  int j;
  I2C_Set_sda_high();  
  P3DIR &= ~(SDA);			//将SDA管脚设置为输入方向
  _NOP();
  _NOP();
  _NOP();
  _NOP(); 
  for(i = 0;i < 8;i++)
  {
    I2C_Set_sck_high();	
    if(P3IN & SDA)
    {
      nTemp |= (0x80 >> i);
    }
    for(j = 30;j > 0;j--);
    I2C_Set_sck_low();
  }
  return nTemp;
} 
