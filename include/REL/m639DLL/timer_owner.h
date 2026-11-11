#ifndef M639_TIMER_OWNER_H
#define M639_TIMER_OWNER_H
struct MGTIMER;
struct TimerOwner {
    MGTIMER *timer; // Timer state and display resources owned by this object.
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
