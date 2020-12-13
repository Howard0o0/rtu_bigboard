//////////////////////////
//作者：潘立航
//时间：19.10
//功能：拍摄、存储并发送图片
//////////////////////////

#include "camera.h"
#include "msp430common.h"
#include "common.h"
#include "uart2.h"
#include "uart0.h"
#include "uart3.h"
#include "string.h"
#include "flash.h"
#include "GTM900C.h"
#include "message.h"
#include "hydrologycommand.h"
#include "hydrologytask.h"
#include "rtc.h"

//摄像头指令中的常量
const static char reso = RESOLUTION1280_1024;             //分辨率

//接收摄像头返回的信息
char ack[4] = {0,};                                        //用于存储ack帧
char pic_info[10]={0,};                                    //用于存储返回的图片信息帧
char *pic_packet;                                         //用于存储图片数据帧中的数据
char device_id;

//发送图片报文用到的全局变量
uint8_t   pack_num_sd;                                      //发送图片的包总数 
char serial_num = 0x00;                                    //当前包序列号
char send_time[6] = {0,};                                  //发报时间
char shoot_time[5] = {0,};                                 //拍摄时间

extern void convertSampleTimetoHydrology(char* src,char* dst);
bool last_packet ;
cameratype camera_i, camera_ii;

//发送拍摄指令
void camera_TakeCmdSend(char device )
{
  uint16_t size = PAC_SIZE;
  char *pack_size;                                        //命令每包图片长度
  pack_size = (char*) (&size);
  char take_cmd [TAKECMD_LEN]= {0x55,0x48,0x00,0x00,0x00,0x00,0x23};
  
  take_cmd[2] = device;
  take_cmd[3] = reso;
  take_cmd[4] = pack_size [0];
  take_cmd[5] = pack_size [1];
  
  UART2_Send(take_cmd,TAKECMD_LEN,0);
  //TraceHexMsg(take_cmd,TAKECMD_LEN);
}
//发送取包指令
void camera_GetCmdSend(char *pack_cnt,char device )
{
  char get_cmd [GETCMD_LEN]= {0x55,0x45,0x00,0x00,0x00,0x23};
  get_cmd[2] = device;
  get_cmd[3] = pack_cnt [0];
  get_cmd[4] = pack_cnt [1];
  UART2_Send(get_cmd,GETCMD_LEN,0);
  //TraceHexMsg(get_cmd,GETCMD_LEN);
}
//读取ack
int camera_AckRead()
{
  int num=0;
  if (UART2_RecvLineLongWait(ack,ACK_LEN,&num)<0)
    return -1; 
  //TraceHexMsg(ack,ACK_LEN);
  return 0;
}
//读取图像信息数据包
int camera_InfoRead()
{
  int num=0;
  char pic_size[4] = {0,};                                   //用于存储图片信息帧中的图片大小
  cameratype *camera;
  
  if (UART2_RecvLineLongWait(pic_info,INFO_LEN,&num)<0)
    return -1;
  TraceHexMsg(pic_info,INFO_LEN);
  
  //从信息帧中获取图片相关信息
  pic_size[0] = pic_info[3];
  pic_size[1] = pic_info[4];
  pic_size[2] = pic_info[5];
  pic_size[3] = pic_info[6];
  
  device_id = pic_info[2];

  switch (device_id)
  {
    case 0 : {camera = &camera_i;break;}
    case 1 : {camera = &camera_ii;break;}
    default:{camera = &camera_i;break;}
  }
  for(int i = 3;i >= 0;i--)                             
  {//pic_size 数组转存到int变量里
    camera->picture_len <<=8;
    camera->picture_len += (uint8_t)pic_size[i];   
  }
  camera->last_len = camera->picture_len % PAC_SIZE;
  //获取分包数量
  camera->pack_num_rc = (uint8_t)pic_info[8];
  camera->pack_num_rc <<= 8;
  camera->pack_num_rc += (uint8_t)pic_info[7];
  
  switch (device_id)
  {
    case 0 : 
      {
        FLASH_EraseOneBank (BANK_D_START_ADDR);                       //擦除整个BANK D
        camera_ii.addr_r=camera_ii.addr_w += camera_i.picture_len;
        break;
      }
    case 1 : 
      { 
        long extra_bytes = camera_i.picture_len+camera_ii.picture_len-BANK_D_SIZE;
        if(extra_bytes > 0)
        {
          TraceMsg("BANK_D IS NOT BIG ENOUGH!",1);
          
          printf("picture1's length is %ldB",camera_i.picture_len);
          printf("picture2's length is %ldB",camera_ii.picture_len);
          for (int i=1;i<=extra_bytes;i+=64)
            FLASH_EraseOneSEG(BANK_D_END_ADDR+i);
        }
        break; 
      }
    default: ;
  }
    
  return 0;
}

