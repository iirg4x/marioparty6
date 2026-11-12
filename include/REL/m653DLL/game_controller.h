#ifndef M653_GAME_CONTROLLER_H
#define M653_GAME_CONTROLLER_H
#include "REL/framework/minigame_core.h"
#include "REL/framework/thread.h"
#include "REL/framework/timer.h"
#include "REL/framework/event_packet.h"
#include "REL/m653DLL/resource_handles.h"
struct ResourceOwner;
struct GameWorld;

struct M653GameController : MinigameCore, ThreadEntry, EventListener {
    M653GameController();
    ResourcePointer<GameWorld> world;
    TimerOwner timer;
    s32 count;
    virtual void run(void *);
    virtual ~M653GameController();
    virtual void setupObjects(OMOBJMAN *);
    virtual void updateGame();
    virtual void finishGame();
    virtual void receive(const EventPacket *);
};

void fn_1_81AC(s32 firstCategory, void *firstOwner, s32 secondCategory, void *secondOwner);
void fn_1_8640(M653GameController *owner);
OwnedModel *fn_1_872C(M653GameController *owner, s32 selector);
void fn_1_87C4(M653GameController *owner, s32 selector);
void fn_1_8880(M653GameController *owner, s32 selector);
#endif
