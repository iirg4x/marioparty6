#ifndef REL_M655DLL_GOAL_H
#define REL_M655DLL_GOAL_H
#include "REL/framework/model.h"
#include "REL/framework/singleton.h"
#include "REL/framework/event_packet.h"
#include <vector>
#include "REL/framework/category_sphere.h"

/* The vector of positions is a base: the assertion in fn_1_31AC reads a bare size(). */
struct Goal : MinigameSingleton<Goal>, std::vector<int>, EventListener {
    void fn_1_3430(float value);
    Vec *fn_1_3470();
    void fn_1_31AC();
    Goal();
    virtual ~Goal();
    virtual void receive(const EventPacket *packet);

    OwnedModel effectObject;
    RegisteredValue light;
};

#endif
