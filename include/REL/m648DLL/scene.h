#ifndef REL_M648_SCENE_H
#define REL_M648_SCENE_H
#include "dolphin/types.h"
#include "REL/framework/thread.h"
#include "REL/framework/minigame_core.h"
#include "REL/framework/timer.h"

struct TimerManager;
struct MinigameState : MinigameCore, ThreadEntry {
    TimerManager *resource;
    TimerOwner timer;
    s32 progress;
    bool nightMode;
    u32 record;
    MinigameState();
    virtual ~MinigameState();
    virtual void setupObjects(OMOBJMAN *);
    virtual void updateGame();
    virtual void finishGame();
    virtual void fn_1_4148();
    virtual void run(void *);
    s32 updateRecord();
    void showResults();
    void showFinishOnly();
    u8 advanceProgress();
};
#endif
