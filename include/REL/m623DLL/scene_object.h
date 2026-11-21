#ifndef REL_M623DLL_SCENE_OBJECT_H
#define REL_M623DLL_SCENE_OBJECT_H
#include "REL/framework/minigame_core.h"
#include "REL/framework/thread.h"
#include "REL/framework/event_packet.h"
#include <memory>
class SceneState;
// Owns the scene thread and the shared scene state.
struct SceneObject : MinigameCore, ThreadEntry, EventListener {
    SceneObject();
    virtual ~SceneObject();
    virtual void setupObjects(OMOBJMAN *);
    virtual void updateGame();
    virtual void finishGame();
    virtual void receive(const EventPacket *);
    virtual void run(void *initialArgument);
    std::auto_ptr<SceneState> state;
};
#endif
