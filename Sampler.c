//////////////////////////////////////////////////////
//     文件名: Sampler.c
//   文件版本: 1.0.0
//   创建时间: 09年 11月30日
//   更新内容:
//       作者: 林智
//       附注: 无
//
//////////////////////////////////////////////////////

#include "msp430common.h"

#include "Sampler.h"
#include "Store.h"
#include "adc.h"
#include "common.h"
#include "hydrologycommand.h"
#include "message.h"
#include "rtc.h"
#include "stdint.h"

// static int s_pulse1_flag=0;
// static int s_pulse1_flag1=0;
// static int s_pulse1_flag2=0;

// static int s_pulse2_flag=0;
// static int s_pulse3_flag=0;
// static int s_pulse4_flag=0;

// static unsigned int  s_pulse1_num=0; //计数
// static unsigned int  s_pulse2_num=0;
// static unsigned int  s_pulse3_num=0;
// static unsigned int  s_pulse4_num=0;

char g_pulse_rate[ 4 ] = { 0, 0, 0, 0 };  //  满计数
char g_pulse_range[ 4 ][ 3 ];

//复警告标记 ,  不会重复警告.以及抖动警告的出现
int s_alert_flag[ 8 ];

char alert_str[ 60 ];  // A,0000,B,0000,C,0000,D,0000,E,0000,F,0000,G,0000,H,0000
int  alert_str_idx = 0;

int Sampler_Init() {  //对脉冲和开关量的端口进行初始化.

	//脉冲为P2 -->P1    ly
	//开关为P5 -->P4
	P1SEL = 0x00;
	P1DIR = 0x00;
	P4SEL = 0x00;

	char _temp = 0x00;

	//  读取IO方向设置
	if (Store_ReadIoDirConfig(&_temp) >= 0)
		P4DIR = _temp;
	printf("");
	//  读取IO电平设置
	if (Store_ReadIoLevelConfig(&_temp) >= 0)
		P4OUT = _temp;

	//重启会使值丢失,我们这里复制出来.

	//  脉冲频率
	Store_ReadPulseRate(1, &(g_pulse_rate[ 0 ]));
	Store_ReadPulseRate(2, &(g_pulse_rate[ 1 ]));
	Store_ReadPulseRate(3, &(g_pulse_rate[ 2 ]));
	Store_ReadPulseRate(4, &(g_pulse_rate[ 3 ]));

	//  脉冲最大值
	Store_ReadPulseRangeBytes(1, g_pulse_range[ 0 ]);
	Store_ReadPulseRangeBytes(2, g_pulse_range[ 1 ]);
	Store_ReadPulseRangeBytes(3, g_pulse_range[ 2 ]);
	Store_ReadPulseRangeBytes(4, g_pulse_range[ 3 ]);
	RTC_RetrievePulseBytes();  //提取脉冲值

	// 2418 P2设置,P2.3 开启中断 放在RTC准备好了之后 . P2.0-2.4中断flag high-to-low transition
	// 5438 P1设置  P1.3                                P1.0-1.4
	P1IE  = 0xFF;  //中断使能
	P1IES = 0xFF;  //下降沿

	return 0;
}

//控制8个IO口的电平,必须设置为输出. 起始为1
int Sampler_IO_Level(int _ioIdx, int _level) {
	if (_ioIdx < 1 || _ioIdx > 8)
		return -3;
	//读取IO配置
	char _temp1 = 0x00;
	char _temp2 = 0x00;
	char _temp3 = 0x00;
	if (Store_ReadIoDirConfig(&_temp1) < 0) {  //如果无法读出就都默认为输出好了.
		//让他成功.
		_temp1 = 0xFF;
	}
	--_ioIdx;
	_temp2 = (0x01 << _ioIdx);
	if (_temp2 & _temp1 == 0x00) {	//该位不为输出
		return -2;
	}

	//该设置要保存起来
	if (Store_ReadIoLevelConfig(&_temp3) < 0)
		return -1;
	if (_level)
		_temp3 |= _temp2;
	else
		_temp3 &= ~_temp2;
	if (Store_SetIoLevelConfig(_temp3) < 0)
		return -1;

	P4DIR |= _temp2;
	//驱动IO口
	if (_level)  //对该位进行设置
		P4OUT |= _temp2;
	else
		P4OUT &= ~_temp2;
	return 0;
}

