// The minigame framework object's HUD timer (timer.cpp). The member is named by the constructor's
// assertion, "m_tm != NULL"; the class and method names are descriptive.
#ifndef REL_FRAMEWORK_TIMER_H
#define REL_FRAMEWORK_TIMER_H

extern "C" {
#include "game/mg/timer.h"
}

// Provides the minigame framework object with control of its HUD timer.
struct TimerOwner {
    MGTIMER *m_tm; // Timer state and display resources owned by this object.
    TimerOwner(int displayType);
    ~TimerOwner();
    void setParameters(int startTime, int endTime, int recordTime);
    void setPosition(float screenX, float screenY);
    void getPosition(float *screenX, float *screenY);
    void setMode(int endBehavior);
    void start();
    void setRecordTime(int recordTime);
    void setBackColor(unsigned char red, unsigned char green, unsigned char blue);
    int getValue();
    void turnModeOff();
    void showRecord();
    void reset();
    unsigned char isDone();
};

#endif
