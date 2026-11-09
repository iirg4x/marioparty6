#ifndef REL_M654_TIMER_OWNER_H
#define REL_M654_TIMER_OWNER_H
extern "C" {
#include "game/mg/timer.h"
}
struct TimerOwner {
    MGTIMER *timer;
    TimerOwner(int);
    ~TimerOwner();
    unsigned char isDone();
    void setParameters(int, int, int);
    void setPosition(float, float);
    void setMode(int);
    void start();
};
#endif
