#ifndef REL_M622DLL_MINIGAME_OWNER_H
#define REL_M622DLL_MINIGAME_OWNER_H

#include "REL/framework/minigame_core.h"
#include "REL/framework/thread.h"
#include "REL/framework/timer.h"

struct MinigameThreadOwner : MinigameCore, ThreadEntry {
    MinigameThreadOwner();
    virtual ~MinigameThreadOwner();
    virtual void setupObjects(OMOBJMAN *);
    virtual void updateGame();
    virtual void finishGame();
    virtual void fn_1_444();
    virtual void run(void *);
    u8 fn_1_418();
    TimerOwner timerState;
    bool dayMode;
    u8 unk35D[11]; /* No code names this field. */
};

#endif
