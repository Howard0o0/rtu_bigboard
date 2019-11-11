#ifndef  _HYDROLOGYTASK_H_
#define  _HYDROLOGYTASK_H_

#include "stdint.h"



void HydrologyOneMinute();

int HydrologyOnline();

int HydrologyTask();

int HydrologyPacketJudgment();

int HydrologyCancelMark();

void HydrologyTimeBase();

void convertSendTimetoHydrology(char* src,char* dst);


int HydrologyRecord(int index);


extern uint16_t time_1s;
extern uint16_t time_10min;
extern char Picture_Flag;


#endif /* _HYDROLOGYTASK_H_ */