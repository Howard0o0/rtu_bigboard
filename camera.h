#pragma once 
#include "stdbool.h"
#include "stdint.h"

#define PAC_SIZE                512                     //要求摄像头返回分包图片大小
#define PAC_MAXSIZE             PAC_SIZE+9              //图像数据包的最大容量，加上2B的校验和字段
//#define PICTURE_START_ADDR      (0x40000 - 0xc800)    //图片存储的起始flash地址，0x33800
#define READ_LEN                512                     //一次读取的长度

#define TAKECMD_LEN             7
#define GETCMD_LEN              6
#define ACK_LEN                 4
#define INFO_LEN                10

#define PACKET_HEAD_LEN         17 
#define PACKET_INFO_LEN         25
#define PACKET_ELEMENT_LEN      READ_LEN
#define PACKET_PACKET_LEN       PACKET_HEAD_LEN + PACKET_INFO_LEN + PACKET_ELEMENT_LEN +3

//分辨率
#define RESOLUTION160_128       0x31    
#define RESOLUTION320_240       0x32
#define RESOLUTION640_480       0x33
#define RESOLUTION1280_1024     0x34

typedef struct camera_info
{
  uint16_t count;                           //获取分包的计数
  uint32_t picture_len;                     //图片的总长度
  uint16_t last_len ;                       //从摄像头接收的最后一包数据长度
  uint16_t pack_num_rc ;                    //从摄像头接收的分包数量
  bool fstflg ;                      //是否是发送的第一包
  unsigned long int addr_w ;              //写地址
  unsigned long int addr_r ;              //读地址
}cameratype;

//外部接口 message.c \ hydrologyProcessSend 中调用
int camera_Take(char device);
int camera_PacketSend(char device);
void camera_Init();
char camera_change(char device);
void camera();
extern void getstreamid(char streamid[2]);
//extern int Hydrology_SendData(char* data,int len);
