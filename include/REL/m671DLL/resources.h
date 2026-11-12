#ifndef REL_M671DLL_RESOURCES_H
#define REL_M671DLL_RESOURCES_H

#include <vector>
#include "REL/framework/event_packet.h"
#include "REL/framework/singleton.h"
#include "REL/framework/model.h"
#include "game/animdata.h"

/* The animations and models the whole minigame shares. */
class Resources : public MinigameSingleton<Resources>, public EventListener {
public:
    short lightId;
    std::vector<ANIMDATA *> animations;
    std::vector<OwnedModel *> models;
    Resources();
    virtual ~Resources();
    virtual void receive(const EventPacket *);
    void createEffect(const EventPacket *);
    void fn_1_718C();
    void fn_1_74B8();
    void fn_1_751C();
    void fn_1_75A4();
    void fn_1_763C();
    void fn_1_76B8(const EventPacket *event);
    short id() { return lightId; }
    ANIMDATA *animation(unsigned long index) { return animations.at(index); }
};

ANIMDATA *fn_1_6B8C(u32 dataNumber);
void fn_1_773C(std::vector<OwnedModel *> *models);

#endif