int Sampler_Open() {
	//打开传感器电源
	P9DIR |= BIT3;
	P9OUT |= BIT3;

	//电压产生
	System_Delayms(500);
	ADC_Open();  //
	return 0;
}

int Sampler_Close() {
	ADC_Close();
	P9DIR |= BIT3;
	P9OUT &= ~BIT3;
	return 0;
}

int Sampler_Sample() {
	ADC_Sample();
	return 0;
}

int Sampler_GSM_ReadAlertString(char* _dest) {	//出错就返回-1
	int  _idx	= 0;
	char _tempChar1 = 0x00;
	char _tempChar2 = 0x00;
	char _buffer[ 3 ];
	if (Store_ReadAnalogSelect(&_tempChar1) < 0)
		return -1;
	//添加模拟量
	for (int i = 0; i < 8; ++i) {
		if (_tempChar1 & 0x01) {  //如果该位有
			//则添上类型码和数据
			_dest[ _idx ] = 'A' + i;
			ADC_ReadAnalogStr(i + 1, &(_dest[ ++_idx ]));
			_idx += 4;
		}
		//然后是下一个
		_tempChar1 >>= 1;  //左移一位
	}
	//添加脉冲量
	if (Store_ReadPulseSelect(&_tempChar1) < 0)
		return -1;
	for (int i = 0; i < 4; ++i) {
		if (_tempChar1 & 0x80) {  //如果该位有才添加该位
			_dest[ _idx++ ] = 'I' + i;
			RTC_ReadPulseBytes(i + 1, _buffer);
			Utility_CharToHex(_buffer[ 0 ], &(_dest[ _idx ]));
			_idx += 2;
			Utility_CharToHex(_buffer[ 1 ], &(_dest[ _idx ]));
			_idx += 2;
			Utility_CharToHex(_buffer[ 2 ], &(_dest[ _idx ]));
			_idx += 2;
		}
		_tempChar1 <<= 1;
	}
	//开关量
	_tempChar1 = 0x01;
	//_tempChar2装开关量的配置
	if (Store_ReadIoSelect(&_tempChar2) < 0)
		return -1;
	for (int i = 0; i < 8; ++i) {	  //对于8个位
		if (_tempChar2 & 0x01) {  //为1的位要记录0或1
			_dest[ _idx++ ] = 'M' + i;
			if (P4IN & _tempChar1) {
				_dest[ _idx++ ] = '1';
			}
			else {
				_dest[ _idx++ ] = '0';
			}
		}
		//判断下一个
		_tempChar1 >>= 1;
		_tempChar2 >>= 1;
	}
	//结束  对于#号,发送程序自己加
	return _idx;
}

int Sampler_DTU_ReadAlertString(char* _dest) {
	Utility_Strncpy(_dest, alert_str, alert_str_idx);
	return alert_str_idx;
}

