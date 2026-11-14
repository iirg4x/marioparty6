#ifndef REL_M672DLL_SCENE_THREAD_OWNER_H
#define REL_M672DLL_SCENE_THREAD_OWNER_H
#include "REL/framework/minigame_core.h"
#include "REL/framework/thread.h"
#include "REL/framework/timer.h"
#include <memory>
struct SceneChild;

struct SceneOwner : MinigameCore, ThreadEntry {
    SceneOwner();
    virtual void run(void *);
    virtual ~SceneOwner();
    virtual void setupObjects(OMOBJMAN *);
    virtual void updateGame();
    virtual void finishGame();
    std::auto_ptr<TimerOwner> hudTimer;
    std::auto_ptr<SceneChild> child;
};

#endif