//读取图像数据包
int camera_DatapackRead(char device)
{
  int num=0;
  int len=PAC_MAXSIZE;
  cameratype *camera;
  
  switch (device)
  {
    case 0 : {camera = &camera_i;break;}
    case 1 : {camera = &camera_ii;break;}
    default:{camera = &camera_i;break;}
  }
  if (last_packet) 
    len = camera->last_len+9;
  if (UART2_RecvLineLongWait(pic_packet,len,&num)<0)
    return -1;
  //TraceHexMsg(pic_packet,len);
  return len;
}

//发送拍摄指令，读取返回信息
int camera_PictureTake(char device)
{ 
  camera_TakeCmdSend(device);
  
  if (camera_AckRead()<0)return -1;
  if(ack[1] != 0x48) return -1;
  if (camera_InfoRead()<0)return -1;
  if(pic_info[1] != 0x52) return -1;
  
  return 0;
}

//获取一个分包并存储
int camera_PictureGet(unsigned long addr, char device)
{ 
  cameratype *camera;
  switch (device)
  {
    case 0x00 : {camera = &camera_i;break;}
    case 0x01 : {camera = &camera_ii;break;}
    default:{camera = &camera_i;break;}
  }
  int len;
  camera_GetCmdSend((char *)(&camera->count), device);
  
  if (camera_AckRead()<0) return -1;
  if(ack[1] != 0x45) return -1;
  
  len = camera_DatapackRead(device);
  if (len <0) return -1;                                                 //获取 pic_packet ，pic_bytes 值
  if (pic_packet[1] != 0x46 ) return -1;
  
  Write_flash_Segment (addr,&pic_packet[7],len-9);
  return 0;
}

