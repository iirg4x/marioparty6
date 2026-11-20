#ifndef REL_M611DLL_MINIGAME_THREAD_OWNER_H
#define REL_M611DLL_MINIGAME_THREAD_OWNER_H

#include "REL/framework/minigame_core.h"
#include "REL/framework/thread.h"
#include "REL/framework/timer.h"

// Runs the minigame sequence and owns its timer and game objects.
struct MinigameThreadOwner : MinigameCore, ThreadEntry {
    TimerOwner timer;
    bool dayMode;
    u8 unk35D[0xB]; /* No code names this field. */
    MinigameThreadOwner();
    virtual void setupObjects(OMOBJMAN *);
    virtual void updateGame();
    virtual void finishGame();
    virtual ~MinigameThreadOwner();
    virtual void fn_1_1C28();
    void run(void *);
    u8 fn_1_1BFC();
};

#endif
