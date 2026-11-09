#include "REL/framework/game_object_man.h"
#include "REL/m654DLL/timer_owner.h"
#include "REL/framework/minigame_core.h"
#include "REL/framework/thread.h"
struct Player {
    float fn_1_3719C();
    float fn_1_37264();
};
struct ScalarSourceOwner {
    void *vtable;
    Player *player;
};
extern "C" void fn_1_75BC(void *source, float value);
extern "C" {
extern const unsigned char lbl_1_data_108[];
void __dt__14M639StateOwnerFv(void *, short);
void fn_1_1AFAC(void *, short);
void fn_1_15714(void *, short);
void fn_1_23404(void *, short);
void *fn_1_1E2B4(void *, short);
}
void operator delete(void *);
struct EventManager : MinigameSingleton<EventManager> {
    EventManager();
    ~EventManager();
};
struct OwnedSceneChild {
    EventManager eventManager;
    GameObjectManager registry;
    TimerOwner timerOwner;
    OwnedSceneChild() : timerOwner(0) {}
};
struct SceneChildOwner {
    OwnedSceneChild *child;
    SceneChildOwner(OwnedSceneChild *value) : child(value) {}
    ~SceneChildOwner();
};
struct SceneObjectOwnerData {
    const unsigned char *vtable;
    unsigned long state[7];
    const unsigned char *childVtable;
    unsigned char childState[0x334];
    SceneChildOwner childOwner;
};

extern "C" { extern const unsigned char __vt__16SceneObjectOwner[]; }
extern "C" { void fn_1_20078(void *, short); }
extern "C" { void *fn_1_195E4(void *, short); }

struct SceneObjectOwner : MinigameCore, ThreadEntry {
    SceneObjectOwner();
    virtual ~SceneObjectOwner();
    virtual void setupObjects(OMOBJMAN *);
    virtual void run(void *argument);
    virtual void updateGame();
    virtual void finishGame();
    unsigned char fn_1_10DC();
    SceneChildOwner childOwner;
};