// 0123456798901234567890123456789012345678901234567890123456789
// 3:A=1024;C=1022;H=3333;
//返回1表示 要警告
//对于未选择的通道 则不管它
int Sampler_CheckNormal() {
	int  _max	= 4096;	 //上限变量
	int  _min	= 0;	 //下限变量
	char _tempChar1 = 0x00;	 //临时变量
	int  _tempInt	= 0;	 //临时变量

	int _need_alert = 0;  //是否需要报警.

	alert_str_idx		     = 1;  //重新填写alert_str;
	alert_str[ alert_str_idx++ ] = ':';
	char _alert_num		     = 0;  //用来指示报警数量.

	if (Store_ReadAnalogSelect(&_tempChar1) < 0) {	//无法读出选择串,就当都选择了.
		_tempChar1 = 0xFF;
	}
	//下面依次检查各个通道
	for (int i = 0; i < 8; ++i) {
		_tempInt = _tempChar1 & (0x01 << i);
		if (_tempInt == 0) {  //这个通道未被使用
			continue;
		}
		if (Store_ReadDataMinInt(i + 1, &_min) < 0)
			_min = 0;
		if (Store_ReadDataMaxInt(i + 1, &_max) < 0)
			_max = 4096;

		if (A[ i ] < _min || A[ i ] > _max) {  //只要超过范围都会在报警信息里留下记录
			//生成该通道上的报警条目,
			++_alert_num;
			alert_str[ alert_str_idx++ ] = 'A' + i;
			alert_str[ alert_str_idx++ ] = '=';
			_tempInt		     = A[ i ];
			Utility_UintToStr4(_tempInt, &alert_str[ alert_str_idx ]);
			alert_str_idx += 4;
			alert_str[ alert_str_idx++ ] = ';';  //以一个,号进行分隔
							     //完成这部分的填写
		}
		//下面关于记号和警报
		//如果这一路 警告过了.
		//判断是否需要解除 重复警告标记
		if (s_alert_flag[ i ] > 0) {
			//下面遇到了 隐式类型转换的麻烦. , 做为一个经验!
			//( A[i] < _max-100)  当 _max-100<0的时候 ,无符号的A[i]将产生麻烦
			if ((_max - 100 > 0 && A[ i ] < _max - 100) || (_max - 100 <= 0 && A[ i ] < _max)) {
				TraceMsg("A[i] = ", 0);
				TraceInt4(A[ i ], 1);
				TraceMsg("max = ", 0);
				TraceInt4(_max, 1);
				TraceMsg("remove alert flag for max-limited.", 1);
				s_alert_flag[ i ] = 0;
			}
			continue;
		}

		if (s_alert_flag[ i ] < 0) {
			if ((_min + 100 < 4096 && A[ i ] > _min + 100)
			    || (_min + 100 >= 4096 && A[ i ] > _min)) {
				TraceMsg("A[i] = ", 0);
				TraceInt4(A[ i ], 1);
				TraceMsg("min = ", 0);
				TraceInt4(_min, 1);
				TraceMsg("remove alert flag for min-limited.", 1);
				s_alert_flag[ i ] = 0;
			}
			continue;
		}

		if (A[ i ] < _min) {
			s_alert_flag[ i ] = -1;	 //设置标记,1表示曾经因为太小而被报警
			_need_alert	  = 1;
		}
		if (A[ i ] > _max) {
			s_alert_flag[ i ] = 1;	//设置标记
			_need_alert	  = 1;
		}
	}
	//然后更新报警数目字符.
	alert_str[ 0 ] = '0' + _alert_num;
	if (_need_alert)
		return 1;
	else
		return 0;
}