//组包函数
//组装报文头部
int camera_PicturePacketHead(char *head,int pic_len,bool fstflg)        
{
  char frame_start[2] = {SOH1,SOH2};                                     //帧起始符
  uint16_t packetlen_int ;                                               //正文长度
  if (fstflg)                                                            //只有第一包正文长度有 PACKET_INFO_LEN
    packetlen_int = pic_len + PACKET_INFO_LEN +3;
  else
    packetlen_int = pic_len + 3;
  char *packet_len;                                                   
  char packet_start = SYN;                                              //报文起始符
  char pac_totalnum_serialnum[3];                                       //M3模式下的包总数及序列号                   
  
  packet_len = (char*)(&packetlen_int);                                 //正文长度
  
  pac_totalnum_serialnum[0] = (pack_num_sd & 0xf0)>>4;
  pac_totalnum_serialnum[1] = (pack_num_sd & 0x0f)<<4; 
  pac_totalnum_serialnum[2] = ++serial_num;
  
  int index = 0;
  head[index++] = frame_start[0];
  head[index++] = frame_start[1];
  
  if (Hydrology_ReadStoreInfo(HYDROLOGY_CENTER_ADDR,&head[index],HYDROLOGY_CENTER_LEN-3)<0)     //读中心站地址
    return -1;   
  index += (HYDROLOGY_CENTER_LEN-3);
  if (Hydrology_ReadStoreInfo(HYDROLOGY_REMOTE_ADDR,&head[index],HYDROLOGY_REMOTE_LEN)<0)       //读遥测站地址
    return -1;     
  index += HYDROLOGY_REMOTE_LEN;
  if (Hydrology_ReadStoreInfo(HYDROLOGY_PASSWORD_ADDR,&head[index],HYDROLOGY_PASSWORD_LEN)<0)   //读密码
    return -1; 
  index += HYDROLOGY_PASSWORD_LEN;
  head[index++] = Picture;                                                                      //图片功能码
  head[index++] = packet_len[1];
  head[index++] = packet_len[0];
  head[index++] = packet_start; 
  head[index++] = pac_totalnum_serialnum[0];
  head[index++] = pac_totalnum_serialnum[1];
  head[index++] = pac_totalnum_serialnum[2];
  return 0;
}
//组装正文信息
int camera_PicturePacketInfo(char *packet_info)
{
  char ser_num [2];                                                                     //流水号
  char remoteaddr_id = 0xF1;                                                            //遥测站地址标识符
  char observationtime_id = 0xF0;                                                       //观测时间要素标识符
  char picture_id = 0xF3;                                                               //图片要素标识符
  
  int index= 0;
  getstreamid(ser_num);                                                                 //生成流水号
  packet_info[index++] = ser_num[0];
  packet_info[index++] = ser_num[1];
  
  memcpy(&packet_info[index],send_time,6);                                              //组装发报时间
  index += 6;
  
  packet_info[index++] = remoteaddr_id;
  packet_info[index++] = remoteaddr_id;                                                 //遥测站地址标识符
  
  if (Hydrology_ReadStoreInfo(HYDROLOGY_REMOTE_ADDR,&packet_info[index],HYDROLOGY_REMOTE_LEN)<0) //读遥测站地址
    return -1;  
  index += HYDROLOGY_REMOTE_LEN;
  
  if (Hydrology_ReadStoreInfo(HYDROLOGY_RTUTYPE,&packet_info[index++],HYDROLOGY_RTUTYPE_LEN)<0) //读遥测站分类码
    return -1; 
  
  packet_info[index++] = observationtime_id;
  packet_info[index++] = observationtime_id;                                                    //观测时间要素标识符
  
  memcpy(&packet_info[index],shoot_time,5);                                                     //组装观测时间
  index += 5;
  
  packet_info[index++] = picture_id;
  packet_info[index++] = picture_id;                                                            //图片要素标识符
  return 0;
}

//组装正文
void camera_PicturePacketElement(long addr,int pic_len,char *element)//pic_len 是图片数据长度
{
  int i = 0;
  while (i < pic_len)
    element[i++] = Read_Flashw (addr++);                              //读图片要素 
}

//组装报文
int camera_PicturePacket(long addr,int pic_len,bool lstflg,bool fstflg)         //形参 pic_len 图片数据长度， lstflg 是否为最后一包标志
{
  //报文数组
  char head[PACKET_HEAD_LEN]={0,};                                  //报文头部
  char packet_info[PACKET_INFO_LEN]={0,};                           //正文头部
  char element[PACKET_ELEMENT_LEN]={0,};                            //要素
  char up_picture_packet[PACKET_PACKET_LEN];                        //报文
  
  char packet_end;                                                  //报文结束符
  short CRC;                                                        //校验码
  int index = 0;

  if (camera_PicturePacketHead(head,pic_len,fstflg)<0)              //组装报文头部
    return -1;               
  memcpy(up_picture_packet,head,PACKET_HEAD_LEN);
  index += PACKET_HEAD_LEN;
  if (fstflg)                                                       //只有第一包需要正文信息
  {
    if (camera_PicturePacketInfo(packet_info)<0)                    //组装正文头部
      return -1;
    memcpy(&up_picture_packet[index],packet_info,PACKET_INFO_LEN);
    index += PACKET_INFO_LEN;
    fstflg = false;                                                //第一包标志解除
  }
  
  camera_PicturePacketElement(addr,pic_len,element);               //组装图片要素
  
  memcpy(&up_picture_packet[index],element,pic_len);   
  index += pic_len;
  //结束符分两种
  if (lstflg) packet_end =ETX;                                    //最后一包的结束符
  else packet_end =ETB;                                           //表示后续还有包
  
  up_picture_packet[index++] = packet_end;
  CRC = hydrologyCRC16(up_picture_packet,index);
  
  up_picture_packet[index++] = CRC >> 8;
  up_picture_packet[index++] = CRC & 0xFF;
  
  //发送一帧报文
  int i = 2;
  while (GPRS_Send(up_picture_packet,index,0,HYDROLOGY_CENTER1_IP) == FALSE && i>0) i--;
  if (i<=0) return -1;
  
  TraceHexMsg(up_picture_packet,index);
  return 0;
}

