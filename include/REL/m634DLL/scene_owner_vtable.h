#ifndef M634DLL_SCENE_OWNER_VTABLE_H
#define M634DLL_SCENE_OWNER_VTABLE_H
#include <memory>
#include "REL/framework/minigame_core.h"
#include "REL/framework/thread.h"
#include "REL/m634DLL/ActModels.h"
#include "REL/framework/timer.h"
#include "REL/m634DLL/scene_child.h"

struct SceneObjectOwner : MinigameCore, ThreadEntry {
    std::auto_ptr<SceneChild> child;
    std::auto_ptr<ActModels> animation;
    std::auto_ptr<TimerOwner> timer;
    s32 frame;
    u8 mode;
    u8 complete;
    s32 selectedMode;

    SceneObjectOwner();
    virtual ~SceneObjectOwner();
    virtual void setupObjects(OMOBJMAN *manager);
    virtual void updateGame();
    virtual void finishGame();
    virtual void run(void *initialArgument);
    s32 fn_1_D840();
    u8 fn_1_DAFC();
};
#endif
