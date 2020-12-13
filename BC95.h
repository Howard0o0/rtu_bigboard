
/*
 * Author2: 		howard
 * changes: 		make connect to Iot cloud and build communication
 * instruction: 	UDP is valid when sim card is China-Net,NB module has to send&&receive messages via a cloud platform(Huawei Cloud)
 * How to drive: 	1.BC95_Open()  
 * 					2.BC95_ConfigProcess() => [config ip address and port in this function by calling BC95_ConnectToIotCloud(char *serverIp, char *serverPort); ]
 * 					3.BC95_SendSocketData() => send a message to cloud platform
 * 					4.
*/

#define BC95_SOCKET_DATA_LEN 500
#define BC95_SOCKET_UP_DATA_LENGTH_LINK 4   //这是BC95上传变长数据时需要在数据前面指定上传的数据长度

typedef enum tagBC95State
{
	BC95StateOK,
	BC95ErrorSIM,
	BC95ErrorCFUN,
	BC95ErrorCSQ,
	BC95ErrorCGATT,
	BC95ErrorCEREG,
	BC95ErrorCSCON,
	BC95ErrorCPSMS,
	BC95ErrorNNMI,
	BC95ErrorATE,
	BC95ErrorSEND,
	BC95ErrorNCDP,
	BC95ErrorCreatSocket,
	BC95ErrorNSMI
}BC95State;

/* Interfaces for upper application */
int BC95_Open();
void BC95_Close();
BC95State BC95_ConfigProcess();
int BC95_Send(char * data);  // send AT command to BC95
char *BC95_Receive(void); 	// receive AT command from BC95
BC95State BC95_SendDataToCloud(char *send_data, int send_data_len,int needCloudConfirm);
void BC95_RecvDataFromCloud(char *recv_data, int *recv_data_len);
void BC95_Test(void);
//int BC95_QueryTime(char *newtime);  //success return 0, fail return -1
int BC95_QueryTime(char *year,char *month,char *date,char *hour,char *min,char *second);  //success return 0, fail return -1



/*  underlying function */
BC95State BC95_SendSocketData(char *send_data, int send_data_len);
void BC95_RecvSocketData(char *recv_data, int *recv_data_len);
int BC95_Char_to_Hex(char chr);
int  BC95_UintToStr4 ( unsigned int _src, char* _dest );
char* BC95_QueryMsg(void);  //query Received msg state
void BC95_SetScramb();
void BC95_SetAutoConfig();
BC95State BC95_QuerySimState();
BC95State BC95_QueryFunState();
void BC95_SetCFun();
BC95State BC95_QueryCSQState();
BC95State BC95_QueryCGATTState();
BC95State BC95_SetNNMI();
BC95State BC95_QueryIsPSMS ();
void BC95_SetCpsms();
BC95State BC95_CloseATE();
void BC95_AnalysisRecvSocketData(char *revc_data, int revc_data_len, char *down_Stream_data, int *down_Stream_data_len);
BC95State BC95_SendAndRecvData(char *send_data, int send_data_len, char *recv_data, int *recv_data_len);
int BC95_Extract_DownData(char *inputData, int inputDataLen, char *outputData);
BC95State BC95_ConnectToIotCloud(char *serverIp, char *serverPort);
BC95State BC95_SendUdpMsg(char *ipAddr,char *port, char *msg);   // invalid when loaded with 电信卡
void BC95_AtTest(void);
char *makeUDPBody(char *socket,char *ipAddr,char *port,char *len,char *data);  // invalid when loaded with 电信卡

void BC95_Communication_test();












