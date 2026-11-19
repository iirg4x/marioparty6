#ifndef REL_M605DLL_FALLING_ROCK_H
#define REL_M605DLL_FALLING_ROCK_H

#include <memory>
#include "dolphin/mtx.h"
#include "REL/framework/event_packet.h"
#include "REL/framework/event_callback_inline.h"
#include "REL/framework/game_object.h"
#include "REL/framework/model.h"
#include "REL/framework/registered_value.h"

// Follows the players down the stage as a falling rock.
class FallingRock : public EventListener, public GameObject {
public:
    OwnedModel model;
    Qtrn rotation;
    f32 angle;
    RegisteredValue body;
    Vec previousPosition;
    Sphere sweepStart;
    Sphere sweepEnd;
    RegisteredValue sweep;
    RegisteredValue lower;
    std::auto_ptr<GameObject> effect;
    Vec displacement;
    void (FallingRock::*callback)();
    bool limitReached;
    bool fallEnded;
    f32 verticalStep;

    FallingRock();
    virtual ~FallingRock();
    virtual void receive(const EventPacket *packet);
    virtual void update();
    void fn_1_136C();
    void fn_1_1514();
    void fn_1_15B8();
    void fn_1_1658();
    void fn_1_191C();
    void fn_1_1A30();
};

void fn_1_1BA8(FallingRock *rock);
s32 fn_1_1D60(FallingRock *rock, const Sphere *request);
bool fn_1_1E38(FallingRock *rock, Sphere *resource);

#endif