//  保存内容为
//  校验字节 0909011230字节A1字节A2字节B1字节B2字节..字节I1字节I2...开关字节
//  如下:
//  0           1          2          3
//  0  1234567890 1234567890123456 789012345678  9
//  校 0909011230 AABBCCDDEEFFGGHH IIIJJJKKKLLL 开关
//
int Sampler_SaveData(char* _saveTime) {
	char _data[ 40 ];    //数据条为40
	_data[ 0 ]  = 0x00;  // 已发送标记 记为0x00 ,
	_data[ 1 ]  = _saveTime[ 0 ] / 10 + '0';
	_data[ 2 ]  = _saveTime[ 0 ] % 10 + '0';
	_data[ 3 ]  = _saveTime[ 1 ] / 10 + '0';
	_data[ 4 ]  = _saveTime[ 1 ] % 10 + '0';
	_data[ 5 ]  = _saveTime[ 2 ] / 10 + '0';
	_data[ 6 ]  = _saveTime[ 2 ] % 10 + '0';
	_data[ 7 ]  = _saveTime[ 3 ] / 10 + '0';
	_data[ 8 ]  = _saveTime[ 3 ] % 10 + '0';
	_data[ 9 ]  = _saveTime[ 4 ] / 10 + '0';
	_data[ 10 ] = _saveTime[ 4 ] % 10 + '0';

	_data[ 11 ] = A[ 0 ] >> 8;
	_data[ 12 ] = A[ 0 ] & 0x00FF;
	_data[ 13 ] = A[ 1 ] >> 8;
	_data[ 14 ] = A[ 1 ] & 0x00FF;
	_data[ 15 ] = A[ 2 ] >> 8;
	_data[ 16 ] = A[ 2 ] & 0x00FF;
	_data[ 17 ] = A[ 3 ] >> 8;
	_data[ 18 ] = A[ 3 ] & 0x00FF;
	_data[ 19 ] = A[ 4 ] >> 8;
	_data[ 20 ] = A[ 4 ] & 0x00FF;
	_data[ 21 ] = A[ 5 ] >> 8;
	_data[ 22 ] = A[ 5 ] & 0x00FF;
	_data[ 23 ] = A[ 6 ] >> 8;
	_data[ 24 ] = A[ 6 ] & 0x00FF;
	_data[ 25 ] = A[ 7 ] >> 8;
	_data[ 26 ] = A[ 7 ] & 0x00FF;

	RTC_ReadPulseBytes(1, &(_data[ 27 ]));
	RTC_ReadPulseBytes(2, &(_data[ 30 ]));
	RTC_ReadPulseBytes(3, &(_data[ 33 ]));
	RTC_ReadPulseBytes(4, &(_data[ 36 ]));
	char _tempIO = 0x00;
	//
	//  判断开关口
	//
	//  对于开关的保存数据,
	//  我们首先读取 方向,
	//  对于输入的,我们保存  P5IN的电平值
	//  对于输出的,我们保存 对P5OUT的配置
	char _dir;
	Store_ReadIoDirConfig(&_dir);
	P4DIR = _dir;  //再更新一次,保持与ROM中相同
	char _level;
	Store_ReadIoLevelConfig(&_level);
	P4OUT = _level;	 //再更新一次,保持与ROM中相同

	char _bit = BIT0;
	for (int i = 0; i < 8; ++i) {
		if (_dir & _bit) {	    //表示该位为输出
			if (_level & _bit)  //配置为高电平
				_tempIO |= _bit;
			else
				_tempIO &= ~_bit;
		}
		else {	//表示该位为输入
			if (P4IN & _bit)
				_tempIO |= _bit;
			else
				_tempIO &= ~_bit;
		}
		_bit <<= 1;
	}
	//然后填写
	_data[ 39 ] = _tempIO;
	//写好_data了
	if (Store_WriteDataItemAuto(_data) < 0) {
		return -1;
	}
	return 0;
}

long count_pulse1 = 0;
long count_pulse2 = 0;
long count_pulse3 = 0;

char char_1[ 3 ] = { 0, 0, 0 };
long long_1[ 3 ] = { 0, 0, 0 };
char char_2[ 3 ] = { 0, 0, 0 };
long long_2[ 3 ] = { 0, 0, 0 };
char char_3[ 3 ] = { 0, 0, 0 };
long long_3[ 3 ] = { 0, 0, 0 };

int i;

void chartolong(char* arr1, long* arr2) {
	arr2[ 0 ] = ( long )arr1[ 0 ];
	arr2[ 1 ] = ( long )arr1[ 1 ];
	arr2[ 2 ] = ( long )arr1[ 2 ];
}

void longtochar(char* arr1, long* arr2) {
	arr1[ 0 ] = ( char )arr2[ 0 ];
	arr1[ 1 ] = ( char )arr2[ 1 ];
	arr1[ 2 ] = ( char )arr2[ 2 ];
}

// char ISR_Count_Temp[5] = {0,0,0,0,0};

