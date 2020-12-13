#include "main.h"
#include "BC95.h"
#include "DTU.h"
#include "Sampler.h"
#include "blueTooth.h"
#include "common.h"
#include "console.h"
#include "driverlib.h"
#include "hydrologycommand.h"
#include "hydrologytask.h"
#include "ioDev.h"
#include "led.h"
#include "lowpower.h"
#include "msp430common.h"
#include "reportbinding.h"
#include "rom.h"
#include "rtc.h"
#include "store.h"
#include "timer.h"
#include "uart0.h"
#include "uart1.h"
#include "uart3.h"
#include "uart_config.h"
#include "wifi_config.h"
#include <string.h>

extern bool Is_uart3_RX_INT;  //增加 LSHB200508
int	    Opened = 1;	      // Opened控制打印信息开关 LSHB200510

int IsDebug = 0;

bool Is_RS485_1 = false, Is_bluetooth = false,
     Is_uart1_RX_INT =
	     false;  //工作指示灯控制，三种情况(1)当前UART1是否是485，（2）当前UART1是否是蓝牙，（3）是否发生了uart1中断
		     // LSHB 20200506

int main(void) {

	Restart_Init();
	Led_Round();
	Select_Debug_Mode(0);
	TraceOpen();
	TraceMsg("Device Open !", 1);
	Main_Init();

	/*++++++++Test+++++++*/
	// GSM_Open();
	// char newtime[ 6 ] = { 0 };
	// GSM_AT_QueryTime(&newtime[ 0 ], &newtime[ 1 ], &newtime[ 2 ], &newtime[ 3 ], &newtime[ 4 ],
	// 		 &newtime[ 5 ]);
	// System_Delayms(50);
	// printf("update rtc : %d-%d-%d %d:%d:%d \r\n\r\n", newtime[ 0 ], newtime[ 1 ], newtime[ 2 ],
	//        newtime[ 3 ], newtime[ 4 ], newtime[ 5 ]);
	// _RTC_SetTime(newtime[ 5 ], newtime[ 4 ], newtime[ 3 ], newtime[ 2 ], newtime[ 1 ], 1, newtime[ 0 ],
	// 	     0);
	LowpowerTest();
	/*--------Test-------*/

	/*                                                            //地下水程序中关闭蓝牙  LSHB 200510
	 BleDriverInstall();
	 printf("\r\nBLE driver installed \r\n");
	 PT_IODev  ptDevBle =  getIODev();
	 printf("BLE is waiting for matched in 30s ...\r\n");
	 ptDevBle->init();
	 ptDevBle->bletaskstart();
	*/

	// Sampler_Open();

	Hydrology_InitWaitConfig();
	System_Delayus(300);
	HydrologyUpdateElementTable();	//更新要素表
	HydrologyDataPacketInit();	//根据要素表获得DP的大小和初始化

	Opened = 1;  // 0:关闭打印信息 ,1：打开打印信息   LSHB200510

#if 0
    Hydrology_SetStartIdx(1);  //清除发送和存储标记位
    Hydrology_SetEndIdx(1);
    Hydrology_SetDataPacketCount(0);
#endif

	/*                                                     //地下水程序中关闭蓝牙  LSHB 200510
	  ptDevBle->bletaskend();
	 */
	while (1) {
		//  ptDevBle->bletaskstart();
		if (HydrologyTask() == 0)
			gotoSleepLPM3(4, INTERVAL_HOUR);
		// ptDevBle->bletaskend();
	}
}

void Restart_Init() {

	P1SEL = 0x00;  //关闭脉冲端口中断
	P1DIR = 0x00;
	Clock_Init();  // CPU时钟初始化

	_EINT();  //打开中断
	// IE1|=OFIE+NMIIE+ACCVIE;     //振荡器故障中断允许、NMI中断允许、FLASH存储器非法访问中断允许
	SFRIE1 |= OFIE + NMIIE + ACCVIE;
	WatchDog_Init();
	Led_Init();  // 指示灯 初始化
	Led_OffAll();

	//    P9DIR |= BIT7;              //ly 拉高232芯片控制引脚
	//    P9OUT |= BIT7;

	Timer0_A0_Init(1024);	//系统运行状态指示灯控制   LSHB 20200506
	Timer1_A0_Init(30720);	// 系统定时器初始化
	TimerB_Init(61440);

	Store_Init();  // 初始化ROM
	RTC_Open();
	Sampler_Init();	 //端口初始化,需要先初始化Store

	P10DIR |= BIT0;	 // ly P100拉高，uart3用于调试，低的话P104，,105就是485口
	P10OUT |= BIT0;

	P10DIR |= BIT1;	 // ly P101拉高，uart1 P104，,105，低的话就是485口,高的话就是蓝牙
	P10OUT &= ~BIT1;
	Is_RS485_1 = true;  //现在把uart1暂时直接定义为485-1，以后使用蓝牙双功能时再改    LSHB 20200506
	P11DIR |= BIT2;	 //工作指示灯控制P11.2，1：系统工作正常，0：系统发生错误，相关指示灯闪烁

	P9DIR |= BIT6;	//初始化硬件看门狗   LSHB 20200510
	P9OUT |= BIT6;
	P9OUT &= ~BIT3;	 //关闭24V电源

	Led1_WorkLightOff();  //为了节能关闭工作指示灯

	Console_Open();

	/*wyq  调试485去掉联网的步骤*/
	//     P3DIR &= ~BIT2;
	//    if(P3IN & BIT2)
	//    {
	//        IsDebug = 1;
	//    }
	//    else
	//    {
	//        IsDebug = 0;
	//    }

	return;
}

