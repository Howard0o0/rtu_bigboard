#include "lowpower.h"
#include "common.h"
#include "driverlib.h"
#include "hydrologycommand.h"
#include "hydrologytask.h"
#include "main.h"
#include "msp430common.h"
#include "rom.h"
#include "rtc.h"

int g_sleep_interval_type = INTERVAL_MIN;
int g_sleep_interval	  = 5;

/*
  intervalType : INTERVAL_HOUR , INTERVAL_MIN
  return : 0(success)  else(failed)
*/
static int SetWakeupInterval(const char interval, const char intervalType) {
	clearAlarmAFlag();
	char nextHour, nextMin;
	char currHour = _BCDtoHEX(_RTC_Read_OneByte(RegAddr_Hour));
	char currMin  = _BCDtoHEX(_RTC_Read_OneByte(RegAddr_Min));
	if (currHour < 0 || currHour >= 24) {
		printf("currHour error : %d:%d", currHour, currMin);
		return -1;
	}

	if (currMin < 0 || currMin >= 60) {
		printf("currMin error : %d:%d", currHour, currMin);
		return -2;
	}

	if (intervalType == INTERVAL_HOUR) {
		if (interval < 0 || interval >= 24) {
			printf("interval error : %d \n", interval);
			return -3;
		}
		char carry = currMin >= 59 ? 1 : 0;
		nextHour   = (((currHour + carry) / interval + 1) * interval - 1) % 24;
		nextMin	   = 59;
	}
	else {
		if (interval < 0 || interval >= 60) {
			printf("interval error : %d \n", interval);
			return -4;
		}
		char carry = (currMin + 1) % interval == 0 ? 1 : 0;
		nextMin	   = (((currMin + carry) / interval + 1) * interval - 1) % 60;
		nextHour   = (currHour + ((((currMin + carry) / interval + 1) * interval - 1) / 60)) % 24;
	}

	RTC_SetAlarm(nextHour, nextMin, 0);
	printf("next wakeup time: %d:%d:00 \n", nextHour, nextMin);

	// Enable P2.0 internal resistance as pull-Up resistance
	GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P2, GPIO_PIN2);
	// P2.0 interrupt enabled
	GPIO_enableInterrupt(GPIO_PORT_P2, GPIO_PIN0);
	// P2.0 Hi/Lo edge
	GPIO_selectInterruptEdge(GPIO_PORT_P2, GPIO_PIN0, GPIO_HIGH_TO_LOW_TRANSITION);
	// P2.0 IFG cleared
	GPIO_clearInterrupt(GPIO_PORT_P2, GPIO_PIN0);

	return 0;
}
void gotoSleepLPM3() {

	if (SetWakeupInterval(g_sleep_interval, g_sleep_interval_type)
	    != 0)  // SetWakeupInterval(10, INTERVAL_MIN)表示10分钟唤醒一次，setWakeupInterval(4,
		   // INTERVAL_HOUR)表示4小时唤醒一次
		return;
	printf("going to sleep \r\n ");
	__bis_SR_register(LPM3_bits + GIE);
	printf("wake up!\r\n\r\n");
}

void rtcIntPinInit() {
	// Enable P2.0 internal resistance as pull-Up resistance
	GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P2, GPIO_PIN2);
	// P2.0 interrupt enabled
	GPIO_enableInterrupt(GPIO_PORT_P2, GPIO_PIN0);
	// P2.0 Hi/Lo edge
	GPIO_selectInterruptEdge(GPIO_PORT_P2, GPIO_PIN0, GPIO_HIGH_TO_LOW_TRANSITION);
	// P2.0 IFG cleared
	GPIO_clearInterrupt(GPIO_PORT_P2, GPIO_PIN0);
}

#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector = PORT2_VECTOR  // P20 用于外部时钟唤醒中断
__interrupt
#elif defined(__GNUC__)
__attribute__((interrupt(PORT2_VECTOR)))
#endif
	void
	Port_2(void) {
	if (GPIO_getInterruptStatus(GPIO_PORT_P2, GPIO_PIN0) != 0) {
		// P2.0 IFG cleared
		GPIO_clearInterrupt(GPIO_PORT_P2, GPIO_PIN0);
		// wakeup cpu
		__bic_SR_register_on_exit(SCG1 + SCG0 + OSCOFF + CPUOFF);

		// printf("wake up!\r\n");
	}
}