void ISR_Count_Cal(char* ISR_Count_Arr) {
	long ISR_Count = 0;

	ISR_Count = (ISR_Count_Arr[ 4 ] * 0x100000000) + (ISR_Count_Arr[ 3 ] * 0x1000000)
		    + (ISR_Count_Arr[ 2 ] * 0x10000) + (ISR_Count_Arr[ 1 ] * 0x100) + (ISR_Count_Arr[ 0 ]);
	//  ISR_Count = 255;
	ISR_Count++;

	ISR_Count_Arr[ 4 ] = (ISR_Count & 0xFF00000000) >> 32;
	ISR_Count_Arr[ 3 ] = (ISR_Count & 0x00FF000000) >> 24;
	ISR_Count_Arr[ 2 ] = (ISR_Count & 0x0000FF0000) >> 16;
	ISR_Count_Arr[ 1 ] = (ISR_Count & 0x000000FF00) >> 8;
	ISR_Count_Arr[ 0 ] = (ISR_Count & 0x00000000FF) >> 0;
}

static char s_pulse1_flag = 0;
static char s_pulse2_flag = 0;
static char s_pulse3_flag = 0;
static char s_pulse4_flag = 0;

static char s_pulse5_flag = 0;
static char s_pulse6_flag = 0;
static char s_pulse7_flag = 0;
static char s_pulse8_flag = 0;
extern char s_isr_count_flag;
#pragma vector = PORT1_VECTOR
__interrupt void PORT1_ISR(void) {
	_DINT();
	// char isr_count_flag = 0;
	char	   ISR_Count_Temp1[ 5 ] = { 0, 0, 0, 0, 0 };
	char	   ISR_Count_Temp2[ 5 ] = { 0, 0, 0, 0, 0 };
	char	   ISR_Count_Temp3[ 5 ] = { 0, 0, 0, 0, 0 };
	char	   ISR_Count_Temp4[ 5 ] = { 0, 0, 0, 0, 0 };
	char	   ISR_Count_Temp5[ 5 ] = { 0, 0, 0, 0, 0 };
	char	   ISR_Count_Temp6[ 5 ] = { 0, 0, 0, 0, 0 };
	char	   ISR_Count_Temp7[ 5 ] = { 0, 0, 0, 0, 0 };
	char	   ISR_Count_Temp8[ 5 ] = { 0, 0, 0, 0, 0 };
	static int a[ 8 ]		= { 0 };
	int	   i			= 0;
	// TraceInt4(i++,1);

	// Hydrology_ReadStoreInfo(HYDROLOGY_ISR_COUNT_FLAG,&isr_count_flag,HYDROLOGY_ISR_COUNT_FLAG_LEN);

	if (s_isr_count_flag == 1) {
		//脉冲1
		if (P1IFG & BIT0) {  //设置标记
			P1IFG &= ~(BIT0);
			s_pulse1_flag = 1;
			if (s_pulse1_flag && s_pulse2_flag) {

				Hydrology_ReadStoreInfo(HYDROLOGY_ISR_COUNT1, ISR_Count_Temp1,
							HYDROLOGY_ISR_COUNT_LEN);
				ISR_Count_Cal(ISR_Count_Temp1);
				Hydrology_WriteStoreInfo(HYDROLOGY_ISR_COUNT1, ISR_Count_Temp1,
							 HYDROLOGY_ISR_COUNT_LEN);

				TraceMsg("PULS1.1:", 1);
				TraceInt4(++a[ 0 ], 1);
				s_pulse1_flag = 0;
				s_pulse2_flag = 0;
			}
			// EnP2Int(1);
			// DsP2Int(0);
		}
		if (P1IFG & BIT1) {
			P1IFG &= ~(BIT1);
			s_pulse2_flag = 1;
			//检查标记
			if (s_pulse1_flag && s_pulse2_flag) {
				// RTC_ReadTimeBytes6(s_rtc_nowTime2);

				Hydrology_ReadStoreInfo(HYDROLOGY_ISR_COUNT1, ISR_Count_Temp1,
							HYDROLOGY_ISR_COUNT_LEN);
				ISR_Count_Cal(ISR_Count_Temp1);
				Hydrology_WriteStoreInfo(HYDROLOGY_ISR_COUNT1, ISR_Count_Temp1,
							 HYDROLOGY_ISR_COUNT_LEN);

				TraceMsg("PULS1.2:", 1);
				TraceInt4(++a[ 0 ], 1);
				s_pulse1_flag = 0;
				s_pulse2_flag = 0;
			}
			// DsP2Int(1);
			// EnP2Int(0);//使能对应的另一个中断
		}
		//脉冲2
		if (P1IFG & BIT2) {
			P1IFG &= ~(BIT2);
			s_pulse3_flag = 1;
			if (s_pulse3_flag && s_pulse4_flag) {
				Hydrology_ReadStoreInfo(HYDROLOGY_ISR_COUNT2, ISR_Count_Temp2,
							HYDROLOGY_ISR_COUNT_LEN);
				ISR_Count_Cal(ISR_Count_Temp2);
				Hydrology_WriteStoreInfo(HYDROLOGY_ISR_COUNT2, ISR_Count_Temp2,
							 HYDROLOGY_ISR_COUNT_LEN);

				TraceMsg("PULS2.1:", 1);
				TraceInt4(++a[ 1 ], 1);
				s_pulse3_flag = 0;
				s_pulse4_flag = 0;
			}
			// DsP2Int(2);//然后就不接收这个中断,直到对应的另一个中断到来.
			// EnP2Int(3);//使能对应的另一个中断
		}
		if (P1IFG & BIT3) {
			P1IFG &= ~(BIT3);
			//检查标记
			s_pulse4_flag = 1;
			if (s_pulse3_flag && s_pulse4_flag) {
				Hydrology_ReadStoreInfo(HYDROLOGY_ISR_COUNT2, ISR_Count_Temp2,
							HYDROLOGY_ISR_COUNT_LEN);
				ISR_Count_Cal(ISR_Count_Temp2);
				Hydrology_WriteStoreInfo(HYDROLOGY_ISR_COUNT2, ISR_Count_Temp2,
							 HYDROLOGY_ISR_COUNT_LEN);

				TraceMsg("PULS2.2:", 1);
				TraceInt4(++a[ 1 ], 1);
				s_pulse3_flag = 0;
				s_pulse4_flag = 0;
			}
			// DsP2Int(3); //然后就不接收这个中断,直到对应的另一个中断到来.
			// EnP2Int(2); //使能对应的另一个中断
		}
		//脉冲3
		if (P1IFG & BIT4) {
			P1IFG &= ~(BIT4);
			s_pulse5_flag = 1;
			if (s_pulse5_flag && s_pulse6_flag) {
				Hydrology_ReadStoreInfo(HYDROLOGY_ISR_COUNT3, ISR_Count_Temp3,
							HYDROLOGY_ISR_COUNT_LEN);
				ISR_Count_Cal(ISR_Count_Temp3);
				Hydrology_WriteStoreInfo(HYDROLOGY_ISR_COUNT3, ISR_Count_Temp3,
							 HYDROLOGY_ISR_COUNT_LEN);

				TraceMsg("PULS3.1:", 1);
				TraceInt4(++a[ 2 ], 1);
				s_pulse5_flag = 0;
				s_pulse6_flag = 0;
			}
			// DsP2Int(4);//然后就不接收这个中断,直到对应的另一个中断到来.
			// EnP2Int(5);//使能对应的另一个中断
		}
		if (P1IFG & BIT5) {
			P1IFG &= ~(BIT5);
			//检查标记
			s_pulse6_flag = 1;
			if (s_pulse5_flag && s_pulse6_flag) {

				Hydrology_ReadStoreInfo(HYDROLOGY_ISR_COUNT3, ISR_Count_Temp3,
							HYDROLOGY_ISR_COUNT_LEN);
				ISR_Count_Cal(ISR_Count_Temp3);
				Hydrology_WriteStoreInfo(HYDROLOGY_ISR_COUNT3, ISR_Count_Temp3,
							 HYDROLOGY_ISR_COUNT_LEN);

				TraceMsg("PULS3.2:", 1);
				TraceInt4(++a[ 2 ], 1);
				s_pulse5_flag = 0;
				s_pulse6_flag = 0;
			}
			// DsP2Int(5);//然后就不接收这个中断,直到对应的另一个中断到来.
			// EnP2Int(4);//使能对应的另一个中断
		}
		//脉冲4
		if (P1IFG & BIT6) {
			P1IFG &= ~(BIT6);
			s_pulse7_flag = 1;
			if (s_pulse7_flag && s_pulse8_flag) {
				Hydrology_ReadStoreInfo(HYDROLOGY_ISR_COUNT4, ISR_Count_Temp4,
							HYDROLOGY_ISR_COUNT_LEN);
				ISR_Count_Cal(ISR_Count_Temp4);
				Hydrology_WriteStoreInfo(HYDROLOGY_ISR_COUNT4, ISR_Count_Temp4,
							 HYDROLOGY_ISR_COUNT_LEN);

				TraceMsg("PULS4.1:", 1);
				TraceInt4(++a[ 3 ], 1);
				s_pulse7_flag = 0;
				s_pulse8_flag = 0;
			}
			// DsP2Int(6);//然后就不接收这个中断,直到对应的另一个中断到来.
			// EnP2Int(7);//使能对应的另一个中断
		}
		if (P1IFG & BIT7) {
			P1IFG &= ~(BIT7);
			//检查标记
			s_pulse8_flag = 1;
			if (s_pulse7_flag && s_pulse8_flag) {

				Hydrology_ReadStoreInfo(HYDROLOGY_ISR_COUNT4, ISR_Count_Temp4,
							HYDROLOGY_ISR_COUNT_LEN);
				ISR_Count_Cal(ISR_Count_Temp4);
				Hydrology_WriteStoreInfo(HYDROLOGY_ISR_COUNT4, ISR_Count_Temp4,
							 HYDROLOGY_ISR_COUNT_LEN);

				TraceMsg("PULS4.2:", 1);
				TraceInt4(++a[ 3 ], 1);
				s_pulse7_flag = 0;
				s_pulse8_flag = 0;
			}
			// DsP2Int(7);//然后就不接收这个中断,直到对应的另一个中断到来.
			// EnP2Int(6);//使能对应的另一个中断
		}
	}
	else {
		if (P1IFG & BIT0) {
			P1IFG &= ~(BIT0);
			//设置标记
			Hydrology_ReadStoreInfo(HYDROLOGY_ISR_COUNT1, ISR_Count_Temp1,
						HYDROLOGY_ISR_COUNT_LEN);
			ISR_Count_Cal(ISR_Count_Temp1);
			Hydrology_WriteStoreInfo(HYDROLOGY_ISR_COUNT1, ISR_Count_Temp1,
						 HYDROLOGY_ISR_COUNT_LEN);

			TraceMsg("PULS1:", 1);
			TraceInt4(++a[ 0 ], 1);
		}
		//脉冲2
		if (P1IFG & BIT1) {
			P1IFG &= ~(BIT1);
			//检查标记

			Hydrology_ReadStoreInfo(HYDROLOGY_ISR_COUNT2, ISR_Count_Temp2,
						HYDROLOGY_ISR_COUNT_LEN);
			// TraceHexMsg(ISR_Count_Temp,5);
			ISR_Count_Cal(ISR_Count_Temp2);
			Hydrology_WriteStoreInfo(HYDROLOGY_ISR_COUNT2, ISR_Count_Temp2,
						 HYDROLOGY_ISR_COUNT_LEN);

			TraceMsg("PULS2:", 1);
			TraceInt4(++a[ 1 ], 1);
		}

		//脉冲3
		if (P1IFG & BIT2) {
			P1IFG &= ~(BIT2);

			Hydrology_ReadStoreInfo(HYDROLOGY_ISR_COUNT3, ISR_Count_Temp3,
						HYDROLOGY_ISR_COUNT_LEN);
			// TraceHexMsg(ISR_Count_Temp,5);
			ISR_Count_Cal(ISR_Count_Temp3);
			Hydrology_WriteStoreInfo(HYDROLOGY_ISR_COUNT3, ISR_Count_Temp3,
						 HYDROLOGY_ISR_COUNT_LEN);

			TraceMsg("PULS3:", 1);
			TraceInt4(++a[ 2 ], 1);
		}
		//脉冲4
		if (P1IFG & BIT3) {
			P1IFG &= ~(BIT3);
			Hydrology_ReadStoreInfo(HYDROLOGY_ISR_COUNT4, ISR_Count_Temp4,
						HYDROLOGY_ISR_COUNT_LEN);
			//  TraceHexMsg(ISR_Count_Temp,5);
			ISR_Count_Cal(ISR_Count_Temp4);
			Hydrology_WriteStoreInfo(HYDROLOGY_ISR_COUNT4, ISR_Count_Temp4,
						 HYDROLOGY_ISR_COUNT_LEN);

			TraceMsg("PULS4:", 1);
			TraceInt4(++a[ 3 ], 1);
		}
		//脉冲5
		if (P1IFG & BIT4) {
			P1IFG &= ~(BIT4);
			Hydrology_ReadStoreInfo(HYDROLOGY_ISR_COUNT5, ISR_Count_Temp5,
						HYDROLOGY_ISR_COUNT_LEN);
			//  TraceHexMsg(ISR_Count_Temp,5);
			ISR_Count_Cal(ISR_Count_Temp5);
			Hydrology_WriteStoreInfo(HYDROLOGY_ISR_COUNT5, ISR_Count_Temp5,
						 HYDROLOGY_ISR_COUNT_LEN);

			TraceMsg("PULS5:", 1);
			TraceInt4(++a[ 4 ], 1);
		}
		//脉冲6
		if (P1IFG & BIT5) {
			P1IFG &= ~(BIT5);
			Hydrology_ReadStoreInfo(HYDROLOGY_ISR_COUNT6, ISR_Count_Temp6,
						HYDROLOGY_ISR_COUNT_LEN);
			//  TraceHexMsg(ISR_Count_Temp,5);
			ISR_Count_Cal(ISR_Count_Temp6);
			Hydrology_WriteStoreInfo(HYDROLOGY_ISR_COUNT6, ISR_Count_Temp6,
						 HYDROLOGY_ISR_COUNT_LEN);

			TraceMsg("PULS6:", 1);
			TraceInt4(++a[ 5 ], 1);
		}
		//脉冲7
		if (P1IFG & BIT6) {
			P1IFG &= ~(BIT6);
			Hydrology_ReadStoreInfo(HYDROLOGY_ISR_COUNT7, ISR_Count_Temp7,
						HYDROLOGY_ISR_COUNT_LEN);
			//  TraceHexMsg(ISR_Count_Temp,5);
			ISR_Count_Cal(ISR_Count_Temp7);
			Hydrology_WriteStoreInfo(HYDROLOGY_ISR_COUNT7, ISR_Count_Temp7,
						 HYDROLOGY_ISR_COUNT_LEN);

			TraceMsg("PULS7:", 1);
			TraceInt4(++a[ 6 ], 1);
		}
		//脉冲8
		if (P1IFG & BIT7) {
			P1IFG &= ~(BIT7);

			Hydrology_ReadStoreInfo(HYDROLOGY_ISR_COUNT8, ISR_Count_Temp8,
						HYDROLOGY_ISR_COUNT_LEN);
			//  TraceHexMsg(ISR_Count_Temp,5);
			ISR_Count_Cal(ISR_Count_Temp8);
			Hydrology_WriteStoreInfo(HYDROLOGY_ISR_COUNT8, ISR_Count_Temp8,
						 HYDROLOGY_ISR_COUNT_LEN);

			TraceMsg("PULS8:", 1);
			TraceInt4(++a[ 7 ], 1);
		}
	}
	P1IFG &= 0x00;
	_EINT();
	return;
}