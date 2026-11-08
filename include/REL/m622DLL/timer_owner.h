#ifndef REL_M622DLL_TIMER_OWNER_H
#define REL_M622DLL_TIMER_OWNER_H

extern "C" {
#include "game/mg/timer.h"
}

struct TimerOwner {
    MGTIMER *timer;
    TimerOwner(int displayType);
    ~TimerOwner();
    void setParameters(int startTime, int endTime, int recordTime);
    void setPosition(float screenX, float screenY);
    void getPosition(float *screenX, float *screenY);
    void setMode(int endBehavior);
    void start();
    void setRecordTime(int recordTime);
    void setBackColor(unsigned char red, unsigned char green, unsigned char blue);
    void getValue();
    void turnModeOff();
    void showRecord();
    void reset();
    unsigned char isDone();
};

#endif
