#ifndef REL_M603DLL_FLOW_H
#define REL_M603DLL_FLOW_H
#include "REL/framework/minigame_core.h"
#include "REL/m603DLL/framework/thread.h"
#include "REL/m603DLL/timer_owner.h"
class MinigameThreadOwner : public MinigameCore, public ThreadEntry {
public:
    TimerOwner timer;
    int frame;
    unsigned char unknown360[5]; /* No code names this field. */
    bool nightMode;
    MinigameThreadOwner();
    virtual void setupObjects(OMOBJMAN *);
    virtual void updateGame();
    virtual void finishGame();
    virtual void fn_1_A08();
    virtual void run(void *);
    virtual ~MinigameThreadOwner();
    bool fn_1_99C();
    void fn_1_60C();
    void fn_1_654();
    void fn_1_7C4();
};
#endif