int Restart_GSMInit() {
	GSM_Open();
	if (GSM_CheckOK() < 0) {
		System_Delayms(1000);
		GSM_Open();
		if (GSM_CheckOK() < 0) {
			System_Delayms(1000);
			GSM_Open();
			if (GSM_CheckOK() < 0) {  //无法工作就放弃了.
				GSM_Close(1);
				return -1;
			}
		}
	}

	char _phone[ 12 ];
	char _data[ 30 ];  // 30足够了

	_data[ 0 ] = '$';
	if (Store_ReadDeviceNO(&_data[ 1 ]) < 0) {
		_data[ 1 ]  = '0';
		_data[ 2 ]  = '0';
		_data[ 3 ]  = '0';
		_data[ 4 ]  = '0';
		_data[ 5 ]  = '0';
		_data[ 6 ]  = '0';
		_data[ 7 ]  = '0';
		_data[ 8 ]  = '0';
		_data[ 9 ]  = '0';
		_data[ 10 ] = '0';
		_data[ 11 ] = '0';
	}

	Utility_Strncpy(&_data[ 12 ], "<restart#", 9);
	if (Store_GSM_ReadCenterPhone(5, _phone) == 0) {
		if (_phone[ 0 ] != '0' && _phone[ 1 ] != '0' && _phone[ 2 ] != '0') {
			GSM_SendMsgTxt(_phone, _data, 21);
		}
	}
	GSM_Process(1, 0);     // GSM 主流程
	GSM_Close(0);	       //关闭 GSM
	System_Delayms(2000);  //避免关机后过快开机导致失败.
	return 0;
}

int Restart_DTUInit() {
	char _data[ UART3_MAXBUFFLEN ];
	int  _dataLen = 0;
	int  _repeats = 0;
	int  _out     = 0;
	//
	//这里增加一个DTU等待串口配置的过程.
	//
	if (trace_open == 0) {
		//如果没打开调试的话,串口是要额外打开的
		// Console_Open();
	}
	UART3_ClearBuffer();
	// Console_WriteStringln("ACK");

	if (UART3_RecvLineLongWait(_data, UART3_MAXBUFFLEN, &_dataLen) == 0) {
		//如果等到串口数据, 就进入配置状态.等待15秒钟
		// Console_WriteStringln("waiting for 15 seconds .");
		if (Main_ProcCommand(_data, _dataLen, NULL) == 3) {  //返回了一个SYN,就进入配置状态
			while (1) {
				_repeats = 0;
				while (UART3_RecvLineLongWait(_data, UART3_MAXBUFFLEN, &_dataLen) < 0) {
					++_repeats;
					if (_repeats > 4) {
						_out = 1;
						break;
					}
				}
				if (_out != 0)
					break;
				if (Main_ProcCommand(_data, _dataLen, NULL) == 2) {  //表示退出配置状态
					break;
				}
			}
		}
	}
	//当不在调试状态的时候要关闭的
	if (trace_open == 0) {	//如果调试是关闭的,串口到此要关闭
				// Console_Close();
	}

	return 0;
}

int WorkMode_Init(char* ptype) {
	char _curType, _selType;
	int  _ret;

#if 0
    //获取选择工作状态    DTU/GSM   232 
    P3DIR &= ~BIT3;   //P33为输入
    P3DIR &= ~BIT2;   //P32为输入
    if(P3IN & BIT2)
    {//P32为高
        _selType='D';
    }
    else
    {
        _selType='G';
    }
#endif
	_selType = 'S';	 // GPRS模式 GTM900
	switch (_selType) {
	case 'G':
		g_main_type = MAIN_TYPE_GSM;
		TraceMsg("Device is GSM Mode !", 1);
		break;
	case 'D':
		g_main_type = MAIN_TYPE_DTU;
		TraceMsg("Device is DTU Mode !", 1);
		break;

	case 'S':
		g_main_type = MAIN_TYPE_GTM900;
		TraceMsg("Device is GPRS Mode !", 1);
		break;

	default:
		//这里出现问题.就重启

		TraceMsg("Bad Mode !", 1);
		System_Reset();
		break;
	}

	//判断当前工作状态,   DTU/GSM
	if (Store_ReadSystemType(&_curType) < 0) {  //如果无法读出 保存的模式,就不判断是否 重新设置了.
		_ret = 0;
	}
	else {
		_ret = _selType != _curType ? 1 : 0;
	}

	*ptype = _selType;

	return _ret;
}

void Main_Init() {  //时钟使用8M
	//开中断,以防之前部分有错误导致中断被关闭
	EnInt();
	TimerB_Clear();
	WatchDog_Clear();  // 清除复位计数器
	//在里再次运行这个函数  增加可靠性.

	TraceOpen();
	RTC_Open();  // 打开RTC

	Sampler_Init();
}

void Main_GotoSleep() {
	//开中断,以防之前部分有错误导致中断被关闭
	EnInt();

	TraceMsg("Device is going to sleep !", 1);
	//关闭外围,进入休眠
	// RTC_Close();
	Console_Close();
	//    DTU_Close();
	//    GSM_Close(1);
	//    call gprs close
	Sampler_Close();
	TimerB_Clear();
	WatchDog_Clear();
	// TraceMsg("Device sleep !",1); //此处串口已经关闭
	LPM2;
	//    LPM3;
}
