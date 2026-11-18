#ifndef REL_M628DLL_EVENT_DISPLACEMENT_H
#define REL_M628DLL_EVENT_DISPLACEMENT_H
#include "dolphin/mtx.h"
#include "REL/framework/event_packet.h"

// Applies an event displacement to the horizontal position.
inline void addEventDisplacement(Vec *position, const EventPacket &event)
{
    Vec *value = (Vec *)event.context;
    position->x += value->x;
    position->z += value->z;
}
#endif
