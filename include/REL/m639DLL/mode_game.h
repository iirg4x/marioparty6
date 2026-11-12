#ifndef M639_MODE_GAME_H
#define M639_MODE_GAME_H
// The charge each player has built up, reported by events and read by the gauge and the mode choice.
#include "REL/framework/event_callback_inline.h"
#include "REL/framework/game_object.h"
#include "REL/framework/singleton.h"

class ModeGame : public MinigameSingleton<ModeGame>, public EventListener, public GameObject {
public:
    float values[4];
    ModeGame();
    virtual ~ModeGame();
    virtual void receive(const EventPacket *packet);
    virtual void update();
    float fn_1_1CC(int index);
    float fn_1_2B0(int team);
    bool fn_1_3B0(int team);
};

#endif
