// Adapts the minigame timer API to the owning framework object's lifecycle and controls.
static const char rcsid[] = "$Id: timer.cpp,v 1.14.2.2 2004/05/17 02:46:28 shohyama Exp $";

#include "dolphin/math.h"

extern "C" {
#include "game/mg/timer.h"
#include "game/flag.h"
#include "REL/framework/assertion.h"
}

void operator delete(void *);

// Provides the minigame framework object with control of its HUD timer.
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

// Minigame object construction creates its timer and rejects unsupported display types.
TimerOwner::TimerOwner(int displayType) {
    switch (displayType) {
    case 0: timer = MgTimerCreate(MGTIMER_TYPE_NORMAL); break;
    case 1: timer = MgTimerCreate(MGTIMER_TYPE_RECORD); break;
    case 2: timer = MgTimerCreate(MGTIMER_TYPE_SCORE); break;
    default: __msl_assertion_failed("false", "timer.cpp", 36); break;
    }
    timer ? (void)0 : __msl_assertion_failed("m_tm != NULL", "timer.cpp", 38);
}

// Minigame object destruction releases the timer process and its display resources.
TimerOwner::~TimerOwner() { MgTimerKill(timer); }

// Minigame setup supplies the starting, ending, and record frame counts; the timer can count up or
// down between the first two.
void TimerOwner::setParameters(int startTime, int endTime, int recordTime) {
    MgTimerParamSet(timer, startTime, endTime, recordTime);
}

// Calls the timer value getter but discards its current frame-count return value.
void TimerOwner::getValue() { MgTimerValueGet(timer); }

// Minigame setup or UI control places the HUD timer in screen-pixel coordinates.
void TimerOwner::setPosition(float screenX, float screenY) {
    MgTimerPosSet(timer, screenX, screenY);
}

// Minigame UI control reads the timer display position in screen pixels.
void TimerOwner::getPosition(float *screenX, float *screenY) {
    MgTimerPosGet(timer, screenX, screenY);
}

// Minigame control starts the timer and selects its end behavior.
void TimerOwner::setMode(int endBehavior) {
    switch (endBehavior) {
    case 0: MgTimerModeOnSet(timer, 0); break;
    case 1: MgTimerModeOnSet(timer, 1); break;
    case 2: MgTimerModeOnSet(timer, 2); break;
    default: __msl_assertion_failed("false", "timer.cpp", 79); break;
    }
}

// Minigame control code puts the timer process in its idle display mode.
void TimerOwner::turnModeOff() { MgTimerModeOffSet(timer); }

// When stopF is clear, this sets it and selects the active mode; the callback then skips counting
// and applies the end behavior.
void TimerOwner::start() {
    if (!timer->stopF) { timer->stopF = TRUE; timer->mode = MGTIMER_MODE_ON; }
}

// Record-screen control shows the normal timer message, or the digit sprites and score box for
// record, score, and wide-score timers.
void TimerOwner::showRecord() { MgTimerRecordDispOn(timer); }

// Minigame control resets the timer before a new run, using the normal timer type.
void TimerOwner::reset() {
    MgTimerKill(timer);
    timer = MgTimerCreate(0);
}

// Minigame control checks whether the current frame count equals the end value.
unsigned char TimerOwner::isDone() { return MgTimerDoneCheck(timer) != 0; }

// Outside practice mode, sets a supplied record; -1 updates the record only when the live time
// beats it.
void TimerOwner::setRecordTime(int recordTime) {
    if (!_CheckFlag(FLAG_MG_PRACTICE)) MgTimerRecordSet(timer, recordTime);
}

// Minigame UI control changes an existing score box's background using 8-bit RGB values.
void TimerOwner::setBackColor(unsigned char red, unsigned char green, unsigned char blue) {
    MgTimerBackColorSet(timer, red, green, blue);
}