static void CalcNextWakeupTime(char* nextHour, char* nextMin) {

	char interval	  = g_sleep_interval;
	char intervalType = g_sleep_interval_type;
	char currHour	  = _BCDtoHEX(_RTC_Read_OneByte(RegAddr_Hour));
	char currMin	  = _BCDtoHEX(_RTC_Read_OneByte(RegAddr_Min));
	if (currHour < 0 || currHour >= 24) {
		printf("currHour error : %d:%d", currHour, currMin);
		return;
	}

	if (currMin < 0 || currMin >= 60) {
		printf("currMin error : %d:%d", currHour, currMin);
		return;
	}

	if (intervalType == INTERVAL_HOUR) {
		if (interval < 0 || interval >= 24) {
			printf("interval error : %d \n", interval);
			return;
		}
		char carry = currMin >= 59 ? 1 : 0;
		*nextHour  = (((currHour + carry) / interval + 1) * interval - 1) % 24;
		*nextMin   = 59;
	}
	else {
		if (interval < 0 || interval >= 60) {
			printf("interval error : %d \n", interval);
			return;
		}
		char carry = (currMin + 1) % interval == 0 ? 1 : 0;
		*nextMin   = (((currMin + carry) / interval + 1) * interval - 1) % 60;
		*nextHour  = (currHour + ((((currMin + carry) / interval + 1) * interval - 1) / 60)) % 24;
	}
}
void SleepIfWakeupAcceidently() {

	char rtc_nowTime[ 6 ];
	char date, control;
	_RTC_ReadTime(&rtc_nowTime[ 5 ], &rtc_nowTime[ 4 ], &rtc_nowTime[ 3 ], &rtc_nowTime[ 2 ],
		      &rtc_nowTime[ 1 ], &date, &rtc_nowTime[ 0 ], &control);
	char curr_hour = rtc_nowTime[ 3 ];
	char curr_min  = rtc_nowTime[ 4 ];

	printf("curr time : %d:%d \r\n", curr_hour, curr_min);

	/* 如果是正常唤醒时间，直接返回，继续往下走 */
	if (g_sleep_interval_type == INTERVAL_MIN && (curr_min + 1) % g_sleep_interval == 0)
		return;
	if (g_sleep_interval_type == INTERVAL_HOUR && (curr_hour + 1) % g_sleep_interval == 0
	    && curr_min == 59)
		return;

	char wakeup_hour, wakeup_min;
	CalcNextWakeupTime(&wakeup_hour, &wakeup_min);

	/* 如果是当前时间是异常唤醒，计算下距离唤醒时间还有多久，如果超过3分钟就继续休眠
	 */
	if (curr_hour <= wakeup_hour && curr_min < wakeup_min && !IsDebug
	    && (wakeup_hour - curr_hour) * 60 + wakeup_min - curr_min >= 3) {
		printf("accident wakeup, keep sleeping...\r\n");
		gotoSleepLPM3();
	}
}

void SetSleepInterval(int interval, int interval_type) {
	if (interval_type == INTERVAL_HOUR && (interval < 0 || interval >= 24)) {
		printf("invalid interval %d, interval hour range in [1,23] \r\n\r\n", interval);
		return;
	}
	if (interval_type == INTERVAL_MIN && (interval < 0 || interval >= 60)) {
		printf("invalid interval %d, interval min range in [1,59] \r\n\r\n", interval);
		return;
	}

	g_sleep_interval      = interval;
	g_sleep_interval_type = interval_type;
}

/*+++++++++++++++Test++++++++++++++++*/

void LowpowerTest() {
	SetSleepInterval(24, INTERVAL_HOUR);
	SleepIfWakeupAcceidently();
	while (1) {

		printf("work 1...\r\n");
		System_Delayms(1000);
		printf("work 2...\r\n");
		System_Delayms(1000);
		printf("work 3...\r\n");
		System_Delayms(1000);

		gotoSleepLPM3();
	}
}

/*---------------Test----------------*/