//外部调用函数
//拍摄并存储图片
int camera_Take(char device)
{
  UART2_Open(1);
  //UART0_Open(1);
  char shoot_time_DEC[5] = {0,};
  RTC_ReadTimeBytes5(shoot_time_DEC);
  convertSampleTimetoHydrology(shoot_time_DEC,shoot_time);      //获取拍摄时间
  
  int i = 4;
  while (camera_PictureTake(device)<0 && i>0)
    i--;
  if (i==0) return -1;

  pic_packet = (char*)malloc(PAC_MAXSIZE); 
  
  cameratype *camera;
  switch (device)
  {
    case 0 : {camera = &camera_i;break;}
    case 1 : {camera = &camera_ii;break;}
    default:{camera = &camera_i;break;}
  }

  while (camera->count < camera->pack_num_rc) 
  { 
    camera->count++;
    if (camera->count >= camera->pack_num_rc) 
      last_packet = true;
    else
      last_packet = false;
    
    int j = 4;
    while (camera_PictureGet(camera->addr_w,device)<0 && j>0)
      j--;
    if (j==0)  
    {
      free(pic_packet);
      return -1;
    }
    camera->addr_w += PAC_SIZE;
  }                                 //当还有包没有获取时循环

  free(pic_packet);
  UART2_Close();
  return 0;
}

//发送图片报文
int camera_PacketSend(char device)
{
  Hydrology_ReadTime(send_time);                        //获取发报时间
  cameratype *camera;
  serial_num = 0;
  switch (device)
  {
    case 0x00 : {camera = &camera_i;break;}
    case 0x01 : {camera = &camera_ii;break;}
    default:{camera = &camera_i;break;}
  }
  const int rest_bytes = camera->picture_len % READ_LEN;       //最后一包数据的长度
  int cnt;
 
  pack_num_sd = camera->picture_len / READ_LEN;                 //报文数量
  cnt = pack_num_sd; 
  if(rest_bytes != 0) pack_num_sd++;                    //报文数量还要加上最后一包
  
  while (cnt-->0)
  {
   if (camera_PicturePacket(camera->addr_r,READ_LEN,false,camera->fstflg)<0) 
     return -1;  
   if(camera->fstflg) 
     camera->fstflg = false;
   camera->addr_r += READ_LEN;
  }
  if (camera_PicturePacket(camera->addr_r,rest_bytes,true,camera->fstflg)<0) 
   return -1;
  
  return 0;
}

//初始化全局变量（每次拍摄前都需要重置的一些变量）
void camera_Init()
{                                
  camera_i.count =camera_ii.count = 0;
  camera_i.fstflg = camera_ii.fstflg = true;
  camera_i.addr_r = camera_ii.addr_r = BANK_D_START_ADDR;
  camera_i.addr_w = camera_ii.addr_w = BANK_D_START_ADDR;
}

char camera_change(char device)
{
  switch (device)
  {
    case 0: 
    {
      P10OUT |= BIT2;
      break;
    }
    case 1: 
    {
      P10OUT &=~ BIT2;
      break;
    }
    default: 
    {
      P10OUT &=~ BIT2;
      break;
    }
  }
  return device;
}

void camera()
{
  camera_Init();
  
  if (camera_Take(camera_change(0))<0 || camera_Take(camera_change(1))<0) 
    TraceMsg("Camera Write Failed!",1);
  else 
    if (camera_PacketSend(0)<0 || camera_PacketSend(1)<0) 
      TraceMsg("Camera Send Failed!",1);
  System_Delayms(5000);                                  //延时5s
  GPRS_Close_GSM();                                      //发送完毕之后主动关闭GPRS
}