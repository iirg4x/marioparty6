#include <vector>
#ifndef REL_M672DLL_RESOURCES_H
#define REL_M672DLL_RESOURCES_H

#include "REL/framework/event_packet.h"
#include <math.h>
#include "REL/framework/model.h"
extern "C" {
#include "game/hu3d.h"
}

#include "REL/framework/singleton.h"
class Resources;

/* The animations and models the whole minigame shares. */
class Resources : public MinigameSingleton<Resources>, public EventListener {
public:
    short lightId;
    // Return the shared light identifier.
    operator short() { return lightId; }
    std::vector<ANIMDATA *> animations;
    std::vector<OwnedModel *> models;
    ANIMDATA *animation(unsigned long index);
    Resources();
    virtual ~Resources();
    virtual void receive(const EventPacket *);
};
// Return the animation at the supplied index.
inline ANIMDATA *Resources::animation(unsigned long index)
{
    return animations.at(index);
}

extern "C" void fn_1_6E04(Resources *);
void fn_1_71BC(void *);
void fn_1_7230(Resources *);
void fn_1_7404(void *);
void fn_1_7408(void *);

#endif
