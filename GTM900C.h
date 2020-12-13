#ifndef GTM900C_H_
#define GTM900C_H_





void GPRS_PrepareForSend();

int GPRS_AT_OFF_CALL();

int GRPS_AT_Receive();

int GPRS_QueryRemainData();

char* GPRS_Receive();

int GPRS_Send(char* pSend, int sendDataLen, int isLastPacket, int center);

int GPRS_Create_TCP_Link(int center);

int GPRS_Close_TCP_Link();

void GPRS_Close_GSM();

void JudgeServerDataArrived(void);

int Hydrology_ProcessGPRSReceieve();

extern char* _ReceiveData;
extern int _ReceiveDataLen;
extern int gprsConfigSuccess;
#endif /* GTM900C_H_ */