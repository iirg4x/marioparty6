#ifndef M681_GAME_SEQUENCE_H
#define M681_GAME_SEQUENCE_H

#include "dolphin/types.h"
#include "REL/m681DLL/framework/thread.h"
#include "REL/framework/minigame_core.h"
#include "REL/framework/event_packet.h"

struct EffectLayer;
struct EffectOwner {
    EffectLayer *value;
    explicit EffectOwner(EffectLayer *object) : value(object) {}
    ~EffectOwner();
};

class GameSequenceController : public MinigameCore, public ThreadEntry,
                           public EventListener {
public:
    GameSequenceController();
    virtual ~GameSequenceController();
    virtual void setupObjects(OMOBJMAN *);
    virtual void updateGame();
    virtual void finishGame();
    virtual void receive(const EventPacket *);
    virtual void run(void *);
    u8 eventReceived;
    EffectOwner ownedObject;
};

extern "C" {
}
u8 fn_1_1744(GameSequenceController *);
extern "C" {
}
#endif
