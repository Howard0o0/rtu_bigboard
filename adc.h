//////////////////////////////////////////////////////
//     文件名: adc.h
//   文件版本: 1.0.0
//   创建时间: 09年 11月30日
//   更新内容: 
//       作者: 林智
//       附注: 无
// 
//////////////////////////////////////////////////////
#ifndef  _ADC_H_
#define  _ADC_H_


extern unsigned int A[16]; 

void ADC_Open();
void ADC_Sample();
int  ADC_ReadAnalogStr(int _index,char *_dest);
void ADC_Close();


#endif /* _ADC_H */
 
