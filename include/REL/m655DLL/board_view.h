#ifndef M655DLL_BOARD_VIEW_H
#define M655DLL_BOARD_VIEW_H
#include "REL/framework/minigame_core.h"
#include <memory>
#include "REL/framework/thread.h"
#include "REL/framework/event_packet.h"
struct EffectLayer;

struct BoardView : MinigameCore, ThreadEntry, EventListener {
    std::auto_ptr<EffectLayer> nested;
    BoardView();
    virtual ~BoardView();
    virtual void setupObjects(OMOBJMAN *);
    virtual void updateGame();
    virtual void finishGame();
    virtual void run(void *);
    virtual void receive(const EventPacket *);
};
#endif
