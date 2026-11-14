#ifndef M681_GAME_SEQUENCE_H
#define M681_GAME_SEQUENCE_H

#include <memory>
#include "dolphin/types.h"
#include "REL/framework/thread.h"
#include "REL/framework/minigame_core.h"
#include "REL/framework/event_packet.h"

struct GameManagers;

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
    std::auto_ptr<GameManagers> ownedObject;
};

u8 fn_1_1744(GameSequenceController *);

#endif
