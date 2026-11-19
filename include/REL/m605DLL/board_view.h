#ifndef REL_M605DLL_BOARD_VIEW_H
#define REL_M605DLL_BOARD_VIEW_H
#include "REL/framework/thread.h"
#include "REL/framework/event_packet.h"
#include "REL/framework/owned_pointer.h"
#include "REL/framework/game_object_man.h"
#include "REL/framework/minigame_core.h"

#include "REL/framework/event_manager.h"

struct EffectLayer;

// Owns the minigame objects and coordinates its thread, events and game updates.
struct BoardView : MinigameCore, ThreadEntry, EventListener {
    OwnedPointer<EffectLayer> effectOwner;
    BoardView();
    virtual ~BoardView();
    virtual void setupObjects(OMOBJMAN *);
    virtual void updateGame();
    virtual void finishGame();
    virtual void receive(const EventPacket *);
    virtual void run(void *);
};
struct Octree;

// Holds the scene collision tree created by AnimationOwner.
extern Octree *lbl_1_bss_2C;

#endif
