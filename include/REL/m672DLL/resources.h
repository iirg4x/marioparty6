#include <vector>
#ifndef REL_M672DLL_RESOURCES_H
#define REL_M672DLL_RESOURCES_H

#include "REL/framework/event_packet.h"
#include <math.h>
#include "REL/m672DLL/model_bf64.h"

#include "REL/framework/minigame_singleton.h"
class Resources;
typedef MinigameSingleton<Resources> Singleton3488;

/* The animations and models the whole minigame shares. */
class Resources : public Singleton3488, public EventListener {
public:
    short lightId;
    operator short() { return lightId; }
    std::vector<ANIMDATA *> animations;
    std::vector<Model *> models;
    ANIMDATA *animation(unsigned long index);
    Resources();
    virtual ~Resources();
    virtual void receive(const EventPacket *);
};

inline ANIMDATA *Resources::animation(unsigned long index)
{
    return animations.at(index);
}

#endif
