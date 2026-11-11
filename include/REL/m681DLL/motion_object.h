#ifndef REL_M681DLL_MOTION_OBJECT_H
#define REL_M681DLL_MOTION_OBJECT_H

#include "REL/framework/model.h"
#include "REL/framework/model_motion.h"
#include "REL/framework/game_object.h"
#include "REL/framework/event_packet.h"

struct MotionObject : GameObject, ModelMotion, EventListener {
    const unsigned long *modelData;
    unsigned char enabled;
    MotionObject(Vec *position, int variant);
    virtual ~MotionObject();
    virtual void update();
    virtual void receive(const EventPacket *input);
};

#endif
