#ifndef LOWPOWER_H
#define LOWPOWER_H

enum intervalType_t { INTERVAL_HOUR, INTERVAL_MIN };

void gotoSleepLPM3();

void SleepIfWakeupAcceidently();

void SetSleepInterval(int interval, int interval_type);

/*+++++++++++++++Test++++++++++++++++*/

void LowpowerTest();

/*---------------Test----------------*/

#endif