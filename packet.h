#pragma once

#define SAMPLETIMERINTERVAL 1

//typedef struct tagpacket
//{
//    void* header;//
//    void* body;//
//    char end[2];//
//    char crc[4];//
//    int len;//
//}packet;

//typedef int (*pfuncPacket)();
//typedef int (*pfuncEncapPacket)(packet* buffer,char* output,int outputlen);
//
//typedef struct tagpacketHander
//{
//    pfuncPacket pmakeHeader;//
//    pfuncPacket pmakeBody;//
//    pfuncPacket pmakeEnd;//
//    pfuncEncapPacket pencapPacket;//
//    pfuncPacket pdecapPacket;//
//}packetHander;

//short CRC16_2(char *puchMsg, int usDataLen);
//short CRC16_1(char* pchMsg, int wDataLen);
float ConvertVoltage(int v);
double ConvertAnaolgy(int v,char* range);
//void getBCDnums(double num, int * intergerpart, int* decimerpart, int d, char* pout_intergerValue, char * pout_decimerValue);
//int  getEvenNum(int num);


