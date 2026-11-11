#ifndef REL_M672DLL_SCENE_THREAD_OWNER_H
#define REL_M672DLL_SCENE_THREAD_OWNER_H
#include "REL/framework/minigame_core.h"
#include "REL/framework/thread.h"
#include "REL/framework/timer.h"
#include <memory>
struct SceneChild672;

struct SceneOwner672 : MinigameCore, ThreadEntry {
    SceneOwner672();
    virtual void run(void *);
    virtual ~SceneOwner672();
    virtual void setupObjects(OMOBJMAN *);
    virtual void updateGame();
    virtual void finishGame();
    std::auto_ptr<TimerOwner> sequence;
    std::auto_ptr<SceneChild672> child;
};

#endif
