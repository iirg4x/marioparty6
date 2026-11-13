#ifndef REL_M681DLL_CHARACTER_MODEL_H
#define REL_M681DLL_CHARACTER_MODEL_H

#include <memory>
#include "REL/framework/model.h"
#include "REL/framework/model_motion.h"
#include "REL/framework/frame_interpolator.h"
#include "REL/framework/game_object.h"
#include "REL/framework/event_packet.h"

class PlayerStand : public GameObject, public ModelMotion, public EventListener {
public:
    s32 player;
    char *standNode;
    s32 firstTimer;
    s32 secondTimer;
    // Empty owner with virtual deletion; the dynamic type is unknown.
    std::auto_ptr<GameObject> nestedResource;
    FrameInterpolator<float> transition;
    f32 *nodeRotation;
    PlayerStand(int player);
    virtual ~PlayerStand();
    virtual void update();
    virtual void receive(const EventPacket *request);
    void fn_1_83F8();
    void fn_1_8450();
};

Vec *fn_1_8184(PlayerStand *owner, Vec *value);

#endif
