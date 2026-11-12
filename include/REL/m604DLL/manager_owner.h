#ifndef M604_MANAGER_OWNER_H
#define M604_MANAGER_OWNER_H
#include <memory>
#include "dolphin/types.h"
#include "REL/framework/minigame_core.h"
#include "REL/framework/thread.h"
#include "REL/framework/timer.h"
struct ObjectSets;

struct CleanupSequenceOwner : MinigameCore, ThreadEntry {
    CleanupSequenceOwner();
    virtual ~CleanupSequenceOwner();
    virtual void setupObjects(OMOBJMAN *);
    virtual void updateGame();
    virtual void finishGame();
    virtual void run(void *argument);
    std::auto_ptr<ObjectSets> m_sets;
    std::auto_ptr<TimerOwner> m_timer;
    s32 mode;
    s32 selectionState;
    s32 selectedValue;
    s32 timer;
    u8 timerActive;
    s32 nightFlag;
};

#endif
