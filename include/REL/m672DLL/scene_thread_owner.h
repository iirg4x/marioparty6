#ifndef REL_M672DLL_SCENE_THREAD_OWNER_H
#define REL_M672DLL_SCENE_THREAD_OWNER_H
#include "REL/framework/minigame_core.h"
#include "REL/m672DLL/framework/thread.h"
#include "REL/m672DLL/framework/owner.h"
struct SceneChild672;
typedef struct MgTimer_s MGTIMER;
struct TimerOwner {
    MGTIMER *timer;
    explicit TimerOwner(int);
    ~TimerOwner();
    unsigned char isDone();
    void setParameters(int, int, int);
    void setPosition(float, float);
    void setMode(int);
    void start();
};

struct SceneOwner672 : MinigameCore, ThreadEntry {
    SceneOwner672();
    virtual void run(void *);
    virtual ~SceneOwner672();
    virtual void setupObjects(OMOBJMAN *);
    virtual void updateGame();
    virtual void finishGame();
    ScenePointer672<TimerOwner> sequence;
    ScenePointer672<SceneChild672> child;
};

#endif
