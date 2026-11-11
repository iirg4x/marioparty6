// Adapts the minigame timer API to the owning framework object's lifecycle and controls.
static const char rcsid[] = "$Id: timer.cpp,v 1.14.2.2 2004/05/17 02:46:28 shohyama Exp $";

#include "dolphin/math.h"
#include "REL/framework/timer.h"

extern "C" {
#include "game/flag.h"
#include "REL/framework/assertion.h"
#include "REL/framework/hu_archive.h"
}

// Minigame object construction creates its timer and rejects unsupported display types.
TimerOwner::TimerOwner(int displayType) {
    switch (displayType) {
    case 0: m_tm = MgTimerCreate(MGTIMER_TYPE_NORMAL); break;
    case 1: m_tm = MgTimerCreate(MGTIMER_TYPE_RECORD); break;
    case 2: m_tm = MgTimerCreate(MGTIMER_TYPE_SCORE); break;
    default: __msl_assertion_failed("false", "timer.cpp", 36); break;
    }
    m_tm ? (void)0 : __msl_assertion_failed("m_tm != NULL", "timer.cpp", 38);
}

// Minigame object destruction releases the timer process and its display resources.
TimerOwner::~TimerOwner() { MgTimerKill(m_tm); }

// Minigame setup supplies the starting, ending, and record frame counts; the timer can count up or
// down between the first two.
void TimerOwner::setParameters(int startTime, int endTime, int recordTime) {
    MgTimerParamSet(m_tm, startTime, endTime, recordTime);
}

// Minigame result code reads the current frame count.
int TimerOwner::getValue() { return MgTimerValueGet(m_tm); }

// Minigame setup or UI control places the HUD timer in screen-pixel coordinates.
void TimerOwner::setPosition(float screenX, float screenY) {
    MgTimerPosSet(m_tm, screenX, screenY);
}

// Minigame UI control reads the timer display position in screen pixels.
void TimerOwner::getPosition(float *screenX, float *screenY) {
    MgTimerPosGet(m_tm, screenX, screenY);
}

// Minigame control starts the timer and selects its end behavior.
void TimerOwner::setMode(int endBehavior) {
    switch (endBehavior) {
    case 0: MgTimerModeOnSet(m_tm, 0); break;
    case 1: MgTimerModeOnSet(m_tm, 1); break;
    case 2: MgTimerModeOnSet(m_tm, 2); break;
    default: __msl_assertion_failed("false", "timer.cpp", 79); break;
    }
}

// Minigame control code puts the timer process in its idle display mode.
void TimerOwner::turnModeOff() { MgTimerModeOffSet(m_tm); }

// When stopF is clear, this sets it and selects the active mode; the callback then skips counting
// and applies the end behavior.
void TimerOwner::start() {
    if (!m_tm->stopF) { m_tm->stopF = TRUE; m_tm->mode = MGTIMER_MODE_ON; }
}

// Record-screen control shows the normal timer message, or the digit sprites and score box for
// record, score, and wide-score timers.
void TimerOwner::showRecord() { MgTimerRecordDispOn(m_tm); }

// Minigame control resets the timer before a new run, using the normal timer type.
void TimerOwner::reset() {
    MgTimerKill(m_tm);
    m_tm = MgTimerCreate(0);
}

// Minigame control checks whether the current frame count equals the end value.
unsigned char TimerOwner::isDone() { return MgTimerDoneCheck(m_tm) != 0; }

// Outside practice mode, sets a supplied record; -1 updates the record only when the live time
// beats it.
void TimerOwner::setRecordTime(int recordTime) {
    if (!fn_8006A124(65551)) MgTimerRecordSet(m_tm, recordTime);
}

// Minigame UI control changes an existing score box's background using 8-bit RGB values.
void TimerOwner::setBackColor(unsigned char red, unsigned char green, unsigned char blue) {
    MgTimerBackColorSet(m_tm, red, green, blue);
}
