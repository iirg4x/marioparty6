#ifndef REL_M624DLL_SEQUENCE_OWNER_H
#define REL_M624DLL_SEQUENCE_OWNER_H

#include "REL/framework/thread.h"
#include "REL/framework/minigame_core.h"
#include "REL/framework/timer.h"
#include "REL/m624DLL/handle_allocator.h"

struct SequenceOwner : MinigameCore, ThreadEntry {
    SequenceOwner();
    virtual ~SequenceOwner();
    virtual void setupObjects(OMOBJMAN *);
    virtual void updateGame();
    virtual void finishGame();
    virtual void run(void *);
    TimerOwner timerState;
    HandleAllocator auxiliaryTimer;
};

#endif
