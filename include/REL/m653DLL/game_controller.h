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
#endif
